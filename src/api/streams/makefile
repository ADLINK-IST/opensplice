SUBSYSTEMS :=
SUBSYSTEMS += templates
SUBSYSTEMS += idl

PSUBSYSTEMS :=
ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DCPS_CCPP), yes)
ifeq ($(INCLUDE_API_STREAMS_CCPP), yes)
PSUBSYSTEMS += ccpp
endif
endif

ifeq ($(INCLUDE_API_DCPS_SACPP), yes)
ifeq ($(INCLUDE_API_STREAMS_SACPP), yes)
PSUBSYSTEMS += sacpp
endif
endif

ifeq ($(INCLUDE_API_DCPS_ISOCPP),yes)
ifeq ($(INCLUDE_API_STREAMS_ISOCPP), yes)
PSUBSYSTEMS += isocpp
endif # INCLUDE_API_STREAMS_ISOCPP
endif # INCLUDE_API_DCPS_ISOCPP
endif # INCLUDE_CXX

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
