JAR_MODULE	 = dcpscj.jar
JAR_LOCATION = $(OSPL_HOME)
JPACKAGES	 = DDS org/opensplice/dds/dcps
#TODO REMOVE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
JACORB_HOME  = /work/emiel/JacORB/v2.3.0/i686.Linux

all: precompile
link: jar

#step 1: Copy contents of saj/code directory to cj/code directory
#step 2: invoke jacorb to generate the builtintopics idl. This will overwrite any existing files in the code/ directory
#step 3: make link target
precompile:
	mkdir -p code
	cp -r ../../saj/java/code/* code/
    java -classpath "$(JACORB_HOME)/lib/idl.jar;$(JACORB_HOME)/lib/logkit-1.2.jar;$CLASSPATH" org.jacorb.idl.parser -d code -forceOverwrite code/dds_builtinTopics.idl
	(make link)

#old command:	$(JACORB_HOME)/bin/idl -d code -forceOverwrite code/dds_builtinTopics.idl

include $(OSPL_HOME)/setup/makefiles/target.mak

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE)
	@rm -rf code/*



