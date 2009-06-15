

$(DEPENDENCIES):
	mkdir -p code
	cp -r ../../saj/java/code/* code/
	$(JACORB_HOME)/bin/idl -d code -forceOverwrite code/dds_builtinTopics.idl
-include $(DEPENDENCIES)
