#include <stdio.h>
#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"
#include "utils/array.h"
#include "utils/float.h"

PG_MODULE_MAGIC;

typedef struct SparsePair
{
    int   i;
    float x;
} SparsePair;


typedef struct SparseVector
{
    int32        vl_len_;
    int          size;
    SparsePair   x[FLEXIBLE_ARRAY_MEMBER];
} SparseVector;

// macros for postgres array
#define ARRNELEMS(x)  ArrayGetNItems(ARR_NDIM(x), ARR_DIMS(x))
#define ARRPTR(x)  ( (float *) ARR_DATA_PTR(x) )

// sparse_vector access macros
#define SET_SIZE(sparse_vector, _size) ( (sparse_vector)->size = _size )
#define SIZE(sparse_vector)           ( (sparse_vector)->size )

#define DatumGetSparseVector(x)   ((SparseVector *) PG_DETOAST_DATUM(x))
#define PG_GETARG_SPARSE_VECTOR_P(x)    DatumGetSparseVector(PG_GETARG_DATUM(x))
#define PG_RETURN_SPARSE_VECTOR_P(x)    PG_RETURN_POINTER(x)
#define SPARSE_VECTOR_SIZE(array_size)    (offsetof(SparseVector, x) + sizeof(SparsePair)*(array_size))

/*
** Input/Output
*/
PG_FUNCTION_INFO_V1(sparse_pair_in);
PG_FUNCTION_INFO_V1(sparse_pair_out);
PG_FUNCTION_INFO_V1(sparse_pair_recv);
PG_FUNCTION_INFO_V1(sparse_pair_send);

PG_FUNCTION_INFO_V1(sparse_vector_in);
PG_FUNCTION_INFO_V1(sparse_vector_out);

PG_FUNCTION_INFO_V1(sparse_vector_a_f4);


/*
** Internal functions
*/
char* sparse_pair_to_str(SparsePair *sparse_pair);
static void insert_sparse_pair(SparsePair *sp, int i, float x);


Datum
sparse_pair_in(PG_FUNCTION_ARGS)
{
    char       *str = PG_GETARG_CSTRING(0);
    int         i;
    float       x;
    SparsePair *result;

    if (sscanf(str, " ( %d , %f )", &i, &x) != 2)
        ereport(
            ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), errmsg("invalid input syntax for sparse pair: \"%s\"", str))
        );

    result = (SparsePair *) palloc(sizeof(SparsePair));
    result->i = i;
    result->x = x;

    PG_RETURN_POINTER(result);
}

char* sparse_pair_to_str(SparsePair *sparse_pair)
{
    char *res;

    res = (char *) palloc(100);
    snprintf(res, 100, "(%d, %f)", sparse_pair->i, sparse_pair->x);
    return res;
}

Datum
sparse_pair_out(PG_FUNCTION_ARGS)
{
    SparsePair *sparse_pair = (SparsePair *) PG_GETARG_POINTER(0);
    char       *res = sparse_pair_to_str(sparse_pair);

    PG_RETURN_CSTRING(res);
}


Datum
sparse_pair_recv(PG_FUNCTION_ARGS)
{
    StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);
    SparsePair *result;

    result = (SparsePair *) palloc(sizeof(SparsePair));
    result->i = pq_getmsgint64(buf);
    result->x = pq_getmsgfloat4(buf);

    PG_RETURN_POINTER(result);
}


Datum
sparse_pair_send(PG_FUNCTION_ARGS)
{
    SparsePair    *sparse_pair = (SparsePair *) PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendint64(&buf, sparse_pair->i);
    pq_sendfloat4(&buf, sparse_pair->x);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

static void insert_sparse_pair(SparsePair *sp, int i, float x)
{
    sp->i = i;
    sp->x = x;
}

Datum
sparse_vector_a_f4(PG_FUNCTION_ARGS)
{
    ArrayType    *a = PG_GETARG_ARRAYTYPE_P(0);
    SparseVector *res;

    int           k,
                  j,
                  array_size,
                  size;

    float        *da;

    j = 0;
    array_size = ARRNELEMS(a);
    da = ARRPTR(a);
    size = SPARSE_VECTOR_SIZE(array_size);
    res = (SparseVector *) palloc0(size);
    SET_VARSIZE(res, size);

    for (k = 0; k < array_size; k++)
    {
        if (da[k] == 0)
            continue;

        insert_sparse_pair(&res->x[j], k, da[k]);
        j++;
    }
    SET_SIZE(res, j);

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

        char* sparse_pair_str = sparse_pair_to_str(&sparse_vector->x[i]);
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

    char       *str = PG_GETARG_CSTRING(0);
    SparseVector      *res;
    PG_RETURN_SPARSE_VECTOR_P(res);
}
