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

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

MPC_CISH_TYPE_TO_GEN ?= "make"
MPC_JISH_TYPE_TO_GEN ?= "ospljavamake"

ifeq (vxworks6,$(findstring vxworks6,$(SPLICE_TARGET))) # if VxWorks 6.x
   MPC_CISH_ARGS += "--noreldefs"
endif

FOO = $(shell expr "${SPLICE_TARGET}" : '[^-]*\(.*\)')
ifeq (, $(findstring release, $(FOO)))
HDE_SUBDIR_SUFFIX = $(FOO)
else
HDE_SUBDIR_SUFFIX =
endif
HDE_SUBDIR = $(shell expr "${SPLICE_TARGET}" : '\([^-]*\)')

TMP_OSPL_HOME = $(OSPL_HOME_NORMALIZED)/install/HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)

ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
GEN_OSPL_HOME = $(shell cygpath --mixed "$(TMP_OSPL_HOME)")
else
GEN_OSPL_HOME = $(TMP_OSPL_HOME)
endif

examplempc:
	magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/CSharp.mwc
	mwc.pl --type $(MPC_JISH_TYPE_TO_GEN) $(MPC_JISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/JustJavaScripts.mwc
	magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples.mwc
	magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples_simple.mwc
        ifneq "$(MPC_CISH_ARGS2)" ""
	   magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples.mwc
	   magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/examples/examples_simple.mwc
        endif
	magic_make.pl --nomake --noclean --check-mpc --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/custom_lib

demosmpc:
	mwc.pl --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --ospl-home "$(GEN_OSPL_HOME)" HDE/$(HDE_SUBDIR)$(HDE_SUBDIR_SUFFIX)/demos/iShapes

clean:
	@rm -rf HDE RTS VC
	@rm -f x86.linux*.xml
	@rm -f x86.win32*.xml
	@rm -f x86_64.win64*.xml

