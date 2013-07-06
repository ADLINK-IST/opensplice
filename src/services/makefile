#
# Set subsystems to be processed
#
SUBSYSTEMS	:= spliced

ifeq ($(INCLUDE_SERVICES_DDSI),yes)
SUBSYSTEMS	+= ddsi2
endif

ifeq ($(INCLUDE_SERVICES_COMMON),yes)
SUBSYSTEMS	+= common
endif

ifeq ($(INCLUDE_SERVICES_DURABILITY),yes)
SUBSYSTEMS	+= durability
endif

ifeq ($(INCLUDE_SERVICES_CMSOAP),yes)
SUBSYSTEMS	+= cmsoap
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
