#
# Set subsystems to be processed
#
ifneq (,$(findstring vxworks,$(SPLICE_TARGET)))
SUBSYSTEMS      := vxworks
else
SUBSYSTEMS      := unix
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
