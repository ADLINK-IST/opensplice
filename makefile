#
# Set subsystems to be processed
#
SUBSYSTEMS	:= setup
SUBSYSTEMS	+= src

ifneq (,$(OSPL_OUTER_HOME))
#Keep the extract step as the last thing to be done, as it requires
#other libs are already built.
SUBSYSTEMS	+= $(OSPL_OUTER_HOME)/extract
endif

ifeq (,$(TAO_ROOT))
MPC_CISH_ARGS += --features no_tao=1
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

clean: clean_demos
	@rm -rf $(OSPL_HOME)/lib/$(SPLICE_TARGET)
	@rm -rf $(OSPL_HOME)/exec/$(SPLICE_TARGET)

.PHONY: install
install: link
	@$(MAKE) -C install
#	(if [ "$$SPLICE_HOST" != "$$SPLICE_TARGET" ]; then SPLICE_PLATFORM=$$SPLICE_HOST; export SPLICE_PLATFORM; . ./setup; make; fi)
#	(cd install; make)

MPC_CISH_TYPE_TO_GEN ?= "make"
MPC_JISH_TYPE_TO_GEN ?= "ospljavamake"

.PHONY: get_target_os_header_dir examplempc
examplempc:
	-@cd ./examples && ls Makefile* | xargs -n 1 -I {} make -f {} realclean
	-@cd ./examples && ls *.sln | xargs -n 1 -I {} devenv.com {} /Clean
	mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) examples/CSharp.mwc
	mwc.pl --src-co --type $(MPC_JISH_TYPE_TO_GEN) $(MPC_JISH_ARGS) examples/JustJavaScripts.mwc
	mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) examples/examples.mwc
	mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) examples/examples_simple.mwc
        ifneq "$(MPC_CISH_ARGS2)" ""
	   mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2)  examples/examples.mwc
	   mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2)  examples/examples_simple.mwc
        endif

build_demos:
	magic_make.pl --make --type $(MPC_CISH_TYPE_TO_GEN) --src-co $(MPC_ARGS) demos/iShapes

doxygen:
	mkdir -p ./internal_docs/isocpp
	doxygen ./etc/doxygen_isocpp_internal.cfg

clean_demos:
	magic_make.pl --squeaky --clean --type $(MPC_CISH_TYPE_TO_GEN) --src-co $(MPC_ARGS) demos/iShapes

get_target_os_header_dir:
	-@echo $(OS)$(OS_REV)
