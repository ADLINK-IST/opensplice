#
# Set subsystems to be processed
#
SUBSYSTEMS	 :=

ifeq ($(INCLUDE_API_CM),yes)
SUBSYSTEMS	 += cm
endif

ifeq ($(INCLUDE_API_DCPS),yes)
SUBSYSTEMS	 += dcps
endif

ifeq ($(INCLUDE_DLRL),yes)
ifeq ($(INCLUDE_API_DLRL),yes)
SUBSYSTEMS	 += dlrl
endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
