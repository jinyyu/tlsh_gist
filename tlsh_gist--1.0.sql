

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

CREATE FUNCTION tlsh_equal(tlsh, tlsh)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tlsh_mean(tlsh, tlsh)
RETURNS tlsh
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OPERATOR = (
        LEFTARG = tlsh,
        RIGHTARG = tlsh,
        PROCEDURE = tlsh_equal,
        COMMUTATOR = '='
);


-- gist function
CREATE FUNCTION tlsh_same(tlsh, tlsh)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;


CREATE FUNCTION tlsh_consistent(internal, tlsh, smallint, oid, internal)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION tlsh_union(internal, internal)
RETURNS tlsh
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION tlsh_penalty(internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION tlsh_picksplit(internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION tlsh_same(tlsh, tlsh, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION tlsh_distance(internal, tlsh, smallint, oid, internal)
RETURNS float8
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;


CREATE OPERATOR CLASS gist_tlsh_ops
FOR TYPE tlsh USING gist
AS
        OPERATOR	    1	    = ,
        OPERATOR        2       <-> (tlsh, tlsh) FOR ORDER BY pg_catalog.float_ops,
        FUNCTION        1       tlsh_consistent (internal, tlsh, smallint, oid, internal),
        FUNCTION        2       tlsh_union (internal, internal),
        FUNCTION        5       tlsh_penalty (internal, internal, internal),
        FUNCTION        6       tlsh_picksplit (internal, internal),
        FUNCTION        7       tlsh_same (tlsh, tlsh, internal),
        FUNCTION        8       tlsh_distance (internal, tlsh, smallint, oid, internal),
        STORAGE         tlsh;