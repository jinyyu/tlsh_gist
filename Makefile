MODULE_big = tlsh_gist
OBJS = tlsh_gist.o
EXTENSION = tlsh_gist
DATA = tlsh_gist.sql
REGRESS = tlsh_gist

# add include and library paths for both Instant Client and regular Client
PG_CPPFLAGS = -I/usr/local/include
SHLIB_LINK = -L/usr/local/lib/ -ltlsh

ifdef NO_PGXS
subdir = contrib/tlsh_gist
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
else
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
endif