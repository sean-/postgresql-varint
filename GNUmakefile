PG_CONFIG   ?= pg_config

EXTENSION = varint
EXTVERSION = $(shell grep default_version $(EXTENSION).control | \
    sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

DATA         = $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql))
DOCS         = README
TESTS        = $(wildcard test/sql/*.sql)
#TESTS        = test/sql/varint-test.sql
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test
OBJS         = src/pg_varint.o src/varint.o
MODULE_big   = varint
EXTRA_CLEAN  = src/test_encoding \
	test/log/ \
	test/results/ \
	test/regression.* \
	test/tmp_check/ \
	$(EXTENSION).so

PG_RECENT = $(shell $(PG_CONFIG) --version | grep -qE " 8\.| 9\.0" && echo no || echo yes)

ifeq ($(PG_RECENT),yes)
all: sql/$(EXTENSION)--$(EXTVERSION).sql

sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
	cp $< $@

DATA = $(wildcard sql/*--*.sql) sql/$(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN = sql/$(EXTENSION)--$(EXTVERSION).sql
endif

src/test_encoding: src/test_encoding.o src/varint.o
	$(CC) $(CFLAGS) -o src/test_encoding src/test_encoding.o src/varint.o

src/test_encoding.o:  src/test_encoding.c
	$(CC) $(CFLAGS) -c -o src/test_encoding.o src/test_encoding.c

pg_config::
	@echo $(shell which $(PG_CONFIG))
	@$(PG_CONFIG)

EXTRA_CLEAN+= src/test_encoding src/test_encoding.o
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
