#ifndef OSPL_TESTS_CPP_PUBLISHER_H
#define OSPL_TESTS_CPP_PUBLISHER_H

#include "ccpp_dds_dcps.h"
#include "ccpp_dds1578.h"

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

long getBytesBeforeDecompression (const char *stats)
{
   long result = 0;
   const char *pos = strstr (stats, "<nofBytesBeforeDecompression>");
   if (sscanf (pos, "<nofBytesBeforeDecompression>%ld</nofBytesBeforeDecompression>", &result) != 1)
   {
      printf ("Parse failure : nofBytesBeforeDecompression\n");
   }
   return result;
}

long getBytesAfterDecompression (const char *stats)
{
   long result = 0;
   const char *pos = strstr (stats, "<nofBytesAfterDecompression>");
   if (pos == NULL || sscanf (pos, "<nofBytesAfterDecompression>%ld</nofBytesAfterDecompression>", &result) != 1)
   {
      printf ("Parse failure : nofBytesAfterDecompression\n");
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
* @brief A very simple subscriber process for a very very simple
* publish / subscribe test.
* The subscriber publishes a sample. This subscriber reads it.
*/
class CPPSubscriberProcess
  : public virtual OSPLTestLib::CPPTestProcess
{
    public:
        CPPSubscriberProcess (const char* the_name) :
          CPPTestProcess (the_name) {}

       virtual ~CPPSubscriberProcess () {};
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
    * The test creates a simplest possible reader with all default QoS.
    * It the notifies the other (subscriber) porcess by setting its status to 1.
    * It reads a sample from the reader and then tells the subscriber to shutdown
    * and shuts down itself.
    */
    virtual int run ()
    {
        DDS::ReturnCode_t result;
        DDS::TopicQos tQos;
        DDS::SubscriberQos sQos;
        const char *networkxml, *netstats;
        long bytesBefore, bytesAfter;
        bool terminated = false;

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

        net1578::paragraphTypeSupport para_type_support;

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

        domain_participant->get_default_subscriber_qos (sQos);
        sQos.partition.name.length (1);
        sQos.partition.name[0] = CORBA::string_dup ("net1578");

        ::DDS::Subscriber_var para_subscriber
                = domain_participant->create_subscriber (sQos,
                                                        ::DDS::SubscriberListener::_nil (),
                                                        0U);
        if (::DDS::is_nil (para_subscriber.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil subscriber.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

        ::DDS::DataReader_var generic_reader
                = para_subscriber->create_datareader (para_topic,
                                                      DATAREADER_QOS_DEFAULT,
                                                      ::DDS::DataReaderListener::_nil (),
                                                      0U);

        net1578::paragraphDataReader_var para_reader
                = net1578::paragraphDataReader::_narrow (generic_reader.in ());

        if (::DDS::is_nil (para_reader.in ()))
        {
            fprintf (stderr, "ERROR %s (%s - %d): Nil reader.\n",
                              process_name_.in (), __FILE__, __LINE__);
            return -1;
        }

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
            bytesBefore = getBytesBeforeDecompression (netstats);
            bytesAfter = getBytesAfterDecompression (netstats);
        }

        // Create a data available status condition fron the reader
        ::DDS::StatusCondition_var reader_status_condition
                = para_reader->get_statuscondition ();
        reader_status_condition->set_enabled_statuses (::DDS::DATA_AVAILABLE_STATUS);

        // Attach it to a wait set
        ::DDS::WaitSet wait_set;
        wait_set.attach_condition (reader_status_condition);

        ::DDS::ConditionSeq_var  condition_list_out = new ::DDS::ConditionSeq ();

        // Set the other process status to 1 to indicate it can start writing now
        this->process_controller_->set_process_status (1, this->other_process_name_.in ());

        while (!terminated)
        {
            result = wait_set.wait (condition_list_out.inout (), ::DDS::DURATION_INFINITE);

            if (result != ::DDS::RETCODE_OK)
            {
                fprintf (stderr, "ERROR %s (%s - %d): Wait failed.\n",
                                  process_name_.in (), __FILE__, __LINE__);
                return -1;
            }

            if (condition_list_out->length () != 1
                || condition_list_out[0U].in () != reader_status_condition.in ())
            {
                fprintf (stderr, "ERROR %s (%s - %d): Unexpected status condition value.\n",
                                  process_name_.in (), __FILE__, __LINE__);
                return -1;
            }

            net1578::paragraphSeq para_samples;
            ::DDS::SampleInfoSeq_var sample_info_seq = new ::DDS::SampleInfoSeq ();

            result = para_reader->take (para_samples,
                                        sample_info_seq,
                                        ::DDS::LENGTH_UNLIMITED,
                                        ::DDS::NOT_READ_SAMPLE_STATE,
                                        ::DDS::ANY_VIEW_STATE,
                                        ::DDS::ALIVE_INSTANCE_STATE);

            for (unsigned int i = 0; i < para_samples.length(); i++)
            {
                const char *msg = para_samples[i].content.in ();
                if (msg)
                {
                    if (strcmp (msg,  "***EOT***"))
                    {
                        if (sum (msg) != para_samples[i].checksum)
                        {
                            fprintf
                            (
                                stderr,
                                "ERROR %s (%s - %d): Bad checksum %d for %s\n",
                                process_name_.in (),
                                __FILE__,
                                __LINE__,
                                para_samples[i].checksum,
                                msg
                            );
                            return -1;
                        }
                    }
                    else
                    {
                        terminated = true;
                    }
                }
            }

            para_reader->return_loan (para_samples, sample_info_seq);

            if (result != ::DDS::RETCODE_OK)
            {
                fprintf (stderr, "ERROR %s (%s - %d): Read failed.\n",
                                  process_name_.in (), __FILE__, __LINE__);
                return -1;
            }
        }

        netstats = cmx_entityStatistics (networkxml);
        bytesBefore = getBytesBeforeDecompression (netstats) - bytesBefore;
        bytesAfter = getBytesAfterDecompression (netstats) - bytesAfter;

        printf ("Received %ld bytes; decompressed to %ld bytes\n", bytesBefore, bytesAfter);

        if (bytesBefore == 0)
        {
            fprintf (stderr, "Error - no compressed data registered\n");
            exit (EXIT_FAILURE);
        }
        if (bytesBefore >= bytesAfter)
        {
            fprintf (stderr, "Decompression error - no gain from compression\n");
            exit (EXIT_FAILURE);
        }

        // Shutdown the publisher and then exit
        this->process_controller_->shutdown (this->other_process_name_.in ());

        // Success
        return 0;
    }


};

#endif /* OSPL_TESTS_CPP_PUBLISHER_H */

