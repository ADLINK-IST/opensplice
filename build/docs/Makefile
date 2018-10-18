
MODULE    = Documentation
PRE_RULES = xml generate
COMPS = NETWORK_CHANNELS BANDWIDTH_LIMITING NETWORK_PARTITIONS ENCRYPTION SSL

.PHONY: generate
generate:
	$(MAKE) -C ConfGuide html

	$(MAKE) -C GettingStartedGuide html
	$(MAKE) -C GettingStartedGuide latexpdf

	$(MAKE) -C RMIGettingStarted html
	$(MAKE) -C RMIGettingStarted latexpdf

	$(MAKE) -C GPBTutorial html
	$(MAKE) -C GPBTutorial latexpdf

	$(MAKE) -C DDSTutorial html
	$(MAKE) -C DDSTutorial latexpdf

	$(MAKE) -C EvaluationGuide html
	$(MAKE) -C EvaluationGuide latexpdf

	$(MAKE) -C NodeMonitorGuide html
	$(MAKE) -C NodeMonitorGuide latexpdf

	$(MAKE) -C DeploymentGuide html
	$(MAKE) -C DeploymentGuide latexpdf

	$(MAKE) -C IDLPreProcGuide html
	$(MAKE) -C IDLPreProcGuide latexpdf

	$(MAKE) -C RnRAPIReference html
	$(MAKE) -C RnRAPIReference latexpdf

	$(MAKE) -C TesterUserGuide html
	$(MAKE) -C TesterUserGuide latexpdf

	$(MAKE) -C RnRManagerGuide html
	$(MAKE) -C RnRManagerGuide latexpdf

	$(MAKE) -C StreamsAPIReference html
	$(MAKE) -C StreamsAPIReference latexpdf

	$(MAKE) -C ModelingGuide html
	$(MAKE) -C ModelingGuide latexpdf

	$(MAKE) -C DDSMATLABGuide html
	$(MAKE) -C DDSMATLABGuide latexpdf

	$(MAKE) -C DDSSimulinkGuide html
	$(MAKE) -C DDSSimulinkGuide latexpdf

	$(MAKE) -C DDSLabVIEWGuide html
	$(MAKE) -C DDSLabVIEWGuide latexpdf

	$(MAKE) -C PythonDCPSAPIGuide html
	$(MAKE) -C PythonDCPSAPIGuide latexpdf

	$(MAKE) -C NodeJSDCPSAPIGuide html
	$(MAKE) -C NodeJSDCPSAPIGuide latexpdf

docclean:
	$(MAKE) -C ConfGuide clean
	$(MAKE) -C GettingStartedGuide clean
	$(MAKE) -C RMIGettingStarted clean
	$(MAKE) -C GPBTutorial clean
	$(MAKE) -C DDSTutorial clean
	$(MAKE) -C EvaluationGuide clean
	$(MAKE) -C NodeMonitorGuide clean
	$(MAKE) -C DeploymentGuide clean
	$(MAKE) -C IDLPreProcGuide clean
	$(MAKE) -C RnRAPIReference clean
	$(MAKE) -C TesterUserGuide clean
	$(MAKE) -C RnRManagerGuide clean
	$(MAKE) -C StreamsAPIReference clean
	$(MAKE) -C ModelingGuide clean
	$(MAKE) -C DDSMATLABGuide clean
	$(MAKE) -C DDSSimulinkGuide clean
	$(MAKE) -C DDSLabVIEWGuide clean
	$(MAKE) -C PythonDCPSAPIGuide clean
	$(MAKE) -C NodeJSDCPSAPIGuide clean

	rm -rf configuide/source/ddsi2e.xml

.PHONY: xml
xml: ConfGuide/source/config.xml

ConfGuide/source/config.xml: $(OSPL_HOME)/src/tools/cm/config/code/splice_metaconfig_6.1.xml
	$(OSPL_HOME)/build/docs/extract.py $(OSPL_HOME)/src/tools/cm/config/code/splice_metaconfig_6.1.xml osplconf  
	mv osplconf.xml ConfGuide/source
