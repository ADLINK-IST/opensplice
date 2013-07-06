SUBSYSTEMS := common 

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

doc:
ifneq ($(STAF_HOME),)
	@java -jar "$(STAF_HOME)/services/stax/STAXDoc.jar" -verbose -d docs/api -sourcepath . common
else
	@echo "Failed: STAF_HOME not set, STAXDoc.jar not available"
endif

testdoc:
	@mkdir -p docs/testcases
	@doxygen common/testcases.doxygen
