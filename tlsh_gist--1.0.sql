

CREATE FUNCTION tlsh_in(cstring)
    RETURNS tlsh
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tlsh_out(tlsh)
    RETURNS cstring
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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

CREATE FUNCTION tlsh_similarity_op(tlsh,tlsh)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT STABLE PARALLEL SAFE;  -- stable because depends on pg_trgm.similarity_threshold

CREATE OPERATOR % (
        LEFTARG = tlsh,
        RIGHTARG = tlsh,
        PROCEDURE = tlsh_similarity_op,
        COMMUTATOR = '%',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

CREATE FUNCTION tlsh_equal(tlsh, tlsh)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE OPERATOR = (
        LEFTARG = tlsh,
        RIGHTARG = tlsh,
        PROCEDURE = tlsh_equal,
        COMMUTATOR = '='
);


CREATE FUNCTION tlsh_mean(tlsh, tlsh)
RETURNS tlsh
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;



-- gist function

CREATE FUNCTION gtlsh_in(cstring)
    RETURNS gtlsh
    AS 'MODULE_PATHNAME', 'tlsh_in'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;


CREATE FUNCTION gtlsh_out(gtlsh)
    RETURNS cstring
    AS 'MODULE_PATHNAME', 'tlsh_in'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;


CREATE TYPE gtlsh (
   internallength = 35,
   input = gtlsh_in,
   output = gtlsh_out
);


CREATE FUNCTION tlsh_consistent(internal, tlsh, smallint, oid, internal)
RETURNS bool
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT STABLE PARALLEL SAFE;

CREATE FUNCTION tlsh_union(internal, internal)
RETURNS gtlsh
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tlsh_compress(internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tlsh_decompress(internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE FUNCTION tlsh_penalty(internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tlsh_picksplit(internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tlsh_same(gtlsh, gtlsh, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tlsh_distance(internal, tlsh, smallint, oid, internal)
RETURNS float8
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE PARALLEL SAFE;


CREATE OPERATOR CLASS gist_tlsh_ops
FOR TYPE tlsh USING gist
AS
        OPERATOR	    1	    = ,
        OPERATOR        2       <-> (tlsh, tlsh) FOR ORDER BY pg_catalog.integer_ops,
        OPERATOR        3       % (tlsh, tlsh),
        FUNCTION        1       tlsh_consistent (internal, tlsh, smallint, oid, internal),
        FUNCTION        2       tlsh_union (internal, internal),
        FUNCTION        3       tlsh_compress (internal),
        FUNCTION        4       tlsh_decompress (internal),
        FUNCTION        5       tlsh_penalty (internal, internal, internal),
        FUNCTION        6       tlsh_picksplit (internal, internal),
        FUNCTION        7       tlsh_same (gtlsh, gtlsh, internal),
        FUNCTION        8       (tlsh, tlsh)tlsh_distance (internal, tlsh, smallint, oid, internal),
        STORAGE         gtlsh;