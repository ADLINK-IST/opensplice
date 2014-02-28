#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_CSMOD = $(DDS_DCPSSACS)
CS_NAMESPCS	 = DDS DDS/OpenSplice DDS/OpenSplice/Gapi DDS/OpenSplice/Common DDS/OpenSplice/Database DDS/OpenSplice/CustomMarshalers DDS/OpenSplice/Kernel

# Also specify an assembly for testing purposes.
TARGET_ASSEMBLY = $(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)

# Input IDL files.
IDL_DIR     := $(OSPL_HOME)/etc/idl
vpath %.idl $(IDL_DIR)
TOPIC_IDL   := dds_dcps_builtintopics.idl dds_builtinTopics.idl dds_namedQosTypes.idl

# idlpp compiler settings.
IDLPP       = $(WINCMD) idlpp 
IDL_INC_FLAGS= -I$(IDL_DIR)
IDLPPFLAGS  := $(IDL_INC_FLAGS) -l cs -S -o custom-psm

# idlpp output
IDL_CS   = $(TOPIC_IDL:%.idl=%.cs) $(TOPIC_IDL:%.idl=I%Dcps.cs) $(TOPIC_IDL:%.idl=%Dcps.cs) $(TOPIC_IDL:%.idl=%SplDcps.cs)

include $(OSPL_HOME)/setup/makefiles/target.mak

all link: $(TARGET_ASSEMBLY)

.PHONY: make_idl_preprocessor

# Make preprocessor if not already done so.
$(addprefix $(IDL_DIR)/,$(TOPIC_IDL)): make_idl_preprocessor

make_idl_preprocessor:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

# Generate the C# interfaces from the IDL descriptions.
%.cs I%Dcps.cs %Dcps.cs : %.idl
	$(IDLPP) $(IDLPPFLAGS) $<
	
# Generate the assembly as well.
$(TARGET_ASSEMBLY): $(CS_FILES)
	$(CSC) $(CSFLAGS) -out:$(TARGET_ASSEMBLY) $(CSTARGET_LIB) $(CSLIBS) $(CS_FILES)

