#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_KERNEL)
ODL_MODULES	:= kernelModule
EXTRACTED_LIB = libddscore

include	$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_KERNEL
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/database/serialization/include

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

LDLIBS	+= -l$(DDS_SERIALIZATION) -l$(DDS_DATABASE) -l$(DDS_OS)

-include $(DEPENDENCIES)
