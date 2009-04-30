# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC	:= ospl

include $(OSPL_HOME)/setup/makefiles/target.mak

LDLIBS  += -l$(DDS_OS) -l$(DDS_CONF) -l$(DDS_CONFPARSER) -l$(DDS_DATABASE) -l$(DDS_UTIL)

CINCS	+= -I$(OSPL_HOME)/src/database/database/include
CINCS	+= -I$(OSPL_HOME)/src/configuration/config/include
CINCS	+= -I$(OSPL_HOME)/src/configuration/parser/include

-include $(DEPENDENCIES)
