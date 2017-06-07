#
# Set subsystems to be processed
#
SUBSYSTEMS	:= setup
SUBSYSTEMS	+= external
SUBSYSTEMS	+= src
ifeq ($(OSPL_DOCS),update)
SUBSYSTEMS	+= docs
endif


ifneq (,$(OSPL_OUTER_HOME))
#Keep the extract step as the last thing to be done, as it requires
#other libs are already built.
SUBSYSTEMS	+= $(OSPL_OUTER_HOME)/extract
endif

ifeq ($(OSPL_PYINSTALL_IS_ON), yes)
PYI	:= $(PYINSTALLER_COMMAND)
SCRIPT_BASE_DIR	:=examples/dcps/PerformanceScripts
SCRIPT_BLD_O	:= --workpath $(SCRIPT_BASE_DIR)/bld --distpath $(SCRIPT_BASE_DIR)/dist
endif

ifeq (,$(TAO_ROOT))
MPC_CISH_ARGS += --features no_tao=1
endif

ifeq ($(GCC_SUPPORTS_CPLUSPLUS11), 1)
MPC_CISH_ARGS += --features isocpp2_cxx11=1
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

clean: clean_demos clean_scripts
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
	unset OSPL_HOME_NORMALIZED; mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) examples/CSharp.mwc
	mwc.pl --src-co --type $(MPC_JISH_TYPE_TO_GEN) $(MPC_JISH_ARGS) examples/JustJavaScripts.mwc
	mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) examples/examples.mwc
	mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) examples/examples_simple.mwc
ifneq "$(MPC_CISH_ARGS2)" ""
	mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2)  examples/examples.mwc
	mwc.pl --src-co --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS2)  examples/examples_simple.mwc
endif

build_demos:
	magic_make.pl --make --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS) --src-co $(MPC_ARGS) demos/iShapes

ifeq ($(OSPL_PYINSTALL_IS_ON), yes)
.PHONY: build_example_script
build_scripts:

	$(PYI) --onedir --onefile $(SCRIPT_BLD_O) $(SCRIPT_BASE_DIR)/Roundtrip.py
	$(PYI) --onedir --onefile $(SCRIPT_BLD_O) $(SCRIPT_BASE_DIR)/Throughput.py

endif

clean_scripts:
ifeq ($(OSPL_PYINSTALL_IS_ON), yes)
	@rm -rf $(SCRIPT_BASE_DIR)/bld/
	@rm -rf $(SCRIPT_BASE_DIR)/dist/
endif

doxygen:
	mkdir -p ./ospl_docs/docs/c99
	doxygen ./etc/doxygen_c99_api.cfg
	mkdir -p ./ospl_docs/docs/face/java
	doxygen ./etc/doxygen_java_face_api.cfg
	mkdir -p ./ospl_docs/docs/face/cpp
	doxygen ./etc/doxygen_isocpp2_face_api.cfg
	mkdir -p ./ospl_docs/docs/java5
	doxygen ./etc/doxygen_java5_api.cfg
	mkdir -p ./internal_docs/isocpp
	doxygen ./etc/doxygen_isocpp_internal.cfg
	python ./src/api/dcps/isocpp2/predoxygen.py -i ./src/api/dcps/isocpp2/include/dds -o ./src/api/dcps/isocpp2/doxy
	mkdir -p ./internal_docs/isocpp2
	doxygen ./etc/doxygen_isocpp2_internal.cfg
	mkdir -p ./ospl_docs/docs/cs_api
	doxygen ./etc/doxygen_cs_api.cfg

clean_demos:
	magic_make.pl --squeaky --clean --type $(MPC_CISH_TYPE_TO_GEN) --src-co $(MPC_ARGS) demos/iShapes

get_target_os_header_dir:
	-@echo $(OS)$(OS_REV_SUFFIX)

get_cc:
	-@echo $(CC)
