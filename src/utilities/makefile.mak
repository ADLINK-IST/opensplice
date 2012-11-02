# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_UTIL)
EXTRACTED_LIB = libddscore

ifneq (,$(findstring vxworks6,$(SPLICE_TARGET)))
OS_UTILS        = ut_avltree.c ut_collection.c
C_FILES         = $(filter-out $(OS_UTILS),$(notdir $(wildcard $(CODE_DIR)/*.c)))
endif

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_UTIL
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
# CINCS    += -I$(OSPL_HOME)/src/database/database/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

LDLIBS   += -l$(DDS_OS)

-include $(DEPENDENCIES)
