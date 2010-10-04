TARGET_CSEXEC = MessageBoard
CS_NAMESPCS	 = Chatroom

# Specify the location of the IDL data model, and the generation results of idlpp.
IDL_DIR     := ../../Data
TOPIC_IDL   := $(notdir $(wildcard $(IDL_DIR)/*.idl))
IDL_CS    = $(TOPIC_IDL:%.idl=%.cs) $(TOPIC_IDL:%.idl=I%Dcps.cs) $(TOPIC_IDL:%.idl=%Dcps.cs) $(TOPIC_IDL:%.idl=%SplDcps.cs)

TARGET_LINK_DIR = ../../exec/$(SPLICE_TARGET)
CODE_DIR	:= ../../MessageBoard

# Specify the CSharp API on which this application depends.
CS_API = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API)

include $(OSPL_HOME)/setup/makefiles/target.mak

all link: Chatter.exe UserLoad.exe

# Generate the C# interfaces from the IDL descriptions.
$(IDL_CS): $(IDL_DIR)/$(TOPIC_IDL)
	idlpp -l cs -S $<

Chatter.exe: ../../Chatter/Chatroom/Chatter.cs ../../Chatter/Chatroom/ErrorHandler.cs $(IDL_CS)
	@cp $(CS_LIB_FILES) .
	$(CSC) $(CSFLAGS) -out:Chatter.exe $(CSTARGET_EXEC) $(CS_LIBS) $^
	@rm $(notdir $(CS_LIB_FILES))
	cp Chatter.exe $(TARGET_LINK_DIR)

UserLoad.exe: ../../UserLoad/Chatroom/UserLoad.cs ../../UserLoad/Chatroom/ErrorHandler.cs $(IDL_CS)
	@cp $(CS_LIB_FILES) .
	$(CSC) $(CSFLAGS) -out:UserLoad.exe $(CSTARGET_EXEC) $(CS_LIBS) $^
	@rm $(notdir $(CS_LIB_FILES))
	cp UserLoad.exe $(TARGET_LINK_DIR)
