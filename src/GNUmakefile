PG_CONFIG?=pg_config

OBJS = pg_varint.o varint.o
MODULE_big = varint
EXTENSION = varint
DATA = varint--1.0.sql
DOCS = README.varint
EXTRA_CLEAN = test_encoding

REGRESS = varint

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

test_encoding: test_encoding.o varint.o
	$(CC) $(CFLAGS) -o test_encoding test_encoding.o varint.o

test_encoding.o:  test_encoding.c
	$(CC) $(CFLAGS) -c -o test_encoding.o test_encoding.c

pg_config::
	@echo $(shell which $(PG_CONFIG))
	@$(PG_CONFIG)
