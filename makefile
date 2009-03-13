#
# Set subsystems to be processed
#
SUBSYSTEMS	:= setup
SUBSYSTEMS	+= src

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

clean:
	@rm -rf $(OSPL_HOME)/lib/$(SPLICE_TARGET)
	@rm -rf $(OSPL_HOME)/bin/$(SPLICE_TARGET)

.PHONY: install
install: link
	@$(MAKE) -C install
#	(if [ "$$SPLICE_HOST" != "$$SPLICE_TARGET" ]; then SPLICE_PLATFORM=$$SPLICE_HOST; export SPLICE_PLATFORM; . ./setup; make; fi)
#	(cd install; make)
