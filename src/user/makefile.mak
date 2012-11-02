TARGET_DLIB		:= $(DDS_USER)
EXTRACTED_LIB = libddscore

include		$(OSPL_HOME)/setup/makefiles/target.mak

ifdef INCLUDE_PLUGGABLE_REPORTING
ifneq (,$(findstring yes,$(INCLUDE_PLUGGABLE_REPORTING)))
CPPFLAGS	+= -DINCLUDE_PLUGGABLE_REPORTING
CFLAGS      += -DINCLUDE_PLUGGABLE_REPORTING
endif
endif

CPPFLAGS	+= -DOSPL_BUILD_USER
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/kernel/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/config/include
CINCS		+= -I$(OSPL_HOME)/src/configuration/parser/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)
LDLIBS   += -l$(DDS_OS) -l$(DDS_DATABASE) -l$(DDS_KERNEL) 
LDLIBS   += -l$(DDS_CONF) -l$(DDS_CONFPARSER)

-include $(DEPENDENCIES)
