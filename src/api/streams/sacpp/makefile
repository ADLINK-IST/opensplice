include $(OSPL_HOME)/setup/makefiles/makefile.mak

all link: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@ 


clean:
	@rm -rf bld/$(SPLICE_TARGET)
	@rm -rf include/ccpp_streams_if.h include/streams_dcps.h include/streams_dcpsSplDcps.h
