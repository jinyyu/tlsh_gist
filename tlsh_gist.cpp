#include "tlsh_gist.h"
#include "tlsh/tlsh.h"

#include "access/reloptions.h"
#include "access/gist.h"
#include "access/stratnum.h"
#include "fmgr.h"
#include "port/pg_bitutils.h"

extern "C"
{
    PG_FUNCTION_INFO_V1(tlsh_dist);
    PG_FUNCTION_INFO_V1(tlsh_consistent);
    PG_FUNCTION_INFO_V1(tlsh_union);
    PG_FUNCTION_INFO_V1(tlsh_penalty);
    PG_FUNCTION_INFO_V1(tlsh_picksplit);
    PG_FUNCTION_INFO_V1(tlsh_same);
}

static void construct_tlsh(Tlsh &tlsh, unsigned char *data)
{
    char hex_str[TLSH_HASH_LENGTH + 1];
    to_hex(data, TLSH_INTERNAL_LENGTH, hex_str);
    hex_str[TLSH_HASH_LENGTH] = 0;
    tlsh.fromTlshStr(hex_str);
    if (!tlsh.isValid())
    {
        elog(ERROR, "invalid tlsh data");
    }
}

Datum tlsh_dist(PG_FUNCTION_ARGS)
{
    unsigned char *left_data = (unsigned char *)PG_GETARG_POINTER(0);
    unsigned char *right_data = (unsigned char *)PG_GETARG_POINTER(1);

    Tlsh tlsh_left;
    construct_tlsh(tlsh_left, left_data);

    Tlsh tlsh_right;
    construct_tlsh(tlsh_right, right_data);

    int diff = tlsh_left.totalDiff(&tlsh_right);
    PG_RETURN_INT32(diff);
}

Datum tlsh_consistent(PG_FUNCTION_ARGS)
{
    elog(INFO, "tlsh_consistent");
    GISTENTRY *entry = (GISTENTRY *)PG_GETARG_POINTER(0);
    Datum query = PG_GETARG_DATUM(1);
    StrategyNumber strategy = (StrategyNumber)PG_GETARG_UINT16(2);
    /* Oid subtype = PG_GETARG_OID(3); */
    bool *recheck = (bool *)PG_GETARG_POINTER(4);
    Datum key = ObjectIdGetDatum(entry->key);
    bool retval;

    if (strategy != DistanceStrategyNumber)
    {
        elog(ERROR, "invalid strategy %d", strategy);
    }

    int32 dist = DatumGetInt32(DirectFunctionCall2(tlsh_dist, query, key));

    retval = (dist <= tlsh_dist_threshold ? true : false);
    *recheck = false; /* or false if check is exact */

    PG_RETURN_BOOL(retval);
}

Datum tlsh_union(PG_FUNCTION_ARGS)
{
    elog(ERROR, "tlsh_union not impl");
}

Datum tlsh_penalty(PG_FUNCTION_ARGS)
{
    elog(INFO, "tlsh_penalty");

    GISTENTRY *origentry = (GISTENTRY *)PG_GETARG_POINTER(0);
    GISTENTRY *newentry = (GISTENTRY *)PG_GETARG_POINTER(1);
    float *penalty = (float *)PG_GETARG_POINTER(2);

    int32 dist = DatumGetInt32(DirectFunctionCall2(tlsh_dist, origentry->key, newentry->key));

    *penalty = float(dist);
    PG_RETURN_POINTER(penalty);
}

Datum tlsh_picksplit(PG_FUNCTION_ARGS)
{
    elog(INFO, "tlsh_picksplit");

    GistEntryVector *entryvec = (GistEntryVector *)PG_GETARG_POINTER(0);
    GIST_SPLITVEC *v = (GIST_SPLITVEC *)PG_GETARG_POINTER(1);
    OffsetNumber maxoff = entryvec->n - 1; /* Valid items in entryvec->vector[] are indexed 1..maxoff */

    memset(v, 0, sizeof(GIST_SPLITVEC));

    int32 dist_table[maxoff + 1][maxoff + 1];
    memset(dist_table, 0, sizeof(dist_table));
    int left_number = 1;
    int right_number = 1;
    int max_dist = 0;

    for (OffsetNumber i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
    {
        for (OffsetNumber j = i + 1; j <= maxoff; j = OffsetNumberNext(j))
        {
            Datum datum_i = entryvec->vector[i].key;
            Datum datum_j = entryvec->vector[j].key;
            int32 dist = DatumGetInt32(DirectFunctionCall2(tlsh_dist, datum_i, datum_j));
            dist_table[i][j] = dist;
            dist_table[j][i] = dist;
            if (dist > max_dist)
            {
                left_number = i;
                right_number = j;
                max_dist = dist;
            }
        }
    }

    v->spl_left = (OffsetNumber *)palloc0(entryvec->n * sizeof(OffsetNumber));
    v->spl_ldatum = entryvec->vector[left_number].key;

    v->spl_right = (OffsetNumber *)palloc0(entryvec->n * sizeof(OffsetNumber));
    v->spl_rdatum = entryvec->vector[right_number].key;

    for (OffsetNumber i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
    {
        int32 left_dist = dist_table[i][left_number];
        int32 right_dist = dist_table[i][right_number];
        if (left_dist < right_dist)
        {
            //离左边近
            v->spl_left[v->spl_nleft++] = i;
        }
        else
        {
            v->spl_right[v->spl_nright++] = i;
        }
    }

    PG_RETURN_POINTER(v);
}

Datum tlsh_same(PG_FUNCTION_ARGS)
{
    elog(INFO, "tlsh_same");

    Datum d1 = ObjectIdGetDatum(PG_GETARG_POINTER(0));
    Datum d2 = ObjectIdGetDatum(PG_GETARG_POINTER(1));
    bool *result = (bool *)PG_GETARG_POINTER(2);

    int ret = memcmp((void *)d1, (void *)d2, TLSH_INTERNAL_LENGTH);
    *result = (ret == 0 ? true : false);
    PG_RETURN_POINTER(result);
}
