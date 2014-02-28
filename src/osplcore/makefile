include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all compile link gcov qac: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

analyse:
	@:

clean:
	@rm -rf bld/$(SPLICE_TARGET)

else

all compile link gcov qac: bld/$(SPLICE_HOST)/makefile bld/$(SPLICE_TARGET)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST);export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST);$(MAKE) $@)
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

analyse:
	@:

clean:
	@rm -rf bld/$(SPLICE_TARGET)
	@rm -rf bld/$(SPLICE_HOST)

endif
