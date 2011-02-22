/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef CCPP_UTILS_H
#define CCPP_UTILS_H

#include "gapi.h"
#include "ccpp.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    OS_DCPS_API void ccpp_AllocateGapiSeq(gapi_octet* *buffer, gapi_unsigned_long len);
    OS_DCPS_API void ccpp_AllocateGapiSeq(gapi_string* *buffer, gapi_unsigned_long len);
    OS_DCPS_API void ccpp_AllocateGapiSeq( gapi_instanceHandle_t* *buffer, gapi_unsigned_long len);

    OS_DCPS_API void ccpp_AllocateDdsSeq(CORBA::Octet * *buffer, CORBA::ULong len);
    OS_DCPS_API void ccpp_AllocateDdsSeq(char** *buffer, CORBA::ULong len);
    OS_DCPS_API void ccpp_AllocateDdsSeq(::DDS::InstanceHandle_t* *buffer, CORBA::ULong len);

    OS_DCPS_API void ccpp_CopySeqElemIn(CORBA::Octet & from, gapi_octet & to);
    OS_DCPS_API void ccpp_CopySeqElemIn(char* & from, gapi_string & to);
    OS_DCPS_API void ccpp_CopySeqElemIn(::DDS::InstanceHandle_t & from, gapi_instanceHandle_t & to);

    OS_DCPS_API void ccpp_CopySeqElemOut(gapi_octet & from, CORBA::Octet & to);
    OS_DCPS_API void ccpp_CopySeqElemOut(gapi_string & from, char* & to);
    OS_DCPS_API void ccpp_CopySeqElemOut(gapi_instanceHandle_t & from, ::DDS::InstanceHandle_t & to);
    OS_DCPS_API void ccpp_CopySeqElemOut(gapi_qosPolicyCount_s & from, ::DDS::QosPolicyCount & to);

    OS_DCPS_API void ccpp_sequenceCopyIn( const ::DDS::StringSeq &from, gapi_stringSeq &to);
    OS_DCPS_API void ccpp_sequenceCopyOut( const gapi_stringSeq &from, ::DDS::StringSeq &to);

    template <class CCPP_SEQT, class CCPP_TYPE, class GAPI_SEQT, class GAPI_TYPE>
    void ccpp_sequenceCopyIn( const CCPP_SEQT &from, GAPI_SEQT &to)
    {
        to._maximum = from.maximum();
        to._length = from.length();
        to._release = FALSE;
        if (to._maximum > 0){
          to._buffer = const_cast<GAPI_TYPE *>(from.get_buffer());
        } else {
          to._buffer = NULL;
        };
    }

    template <class GAPI_SEQT, class GAPI_TYPE, class CCPP_SEQT, class CCPP_TYPE>
    void ccpp_sequenceCopyOut( const GAPI_SEQT &from, CCPP_SEQT &to)
    {
       to.length(from._length);
       for (CORBA::ULong i=0; i<from._length; i++)
       {
         ccpp_CopySeqElemOut(from._buffer[i], to[i]);
       }
    }

    template <class GAPI_SEQT>
    void ccpp_sequenceInitialize( GAPI_SEQT &target)
    {
      target._maximum = 0;
      target._length = 0;
      target._release = 0;
      target._buffer = NULL;
    }

    typedef struct ccpp_UserData *ccpp_UserData_ptr;

    struct ccpp_UserData : virtual public CORBA::LocalObject
    {

        CORBA::Object_ptr ccpp_object;
        ::DDS::Listener_ptr ccpp_listener;
        ccpp_UserData_ptr ccpp_statusconditiondata;

        ccpp_UserData(
            CORBA::Object_ptr myObject,
            ::DDS::Listener_ptr myListener = NULL,
            ::DDS::ccpp_UserData_ptr myStatusConditionData = NULL
        ) : ccpp_object(myObject),
            ccpp_listener(myListener),
            ccpp_statusconditiondata(myStatusConditionData)
        {
            CORBA::Object::_duplicate(ccpp_object);
            if (ccpp_listener)
            {
                ::DDS::Listener::_duplicate(ccpp_listener);
            }
        }

        void setListener(::DDS::Listener_ptr a_listener)
        {
          if (ccpp_listener)
          {
            CORBA::release(ccpp_listener);
          }
          ccpp_listener = a_listener;
          ::DDS::Listener::_duplicate(a_listener);
        }

        virtual ~ccpp_UserData ()
        {
            CORBA::release(ccpp_object);
            if (ccpp_listener)
            {
                CORBA::release(ccpp_listener);
            }
            if (ccpp_statusconditiondata)
            {
// removed the follwing line because some bug in the gapi are fixed, which exposes
// some really bad constructs in C++ language binding that must be removed
// For now this seems to avoid running into problems but not sure if this
// is bullet proof.
//                delete ccpp_statusconditiondata;
            }
        }
    };

    OS_DCPS_API void ccpp_Duration_copyIn( const ::DDS::Duration_t & from, gapi_duration_t &to);

    OS_DCPS_API void ccpp_Duration_copyOut( const gapi_duration_t & from, ::DDS::Duration_t &to);

    OS_DCPS_API void ccpp_ParticipantBuiltinTopicData_copyOut(
        const gapi_participantBuiltinTopicData & from,
        ::DDS::ParticipantBuiltinTopicData & to);

    OS_DCPS_API void ccpp_TopicBuiltinTopicData_copyOut(
        const gapi_topicBuiltinTopicData & from,
        ::DDS::TopicBuiltinTopicData & to);

    OS_DCPS_API void ccpp_SubscriptionBuiltinTopicData_copyOut(
        const gapi_subscriptionBuiltinTopicData & from,
        ::DDS::SubscriptionBuiltinTopicData & to);

    OS_DCPS_API void ccpp_PublicationBuiltinTopicData_copyOut(
        const gapi_publicationBuiltinTopicData & from,
        ::DDS::PublicationBuiltinTopicData & to);

    OS_DCPS_API void ccpp_BuiltinTopicKey_copyOut( const gapi_builtinTopicKey_t & from,
        ::DDS::BuiltinTopicKey_t &to);

    OS_DCPS_API void ccpp_CallBack_DeleteUserData( void * entityData, void * args);

    OS_DCPS_API void ccpp_TimeStamp_copyIn(const ::DDS::Time_t & from, gapi_time_t & to);

    OS_DCPS_API void ccpp_TimeStamp_copyOut(const gapi_time_t & from, ::DDS::Time_t & to);

    OS_DCPS_API void ccpp_SampleInfo_copyOut(const gapi_sampleInfo & in, ::DDS::SampleInfo & to);
}


#endif /* UTILS */
