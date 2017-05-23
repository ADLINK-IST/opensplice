SUBSYSTEMS :=

ifeq ($(INCLUDE_API_DCPS_SAJ5),yes)
SUBSYSTEMS += saj
INCLUDE_COMMON=yes
endif # INCLUDE_API_DCPS_SAJ5

ifeq ($(INCLUDE_API_DCPS_CJ5),yes)
SUBSYSTEMS += cj
INCLUDE_COMMON=yes
endif # INCLUDE_API_DCPS_CJ5

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
