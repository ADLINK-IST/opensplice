# target context in platform specific config.mak

# Set name context of used tooling
CC		 = cc -m64
CXX		 = CC -m64

# Binary used for linking
#LD_SO            = $(CC) -G
LD_SO            = $(CXX) -G
# Binary used for linking executables
LD_EXE           = $(CC)
LD_CXX           = $(CXX)

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
SHCFLAGS         = -G -Kpic

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O -DNDEBUG
CFLAGS_DEBUG     = -g -DDEBUG -D_TYPECHECK_
CFLAGS_STRICT	 = -D_REENTRANT

# Set compiler options (_XOPEN_SOURCE=500 to survive
# compiling the "new" cppgen, without it, PRIVATE => 0x20
# is picked up in sys/mman.h breaking the lexer, and
# _XOPEN_SOURCE=500 happens to work, unlike many other
# settings ...)
CFLAGS		 = $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) $(MTCFLAGS)
CXXFLAGS	 = $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) -features=extensions -D_XOPEN_SOURCE=500 $(MTCFLAGS)

# Set CPP flags
#CPPFLAGS	 = -D__GNUC__   # defined because CPP on SPARC solaris
				# fails for netinet/in.h because this
				# symbol is not defined, only gcc defines this
				# It's not clean when other compilers are
				# used, but it should nevertheless function
CPPFLAGS	= -DOSPL_ENV_$(SPECIAL)

# Set compiler options for multi threaded process
	# notify usage of posix threads
MTCFLAGS	= -mt -D_POSIX_PTHREAD_SEMANTICS

# Set linker options
LDFLAGS		 = -L$(SPLICE_LIBRARY_PATH)

# Identify linker options for building shared libraries
SHLDFLAGS	 = -G -Kpic

# Set library context
LDLIBS		 = -lsocket -lnsl -ldl -lintl -lpthread -lrt -lm
# Set library context for building shared libraries
SHLDLIBS	 = -lsocket -lnsl -Bdynamic -ldl -lintl -lpthread -lrt

# Set component specific libraries that are platform dependent
LDLIBS_CXX =
LDLIBS_NW = -lsocket -lnsl
LDLIBS_OS = -lrt -lpthread -ldl
LDLIBS_CMS = -lxnet -lnsl -lsocket

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

ifdef LKST_HOME
CPPFLAGS += -I$(LKST_HOME) -DHAVE_LKST
CFLAGS += -I$(LKST_HOME) -DHAVE_LKST
LDFLAGS += -L$(LKST_HOME)
LDLIBS += -llkst
endif

ifdef VALGRIND_HOME
CPPFLAGS += -I$(VALGRIND_HOME) -DHAVE_VALGRIND
CFLAGS += -I$(VALGRIND_HOME) -DHAVE_VALGRIND
endif
