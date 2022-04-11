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

	unsigned char* buffer = (unsigned char*) palloc(TLSH_INTERNAL_LENGTH);
	from_hex(str, TLSH_HASH_LENGTH, buffer);

	PG_RETURN_POINTER(buffer);
}

Datum tlsh_out(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	unsigned char *buffer = (unsigned char *)PG_GETARG_POINTER(0);
	char *str = (char*) palloc(TLSH_HASH_LENGTH + 1);
	to_hex(buffer, TLSH_INTERNAL_LENGTH, str);
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
