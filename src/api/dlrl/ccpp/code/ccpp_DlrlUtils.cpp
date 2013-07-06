/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/* DLRL ccpp API includes */
#include "ccpp_DlrlUtils.h"
#include "ccpp_ObjectHome_impl.h"
#include "ccpp_ObjectRoot_impl.h"

/* DCPS ccpp includes */
#include "gapi.h"
#include "ccpp_Entity_impl.h"
#include "ccpp_Selection_impl.h"

/* Generic DLRL includes */
#include "DLRL_Report.h"
#include "DLRL_Exception.h"
#include "DLRL_Util.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

/* All bridge include files which define the operations to be implemented by
 * the DLRL ccpp API
 */
#include "DK_CacheAccessBridge.h"
#include "DK_CollectionBridge.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_ObjectBridge.h"
#include "DK_ObjectHomeBridge.h"
#include "DK_ObjectReaderBridge.h"
#include "DK_ObjectRelationReaderBridge.h"
#include "DK_ObjectWriterBridge.h"
#include "DK_SelectionBridge.h"
#include "DK_UtilityBridge.h"

/* DCPS GAPI includes */
#include "gapi_publisher.h"
#include "gapi_subscriber.h"
#include "gapi_topic.h"
#include "gapi_dataReader.h"
#include "gapi_dataWriter.h"
#include "gapi_object.h"
#include "gapi_entity.h"

/* Local helper function for ccpp_ObjectHomeBridge_us_triggerListeners. */
static void
ccpp_ObjectHomeBridge_us_listenerWalk(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    Coll_List* samples,
    ccpp_DlrlUtils_us_invokeXXXObjectCallback invokexxxObjectCallback);

/**************************************/
/************ CacheBridge *************/
/**************************************/
extern "C"
{
  u_publisher
  ccpp_CacheBridge_us_createPublisher(
      DLRL_Exception* exception,
      void* userData,
      u_participant participant,
      DLRL_LS_object ls_participant,
      DLRL_LS_object *ls_publisher)
  {
      DDS::DomainParticipant_ptr ccppParticipant = NULL;
      DDS::PublisherQos pQos;
      DDS::Publisher_ptr ccppPublisher = NULL;
      gapi_publisher gapiPublisher = NULL;
      _Publisher _publisher = NULL;
      gapi_returnCode_t result = GAPI_RETCODE_OK;
      u_publisher upublisher = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(participant);
      assert(ls_participant);
      assert(!(*ls_publisher));

      ccppParticipant = DOWNCAST_DDS_PARTICIPANT(ls_participant);
      ccppParticipant->get_default_publisher_qos(pQos);
      pQos.entity_factory.autoenable_created_entities = FALSE;
      ccppPublisher = ccppParticipant->create_publisher(pQos, NULL, 0);
      if(!ccppPublisher){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Unable to create a "
              "publisher. Check DCPS error log file for (possibly) more "
              "information.");
      }

      gapiPublisher = reinterpret_cast<gapi_publisher>(DDS_GET_GAPI_SELF(ccppPublisher));
      if(!gapiPublisher){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Unable to create a "
              "publisher. Check DCPS error log file for (possibly) more "
              "information.");
      }
      _publisher = gapi_publisherClaim(gapiPublisher, &result);
      DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result, "Unable to create "
          "a publisher.");
      upublisher = _PublisherUpublisher(_publisher);
      if(upublisher)
      {
          /* now create a proxy to this user layer upublisher which can be used by the DLRL
           * in a safe manner, as the user layer upublisher returned by the _PublisherUpublisher
           * operation is owned by the gapi.
           */
          upublisher = u_publisher(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(upublisher)));
      }
      _EntityRelease(_publisher);/* before the propagate */
      DLRL_Exception_PROPAGATE(exception);/* after the release */
      if(!upublisher){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Unable to create a "
              "publisher. Check DCPS error log file for (possibly) more "
              "information.");
      }
      *ls_publisher = UPCAST_DLRL_LS_OBJECT(ccppPublisher);

      DLRL_Exception_EXIT(exception);
      /*no rollback from DCPSError exceptions*/
      DLRL_INFO(INF_EXIT);
      return upublisher;
  }

  u_subscriber
  ccpp_CacheBridge_us_createSubscriber(
      DLRL_Exception* exception,
      void* userData,
      u_participant participant,
      DLRL_LS_object ls_participant,
      DLRL_LS_object *ls_subscriber)
  {
      DDS::DomainParticipant_ptr ccppParticipant = NULL;
      DDS::SubscriberQos sQos;
      DDS::Subscriber_ptr ccppSubscriber = NULL;
      gapi_subscriber gapiSubscriber = NULL;
      _Subscriber _subscriber = NULL;
      gapi_returnCode_t result = GAPI_RETCODE_OK;
      u_subscriber usubscriber = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(participant);
      assert(ls_participant);
      assert(!(*ls_subscriber));

      ccppParticipant = DOWNCAST_DDS_PARTICIPANT(ls_participant);
      ccppParticipant->get_default_subscriber_qos(sQos);
      sQos.entity_factory.autoenable_created_entities = FALSE;
      ccppSubscriber = ccppParticipant->create_subscriber(sQos, NULL, 0);
      if(!ccppSubscriber){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                               "Unable to create a subscriber. "
                               "Check DCPS error log file for (possibly) more "
                               "information.");
      }
      gapiSubscriber = reinterpret_cast<gapi_subscriber>(
          DDS_GET_GAPI_SELF(ccppSubscriber));
      if(!gapiSubscriber){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                               "Unable to create a subscriber. "
                               "Check DCPS error log file for (possibly) more "
                               "information.");
      }
      _subscriber = gapi_subscriberClaim(gapiSubscriber, &result);
      DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result,
                                           "Unable to create a subscriber.");
      usubscriber = _SubscriberUsubscriber(_subscriber);
      if(usubscriber)
      {
          /* now create a proxy to this user layer usubscriber which can be used by the DLRL
           * in a safe manner, as the user layer topic returned by the _SubscriberUsubscriber
           * operation is owned by the gapi.
           */
          usubscriber = u_subscriber(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(usubscriber)));
      }
      _EntityRelease(_subscriber);/* before the propagate */
      DLRL_Exception_PROPAGATE(exception);/* after the release */
      if(!usubscriber){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                               "Unable to create a subscriber. "
                               "Check DCPS error log file for (possibly) more "
                               "information.");
      }
      *ls_subscriber = UPCAST_DLRL_LS_OBJECT(ccppSubscriber);
      DLRL_Exception_EXIT(exception);
      /*no rollback from DCPSError exceptions*/
      DLRL_INFO(INF_EXIT);
      return usubscriber;
  }

  void
  ccpp_CacheBridge_us_deletePublisher(
      DLRL_Exception* exception,
      void* userData,
      u_participant participant,
      DLRL_LS_object ls_participant,
      DLRL_LS_object ls_publisher)
  {
      DDS::DomainParticipant_ptr ccppParticipant = NULL;
      DDS::Publisher_ptr ccppPublisher = NULL;
      DDS::ReturnCode_t returnCode = DDS::RETCODE_OK;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(participant);
      assert(ls_participant);
      assert(ls_publisher);

      ccppParticipant = DOWNCAST_DDS_PARTICIPANT(ls_participant);
      ccppPublisher = DOWNCAST_DDS_PUBLISHER(ls_publisher);

      returnCode = ccppParticipant->delete_publisher(ccppPublisher);
      DLRL_DcpsException_PROPAGATE(exception, returnCode, "Delete of publisher "
          "failed.");
      CORBA::release(ccppPublisher);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_deleteSubscriber(
      DLRL_Exception* exception,
      void* userData,
      u_participant participant,
      DLRL_LS_object ls_participant,
      DLRL_LS_object ls_subscriber)
  {
      DDS::DomainParticipant_ptr ccppParticipant = NULL;
      DDS::Subscriber_ptr ccppSubscriber = NULL;
      DDS::ReturnCode_t returnCode = DDS::RETCODE_OK;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(participant);
      assert(ls_participant);
      assert(ls_subscriber);

      ccppParticipant = DOWNCAST_DDS_PARTICIPANT(ls_participant);
      ccppSubscriber = DOWNCAST_DDS_SUBSCRIBER(ls_subscriber);

      returnCode = ccppParticipant->delete_subscriber(ccppSubscriber);
      DLRL_DcpsException_PROPAGATE(exception, returnCode,
                                   "Delete of subscriber failed.");
      CORBA::release(ccppSubscriber);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_triggerListenersWithStartOfUpdates(
      DLRL_Exception* exception,
      DK_CacheAdmin* relatedCache,
      void* userData)
  {
      Coll_Set* listeners;
      Coll_Iter* iterator;
      DDS::CacheListener_ptr aListener;

      DLRL_INFO(INF_ENTER);

      listeners = DK_CacheAdmin_us_getListeners(relatedCache);
      iterator = Coll_Set_getFirstElement(listeners);
      try {
          while(iterator){
        	  aListener = DOWNCAST_DDS_CACHELISTENER(
            	  Coll_Iter_getObject(iterator));
              aListener->on_begin_updates();
              iterator = Coll_Iter_getNext(iterator);
          }
      } DLRL_CCPP_EXCEPTION_CATCH_AND_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_triggerListenersWithEndOfUpdates(
      DLRL_Exception* exception,
      DK_CacheAdmin* relatedCache,
      void* userData)
  {
      Coll_Set* listeners;
      Coll_Iter* iterator;
      DDS::CacheListener_ptr aListener;

      DLRL_INFO(INF_ENTER);

      listeners = DK_CacheAdmin_us_getListeners(relatedCache);
      iterator = Coll_Set_getFirstElement(listeners);
      try {
          while(iterator){
              aListener = DOWNCAST_DDS_CACHELISTENER(
            	  Coll_Iter_getObject(iterator));
              aListener->on_end_updates();
              iterator = Coll_Iter_getNext(iterator);
          }
      } DLRL_CCPP_EXCEPTION_CATCH_AND_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_triggerListenersWithUpdatesEnabled(
      DLRL_Exception* exception,
      DK_CacheAdmin* relatedCache,
      void* userData)
  {
      Coll_Set* listeners;
      Coll_Iter* iterator;
      DDS::CacheListener_ptr aListener;

      DLRL_INFO(INF_ENTER);

      listeners = DK_CacheAdmin_us_getListeners(relatedCache);
      iterator = Coll_Set_getFirstElement(listeners);
      try {
          while(iterator){
              aListener = DOWNCAST_DDS_CACHELISTENER(
            	  Coll_Iter_getObject(iterator));
              aListener->on_updates_enabled();
              iterator = Coll_Iter_getNext(iterator);
          }
      } DLRL_CCPP_EXCEPTION_CATCH_AND_PROPAGATE(exception);
      while(iterator){
          aListener = DOWNCAST_DDS_CACHELISTENER(
              Coll_Iter_getObject(iterator));
          aListener->on_updates_enabled();
          /* TODO maybe an exception occured on application level, we must forward
           * it!!
           */
      }

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_triggerListenersWithUpdatesDisabled(
      DLRL_Exception* exception,
      DK_CacheAdmin* relatedCache,
      void* userData)
  {
      Coll_Set* listeners;
      Coll_Iter* iterator;
      DDS::CacheListener_ptr aListener;

      DLRL_INFO(INF_ENTER);

      listeners = DK_CacheAdmin_us_getListeners(relatedCache);
      iterator = Coll_Set_getFirstElement(listeners);
      try {
          while(iterator){
              aListener = DOWNCAST_DDS_CACHELISTENER(
            	  Coll_Iter_getObject(iterator));
              aListener->on_updates_disabled();
              iterator = Coll_Iter_getNext(iterator);
          }
      } DLRL_CCPP_EXCEPTION_CATCH_AND_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_homesAction(
      DLRL_Exception* exception,
      void* userData,
      const Coll_List* homes,
      void** arg)
  {
      Coll_Iter* iterator = NULL;
      DK_ObjectHomeAdmin* home = NULL;
      DDS::ObjectHomeSeq* homeSeq = NULL;
      LOC_unsigned_long count = 0;
      DDS::ObjectHome_ptr lsHome = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(homes);

      if(!(*arg)){
          DDS::ObjectHomeSeq* cppHomes;
          LOC_unsigned_long size;

          size = Coll_List_getNrOfElements(homes);
          cppHomes = new DDS::ObjectHomeSeq(size);
          DLRL_VERIFY_ALLOC(cppHomes, exception, "Unable to allocate memory");
          cppHomes->length(size);
          *arg = reinterpret_cast<void*>(cppHomes);
      }
      homeSeq = reinterpret_cast<DDS::ObjectHomeSeq*>(*arg);

      iterator = Coll_List_getFirstElement(homes);
      while(iterator){
          /* get the object */
          home = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
          lsHome = DOWNCAST_DDS_OBJECTHOME(DK_ObjectHomeAdmin_ts_getLSHome(
              home,
              exception,
              NULL));
          DLRL_Exception_PROPAGATE(exception);
          /* assign the home */
          (*homeSeq)[count] = lsHome;
          /* move to the next object */
          iterator = Coll_Iter_getNext(iterator);
          count++;
      }

      DLRL_Exception_EXIT(exception);
      /* dont translate exception, not an API routine */
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_accessesAction(
      DLRL_Exception* exception,
      void* userData,
      const Coll_Set* accesses,
      void** arg)
  {
      Coll_Iter* iterator = NULL;
      DK_CacheAccessAdmin* access = NULL;
      DDS::CacheAccessSeq* accessSeq = NULL;
      LOC_unsigned_long count = 0;
      DDS::CacheAccess_ptr lsAccess = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(accesses);
      assert(!userData);

      if(!(*arg)){
          LOC_unsigned_long size;
          DDS::CacheAccessSeq* cppCacheAccesses;

          size = Coll_Set_getNrOfElements(accesses);
          cppCacheAccesses = new DDS::CacheAccessSeq(size);
          DLRL_VERIFY_ALLOC(cppCacheAccesses, exception, "Unable to allocate memory");
          cppCacheAccesses->length(size);
          *arg = reinterpret_cast<void*>(cppCacheAccesses);
      }
      accessSeq = reinterpret_cast<DDS::CacheAccessSeq*>(*arg);

      iterator = Coll_Set_getFirstElement(accesses);
      while(iterator){
          /* get the object */
          access = (DK_CacheAccessAdmin*)Coll_Iter_getObject(iterator);
          lsAccess = DOWNCAST_DDS_ACCESS(DK_CacheAccessAdmin_ts_getLSAccess(
              access,
              exception,
              NULL));
          DLRL_Exception_PROPAGATE(exception);
          /* assign the access to the seq */
          (*accessSeq)[count] = lsAccess;
          /* move to the next object */
          iterator = Coll_Iter_getNext(iterator);
          count++;
      }

      DLRL_Exception_EXIT(exception);
      /* dont translate exception, not an API routine */
      DLRL_INFO(INF_EXIT);
  }

  LOC_boolean
  ccpp_CacheBridge_us_isDataAvailable(
      DLRL_Exception* exception,
      void* userData,
      DLRL_LS_object ls_subscriber)
  {
      gapi_statusMask mask;
      LOC_boolean retVal = FALSE;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(ls_subscriber);

      mask = gapi_entity_get_status_changes(DDS_GET_GAPI_SELF(
          DOWNCAST_DDS_ENTITY(ls_subscriber)));
      if(mask & GAPI_DATA_ON_READERS_STATUS){
          retVal = TRUE;
      }

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return retVal;
  }

  void
  ccpp_CacheBridge_us_listenersAction(
      DLRL_Exception* exception,
      void* userData,
      const Coll_Set* listeners,
      void** arg)
  {
      Coll_Iter* iterator = NULL;
      DDS::CacheListenerSeq* listenerSeq = NULL;
      LOC_unsigned_long count = 0;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(listeners);
      assert(!userData);

      if(!(*arg)){
          LOC_unsigned_long size;
          DDS::CacheListenerSeq* cppListenerSeq;

          size = Coll_Set_getNrOfElements(listeners);
          cppListenerSeq = new DDS::CacheListenerSeq(size);
          DLRL_VERIFY_ALLOC(cppListenerSeq, exception, "Unable to allocate memory");
          cppListenerSeq->length(size);
          *arg = reinterpret_cast<void*>(cppListenerSeq);
      }
      listenerSeq = reinterpret_cast<DDS::CacheListenerSeq*>(*arg);

      iterator = Coll_Set_getFirstElement(listeners);
      while(iterator){
          DDS::CacheListener_ptr aCacheListener;

          /* assign the listener to the seq */
          aCacheListener = DOWNCAST_DDS_CACHELISTENER(Coll_Iter_getObject(iterator));
          //DLRL_Exception_PROPAGATE(exception);
          (*listenerSeq)[count] = DDS::CacheListener::_duplicate(aCacheListener);
          /* move to the next object */
          iterator = Coll_Iter_getNext(iterator);
          count++;
      }

      DLRL_Exception_EXIT(exception);
      /* dont translate exception, not an API routine */
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheBridge_us_objectsAction(
      DLRL_Exception* exception,
      void* userData, void** arg,
      LOC_unsigned_long totalSize,
      LOC_unsigned_long* elementIndex,
      DK_ObjectArrayHolder* holder)
  {
      DDS::ObjectRootSeq* rootSeq = NULL;
      LOC_unsigned_long elementCount = 0;
      DDS::ObjectRoot* object = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(holder);

      if(!(*arg)){
          DDS::ObjectRootSeq* cppObjectRootSeq;

          cppObjectRootSeq = new DDS::ObjectRootSeq(totalSize);
          DLRL_VERIFY_ALLOC(cppObjectRootSeq, exception, "Unable to allocate memory");
          cppObjectRootSeq->length(totalSize);
          *arg = reinterpret_cast<void*>(cppObjectRootSeq);
      }
      rootSeq = reinterpret_cast<DDS::ObjectRootSeq*>(*arg);
      for(elementCount = 0; elementCount < holder->size; elementCount++){
          assert((*elementIndex) < totalSize);

          object = DOWNCAST_DDS_OBJECTROOT(DK_ObjectAdmin_us_getLSObject(
              holder->objectArray[elementCount]));
          object->_add_ref();
          (*rootSeq)[(*elementIndex)] = object;
          (*elementIndex)++;
      }

      DLRL_Exception_EXIT(exception);
      /* Do not translate (handle) exception here, this is not an API function.*/
      DLRL_INFO(INF_EXIT);
  }

/**************************************/
/********** CacheAccessBridge *********/
/**************************************/

  void
  ccpp_CacheAccessBridge_us_containedTypesAction(
      DLRL_Exception* exception,
      void* userData,
      LOC_long* indexes,
      LOC_unsigned_long totalSize,
      void** arg)
  {
      LOC_unsigned_long count = 0;
      LOC_long index = 0;
      DDS::LongSeq* cppLongSeq = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);

      cppLongSeq = new DDS::LongSeq(totalSize);
      DLRL_VERIFY_ALLOC(cppLongSeq, exception, "Unable to allocate memory");
      cppLongSeq->length(totalSize);
      *arg = reinterpret_cast<void*>(cppLongSeq);

      for(count = 0; count < totalSize; count++){
          (*cppLongSeq)[count] = static_cast<CORBA::Long>(indexes[count]);
      }

      DLRL_Exception_EXIT(exception);

      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheAccessBridge_us_containedTypeNamesAction(
      DLRL_Exception* exception,
      void* userData,
      LOC_unsigned_long totalSize,
      LOC_unsigned_long index,
      LOC_string name, void** arg)
  {
      DDS::StringSeq* typeSeq = NULL;
      LOC_string tmpName = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);

      if(!(*arg)){
          typeSeq = new DDS::StringSeq(totalSize);
          DLRL_VERIFY_ALLOC(typeSeq, exception, "Unable to allocate memory");
          typeSeq->length(totalSize);
          *arg = reinterpret_cast<void*>(typeSeq);
      } else {
          typeSeq = reinterpret_cast<DDS::StringSeq*>(*arg);
      }

      DLRL_STRDUP(tmpName, name, exception, "Unable to allocate memory");

      (*typeSeq)[index] = tmpName;

      DLRL_Exception_EXIT(exception);

      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheAccessBridge_us_objectsAction(
      DLRL_Exception* exception,
      void* userData,
      void** arg,
      LOC_unsigned_long totalSize,
      LOC_unsigned_long* elementIndex,
      Coll_Set* objects)
  {
      DDS::ObjectRootSeq* rootSeq = NULL;
      Coll_Iter* iterator = NULL;
      DK_ObjectAdmin* object = NULL;
      DDS::ObjectRoot* ls_object = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(objects);

      if(!(*arg)){
          rootSeq = new DDS::ObjectRootSeq(totalSize);
          DLRL_VERIFY_ALLOC(rootSeq, exception, "Unable to allocate memory");
          rootSeq->length(totalSize);
          *arg = reinterpret_cast<void*>(rootSeq);
      } else {
          rootSeq = reinterpret_cast<DDS::ObjectRootSeq*>(*arg);
      }

      iterator = Coll_Set_getFirstElement(objects);
      while(iterator){
          assert((*elementIndex) < totalSize);
          object=reinterpret_cast<DK_ObjectAdmin*>(Coll_Iter_getObject(iterator));
          ls_object = DOWNCAST_DDS_OBJECTROOT(DK_ObjectAdmin_us_getLSObject(object));
          DLRL_Exception_PROPAGATE(exception);
          ls_object->_add_ref();
          (*rootSeq)[*elementIndex] = ls_object;
          (*elementIndex)++;
          iterator = Coll_Iter_getNext(iterator);
      }

      DLRL_Exception_EXIT(exception);
      /* Do not translate (handle) exception here, this is not an API function.*/
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_CacheAccessBridge_us_invalidObjectsAction(
      DLRL_Exception* exception,
      void* userData,
      void** arg,
      Coll_List* invalidObjects)
  {
      LOC_unsigned_long size = 0;
      DDS::ObjectRootSeq* rootSeq = NULL;
      LOC_unsigned_long count = 0;
      DK_ObjectAdmin* object = NULL;
      DDS::ObjectRoot* ls_object = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(invalidObjects);

      size = Coll_List_getNrOfElements(invalidObjects);
      rootSeq = new DDS::ObjectRootSeq(size);
      DLRL_VERIFY_ALLOC(rootSeq, exception, "Unable to allocate memory");
      rootSeq->length(size);
      *arg = reinterpret_cast<void*>(rootSeq);

      for(count = 0; count < size; count++){
          object = (DK_ObjectAdmin*)Coll_List_popBack(invalidObjects);
          /* dont release the 'object', it is not a duplicate */
          ls_object = DOWNCAST_DDS_OBJECTROOT(
              DK_ObjectAdmin_us_getLSObject(object));
          ls_object->_add_ref();
          (*rootSeq)[count] = ls_object;
      }

      DLRL_Exception_EXIT(exception);
      /* Do not translate (handle) exception here, this is not an API function.*/
      DLRL_INFO(INF_EXIT);
  }

/**************************************/
/********** CollectionBridge **********/
/**************************************/
  DLRL_LS_object
  ccpp_CollectionBridge_us_createLSCollection(
      DLRL_Exception* exception,
      void* userdata,
      DK_Collection* collection,
      DK_RelationType relationType)
  {
      DK_ObjectHomeAdmin* targetHome = NULL;
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      DLRL_LS_object retVal = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userdata);
      assert(collection);
      assert(relationType < DK_RelationType_elements);

      targetHome = DK_Collection_us_getTargetHome(collection);
      assert(targetHome);
      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(targetHome));
      assert(targetObjectCachedData);
      /* All collection creation operations returns a 'dup' of the created object,
       * well the ref count is from the new actually, but the point is the object
       * can be stored in the kernel and the kernel is thus the owner of the
       * created object.
       */
      switch (relationType){
          case DK_RELATION_TYPE_STR_MAP:
              retVal = targetObjectCachedData->createTypedStrMap(
                  exception,
                  collection);
              DLRL_Exception_PROPAGATE(exception);
              break;
          case DK_RELATION_TYPE_INT_MAP:
              retVal = targetObjectCachedData->createTypedIntMap(
                  exception,
                  collection);
              DLRL_Exception_PROPAGATE(exception);
              break;
          case DK_RELATION_TYPE_SET:
              retVal = targetObjectCachedData->createTypedSet(
                  exception,
                  collection);
              DLRL_Exception_PROPAGATE(exception);
              break;
          default:
              assert(FALSE);
      }
      assert(retVal);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return retVal;
  }

  void
  ccpp_DCPSUtilityBridge_us_registerType(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DK_CacheAdmin* cache,
      LOC_char* topicName,
      LOC_char* typeName)
  {
      DDS::DomainParticipant_ptr lsParticipant = NULL;
      DDS::ObjectHome_impl* lsHome = NULL;
      DDS::ReturnCode_t returnCode = DDS::RETCODE_OK;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(home);
      assert(cache);
      assert(topicName);
      assert(typeName);

      /* get the ccpp objects */
      lsParticipant = DOWNCAST_DDS_PARTICIPANT(DK_CacheAdmin_us_getLSParticipant(
          cache));
      lsHome =DOWNCAST_DDS_OBJECTHOME_IMPL(DK_ObjectHomeAdmin_us_getLSHome(home));

      DLRL_INFO(INF_CALLBACK, "objectHome->registerType(participant, typeName, "
          "topicName)");
      returnCode = lsHome->registerType(lsParticipant, typeName, topicName);
      DLRL_DcpsException_PROPAGATE(exception, returnCode, "Unable to register "
          "type to DCPS TypeSupport entity for topic %s of %s '%p'.",
          DLRL_VALID_NAME(topicName), "DK_ObjectHomeAdmin", home);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

/**************************************/
/********** DCPSUtilityBridge *********/
/**************************************/
  C_STRUCT(u_topic)*
  ccpp_DCPSUtilityBridge_us_createTopic(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      LOC_char* topicName,
      LOC_char* typeName,
      void** topicUserData,
      void** ls_topic,
      LOC_boolean isMainTopic)
  {
      DDS::DomainParticipant_ptr participant = NULL;
      DDS::ObjectHome_ptr lsHome = NULL;
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      ccpp_TypedTopicCache* typedTopicCachedData = NULL;
      DDS::Topic_ptr ccppTopic = NULL;
      DDS::TopicQos topicQos;
      DDS::Duration_t timeout = {0L, 0UL};
      gapi_topic gapiTopic = NULL;
      _Topic _topic = NULL;
      gapi_returnCode_t result = GAPI_RETCODE_OK;
      u_topic utopic = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(home);
      assert(topicName);
      assert(typeName);

      /* the cache getter on the object home doesnt require a release on the
       * returned pointer
       */
      participant = DOWNCAST_DDS_PARTICIPANT(DK_CacheAdmin_us_getLSParticipant(
          DK_ObjectHomeAdmin_us_getCache(home)));
      lsHome = DOWNCAST_DDS_OBJECTHOME(DK_ObjectHomeAdmin_us_getLSHome(home));

      targetObjectCachedData =(ccpp_TypedObjectCache*)
          DK_ObjectHomeAdmin_us_getUserData(home);
      assert(targetObjectCachedData);
      typedTopicCachedData = reinterpret_cast<ccpp_TypedTopicCache*>(os_malloc(sizeof(ccpp_TypedTopicCache)));
      DLRL_VERIFY_ALLOC(typedTopicCachedData, exception, "Out of resources");
      memset(typedTopicCachedData, 0, sizeof(ccpp_TypedTopicCache));
      targetObjectCachedData->initializeTopicCache(typedTopicCachedData);
      *topicUserData = reinterpret_cast<void*>(typedTopicCachedData);

      /* Workaround for the DCPS limitation that topic entities cannot
       * be created in a disabled state (which is a DLRL requirement):
       * Until this is addressed, look up any existing topics that may
       * be registered (with the first "find_topic" below), before trying
       * to create any new ones.  This way it reduces the chance that
       * qos settings will clash for a particular topic (as was the case
       * in the WhiteListedMessageBoard example).
       */
      ccppTopic = participant->find_topic (topicName, timeout);
      if(!ccppTopic){
          participant->get_default_topic_qos(topicQos);
          ccppTopic = participant->create_topic(
              topicName,
              typeName,
              topicQos,
              NULL,
              0);
          if(!ccppTopic){
              ccppTopic = participant->find_topic (topicName, timeout);
              if(!ccppTopic){
                  DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                      "Unable to find the DCPS topic with name '%s' and type '%s' "
                      "after creation also failed. Check DCPS error log for "
                      "(possibly) more information.",  topicName, typeName);
              }
          }
      }
      gapiTopic = reinterpret_cast<gapi_topic>(DDS_GET_GAPI_SELF(ccppTopic));
      if(!gapiTopic){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
              "Unable to create the DCPS Topic entity for topic %s with type %s "
              "of %s '%p'. Check DCPS error log file for (possibly) more "
              "information.", DLRL_VALID_NAME(topicName),
              DLRL_VALID_NAME(typeName), "DK_ObjectHomeAdmin", home);
      }
      _topic = gapi_topicClaim(gapiTopic, &result);
      DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result,
                                           "Failed to claim the topic handle");
      utopic = _TopicUtopic(_topic);
      if(utopic)
      {
          /* now create a proxy to this user layer topic which can be used by the DLRL
           * in a safe manner, as the user layer topic returned by the _TopicUtopic
           * operation is owned by the gapi.
           */
          utopic = u_topic(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(utopic)));
      }
      _EntityRelease(_topic);/* before the propagate */
      DLRL_Exception_PROPAGATE(exception);/* after the release */
      if(!utopic)
      {
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
              "Unable to create the DCPS Topic entity for topic %s with type %s "
              "of %s '%p'. Check DCPS error log file for (possibly) more "
              "information.", DLRL_VALID_NAME(topicName),
              DLRL_VALID_NAME(typeName), "DK_ObjectHomeAdmin", home);
      }
      /* Do not need to dup, as the create_topic gave us a dupped value */
      *ls_topic = UPCAST_DLRL_LS_OBJECT(ccppTopic);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return utopic;
  }

  u_reader
  ccpp_DCPSUtilityBridge_us_createDataReader(
      DLRL_Exception* exception,
      void* userData,
      DK_TopicInfo* topicInfo,
      void** ls_reader)
  {
      DK_ObjectHomeAdmin* home = NULL;
      DK_CacheAdmin* cache = NULL;
      DDS::DomainParticipant_ptr participant = NULL;
      DDS::Topic_ptr topic = NULL;
      DDS::Subscriber_ptr subscriber = NULL;
      LOC_char* topicName = NULL;
      DDS::TopicDescription_var topicDes = NULL;
      DDS::DataReaderQos readerQos;
      DDS::TopicQos topicQos;
      DDS::ReturnCode_t retCode = DDS::RETCODE_OK;
      DDS::DataReader_ptr ccppReader = NULL;
      gapi_dataReader gapiReader = NULL;
      _DataReader _reader = NULL;
      gapi_returnCode_t result = GAPI_RETCODE_OK;
      u_reader ureader = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(topicInfo);

      home = DK_TopicInfo_us_getOwner(topicInfo);/* no duplicate done */
      cache = DK_ObjectHomeAdmin_us_getCache(home);
      /* the cache getter on the object home doesnt require a release on the
       * returned pointer
       */
      participant = DOWNCAST_DDS_PARTICIPANT(
          DK_CacheAdmin_us_getLSParticipant(cache));
      topic = DOWNCAST_DDS_TOPIC(DK_TopicInfo_us_getLSTopic(topicInfo));
      subscriber = DOWNCAST_DDS_SUBSCRIBER(
          DK_CacheAdmin_us_getLSSubscriber(cache));
      topicName = (LOC_char*)DK_TopicInfo_us_getTopicName(topicInfo);

      topicDes = participant->lookup_topicdescription (topicName);
      if(!topicDes.in()){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Creation of data "
              "reader for topic %s failed! Check DCPS error log file for "
              "(possibly) more information.", topicName);
      }
      subscriber->get_default_datareader_qos(readerQos);
      topic->get_qos(topicQos);
      retCode = subscriber->copy_from_topic_qos(readerQos, topicQos);
      DLRL_DcpsException_PROPAGATE(exception, retCode, "Unable to create "
          "DataReader for topic %s. Copy from topic QoS failed!", topicName);
      ccppReader = subscriber->create_datareader(
          topicDes.in(),
          readerQos,
          NULL,
          0);
      if(!ccppReader){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Creation of data "
              "reader for topic %s failed! Check DCPS error log file for "
              "(possibly) more information.", topicName);
      }

      gapiReader = reinterpret_cast<gapi_dataReader>(
          DDS_GET_GAPI_SELF(ccppReader));
      if(!gapiReader)
      {
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
              "Unable to create the DCPS DataReader entity for topic %s of DLRL "
              "Kernel ObjectHomeAdmin '%s'. Check DCPS error log file for "
              "(possibly) more information.", DLRL_VALID_NAME(topicName),
              DLRL_VALID_NAME(DK_ObjectHomeAdmin_us_getName(home)));
      }
      _reader = gapi_dataReaderClaim(gapiReader, &result);
      DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result, "Failed to claim "
          "the data reader handle");
      ureader = u_reader(_DataReaderUreader(_reader));
      if(ureader)
      {
          /* now create a proxy to this user layer ureader which can be used by the DLRL
           * in a safe manner, as the user layer ureader returned by the _DataReaderUreader
           * operation is owned by the gapi.
           */
          ureader = u_reader(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(ureader)));
      }
      _EntityRelease(_reader);/* before the propagate */
      DLRL_Exception_PROPAGATE(exception);/* after the release */
      if(!ureader){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
              "Unable to create the DCPS DataReader entity for topic %s of DLRL "
              "Kernel ObjectHomeAdmin '%s'. Check DCPS error log file for "
              "(possibly) more information.", DLRL_VALID_NAME(topicName),
              DLRL_VALID_NAME(DK_ObjectHomeAdmin_us_getName(home)));
      }
      /* Do not need to dup, as the reader gave us a dupped value */
      *ls_reader = UPCAST_DLRL_LS_OBJECT(ccppReader);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return ureader;
  }

  u_writer
  ccpp_DCPSUtilityBridge_us_createDataWriter(
      DLRL_Exception* exception,
      void* userData,
      DK_TopicInfo* topicInfo,
      void** ls_writer)
  {
      DK_ObjectHomeAdmin* home = NULL;
      DK_CacheAdmin* cache = NULL;
      DDS::Publisher_ptr publisher = NULL;
      DDS::Topic_ptr topic = NULL;
      DDS::DataWriterQos writerQos;
      DDS::TopicQos topicQos;
      DDS::ReturnCode_t retCode = DDS::RETCODE_OK;
      DDS::DataWriter_ptr ccppWriter = NULL;
      gapi_dataWriter gapiWriter = NULL;
      _DataWriter _writer = NULL;
      gapi_returnCode_t result = GAPI_RETCODE_OK;
      u_writer uwriter = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(topicInfo);

      /* no duplicates done */
      home = DK_TopicInfo_us_getOwner(topicInfo);
      assert(home);
      cache = DK_ObjectHomeAdmin_us_getCache(home);
      assert(cache);
      publisher = DOWNCAST_DDS_PUBLISHER(DK_CacheAdmin_us_getLSPublisher(cache));
      assert(publisher);
      topic = DOWNCAST_DDS_TOPIC(DK_TopicInfo_us_getLSTopic(topicInfo));
      assert(topic);

      publisher->get_default_datawriter_qos(writerQos);
      topic->get_qos(topicQos);

      retCode = publisher->copy_from_topic_qos(writerQos, topicQos);
      DLRL_DcpsException_PROPAGATE(exception, retCode,"Unable to create "
          "DataWriter for topic %s. Copy from topic QoS failed!",
          DK_TopicInfo_us_getTopicName(topicInfo));
      ccppWriter = publisher->create_datawriter(topic, writerQos, NULL, 0);
      if(!ccppWriter){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Creation of data "
              "writer failed! Check DCPS error log file for (possibly) more "
              "information.");
      }

      gapiWriter = reinterpret_cast<gapi_dataWriter>(
          DDS_GET_GAPI_SELF(ccppWriter));
      if(!gapiWriter){
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Unable to create the "
              "DCPS DataWriter entity. Check DCPS error log file for (possibly) "
              "more information.");
      }
      _writer = gapi_dataWriterClaim(gapiWriter, &result);
      DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result, "Unable to claim "
          "data writer handle");
      uwriter = U_WRITER_GET(_writer);
      if(uwriter)
      {
          /* now create a proxy to this user layer topic which can be used by the DLRL
           * in a safe manner, as the user layer topic returned by the _TopicUtopic
           * operation is owned by the gapi.
           */
          uwriter = u_writer(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(uwriter)));
      }
      _EntityRelease(_writer);/* before the propagate */
      DLRL_Exception_PROPAGATE(exception);/* after the release */
      if(!uwriter)
      {
          DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Unable to create the "
              "DCPS DataWriter entity. Check DCPS error log file for (possibly) "
              "more information.");
      }
      /* Do not need to dup, as the create_writer gave us a dupped value */
      *ls_writer = UPCAST_DLRL_LS_OBJECT(ccppWriter);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return uwriter;
  }

  void
  ccpp_DCPSUtilityBridge_us_deleteDataReader(
      DLRL_Exception* exception,
      void* userData,
      DK_CacheAdmin* cache,
      u_reader reader,
      DLRL_LS_object ls_reader)
  {
      DDS::DataReader_ptr ccppReader = NULL;
      DDS::Subscriber_ptr ccppSubscriber = NULL;
      DDS::ReturnCode_t retCode = DDS::RETCODE_OK;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(cache);
      assert(ls_reader);

      ccppReader = DOWNCAST_DDS_READER(ls_reader);
      ccppSubscriber = DOWNCAST_DDS_SUBSCRIBER(
          DK_CacheAdmin_us_getLSSubscriber(cache));
      retCode = ccppSubscriber->delete_datareader(ccppReader);
      DLRL_DcpsException_PROPAGATE(exception, retCode, "Delete of datareader "
          "failed");
      CORBA::release(ccppReader);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_DCPSUtilityBridge_us_deleteDataWriter(
      DLRL_Exception* exception,
      void* userData,
      DK_CacheAdmin* cache,
      u_writer writer,
      DLRL_LS_object ls_writer)
  {
      DDS::DataWriter_ptr ccppWriter = NULL;
      DDS::Publisher_ptr ccppPublisher = NULL;
      DDS::ReturnCode_t retCode = DDS::RETCODE_OK;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(cache);
      assert(ls_writer);

      ccppWriter = DOWNCAST_DDS_WRITER(ls_writer);
      ccppPublisher = DOWNCAST_DDS_PUBLISHER(
          DK_CacheAdmin_us_getLSPublisher(cache));
      retCode = ccppPublisher->delete_datawriter(ccppWriter);
      DLRL_DcpsException_PROPAGATE(exception, retCode, "Delete of datawriter "
          "failed");
      CORBA::release(ccppWriter);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_DCPSUtilityBridge_us_deleteTopic(
      DLRL_Exception* exception,
      void* userData,
      DK_CacheAdmin* cache,
      DK_TopicInfo* topicInfo)
  {
      DDS::DomainParticipant_ptr ccppParticipant = NULL;
      DDS::Topic_ptr ccppTopic = NULL;
      DDS::ReturnCode_t retCode = DDS::RETCODE_OK;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(cache);
      assert(topicInfo);

      ccppParticipant = DOWNCAST_DDS_PARTICIPANT(
          DK_CacheAdmin_us_getLSParticipant(cache));
      ccppTopic = DOWNCAST_DDS_TOPIC(DK_TopicInfo_us_getLSTopic(topicInfo));

      retCode = ccppParticipant->delete_topic(ccppTopic);
      DLRL_DcpsException_PROPAGATE(exception, retCode, "Delete of topic failed");
      CORBA::release(ccppTopic);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_DCPSUtilityBridge_us_releaseTopicUserData(
      void* userData,
      void* topicUserData)
  {
      DLRL_INFO(INF_ENTER);

      assert(!userData);
      assert(topicUserData);

      /* just free the memory */
      os_free(topicUserData);

      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_DCPSUtilityBridge_us_enableEntity(
      DLRL_Exception* exception,
      void* userData,
      DLRL_LS_object ls_entity)
  {
      DDS::Entity_ptr ccppEntity = NULL;
      DDS::ReturnCode_t retCode = DDS::RETCODE_OK;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(ls_entity);

      ccppEntity = DOWNCAST_DDS_ENTITY(ls_entity);
      retCode = ccppEntity->enable();
      DLRL_DcpsException_PROPAGATE(exception, retCode,"Unable to enable entity.");

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void ccpp_ObjectBridge_us_clearLSObjectAdministration(
      DK_ObjectHomeAdmin* home,
      DLRL_LS_object ls_object)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(home);
      assert(ls_object);

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));

      targetObjectCachedData->clearLSObjectAdministration(ls_object);

      DLRL_INFO(INF_EXIT);
  }


/**************************************/
/********** ObjectHomeBridge **********/
/**************************************/

  void
  ccpp_ObjectHomeBridge_us_loadMetamodel(
      DLRL_Exception* exception,
      DK_ObjectHomeAdmin* home,
      void* userData)
  {
      DDS::ObjectHome_impl* lsHome = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(home);
      assert(!userData);

      lsHome =DOWNCAST_DDS_OBJECTHOME_IMPL(DK_ObjectHomeAdmin_us_getLSHome(home));

      try
      {
          lsHome->loadMetaModel();
      } DLRL_CCPP_EXCEPTION_CATCH_AND_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_unregisterAdminWithLSHome(
      void* userData,
      DLRL_LS_object ls_home,
      LOC_boolean isRegistered)
  {
      DLRL_INFO(INF_ENTER);

      assert(ls_home);
      assert(!userData);

      if(isRegistered){
          CORBA::release(reinterpret_cast<CORBA::Object_ptr>(ls_home));
      }
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_deleteUserData(
      void* userData,
      void* homeUserData)
  {
      DLRL_INFO(INF_ENTER);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_triggerListeners(
      DLRL_Exception* exception,
      void *userData,
      DK_ObjectHomeAdmin* home,
      Coll_List* newSamples,
      Coll_List* modifiedSamples,
      Coll_List* deletedSamples)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(!userData);
      assert(exception);
      assert(home);
      assert(newSamples);
      assert(modifiedSamples);
      assert(deletedSamples);

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* process new samples. */
      ccpp_ObjectHomeBridge_us_listenerWalk(
          exception,
          home,
          newSamples,
          targetObjectCachedData->invokeNewObjectCallback);
      DLRL_Exception_PROPAGATE(exception);

      /* process modified samples. */
      ccpp_ObjectHomeBridge_us_listenerWalk(
          exception,
          home,
          modifiedSamples,
          targetObjectCachedData->invokeModifiedObjectCallback);
      DLRL_Exception_PROPAGATE(exception);

      /* process deleted samples. */
      ccpp_ObjectHomeBridge_us_listenerWalk(
          exception,
          home,
          deletedSamples,
          targetObjectCachedData->invokeDeletedObjectCallback);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

} /* extern "C" */


void
ccpp_ObjectHomeBridge_us_listenerWalk(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    Coll_List* samples,
    ccpp_DlrlUtils_us_invokeXXXObjectCallback invokexxxObjectCallback)
{
    bool fullyProcessed = false;
    DK_ObjectHomeAdmin* currentHome = NULL;
    Coll_Set* listeners = NULL;
    Coll_Iter* listenerIter = NULL;
    Coll_Iter* objectIter = NULL;
    DLRL_LS_object anObject = NULL;
    DLRL_LS_object aListener = NULL;
    DK_ObjectAdmin* admin;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(home);
    assert(samples);
    assert(invokexxxObjectCallback);

    /* Iterate through the list of objects. */
    objectIter = Coll_List_getFirstElement(samples);
    try {
        while (objectIter) {
            bool fullyProcessed = false;
            admin = reinterpret_cast<DK_ObjectAdmin*>(
            	Coll_Iter_getObject(objectIter));
            anObject = DK_ObjectAdmin_us_getLSObject(admin);
            currentHome = home;

            /* Iterate from the current home to its top parent. */
            while (currentHome && !fullyProcessed) {
                listeners = DK_ObjectHomeAdmin_us_getListeners(currentHome);
                listenerIter = Coll_Set_getFirstElement(listeners);

                /* Iterate through the list of Listeners attached to the current home. */
                while(listenerIter){
                    aListener = reinterpret_cast<DLRL_LS_object>(
                    	Coll_Iter_getObject(listenerIter));

                    /* Invoke the current Listener for the current object.
                     * However, stop iterating to the parent-home when a listener returns TRUE. */
                    fullyProcessed = fullyProcessed | invokexxxObjectCallback(
                    	aListener,
                    	anObject);

                    /* Obtain the next listener attached to the current home. */
                    listenerIter = Coll_Iter_getNext(listenerIter);
                }
                /* Obtain the parent-home. */
                currentHome = DK_ObjectHomeAdmin_us_getParent(currentHome);
            }
            /* Obtain the next object. */
            objectIter = Coll_Iter_getNext(objectIter);
        }
    } DLRL_CCPP_EXCEPTION_CATCH_AND_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

extern "C"
{
  DLRL_LS_object
  ccpp_ObjectHomeBridge_us_createTypedObject(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DK_TopicInfo* topicInfo,
      DLRL_LS_object ls_topic,
      DK_ObjectAdmin* objectAdmin)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      ccpp_TypedTopicCache* typedTopicCachedData = NULL;
      DDS::ObjectRoot_impl* typedObject = NULL;
      DLRL_LS_object lsObject;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(home);
      assert(topicInfo);
      assert(objectAdmin);
      /* ls_topic may be null */

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* Use a cached type specific C++ function to instantiate the typed object*/
      typedObject = targetObjectCachedData->createTypedObject(exception);
      DLRL_Exception_PROPAGATE(exception);

      /* Store the pointer to the ObjectAdmin in the object. */
      DK_Entity_ts_duplicate(reinterpret_cast<DK_Entity*>(objectAdmin));
      typedObject->object = objectAdmin;

      /* If no corresponding topic sample if available yet, then instantiate one*/
      if(!ls_topic){
          typedTopicCachedData = reinterpret_cast<ccpp_TypedTopicCache*>(
              DK_TopicInfo_us_getTopicUserData(topicInfo));
          ls_topic = typedTopicCachedData->createTypedTopic(exception);
          DLRL_Exception_PROPAGATE(exception);
      }

      /* Store the pointer to the corresponding topic sample in the object. */
      lsObject = VB_UPCAST_DLRL_LS_OBJECT(typedObject);
      targetObjectCachedData->setCurrentTopic(lsObject, ls_topic);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);

      return lsObject;
  }

  void
  ccpp_ObjectHomeBridge_us_doCopyInForTopicOfObject(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DK_ObjectWriter* objWriter,
      DK_ObjectAdmin* objectAdmin,
      void* message,
      void* dataSample)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      DK_TopicInfo* topicInfo = NULL;
      ccpp_TypedTopicCache* typedTopicCachedData = NULL;
      DLRL_LS_object lsObject = NULL;
      DLRL_LS_object lsTopic = NULL;
      c_base base;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(home);
      assert(objWriter);
      assert(objectAdmin);
      assert(message);
      assert(dataSample);

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* get the cached C++ functions from the user data field stored in the
       * TopicInfo.
       */
      topicInfo = DK_ObjectWriter_us_getTopicInfo(objWriter);
      typedTopicCachedData = reinterpret_cast<ccpp_TypedTopicCache*>(
          DK_TopicInfo_us_getTopicUserData(topicInfo));
      assert(typedTopicCachedData);

      /* Obtain the pointer to the corresponding topic sample in the object. */
      lsObject = DK_ObjectAdmin_us_getLSObject(objectAdmin);
      lsTopic = targetObjectCachedData->getCurrentTopic(lsObject);

      /* Invoke the CopyIn function to translate the C++ sample to a database
       * sample.
       */
      base = c_getBase(c_object(message));
      typedTopicCachedData->copyIn(base, lsTopic, dataSample);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_setDefaultTopicKeys(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DK_TopicInfo* topicInfo,
      DLRL_LS_object ls_object,
      DK_ObjectID* oid,
      LOC_string className)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      ccpp_TypedTopicCache* typedTopicCachedData = NULL;
      DLRL_LS_object lsTopic = NULL;
      DDS::DLRLOid lsOid;

      DLRL_INFO(INF_ENTER);
      assert(exception);
      assert(home);
      assert(topicInfo);
      assert(ls_object);
      assert(oid);
      /* className may be null */

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* get the cached C++ functions from the user data field stored in the
       * TopicInfo.
       */
      typedTopicCachedData = reinterpret_cast<ccpp_TypedTopicCache*>(
          DK_TopicInfo_us_getTopicUserData(topicInfo));
      assert(typedTopicCachedData);

      /* Obtain the pointer to the corresponding topic sample in the object. */
      lsTopic = targetObjectCachedData->getCurrentTopic(ls_object);

      /* Obtain the C++ representation of the OID and write it in the sample. */
      lsOid.systemId = static_cast<CORBA::Long>(oid->oid[0]);
      lsOid.localId = static_cast<CORBA::Long>(oid->oid[1]);
      lsOid.serial = static_cast<CORBA::Long>(oid->oid[2]);
      typedTopicCachedData->setTopicOidField(lsTopic, lsOid);

      /* If a ClassName is present, also write it into the topic. */
      if (className)
      {
          typedTopicCachedData->setTopicClassName(lsTopic, className);
      }

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_createTypedObjectSeq(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* targetLockedHome,
      void** arg,
      LOC_unsigned_long size)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(targetLockedHome);
      assert(!(*arg));

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(targetLockedHome));
      assert(targetObjectCachedData);

      targetObjectCachedData->createTypedObjectSeq(exception, arg, size);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_addElementToTypedObjectSeq(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* targetLockedHome,
      void* arg,
      DLRL_LS_object lsObject,
      LOC_unsigned_long count)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(targetLockedHome);
      assert(arg);

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(targetLockedHome));
      assert(targetObjectCachedData);

      targetObjectCachedData->addElementToTypedObjectSeq(exception, arg, lsObject, count);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_createTypedSelectionSeq(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* targetLockedHome,
      void** arg,
      LOC_unsigned_long size)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(targetLockedHome);
      assert(!(*arg));

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(targetLockedHome));
      assert(targetObjectCachedData);

      targetObjectCachedData->createTypedSelectionSeq(exception, arg, size);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_addElementToTypedSelectionSeq(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* targetLockedHome,
      void* arg,
      DLRL_LS_object lsObject,
      LOC_unsigned_long count)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(targetLockedHome);
      assert(arg);

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(targetLockedHome));
      assert(targetObjectCachedData);

      targetObjectCachedData->addElementToTypedSelectionSeq(exception, arg, lsObject, count);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectHomeBridge_us_createTypedListenerSeq(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* targetLockedHome,
      void** arg,
      LOC_unsigned_long size)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(targetLockedHome);
      assert(!(*arg));

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(targetLockedHome));
      assert(targetObjectCachedData);

      targetObjectCachedData->createTypedListenerSeq(exception, arg, size);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }


  void
  ccpp_ObjectHomeBridge_us_addElementToTypedListenerSeq(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* targetLockedHome,
      void* arg,
      DLRL_LS_object lsObject,
      LOC_unsigned_long count)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(targetLockedHome);
      assert(arg);

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(targetLockedHome));
      assert(targetObjectCachedData);

      targetObjectCachedData->addElementToTypedListenerSeq(exception, arg, lsObject, count);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }


/**************************************/
/********* ObjectReaderBridge *********/
/**************************************/
  void
  ccpp_ObjectReaderBridge_us_updateObject(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DK_ObjectAdmin* object,
      DLRL_LS_object ls_topic)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      DLRL_LS_object lsObject = NULL;
      DLRL_LS_object currentTopic = NULL;
      DLRL_LS_object previousTopic = NULL;

      DLRL_INFO(INF_ENTER);
      assert(exception);
      assert(home);
      assert(object);
      /* ls_topic may be NULL */

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* Obtain the pointer to the corresponding C++ object. */
      lsObject = DK_ObjectAdmin_us_getLSObject(object);
      assert(lsObject);

      /* Replace the previous topic field by the current topic field. */
      previousTopic = targetObjectCachedData->getCurrentTopic(lsObject);
      targetObjectCachedData->setPreviousTopic(lsObject, previousTopic);

      /* set new topic sample as current topic. */
      targetObjectCachedData->setCurrentTopic(lsObject, ls_topic);

      /* Set the validity field for the previous topic if it has been modified. */
      if (previousTopic) {
          DDS::ObjectRoot_impl* lsObjectRoot;

          /* Obtain the C++ representation of the object. */
          lsObjectRoot = DOWNCAST_DDS_OBJECTROOT_IMPL(
              DK_ObjectAdmin_us_getLSObject(object));

          /* Set the validity for the previousTopic field. */
          lsObjectRoot->prevTopicValid = true;
      }

   /*not (yet) used:   DLRL_Exception_EXIT(exception);*/
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectReaderBridge_us_doLSReadPreProcessing(
      DK_ReadInfo* readInfo,
      DK_ObjectReader* objReader)
  {
      DK_TopicInfo* topicInfo = NULL;
      DLRL_LS_object ls_reader  = NULL;
      ccpp_TypedTopicCache* typedTopicCachedData = NULL;
      DLRL_Exception* exception = NULL;

      DLRL_INFO(INF_ENTER);
      assert(readInfo);
      assert(objReader);

      /* get the cached C++ functions from the user data field stored in the
       * TopicInfo.
       */
      topicInfo = DK_ObjectReader_us_getTopicInfo(objReader);
      assert(topicInfo);
      typedTopicCachedData = reinterpret_cast<ccpp_TypedTopicCache*>(
          DK_TopicInfo_us_getTopicUserData(topicInfo));
      assert(typedTopicCachedData);

      /* Get the exception struct. */
      exception = readInfo->exception;

      /* set C++ copy out routine. */
      readInfo->dstInfo = typedTopicCachedData;
      readInfo->copyOut = typedTopicCachedData->copyOut;

      /* continue the read in the kernel. */
      DK_ObjectReader_us_doRead(objReader, exception, readInfo);
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  DLRL_LS_object
  ccpp_ObjectReaderBridge_us_createLSTopic(
      DLRL_Exception* exception,
      DK_ObjectAdmin* objectAdmin,/* if this is null it means that you should create a new ls topic */
      void* dstInfo,
      void (*ls_copyOut)(void*, void*),
      void* sampleData)
  {
      DLRL_LS_object lsTopic = NULL;

      DLRL_INFO(INF_ENTER);
      assert(dstInfo);
      assert(ls_copyOut);

      /* If the object existed before, examine whether its previous topic can be
       * recycled.
       */
      if (objectAdmin) {
          DK_ObjectHomeAdmin* home = NULL;
          ccpp_TypedObjectCache* targetObjectCachedData = NULL;
          DLRL_LS_object lsObject = NULL;

          /* get the cached C++ functions from the user data field stored in the
           * ObjectHomeAdmin.
           */
          home = DK_ObjectAdmin_us_getHome(objectAdmin);
          targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
              DK_ObjectHomeAdmin_us_getUserData(home));
          assert(targetObjectCachedData);

          /* Attempt to access the previous topic field of the owning object. */
          lsObject = DK_ObjectAdmin_us_getLSObject(objectAdmin);
          lsTopic = targetObjectCachedData->getPreviousTopic(lsObject);
      }
      /* If the previous topic field cannot yet be recycled, create a new one. */
      if (!objectAdmin || !lsTopic) {
          ccpp_TypedTopicCache* typedTopicCachedData = NULL;

          /* get the cached C++ functions from the dstInfo user-data. */
          typedTopicCachedData = reinterpret_cast<ccpp_TypedTopicCache*>(dstInfo);
          assert(typedTopicCachedData);

          lsTopic = typedTopicCachedData->createTypedTopic(exception);
          DLRL_Exception_PROPAGATE(exception);
      }

      /* Now copy the state of the topic out of the database. */
      DLRL_INFO(INF_DCPS, "copy_out(...)");
      ls_copyOut(sampleData, lsTopic);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return lsTopic;
  }

  void ccpp_ObjectReaderBridge_us_setCollectionToLSObject(
       DLRL_Exception* exception,
       void* userData,
       DK_ObjectHomeAdmin* home,
       DK_ObjectAdmin* objectAdmin,
       DK_Collection* collection,
       LOC_unsigned_long collectionIndex)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      DLRL_LS_object ownerRoot;
      DLRL_LS_object lsCollection;

      DLRL_INFO(INF_ENTER);

      assert(!userData);
      assert(exception);
      assert(home);
      assert(objectAdmin);
      assert(collection);

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      targetObjectCachedData =reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      ownerRoot = DK_ObjectAdmin_us_getLSObject(objectAdmin);
      lsCollection = DK_Collection_us_getLSObject(collection);

      targetObjectCachedData->setCollections(
          ownerRoot,
          lsCollection,
          collectionIndex);

      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_ObjectReaderBridge_us_resetLSModificationInfo(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectAdmin* objectAdmin)
  {
      DDS::ObjectRoot_impl* lsObject;

      DLRL_INFO(INF_ENTER);
      assert(exception);
      assert(objectAdmin);
      /* userData may be NULL */

      /* Obtain the C++ representation of the object. */
      lsObject = DOWNCAST_DDS_OBJECTROOT_IMPL(
          DK_ObjectAdmin_us_getLSObject(objectAdmin));
      assert(lsObject);

      /* Reset the validity for the previousTopic field. */
      lsObject->prevTopicValid = false;

      DLRL_INFO(INF_EXIT);
  }

/******************************************************************/
/*****               ObjectRelationReaderBridge               *****/
/******************************************************************/


  void
  ccpp_ObjectRelationReaderBridge_us_setRelatedObjectForObject(
      void* userData,
      DK_ObjectHomeAdmin* ownerObjectHome,
      DK_ObjectAdmin* owner,
      LOC_unsigned_long relationIndex,
      DK_ObjectAdmin* relationObjectAdmin,
      LOC_boolean isValid)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      DLRL_LS_object ownerRoot;
      DLRL_LS_object targetRoot = NULL;

      DLRL_INFO(INF_ENTER);

      assert(!userData);
      assert(ownerObjectHome);
      assert(owner);
      /* relationObjectAdmin may be null, isValid is not used by ccpp api */

      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(ownerObjectHome));
      assert(targetObjectCachedData);

      ownerRoot = DK_ObjectAdmin_us_getLSObject(owner);
      if(relationObjectAdmin)
      {
          /* TODO: Measures need to be taken in case target is not dereferenced
           * yet. */
          targetRoot = DK_ObjectAdmin_us_getLSObject(relationObjectAdmin);
      }

      targetObjectCachedData->changeRelations(
          ownerRoot,
          targetRoot,
          relationIndex);

      DLRL_INFO(INF_EXIT);
  }

/**************************************/
/****** ObjectWriterReaderBridge ******/
/**************************************/
  u_instanceHandle
  ccpp_ObjectWriterBridge_us_registerInstance(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectWriter* objWriter,
      DK_ObjectAdmin* objectAdmin)
  {
      u_writer writer = NULL;
      DDS::Entity_impl* ls_writer = NULL;
      gapi_fooDataWriter gapiWriter = NULL;
      DK_TopicInfo* topicInfo = NULL;
      DLRL_LS_object lsObject = NULL;
      DLRL_LS_object currentTopic = NULL;
      gapi_foo* gapiFooTopic = NULL;
      DK_ObjectHomeAdmin* home = NULL;
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      gapi_instanceHandle_t gapiHandle;
      u_instanceHandle handle = DK_DCPSUtility_ts_getNilHandle();
      gapi_returnCode_t retCode = GAPI_RETCODE_OK;

      DLRL_INFO(INF_ENTER);
      assert(objWriter);
      assert(objectAdmin);
      assert(exception);
      /* userData may be null. */

      /* get the underlying kernel writer. */
      writer = DK_ObjectWriter_us_getWriter(objWriter);

      /* get the C++ specific writer, and use it to obtain the gapi writer. */
      ls_writer = DOWNCAST_DDS_ENTITY_IMPL(
          DK_ObjectWriter_us_getLSWriter(objWriter));
      assert(ls_writer);
      gapiWriter = ls_writer->get_gapi_self();

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      topicInfo = DK_ObjectWriter_us_getTopicInfo(objWriter);
      home = DK_TopicInfo_us_getOwner(topicInfo);
      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* get the the C++ specific object representation and use it to get to its
       * currentTopic field.
       */
      lsObject = DK_ObjectAdmin_us_getLSObject(objectAdmin);
      currentTopic = targetObjectCachedData->getCurrentTopic(lsObject);
      gapiFooTopic = reinterpret_cast<gapi_foo*>(currentTopic);

      /* Register currentTopic using the gapi writer and translate the resulting
       * handle to a kernel handle.
       */
      handle = (gapi_instanceHandle_t)gapi_fooDataWriter_register_instance(
          gapiWriter,
          gapiFooTopic);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return handle;
  }

  void
  ccpp_ObjectWriterBridge_us_write(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectWriter* writer,
      DK_ObjectAdmin* object)
  {
      DDS::Entity_impl* ls_writer = NULL;
      gapi_fooDataWriter gapiWriter = NULL;
      DK_TopicInfo* topicInfo = NULL;
      DLRL_LS_object lsObject = NULL;
      DLRL_LS_object currentTopic = NULL;
      gapi_foo* gapiFooTopic = NULL;
      DK_ObjectHomeAdmin* home = NULL;
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      gapi_instanceHandle_t gapiHandle;
      gapi_returnCode_t retCode;

      DLRL_INFO(INF_ENTER);
      assert(exception);
      assert(writer);
      assert(object);
      /* userData may be null. */
      assert(DK_ObjectAdmin_us_getWriteState(object) == DK_OBJECT_STATE_OBJECT_NEW ||
          DK_ObjectAdmin_us_getWriteState(object) == DK_OBJECT_STATE_OBJECT_MODIFIED);

      /* get the C++ specific writer, and use it to obtain the gapi writer. */
      ls_writer = DOWNCAST_DDS_ENTITY_IMPL(DK_ObjectWriter_us_getLSWriter(writer));
      assert(ls_writer);
      gapiWriter = ls_writer->get_gapi_self();

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      topicInfo = DK_ObjectWriter_us_getTopicInfo(writer);
      home = DK_TopicInfo_us_getOwner(topicInfo);
      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* get the the C++ specific object representation and use it to get to its
       * currentTopic field.
       */
      lsObject = DK_ObjectAdmin_us_getLSObject(object);
      currentTopic = targetObjectCachedData->getCurrentTopic(lsObject);
      gapiFooTopic = reinterpret_cast<gapi_foo*>(currentTopic);

      /* Translate the kernel handle to a gapi handle and write the sample with
       * it.
       */
      gapiHandle  = DK_ObjectAdmin_us_getHandle(object);
      retCode = gapi_fooDataWriter_write (gapiWriter, gapiFooTopic, gapiHandle);
      DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, retCode,
          "Unable to write object data.");

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }


  void
  ccpp_ObjectWriterBridge_us_destroy(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectWriter* writer,
      DK_ObjectAdmin* object)
  {
      DDS::Entity_impl* ls_writer = NULL;
      gapi_fooDataWriter gapiWriter = NULL;
      DK_TopicInfo* topicInfo = NULL;
      DLRL_LS_object lsObject = NULL;
      DLRL_LS_object currentTopic = NULL;
      gapi_foo* gapiFooTopic = NULL;
      DK_ObjectHomeAdmin* home = NULL;
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;
      gapi_instanceHandle_t gapiHandle;
      gapi_returnCode_t retCode;

      DLRL_INFO(INF_ENTER);
      assert(exception);
      assert(writer);
      assert(object);
      /* userData may be null. */
      assert(DK_ObjectAdmin_us_getWriteState(object) == DK_OBJECT_STATE_OBJECT_DELETED);

      /* get the C++ specific writer, and use it to obtain the gapi writer. */
      ls_writer = DOWNCAST_DDS_ENTITY_IMPL(DK_ObjectWriter_us_getLSWriter(writer));
      assert(ls_writer);
      gapiWriter = ls_writer->get_gapi_self();

      /* get the cached C++ functions from the user data field stored in the
       * ObjectHomeAdmin.
       */
      topicInfo = DK_ObjectWriter_us_getTopicInfo(writer);
      home = DK_TopicInfo_us_getOwner(topicInfo);
      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      /* get the the C++ specific object representation and use it to get to its
       * currentTopic field.
       */
      lsObject = DK_ObjectAdmin_us_getLSObject(object);
      currentTopic = targetObjectCachedData->getCurrentTopic(lsObject);
      gapiFooTopic = reinterpret_cast<gapi_foo*>(currentTopic);

      /* Translate the kernel handle to a gapi handle and write the sample with
       * it.
       */
      gapiHandle  = DK_ObjectAdmin_us_getHandle(object);
      retCode = gapi_fooDataWriter_dispose (gapiWriter, gapiFooTopic, gapiHandle);
      DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, retCode,
          "Unable to dispose object data.");

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

/***********************************************************/
/********************* SelectionBridge *********************/
/***********************************************************/
  DK_ObjectAdmin**
  ccpp_SelectionBridge_us_checkObjects(
      DLRL_Exception* exception,
      void* userData,
      DK_SelectionAdmin* selection,
      DLRL_LS_object filterCriterion,
      DK_ObjectAdmin** objectArray,
      LOC_unsigned_long size,
      LOC_unsigned_long* passedAdminsArraySize)
  {
      DDS::Selection_impl* lsSelection = NULL;
      CORBA::ULong i, cpp_size;
      DK_ObjectAdmin* anObjectAdmin = NULL;
      DDS::ObjectRoot* anObject;
      DDS::ObjectRootSeq lsObjects;
      void *blob = NULL;
      DK_ObjectAdmin** passedAdmins = NULL;

      DLRL_INFO(INF_ENTER);
      assert(exception);
      assert(selection);
      assert(filterCriterion);
      assert(objectArray);
      assert(passedAdminsArraySize);
      /* userData may be null. */

      /* Get the C++ representation of the Selection object. */
      lsSelection = DOWNCAST_DDS_SELECTION_IMPL(
          DK_SelectionAdmin_us_getLSSelection(selection));
      assert(lsSelection);

      /*Prepare a sequence containing the list of all ObjectRoots to be checked.*/
      cpp_size = static_cast<CORBA::ULong>(size);
      lsObjects.length(cpp_size);
      for(i = 0; i < cpp_size; i++) {
          anObjectAdmin = reinterpret_cast<DK_ObjectAdmin*>(objectArray[i]);
          anObject = DOWNCAST_DDS_OBJECTROOT(DK_ObjectAdmin_us_getLSObject(
              anObjectAdmin));
          assert(anObject);
          anObject->_add_ref();
          lsObjects[i] = anObject;
      }

      /* now we can call the C++ callback routine that will return a sequence of
       * all objects that pass the filter. before doing the callback, release the
       * admin mutex, we still have the update mutex locked though!
       */
      DLRL_ALLOC_WITH_SIZE(blob, (sizeof(DK_ObjectAdmin*) * size), exception,
          "Unable to allocate array container for ObjectAdmins!");
      passedAdmins = reinterpret_cast<DK_ObjectAdmin**>(blob);
      DK_SelectionAdmin_unlockHome(selection);
      cpp_size = lsSelection->check_objects(
          exception,
          filterCriterion,
          lsObjects,
          objectArray,
          passedAdmins);
      DK_SelectionAdmin_lockHome(selection);
      DLRL_Exception_PROPAGATE(exception);

      *passedAdminsArraySize = static_cast<LOC_unsigned_long>(cpp_size);
      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return passedAdmins;
  }

  LOC_boolean
  ccpp_ObjectHomeBridge_us_checkObjectForSelection(
      DK_ObjectHomeAdmin* home,
      DLRL_Exception* exception,
      void* userData,
      DK_SelectionAdmin* selection,
      DK_ObjectAdmin* objectAdmin)
  {
      LOC_boolean retVal = FALSE;
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(home);
      assert(exception);
      assert(!userData);
      assert(selection);
      assert(objectAdmin);

      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      retVal = targetObjectCachedData->checkObjectForSelection(
          exception,
          DK_SelectionAdmin_us_getLSFilter(selection),
          DK_ObjectAdmin_us_getLSObject(objectAdmin));
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
      return retVal;
  }

  void
  ccpp_SelectionBridge_us_triggerListenerInsertedObject(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DLRL_LS_object listener,
      DK_ObjectAdmin* objectAdmin)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(home);
      assert(exception);
      assert(!userData);
      assert(listener);
      assert(objectAdmin);

      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      targetObjectCachedData->triggerListenerInsertedObject(
          exception,
          listener,
          DK_ObjectAdmin_us_getLSObject(objectAdmin));
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_SelectionBridge_us_triggerListenerModifiedObject(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DLRL_LS_object listener,
      DK_ObjectAdmin* objectAdmin)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(home);
      assert(exception);
      assert(!userData);
      assert(listener);
      assert(objectAdmin);

      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      targetObjectCachedData->triggerListenerModifiedObject(
          exception,
          listener,
          DK_ObjectAdmin_us_getLSObject(objectAdmin));
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_SelectionBridge_us_triggerListenerRemovedObject(
      DLRL_Exception* exception,
      void* userData,
      DK_ObjectHomeAdmin* home,
      DLRL_LS_object listener,
      DK_ObjectAdmin* objectAdmin)
  {
      ccpp_TypedObjectCache* targetObjectCachedData = NULL;

      DLRL_INFO(INF_ENTER);

      assert(home);
      assert(exception);
      assert(!userData);
      assert(listener);
      assert(objectAdmin);

      targetObjectCachedData = reinterpret_cast<ccpp_TypedObjectCache*>(
          DK_ObjectHomeAdmin_us_getUserData(home));
      assert(targetObjectCachedData);

      targetObjectCachedData->triggerListenerRemovedObject(
          exception,
          listener,
          DK_ObjectAdmin_us_getLSObject(objectAdmin));
      DLRL_Exception_PROPAGATE(exception);

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

/**************************************/
/************ UtilityBridge ***********/
/**************************************/

  LOC_boolean
  ccpp_UtilityBridge_us_AreSameLSObjects(
      void* userData,
      DLRL_LS_object obj1,
      DLRL_LS_object obj2)
  {

      DLRL_INFO(INF_ENTER);

      assert(obj1);
      assert(obj2);
      assert(!userData);

      DLRL_INFO(INF_EXIT);
      return obj1 == obj2;
  }

  DLRL_LS_object
  ccpp_UtilityBridge_us_duplicateLSValuetypeObject(
      void* userData,
      DLRL_LS_object object)
  {
      CORBA::ValueBase* cppDubObject;

      DLRL_INFO(INF_ENTER);
      assert(!userData);
      assert(object);

      cppDubObject = reinterpret_cast<CORBA::ValueBase*>(object);
      cppDubObject->_add_ref();

      DLRL_INFO(INF_EXIT);
      return object;
  }

  void
  ccpp_UtilityBridge_us_releaseLSValuetypeObject(
      void* userData,
      DLRL_LS_object object)
  {
     CORBA::ValueBase* cppDubObject;

      DLRL_INFO(INF_ENTER);
      assert(!userData);
      assert(object);

      cppDubObject = reinterpret_cast<CORBA::ValueBase*>(object);
      cppDubObject->_remove_ref();

      DLRL_INFO(INF_EXIT);
  }

  DLRL_LS_object
  ccpp_UtilityBridge_us_duplicateLSInterfaceObject(
      void* userData,
      DLRL_LS_object object)
  {
      DLRL_LS_object dupObject = NULL;

      DLRL_INFO(INF_ENTER);
      assert(!userData);
      assert(object);

      dupObject = UPCAST_DLRL_LS_OBJECT(CORBA::Object::_duplicate(
          reinterpret_cast<CORBA::Object_ptr>(object)));

      DLRL_INFO(INF_EXIT);
      return dupObject;
  }

  void
  ccpp_UtilityBridge_us_releaseLSInterfaceObject(
      void* userData,
      DLRL_LS_object object)
  {
      DLRL_INFO(INF_ENTER);

      assert(!userData);
      assert(object);

      CORBA::release(reinterpret_cast<CORBA::Object_ptr>(object));

      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_UtilityBridge_us_createStringSeq(
      DLRL_Exception* exception,
      void* userData,
      void** arg,
      LOC_unsigned_long size)
  {
      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(!(*arg));

      if(!(*arg)){
          DDS::StringSeq* cppStringSeq;

          cppStringSeq = new DDS::StringSeq(size);
          DLRL_VERIFY_ALLOC(cppStringSeq, exception, "Unable to allocate memory");
          cppStringSeq->length(size);
          *arg = reinterpret_cast<void*>(cppStringSeq);
      }

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_UtilityBridge_us_createIntegerSeq(
      DLRL_Exception* exception,
      void* userData,
      void** arg,
      LOC_unsigned_long size)
  {
      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(!(*arg));

      if(!(*arg)){
          DDS::LongSeq* cppLongSeq;

          cppLongSeq = new DDS::LongSeq(size);
          DLRL_VERIFY_ALLOC(cppLongSeq, exception, "Unable to allocate memory");
          cppLongSeq->length(size);
          *arg = reinterpret_cast<void*>(cppLongSeq);
      }

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_UtilityBridge_us_addElementToStringSeq(
      DLRL_Exception* exception,
      void* userData,
      void* arg,
      LOC_unsigned_long count,
      void* key)
  {
      LOC_string value;
      DDS::StringSeq* seq;
      DLRL_INFO(INF_ENTER);

      assert(exception);
      assert(!userData);
      assert(arg);
      assert(key);

      seq = reinterpret_cast<DDS::StringSeq*>(arg);
      value = CORBA::string_dup((LOC_string)key);
      DLRL_VERIFY_ALLOC(value, exception, "Out of resources");
      (*seq)[count] = value;

      DLRL_Exception_EXIT(exception);
      DLRL_INFO(INF_EXIT);
  }

  void
  ccpp_UtilityBridge_us_addElementToIntegerSeq(
      DLRL_Exception* exception,
      void* userData,
      void* arg,
      LOC_unsigned_long count,
      void* key)
  {
      DDS::LongSeq* seq;
      DLRL_INFO(INF_ENTER);

      seq = reinterpret_cast<DDS::LongSeq*>(arg);
      (*seq)[count] = static_cast<CORBA::Long>(*(
          reinterpret_cast<LOC_long*>(key)));

      DLRL_INFO(INF_EXIT);
  }

} /* extern "C" */

/**************************************/
/*********** Initialization ***********/
/**************************************/

class
ccpp_DlrlInitializer{

    public:
        ccpp_DlrlInitializer(){
            ccpp_DlrlUtils_initializeDLRLKernel();
        }

    private:

        void ccpp_DlrlUtils_initializeDLRLKernel(){
            DK_CacheBridge* cacheBridge;
            DK_CacheAccessBridge* cacheAccessBridge;
            DK_CollectionBridge* collectionBridge;
            DK_DCPSUtilityBridge* dcpsUtilityBridge;
            DK_ObjectBridge* objectBridge;
            DK_ObjectHomeBridge* objectHomeBridge;
            DK_ObjectReaderBridge* objectReaderBridge;
            DK_ObjectRelationReaderBridge* objectRelationReaderBridge;
            DK_ObjectWriterBridge* objectWriterBridge;
            DK_SelectionBridge* selectionBridge;
            DK_UtilityBridge* utilityBridge;

            DLRL_INFO(INF_ENTER);

            cacheBridge = DK_CacheFactoryAdmin_us_getCacheBridge();
            cacheAccessBridge = DK_CacheFactoryAdmin_us_getCacheAccessBridge();
            collectionBridge = DK_CacheFactoryAdmin_us_getCollectionBridge();
            dcpsUtilityBridge = DK_CacheFactoryAdmin_us_getDcpsUtilityBridge();
            objectBridge = DK_CacheFactoryAdmin_us_getObjectBridge();
            objectHomeBridge = DK_CacheFactoryAdmin_us_getObjectHomeBridge();
            objectReaderBridge = DK_CacheFactoryAdmin_us_getObjectReaderBridge();
            objectRelationReaderBridge = DK_CacheFactoryAdmin_us_getObjectRelationReaderBridge();
            objectWriterBridge = DK_CacheFactoryAdmin_us_getObjectWriterBridge();
            selectionBridge = DK_CacheFactoryAdmin_us_getSelectionBridge();
            utilityBridge = DK_CacheFactoryAdmin_us_getUtilityBridge();

            cacheBridge->createPublisher = ccpp_CacheBridge_us_createPublisher;
            cacheBridge->createSubscriber = ccpp_CacheBridge_us_createSubscriber;
            cacheBridge->deletePublisher = ccpp_CacheBridge_us_deletePublisher;
            cacheBridge->deleteSubscriber = ccpp_CacheBridge_us_deleteSubscriber;
            cacheBridge->triggerListenersWithStartOfUpdates = ccpp_CacheBridge_us_triggerListenersWithStartOfUpdates;
            cacheBridge->triggerListenersWithEndOfUpdates = ccpp_CacheBridge_us_triggerListenersWithEndOfUpdates;
            cacheBridge->triggerListenersWithUpdatesEnabled = ccpp_CacheBridge_us_triggerListenersWithUpdatesEnabled;
            cacheBridge->triggerListenersWithUpdatesDisabled = ccpp_CacheBridge_us_triggerListenersWithUpdatesDisabled;
            cacheBridge->homesAction = ccpp_CacheBridge_us_homesAction;
            cacheBridge->listenersAction = ccpp_CacheBridge_us_listenersAction;
            cacheBridge->accessesAction = ccpp_CacheBridge_us_accessesAction;
            cacheBridge->objectsAction = ccpp_CacheBridge_us_objectsAction;
            cacheBridge->isDataAvailable = ccpp_CacheBridge_us_isDataAvailable;

            cacheAccessBridge->containedTypesAction = ccpp_CacheAccessBridge_us_containedTypesAction;
            cacheAccessBridge->containedTypeNamesAction = ccpp_CacheAccessBridge_us_containedTypeNamesAction;
            cacheAccessBridge->objectsAction = ccpp_CacheAccessBridge_us_objectsAction;
            cacheAccessBridge->invalidObjectsAction = ccpp_CacheAccessBridge_us_invalidObjectsAction;

            collectionBridge->createLSCollection = ccpp_CollectionBridge_us_createLSCollection;

            dcpsUtilityBridge->registerType = ccpp_DCPSUtilityBridge_us_registerType;
            dcpsUtilityBridge->createTopic = ccpp_DCPSUtilityBridge_us_createTopic;
            dcpsUtilityBridge->createDataReader = ccpp_DCPSUtilityBridge_us_createDataReader;
            dcpsUtilityBridge->createDataWriter = ccpp_DCPSUtilityBridge_us_createDataWriter;
            dcpsUtilityBridge->deleteDataReader = ccpp_DCPSUtilityBridge_us_deleteDataReader;
            dcpsUtilityBridge->deleteDataWriter = ccpp_DCPSUtilityBridge_us_deleteDataWriter;
            dcpsUtilityBridge->deleteTopic = ccpp_DCPSUtilityBridge_us_deleteTopic;
            dcpsUtilityBridge->releaseTopicUserData = ccpp_DCPSUtilityBridge_us_releaseTopicUserData;
            dcpsUtilityBridge->enableEntity = ccpp_DCPSUtilityBridge_us_enableEntity;

            objectBridge->setIsAlive = NULL;/* not needed by CCPP API */
            objectBridge->setIsRegistered = NULL;/* not needed by CCPP API */
            objectBridge->notifyWriteStateChange = NULL;/* not needed by CCPP API */
            objectBridge->clearLSObjectAdministration = ccpp_ObjectBridge_us_clearLSObjectAdministration;

            objectHomeBridge->loadMetamodel = ccpp_ObjectHomeBridge_us_loadMetamodel;
            objectHomeBridge->unregisterAdminWithLSHome = ccpp_ObjectHomeBridge_us_unregisterAdminWithLSHome;
            objectHomeBridge->deleteUserData = ccpp_ObjectHomeBridge_us_deleteUserData;
            objectHomeBridge->triggerListeners = ccpp_ObjectHomeBridge_us_triggerListeners;
            objectHomeBridge->createTypedObject = ccpp_ObjectHomeBridge_us_createTypedObject;
            objectHomeBridge->doCopyInForTopicOfObject = ccpp_ObjectHomeBridge_us_doCopyInForTopicOfObject;
            objectHomeBridge->setDefaultTopicKeys = ccpp_ObjectHomeBridge_us_setDefaultTopicKeys;
            objectHomeBridge->createTypedObjectSeq = ccpp_ObjectHomeBridge_us_createTypedObjectSeq;
            objectHomeBridge->addElementToTypedObjectSeq = ccpp_ObjectHomeBridge_us_addElementToTypedObjectSeq;
            objectHomeBridge->createTypedSelectionSeq = ccpp_ObjectHomeBridge_us_createTypedSelectionSeq;
            objectHomeBridge->addElementToTypedSelectionSeq = ccpp_ObjectHomeBridge_us_addElementToTypedSelectionSeq;
            objectHomeBridge->createTypedListenerSeq = ccpp_ObjectHomeBridge_us_createTypedListenerSeq;
            objectHomeBridge->addElementToTypedListenerSeq = ccpp_ObjectHomeBridge_us_addElementToTypedListenerSeq;
            objectHomeBridge->checkObjectForSelection = ccpp_ObjectHomeBridge_us_checkObjectForSelection;

            objectReaderBridge->updateObject = ccpp_ObjectReaderBridge_us_updateObject;
            objectReaderBridge->doLSReadPreProcessing = ccpp_ObjectReaderBridge_us_doLSReadPreProcessing;
            objectReaderBridge->setCollectionToLSObject = ccpp_ObjectReaderBridge_us_setCollectionToLSObject;
            objectReaderBridge->createLSTopic = ccpp_ObjectReaderBridge_us_createLSTopic;
            objectReaderBridge->resetLSModificationInfo = ccpp_ObjectReaderBridge_us_resetLSModificationInfo;

            objectRelationReaderBridge->setRelatedObjectForObject = ccpp_ObjectRelationReaderBridge_us_setRelatedObjectForObject;

            objectWriterBridge->registerInstance = ccpp_ObjectWriterBridge_us_registerInstance;
            objectWriterBridge->write = ccpp_ObjectWriterBridge_us_write;
            objectWriterBridge->destroy = ccpp_ObjectWriterBridge_us_destroy;

            selectionBridge->checkObjects = ccpp_SelectionBridge_us_checkObjects;
            selectionBridge->triggerListenerInsertedObject = ccpp_SelectionBridge_us_triggerListenerInsertedObject;
            selectionBridge->triggerListenerModifiedObject = ccpp_SelectionBridge_us_triggerListenerModifiedObject;
            selectionBridge->triggerListenerRemovedObject = ccpp_SelectionBridge_us_triggerListenerRemovedObject;

            utilityBridge->getThreadCreateUserData = NULL;/* not used within ccpp api */
            utilityBridge->doThreadAttach = NULL;/* not used within ccpp api */
            utilityBridge->doThreadDetach = NULL;/* not used within ccpp api */
            utilityBridge->getThreadSessionUserData = NULL;/* not used within ccpp api */
            utilityBridge->areSameLSObjects = ccpp_UtilityBridge_us_AreSameLSObjects;
            utilityBridge->createIntegerSeq = ccpp_UtilityBridge_us_createIntegerSeq;
            utilityBridge->createStringSeq = ccpp_UtilityBridge_us_createStringSeq;
            utilityBridge->addElementToStringSeq = ccpp_UtilityBridge_us_addElementToStringSeq;
            utilityBridge->addElementToIntegerSeq = ccpp_UtilityBridge_us_addElementToIntegerSeq;

            utilityBridge->duplicateLSValuetypeObject = ccpp_UtilityBridge_us_duplicateLSValuetypeObject;
            utilityBridge->releaseLSValuetypeObject = ccpp_UtilityBridge_us_releaseLSValuetypeObject;
            utilityBridge->duplicateLSInterfaceObject = ccpp_UtilityBridge_us_duplicateLSInterfaceObject;
            utilityBridge->releaseLSInterfaceObject = ccpp_UtilityBridge_us_releaseLSInterfaceObject;
            utilityBridge->localDuplicateLSInterfaceObject = ccpp_UtilityBridge_us_duplicateLSInterfaceObject;
            utilityBridge->localDuplicateLSValuetypeObject = ccpp_UtilityBridge_us_duplicateLSValuetypeObject;

            DLRL_INFO(INF_EXIT);
        }
};

static ccpp_DlrlInitializer initializer;

const char *
ccpp_DlrlUtils_returnCodeGAPIToString(
    gapi_returnCode_t returnCode)
{
    const char *returnString = NULL;

    DLRL_INFO(INF_ENTER);

    switch (returnCode)
    {
        case GAPI_RETCODE_ERROR:
            returnString = "RETCODE_ERROR";
            break;
        case GAPI_RETCODE_UNSUPPORTED:
            returnString = "RETCODE_UNSUPPORTED";
            break;
        case GAPI_RETCODE_BAD_PARAMETER:
            returnString = "RETCODE_BAD_PARAMETER";
            break;
        case GAPI_RETCODE_PRECONDITION_NOT_MET:
            returnString = "RETCODE_PRECONDITION_NOT_MET";
            break;
        case GAPI_RETCODE_OUT_OF_RESOURCES:
            returnString = "RETCODE_OUT_OF_RESOURCES";
            break;
        case GAPI_RETCODE_NOT_ENABLED:
            returnString = "RETCODE_NOT_ENABLED";
            break;
        case GAPI_RETCODE_IMMUTABLE_POLICY:
            returnString = "RETCODE_IMMUTABLE_POLICY";
            break;
        case GAPI_RETCODE_INCONSISTENT_POLICY:
            returnString = "RETCODE_INCONSISTENT_POLICY";
            break;
        case GAPI_RETCODE_ALREADY_DELETED:
            returnString = "RETCODE_ALREADY_DELETED";
            break;
        case GAPI_RETCODE_TIMEOUT:
            returnString = "RETCODE_TIMEOUT";
            break;
        case GAPI_RETCODE_NO_DATA:
            returnString = "RETCODE_NO_DATA";
            break;
        case GAPI_RETCODE_OK:
            returnString = "RETCODE_OK";
            break;
        default:
            returnString = "Unknown DCPS returncode";
            break;
    }
    return returnString;

    DLRL_INFO(INF_EXIT);
}

const char *
ccpp_DlrlUtils_returnCodeDCPSToString(
    DDS::ReturnCode_t returnCode)
{
    const char *returnString = NULL;

    DLRL_INFO(INF_ENTER);

    switch (returnCode)
    {
        case DDS::RETCODE_ERROR:
            returnString = "RETCODE_ERROR";
            break;
        case DDS::RETCODE_UNSUPPORTED:
            returnString = "RETCODE_UNSUPPORTED";
            break;
        case DDS::RETCODE_BAD_PARAMETER:
            returnString = "RETCODE_BAD_PARAMETER";
            break;
        case DDS::RETCODE_PRECONDITION_NOT_MET:
            returnString = "RETCODE_PRECONDITION_NOT_MET";
            break;
        case DDS::RETCODE_OUT_OF_RESOURCES:
            returnString = "RETCODE_OUT_OF_RESOURCES";
            break;
        case DDS::RETCODE_NOT_ENABLED:
            returnString = "RETCODE_NOT_ENABLED";
            break;
        case DDS::RETCODE_IMMUTABLE_POLICY:
            returnString = "RETCODE_IMMUTABLE_POLICY";
            break;
        case DDS::RETCODE_INCONSISTENT_POLICY:
            returnString = "RETCODE_INCONSISTENT_POLICY";
            break;
        case DDS::RETCODE_ALREADY_DELETED:
            returnString = "RETCODE_ALREADY_DELETED";
            break;
        case DDS::RETCODE_TIMEOUT:
            returnString = "RETCODE_TIMEOUT";
            break;
        case DDS::RETCODE_NO_DATA:
            returnString = "RETCODE_NO_DATA";
            break;
        case DDS::RETCODE_OK:
            returnString = "RETCODE_OK";
            break;
        default:
            returnString = "Unknown DCPS returncode";
            break;
    }
    return returnString;

    DLRL_INFO(INF_EXIT);
}

void
ccpp_DlrlUtils_us_handleException(
    DLRL_Exception* exception)
{
    DLRL_INFO(INF_ENTER);

    switch(exception->exceptionID)
    {
        case DLRL_NO_EXCEPTION:
            /* do nothing */
            break;
        case DLRL_DCPS_ERROR:
            DLRL_INFO(INF_EXIT);
            throw DDS::DCPSError(exception->exceptionMessage);
            break;
        case DLRL_BAD_HOME_DEFINITION:
            DLRL_INFO(INF_EXIT);
            throw DDS::BadHomeDefinition(exception->exceptionMessage);
            break;
        case DLRL_BAD_PARAMETER:
            DLRL_INFO(INF_EXIT);
            throw DDS::BadParameter(exception->exceptionMessage);
            break;
        case DLRL_SQL_ERROR:
            DLRL_INFO(INF_EXIT);
            throw DDS::SQLError(exception->exceptionMessage);
            break;
        case DLRL_NOT_FOUND:
            DLRL_INFO(INF_EXIT);
            throw DDS::NotFound(exception->exceptionMessage);
            break;
        case DLRL_ALREADY_EXISTING:
            DLRL_INFO(INF_EXIT);
            throw DDS::AlreadyExisting(exception->exceptionMessage);
            break;
        case DLRL_INVALID_OBJECTS:
            DLRL_INFO(INF_EXIT);
            throw DDS::InvalidObjects(exception->exceptionMessage);
            break;
        case DLRL_PRECONDITION_NOT_MET:
            DLRL_INFO(INF_EXIT);
            throw DDS::PreconditionNotMet(exception->exceptionMessage);
            break;
        case DLRL_OUT_OF_MEMORY:
            DLRL_INFO(INF_EXIT);
            throw DDS::OutOfMemory(exception->exceptionMessage);
            break;
         case DLRL_ALREADY_DELETED:
            DLRL_INFO(INF_EXIT);
            throw DDS::AlreadyDeleted(exception->exceptionMessage);
            break;
         case DLRL_NO_SUCH_ELEMENT:
            DLRL_INFO(INF_EXIT);
            throw DDS::NoSuchElement(exception->exceptionMessage);
            break;
         case DLRL_ERROR:
            DLRL_INFO(INF_EXIT);
            throw DDS::DLRLError(exception->exceptionMessage);
            break;
        default:
            assert(FALSE);/* unrecognized exception, not allowed. */
            break;
    }

    DLRL_INFO(INF_EXIT);
}
