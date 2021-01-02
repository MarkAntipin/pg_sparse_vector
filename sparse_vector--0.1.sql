-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION sparse_vector" to load this file. \quit


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

CREATE FUNCTION dot_product(sparse_vector, sparse_vector) RETURNS float4
AS 'MODULE_PATHNAME', 'sparse_vector_dot_product'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
