#include "tlsh_gist.h"
#include "tlsh/tlsh.h"

extern "C"
{

	PG_FUNCTION_INFO_V1(tlsh_dist);
    PG_FUNCTION_INFO_V1(tlsh_consistent);
    PG_FUNCTION_INFO_V1(tlsh_union);
    PG_FUNCTION_INFO_V1(tlsh_penalty);
    PG_FUNCTION_INFO_V1(tlsh_picksplit);
    PG_FUNCTION_INFO_V1(tlsh_same);
}

static void construct_tlsh(Tlsh& tlsh, unsigned char * data)
{
    char hex_str[TLSH_HASH_LENGTH+1];
    to_hex(data, TLSH_INTERNAL_LENGTH, hex_str);
    hex_str[TLSH_HASH_LENGTH] = 0;
    tlsh.fromTlshStr(hex_str);
    if (!tlsh.isValid()) {
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
   
   int diff =  tlsh_left.totalDiff(&tlsh_right);
   PG_RETURN_INT32(diff);
}


Datum tlsh_consistent(PG_FUNCTION_ARGS)
{
    elog(ERROR, "tlsh_consistent not impl");
}

Datum tlsh_union(PG_FUNCTION_ARGS)
{
    elog(ERROR, "tlsh_union not impl");
}

Datum tlsh_penalty(PG_FUNCTION_ARGS)
{
    elog(ERROR, "tlsh_penalty not impl");
}

Datum tlsh_picksplit(PG_FUNCTION_ARGS)
{
    elog(ERROR, "tlsh_picksplit not impl");
}

Datum tlsh_same(PG_FUNCTION_ARGS)
{
    elog(ERROR, "tlsh_same not impl");
}