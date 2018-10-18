TARGET_EXEC := durability

include $(OSPL_HOME)/setup/makefiles/target.mak

ifeq ($(INCLUDE_SQLITE), yes)
DURABILITY_PLUGIN_LIBS += -lsqlite3-ospl
DURABILITY_PLUGIN_CINCS += -I$(SQLITE_HOME)
DURABILITY_PLUGIN_CPPFLAGS += -DKV_INCLUDE_SQLITE=1
endif

ifeq ($(INCLUDE_LEVELDB), yes)
DURABILITY_PLUGIN_LIBS += -lleveldb-ospl
DURABILITY_PLUGIN_CINCS += -I$(LEVELDB_HOME)/include
DURABILITY_PLUGIN_CPPFLAGS += -DKV_INCLUDE_LEVELDB=1
endif

LD_EXE = $(LD_CXX)  # c++ binary (because of LevelDB)

CINCS  += -I$(OSPL_HOME)/src/database/serialization/include
CINCS  += -I$(OSPL_HOME)/src/database/database/include
CINCS  += -I$(OSPL_HOME)/src/kernel/include
CINCS  += -I$(OSPL_HOME)/src/user/include
CINCS  += -I$(OSPL_HOME)/src/services/durability/include
CINCS  += -I$(OSPL_HOME)/src/services/durability/code
CINCS  += -I$(OSPL_HOME)/src/services/durability/bld/$(SPLICE_TARGET)
CINCS  += -I$(OSPL_HOME)/src/configuration/config/include

LDLIBS := -l$(DDS_DURABILITY) $(DURABILITY_PLUGIN_LIBS) -l$(DDS_CORE) $(LDLIBS)
