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

// sparse_vector access macros
#define SET_SIZE(sparse_vector, _size) ( (sparse_vector)->size = _size )
#define SIZE(sparse_vector)           ( (sparse_vector)->size )

#define DatumGetSparseVector(x)   ((SparseVector *) PG_DETOAST_DATUM(x))
#define PG_GETARG_SPARSE_VECTOR_P(x)    DatumGetSparseVector(PG_GETARG_DATUM(x))
#define PG_RETURN_SPARSE_VECTOR_P(x)    PG_RETURN_POINTER(x)
#define SPARSE_VECTOR_SIZE(array_size)    (offsetof(SparseVector, x) + sizeof(SparsePair)*(array_size))
