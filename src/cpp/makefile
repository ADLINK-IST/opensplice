include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all compile link gcov qac: bld/$(SPLICE_HOST)/makefile
	@$(MAKE) -C bld/$(SPLICE_HOST) $@

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_HOST)

else

all compile link metre splint qac: bld/$(SPLICE_HOST)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST);export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; cd $(OSPL_HOME)/src/abstraction; make)
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C bld/$(SPLICE_HOST) $@)

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; cd $(OSPL_HOME)/src/abstraction; make $@)
	@rm -rf bld/$(SPLICE_HOST)

endif
