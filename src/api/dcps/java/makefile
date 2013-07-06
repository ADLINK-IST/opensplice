SUBSYSTEMS :=

ifeq ($(INCLUDE_API_DCPS_SAJ),yes)
SUBSYSTEMS += saj
INCLUDE_COMMON=yes
endif # INCLUDE_API_DCPS_SAJ

ifeq ($(INCLUDE_API_DCPS_CJ),yes)
SUBSYSTEMS += cj
INCLUDE_COMMON=yes
endif # INCLUDE_API_DCPS_CJ

ifeq ($(INCLUDE_COMMON),yes)
SUBSYSTEMS += common
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
