#
# Set subsystems to be processed
#
ifndef OSPL_OUTER_HOME
ifneq (,$(findstring win32,$(SPLICE_TARGET)))
SUBSYSTEMS	:= win32
else
SUBSYSTEMS	:= unix
endif #win32

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

else # OSPL_OUTER_HOME

.PHONY:
all compile link gcov qac clean analyse:
	@$(MAKE) -C $(OSPL_OUTER_HOME)/src/tools/ospl $@

endif
