#
# Set subsystems to be processed
#
ifeq (,$(findstring wince,$(SPLICE_TARGET))) # if not windows ce
ifeq (,$(findstring vxworks,$(SPLICE_TARGET))) # if not vxworks
ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)))) # if windows
SUBSYSTEMS := win32
else
SUBSYSTEMS := unix
endif #win32
endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
