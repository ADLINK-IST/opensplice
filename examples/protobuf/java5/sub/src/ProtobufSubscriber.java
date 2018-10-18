import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Sample.Iterator;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.topic.Topic;

import address.Address.Person;
import address.Address.Person.PhoneNumber;

public class ProtobufSubscriber {
    public static void main(String[] args) {
        ServiceEnvironment env;
        DomainParticipantFactory domainParticipantFactory;
        DomainParticipant participant;
        Topic<Person> topic;
        DataReader<Person> reader;
        Subscriber subscriber;
        WaitSet ws;
        PolicyFactory policyFactory;
        int expectedUpdates = 2;

        System.setProperty(
                ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");

        env = ServiceEnvironment.createInstance(ProtobufSubscriber.class
                .getClassLoader());

        policyFactory = PolicyFactory.getPolicyFactory(env);
        participant = null;
        domainParticipantFactory = DomainParticipantFactory.getInstance(env);

        try {
            participant = domainParticipantFactory.createParticipant();
            // Creating a Topic for a Protobuf class
            topic = participant.createTopic("Person", Person.class);

            // Creating a Subscriber and DataReader for the Protobuf Topic
            subscriber = participant.createSubscriber();
            reader = subscriber.createDataReader(
                    topic,
                    subscriber.getDefaultDataReaderQos().withPolicy(
                            policyFactory.Reliability().withReliable()));

            // Creating a WaitSet to block for incoming samples

            ws = env.getSPI().newWaitSet();
            ws.attachCondition(reader.createReadCondition(subscriber
                    .createDataState().withAnyViewState()
                    .withAnyInstanceState().withAnySampleState()));

            System.out.println("Subscriber: waiting for incoming samples...");

            do {
                // Waiting for data to become available
                ws.waitForConditions(30, TimeUnit.SECONDS);

                // Take all data and print it to the screen
                expectedUpdates -= printAllData(reader);
            } while (expectedUpdates > 0);

        } catch (RuntimeException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
        } catch (TimeoutException e) {
            System.out
                    .println("Subscriber: time-out while waiting for updates.");
        } finally {
            System.out.println("Subscriber: terminating...");

            if (participant != null) {
                participant.close();
            }
        }
    }

    public static int printAllData(DataReader<Person> reader) {
        Iterator<Person> iter;
        Sample<Person> sample;
        Person data;
        String states;
        int sampleCount = 0;

        iter = reader.take();

        while (iter.hasNext()) {
            sample = iter.next();
            sampleCount++;

            states = "(" + sample.getSampleState() + ", "
                    + sample.getViewState() + ", " + sample.getInstanceState()
                    + ")";

            if (sample.getData() != null) {
                System.out
                        .println("Subscriber: reading sample " + states + ":");
                printPerson(sample.getData(), "");
            } else {
                data = ((org.opensplice.dds.sub.Sample<Person>) sample)
                        .getKeyValue();
                System.out.println("Subscriber: reading invalid sample "
                        + states + ":");
                System.out.println("- Name  = " + data.getName());
                System.out.println("   - Company    =");
                System.out.println("      - Name    = "
                        + data.getWorksFor().getName());
            }
        }
        return sampleCount;
    }

    public static void printPerson(Person person, String tabs) {

        System.out.println(tabs + "- Name       = " + person.getName());
        System.out.println(tabs + "- Age        = " + person.getAge());
        System.out.println(tabs + "- Email      = " + person.getEmail());

        for (PhoneNumber phone : person.getPhoneList()) {
            System.out.println(tabs + "- Phone      = " + phone.getNumber()
                    + " ("
                    + phone.getType() + ")");

        }
        System.out.println(tabs + "- Company    =");
        System.out.println(tabs + "   - Name    = "
                + person.getWorksFor().getName());
        System.out.println(tabs + "   - Address = "
                + person.getWorksFor().getAddress());

        if (person.getWorksFor().hasPhone()) {
            System.out.println(tabs + "   - Phone   = "
                    + person.getWorksFor().getPhone().getNumber() + " ("
                    + person.getWorksFor().getPhone().getType() + ")");
        } else {
            System.out.println(tabs + "   - Phone   = ");
        }
    }
}
