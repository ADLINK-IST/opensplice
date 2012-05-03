# target context is set in platform specific config.mak

# Set name context of used tooling
CC		 = gcc
CXX		 = g++
CSC		 = gmcs

    # Binary used for filtering
FILTER           =
    # Binary used for linking
LD_SO            = $(CC)
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
AR               = /usr/bin/ar
AR_CMDS          = rv
	# preprocessor
MAKEDEPFLAGS     = -M
CPP		 = cpp
GCPP		 = g++ -E
	# gcov
GCOV		 = gcov

	#Javac
JCC              = javac

	#JAR
JAR		 = jar

#JAVAH
JAVAH            = javah
JAVAH_FLAGS      = -force

	#Java
JAVA		 = java
JAVA_SRCPATH_SEP = :
JAVA_LDFLAGS	 = -L"$(JAVA_HOME)/jre/lib/amd64"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/jre/lib/amd64/server"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/jre/lib/amd64/native_threads"
JAVA_INCLUDE	 = -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	 += -I"$(JAVA_HOME)/include/linux"
	
	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = -fPIC

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O4 -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
#CFLAGS_STRICT	 = -Wall
CFLAGS_STRICT	 = -Wall -W -pedantic -Wno-long-long -Wno-variadic-macros

# Set compiler options for single threaded process
CFLAGS		 = -D_GNU_SOURCE -DVERSION=\"$(PACKAGE_VERSION)\" $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT)
CXXFLAGS	 = -D_GNU_SOURCE -DVERSION=\"$(PACKAGE_VERSION)\" $(CFLAGS_OPT) $(CFLAGS_DEBUG)

# Set CPP flags
CPPFLAGS	 = -m64 -pipe -DOSPL_ENV_$(SPECIAL) -D_XOPEN_SOURCE=500

# Set compiler options for multi threaded process
	# notify usage of posix threads
#MTCFLAGS	 = -D_POSIX_C_SOURCE=199506L
MTCFLAGS	+= -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS		 = -static-libgcc -L$(SPLICE_LIBRARY_PATH)

# Identify linker options for building shared libraries
SHLDFLAGS	 = -shared -fPIC

# Set library context
#RP: -lrt cannot be found
#LDLIBS		 = -lc -lm -lrt -lpthread
LDLIBS		 = -lc -lm -lpthread# -lposix4

# Set library context for building shared libraries
SHLDLIBS	 = 

# Set component specific libraries that are platform dependent
LDLIBS_CXX = -lstdc++
LDLIBS_NW = 
LDLIBS_OS = -lrt -lpthread -ldl
LDLIBS_CMS = 
LDLIBS_JAVA = -ljvm -ljava -lverify -lhpi
LDLIBS_ODBC= -lodbc
LDLIBS_ZLIB = -lz
LDFLAGS_ZLIB =
CINCS_ZLIB =

#set platform specific pre- and postfixes for the names of libraries and executables
OBJ_POSTFIX = .o
SLIB_PREFIX = lib
SLIB_POSTFIX = .a
DLIB_PREFIX = lib
DLIB_POSTFIX = .so
EXEC_PREFIX = 
EXEC_POSTFIX = 
INLINESRC_POSTFIX = .i
CSLIB_PREFIX =
CSLIB_POSTFIX = .dll
CSMOD_PREFIX =
CSMOD_POSTFIX = .netmodule
CSEXEC_PREFIX =
CSEXEC_POSTFIX = .exe
CSDBG_PREFIX =
CSEXEC_DBG_POSTFIX = .exe.mdb
CSMOD_DBG_POSTFIX = .netmodule.mdb
CSLIB_DBG_POSTFIX = .dll.mdb
CS_LIBPATH_SEP = ,

# Identify linker options for building shared C# libraries and or executables.
CSTARGET_LIB = -target:library
CSTARGET_MOD = -t:module
CSTARGET_EXEC = -target:exe
