include  $(OSPL_HOME)/setup/vcpp.mak

# target context in platform specific config.mak

WINCMD = $(OSPL_HOME)/bin/ospl_wincmd

# Set name context of used tooling
CC  = $(WINCMD) cl
CXX = $(WINCMD) cl
CSC = $(WINCMD) Csc

# Binary used for filtering: in the future must be empty
FILTER =

# Binary used for linking
LD_SO            = $(WINCMD) link
	# Binary used for linking executables
LD_EXE           = $(WINCMD) link
LD_CXX           = $(WINCMD) link
	# GNU yacc
YACC		 = bison
	# GNU lex
LEX		 = flex
	# GNU make
MAKE		 = make
	# Cygwin's touch
TOUCH		 = touch
	# Tool used for creating soft/hard links.
LN               = ospl_wincmd ospl_winln
	# Archiving
AR               = sh $(OSPL_HOME)/bin/ospl_winlib
AR_CMDS          =
	# preprocessor:
MAKEDEPFLAGS     = -E
        #    keep using GNU cpp, since MS stuff unable to generate gmake deps
CPP		 = $(CC)
GCPP  	 = $(CXX)

	# gcov
GCOV		 = gcov

	#Javac
JCC              = $(WINCMD) javac
JCFLAGS_JACORB   = -endorseddirs "$(JACORB_HOME)/lib/endorsed"
JACORB_INC       =

ifdef JAVA_COMPATJAR
ifneq (,$(JAVA_COMPATJAR))
JCFLAGS_COMPAT   = -source 1.6 -target 1.6 -bootclasspath "$(JAVA_COMPATJAR)"
endif
endif

	#JAR
JAR		 = jar

	#Java
JAVA		 = $(WINCMD) java
JAVA_LDFLAGS	= -L"$(JAVA_HOME)/lib"
JAVA_INCLUDE	= -I"$(JAVA_HOME)/include"
JAVA_INCLUDE	+= -I"$(JAVA_HOME)/include/win32"
JAVAH		    = $(WINCMD) javah
JAVAH_FLAGS	    = -force

	#soapcpp
SOAPCPP		= soapcpp2

# Identify compiler flags for building shared libraries
SHCFLAGS         =

# Values of compiler flags can be overruled
CFLAGS_OPT       = -O2 -DNDEBUG
CFLAGS_DEBUG     = -Z7 -Od -D_TYPECHECK_
CFLAGS_STRICT	 = -W3

ifneq (,$(or $(findstring 15,$(VS_VER)),$(findstring 16,$(VS_VER))))
# VS2008 & VS 2010
   VS_INCLUDE =  -I"$(VS_HOME)/VC/include"
   VS_INCLUDE += -I"$(WINDOWSSDKDIR)/Include"
 
   VS_LIB_FLAGS  = -L"$(VS_HOME)/VC/lib/amd64"
   VS_LIB_FLAGS += -L"$(WINDOWSSDKDIR)/lib/x64"
else
# VS2012 onwards
   VS_INCLUDE =  -I"$(VS_HOME)/VC/include"
   VS_INCLUDE += -I"$(WINDOWSSDKDIR)/Include/shared" -I"$(WINDOWSSDKDIR)/Include/um"

   VS_LIB_FLAGS += -L"$(VS_HOME)/VC/lib/amd64"
   VS_LIB_FLAGS += -L"$(WINDOWSSDKDIR)/lib/win8/um/x64"

   VS_LIB_FLAGS += -L"$(WINDOWSSDKDIR)/Lib/winv6.3/um/x64"

   VS_LIB_FLAGS += -L"$(WINDOWSSDKDIR)/lib/x64"
endif

# Set compiler options for single threaded process
#    The definition _CRT_SECURE_NO_DEPRECATE is to suppress warnings, remove when not using deprecated functions
CFLAGS	= -TC $(VS_INCLUDE) $(CFLAGS_OPT) $(CFLAGS_DEBUG) $(CFLAGS_STRICT)
CXXFLAGS	= -EHsc -TP $(VS_INCLUDE) $(CFLAGS_OPT) $(CFLAGS_DEBUG)
CSFLAGS	= -noconfig -nowarn:1701,1702 -errorreport:prompt -warn:4 $(CSFLAGS_DEBUG) -optimize-

# Set CPP flags
CPPFLAGS	 = -nologo -DOSPL_ENV_$(SPECIAL) -DWIN32 -DWIN64 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE $(VS_INCLUDE)

# Set compiler options for multi threaded process
# notify usage of posix threads
MTCFLAGS	=

# Set linker options
LDFLAGS = -nologo -incremental:no -machine:X64 -subsystem:console -L$(SPLICE_LIBRARY_PATH) $(VS_LIB_FLAGS)

# Identify linker options for building shared libraries
SHLDFLAGS = -dll -machine:X64 -incremental:no

# Identify linker options for building shared C# libraries and or executables.
CSTARGET_LIB = -t:library
CSTARGET_MOD = -t:module
CSTARGET_EXEC = -t:exe

# Set library context
LDLIBS		 =

# Set library context for building shared libraries
SHLDLIBS	 =

# Set component specific libraries that are platform dependent
LDLIBS_CXX =
LDLIBS_NW = -lws2_32
LDLIBS_OS = -lkernel32 -lAdvapi32
LDLIBS_CMS = -lws2_32
LDLIBS_JAVA = -ljvm
LDLIBS_ODBC= -lodbc32

LDLIBS_ZLIB = -lzlib
LDFLAGS_ZLIB = "-L$(ZLIB_HOME)"
CINCS_ZLIB = "-I$(ZLIB_HOME)"

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
INLINESRC_POSTFIX = .inl
CSLIB_PREFIX =
CSLIB_POSTFIX = .dll
CSMOD_PREFIX =
CSMOD_POSTFIX = .netmodule
CSEXEC_PREFIX =
CSEXEC_POSTFIX = .exe
CSDBG_PREFIX =
CSEXEC_DBG_POSTFIX = .pdb
CSMOD_DBG_POSTFIX = .pdb
CSLIB_DBG_POSTFIX = .pdb
CS_LIBPATH_SEP = ;
