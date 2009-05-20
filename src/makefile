#
# Set subsystems to be processed
#
SUBSYSTEMS	:= abstraction
ifeq ($(INCLUDE_UTILITIES),yes)
SUBSYSTEMS	+= utilities # should never be dependent on other layers than the abstraction layer
endif
SUBSYSTEMS	+= database
SUBSYSTEMS	+= kernel
SUBSYSTEMS	+= configuration
SUBSYSTEMS	+= user
ifeq ($(INCLUDE_API),yes)
SUBSYSTEMS	+= api
endif
ifeq ($(INCLUDE_SERVICES),yes)
SUBSYSTEMS	+= services
endif

ifeq ($(INCLUDE_TOOLS),yes)
ifeq ($(INCLUDE_TOOLS_IDLPP),yes)
SUBSYSTEMS	+= cpp
endif
SUBSYSTEMS	+= tools
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
