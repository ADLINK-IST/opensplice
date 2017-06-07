include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifneq (,$(or $(findstring win32,$(SPLICE_TARGET)),$(findstring win64,$(SPLICE_TARGET))))
all link:
	@$(MAKE) -f makefile.cygwin $@

clean:
	@rm -rf bld/

else
all link: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@


clean:
	@rm -rf bld/
endif
