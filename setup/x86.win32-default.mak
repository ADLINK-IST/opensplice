# target context in platform specific config.mak

WINCMD = $(OSPL_HOME)/bin/ospl_wincmd
WINCSC = sh $(OSPL_HOME)/bin/ospl_wincsc

# Set name context of used tooling
CC  = $(WINCMD) cl
CXX = $(WINCMD) cl
CSC = $(WINCSC) Csc
        
# Binary used for filtering: in the future must be empty
FILTER = 
        
# Binary used for linking
LD_SO            = $(WINCMD) link
	# Binary used for linking executables
LD_EXE           = $(WINCMD) link
	# GNU yacc
YACC		 = bison
	# GNU lex
LEX		 = flex
	# GNU make
MAKE		 = make
	# Solaris native touch
TOUCH		 = touch
	# Archiving
AR               = sh $(OSPL_HOME)/bin/ospl_winlib
AR_CMDS          = 
	# preprocessor: 
MAKEDEPFLAGS     = -M
        #    keep using GNU cpp, since MS stuff unable to generate gmake deps
CPP		 = cpp
GCPP  	 = ospl_wincpp

	# gcov
GCOV		 = gcov

	#Javac
JCC              = javac

	#JAR
JAR		 = jar

	#Java
JAVA		 = java
JAVA_SRCPATH_SEP = ;
JAVA_LDFLAGS	= -L"$(JAVA_HOME)/lib"
JAVA_INCLUDE	= -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	+= -I"$(JAVA_HOME)/include/win32"

	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         =

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O2 -DNDEBUG
CFLAGS_DEBUG     = -Z7 -Od -D_TYPECHECK_
CFLAGS_STRICT	 = -W3

ifeq ("$(VS_VER)","14")
   VS_INCLUDE =  -I"$(VS_HOME)/VC/INCLUDE" 
   VS_INCLUDE += -I"$(VS_HOME)/VC/PlatformSDK/include" 
   VS_INCLUDE += -I"$(VS_HOME)/VC/atlmfc/include"

   VS_LIB_FLAGS  = -L"$(VS_HOME)/VC/lib" 
   VS_LIB_FLAGS += -L"$(VS_HOME)/VC/PlatformSDK/lib"
endif

ifeq ("$(VS_VER)","15")
   VS_INCLUDE =  -I"$(VS_HOME)/VC/include" 
   VS_INCLUDE += -I"$(VS_HOME)/../Microsoft SDKs/Windows/v6.0A/Include" 

   VS_LIB_FLAGS  = -L"$(VS_HOME)/VC/lib" 
   VS_LIB_FLAGS += -L"$(VS_HOME)/../Microsoft SDKs/Windows/v6.0A/lib"
endif

# Set compiler options for single threaded process
#    The definition _CRT_SECURE_NO_DEPRECATE is to suppress warnings, remove when not using deprecated functions
CFLAGS	= -nologo -TC $(VS_INCLUDE) $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT)
CXXFLAGS	= -EHsc -nologo -TP $(VS_INCLUDE) $(CFLAGS_OPT) $(CFLAGS_DEBUG)
CSFLAGS	= -noconfig -nowarn:1701,1702 -errorreport:prompt -warn:4 $(CSFLAGS_DEBUG) -optimize-

# Set CPP flags
CPPFLAGS	 = -DOSPL_ENV_$(SPECIAL) -DWIN32 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_USE_32BIT_TIME_T -DVERSION="\"$(PACKAGE_VERSION)\""

# Set compiler options for multi threaded process
# notify usage of posix threads
MTCFLAGS	=

# Set linker options
LDFLAGS = -incremental:no -machine:IX86 -subsystem:console -L$(SPLICE_LIBRARY_PATH) $(VS_LIB_FLAGS)

# Identify linker options for building shared libraries
SHLDFLAGS = -dll -machine:IX86 -incremental:no

# Identify linker options for building shared C# libraries and or executables.
CSTARGET_LIB = -t:library
CSTARGET_EXEC = -target:exe

# Set library context
LDLIBS		 =

# Set library context for building shared libraries
SHLDLIBS	 = 

# Set component specific libraries that are platform dependent
LDLIBS_CXX =
LDLIBS_NW = -lws2_32
LDLIBS_OS = -lkernel32
LDLIBS_CMS = -lws2_32
LDLIBS_JAVA = -ljvm
LDLIBS_ODBC= -lodbc32

#set platform specific pre- and postfixes for the names of libraries and executables
OBJ_POSTFIX = .obj
SLIB_PREFIX =
SLIB_POSTFIX = .lib
DLIB_PREFIX =
DLIB_POSTFIX = .dll
JLIB_PREFIX =
JLIB_POSTFIX = .dll
EXEC_PREFIX =
EXEC_POSTFIX = .exe
INLINESRC_POSTFIX = .i
CSLIB_PREFIX =
CSLIB_POSTFIX = .dll
CSEXEC_PREFIX =
CSEXEC_POSTFIX = .exe
