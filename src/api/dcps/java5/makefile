SUBSYSTEMS :=

ifeq ($(INCLUDE_API_DCPS_SAJ5),yes)
SUBSYSTEMS += saj
ifneq ($(INCLUDE_API_DCPS_SAJ),yes)
INCLUDE_COMMON=yes
endif
endif # INCLUDE_API_DCPS_SAJ5

ifeq ($(INCLUDE_COMMON),yes)
SUBSYSTEMS += ../java/common
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
