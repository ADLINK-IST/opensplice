#
# Set subsystems to be processed
#
SUBSYSTEMS	:= spliced

ifeq ($(INCLUDE_SERVICES_DDSI),yes)
SUBSYSTEMS	+= ddsi2
endif

ifeq ($(INCLUDE_SERVICES_CMSOAP),yes)
SUBSYSTEMS	+= cmsoap
endif

ifeq ($(INCLUDE_SERVICES_CMAGENT),yes)
SUBSYSTEMS      += cmagent
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
