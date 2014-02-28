# target context is set in platform specific config.mak

# Set name context of used tooling
CC		 = opencc
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
GCPP		 = openCC -E
	# gcov
GCOV		 = gcov

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
JAR		 = jar

#JAVAH
JAVAH            = javah
JAVAH_FLAGS      = -force

	#Java
JAVA		 = java
JAVA_LDFLAGS	 = -L"$(JAVA_HOME)/jre/lib/amd64"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/jre/lib/amd64/server"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/jre/lib/amd64/native_threads"
JAVA_INCLUDE	 = -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	 += -I"$(JAVA_HOME)/include/linux"

	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = -fPIC
# -ipa should go in SHCFLAGS but ddsi breaks opencc with it

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O3 -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
#CFLAGS_STRICT	 = -Wall
CFLAGS_STRICT	 = -Wall -W -pedantic -Wno-long-long
ifeq ($(GCC_WERROR_IS_SWITCH_SUPPORT),1)
    # Seperate from STRICT because this option won't work with -O0 on older compilers now it can be overruled
    CFLAGS_STRICT_UNINITIALIZED = -Werror=uninitialized
endif

# Set compiler options for single threaded process
CFLAGS		 = -D_GNU_SOURCE -DOSPL_LINUX $(CFLAGS_OPT_C) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) $(CFLAGS_STRICT_UNINITIALIZED)
CXXFLAGS	 = -D_GNU_SOURCE -DOSPL_LINUX $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT_UNINITIALIZED)

# For Linux, this test release version supports symbolic names in stead of IP addresses
CFLAGS      += -m64 -pipe -fPIC -DDO_HOST_BY_NAME

# Set CPP flags
CPPFLAGS	 = -DOSPL_ENV_$(SPECIAL) -D_XOPEN_SOURCE=500

# Set compiler options for multi threaded process
	# notify usage of posix threads
#MTCFLAGS	 = -D_POSIX_C_SOURCE=199506L
MTCFLAGS	+= -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS		 = -static-libgcc -L$(SPLICE_LIBRARY_PATH)
ifeq ($(INCLUDE_LD_NOASNEEDED),yes)
LDFLAGS += -Wl,-no-as-needed
endif

# Identify linker options for building shared libraries
SHLDFLAGS	 = -shared -fPIC
# -ipa broken on current open64

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
LDLIBS_JAVA = -ljvm -ljava -lverify
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
INLINESRC_POSTFIX = .inl
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
