# pg_sparse_vector
PostgreSQL extension. It adds support for sparse vector object and allowing to perform operations on them

### installation
```bash
git clone https://github.com/MarkAntipin/pg_sparse_vector.git
cd pg_sparse_vector
make install
```
In PostgreSQL console:
```postgresql
create extension sparse_vector;
```

### requirements
Assuming that you have installed PostgreSQL already,
you have to install `postgresql-server-dev-{PostgreSQL version}`

 ```bash
sudo apt install postgresql-server-dev-{PostgreSQL version}
export PATH=/usr/lib/postgresql/{PostgreSQL version}/bin:$PATH
pg_config --pgxs
```

Make sure everything is installed
 ```bash
pg_config --pgxs
```

### What is in extension?
New type `sparse_vector` which allows you to efficiently store Sparse Vectors

```postgresql
create table sparse_data (
    vector sparse_vector
);
```
To create `sparse_vector` you can use sparse_vector(ARRAY[]) function
```postgresql
insert into sparse_data (vector)
values (sparse_vector(ARRAY[1.1, 1.4, 5, 0, 3, 0, 0, 4.34]));
```
Or you can immediately normalize your vector with sparse_vector_norm(ARRAY[])
```postgresql
insert into sparse_data (vector)
values (sparse_vector_norm(ARRAY[1.1, 1.4, 5, 0, 3, 0, 0, 4.34]));
```

To find cosine similarity between your vectors you can run
```postgresql
select cosine_similarity(sparse_vector(ARRAY[4.678, 0, 0, 12]), vector) as sim
from sparse_data;
```
Or if all vectors in your table are normalized (it will be faster)
```postgresql
select dot_product(sparse_vector_norm(ARRAY[4.678, 0, 0, 12]), vector) as sim
from sparse_data;
```
