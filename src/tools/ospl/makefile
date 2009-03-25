#
# Set subsystems to be processed
#
ifeq (,$(findstring vxworks,$(SPLICE_TARGET)))
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
SUBSYSTEMS	:= win32
else
SUBSYSTEMS	:= unix
endif #win32
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

