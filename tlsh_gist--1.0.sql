

CREATE FUNCTION tlsh_in(cstring)
    RETURNS tlsh
    AS 'MODULE_PATHNAME', 'tlsh_in'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tlsh_out(tlsh)
    RETURNS cstring
    AS 'MODULE_PATHNAME', 'tlsh_out'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE tlsh (
   internallength = 35,
   input = tlsh_in,
   output = tlsh_out
);