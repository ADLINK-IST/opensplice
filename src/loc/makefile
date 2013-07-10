#
# Set subsystems to be processed
#

SUBSYSTEMS	 := util
SUBSYSTEMS	 += collections

ifeq ($(INCLUDE_DLRL),yes)
BUILD_EXTRA_NEEDED_LOCS = yes
endif

ifeq ($(BUILD_EXTRA_NEEDED_LOCS),yes)
SUBSYSTEMS	 += metamodel
SUBSYSTEMS	 += kernel
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
