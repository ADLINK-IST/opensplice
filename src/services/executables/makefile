#
# Set subsystems to be processed
#
SUBSYSTEMS := spliced

ifeq ($(INCLUDE_SERVICES_DDSI),yes)
SUBSYSTEMS += ddsi2
endif

ifeq ($(INCLUDE_SERVICES_DDSI2E),yes)
SUBSYSTEMS += ddsi2e
endif

ifeq ($(INCLUDE_SERVICES_DURABILITY),yes)
SUBSYSTEMS += durability
endif

ifeq ($(INCLUDE_SERVICES_CMSOAP),yes)
SUBSYSTEMS += cmsoap
endif

ifeq ($(INCLUDE_SERVICES_NWBRIDGE),yes)
SUBSYSTEMS += nwbridge
endif

ifndef OSPL_OUTER_HOME
ifeq ($(INCLUDE_SERVICES_WRAPPER_LIB),yes)
SUBSYSTEMS += servicesWrapperLib
endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
