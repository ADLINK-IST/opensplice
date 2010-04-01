CODE_DIR	?= ../../code
IDLPP		:= idlpp
IDLPP_FLAGS := 

# IDL		:= $(ORB_COMPILER)
# IDL_FLAGS	:= $(ORB_CXXFLAGS)
IDL_FLAGS	+= -I$(OSPL_HOME)/src/api/dcps/corbac++/code -I../../code

IDL_FILES	= $(notdir $(wildcard $(CODE_DIR)/*.idl))

.PRECIOUS: %C.cpp %Dcps.cpp %SplDcps.cpp

IDL_C_H		 = $(IDL_FILES:%.idl=%C.h) $(IDL_FILES:%.idl=%C.i)
IDL_C_CPP	 = $(IDL_FILES:%.idl=%C.cpp)
IDLPP_H		 = $(IDL_FILES:%.idl=%.h) $(IDL_FILES:%.idl=%Dcps.h) $(IDL_FILES:%.idl=%SplDcps.h)
IDLPP_CPP	 = $(IDL_FILES:%.idl=%Dcps.cpp) $(IDL_FILES:%.idl=%SplDcps.cpp)
IDL_H		 = $(IDL_C_H) $(IDLPP_H)
IDL_C		 = $(IDL_C_CPP) $(IDLPP_CPP)

IDL_O		 = $(IDL_C:%.cpp=%$(OBJ_POSTFIX))


	
%C.h %C.i %C.cpp %C.inl : %.idl
#	$(IDL) $(IDL_FLAGS) $<
	$(ORB_COMPILER) $(ORB_CXXFLAGS) $<

%.h %Dcps.h %SplDcps.h %Dcps.cpp %SplDcps.cpp : %.idl
	$(IDLPP) $<
