JAR_MODULE		= dcpssaj.jar
JAR_LOCATION 		= $(OSPL_HOME)

TOPICIDL		= $(OSPL_HOME)/etc/idl/dds_builtinTopics.idl
IDL_CODE_DIR	= bld/code/$(SPLICE_TARGET)
JTIMESTAMP		= bld/code/$(SPLICE_TARGET)/.java_idl.timestamp

JPACKAGES	= DDS org/opensplice/dds/dcps
JCODE_DIR   = code

JCODE_PATH = $(JCODE_DIR)$(JAVA_SRCPATH_SEP)$(IDL_CODE_DIR)

IDLPP		= $(WINCMD) idlpp

all link: idlpp $(JTIMESTAMP) jar

include $(OSPL_HOME)/setup/makefiles/target.mak

$(JTIMESTAMP): $(TOPICIDL)
	@mkdir -p $(IDL_CODE_DIR) $(CLASS_DIR)
	@rm -rf $(IDL_CODE_DIR)/* $(CLASS_DIR)/*
	$(IDLPP) -l java -S -d $(IDL_CODE_DIR) $(TOPICIDL)
	@touch $(JTIMESTAMP)
	$(JCC) $(JFLAGS) -d $(CLASS_DIR) $(JCFLAGS) $(IDL_CODE_DIR)/DDS/*.java

idlpp:
	cd $(OSPL_HOME)/src/cpp; make
	cd $(OSPL_HOME)/src/tools/cppgen; make
	cd $(OSPL_HOME)/src/tools/idlpp; make

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) $(IDL_CODE_DIR) 
