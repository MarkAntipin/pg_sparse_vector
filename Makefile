MODULE_big = sparse_vector
OBJS = sparse_vector.o
MODULES = sparse_vector
EXTENSION = sparse_vector
DATA = sparse_vector--0.1.sql

HEADERS = sparse_vector.h

SHLIB_LINK += $(filter -lm, $(LIBS))

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
