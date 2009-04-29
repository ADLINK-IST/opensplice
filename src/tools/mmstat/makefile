#
# Set subsystems to be processed
#
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
   #SUBSYSTEMS := win32
else
   ifneq (,$(findstring vxworks,$(SPLICE_TARGET)))
      SUBSYSTEMS := vxworks
   else
      SUBSYSTEMS := unix
   endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
