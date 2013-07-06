JAR_MODULE     = ospldcg.jar
JAR_LOCATION	= $(OSPL_HOME)
JAR_INC_DIR	   = $(OSPL_HOME)/jar/$(SPLICE_TARGET)

JAVA_INC	= $(OPENORB_HOME)/openorb_orb-1.4.0e.jar:$(OPENORB_HOME)/openorb_orb_tools-1.4.0e.jar:$(OSPL_HOME)/jar/flexlm.jar:$(OSPL_HOME)/jar/EccpressoAll.jar

JAVA_MAIN_CLASS =DCG.Control.DCGStarter

JPACKAGES       = DCG
JPACKAGES       += DCG/DCGUtilities
JPACKAGES       += DCG/Core
JPACKAGES       += DCG/FrontendXML
JPACKAGES       += DCG/FrontendIDL
JPACKAGES       += DCG/BackendDLRL
JPACKAGES       += DCG/Control

OPENORB_JAR=openorb_orb-1.4.0e.jar
OPENORB_TOOLS_JAR=openorb_orb_tools-1.4.0e.jar
OPENORB=$(OSPL_HOME)/jar/$(SPLICE_TARGET)/$(OPENORB_JAR)
OPENORB_TOOLS=$(OSPL_HOME)/jar/$(SPLICE_TARGET)/$(OPENORB_TOOLS_JAR)
ORB=$(OPENORB) $(OPENORB_TOOLS)

all link: $(ORB) jar

$(ORB): $(ESCAPED_OPENORB_HOME)/$(OPENORB_JAR) $(ESCAPED_OPENORB_HOME)/$(OPENORB_TOOLS_JAR)
	@cp "$(OPENORB_HOME)/$(OPENORB_JAR)" $(OPENORB)
	@cp "$(OPENORB_HOME)/$(OPENORB_TOOLS_JAR)" $(OPENORB_TOOLS)

include $(OSPL_HOME)/setup/makefiles/target.mak


analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE)
	@rm -rf manifest/$(SPLICE_TARGET)
	@rm -rf $(ORB)
