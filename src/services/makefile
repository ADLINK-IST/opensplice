#
# Set subsystems to be processed
#
SUBSYSTEMS	:= spliced

ifeq ($(INCLUDE_SERVICES_DDSI),yes)
SUBSYSTEMS	+= ddsi2
endif

ifeq ($(INCLUDE_SERVICES_DDSI2E),yes)
SUBSYSTEMS	+= ddsi2e
endif

ifeq ($(INCLUDE_SERVICES_DURABILITY),yes)
SUBSYSTEMS	+= durability
endif

ifeq ($(INCLUDE_SERVICES_CMSOAP),yes)
SUBSYSTEMS	+= cmsoap
endif

ifeq ($(INCLUDE_SERVICES_CMAGENT),yes)
SUBSYSTEMS      += cmagent
endif

ifeq ($(INCLUDE_SERVICES_NWBRIDGE),yes)
SUBSYSTEMS      += nwbridge
endif

# Now build the process executables for these services where required
ifeq ($(INCLUDE_SERVICES_AS_EXECUTABLES),yes)
SUBSYSTEMS      += executables
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
