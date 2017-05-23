.NOTPARALLEL:

JAR_MODULE	 = cmdataadapter.jar
JAR_LOCATION = $(OSPL_HOME)
JAR_INC_DIR	 = $(OSPL_HOME)/jar/$(SPLICE_TARGET)

JAVA_INC	= $(JAR_INC_DIR)/cmapi.jar:$(JAR_INC_DIR)/dcpsprotobuf.jar


JPACKAGES=	org/opensplice/cmdataadapter \
		org/opensplice/cmdataadapter/protobuf

PROTOBUF_IMPL_FILE =code/org/opensplice/cmdataadapter/protobuf/ProtobufDataAdapterImpl.java
ifeq "$(INCLUDE_TOOLS_PROTOBUF)" "yes"
PROTOBUF_IMPL_TMPL =.tmpl
else
PROTOBUF_IMPL_TMPL =.dummy
endif

all link: $(PROTOBUF_IMPL_FILE) jar

 $(PROTOBUF_IMPL_FILE):
	cat $(PROTOBUF_IMPL_FILE)$(PROTOBUF_IMPL_TMPL) > $(PROTOBUF_IMPL_FILE)

include $(OSPL_HOME)/setup/makefiles/target.mak



clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET) $(PROTOBUF_IMPL_FILE)


