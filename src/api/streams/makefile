SUBSYSTEMS	:=
SUBSYSTEMS += templates
SUBSYSTEMS += idl

ifeq ($(INCLUDE_API_DCPS_CCPP), yes)
ifeq ($(INCLUDE_API_STREAMS_CCPP), yes)
SUBSYSTEMS += ccpp
endif
endif

ifeq ($(INCLUDE_API_DCPS_SACPP), yes)
ifeq ($(INCLUDE_API_STREAMS_SACPP), yes)
SUBSYSTEMS += sacpp 
endif
endif

ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DCPS_ISOCPP),yes)
ifeq ($(INCLUDE_API_STREAMS_ISOCPP), yes)

#include ../../../setup/makefiles/license.mak

clean: clean_isocpp

link: build_isocpp

build_isocpp:
ifeq "$(findstring darwin10_clang, $(SPLICE_TARGET))" ""
	magic_make.pl --make --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS3) --src-co $(MPC_ARGS) isocpp/
else
ifeq "$(wildcard isocpp/Makefile)" ""
	-magic_make.pl --make --type $(MPC_CISH_TYPE_TO_GEN) $(MPC_CISH_ARGS3) --src-co $(MPC_ARGS) isocpp/
	@$(MAKE) -C isocpp clean
endif
	@$(MAKE) -C isocpp CXX='clang++ -DOSPL_DEFAULT_TO_CXX11'
endif

clean_isocpp:
	magic_make.pl --squeaky --clean --type $(MPC_CISH_TYPE_TO_GEN) --src-co $(MPC_ARGS) isocpp/

endif # INCLUDE_API_STREAMS_ISOCPP
endif # INCLUDE_API_DCPS_ISOCPP
endif # INCLUDE_CXX

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
