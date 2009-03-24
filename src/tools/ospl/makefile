#
# Set subsystems to be processed
#
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
SUBSYSTEMS	:= win32
else
SUBSYSTEMS	:= unix
endif #win32

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

