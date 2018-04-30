# target context is set in platform specific config.mak

# LTO doesn't work with static libraries (apparently), but everything works
# fine with nothing but dynamic libraries.
#DYNAMIC_LIB_ONLY := true

IS_DARWIN=yes

# Set name context of used tooling
#CC		 = gcc -std=gnu99 -m64 #-v
#CXX		 = g++ -m64 #-v
ifeq "$(origin CC)" "default"
CC = clang
CXX = clang++
endif
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
AR_CMDS          = r
	# preprocessor
MAKEDEPFLAGS     = -M
CPP		 = clang #cpp #-v
GCPP     = clang++ -E
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
JAVA_LDFLAGS	 = -L"$(JAVA_HOME)/jre/lib"
JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/jre/lib/server"
#JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/lib/ext"
#JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/lib/im"
#JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/lib/security"
#JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/bundle/Libraries"
#JAVA_LDFLAGS	 += -L"$(JAVA_HOME)/bundle/Classes"
JAVA_INCLUDE	 = -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	 += -I"$(JAVA_HOME)/include/darwin"
#JAVA_INCLUDE	 += -I"$(JAVA_HOME)/bundle/Headers"
#JAVA_INCLUDE     += -I"/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers"
#JAVA_INCLUDE     += -I"/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers"

	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = #-dynamiclib #-fno-common

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O4 -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
CFLAGS_STRICT	 = -Wall -W -Wno-deprecated-declarations
CFLAGS_XSTRICT   = -Wconversion
CFLAGS_PERMISSIVE= -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-sign-compare

# Set compiler option
CFLAGS		 = $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) $(MTCFLAGS)
CXXFLAGS	 = -std=c++11 -DOSPL_USE_CXX11 $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(MTCFLAGS) -Wno-deprecated-register
CSFLAGS	     = -noconfig -nowarn:1701,1702 -warn:4 $(CSFLAGS_DEBUG) -optimize-

# For Linux, this test release version supports symbolic names in stead of IP addresses
CFLAGS      += -DDO_HOST_BY_NAME #-D__i386__
CFLAGS      += -DFIX_FAULTY_ATTRIBUTE_NONNULL

# Set CPP flags
CPPFLAGS	 = -DOSPL_ENV_$(SPECIAL) -D_GNU_SOURCE
ifeq (,$(wildcard /etc/gentoo-release))
CPPFLAGS	 += -D_XOPEN_SOURCE=500 -D_DARWIN_C_SOURCE
endif

# Set compiler options for multi threaded process
	# notify usage of posix threads
MTCFLAGS	 = -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS		 = -L$(SPLICE_LIBRARY_PATH) $(CFLAGS) # -Wl,-flat_namespace

# Identify linker options for building shared libraries
SHLDFLAGS	 = -dynamiclib -undefined error

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
LDLIBS_JAVA = -ljvm #-lverify -lhpi
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

ifdef VALGRIND_HOME
CPPFLAGS += -I$(VALGRIND_HOME) -DHAVE_VALGRIND
CFLAGS += -I$(VALGRIND_HOME) -DHAVE_VALGRIND
endif
