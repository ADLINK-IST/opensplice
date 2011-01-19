

#ifndef EXT_PARTICIPANT_LISTENER_IMPL_H_
#define EXT_PARTICIPANT_LISTENER_IMPL_H_

#include "ccpp_dds_dcps.h"

using namespace std;
using namespace DDS;
using namespace CORBA;


class ExtParticipantListenerImpl : virtual public ExtDomainParticipantListener
{
  public:
    ExtParticipantListenerImpl(void)
    {
        os_mutexAttr mutexAttr;
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&mutex, &mutexAttr);

        os_condAttr condAttr;
        condAttr.scopeAttr = OS_SCOPE_PRIVATE;

        os_condInit(&cond, &mutex, &condAttr);
        reset();
    }

    virtual ~ExtParticipantListenerImpl(void) {}

    virtual void on_inconsistent_topic (
        DDS::Topic_ptr ,
        const DDS::InconsistentTopicStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_all_data_disposed (
        DDS::Topic_ptr the_topic
      ) THROW_ORB_EXCEPTIONS
    {
        os_mutexLock(&mutex);
        on_all_data_diposed_conter += 1;
        last_all_data_disposed_topic = the_topic;
        os_condSignal(&cond);
        os_mutexUnlock(&mutex);
    }

    virtual void on_offered_deadline_missed (
        DDS::DataWriter_ptr ,
        const DDS::OfferedDeadlineMissedStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_offered_incompatible_qos (
        DDS::DataWriter_ptr ,
        const DDS::OfferedIncompatibleQosStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_liveliness_lost (
        DDS::DataWriter_ptr ,
        const DDS::LivelinessLostStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_publication_matched (
        DDS::DataWriter_ptr ,
        const DDS::PublicationMatchedStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_requested_deadline_missed (
        DDS::DataReader_ptr ,
        const DDS::RequestedDeadlineMissedStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_requested_incompatible_qos (
        DDS::DataReader_ptr ,
        const DDS::RequestedIncompatibleQosStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_sample_rejected (
        DDS::DataReader_ptr ,
        const DDS::SampleRejectedStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_liveliness_changed (
        DDS::DataReader_ptr,
        const DDS::LivelinessChangedStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_data_available (
        DDS::DataReader_ptr
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_subscription_matched (
        DDS::DataReader_ptr,
        const DDS::SubscriptionMatchedStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_sample_lost (
        DDS::DataReader_ptr,
        const DDS::SampleLostStatus &
      ) THROW_ORB_EXCEPTIONS{}

    virtual void on_data_on_readers (
        DDS::Subscriber_ptr
      ) THROW_ORB_EXCEPTIONS{}

    ::DDS::ReturnCode_t wait_for_on_all_data_disposed(DDS::Duration_t timeout)
    {
        ::DDS::ReturnCode_t result = 0;
        os_time os_timeout;
        os_timeout.tv_sec = timeout.sec;
        os_timeout.tv_nsec = timeout.nanosec;
        os_time dead_line = os_timeAdd(os_hrtimeGet(), os_timeout);

        os_mutexLock(&mutex);
            os_time time = os_hrtimeGet();
            while(on_all_data_diposed_conter == 0 && os_timeCompare(time, dead_line) == OS_LESS)
            {
                os_time time_to_wait = os_timeSub(dead_line, time);
                os_condTimedWait(&cond, &mutex, &time_to_wait);
                time = os_hrtimeGet();
            }
        if(    on_all_data_diposed_conter == 0)
        {
            result = DDS::RETCODE_TIMEOUT;
        }
        os_mutexUnlock(&mutex);
        return result;
    }

    DDS::Topic_ptr get_last_all_data_disposed_topic(void)
    {
        os_mutexLock(&mutex);
            DDS::Topic_ptr result = last_all_data_disposed_topic;
        os_mutexUnlock(&mutex);
        return result;
    }

    long get_all_data_disposed_counter(void)
    {
        os_mutexLock(&mutex);
            long result = on_all_data_diposed_conter;
        os_mutexUnlock(&mutex);
        return result;
    }

    void reset(void)
    {
        os_mutexLock(&mutex);
        on_all_data_diposed_conter = 0;
        last_all_data_disposed_topic = NULL;
        os_mutexUnlock(&mutex);
    }

  private:
      os_mutex mutex;
      os_cond cond;
      int on_all_data_diposed_conter;
      DDS::Topic_ptr last_all_data_disposed_topic;

  private:
    ExtParticipantListenerImpl (const ExtParticipantListenerImpl &);
    void operator= (const ExtParticipantListenerImpl &);
};

#endif
