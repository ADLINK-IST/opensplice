# target context is set in platform specific config.mak

# Set name context of used tooling
CC              = arm-angstrom-linux-gnueabi-gcc
CXX             = arm-angstrom-linux-gnueabi-g++
CSC             = csc
FILTER          = filter_gcc
LD_SO           = $(CC)
LD_EXE          = $(CXX)
YACC            = bison
LEX	            = flex
MAKE            = make
TOUCH           = touch
AR              = arm-angstrom-linux-gnueabi-ar
AR_CMDS         = rv
MAKEDEPFLAGS    = -M
CPP             = arm-angstrom-linux-gnueabi-cpp
GCPP            = arm-angstrom-linux-gnueabi-g++ -E
GCOV            = arm-angstrom-linux-gnueabi-gcov
JCC             = javac
JAR             = jar
JAVA            = java
SOAPCPP         = soapcpp2

JAVA_SRCPATH_SEP = :
JAVA_LDFLAGS	 = -L$(JAVA_HOME)/jre/lib/i386
JAVA_LDFLAGS	 += -L$(JAVA_HOME)/jre/lib/i386/client
JAVA_LDFLAGS	 += -L$(JAVA_HOME)/jre/lib/i386/native_threads
JAVA_INCLUDE	 = -I$(JAVA_HOME)/include
JAVA_INCLUDE	 += -I$(JAVA_HOME)/include/linux

# Identify compiler flags for building shared libraries
SHCFLAGS         = -fpic

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O4 -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
#CFLAGS_STRICT	 = -Wall
CFLAGS_STRICT	 = -Wall -W -pedantic

# Set compiler options for single threaded process
CFLAGS		 = -DVERSION="\\\"$(PACKAGE_VERSION)\\\"" $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT)
CXXFLAGS	 = -DVERSION=\"$(PACKAGE_VERSION)\" $(CFLAGS_OPT) $(CFLAGS_DEBUG)

# Set CPP flags
CPPFLAGS	 = -DOSPL_ENV_$(SPECIAL) -D_GNU_SOURCE
ifeq (,$(wildcard /etc/gentoo-release))
CPPFLAGS	 += -D_XOPEN_SOURCE=500
endif

# Set compiler options for multi threaded process
	# notify usage of posix threads
#MTCFLAGS	 = -D_POSIX_C_SOURCE=199506L
MTCFLAGS	+= -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

# Set linker options
LDFLAGS		 = -static-libgcc -L$(SPLICE_LIBRARY_PATH)

# Identify linker options for building shared libraries
SHLDFLAGS	 = -shared -fpic

# Set library context
#RP: -lrt cannot be found
#LDLIBS          = -lc -lm -lrt -lpthread
LDLIBS           = -lc -lm -lpthread# -lposix4

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
INLINESRC_POSTFIX = .i

# Identify linker options for building shared C# libraries and or executables.
CSTARGET_LIB = -target:library
CSTARGET_EXEC = -target:exe
CSLIB_PREFIX =
CSLIB_POSTFIX = .dll
CSEXEC_PREFIX =
CSEXEC_POSTFIX = .exe
