include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all link: bld/$(SPLICE_HOST)/makefile
	@$(MAKE) -C bld/$(SPLICE_HOST) $@

clean:
	@rm -rf bld/$(SPLICE_HOST)

else

all link: bld/$(SPLICE_HOST)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST);export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C $(OSPL_HOME)/src/abstraction $@)
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C bld/$(SPLICE_HOST) $@)

clean:
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; $(MAKE) -C $(OSPL_HOME)/src/abstraction $@)
	@rm -rf bld/$(SPLICE_HOST)

endif
