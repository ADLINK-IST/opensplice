#
#included by bld/$(SPLICE_TARGET)/makefile

include $(OSPL_HOME)/setup/makefiles/makefile.mak

TARGET_CSEXEC = ping
CS_NAMESPCS	 = . pingpong

# Specify the location of the IDL data model, and the generation results of idlpp.
IDL_DIR     := ../../PingPong
TOPIC_IDL   := $(notdir $(wildcard $(IDL_DIR)/*.idl))
IDLPP_CS    := $(TOPIC_IDL:%.idl=%.cs) $(TOPIC_IDL:%.idl=I%Dcps.cs) $(TOPIC_IDL:%.idl=%Dcps.cs) $(TOPIC_IDL:%.idl=%SplDcps.cs)

TARGET_LINK_DIR = ../../exec/$(SPLICE_TARGET)

CODE_DIR	:= ../../PingPong/Ping

all link: idlgen csapi csc pong.exe

include $(OSPL_HOME)/setup/makefiles/target.mak

# Specify the CSharp API on which this application depends.
CS_API      := $(CSLIB_PREFIX)$(DDS_DCPSSACS)$(CSLIB_POSTFIX)

# Add the generated files to the compiler-list.
CS_FILES += $(IDLPP_CS)

# Fine tune the compiler flags.
CSLIBS += -r:$(CS_API)


idlgen: $(IDLPP_CS) 

csapi: $(TARGET_LINK_DIR) $(CS_API)

# Copy the C# API next to the executable (required when not yet copied to GAC)
$(CS_API):
	cp ../../../../../../lib/$(SPLICE_TARGET)/$(CS_API) .
	cp ../../../../../../lib/$(SPLICE_TARGET)/$(CS_API) $(TARGET_LINK_DIR)

# Generate the C# interfaces from the IDL descriptions.

$(IDLPP_CS): $(IDL_DIR)/$(TOPIC_IDL)
	idlpp -l cs -S $<

pong.exe: ../../PingPong/Pong/pong.cs ../../PingPong/Pong/ErrorHandler.cs ../../PingPong/Pong/ponger.cs ../../PingPong/Pong/stats.cs ../../PingPong/Pong/time.cs $(IDLPP_CS)
	$(CSC) $(CSFLAGS) -out:pong.exe $(CSTARGET_EXEC) $(CSLIBS) $^
	cp pong.exe $(TARGET_LINK_DIR)
