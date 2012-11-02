# included by bld/$(SPLICE_HOST)/makefile

TARGET_CSLIB    := cstestframe
CS_NAMESPCS	= test/framework
TARGET_LINK_DIR = $(OSPL_HOME)/testsuite/lib/$(SPLICE_TARGET)

include $(OSPL_HOME)/setup/makefiles/target.mak
