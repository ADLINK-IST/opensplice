# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB		:= aapi
TARGET_LINK_DIR	:= $(SPLICE_LIBRARY_PATH)

include		$(OSPL_HOME)/setup/makefiles/target.mak

CFLAGS          += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS         += $(SHLDFLAGS)
LDLIBS          += $(SHLDLIBS)

INCLUDE         += -I$(OSPL_HOME)/src/database/database/include
INCLUDE         += -I$(OSPL_HOME)/src/database/database/code

LDLIBS		+= -l$(DDS_OS)

-include $(DEPENDENCIES)
