ifeq ($(JAVA_IS_PERC),yes)
default link all: 
	mvn -P perc,source package

doc:
	mvn -P perc,javadoc package

source:
	mvn -P perc,source package	
	
clean:
	mvn clean
else
default link all: 
	mvn -P jdk,source package

doc:
	mvn -P jdk,javadoc package

source:
	mvn -P jdk,source package	
	
clean:
	mvn clean
endif # INCLUDE_API_DCPS_C99