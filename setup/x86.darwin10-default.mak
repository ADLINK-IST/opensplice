# target context is set in platform specific config.mak

# Set name context of used tooling
CC		 = gcc -std=gnu99 -m32 #-v
CXX		 = g++ -m32 #-v
CSC      = gmcs

    # Binary used for linking
LD_SO            = $(CC)
    # Binary used for linking executables
LD_EXE           = $(CC)
LD_CXX           = $(CXX)
      # Tool used for creating soft/hard links.
LN               = ln -s
        # GNU yacc
YACC		 = bison
	# GNU lex
LEX		 = flex
	# GNU make
MAKE		 = make
	# Solaris native touch
TOUCH		 = touch
	# Archiving
AR               = /usr/bin/ar
AR_CMDS          = rv
	# preprocessor
MAKEDEPFLAGS     = -M
CPP		 = gcc #cpp #-v
GCPP     = g++ -E
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
JAVA_LDFLAGS	 = -L"$(JAVA_HOME)/lib"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/lib/ext"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/lib/im"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/lib/security"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/bundle/Libraries"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/bundle/Classes"
JAVA_INCLUDE	 = -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	 += -I"$(JAVA_HOME)/bundle/Headers"
JAVA_INCLUDE     += -I"/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers"
JAVA_INCLUDE     += -I"/System/Library/Frameworks/JavaVM.framework/Headers"
JAVA_INCLUDE     += -I"/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers"

	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = -dynamiclib #-fno-common

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O3 -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
#CFLAGS_STRICT	 = -Wall
CFLAGS_STRICT	 = -Wall -W -pedantic

# Set compiler options
CFLAGS		 = $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) $(MTCFLAGS)
CXXFLAGS	 = $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(MTCFLAGS)
CSFLAGS	     = -noconfig -nowarn:1701,1702 -warn:4 $(CSFLAGS_DEBUG) -optimize-


# For Linux, this test release version supports symbolic names in stead of IP addresses
CFLAGS      += -DDO_HOST_BY_NAME #-D__i386__

# Set CPP flags
CPPFLAGS	 = -DOSPL_ENV_$(SPECIAL) -D_GNU_SOURCE
ifeq (,$(wildcard /etc/gentoo-release))
CPPFLAGS	 += -D_XOPEN_SOURCE=500 -D_DARWIN_C_SOURCE
endif

# Set compiler options for multi threaded process
	# notify usage of posix threads
MTCFLAGS	 = -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS		 = -static-libgcc -L$(SPLICE_LIBRARY_PATH) -Wl,-flat_namespace

# Identify linker options for building shared libraries
SHLDFLAGS	 = -dynamiclib

# Set library context
#RP: -lrt cannot be found
#LDLIBS          = -lc -lm -lrt -lpthread
LDLIBS           = -lm #-lpthread #-lposix4

# Set library context for building shared libraries
SHLDLIBS	 =

# Set component specific libraries that are platform dependent
LDLIBS_CXX = -lstdc++
LDLIBS_NW =
LDLIBS_OS = -lpthread -ldl #-lrt
LDLIBS_CMS =
LDLIBS_JAVA = -ljvm -lverify -lhpi
LDLIBS_ODBC= -lodbc

#set platform specific pre- and postfixes for the names of libraries and executables
OBJ_POSTFIX = .o
SLIB_PREFIX = lib
SLIB_POSTFIX = .a
DLIB_PREFIX = lib
DLIB_POSTFIX = .dylib
EXEC_PREFIX =
EXEC_POSTFIX =
EXEC_LD_POSTFIX =
INLINESRC_POSTFIX = .inl

# Identify linker options for building shared C# libraries and or executables.
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

CSTARGET_LIB = -target:library
CSTARGET_MOD = -target:module
CSTARGET_EXEC = -target:exe

ifdef LKST_HOME
CPPFLAGS += -I$(LKST_HOME) -DHAVE_LKST
CFLAGS += -I$(LKST_HOME) -DHAVE_LKST
LDFLAGS += -L$(LKST_HOME)
LDLIBS += -llkst
endif
