#include <stdio.h>
#include <math.h>

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"
#include "sparse_vector.h"
#include "utils/array.h"
#include "utils/float.h"

PG_MODULE_MAGIC;

// macros for postgres array
#define ARRNELEMS(x)  ArrayGetNItems(ARR_NDIM(x), ARR_DIMS(x))
#define ARRPTR(x)  ( (float *) ARR_DATA_PTR(x) )

/*
** Input/Output
*/
PG_FUNCTION_INFO_V1(sparse_vector_in);
PG_FUNCTION_INFO_V1(sparse_vector_out);

PG_FUNCTION_INFO_V1(sparse_vector_a_f4);
PG_FUNCTION_INFO_V1(sparse_vector_a_f4_norm);
PG_FUNCTION_INFO_V1(sparse_vector_dot_product);
PG_FUNCTION_INFO_V1(sparse_vector_cosine_similarity);

/*
** Internal functions
*/
char* _sparse_pair_to_str(SparsePair *sparse_pair);
static void _insert_sparse_pair(SparsePair *sp, int i, float x);
float _sparse_vector_norm(SparseVector *sparse_vector);
float _array_norm(float *a, int array_size);
SparseVector* _sparse_vector_from_array(ArrayType *a, bool is_norm);
float _sparse_vector_dot_product(SparseVector *a, SparseVector *b);

char*
_sparse_pair_to_str(SparsePair *sparse_pair)
{
    char *res;

    res = (char *) palloc(100);
    snprintf(res, 100, "(%d, %f)", sparse_pair->i, sparse_pair->x);
    return res;
}

static void
_insert_sparse_pair(SparsePair *sp, int i, float x)
{
    sp->i = i;
    sp->x = x;
}

float
_array_norm(float *a, int array_size)
{
    int   i;
    float res = 0.0;

    for (i = 0; i < array_size; i++)
    {
        res += (a[i] * a[i]);
    }
    return sqrt(res);
}

SparseVector*
_sparse_vector_from_array(ArrayType *a, bool normalize)
{
    SparseVector *res;
    int           k,
                  j,
                  array_size,
                  size;
    float         norm,
                  sp_value;
    float        *da;

    j = 0;
    norm = 1;
    array_size = ARRNELEMS(a);
    da = ARRPTR(a);
    size = SPARSE_VECTOR_SIZE(array_size);
    res = (SparseVector *) palloc0(size);
    SET_VARSIZE(res, size);

    if (normalize)
        norm = _array_norm(da, array_size);

    for (k = 0; k < array_size; k++)
    {
        if (da[k] == 0)
            continue;

        if (normalize)
            sp_value = da[k] / norm;
        else
            sp_value = da[k];

        _insert_sparse_pair(&res->x[j], k, sp_value);
        j++;
    }
    SET_SIZE(res, j);
    return res;
}

Datum
sparse_vector_a_f4(PG_FUNCTION_ARGS)
{
    ArrayType    *a = PG_GETARG_ARRAYTYPE_P(0);
    SparseVector *res = _sparse_vector_from_array(a, false);

    PG_RETURN_SPARSE_VECTOR_P(res);
}

Datum
sparse_vector_a_f4_norm(PG_FUNCTION_ARGS)
{
    ArrayType    *a = PG_GETARG_ARRAYTYPE_P(0);
    SparseVector *res = _sparse_vector_from_array(a, true);

    PG_RETURN_SPARSE_VECTOR_P(res);
}

Datum
sparse_vector_out(PG_FUNCTION_ARGS)
{
    SparseVector  *sparse_vector = PG_GETARG_SPARSE_VECTOR_P(0);
    StringInfoData buf;
    int            size = SIZE(sparse_vector);
    int            i;

    initStringInfo(&buf);

    appendStringInfoChar(&buf, '(');

    for (i = 0; i < size; i++)
    {
        if (i > 0)
            appendStringInfoString(&buf, ", ");

        char* sparse_pair_str = _sparse_pair_to_str(&sparse_vector->x[i]);
        appendStringInfoString(&buf, sparse_pair_str);

    }
    appendStringInfoChar(&buf, ')');

    PG_FREE_IF_COPY(sparse_vector, 0);
    PG_RETURN_CSTRING(buf.data);
}

Datum
sparse_vector_in(PG_FUNCTION_ARGS)
{
    // not implemented; you have to use sparse_vector_a_f4 to get sparse_vector
    ereport(ERROR,
        (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
         errmsg("not implemented; use sparse_vector(ARRAY[])")));

    SparseVector *res = NULL;
    PG_RETURN_SPARSE_VECTOR_P(res);
}

float
_sparse_vector_norm(SparseVector *sparse_vector)
{
    int   i;
    int   size = SIZE(sparse_vector);
    float res = 0.0;

    for (i = 0; i < size; i++)
    {
        res += (sparse_vector->x[i].x * sparse_vector->x[i].x);
    }
    return sqrt(res);
}

float
_sparse_vector_dot_product(SparseVector *a, SparseVector *b)
{
    float           res = 0.0;
    int             k = 0,
                    j = 0,
                    size_a = SIZE(a),
                    size_b = SIZE(b);

    while (k < size_a && j < size_b)
    {
        if (a->x[k].i > b->x[j].i) {
            j++;
        } else if (a->x[k].i < b->x[j].i) {
            k++;
        }
        else {
            res += (a->x[k].x * b->x[j].x);
            j++;
            k++;
        }
    }

    return res;
}


Datum
sparse_vector_dot_product(PG_FUNCTION_ARGS)
{
    SparseVector  *a = PG_GETARG_SPARSE_VECTOR_P(0),
                  *b = PG_GETARG_SPARSE_VECTOR_P(1);

    float          res = _sparse_vector_dot_product(a, b);

    PG_RETURN_FLOAT4(res);
}

Datum
sparse_vector_cosine_similarity(PG_FUNCTION_ARGS)
{
    SparseVector  *a = PG_GETARG_SPARSE_VECTOR_P(0),
                  *b = PG_GETARG_SPARSE_VECTOR_P(1);

    float          res = _sparse_vector_dot_product(a, b) /
        (_sparse_vector_norm(a) * _sparse_vector_norm(b));

    PG_RETURN_FLOAT4(res);
}
