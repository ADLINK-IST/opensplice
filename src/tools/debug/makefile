#
# Set subsystems to be processed
#
SUBSYSTEMS	:=

ifeq ($(INCLUDE_TOOLS_DEBUG_DBD),yes)
SUBSYSTEMS	+= dbd
endif

ifeq ($(INCLUDE_SERVICES_DURABILITY),yes)
ifeq ($(INCLUDE_TOOLS_DEBUG_MMFD),yes)
SUBSYSTEMS	+= mmfd
endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
