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

ifeq ($(INCLUDE_API_STREAMS),yes)
SUBSYSTEMS       += streams
endif

ifeq ($(INCLUDE_API_FACE),yes)
SUBSYSTEMS       += face
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
