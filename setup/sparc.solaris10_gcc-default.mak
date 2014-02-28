# target context is set in platform specific config.mak

# Set name context of used tooling
CC                = gcc
CXX               = g++
CSC               = gmcs

# Binary used for filtering
FILTER           = filter_gcc

# Binary used for linking
LD_SO            = $(CC)
# Binary used for linking executables
LD_EXE           = $(CC)
LD_CXX           = $(CXX)
# GNU yacc
YACC             = bison
# GNU lex
LEX              = flex
# GNU make
MAKE             = make
# Solaris native touch
TOUCH            = touch
# Tool used for creating soft/hard links.
LN               = ln
# Archiving
AR               = ar
AR_CMDS          = rv
# preprocessor
MAKEDEPFLAGS     = -M
CPP              = cpp
GCPP             = g++ -E
# gcov
GCOV             = gcov

#Javac
JCC              = javac
JCFLAGS_JACORB   = -endorseddirs "$(JACORB_HOME)/lib/endorsed"
JACORB_INC       =

ifdef JAVA_COMPATJAR
ifneq (,$(JAVA_COMPATJAR))
JCFLAGS_COMPAT   = -source 1.6 -target 1.6 -bootclasspath "$(JAVA_COMPATJAR)"
endif
endif

#JAR
JAR              = jar

#JAVAH
JAVAH            = javah
JAVAH_FLAGS      = -force

#Java
JAVA             = java
JAVA_LDFLAGS     = -L$(JAVA_HOME)/jre/lib/sparc
JAVA_LDFLAGS     += -L$(JAVA_HOME)/jre/lib/sprc/client
JAVA_LDFLAGS     += -L$(JAVA_HOME)/jre/lib/sparc/native_threads
JAVA_INCLUDE     = -I$(JAVA_HOME)/include
JAVA_INCLUDE     += -I$(JAVA_HOME)/include/solaris

#soapcpp
SOAPCPP          = soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = -fPIC

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O0 -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
CFLAGS_STRICT    = -Wall -W -Wno-long-long -pedantic

# Set compiler options for single threaded process
CFLAGS           =  $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT)
CXXFLAGS         =  $(CFLAGS_OPT) $(CFLAGS_DEBUG)
CSFLAGS          = -noconfig -nowarn:1701,1702 -warn:4 $(CSFLAGS_DEBUG) -optimize-


# Set CPP flags
CPPFLAGS         = -mcpu=v9 -pipe -DOSPL_ENV_$(SPECIAL) -D__EXTENSIONS__

# Set compiler options for multi threaded process
# notify usage of posix threads
MTCFLAGS         += -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS          = -static-libgcc -L$(SPLICE_LIBRARY_PATH)

# Identify linker options for building shared libraries
SHLDFLAGS        = -shared -fPIC

# Set library context
LDLIBS           = -lc -lm -lpthread -lnsl -lsocket -lrt

# Set library context for building shared libraries
SHLDLIBS         =

# Set component specific libraries that are platform dependent
LDLIBS_CXX       = -lstdc++
LDLIBS_NW        =
LDLIBS_OS        = -lrt -lpthread -ldl
LDLIBS_CMS       =
LDLIBS_JAVA      = -ljvm -ljava -lverify -lhpi
LDLIBS_ODBC      = -lodbc
LDLIBS_ZLIB      = -lz
LDFLAGS_ZLIB     =
CINCS_ZLIB       =

#set platform specific pre- and postfixes for the names of libraries and executables
OBJ_POSTFIX         = .o
SLIB_PREFIX         = lib
SLIB_POSTFIX        = .a
DLIB_PREFIX         = lib
DLIB_POSTFIX        = .so
EXEC_PREFIX         =
EXEC_POSTFIX        =
EXEC_LD_POSTFIX     =
INLINESRC_POSTFIX   = .inl
CSLIB_PREFIX        =
CSLIB_POSTFIX       = .dll
CSMOD_PREFIX        =
CSMOD_POSTFIX       = .netmodule
CSEXEC_PREFIX       =
CSEXEC_POSTFIX      = .exe
CSDBG_PREFIX        =
CSEXEC_DBG_POSTFIX  = .exe.mdb
CSMOD_DBG_POSTFIX   = .netmodule.mdb
CSLIB_DBG_POSTFIX   = .dll.mdb
CS_LIBPATH_SEP      = ,

# Identify linker options for building shared C# libraries and or executables.
CSTARGET_LIB     = -target:library
CSTARGET_MOD     = -t:module
CSTARGET_EXEC    = -target:exe
