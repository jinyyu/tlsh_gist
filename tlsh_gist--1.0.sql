

CREATE FUNCTION tlsh_in(cstring)
    RETURNS tlsh
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tlsh_out(tlsh)
    RETURNS cstring
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE tlsh (
   internallength = 35,
   input = tlsh_in,
   output = tlsh_out
);



CREATE FUNCTION tlsh_dist(tlsh,tlsh)
RETURNS int4
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OPERATOR <-> (
        LEFTARG = tlsh,
        RIGHTARG = tlsh,
        PROCEDURE = tlsh_dist,
        COMMUTATOR = '<->'
);