VCPP7 = $(findstring 13.10.,$(OSPL_COMPILER_VER))
VCPP8 = $(findstring 14.00.,$(OSPL_COMPILER_VER))
VCPP9 = $(findstring 15.00.,$(OSPL_COMPILER_VER))
VCPP10 = $(findstring 16.00.,$(OSPL_COMPILER_VER))
VCPP11 = $(findstring 17.00.,$(OSPL_COMPILER_VER))
VCPP12 = $(findstring 18.00.,$(OSPL_COMPILER_VER))

ifeq "$(VCPP7)$(VCPP8)$(VCPP9)$(VCPP10)$(VCPP11)$(VCPP12)" ""
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
else
ifneq ("$(VCPP11)", "")
MPC_CISH_TYPE_TO_GEN = "vc11"
else
ifneq ("$(VCPP12)", "")
MPC_CISH_TYPE_TO_GEN = "vc12"
endif
endif
endif
endif
endif
endif
