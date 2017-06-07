SUBSYSTEMS	:=

ifeq ($(INCLUDE_API_DCPS_SAC),yes)
SUBSYSTEMS	+= c
endif # INCLUDE_API_DCPS_SAC
ifeq ($(INCLUDE_JAVA),yes)
SUBSYSTEMS	+= java
ifeq ($(INCLUDE_API_DCPS_SAJ5),yes)
SUBSYSTEMS	+= java5
endif #INCLUDE_API_DCPS_SAJ5
endif # INCLUDE_JAVA
ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DCPS_SACPP),yes)
SACPP_REQUIRED = yes
endif # INCLUDE_API_DCPS_SACPP
ifeq ($(INCLUDE_API_DCPS_ISOCPP),yes)
SACPP_REQUIRED = yes
endif # Dependency from isocpp on sacpp.
ifeq ($(SACPP_REQUIRED),yes)
SUBSYSTEMS 	+= c++
endif 
ifeq ($(INCLUDE_API_DCPS_ISOCPP),yes)
SUBSYSTEMS 	+= isocpp
endif
endif # INCLUDE_CXX
ifeq ($(INCLUDE_CS),yes)
ifeq ($(INCLUDE_API_DCPS_SACS),yes)
SUBSYSTEMS      += cs
endif # INCLUDE_API_DCPS_SACS
endif # INCLUDE_CS

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
