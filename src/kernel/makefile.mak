#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_KERNEL)
ODL_MODULES	:= kernelModule

include		$(OSPL_HOME)/setup/makefiles/target.mak

CPPFLAGS	+= -DOSPL_BUILD_KERNEL
CFLAGS   += $(SHCFLAGS) $(MTCFLAGS)
LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += $(SHLDLIBS)

INCLUDE		+= -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/database/serialization/include

LDLIBS		+= -l$(DDS_SERIALIZATION) -l$(DDS_DATABASE) -l$(DDS_OS)

-include $(DEPENDENCIES)
