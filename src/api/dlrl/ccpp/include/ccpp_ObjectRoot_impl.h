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
#ifndef CCPP_OBJECTROOT_H
#define CCPP_OBJECTROOT_H

#include "ccpp_dlrl.h"
#include "DLRL_Kernel.h"
#include "ccpp_DlrlUtils.h"
#include "ccpp_StrMap_impl.h"
#include "ccpp_IntMap_impl.h"
#include "ccpp_Set_impl.h"
#include "ccpp_ObjectHome_impl.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    /**
     * <P>The ObjectRoot is the abstract root for any DLRL class. ObjectRoots
     * are used to represent either objects that are in the Cache or clones that
     * are attached to a CacheAccess. A Cache object and its clones have the
     * same OID in common. The OID represents an Object IDentifier that uniquely
     * identifies a DLRL object in a system. An Object in the Cache and its
     * clones in the CacheAccess share the same OID because they represent the
     * same DLRL object in the system (but maybe at different moments in
     * time).</P>
     * <P>DLRL Objects located in the Cache can only be instantiated and
     * modified when samples arrive in the DCPS: Cache objects represent the
     * system state. DLRL Objects located in a readable CacheAccess can
     * represent the system state of a certain moment in time (a so called
     * snapshot), while DLRL Objects in a writable CacheAccess can represent
     * the system state as the application intends it to become (he can
     * instantiate and modify objects in such a CacheAccess directly, and write
     * these modifications into the system).
     * DLRL Objects in a READ_WRITE CacheAccess can represent both at the same
     * time: an object can be cloned in a CacheAccess (representing the system
     * state at the moment of cloning), and can then be modified by the
     * application to represent the intended changes to the system.<P>
     * <P>A DLRL Object has two separate lifecycle states: one representing the
     * changes introduced by incoming modifications by the DCPS (a so called
     * read_state), and one representing modifications that have been made by
     * the local application (a so called write_state). Since a DLRL Object in a
     * Cache or a READ_ONLY CacheAccess can only be modified by the DCPS and
     * not by an application, it has no write_state (it is set to VOID). Since
     * an object in a WRITE_ONLY CacheAccess cannot be updated by the DCPS, it
     * has no read_state (it is set to VOID). Changes introduced by the DCPS
     * will always be reflected in the read_state, changes introduced by the
     * application will always be reflected in the write_state.</P>
     * <P>The lifecycle for the read_state is as follows: when an object
     * instance appears in a Cache or CacheAccess for the first time, its
     * read_state is set to NEW. When in a subsequent update round (either
     * introduced by incoming data in the DCPS or by a manual refresh of the
     * Cache or CacheAccess in which it resides) its value does not get changed,
     * its read_state changes into NOT_MODIFIED. When it does get modified in a
     * subsequent update round, its read_state changes to MODIFIED. When in a
     * subsequent update round the object gets deleted, its read_state changes
     * into DELETED. The following update round any object with a read_state of
     * DELETED will be cleaned by the DLRL from it's administration, any attempt
     * to access the object will result in an AlreadyDeleted exception</P>
     * <P>The lifecycle for the write_state is as follows: when an object
     * instance appears in a CacheAccess for the first time as a result of an
     * update round, its write_state is set to NOT_MODIFIED. When it appears in
     * the CacheAccess as a result of a local object creation, its write_state
     * is set to NEW. In each subsequent update round (introduced by manual
     * refresh of the CacheAccess in which it resides) its write_state is reset
     * to NOT_MODIFIED. When the object does get modified by means of a local
     * modification by the application, its write_state changes to MODIFIED.
     * It changes back to NOT_MODIFIED is the application writes the contents
     * of the CacheAccess into the system. When an application destroys the
     * object, its write_state changes into DELETED. Only when the contents of
     * the CacheAccess are subsequently written into the system will the object
     * be removed from the CacheAccess. Once an object has been removed from
     * the CacheAccess any attempt to access that object may raise a runtime
     * exception of AlreadyDeleted.</P>
     * <P>An object in a Cache or READ_ONLY CacheAccess cannot be modified or
     * destroyed by the local application. Any attempt to do so will result in
     * a PreconditionNotMet being raised.</P>
     */
    class OS_DLRL_API ObjectRoot_impl :
        public virtual DDS::ObjectRoot,
        public LOCAL_REFCOUNTED_VALUEBASE
    {

        friend class DDS::IntMap_impl;
        friend class DDS::StrMap_impl;
        friend class DDS::Set_impl;
        friend class DDS::ObjectHome_impl;

        friend DLRL_LS_object
            (::ccpp_ObjectHomeBridge_us_createTypedObject) (
                DLRL_Exception* exception,
                void* userData,
                DK_ObjectHomeAdmin* home,
                DK_TopicInfo* topicInfo,
                DLRL_LS_object ls_topic,
                DK_ObjectAdmin* objectAdmin);
        friend void
            (::ccpp_ObjectReaderBridge_us_resetLSModificationInfo) (
                DLRL_Exception* exception,
                void* userData,
                DK_ObjectAdmin* objectAdmin);
        friend void
            (::ccpp_ObjectReaderBridge_us_updateObject) (
                DLRL_Exception* exception,
                void* userData,
                DK_ObjectHomeAdmin* home,
                DK_ObjectAdmin* object,
                DLRL_LS_object ls_topic);

        private:
            DDS::DLRLOid myOid;
            virtual CORBA::ValueBase* _copy_value();

        protected:
            DK_ObjectAdmin* object;


        protected:
            CORBA::Boolean prevTopicValid;

            DK_ObjectAdmin*
            getObjectRootAdmin(
                DLRL_Exception* exception,
                DDS::ObjectRoot* objectRoot,
                const char* name);

            DK_MapAdmin*
            getIntMapAdmin(
                DLRL_Exception* exception,
                DDS::IntMap* collection,
                const char* name);

            DK_MapAdmin*
            getStrMapAdmin(
                DLRL_Exception* exception,
                DDS::StrMap* collection,
                const char* name);

            DK_SetAdmin*
            getSetAdmin(
                DLRL_Exception* exception,
                DDS::Set* collection,
                const char* name);

            ObjectRoot_impl();

            virtual ~ObjectRoot_impl();

            virtual void
            m_oid(
                const DDS::DLRLOid &) THROW_VALUETYPE_ORB_EXCEPTIONS;

            virtual const DDS::DLRLOid &
            m_oid(
                void) const THROW_VALUETYPE_ORB_EXCEPTIONS;

            virtual DDS::DLRLOid &
            m_oid(
                void) THROW_VALUETYPE_ORB_EXCEPTIONS;

            virtual void
            m_class_name(
                char *) THROW_VALUETYPE_ORB_EXCEPTIONS;

            virtual void
            m_class_name(
                const char *) THROW_VALUETYPE_ORB_EXCEPTIONS;

            virtual void
            m_class_name(
                const CORBA::String_var&) THROW_VALUETYPE_ORB_EXCEPTIONS;

            virtual const char *
            m_class_name(
                void) const THROW_VALUETYPE_ORB_EXCEPTIONS;

        public:
            /**
             * Returns the Object IDentifier (OID) of this DLRL Object.
             *
             * @return the OID of the DLRL Object.
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             */
            virtual DDS::DLRLOid
            oid(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the index of the ObjectHome to which this DLRL Object
             * belongs.
             *
             * @return the home index.
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             */
            virtual CORBA::Long
            home_index(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the read state of this object. For unregistered objects
             * and for objects in a CacheAccess that were not inserted by means
             * of a refresh of the contracts of the CacheAccess this state will
             * be set to VOID.
             *
             * @return the read state of the DLRL object.
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             */
            virtual DDS::ObjectState
            read_state(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the write state of this object. For unregistered objects
             * and for objects in a Cache or READ_ONLY CacheAccess this state
             * will be set to VOID.
             *
             * @return the write state of the DLRL object.
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             */
            virtual DDS::ObjectState
            write_state (
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns a reference to the corresponding
             * {@link DDS::ObjectHome_impl}, which is the manager of all
             * instances of a specific type.
             *
             * @return the corresponding {@link DDS::ObjectHome_impl}.
             * @throws DDS::AlreadyDeleted if the current Object is already
             * deleted.
             */
            virtual DDS::ObjectHome_ptr
            object_home(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the CacheBase in which this DLRL object is contained.
             *
             * @return the Cache or CacheAccess in which the DLRL object is
             * contained.
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             */
            virtual DDS::CacheBase_ptr
            owner(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Marks an object for destruction. This is only allowed in a
             * writeable CacheAccess. When invoked on an object located in any
             * other type of CacheBase, a PreconditionNotMet will be raised. The
             * object is only deleted after the destruction is commited by means
             * of a write operation on the CacheAccess in which it is located.
             *
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             * @throws DDS::PreconditionNotMet if the object is not in a
             * writeable CacheAccess.
             */
            virtual void
            destroy(
                ) THROW_VALUETYPE_ORB_AND_USER_EXCEPTIONS(
                    DDS::PreconditionNotMet);

            /**
             * Returns whether the object, or one of the objects in its related
             * ObjectScope, has been modified in the last update round. If the
             * read_state() of this ObjectRoot is OBJECT_NEW, then false is
             * always returned.
             *
             * @param scope the scope of related objects that will be checked
             * for applied modifications.
             * @return whether the object (or a related object) has been
             * modified in the last update round.
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             */
            virtual CORBA::Boolean
            is_modified(
                DDS::ObjectScope scope) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns a sequence containing the names of each relation which is
             * seen as invalid by the DLRL. A Relation is invalid when it is a
             * NIL pointer but was modeled as a mandatory relation or when the
             * relation points to an object that is marked to be deleted in the
             * next write() operation. This operation is intended for use when
             * writing object changes and calling the write() operation on the
             * CacheAccess, if an exception is raised then this utility
             * function can be used to determine what is not correct. This
             * operation will always return a zero length array for objects in
             * a Cache.
             *
             * @return the home index.
             * @throws DDS::AlreadyDeleted if the object is already deleted.
             */
            virtual DDS::StringSeq *
            get_invalid_relations(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

    };
};

#endif /* CCPP_OBJECTROOT_H */
