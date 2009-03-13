#
# Set subsystems to be processed
#
SUBSYSTEMS	:=

ifeq ($(INCLUDE_TOOLS_IDLPP),yes)
SUBSYSTEMS	+= idlpp
endif

ifeq ($(INCLUDE_TOOLS_OSPL),yes)
SUBSYSTEMS	+= ospl
endif

ifneq (,$(findstring devdat,$(SPLICE_TARGET)))
SUBSYSTEMS	+= dat
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
