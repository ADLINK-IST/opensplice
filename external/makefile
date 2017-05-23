#
# Set subsystems to be processed
#
PSUBSYSTEMS	:=
ifeq ($(INCLUDE_LZF),yes)
PSUBSYSTEMS	+= lzf
endif
ifeq ($(INCLUDE_SNAPPY),yes)
PSUBSYSTEMS	+= snappy-c
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
