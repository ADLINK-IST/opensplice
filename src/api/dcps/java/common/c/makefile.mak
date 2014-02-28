#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_DLIB	:= $(DDS_DCPSSAJ)

ifeq ($(INCLUDE_API_DCPS_SAJ),yes)
JNI_CLASS_DIR := ../../../../saj/bld/$(SPLICE_TARGET)
else
ifeq ($(INCLUDE_API_DCPS_CJ), yes)
JNI_CLASS_DIR := ../../../../cj/bld/$(SPLICE_TARGET)
endif
endif

JNI_CLASS_DIR_IMPL = $(JNI_CLASS_DIR)/org/opensplice/dds/dcps
JNI_CLASS_DIR_DDS = $(JNI_CLASS_DIR)/DDS

JNI_CLASS := ConditionImpl.class
JNI_CLASS += ContentFilteredTopicImpl.class
JNI_CLASS += DataReaderImpl.class
JNI_CLASS += DataReaderViewImpl.class
JNI_CLASS += DataWriterImpl.class
JNI_CLASS += DomainImpl.class
JNI_CLASS += DomainParticipantImpl.class
JNI_CLASS += EntityImpl.class
JNI_CLASS += FooDataReaderImpl.class
JNI_CLASS += FooDataReaderViewImpl.class
JNI_CLASS += FooDataWriterImpl.class
JNI_CLASS += FooTypeSupportImpl.class
JNI_CLASS += MultiTopicImpl.class
JNI_CLASS += PublisherImpl.class
JNI_CLASS += QueryConditionImpl.class
JNI_CLASS += ReadConditionImpl.class
JNI_CLASS += StatusConditionImpl.class
JNI_CLASS += SubscriberImpl.class
JNI_CLASS += TopicDescriptionImpl.class
JNI_CLASS += TopicImpl.class
JNI_CLASS += TypeSupportImpl.class
JNI_CLASS += DomainParticipantFactoryImpl.class
JNI_CLASS += GuardCondition.class
JNI_CLASS += WaitSet.class
JNI_CLASS += ErrorInfo.class
JNI_CLASS += QosProvider.class

FULL_CLASSPATH="$(JNI_CLASS_DIR):$(JAVAH_INCLUDE)"

# $(JNI_H) is a dependency (see rules.mak)
JNI_H := $(subst .class,.h,$(addprefix saj_,$(subst Impl,,$(JNI_CLASS))))

include	$(OSPL_HOME)/setup/makefiles/target.mak

saj_%.h: $(JNI_CLASS_DIR_IMPL)/%Impl.class
	$(JAVAH) $(JAVAH_FLAGS) -o $@ -classpath $(FULL_CLASSPATH) -jni org.opensplice.dds.dcps.$(notdir $(subst .class,,$<))

saj_%.h: $(JNI_CLASS_DIR_DDS)/%.class
	$(JAVAH) $(JAVAH_FLAGS) -o $@ -classpath $(FULL_CLASSPATH) -jni DDS.$(notdir $(subst .class,,$<))

CPPFLAGS	+= -DOSPL_BUILD_DCPSSAJ
CFLAGS		+= $(SHCFLAGS) $(MTCFLAGS)
CINCS		+= -I$(OSPL_HOME)/src/api/dcps/common/include
CINCS		+= -I$(OSPL_HOME)/src/api/dcps/sac/include
CINCS		+= -I$(OSPL_HOME)/src/api/dcps/gapi/include
CINCS		+= -I$(OSPL_HOME)/src/database/database/include
CINCS		+= -I$(OSPL_HOME)/src/database/serialization/include
CINCS		+= -I$(OSPL_HOME)/src/kernel/include
CINCS		+= -I$(OSPL_HOME)/src/user/include
CINCS		+= -I$(OSPL_HOME)/src/tools/idlpp/include
CINCS		+= $(JAVA_INCLUDE)

LDFLAGS  += $(SHLDFLAGS)
LDLIBS   += -l$(DDS_CORE) $(SHLDLIBS)

-include $(DEPENDENCIES)

