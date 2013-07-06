# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_UTIL)

ifneq (,$(findstring vxworks6,$(SPLICE_TARGET)))
ifneq (,$(UT_TRACE_EXTENSION))
OS_UTILS        = ut_avltree.c ut_collection.c
C_FILES         = $(filter-out $(OS_UTILS),$(notdir $(wildcard $(CODE_DIR)/*.c)))
else
OS_UTILS        = ut_avltree.c ut_collection.c ut_backtrace-symbols.c
C_FILES         = $(filter-out $(OS_UTILS),$(notdir $(wildcard $(CODE_DIR)/*.c)))
endif
else
ifeq (,$(UT_TRACE_EXTENSION))
OS_UTILS        = ut_backtrace-symbols.c
C_FILES         = $(filter-out $(OS_UTILS),$(notdir $(wildcard $(CODE_DIR)/*.c)))
endif
endif

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_UTIL
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
# CINCS    += -I$(OSPL_HOME)/src/database/database/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

LDLIBS   += -l$(DDS_OS)

ifneq (,$(UT_TRACE_EXTENSION))
LDLIBS      += -lbfd -liberty
endif

-include $(DEPENDENCIES)
