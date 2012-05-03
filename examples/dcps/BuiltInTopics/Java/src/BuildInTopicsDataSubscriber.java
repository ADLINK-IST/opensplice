

import java.util.HashMap;

import DDS.*;

/**
 * This examples application monitors the number of nodes that participate in a
 * DDS domain.
 * 
 */
public class BuildInTopicsDataSubscriber {
	public static void main(String[] args) {
		
		boolean automatic = true;
		  if (args.length > 1)
		  {
		    automatic = (args[0].equals("true"));
		  }
		
		String domainId;
		String hostName;

		/* Resolve the singleton DomainParticipantFactory. */
		DomainParticipantFactory dpf = DomainParticipantFactory.get_instance();

		/* Optionally use the domainId provided as input parameter. */
		if (args.length > 0) {
			domainId = args[0];
		} else {
			domainId = System.getenv("OSPL_URI");
		}

		/* Create a DomainParticipant with default QoS and no listener. */
		DomainParticipant participant = dpf.create_participant(domainId,
				PARTICIPANT_QOS_DEFAULT.value, null, STATUS_MASK_NONE.value);

		/* Check whether the participant has been created. */
		if (participant == null) {
			System.err.println("Could not connect to domain '" + domainId
					+ "'. Is OpenSplice running?");
			return;
		} else {
			System.out.println("Connected to domain '" + domainId + "'.");
		}

		/* Resolve the built-in Subscriber. */
		Subscriber builtinSubscriber = participant.get_builtin_subscriber();

		/* Lookup the DataReader for the DCPSParticipant built-in Topic. */
		DataReader reader = builtinSubscriber
				.lookup_datareader("DCPSParticipant");

		/* Safely cast the DataReader to a ParticipantBuiltinTopicDataReader. */
		ParticipantBuiltinTopicDataDataReader participantReader = ParticipantBuiltinTopicDataDataReaderHelper
				.narrow(reader);

		/* Allocate a new typed holder for data samples. */
		ParticipantBuiltinTopicDataSeqHolder data = new ParticipantBuiltinTopicDataSeqHolder();

		/* Allocate a new holder for sample infos. */
		SampleInfoSeqHolder info = new SampleInfoSeqHolder();

		System.out.print("Waiting for historical data... ");

		/* Make sure all historical data is delivered in the DataReader. */
		participantReader.wait_for_historical_data(DURATION_INFINITE.value);

		System.out.println("done");

		/* Allocate a new Waitset */
		WaitSet waitset = new WaitSet();

		/* Create a new ReadCondition for the reader that matches all samples. */
		ReadCondition condition = participantReader.create_readcondition(
				ANY_SAMPLE_STATE.value, ANY_VIEW_STATE.value,
				ANY_INSTANCE_STATE.value);

		/* Attach the condition to the waitset. */
		waitset.attach_condition(condition);

		/* Allocate a holder for conditions. */
		ConditionSeqHolder conditions = new ConditionSeqHolder();

		/* Allocate a map to store node information later on. */
		/* The key of the map is the id of the node and the value is the */
		/* number of active participants on that node. */
		HashMap<Integer, Integer> nodes = new HashMap<Integer, Integer>();

		/* Allocate a map to store node information later on. */
		/* The key of the map is the id of the node and the value is the */
		/* name of the node. */
		HashMap<Integer, String> nodeNames = new HashMap<Integer, String>();

		/*
		 * Block the current thread until the attached condition becomes true or
		 * the user interrupts.
		 */
		int returnCode = waitset._wait(conditions, DURATION_INFINITE.value);
		boolean done = false;
		/* Continue processing until interrupted. */
		while ( ! done) {

			/* Take all available data from the reader. */
			returnCode = participantReader.take(data, info,
					LENGTH_UNLIMITED.value, ANY_SAMPLE_STATE.value,
					ANY_VIEW_STATE.value, ANY_INSTANCE_STATE.value);
			
			/* Verify that data has been taken. */
			if (returnCode == RETCODE_OK.value) {

				/* Iterate the list of taken samples. */
				for (int i = 0; i < data.value.length; i++) {
					/* Resolve the node identification. */
					int nodeId = data.value[i].key[0];

					/* Initialise the number of participants for a node. */
					int participantCount = 0;

					/* Check if we saw a participant for the node before. */
					if (nodes.containsKey(nodeId)) {
						/*
						 * Resolve the actual number of participants on the
						 * node.
						 */
						participantCount = nodes.get(nodeId);
					}

					/* Check sample info to see whether the instance is ALIVE. */
					if (info.value[i].instance_state == ALIVE_INSTANCE_STATE.value) {
						/*
						 * The splicedaemon publishes the host-name in the
						 * user_data field.
						 */
						if (data.value[i].user_data.value.length != 0) {
							hostName = new String(data.value[i].user_data.value);
							nodeNames.put(nodeId, hostName);
						} else {
							hostName = null;
						}

						/* Increase the number of participants. */
						participantCount++;

						/* Update the number of participants for the node. */
						nodes.put(nodeId, participantCount);

						/* If it's the first participant, report the node is up. */
						if (participantCount == 1) {
							System.out.println("Node '" + nodeId
									+ "' started (Total nodes running: "
									+ nodes.size() + ")");
						}
						if (hostName != null) {
							System.out.println("Hostname for node '" + nodeId
									+ " is '" + hostName + "'.");
						}
					} else {
						/* Decrease the number of participants. */
						participantCount--;

						/*
						 * If no more participants exist, report the node is
						 * down.
						 */
						if (participantCount == 0) {
							hostName = nodeNames.get(nodeId);
							nodeNames.remove(nodeId);
							nodes.remove(nodeId);

							if (hostName != null) {
								System.out.println("Node " + nodeId + " ("
										+ hostName
										+ ") stopped (Total nodes running: "
										+ nodes.size() + ")");
							} else {
								System.out.println("Node " + nodeId
										+ " stopped (Total nodes running: "
										+ nodes.size() + ")");
							}
						} else if (participantCount > 0) {
							nodes.put(nodeId, participantCount);
						}
					}

				}
			}
			/* Indicate to reader that data/info is no longer accessed. */
			participantReader.return_loan(data, info);
            
			if (!automatic) {
			      /* Block the current thread until the attached condition becomes 
			       * true or the user interrupts.
			       */
			      System.out.println("=== [BuiltInTopicsDataSubscriber] Waiting ... ");
			      returnCode = waitset._wait(conditions, DURATION_INFINITE.value);
			      done = (returnCode != RETCODE_OK.value);
			      } else {
			      done = true;
			      }
		}
		/* Recursively delete all entities in the DomainParticipant. */
		participant.delete_contained_entities();
		/* Delete DomainParticipant. */
		dpf.delete_participant(participant);

	}

}
