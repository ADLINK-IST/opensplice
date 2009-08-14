#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_CSLIB = $(DDS_DCPSSAC)
CS_NAMESPCS	 = DDS DDS/OpenSplice DDS/OpenSplice/Gapi DDS/OpenSplice/Database DDS/OpenSplice/CustomMarshalers

all link: csc

include $(OSPL_HOME)/setup/makefiles/target.mak

CSLIBS       += -lSystem -lSystem.Data -lSystem.Xml
