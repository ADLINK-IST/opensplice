# included by bld/$(SPLICE_HOST)/makefile

TARGET_EXEC     := fwctrl

include	$(OSPL_HOME)/setup/makefiles/target.mak

#CINCS += -I$(OSPL_HOME)/src/api/dcps/sac/include
#CINCS += -I$(OSPL_OUTER_HOME)/testsuite/dbt/services/durability2/durability_multinode/common/include
#CINCS += -I$(OSPL_HOME)/src/database/database/include
#CINCS += -I$(OSPL_HOME)/src/kernel/include


#-l$(DDS_KERNEL)
#LDLIBS += -lmptestcommon

-include $(DEPENDENCIES)
