#
# Set subsystems to be processed
#
ifeq (,$(findstring vxworks,$(SPLICE_TARGET))) # if not vxworks
ifneq (,$(findstring win32,$(SPLICE_TARGET))) # if windows
SUBSYSTEMS	:= win32
else
SUBSYSTEMS	:= unix
endif #win32
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

