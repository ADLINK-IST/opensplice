include $(OSPL_HOME)/setup/makefiles/makefile.mak

all link analyse test: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

clean:
	@rm -rf bld/$(SPLICE_TARGET) exec/
