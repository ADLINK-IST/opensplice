#
# Set subsystems to be processed
#
SUBSYSTEMS	:= setup
SUBSYSTEMS	+= src

#Keep the extract step as the last thing to be done on VX
#as it creates the one core loadable lib out of all the small ones
ifeq (vxworks5,$(findstring vxworks5,$(SPLICE_TARGET)))
#Build one big loadable core library
SUBSYSTEMS	+= extract
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

clean:
	@rm -rf $(OSPL_HOME)/lib/$(SPLICE_TARGET)
	@rm -rf $(OSPL_HOME)/bin/$(SPLICE_TARGET)

.PHONY: install
install: link
	@$(MAKE) -C install
#	(if [ "$$SPLICE_HOST" != "$$SPLICE_TARGET" ]; then SPLICE_PLATFORM=$$SPLICE_HOST; export SPLICE_PLATFORM; . ./setup; make; fi)
#	(cd install; make)
