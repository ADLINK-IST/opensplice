ifneq (,$(wildcard $(OSPL_HOME)/setup/$(SPLICE_TARGET)))
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
else
include $(OSPL_OUTER_HOME)/setup/$(SPLICE_TARGET)/config.mak
endif

.PRECIOUS: %/.STAMP
%/.STAMP:
	@[ -d $* ] || mkdir -p $*
	@$(TOUCH) $@

%/makefile: %/.STAMP
	@(echo '# automatically generated makefile' ; \
	echo 'include ../../makefile.mak') > $@

%generated/makefile: makefile
	@cp $< $@

%generated/makefile.mak: makefile.mak
	@cp $< $@
