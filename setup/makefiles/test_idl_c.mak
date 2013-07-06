# default values for directory and idl-files to process
IDL_DIR     ?= ../../code
IDL_FILES   ?= $(notdir $(wildcard $(IDL_DIR)/*.idl))
vpath %.idl     $(IDL_DIR)

# idl preprocessor
IDLPP		?= idlpp

# variables that can be used to extend/overrule the defaults for IDL_H and IDL_C
IDL_CTYPE_FILES = $(IDL_FILES:%.idl=%CorbaType.h)
IDL_TYPE_FILES = $(IDL_FILES:%.idl=%SplType.h)
IDL_LOAD_FILES = $(IDL_FILES:%.idl=%SplLoad.c)
IDL_COPY_FILES = $(IDL_FILES:%.idl=%SplCopy.c)
IDL_HELP_FILES = $(IDL_FILES:%.idl=%SplHelp.c)
IDL_STDH_FILES = $(IDL_FILES:%.idl=%.h) $(IDL_FILES:%.idl=%Dcps.h) $(IDL_FILES:%.idl=%SplDcps.h) $(IDL_FILES:%.idl=%SacDcps.h)
IDL_STDC_FILES = $(IDL_FILES:%.idl=%SplDcps.c) $(IDL_FILES:%.idl=%SacDcps.c)

# This determines what/how it will be processed
# IDL_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# IDL_O will be linked into the final target
IDL_H		= $(IDL_STDH_FILES)
IDL_C		= $(IDL_STDC_FILES)
IDL_O		= $(IDL_C:%.c=%$(OBJ_POSTFIX))

IDLPPFLAGS	:= -S -l c
#generic rules for IDL preprocessing
%.h %Dcps.h %SplDcps.h %SplDcps.c %SacDcps.h %SacDcps.c : %.idl
	$(IDLPP) $(IDLPPFLAGS) $<

%CorbaType.h  : %.idl
	$(IDLPP) $(IDLPPFLAGS) -m CORBATYPE $<

%SplType.h  : %.idl
	$(IDLPP) $(IDLPPFLAGS) -m SPLTYPE $<

%SplLoad.c  : %.idl
	$(IDLPP) $(IDLPPFLAGS) -m SPLLOAD $<

%SplCopy.c  : %.idl
	$(IDLPP) $(IDLPPFLAGS) -m SPLCOPY $<

%SplHelp.c  : %.idl
	$(IDLPP) $(IDLPPFLAGS) -m SPLHELP $<

%_register.c %_register.h : %.idl
	$(IDLPP) -o dds-types $(IDLPPFLAGS) -m TYPES=$(IDLPPTYPES) $<
