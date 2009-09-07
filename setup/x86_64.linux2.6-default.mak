# target context is set in platform specific config.mak

# Set name context of used tooling
CC		 = gcc
CXX		 = g++
CSC      = csc

    # Binary used for filtering
FILTER           = filter_gcc
    # Binary used for linking
LD_SO            = $(CC)
    # Binary used for linking executables
LD_EXE           = $(CXX)
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
CPP		 = cpp
GCPP		 = g++ -E
	# gcov
GCOV		 = gcov

	#Javac
JCC              = javac

	#JAR
JAR		 = jar

	#Java
JAVA		 = java
JAVA_SRCPATH_SEP = :
JAVA_LDFLAGS	 = -L$(JAVA_HOME)/jre/lib/amd64
JAVA_LDFLAGS	 += -L$(JAVA_HOME)/jre/lib/amd64/server
JAVA_LDFLAGS	 += -L$(JAVA_HOME)/jre/lib/amd64/native_threads
JAVA_INCLUDE	 = -I$(JAVA_HOME)/include
JAVA_INCLUDE	 += -I$(JAVA_HOME)/include/linux
	
	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         = -fPIC

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O4 -DNDEBUG
CFLAGS_DEBUG     = -g -D_TYPECHECK_
#CFLAGS_STRICT	 = -Wall
CFLAGS_STRICT	 = -Wall -W -pedantic

# Set compiler options for single threaded process
CFLAGS		 = -DVERSION="\\\"$(PACKAGE_VERSION)\\\"" $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT)
CXXFLAGS	 = -DVERSION=\"$(PACKAGE_VERSION)\" $(CFLAGS_OPT) $(CFLAGS_DEBUG)

# Set CPP flags
CPPFLAGS	 = -m64 -DOSPL_ENV_$(SPECIAL) -D_XOPEN_SOURCE=500

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

#set platform specific pre- and postfixes for the names of libraries and executables
OBJ_POSTFIX = .o
SLIB_PREFIX = lib
SLIB_POSTFIX = .a
DLIB_PREFIX = lib
DLIB_POSTFIX = .so
EXEC_PREFIX = 
EXEC_POSTFIX = 
INLINESRC_POSTFIX = .i

# Identify linker options for building shared C# libraries and or executables.
CSTARGET_LIB = -target:library
CSTARGET_EXEC = -target:exe
CSLIB_PREFIX =
CSLIB_POSTFIX = .dll
CSEXEC_PREFIX =
CSEXEC_POSTFIX = .exe