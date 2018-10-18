INCLUDES = \
        -I$(top_srcdir)/src \
	$(EXPAT_CFLAGS)

noinst_LTLIBRARIES  = liblog4c_sd.la

liblog4c_sd_la_SOURCES = \
        error.h \
        defs.h \
        stack.h \
        stack.c \
        list.h \
        list.c \
        malloc.h \
        malloc.c \
        domnode.h \
        factory.h \
        factory.c \
        hash.h \
        hash.c \
        sprintf.h \
        sprintf.c \
        test.h \
        test.c \
        sd_xplatform.h \
        sd_xplatform.c \
	error.c

liblog4c_sd_la_LDFLAGS = $(EXPAT_LIBS)

if USE_EXPAT
liblog4c_sd_la_SOURCES += domnode-expat.c
else
liblog4c_sd_la_SOURCES += \
	domnode.c \
	domnode-xml.h \
	domnode-xml.c \
	domnode-xml-parser.h \
	domnode-xml-parser.c \
	domnode-xml-scanner.h \
	domnode-xml-scanner.c
endif


EXTRA_liblog4c_sd_la_SOURCES = \
	domnode-xml-parser.y \
	domnode-xml-scanner.l \
        sprintf.osf.c

#
# requires bison-1.35 as found on redhat-7.3
#
domnode-xml-parser.c domnode-xml-parser.h:
	bison $(srcdir)/domnode-xml-parser.y
	-mv domnode-xml-parser.[ch] $(srcdir) 2>/dev/null

#
# requires flex-2.5.27 from http://sourceforge.net/projects/lex/
#
domnode-xml-scanner.c domnode-xml-scanner.h:
	/home/legoater/work/sourceforge/flex-2.5.27/flex $(srcdir)/domnode-xml-scanner.l
	-mv domnode-xml-scanner.[ch] $(srcdir) 2>/dev/null

domnode-xml-parser.lo: domnode-xml-scanner.h
domnode-xml-scanner.lo: domnode-xml-parser.h
domnode-xml.lo: domnode-xml-scanner.c domnode-xml-parser.c
