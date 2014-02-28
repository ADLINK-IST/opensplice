JAR_MODULE   =cmapi.jar
JAR_LOCATION =$(OSPL_HOME)
JAR_INC_DIR	 =$(OSPL_HOME)/jar/$(SPLICE_TARGET)
JPACKAGES =	org/opensplice/cm/data \
		org/opensplice/cm/meta \
		org/opensplice/cm/status \
		org/opensplice/cm/qos \
		org/opensplice/cm/qos/holder \
		org/opensplice/cm/statistics \
	 	org/opensplice/cm/transform \
		org/opensplice/cm/transform/xml \
		org/opensplice/cm/com \
                org/opensplice/cm \
                org/opensplice/cm/impl

all link: jar

include $(OSPL_HOME)/setup/makefiles/target.mak

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET) $(JAR_TARGET)/$(JAR_MODULE) manifest/$(SPLICE_TARGET)
