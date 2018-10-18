.PHONY: all all_no_installer all_eval all_lgpl all_distros examplempc
all:
	./make_all

all_no_installer:
	./make_all -noinstaller

all_distros:
	./make_all -eval -lgpl

all_eval:
	./make_all -eval

all_lgpl:
	./make_all -lgpl

all_nosrc:
	./make_all -nosrc
	
all_ste:
	./make_all -ste

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

MPC_CISH_TYPE_TO_GEN ?= "make"
MPC_JISH_TYPE_TO_GEN ?= "ospljavamake"

ifeq (vxworks6,$(findstring vxworks6,$(SPLICE_TARGET))) # if VxWorks 6.x
   MPC_CISH_ARGS += "--noreldefs"
endif

ifeq ($(INCLUDE_API_DCPS_C99), yes)
MPC_CISH_ARGS += --features no_c99=0
endif

ifeq ($(GCC_SUPPORTS_CPLUSPLUS11), 1)
MPC_CISH_ARGS += --features isocpp2_cxx11=1
endif

ifeq ($(OSPL_USE_CXX11), yes)
MPC_CISH_ARGS += --features isocpp2_cxx11=1
endif

FOO = $(shell expr "${SPLICE_TARGET}" : '[^-]*\(.*\)')
ifeq (, $(findstring release, $(FOO)))
HDE_SUBDIR_SUFFIX = $(FOO)
else
HDE_SUBDIR_SUFFIX =
endif


HDE_SUBDIR = $(shell expr "${SPLICE_TARGET}" : '\([^-]*\)')

TMP_OSPL_HOME = $(OSPL_HOME_NORMALIZED)/install/HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)

ifneq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST)), $(findstring wince,$(SPLICE_HOST))))
GEN_OSPL_HOME = $(shell cygpath --mixed "$(TMP_OSPL_HOME)")
ifeq (, $(findstring release, $(FOO)))
ISHAPES_CONFIG = Debug
else
ISHAPES_CONFIG = Release
endif
else
GEN_OSPL_HOME = $(TMP_OSPL_HOME)
ifeq (, $(findstring release, $(FOO)))
ISHAPES_CONFIG = Debug Release
else
ISHAPES_CONFIG = Release Debug
endif
endif

examplempc:
	unset OSPL_HOME; magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/CSharp.mwc
	mwc.pl --type $(MPC_JISH_TYPE_TO_GEN) $(MPC_JISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/JustJavaScripts.mwc
	magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples.mwc
	magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples_simple.mwc
        ifneq "$(MPC_CISH_ARGS2)" ""
	   magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples.mwc
	   magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples_simple.mwc
        endif
	magic_make.pl --nomake --noclean --check-mpc --incdeb --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/custom_lib

demosmpc:
	mwc.pl --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/demos/iShapes -value_template "configurations = $(ISHAPES_CONFIG)"

clean:
	@rm -rf HDE RTS VC STE DRE
	@rm -f x86*.xml
	@rm -f x86_64*.xml

