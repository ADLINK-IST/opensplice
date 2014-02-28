# target context in platform specific config.mak

# Set name context of used tooling
CC		 = cc
CXX		 = CC

#FILTER           = filter_gcc
# Binary used for linking
#LD_SO            = $(CC) -G
LD_SO            = $(CXX) -G
# Binary used for linking executables
LD_EXE           = $(CC)
LD_CXX           = $(CXX)

# Flags for static and dynamic linking
LD_FLAGS_STATIC = -Bstatic
LD_FLAGS_DYNAMIC = -Bdynamic

# GNU yacc
YACC		 = bison

# GNU lex
LEX		 = flex

# GNU make
MAKE		 = make

# Solaris native touch
TOUCH		 = touch

# Tool used for creating soft/hard links.
LN               = ln

# Archiving
AR               = $(CXX)
AR_CMDS          = -xar -mt -o
# preprocessor
MAKEDEPFLAGS     = -xM1
CPP		 = $(CC) $(CFLAGS)
GCPP		 = $(CXX) $(CFLAGS)

#Javac
JCC		= javac
JCFLAGS_JACORB   = -endorseddirs "$(JACORB_HOME)/lib/endorsed"
JACORB_INC       =

ifdef JAVA_COMPATJAR
ifneq (,$(JAVA_COMPATJAR))
JCFLAGS_COMPAT   = -source 1.6 -target 1.6 -bootclasspath "$(JAVA_COMPATJAR)"
endif
endif

#JAR
JAR		= jar

#JAVAH
JAVAH            = javah
JAVAH_FLAGS      = -force

#Java
JAVA		= java
JAVA_LDFLAGS	= -L"$(JAVA_HOME)/jre/lib/sparc"
JAVA_LDFLAGS	+= -L"$(JAVA_HOME)/jre/lib/sparc/client"
JAVA_LDFLAGS	+= -L"$(JAVA_HOME)/jre/lib/sparc/native_threads"
JAVA_INCLUDE	= -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	+= -I"$(JAVA_HOME)/include/solaris"

#soapcpp2
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = -G -xcode=pic32

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O -xarch=v8 -DNDEBUG
CFLAGS_DEBUG     = -g -DDEBUG -D_TYPECHECK_
CFLAGS_STRICT	 = -DSPARC -D_REENTRANT

# Set compiler options for single threaded process
CFLAGS		 = $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT)
CXXFLAGS	 = $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) -features=extensions


# Set CPP flags
#CPPFLAGS	 = -D__GNUC__   # defined because CPP on SPARC solaris
				# fails for netinet/in.h because this
				# symbol is not defined, only gcc defines this
				# It's not clean when other compilers are
				# used, but it should nevertheless function
CPPFLAGS	= -DOSPL_ENV_$(SPECIAL)

# Set compiler options for multi threaded process
	# notify usage of posix threads
MTCFLAGS	+= -mt -D_POSIX_PTHREAD_SEMANTICS

# Set linker options
LDFLAGS		 = -L$(SPLICE_LIBRARY_PATH)

# Identify linker options for building shared libraries
SHLDFLAGS	 = -G -xcode=pic32

# Set library context
LDLIBS		 = -lsocket -lnsl -ldl -lintl -lpthread -lrt -lm
# Set library context for building shared libraries
SHLDLIBS	 = -lsocket -lnsl -Bdynamic -ldl -lintl -lpthread -lrt

# Set component specific libraries that are platform dependent
LDLIBS_CXX =
LDLIBS_NW = -lsocket -lnsl
LDLIBS_OS = -lrt -lpthread -ldl
LDLIBS_CMS = -lxnet -lnsl -lsocket
LDLIBS_ZLIB = -lz
LDFLAGS_ZLIB =
CINCS_ZLIB =

#enable XPG5 plus extensions
CFLAGS		+= -D_XOPEN_SOURCE=500 -D__EXTENSIONS__

LDLIBS_JAVA = -ljvm -ljava -lverify -lhpi
LDLIBS_ODBC= -lodbc

#set platform specific pre- and postfixes for the names of libraries and executables
OBJ_POSTFIX = .o
SLIB_PREFIX = lib
SLIB_POSTFIX = .a
DLIB_PREFIX = lib
DLIB_POSTFIX = .so
EXEC_PREFIX =
EXEC_POSTFIX =
INLINESRC_POSTFIX = .inl

