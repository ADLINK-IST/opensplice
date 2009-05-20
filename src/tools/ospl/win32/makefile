include $(OSPL_HOME)/setup/makefiles/makefile.mak

all compile link gcov qac: bld/$(SPLICE_HOST)/makefile
	@$(MAKE) -C bld/$(SPLICE_HOST) $@

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_HOST)
