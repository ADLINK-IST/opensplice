SUBSYSTEMS	:=

ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DCPS_CCPP),yes)
SUBSYSTEMS 	+= ccpp
endif # INCLUDE_API_DCPS_CCPP
ifeq ($(INCLUDE_API_DCPS_SACPP),yes)
SUBSYSTEMS 	+= sacpp
endif # INCLUDE_API_DCPS_SACPP
endif # INCLUDE_CXX

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
