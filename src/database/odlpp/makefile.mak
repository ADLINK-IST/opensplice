#
# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= odlpp

include		$(OSPL_HOME)/setup/makefiles/target.mak

LDFLAGS		= -L$(OSPL_HOME)/lib/$(SPLICE_HOST)
LDLIBS		+= -l$(DDS_DATABASE) -l$(DDS_OS)
LDLIBS		:=$(filter-out -lefence, $(LDLIBS))

INCLUDE		+= -I../../../database/include

-include $(DEPENDENCIES)
