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

MPC_CISH_TYPE_TO_GEN = "make"
MPC_JISH_TYPE_TO_GEN = "ospljavamake"
ifneq ("$(VCPP7)", "")
MPC_CISH_TYPE_TO_GEN = "vc7"
MPC_JISH_TYPE_TO_GEN = "javabat"
else
ifneq ("$(VCPP8)", "")
MPC_CISH_TYPE_TO_GEN = "vc8"
MPC_JISH_TYPE_TO_GEN = "javabat"
else
ifneq ("$(VCPP9)", "")
MPC_CISH_TYPE_TO_GEN = "vc9"
MPC_JISH_TYPE_TO_GEN = "javabat"
else
ifneq ("$(VCPP10)", "")
MPC_CISH_TYPE_TO_GEN = "vc10"
MPC_JISH_TYPE_TO_GEN = "javabat"
else
ifneq ("$(VCPP11)", "")
MPC_CISH_TYPE_TO_GEN = "vc11"
MPC_JISH_TYPE_TO_GEN = "javabat"
else
ifneq ("$(VCPP12)", "")
MPC_CISH_TYPE_TO_GEN = "vc12"
MPC_JISH_TYPE_TO_GEN = "javabat"
endif
endif
endif
endif
endif
endif

ifeq (vxworks6,$(findstring vxworks6,$(SPLICE_TARGET))) # if VxWorks 6.x
MPC_CISH_TYPE_TO_GEN = "wb30"
endif
