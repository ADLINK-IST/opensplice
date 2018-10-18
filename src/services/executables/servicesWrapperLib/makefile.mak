# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_SERVICES_WRAPPER)

OBJECTS += ../../../spliced/bld/$(SPLICE_TARGET)/mainWrapper$(OBJ_POSTFIX)

ifeq ($(INCLUDE_SERVICES_DDSI),yes)
OBJECTS += ../../../ddsi2/bld/$(SPLICE_TARGET)/mainWrapper$(OBJ_POSTFIX)
endif

ifeq ($(INCLUDE_SERVICES_DDSI2E),yes)
OBJECTS += ../../../ddsi2e/bld/$(SPLICE_TARGET)/mainWrapper$(OBJ_POSTFIX)
endif

ifeq ($(INCLUDE_SERVICES_DURABILITY),yes)
OBJECTS += ../../../durability/bld/$(SPLICE_TARGET)/mainWrapper$(OBJ_POSTFIX)
endif

ifeq ($(INCLUDE_SERVICES_CMSOAP),yes)
OBJECTS += ../../../cmsoap/bld/$(SPLICE_TARGET)/mainWrapper$(OBJ_POSTFIX)
endif

include $(OSPL_HOME)/setup/makefiles/target.mak

-include $(DEPENDENCIES)
