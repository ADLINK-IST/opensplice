#
# Set subsystems to be processed
#
ifneq (,$(findstring win32,$(SPLICE_TARGET))) # if windows
      SUBSYSTEMS := win32
else
   ifeq (,$(findstring wince,$(SPLICE_TARGET)))
      ifeq (,$(findstring vxworks,$(SPLICE_TARGET)))
         SUBSYSTEMS := unix
      endif
   endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
