#ifndef OSPL_TESTS_CPP_PUBLISHER_H
#define OSPL_TESTS_CPP_PUBLISHER_H

#include "ccpp_dds1578.h"
#include "ccpp_dds_dcps.h"

#include "testlibs/CPPTestProcess.h"

#include "cm/cmx_participant.h"
#include "cm/cmx_factory.h"
#include "cm/cmx_entity.h"

#include "os_stdlib.h"

char *getNetworkEntity()
{
   char *participants;
   char *netname;
   char *entity;
   char *next;

   participants = cmx_participantAllParticipants
      (cmx_participantNew (os_getenv ("OSPL_URI"), 0, "testParticipant", NULL));
   netname = strstr (participants, "<name>networking</name>");
   if (netname == NULL)
   {
      printf ("Networking service not found\n");
      return NULL;
   }

   entity = strstr (participants, "<entity>");
   if (netname < entity)
   {
      printf ("Entity list parsing failed\n");
      return NULL;
   }

   while (1)
   {
      next = strstr (entity + 1, "<entity>");
      if (next == NULL)
      {
         return strdup (entity);
      }

      if (next > netname)
      {
         // strndup is not available on win32 and others
         size_t n = next - entity;
         size_t nAvail;
         char *p;

         if ( !entity )
            return 0;

         nAvail = (strlen(entity) + 1 < n + 1 ? strlen(entity) + 1 : n + 1);
         p = (char*) malloc (nAvail);
         memcpy (p, entity, nAvail);
         p[nAvail - 1] = '\0';
         return p;
      }

      entity = next;
   }
}

long getBytesBeforeCompression (const char *stats)
{
   long result = 0;
   const char *pos = strstr (stats, "<nofBytesBeforeCompression>");
   if (sscanf (pos, "<nofBytesBeforeCompression>%ld</nofBytesBeforeCompression>", &result) != 1)
   {
      printf ("Parse failure : nofBytesBeforeCompression\n");
   }
   return result;
}

long getBytesAfterCompression (const char *stats)
{
   long result = 0;
   const char *pos = strstr (stats, "<nofBytesAfterCompression>");
   if (sscanf (pos, "<nofBytesAfterCompression>%ld</nofBytesAfterCompression>", &result) != 1)
   {
      printf ("Parse failure : nofBytesAfterCompression\n");
   }
   return result;
}

unsigned int sum (const char *line)
{
    int i;
    unsigned int result = 0;
    for (i = 0; line[i]; i++)
    {
       result += line[i];
    }
    return result;
}

/**
* @brief A very simple publish process for a very very simple
* publish / subscribe test.
* The publisher publishes a sample. The subscriber reads it.
*/
class CPPPublisherProcess
  : public virtual OSPLTestLib::CPPTestProcess
{
    public:
        CPPPublisherProcess (const char* the_name) :
          CPPTestProcess (the_name) {}

         virtual ~CPPPublisherProcess () {};
    /**
    * @brief test process initialisation
    * Uncomment and doc *iff* required.
    */
    // virtual void init (int argc, char *argv[])
    //  {
    //    OSPLTestLib::CPPTestProcess::init (argc, argv);
    //
    //    // insert any initialisation you require here.
    //}

    /**
    * @brief the test procedure
    * The test creates a simplest possible writer with all default QoS.
    * Waits until notified to start by the other process.
    * Writes a sample and waits to be shutdown by the other process.
    */
    virtual int run ()
    {
        DDS::ReturnCode_t result;
        DDS::TopicQos tQos;
        DDS::PublisherQos pQos;
        DDS::InstanceHandle_t userHandle;
        const char *networkxml, *netstats;
        long bytesBefore, bytesAfter;
        char nextline[2048], newline[2];
        int scanres;

        ::DDS::DomainParticipantFactory_var factory
            = ::DDS::DomainParticipantFactory::get_instance ();

        ::DDS::DomainParticipant_var domain_participant
                = factory->create_participant (this->default_domain_id_,
                                              PARTICIPANT_QOS_DEFAULT,
                                              ::DDS::DomainParticipantListener::_nil (),
                                              0UL);


        if (::DDS::is_nil (domain_participant.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil domain participant - daemon probably not running.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::net1578::paragraphTypeSupport para_type_support;

        result = para_type_support.register_type (domain_participant.in (),
                                                  para_type_support.get_type_name ());

        if (result != ::DDS::RETCODE_OK)
        {
            fprintf (stderr, "ERROR %s (%s - %d): Register type failed.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        domain_participant->get_default_topic_qos (tQos);
        tQos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
        tQos.history.depth = 250;
        ::DDS::Topic_var para_topic
                = domain_participant->create_topic ("net1578_paragraph",
                                                     para_type_support.get_type_name (),
                                                     tQos,
                                                     ::DDS::TopicListener::_nil (),
                                                     0U);

        if (::DDS::is_nil (para_topic.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil topic.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        domain_participant->get_default_publisher_qos (pQos);
        pQos.partition.name.length (1);
        pQos.partition.name[0] = CORBA::string_dup ("net1578");
        ::DDS::Publisher_var para_publisher
                = domain_participant->create_publisher (pQos,
                                                        ::DDS::PublisherListener::_nil (),
                                                        0U);
        if (::DDS::is_nil (para_publisher.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil publisher.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::DataWriter_var generic_writer
                = para_publisher->create_datawriter (para_topic,
                                                     DATAWRITER_QOS_USE_TOPIC_QOS,
                                                      ::DDS::DataWriterListener::_nil (),
                                                      0U);

        net1578::paragraphDataWriter_var para_writer
                = net1578::paragraphDataWriter::_narrow (generic_writer.in ());

        if (::DDS::is_nil (para_writer.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil writer.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        // Wait until the other process sets our status to 1 to indicate it's
        // ready for us to start writing
        this->process_controller_->wait_for_process_state (1);

        cmx_initialise();
        networkxml = getNetworkEntity ();
        netstats = cmx_entityStatistics (networkxml);
        if (netstats == NULL)
        {
            fprintf (stderr, "ERROR %s (%s - %d): Can't find network statistics - are they enabled in ospl.xml ?.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }
        else
        {
            bytesBefore = getBytesBeforeCompression (netstats);
            bytesAfter = getBytesAfterCompression (netstats);
        }

        net1578::paragraph para_sample;

        para_sample.bookid = 1;
        para_sample.content = nextline;

        userHandle = para_writer->register_instance (para_sample);

        while ((scanres = scanf ("%2047[^\n]%1[\n]", nextline, newline)) != EOF)
        {
            if (!scanres)
            {
                getchar ();
                nextline[0] = '\0';
            }

            para_sample.checksum = sum (nextline);
            result = para_writer->write (para_sample, userHandle);

            if (result != ::DDS::RETCODE_OK)
            {
                fprintf (stderr, "ERROR %s (%s - %d): Write failed.\n",
                                  process_name_.in (), __FILE__, __LINE__);
                return -1;
            }
        }

        sprintf (nextline, "***EOT***");
        para_sample.checksum = sum (nextline);
        result = para_writer->write (para_sample, userHandle);

        if (result != ::DDS::RETCODE_OK)
        {
            fprintf (stderr, "ERROR %s (%s - %d): Write failed.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        printf("Sent all text\n");

        netstats = cmx_entityStatistics (networkxml);
        bytesBefore = getBytesBeforeCompression (netstats) - bytesBefore;
        bytesAfter = getBytesAfterCompression (netstats) - bytesAfter;

        printf ("Sent %ld bytes; compressed from %ld bytes\n", bytesAfter, bytesBefore);

        if (bytesBefore == 0)
        {
            fprintf (stderr, "ERROR %s (%s - %d): No compression recorded.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }
        if (bytesBefore <= bytesAfter)
        {
            fprintf (stderr, "ERROR %s (%s - %d): No gain from compression.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        // Park the thread to wait for the publisher to shut us down after
        // it has read the sample
        this->process_controller_->park_thread_to_do_work ();

        return 0;
    }


};

#endif /* OSPL_TESTS_CPP_PUBLISHER_H */
