-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION varint" to load this file. \quit

CREATE FUNCTION varint64_in(cstring)
   RETURNS varint64
   AS 'MODULE_PATHNAME'
   LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_in(cstring) IS 'Convert a cstring to varint64';

CREATE FUNCTION varint64_out(varint64)
   RETURNS cstring
   AS 'MODULE_PATHNAME'
   LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_in(cstring) IS 'Convert a varint64 to cstring';

CREATE FUNCTION varint64_recv(internal)
   RETURNS varint64
   AS 'MODULE_PATHNAME'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION varint64_send(varint64)
   RETURNS bytea
   AS 'MODULE_PATHNAME'
   LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE varint64 (
   internallength = VARIABLE,
   input = varint64_in,
   output = varint64_out,
   receive = varint64_recv,
   send = varint64_send,
   alignment = int4,
   storage = main
);

CREATE FUNCTION varint64_lt(varint64, varint64) RETURNS bool
   AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_lt(varint64, varint64) IS 'Returns true if a < b';

CREATE FUNCTION varint64_le(varint64, varint64) RETURNS bool
   AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_le(varint64, varint64) IS 'Returns true if a <= b';

CREATE FUNCTION varint64_eq(varint64, varint64) RETURNS bool
   AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_eq(varint64, varint64) IS 'Returns true if a = b';

CREATE FUNCTION varint64_ne(varint64, varint64) RETURNS bool
   AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_ne(varint64, varint64) IS 'Returns true if a != b';

CREATE FUNCTION varint64_ge(varint64, varint64) RETURNS bool
   AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_ge(varint64, varint64) IS 'Returns true if a >= b';

CREATE FUNCTION varint64_gt(varint64, varint64) RETURNS bool
   AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_gt(varint64, varint64) IS 'Returns true if a > b';

CREATE OPERATOR < (
   leftarg = varint64, rightarg = varint64, procedure = varint64_lt,
   commutator = > , negator = >= ,
   restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR <= (
   leftarg = varint64, rightarg = varint64, procedure = varint64_le,
   commutator = >= , negator = > ,
   restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR = (
   leftarg = varint64, rightarg = varint64, procedure = varint64_eq,
   commutator = = , negator = <> ,
   restrict = eqsel, join = eqjoinsel
);
CREATE OPERATOR <> (
   leftarg = varint64, rightarg = varint64, procedure = varint64_ne,
   commutator = <> , negator = = ,
   restrict = neqsel, join = neqjoinsel
);
CREATE OPERATOR >= (
   leftarg = varint64, rightarg = varint64, procedure = varint64_ge,
   commutator = <= , negator = < ,
   restrict = scalargtsel, join = scalargtjoinsel
);
CREATE OPERATOR > (
   leftarg = varint64, rightarg = varint64, procedure = varint64_gt,
   commutator = < , negator = <= ,
   restrict = scalargtsel, join = scalargtjoinsel
);

CREATE FUNCTION varint64_cmp(varint64, varint64) RETURNS int4
   AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64_cmp(varint64, varint64) IS 'varint64 B-tree comparison operator';

CREATE OPERATOR CLASS varint64_ops
    DEFAULT FOR TYPE varint64 USING btree AS
        OPERATOR        1       < ,
        OPERATOR        2       <= ,
        OPERATOR        3       = ,
        OPERATOR        4       >= ,
        OPERATOR        5       > ,
        FUNCTION        1       varint64_cmp(varint64, varint64);


CREATE FUNCTION varint64(int2) RETURNS varint64 AS 'MODULE_PATHNAME', 'int2_to_varint64'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64(int2) IS 'Convert a varint64 to an int2';

CREATE FUNCTION varint64(int4) RETURNS varint64 AS 'MODULE_PATHNAME', 'int4_to_varint64'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64(int4) IS 'Convert a varint64 to an int4';

CREATE FUNCTION varint64(int8) RETURNS varint64 AS 'MODULE_PATHNAME', 'int8_to_varint64'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION varint64(int8) IS 'Convert a varint64 to an int8';

CREATE FUNCTION int2(varint64) RETURNS int2 AS 'MODULE_PATHNAME', 'varint64_to_int2'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION int2(varint64) IS 'Convert an int2 to varint64';

CREATE FUNCTION int4(varint64) RETURNS int4 AS 'MODULE_PATHNAME', 'varint64_to_int4'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION int4(varint64) IS 'Convert an int4 to varint64';

CREATE FUNCTION int8(varint64) RETURNS int8 AS 'MODULE_PATHNAME', 'varint64_to_int8'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION int8(varint64) IS 'Convert an int8 to varint64';

CREATE CAST (int2 AS varint64) WITH FUNCTION varint64(int2) AS IMPLICIT;
CREATE CAST (int4 AS varint64) WITH FUNCTION varint64(int4) AS IMPLICIT;
CREATE CAST (int8 AS varint64) WITH FUNCTION varint64(int8) AS IMPLICIT;

CREATE CAST (varint64 AS int2) WITH FUNCTION int2(varint64) AS IMPLICIT;
CREATE CAST (varint64 AS int4) WITH FUNCTION int4(varint64) AS IMPLICIT;
CREATE CAST (varint64 AS int8) WITH FUNCTION int8(varint64) AS IMPLICIT;

-- Deliberately using a function name different than LENGTH because the
-- semantic meaning of sizeof() is different than that of LENGTH (the size of
-- a VARINT64 vs the length of the string representation, respectively).
CREATE FUNCTION sizeof(varint64) RETURNS integer AS 'MODULE_PATHNAME', 'varint64_sizeof'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION sizeof(varint64) IS 'Returns the complete on-disk size of the varint64 (including VARLENA header)';

CREATE FUNCTION sizeof(varint64, bool) RETURNS integer AS 'MODULE_PATHNAME', 'varint64_sizeof2'
  LANGUAGE C IMMUTABLE STRICT;
COMMENT ON FUNCTION sizeof(varint64, bool) IS 'Returns the complete on-disk size of the varint64. When the extra argument is set, it includes the VARLENA header in the calculation.';
