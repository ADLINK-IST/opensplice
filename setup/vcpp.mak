VCPP7 = $(findstring 13.10.,$(OSPL_COMPILER_VER))
VCPP8 = $(findstring 14.00.,$(OSPL_COMPILER_VER))
VCPP9 = $(findstring 15.00.,$(OSPL_COMPILER_VER))
VCPP10 = $(findstring 16.00.,$(OSPL_COMPILER_VER))

ifeq "$(VCPP7)$(VCPP8)$(VCPP9)$(VCPP10)" ""
$(error Could not determine compiler version!)
endif

MPC_JISH_TYPE_TO_GEN = "javabat"

ifneq ("$(VCPP7)", "")
MPC_CISH_TYPE_TO_GEN = "vc7"
else
ifneq ("$(VCPP8)", "")
MPC_CISH_TYPE_TO_GEN = "vc8"
else
ifneq ("$(VCPP9)", "")
MPC_CISH_TYPE_TO_GEN = "vc9"
else
ifneq ("$(VCPP10)", "")
MPC_CISH_TYPE_TO_GEN = "vc10"
endif
endif
endif
endif
