# This is for arm linux where we build on the target rather than crosscompile

PROC     = arm
OS       = linux
OS_REV   =

CC		 = gcc
CXX		 = g++
CSC		 = gmcs

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
	# native touch
TOUCH		 = touch
	# Tool used for creating soft/hard links.
LN               = ln
	# Archiving
AR               = /usr/bin/ar
AR_CMDS          = rv
	# preprocessor
MAKEDEPFLAGS     = -M
CPP		 = gcc
GCPP		 = g++ -E
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
JAVA_SRCPATH_SEP = :
JAVA_LDFLAGS	 = -L"$(JAVA_HOME)/jre/lib/i386"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/jre/lib/i386/client"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/jre/lib/i386/native_threads"
JAVA_INCLUDE	 = -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	 += -I"$(JAVA_HOME)/include/linux"

	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = -fpic

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O3 -fno-strict-aliasing -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
CFLAGS_STRICT	 = -Wall -W -Wno-long-long

# Set compiler options
CFLAGS		 = -pipe $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) $(MTCFLAGS)
CXXFLAGS	 = -pipe $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(MTCFLAGS)
CSFLAGS	     = -noconfig -nowarn:1701,1702 -warn:4 $(CSFLAGS_DEBUG) -optimize-

# For Linux, this version supports symbolic names instead of IP addresses
CFLAGS      += -DDO_HOST_BY_NAME
CPPFLAGS    += -std=c99

# Set CPP flags
CPPFLAGS	 = -DOSPL_ENV_$(SPECIAL) -D_GNU_SOURCE
ifeq (,$(wildcard /etc/gentoo-release))
CPPFLAGS	 += -D_XOPEN_SOURCE=500
endif

# For isocpp2 use c++11 compiler option
ifeq ($(GCC_SUPPORTS_CPLUSPLUS11),1)
ISOCPP2_CXX_FLAGS=-std=c++0x
endif

# Disable licensing because RLM not available on ARM
CPPFLAGS += -DOSPL_NO_LICENSING
CFLAGS += -DOSPL_NO_LICENSING

# Set compiler options for multi threaded process
	# notify usage of posix threads
MTCFLAGS	 = -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS		 = -static-libgcc -L$(SPLICE_LIBRARY_PATH)

# Identify linker options for building shared libraries
SHLDFLAGS	 = -shared -fpic

# Set library context
LDLIBS           = -lc -lm -lpthread -lrt

# Set library context for building shared libraries
SHLDLIBS	 = 

# Set component specific libraries that are platform dependent
LDLIBS_CXX = -lstdc++
LDLIBS_NW = 
LDLIBS_OS = -lrt -ldl -lm
LDLIBS_CMS = 
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
EXEC_LD_POSTFIX =
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

