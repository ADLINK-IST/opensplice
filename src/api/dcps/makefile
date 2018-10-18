SUBSYSTEMS	:=

ifeq ($(INCLUDE_API_DCPS_SAC),yes)
SUBSYSTEMS	+= sac
endif # INCLUDE_API_DCPS_SAC
ifeq ($(INCLUDE_API_DCPS_C99),yes)
SUBSYSTEMS	+= c99
endif # INCLUDE_API_DCPS_C99
ifeq ($(INCLUDE_JAVA),yes)
SUBSYSTEMS	+= java
ifeq ($(INCLUDE_API_DCPS_SAJ5),yes)
SUBSYSTEMS	+= java5
endif #INCLUDE_API_DCPS_SAJ5
endif # INCLUDE_JAVA
ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DCPS_CCPP),yes)
SUBSYSTEMS 	+= c++/ccpp
endif # INCLUDE_API_DCPS_CCPP
ifeq ($(INCLUDE_API_DCPS_SACPP),yes)
SACPP_REQUIRED = yes
endif # INCLUDE_API_DCPS_SACPP
ifeq ($(INCLUDE_API_DCPS_ISOCPP),yes)
SACPP_REQUIRED = yes
endif # Dependency from isocpp on sacpp.
ifeq ($(SACPP_REQUIRED),yes)
SUBSYSTEMS 	+= c++/sacpp
endif 
ifeq ($(INCLUDE_API_DCPS_ISOCPP),yes)
SUBSYSTEMS 	+= isocpp
endif
ifeq ($(INCLUDE_API_DCPS_ISOCPP2),yes)
SUBSYSTEMS 	+= isocpp2
endif
endif # INCLUDE_CXX
ifeq ($(INCLUDE_CS),yes)
ifeq ($(INCLUDE_API_DCPS_SACS),yes)
SUBSYSTEMS      += sacs
endif # INCLUDE_API_DCPS_SACS
endif # INCLUDE_CS
SUBSYSTEMS	+= python
ifeq ($(INCLUDE_API_DCPS_NODEJS),yes)
SUBSYSTEMS      += nodejs
endif # INCLUDE_API_DCPS_NODEJS

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
