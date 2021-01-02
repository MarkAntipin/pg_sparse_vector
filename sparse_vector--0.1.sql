-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION sparse_vector" to load this file. \quit


-- sparse_pair type
CREATE TYPE sparse_pair;

CREATE FUNCTION sparse_pair_in(cstring)
RETURNS sparse_pair
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION sparse_pair_out(sparse_pair)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION sparse_pair_recv(internal)
RETURNS sparse_pair
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION sparse_pair_send(sparse_pair)
RETURNS bytea
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE sparse_pair (
   internallength = 16,
   input = sparse_pair_in,
   output = sparse_pair_out,
   receive = sparse_pair_recv,
   send = sparse_pair_send,
   alignment = double
);


-- sparse_vector type
CREATE TYPE sparse_vector;

CREATE FUNCTION sparse_vector_in(cstring)
RETURNS sparse_vector
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION sparse_vector_out(sparse_vector)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE sparse_vector (
	INTERNALLENGTH = variable,
	INPUT = sparse_vector_in,
	OUTPUT = sparse_vector_out,
	ALIGNMENT = double
);

CREATE FUNCTION sparse_vector(float4[]) RETURNS sparse_vector
AS 'MODULE_PATHNAME', 'sparse_vector_a_f4'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
