.NOTPARALLEL:

JAR_MODULE	 =dcpsprotobuf.jar
JAR_LOCATION =$(OSPL_HOME)
JAR_INC_DIR	 =$(OSPL_HOME)/jar/$(SPLICE_TARGET)
JAVA_MAIN_CLASS =org.opensplice.dds.protobuf.CompilerPlugin
JCFLAGS_EXTRA	= -J-Xss768k

JPACKAGES=	com/prismtech/dds/protobuf \
			com/prismtech/dds/protobuf/backend \
			com/prismtech/dds/protobuf/backend/cafe \
			com/prismtech/dds/protobuf/backend/lite \
			com/prismtech/dds/protobuf/backend/ospl \
			com/prismtech/dds/protobuf/frontend \
			org/omg/dds/protobuf \
			com/google/protobuf \
			com/google/protobuf/compiler

PROTOBUF_HOME_NORMALIZED := $(shell $(OSPL_HOME)/bin/ospl_normalizePath $(PROTOBUF_HOME))
PROTO_DIR       =generated/$(SPLICE_TARGET)/code
PROTO_COMP_CODE =$(PROTO_DIR)/com/google/protobuf/compiler/PluginProtos.java
PROTO_DESC_CODE =$(PROTO_DIR)/com/google/protobuf/DescriptorProtos.java
PROTO_DDS_CODE  =$(PROTO_DIR)/org/omg/dds/protobuf/DescriptorProtos.java
PROTO_ETC_DIR   =$(OSPL_HOME)/etc/protobuf/SAJ5
PROTO_ETC_CODE  =$(PROTO_ETC_DIR)/DescriptorProtos.java
PROTO_GEN_CODE  =$(PROTO_COMP_CODE) $(PROTO_DDS_CODE) $(PROTO_ETC_CODE) $(PROTO_DESC_CODE)

LAUNCH4J_CONFIG =generated/$(SPLICE_TARGET)/launch4j_config.xml
PROTO_EXE       =$(OSPL_HOME)/exec/$(SPLICE_TARGET)/protoc-gen-ddsjava.exe

LAUNCH4J_CONFIG_CPP =generated/$(SPLICE_TARGET)/launch4j_config_cpp.xml
PROTO_EXE_CPP   	=$(OSPL_HOME)/exec/$(SPLICE_TARGET)/protoc-gen-ddscpp.exe

LICENSE_MGR = code/org/opensplice/dds/protobuf/LicenseMgr.java
JCODE_DIR = gpb/code
JCODE_PATH= gpb/code:$(PROTO_DIR):$(PROTOBUF_HOME)/java/src/main/java

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
all link: $(PROTO_GEN_CODE) jar
else
all link: $(PROTO_GEN_CODE) jar $(PROTO_EXE) $(PROTO_EXE_CPP)
endif

include $(OSPL_HOME)/setup/makefiles/target.mak

ifeq (,$(or $(findstring win32,$(SPLICE_HOST)), $(findstring win64,$(SPLICE_HOST))))
$(LAUNCH4J_CONFIG): launch4j_config_template.xml $(PROTO_DIR)/.STAMP
	sed "s@#OSPL_HOME#@$(OSPL_HOME)@;s@#SPLICE_TARGET#@$(SPLICE_TARGET)@" < launch4j_config_template.xml > $(LAUNCH4J_CONFIG)

$(LAUNCH4J_CONFIG_CPP): launch4j_config_template_cpp.xml $(PROTO_DIR)/.STAMP
	sed "s@#OSPL_HOME#@$(OSPL_HOME)@;s@#SPLICE_TARGET#@$(SPLICE_TARGET)@" < launch4j_config_template_cpp.xml > $(LAUNCH4J_CONFIG_CPP)
else
WIN_OSPL_HOME=$(shell cygpath -m $(OSPL_HOME))
$(LAUNCH4J_CONFIG): launch4j_config_template.xml $(PROTO_DIR)/.STAMP
	sed "s@#OSPL_HOME#@$(WIN_OSPL_HOME)@;s@#SPLICE_TARGET#@$(SPLICE_TARGET)@" < launch4j_config_template.xml > $(LAUNCH4J_CONFIG)

$(LAUNCH4J_CONFIG_CPP): launch4j_config_template_cpp.xml $(PROTO_DIR)/.STAMP
	sed "s@#OSPL_HOME#@$(WIN_OSPL_HOME)@;s@#SPLICE_TARGET#@$(SPLICE_TARGET)@" < launch4j_config_template_cpp.xml > $(LAUNCH4J_CONFIG_CPP)
endif

$(PROTO_EXE): $(LAUNCH4J_CONFIG) jar
	launch4jc $(LAUNCH4J_CONFIG)

$(PROTO_EXE_CPP): $(LAUNCH4J_CONFIG_CPP) jar
	launch4jc $(LAUNCH4J_CONFIG_CPP)

$(PROTO_DESC_CODE): $(PROTO_DIR)/.STAMP $(PROTOBUF_HOME)/src/google/protobuf/descriptor.proto
	protoc --java_out=$(PROTO_DIR) --proto_path="$(PROTOBUF_HOME_NORMALIZED)/src" "$(PROTOBUF_HOME_NORMALIZED)/src/google/protobuf/descriptor.proto"

$(PROTO_COMP_CODE): $(PROTO_DIR)/.STAMP
	protoc --java_out=$(PROTO_DIR) --proto_path="$(PROTOBUF_HOME_NORMALIZED)/src" "$(PROTOBUF_HOME_NORMALIZED)/src/google/protobuf/compiler/plugin.proto"

$(PROTO_DDS_CODE): $(PROTO_DIR)/.STAMP
	protoc --java_out=$(PROTO_DIR) --proto_path="$(PROTOBUF_HOME_NORMALIZED)/src" --proto_path="$(OSPL_HOME_NORMALIZED)/src/tools/protobuf/protos" "$(OSPL_HOME_NORMALIZED)/src/tools/protobuf/protos/omg/dds/descriptor.proto"

$(PROTO_DIR)/.STAMP:
	@mkdir -p $(PROTO_DIR)
	@touch $@
	@mkdir -p $(OSPL_HOME)/etc/protobuf
	cp -R gpb/templates/* $(OSPL_HOME)/etc/protobuf/
	

$(PROTO_ETC_CODE): $(PROTO_ETC_DIR)/.STAMP $(PROTO_DDS_CODE)
	cp $(PROTO_DDS_CODE) $(PROTO_ETC_CODE)

$(PROTO_ETC_DIR)/.STAMP:
	@mkdir -p $(PROTO_ETC_DIR)
	@touch $@

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET) generated/$(SPLICE_TARGET)
	@rm -f $(OSPL_HOME)/exec/$(SPLICE_TARGET)/protoc-gen-ddsjava.exe
	@rm -f $(LICENSE_MGR)
	@if [ -d bld ] && [ `find bld/* -maxdepth 0 -type d 2> /dev/null | wc -l` -eq 0 ] ; then \
		echo 1 ;\
		rm -rf $(OSPL_HOME)/etc/protobuf ; \
	fi

run: jar
	$(JAVA) -jar $(JAR_TARGET)/$(JAR_MODULE)
