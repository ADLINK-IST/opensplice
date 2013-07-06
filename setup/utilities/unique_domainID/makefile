include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)),$(findstring win64,$(SPLICE_TARGET))))
all compile link gcov qac:
	@$(MAKE) -f makefile.cygwin $@

analyse:

clean:
	@rm -rf bld/

else
all compile link gcov qac: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/
endif
