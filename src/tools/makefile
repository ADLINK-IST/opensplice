#
# Set subsystems to be processed
#
SUBSYSTEMS	:=

ifeq ($(INCLUDE_JAVA),yes)
ifeq ($(INCLUDE_TOOLS_TUNER),yes)
SUBSYSTEMS	+= cm
endif
endif

ifeq ($(INCLUDE_TOOLS_IDLPP),yes)
SUBSYSTEMS	+= cppgen
SUBSYSTEMS	+= idlpp
endif

ifeq ($(INCLUDE_TOOLS_MMSTAT),yes)
SUBSYSTEMS	+= mmstat
endif

ifeq ($(INCLUDE_TOOLS_DEBUG),yes)
SUBSYSTEMS	+= debug
endif

ifneq (,$(findstring devdat,$(SPLICE_TARGET)))
SUBSYSTEMS	+= dat
endif

ifeq ($(INCLUDE_TOOLS_SHMDUMP),yes)
SUBSYSTEMS  += shmdump
endif

ifeq ($(INCLUDE_TOOLS_CONF2C),yes)
SUBSYSTEMS  += conf2c
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
