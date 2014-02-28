#
# Set subsystems to be processed
#
SUBSYSTEMS	:= cpp
SUBSYSTEMS      += osplcore
ifeq ($(INCLUDE_API),yes)
SUBSYSTEMS	+= api
endif
ifeq ($(INCLUDE_SERVICES),yes)
SUBSYSTEMS	+= services
endif

ifeq ($(INCLUDE_TOOLS),yes)
SUBSYSTEMS	+= tools
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
