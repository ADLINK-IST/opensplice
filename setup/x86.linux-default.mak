# target context is set in platform specific config.mak

# Set name context of used tooling
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
	# Solaris native touch
TOUCH		 = touch
	# Tool used for creating soft/hard links.
LN               = ln -s
	# Archiving
AR               = /usr/bin/ar
# Archiver when using link-time optimizations
AR_LTO           = gcc-ar
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
CFLAGS_OPT       = -O3 -DNDEBUG -g -fno-strict-aliasing
CFLAGS_DEBUG     = -g -D_TYPECHECK_
CFLAGS_STRICT	 = -Wall -W -Wno-long-long
CFLAGS_PERMISSIVE= -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
CFLAGS_LTO       = -flto
ifeq ($(GCC_WERROR_IS_SWITCH_SUPPORT),1)
    # Seperate from STRICT because this option won't work with -O0 on older compilers now it can be overruled
    CFLAGS_STRICT_UNINITIALIZED = -Werror=uninitialized
endif
ifeq ($(GCC_WCONVERSION_IS_SUPPORTED),1)
    CFLAGS_XSTRICT   = -Wconversion
endif

# Set compiler options
CFLAGS		 = -m32 -march=i686 -pipe -D_POSIX_C_SOURCE=200112L -DOSPL_LINUX $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT) $(CFLAGS_STRICT_UNINITIALIZED) $(MTCFLAGS)
CXXFLAGS	 = -m32 -march=i686 -pipe -DOSPL_LINUX $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT_UNINITIALIZED) $(MTCFLAGS)
CSFLAGS	     = -noconfig -nowarn:1701,1702 -warn:4 $(CSFLAGS_DEBUG) -optimize-

# For Linux, this test release version supports symbolic names in stead of IP addresses
CFLAGS      += -DDO_HOST_BY_NAME

# Set CPP flags
CPPFLAGS	 = -DOSPL_ENV_$(SPECIAL) -D_GNU_SOURCE
ifeq (,$(wildcard /etc/gentoo-release))
CPPFLAGS	 += -D_XOPEN_SOURCE=500
endif

# On older platforms (e.g., RHEL/Debian 4) the pthread.h doesn't include the
# newer NPTL signatures. While it is possible to include the NPTL pthread.h,
# this typically coincides with old (incomplete) GLIBC versions being installed
# on the platform (2.3ish), which have pthread_condattr_setclock(...) missing
# despite being available in the NPTL headers. In that case set the following
# define to disable the calls in the abstraction layer to the missing function.
ifneq (,$(wildcard /usr/include/nptl/pthread.h))
CPPFLAGS	 += -DOSPL_NO_POSIX_CLOCK_SELECTION
endif

# For isocpp2 use c++11 compiler option
ifeq ($(GCC_SUPPORTS_CPLUSPLUS11),1)
ISOCPP2_CXX_FLAGS=-std=c++0x
endif

# Set compiler options for multi threaded process
	# notify usage of posix threads
MTCFLAGS	 = -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS		 = -m32 -march=i686 -static-libgcc -L$(SPLICE_LIBRARY_PATH)
ifeq ($(INCLUDE_LD_NOASNEEDED),yes)
LDFLAGS += -Wl,-no-as-needed
endif

# Identify linker options for building shared libraries
SHLDFLAGS		= -shared -fpic -Wl,-Bsymbolic
SHLDCXXFLAGS	= -shared -fpic

# Set library context
LDLIBS = -lc -lm -ldl
LDLIBS += -lpthread

RT_LIB=librt.so
#If it returns just the name it was not found
GCC_PATH_LIBRT=$(shell gcc -m32 -print-file-name=$(RT_LIB))
ifneq "$(RT_LIB)" "$(GCC_PATH_LIBRT)"
LDLIBS += -lrt
endif

# Set library context for building shared libraries
SHLDLIBS	 =

# Set component specific libraries that are platform dependent
LDLIBS_CXX = -lstdc++
LDLIBS_NW =
LDLIBS_OS = -lrt -lpthread -ldl
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
