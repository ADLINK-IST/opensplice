#
include $(OSPL_HOME)/setup/makefiles/makefile.mak

all link: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@


clean:
	@rm -rf bld/$(SPLICE_TARGET)

osplconf osplconf-update ddsi2e-config: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@
