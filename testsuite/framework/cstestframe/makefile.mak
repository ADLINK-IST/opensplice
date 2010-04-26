# included by bld/$(SPLICE_HOST)/makefile

TARGET_CSLIB    := testframe
CS_NAMESPCS	 = test/framework
TARGET_LINK_DIR := $(OSPL_HOME)/testsuite/lib/$(SPLICE_TARGET)

all link: csc

include $(OSPL_HOME)/setup/makefiles/target.mak

#CSLIBS       += -r:System.dll -r:System.Data.dll -r:System.Xml.dll
