TARGET_DLIB		:= ddscore

OBJECTS = $(wildcard $(OSPL_HOME)/extract/libddscore/bld/$(SPLICE_TARGET)/*.o)

include	$(OSPL_HOME)/setup/makefiles/target.mak

