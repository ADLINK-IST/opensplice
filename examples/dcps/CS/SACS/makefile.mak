TARGET_CSEXEC = ping
CS_NAMESPCS	 = Ping

# Specify the location of the IDL data model, and the generation results of idlpp.
IDL_DIR     := ../../PingPong
TOPIC_IDL   := $(notdir $(wildcard $(IDL_DIR)/*.idl))
IDL_CS      := $(TOPIC_IDL:%.idl=%.cs) $(TOPIC_IDL:%.idl=I%Dcps.cs) $(TOPIC_IDL:%.idl=%Dcps.cs) $(TOPIC_IDL:%.idl=%SplDcps.cs)

TARGET_LINK_DIR = ../../exec/$(SPLICE_TARGET)
CODE_DIR	:= ../../PingPong

# Specify the CSharp API on which this application depends.
CS_API = $(OSPL_HOME)/src/api/dcps/sacs/bld/$(SPLICE_TARGET)/$(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API)

include $(OSPL_HOME)/setup/makefiles/target.mak

all link: pong.exe

# Generate the C# interfaces from the IDL descriptions.
$(IDL_CS): $(IDL_DIR)/$(TOPIC_IDL)
	idlpp -l cs -S $<

pong.exe: ../../PingPong/Pong/pong.cs ../../PingPong/Pong/ErrorHandler.cs ../../PingPong/Pong/ponger.cs ../../PingPong/Pong/stats.cs ../../PingPong/Pong/time.cs $(IDL_CS)
	$(CSC) $(CSFLAGS) -out:pong.exe $(CSTARGET_EXEC) $(CS_LIBS) $^
	@rm $(notdir $(CS_LIB_FILES))
	cp pong.exe $(TARGET_LINK_DIR)
