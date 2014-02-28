# Set subsystems to be processed
#

SUBSYSTEMS	 := common
SUBSYSTEMS	 += boundsCheck
SUBSYSTEMS	 += invalidData
SUBSYSTEMS	 += builtin
SUBSYSTEMS	 += condition
SUBSYSTEMS	 += contentFilteredTopic
#SUBSYSTEMS	 += defaultConstructor ## Not yet relevant because default constructor is not yet implemented.
SUBSYSTEMS	 += domainParticipant
SUBSYSTEMS	 += domainParticipantFactory
SUBSYSTEMS	 += errorInfo
SUBSYSTEMS	 += getDiscoveredXxx
SUBSYSTEMS	 += histDataWithCondition
SUBSYSTEMS	 += listener
SUBSYSTEMS	 += publisher
SUBSYSTEMS	 += reader
SUBSYSTEMS	 += status
SUBSYSTEMS	 += subscriber
SUBSYSTEMS	 += topic
SUBSYSTEMS	 += typeSupport
SUBSYSTEMS	 += waitset
SUBSYSTEMS	 += writer
SUBSYSTEMS	 += SampleInfo
SUBSYSTEMS	 += qosProvider

include $(OSPL_HOME)/setup/makefiles/subsystem.mak

clean:
	rm -rf exec/$(SPLICE_TARGET)
