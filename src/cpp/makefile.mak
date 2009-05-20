#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_SLIB	:= $(DDS_CPP)

include $(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_CPP

CINCS	+= -I$(OSPL_HOME)/src/abstraction/os/include

-include $(DEPENDENCIES)
