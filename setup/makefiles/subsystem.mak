all:		link 
compile:	$(addsuffix .ss_compile,	$(SUBSYSTEMS))
link:		$(addsuffix .ss_link,		$(SUBSYSTEMS))
clean:		$(addsuffix .ss_clean,		$(SUBSYSTEMS))
qac:		$(addsuffix .ss_qac,		$(SUBSYSTEMS))
analyse:	$(addsuffix .ss_analyse,	$(SUBSYSTEMS))
gcov:		$(addsuffix .ss_gcov,		$(SUBSYSTEMS))
test:   	$(addsuffix .ss_test,   	$(SUBSYSTEMS))

%.ss_compile:	; @$(MAKE) -C $* compile
%.ss_link:	; @$(MAKE) -C $* link
%.ss_clean:	; @$(MAKE) -C $* clean
%.ss_qac:	; @$(MAKE) -C $* qac
%.ss_analyse:	; @$(MAKE) -C $* analyse
%.ss_gcov:	; @$(MAKE) -C $* gcov
%.ss_test:      ; @$(MAKE) -C $* test

ifneq (,$(wildcard $(OSPL_HOME)/setup/$(SPLICE_TARGET)))
include $(OSPL_HOME)/setup/$(SPLICE_TARGET)/config.mak
else
include $(OSPL_OUTER_HOME)/setup/$(SPLICE_TARGET)/config.mak
endif
