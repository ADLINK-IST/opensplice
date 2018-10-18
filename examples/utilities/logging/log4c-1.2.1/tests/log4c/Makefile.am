INCLUDES = \
	-I$(top_srcdir)/src \
	-DSRCDIR="\"$(srcdir)\""

noinst_PROGRAMS = test_category test_rc bench bench_fwrite \
	test_stream2 test_layout_r cpp_compile_test

if WITH_ROLLINGFILE
noinst_PROGRAMS += test_rollingfile_appender test_rollingfile_appender_mt
endif

cpp_compile_test_SOURCES = cpp_compile_test.cpp

test_category_SOURCES = test_category.c
test_category_LDADD   = $(top_builddir)/src/log4c/liblog4c.la

test_rc_SOURCES = test_rc.c
test_rc_LDADD   = $(top_builddir)/src/log4c/liblog4c.la

bench_SOURCES = bench.c
bench_LDADD   = $(top_builddir)/src/log4c/liblog4c.la

bench_fwrite_SOURCES = bench_fwrite.c
bench_fwrite_LDADD = $(top_builddir)/src/log4c/liblog4c.la -lpthread

test_stream2_SOURCES = \
	test_stream2.c
test_stream2_LDADD   = $(top_builddir)/src/log4c/liblog4c.la

test_layout_r_SOURCES = \
	test_layout_r.c
test_layout_r_LDADD   = $(top_builddir)/src/log4c/liblog4c.la

if WITH_ROLLINGFILE
test_rollingfile_appender_SOURCES = test_rollingfile_appender.c
test_rollingfile_appender_LDADD =  $(top_builddir)/src/log4c/liblog4c.la

test_rollingfile_appender_mt_SOURCES = test_rollingfile_appender_mt.c
test_rollingfile_appender_mt_LDADD =  $(top_builddir)/src/log4c/liblog4c.la \
                                  -lpthread
endif

EXTRA_DIST = \
        test_category.ref \
	test_rc.in \
	test_rc.ref

bench.mmap:
	dd if=/dev/zero of=$@ bs=1k count=64

all-local: bench.mmap 

check-local: 
	@for i in test_category test_rc; do \
	    ./$$i || exit 1; \
	    test -f $(srcdir)/$$i.ref && diff $$i.out $(srcdir)/$$i.ref || exit 1; \
	done

clean-local:
	$(RM) *.out bench.mmap

