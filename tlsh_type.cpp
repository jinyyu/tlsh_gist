#include "tlsh_gist.h"
#include <tlsh/tlsh.h>



extern "C"
{
	PG_FUNCTION_INFO_V1(tlsh_in);
	PG_FUNCTION_INFO_V1(tlsh_out);
	PG_FUNCTION_INFO_V1(tlsh_equal);
}


Datum tlsh_in(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	char *str = PG_GETARG_CSTRING(0);
	if (strlen(str) != TLSH_HASH_LENGTH) {
		elog(ERROR, "invalid tlash hash");
	}

    lsh_bin tmp;
    
	from_hex(str, TLSH_HASH_LENGTH, (uint8*)&tmp);

    lsh_bin* lsh = (lsh_bin*) palloc0(TLSH_INTERNAL_LENGTH);
    
    // Reconstruct checksum, Qrations & lvalue
	for (int k = 0; k < TLSH_CHECKSUM_LEN; k++) {    
		lsh->checksum[k] = swap_byte(tmp.checksum[k]);
	}
	lsh->Lvalue = swap_byte( tmp.Lvalue );
	lsh->Q.QB = swap_byte(tmp.Q.QB);
	for( int i=0; i < CODE_SIZE; i++ ){
		lsh->tmp_code[i] = (tmp.tmp_code[CODE_SIZE-1-i]);
	}

	PG_RETURN_POINTER(lsh);
}

Datum tlsh_out(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	lsh_bin* lsh = (lsh_bin *)PG_GETARG_POINTER(0);
	char *str = (char*) palloc(TLSH_HASH_LENGTH + 1);
	to_hex((uint8*)lsh, TLSH_INTERNAL_LENGTH, str);
	str[TLSH_HASH_LENGTH] = 0;

	PG_RETURN_CSTRING(str);
}

Datum tlsh_equal(PG_FUNCTION_ARGS)
{
	Datum d1 = ObjectIdGetDatum(PG_GETARG_POINTER(0));
    Datum d2 = ObjectIdGetDatum(PG_GETARG_POINTER(1));

    int cmp = tlsh_cmp(d1, d2);
	PG_RETURN_BOOL(cmp == 0);
}
