include $(OSPL_HOME)/setup/makefiles/makefile.mak
include $(OSPL_HOME)/src/tools/idlpp/makefile_templates.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all link: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

clean: clean_cmn

else

all link: bld/$(SPLICE_HOST)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH; export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C $(OSPL_HOME)/src/osplcore $@)
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH; SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C bld/$(SPLICE_HOST) $@)

clean: clean_cmn
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; cd $(OSPL_HOME)/src/database; make $@)

endif

.PHONY: clean_cmn
clean_cmn:
	@rm -rf bld/$(SPLICE_TARGET)
	@if [ -d bld ] && [ `find bld/* -maxdepth 0 -type d 2> /dev/null | wc -l` -eq 0 ] ; then \
		rm -f $(ACE_TAO_1_4_1_TMPLS) ; \
		rm -f $(MICO_2_3_11_TMPLS) ; \
		rm -f $(MICO_2_3_13_TMPLS) ; \
		rm -f $(EORB_3_0_TMPLS) ; \
		rm -f $(OPENFUSION_1_4_1_TMPLS) ; \
		rm -f $(OPENFUSION_1_5_1_TMPLS) ; \
		rm -f $(OPENFUSION_1_6_1_TMPLS) ; \
		rm -f $(OPENFUSION_2_TMPLS) ; \
		rm -f $(DDS_ACE_TAO_5_6_6_TMPLS) ; \
		rm -f $(SACPP_TMPLS) ; \
	fi

