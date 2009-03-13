CODE_DIR        := ../../

include $(OSPL_HOME)/setup/makefiles/target.mak

CHATTER         := Chatter
MSGBOARD        := MessageBoard
USRLOAD         := UserLoad
SUPPORT         := multitopic$(OBJ_POSTFIX) CheckStatus$(OBJ_POSTFIX)

EXEC_DIR	    := ../../exec/$(SPLICE_TARGET)

DCPS_IDL 	:= Chat.idl

DCPS_OBJ	:= $(DCPS_IDL:%.idl=%SplDcps$(OBJ_POSTFIX)) $(DCPS_IDL:%.idl=%SacDcps$(OBJ_POSTFIX))
DCPS_CPP	:= $(DCPS_OBJ:%$(OBJ_POSTFIX)=%.c)
DCPS_HDR	:= $(DCPS_OBJ:%$(OBJ_POSTFIX)=%.h) $(DCPS_IDL:%.idl=%.h)

INCLUDE		+= -I../..
INCLUDE		+= -I$(OSPL_HOME)/src/api/dcps/sac/include
INCLUDE     += -I$(OSPL_HOME)/src/database/database/include
INCLUDE		+= -I$(OSPL_HOME)/src/user/include
INCLUDE		+= -I$(OSPL_HOME)/src/kernel/include

LDLIBS		+= -ldcpssac

all: $(EXEC_DIR) $(EXEC_DIR)/$(CHATTER)$(EXEC_POSTFIX) $(EXEC_DIR)/$(MSGBOARD)$(EXEC_POSTFIX) $(EXEC_DIR)/$(USRLOAD)$(EXEC_POSTFIX)

$(EXEC_DIR):
	mkdir -p $(EXEC_DIR)

$(DCPS_CPP) $(DCPS_HDR): ../../$(DCPS_IDL)
	idlpp -l c -S $(INCLUDE) $<
	
$(CHATTER)$(EXEC_POSTFIX): $(DCPS_OBJ) $(SUPPORT) $(CHATTER)$(OBJ_POSTFIX)
	$(LD_EXE) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(MSGBOARD)$(EXEC_POSTFIX): $(DCPS_OBJ) $(SUPPORT) $(MSGBOARD)$(OBJ_POSTFIX)
	$(LD_EXE) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(USRLOAD)$(EXEC_POSTFIX): $(DCPS_OBJ) $(SUPPORT) $(USRLOAD)$(OBJ_POSTFIX)
	$(LD_EXE) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXEC_DIR)/$(CHATTER)$(EXEC_POSTFIX): $(CHATTER)$(EXEC_POSTFIX)
	rm -f $@
	ln `pwd`/$< $@
	
$(EXEC_DIR)/$(MSGBOARD)$(EXEC_POSTFIX): $(MSGBOARD)$(EXEC_POSTFIX)
	rm -f $@
	ln `pwd`/$< $@

$(EXEC_DIR)/$(USRLOAD)$(EXEC_POSTFIX): $(USRLOAD)$(EXEC_POSTFIX)
	rm -f $@
	ln `pwd`/$< $@

$(DEPENDENCIES): $(DCPS_HDR)

-include $(DEPENDENCIES)
    