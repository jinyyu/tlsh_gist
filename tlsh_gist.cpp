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
    PG_FUNCTION_INFO_V1(tlsh_distance);
}

static Datum copy_tlsh(Datum src);
static void construct_tlsh(Tlsh &tlsh, unsigned char *data);

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
    GISTENTRY *entry = (GISTENTRY *)PG_GETARG_POINTER(0);
    Datum query = PG_GETARG_DATUM(1);
    StrategyNumber strategy = (StrategyNumber)PG_GETARG_UINT16(2);
    /* Oid subtype = PG_GETARG_OID(3); */
    bool *recheck = (bool *)PG_GETARG_POINTER(4);
    Datum key = ObjectIdGetDatum(entry->key);
    bool retval;

    switch (strategy)
    {
    case TLSHEqualStrategyNumber:
    case TLSHDistanceStrategyNumber:
        break;
    default:
        elog(ERROR, "invalid strategy %d", strategy);
        break;
    }

    int32 dist = DatumGetInt32(DirectFunctionCall2(tlsh_dist, query, key));

    retval = (dist <= tlsh_dist_threshold ? true : false);
    *recheck = true; /* or false if check is exact */

    PG_RETURN_BOOL(retval);
}

Datum tlsh_union(PG_FUNCTION_ARGS)
{
    GistEntryVector *entryvec = (GistEntryVector *)PG_GETARG_POINTER(0);
    GISTENTRY *enties = entryvec->vector;
    int numranges = entryvec->n;

    if (numranges <= 2)
    {
        int index = rand() % numranges;
        PG_RETURN_POINTER(copy_tlsh(enties[index].key));
    }

    int32 dist_table[numranges + 1][numranges + 1];
    memset(dist_table, 0, sizeof(dist_table));

    for (int i = 1; i < numranges; i++)
    {
        for (int j = i + 1; j < numranges; j++)
        {
            Datum datum_i = enties[i].key;
            Datum datum_j = enties[j].key;
            int32 dist = DatumGetInt32(DirectFunctionCall2(tlsh_dist, datum_i, datum_j));
            dist_table[i][j] = dist;
            dist_table[j][i] = dist;
        }
    }

    int32 sum_table[numranges + 1];
    memset(sum_table, 0, sizeof(sum_table));

    for (int i = 1; i < numranges; i++)
    {
        for (int j = i + 1; j < numranges; j++)
        {
            int32 dist = dist_table[i][j];
            sum_table[i] += dist;
        }
    }

    int min_index = 0;
    int32 min_dist = sum_table[0];
    for (int i = 1; i < numranges; i++) 
    {
        int32 dist_sum = sum_table[i];
        if (dist_sum < min_dist)
        {
            min_index = i;
            min_dist = min_dist;
        }
    }

    PG_RETURN_POINTER(copy_tlsh(enties[min_index].key));
}

Datum tlsh_penalty(PG_FUNCTION_ARGS)
{
    GISTENTRY *origentry = (GISTENTRY *)PG_GETARG_POINTER(0);
    GISTENTRY *newentry = (GISTENTRY *)PG_GETARG_POINTER(1);
    float *penalty = (float *)PG_GETARG_POINTER(2);

    int32 dist = DatumGetInt32(DirectFunctionCall2(tlsh_dist, origentry->key, newentry->key));

    *penalty = float(dist);
    PG_RETURN_POINTER(penalty);
}

Datum tlsh_picksplit(PG_FUNCTION_ARGS)
{
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
    v->spl_ldatum = copy_tlsh(entryvec->vector[left_number].key);

    v->spl_right = (OffsetNumber *)palloc0(entryvec->n * sizeof(OffsetNumber));
    v->spl_rdatum = copy_tlsh(entryvec->vector[right_number].key);

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
    Datum d1 = ObjectIdGetDatum(PG_GETARG_POINTER(0));
    Datum d2 = ObjectIdGetDatum(PG_GETARG_POINTER(1));
    bool *result = (bool *)PG_GETARG_POINTER(2);
    
    int cmp = tlsh_cmp(d1, d2);
    *result = (cmp == 0);
    PG_RETURN_POINTER(result);
}

Datum tlsh_distance(PG_FUNCTION_ARGS)
{
    GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
        Datum  query = PG_GETARG_DATUM(1);
    StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
    /* Oid subtype = PG_GETARG_OID(3); */
    bool *recheck = (bool *) PG_GETARG_POINTER(4);
    Datum key = ObjectIdGetDatum(entry->key);
    
    switch (strategy)
    {
    case TLSHEqualStrategyNumber:
    case TLSHDistanceStrategyNumber:
        break;
    default:
        elog(ERROR, "invalid strategy %d", strategy);
        break;
    }
    
    int32 dist = DatumGetInt32(DirectFunctionCall2(tlsh_dist, query, key));
    double retval = double(dist);

    *recheck = true; /* or false if check is exact */

    PG_RETURN_FLOAT8(retval);
}

Datum copy_tlsh(Datum src)
{
    Datum dst = (Datum)palloc(TLSH_INTERNAL_LENGTH);
    memcpy((void *)dst, (void *)src, TLSH_INTERNAL_LENGTH);
    return dst;
}

void construct_tlsh(Tlsh &tlsh, unsigned char *data)
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
