include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all link: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@


clean:
	@rm -rf bld/$(SPLICE_TARGET)

else

ifeq ($(OSPL_SPLIT_CORE),$(SPLICE_TARGET))
   define make_target
	@ SPLIT_CORE_PART=1 $(MAKE) -C bld/$(SPLICE_TARGET) $@
	@ SPLIT_CORE_PART=2 $(MAKE) -C bld/$(SPLICE_TARGET) $@
   endef
else
   define make_target
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@
   endef
endif

all link: bld/$(SPLICE_HOST)/makefile bld/$(SPLICE_TARGET)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST);export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH;$(MAKE) $@)
	$(make_target)


clean:
	@rm -rf bld/$(SPLICE_TARGET)
	@rm -rf bld/$(SPLICE_HOST)

endif
