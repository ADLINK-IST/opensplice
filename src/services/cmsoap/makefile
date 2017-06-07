include $(OSPL_HOME)/setup/makefiles/makefile.mak

all link: bld/$(SPLICE_TARGET)/makefile 
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@
clean:
	@rm -rf bld/$(SPLICE_TARGET)
	@rm -rf bldinc/$(SPLICE_TARGET)
	@rm -rf soapinclude/$(SPLICE_TARGET)
	@rm -f $(OSPL_HOME)/exec/$(SPLICE_TARGET)/cmsoap
