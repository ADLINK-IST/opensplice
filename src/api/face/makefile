#
# Set subsystems to be processed
#
SUBSYSTEMS	 :=

ifeq ($(INCLUDE_JAVA),yes)
ifeq ($(INCLUDE_API_DCPS_SAJ5),yes)
SUBSYSTEMS	 += java
endif
endif

ifeq ($(INCLUDE_CXX),yes)
ifeq ($(INCLUDE_API_DCPS_ISOCPP2),yes)
SUBSYSTEMS	 += cpp
endif
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
