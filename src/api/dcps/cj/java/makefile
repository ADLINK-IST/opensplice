all link: precompile jar

JAR_MODULE	 = dcpscj.jar
JAR_LOCATION = $(OSPL_HOME)
JPACKAGES	 = DDS org/opensplice/dds/dcps

# the precompile rule will copy the code from the saj directory
# step 1: Copy contents of saj/java/code directory to cj/java/code directory
# step 2: invoke jacorb to generate the builtintopics idl
# step 3: the jar rule will compile this code producing the JAR_MODULE
precompile:
	mkdir -p code
	cp -r ../../saj/java/code/* code/
	java org.jacorb.idl.parser -d code -forceOverwrite code/dds_builtinTopics.idl

include $(OSPL_HOME)/setup/makefiles/target.mak

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET)

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE)

