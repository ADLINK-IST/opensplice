#
# Set subsystems to be processed
#
SUBSYSTEMS	:= spliced

ifeq ($(INCLUDE_SERVICES_NETWORKING),yes)
SUBSYSTEMS	+= networking
ifeq ($(INCLUDE_SERVICES_DDSI),yes)
SUBSYSTEMS	+= ddsi2
endif
endif

ifeq ($(INCLUDE_SERVICES_COMMON),yes)
SUBSYSTEMS	+= common
endif

ifeq ($(INCLUDE_SERVICES_DURABILITY),yes)
SUBSYSTEMS	+= durability
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
