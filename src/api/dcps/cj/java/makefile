link: precompile

#step 1: Copy contents of saj/java/code directory to cj/java/bld/$SPLICE_TARGET/code directory
#step 2: Copy the makefile_cj to /bld/$SPLICE_TARGET directory and rename it to makefile
#step 3: invoke jacorb to generate the builtintopics idl. This will overwrite any existing files in the cj/java/bld/$SPLICE_TARGET/code/ directory
#step 4: go to the bld/$SPLICE_TARGET directory and invoke make link there, this will compile all java files and generate the jar file.
precompile:
	mkdir -p bld/$(SPLICE_TARGET)/code
	cp -r ../../saj/java/code/* bld/$(SPLICE_TARGET)/code/
	cp -r makefile_cj bld/$(SPLICE_TARGET)/makefile
	java -classpath $(JACORB_HOME)/lib/idl.jar:$(JACORB_HOME)/lib/logkit-1.2.jar:$(CLASSPATH) org.jacorb.idl.parser -d bld/$(SPLICE_TARGET)/code -forceOverwrite  bld/$(SPLICE_TARGET)/code/dds_builtinTopics.idl
	(cd bld/$(SPLICE_TARGET); make link)

include $(OSPL_HOME)/setup/makefiles/target.mak

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET)

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE)



