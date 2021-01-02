MODULE_big = sparse_vector
MODULES = sparse_vector
EXTENSION = sparse_vector
DATA = sparse_vector--0.1.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
