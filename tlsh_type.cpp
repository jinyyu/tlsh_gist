#include "tlsh_gist.h"
#include <tlsh/tlsh.h>

extern "C" {

PG_FUNCTION_INFO_V1(tlsh_in);
PG_FUNCTION_INFO_V1(tlsh_out);

}


Datum
tlsh_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    Tlsh tlsh;
    int ret = tlsh.fromTlshStr(str);
    if (ret != 0 || tlsh.isValid()) {
           elog(ERROR, "invalid tlsh hash");
    }

    char * buffer = (char *) palloc( TLSH_STRING_BUFFER_LEN );
    tlsh.getHash(buffer, TLSH_STRING_BUFFER_LEN);
    PG_RETURN_POINTER(buffer);
}