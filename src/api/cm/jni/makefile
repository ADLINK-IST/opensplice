include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all compile link gcov qac: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET)

else

all compile link gcov qac: bld/$(SPLICE_HOST)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST);export SPLICE_LIBRARY_PATH;cd $(OSPL_HOME)/src/kernel;make;cd $(OSPL_HOME)/src/configuration/;make;cd $(OSPL_HOME)/src/user;make;cd $(OSPL_HOME)/src/api/cm/xml;make)
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C bld/$(SPLICE_HOST) $@)

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_HOST)
endif
