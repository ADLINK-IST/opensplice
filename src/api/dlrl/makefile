#
# Set subsystems to be processed
#
ifeq ($(INCLUDE_API_DLRL_SAJ),yes)
ifeq ($(INCLUDE_JAVA),yes)
SUBSYSTEMS	 += saj
endif
endif
ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DLRL_CCPP),yes)
SUBSYSTEMS 	+= ccpp
endif # INCLUDE_API_DLRL_CCPP
ifeq ($(INCLUDE_API_DLRL_SACPP),yes)
SUBSYSTEMS 	+= sacpp
endif # INCLUDE_API_DLRL_SACPP

endif # INCLUDE_CXX

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
