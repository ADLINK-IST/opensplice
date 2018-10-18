
AM_CPPFLAGS = -I${top_srcdir}/src

SUBDIRS = helloworld helloworld1

DIST_SUBDIRS = helloworld helloworld1 

EXTRA_DIST=log4crc README

noinst_LTLIBRARIES = liblog4c_examples.la liblog4c_userloc.la
noinst_PROGRAMS = application_1 application_2 application_3

liblog4c_examples_la_SOURCES = example_formatters.c example_appenders.c
liblog4c_userloc_la_SOURCES= userloc_formatter.c

application_1_SOURCES = application_1.c
application_2_SOURCES = application_2.c
application_3_SOURCES = application_3.c application_3.h

application_1_LDADD = \
	$(top_builddir)/src/log4c/liblog4c.la
application_2_LDADD = \
	liblog4c_examples.la \
	$(top_builddir)/src/log4c/liblog4c.la
application_3_LDADD = \
	liblog4c_userloc.la \
	liblog4c_examples.la \
	$(top_builddir)/src/log4c/liblog4c.la

