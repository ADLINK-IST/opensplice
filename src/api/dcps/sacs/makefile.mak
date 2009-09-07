#
# included by bld/$(SPLICE_TARGET)/makefile

TARGET_CSLIB = $(DDS_DCPSSACS)
CS_NAMESPCS	 = DDS DDS/OpenSplice DDS/OpenSplice/Gapi DDS/OpenSplice/Database DDS/OpenSplice/CustomMarshalers

all link: csc

include $(OSPL_HOME)/setup/makefiles/target.mak

CSLIBS       += /r:System.dll /r:System.Data.dll /r:System.Xml.dll
