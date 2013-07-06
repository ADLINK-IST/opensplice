include $(OSPL_HOME)/setup/makefiles/makefile.mak

all compile link gcov qac: bld/$(SPLICE_TARGET)/makefile 
	@(cd $(OSPL_HOME)/src/api/cm/xml;make)
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET)
	@rm -rf bldinc/$(SPLICE_TARGET)
	@rm -rf soapinclude/$(SPLICE_TARGET)
	@rm -f $(OSPL_HOME)/exec/$(SPLICE_TARGET)/cmsoap
