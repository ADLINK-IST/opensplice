include $(OSPL_HOME)/setup/makefiles/makefile.mak
include $(OSPL_HOME)/src/tools/idlpp/makefile_templates.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all compile link gcov qac: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET)
	rm -f $(ACE_TAO_1_4_1_TMPLS)
	rm -f $(MICO_2_3_11_TMPLS)
	rm -f $(EORB_3_0_TMPLS)
	rm -f $(OPENFUSION_1_4_1_TMPLS)
	rm -f $(OPENFUSION_1_5_1_TMPLS)
	rm -f $(OPENFUSION_1_6_1_TMPLS)
	rm -f $(SACPP_TMPLS)

else

all compile link metre splint qac: bld/$(SPLICE_HOST)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH; export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; cd $(OSPL_HOME)/src/osplcore; make)
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH; SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C bld/$(SPLICE_HOST) $@)

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; cd $(OSPL_HOME)/src/database; make $@)
	@rm -rf bld/$(SPLICE_HOST)
	rm -f $(ACE_TAO_1_4_1_TMPLS)
	rm -f $(MICO_2_3_11_TMPLS)
	rm -f $(EORB_3_0_TMPLS)
	rm -f $(OPENFUSION_1_4_1_TMPLS)
	rm -f $(OPENFUSION_1_5_1_TMPLS)
	rm -f $(OPENFUSION_1_6_1_TMPLS)
	rm -f $(OPENFUSION_2_TMPLS)
	rm -f $(SACPP_TMPLS)

endif
