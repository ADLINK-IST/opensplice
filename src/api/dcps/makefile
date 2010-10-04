SUBSYSTEMS	:=

ifeq ($(INCLUDE_API_DCPS_GAPI),yes)
SUBSYSTEMS	+= gapi
ifeq ($(INCLUDE_API_DCPS_SAC),yes)
SUBSYSTEMS	+= sac
endif # INCLUDE_API_DCPS_SAC
ifeq ($(INCLUDE_JAVA),yes)
ifeq ($(INCLUDE_API_DCPS_SAJ),yes)
SUBSYSTEMS	+= saj
ifeq ($(INCLUDE_API_DCPS_CJ),yes)
SUBSYSTEMS	+= cj
endif # INCLUDE_API_DCPS_CJ
endif # INCLUDE_API_DCPS_SAJ
endif # INCLUDE_JAVA
ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DCPS_CCPP),yes)
SUBSYSTEMS 	+= ccpp
endif # INCLUDE_API_DCPS_CCPP
ifeq ($(INCLUDE_API_DCPS_SACPP),yes)
SUBSYSTEMS 	+= sacpp
endif # INCLUDE_API_DCPS_SACPP
endif # INCLUDE_CXX
ifeq ($(INCLUDE_CS),yes)
ifeq ($(INCLUDE_API_DCPS_SACS),yes)
SUBSYSTEMS      += sacs
endif # INCLUDE_API_DCPS_SACS
endif # INCLUDE_CS
endif # INCLUDE_API_DCPS_GAPI

ifeq ($(INCLUDE_JAVA),yes)
    ifeq ($(INCLUDE_API_DCPS_JNI),yes)
SUBSYSTEMS	+= jni
endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
