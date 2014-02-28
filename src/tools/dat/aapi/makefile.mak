# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		 := aapi
TARGET_LINK_DIR := $(SPLICE_LIBRARY_PATH)

include $(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS  += $(SHCFLAGS) $(MTCFLAGS)
CINCS   += -I$(OSPL_HOME)/src/database/database/include
CINCS   += -I$(OSPL_HOME)/src/database/database/code

LDFLAGS += $(SHLDFLAGS)
LDLIBS  += $(SHLDLIBS)

-include $(DEPENDENCIES)
