ifeq (armv8,$(findstring armvv8,$(SPLICE_HOST)))
include $(OSPL_HOME)/setup/arm.linux_native-common.mak
else
include $(OSPL_HOME)/setup/arm.linux_cross-common.mak
endif
