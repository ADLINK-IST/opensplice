#
# Set subsystems to be processed
#

SUBSYSTEMS :=
SUBSYSTEMS += $(DDS_RRSTORAGE_SUBSYSTEM)
SUBSYSTEMS += xml

ifeq ($(INCLUDE_JAVA),yes)
SUBSYSTEMS += java jni
endif

include $(OSPL_HOME)/setup/makefiles/subsystem.mak
