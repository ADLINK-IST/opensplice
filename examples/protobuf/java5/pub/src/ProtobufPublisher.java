import java.util.concurrent.TimeoutException;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.status.PublicationMatchedStatus;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.Publisher;
import org.omg.dds.topic.Topic;

import address.Address.Organisation;
import address.Address.Person;
import address.Address.Person.PhoneNumber;
import address.Address.Person.PhoneType;

public class ProtobufPublisher {
    public static void main(String[] args) {
        ServiceEnvironment env;
        DomainParticipantFactory domainParticipantFactory;
        DomainParticipant participant;
        Topic<Person> topic;
        Publisher publisher;
        DataWriter<Person> writer;
        Person.Builder janeDoeBuilder;
        PhoneNumber phone;
        Person janeDoe;
        PolicyFactory policyFactory;

        System.setProperty(
                ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");

        env = ServiceEnvironment.createInstance(ProtobufPublisher.class
                .getClassLoader());

        policyFactory = PolicyFactory.getPolicyFactory(env);
        participant = null;
        domainParticipantFactory = DomainParticipantFactory.getInstance(env);

        try {
            participant = domainParticipantFactory.createParticipant();
            // Creating a Topic for a Protobuf class
            topic = participant.createTopic("Person", Person.class);

            // Creating a Publisher and DataWriter for the Protobuf Topic
            publisher = participant.createPublisher();
            writer = publisher.createDataWriter(
                    topic,
                    publisher.getDefaultDataWriterQos().withPolicy(
                            policyFactory.Reliability().withReliable()));

            waitForSubscriber(writer);
            // Creating a builder the Protobuf data structure
            janeDoeBuilder = Person.newBuilder();

            // Creating Jane Doe
            janeDoeBuilder.setName("Jane Doe")
                    .setEmail("jane.doe@somedomain.com").setAge(23);
            phone = PhoneNumber.newBuilder().setNumber("0123456789").build();
            janeDoeBuilder.addPhone(phone);
            Organisation.Builder orgBuilder = Organisation.newBuilder();
            orgBuilder.setName("Acme Corporation");
            orgBuilder.setAddress("Wayne Manor, Gotham City");
            orgBuilder.setPhone(PhoneNumber.newBuilder()
                    .setNumber("9876543210").setType(PhoneType.WORK));
            janeDoeBuilder.setWorksFor(orgBuilder);

            janeDoe = janeDoeBuilder.build();

            System.out.println("Publisher: publishing Person: "
                    + janeDoe.getName());
            writer.write(janeDoe);

            System.out.println("Publisher: sleeping for 5 seconds...");

            Thread.sleep(5000);

            System.out.println("Publisher: disposing Jane Doe...");

            // Disposing the DDS instance associated with the name field of the
            // Protobuf data structure which is the key in DDS
            writer.dispose(InstanceHandle.nilHandle(env), Person.newBuilder()
                    .setName("Jane Doe").setWorksFor(
                            Organisation.newBuilder().setName("Acme Corporation.").buildPartial()).buildPartial());

        } catch (TimeoutException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
        } catch (InterruptedException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
        } finally {
            System.out.println("Publisher: terminating...");

            if (participant != null) {
                participant.close();
            }
        }
    }

    public static void waitForSubscriber(DataWriter<Person> writer)
            throws InterruptedException {
        PublicationMatchedStatus matched;
        long millis = System.currentTimeMillis();
        long timeout = millis + (30 * 1000);
        boolean stop = false;

        System.out.println("Publisher: waiting for subscriber... ");

        do {
            matched = writer.getPublicationMatchedStatus();

            if (System.currentTimeMillis() > timeout) {
                stop = true;
            }
            if ((matched.getCurrentCount() == 0) && (!stop)) {
                Thread.sleep(500);
            }
        } while ((matched.getCurrentCount() == 0) && (!stop));

        if (matched.getCurrentCount() != 0) {
            System.out.println("Publisher: Subscriber found");
        } else {
            System.out.println("Publisher: Subscriber NOT found");
            throw new InterruptedException(
                    "Publisher: subscriber not detected within 30 seconds.");
        }
    }
}
