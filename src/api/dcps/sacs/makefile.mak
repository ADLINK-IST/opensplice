#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_CSLIB = dcpssacs
CS_NAMESPCS	 = DDS DDS/OpenSplice DDS/OpenSplice/Gapi DDS/OpenSplice/CustomMarshalers

all link: csc

include $(OSPL_HOME)/setup/makefiles/target.mak
