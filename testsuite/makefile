# Set subsystems to be processed
#
SUBSYSTEMS      := framework
SUBSYSTEMS      += dbt

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

gcov:

analyse:

clean:
	@rm -rf $(OSPL_OUTER_HOME)/testsuite/lib/$(SPLICE_TARGET)
	@rm -rf $(OSPL_OUTER_HOME)/testsuite/exec/$(SPLICE_TARGET)

