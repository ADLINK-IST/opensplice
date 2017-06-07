ifeq ($(SPLICE_HOST),$(SPLICE_TARGET))
include $(OSPL_HOME)/setup/arm.linux_native-common.mak
else
include $(OSPL_HOME)/setup/arm.linux_cross-common.mak
endif
