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
#ifndef CCPP_SELECTION_H
#define CCPP_SELECTION_H

#include "ccpp_dlrl.h"
#include "DLRL_Kernel.h"
#include "ccpp_ObjectHome_impl.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    /**
     * <P>A Selection provides the means to designate a subset of DLRL object
     * instances of a specific type. A selection determines the correct subset
     * by means of the provided selection criterion. Selections can be created
     * and deleted by the corresponding typed ObjectHome. </P>
     *
     * <P>The current implementation of the selection does not support
     * QueryCondition as a valid SelectionCriterion. By default every selection
     * will be created with auto_refresh = <code>false</code>. The value of
     * concerns_contained is ignored.
     */
    class OS_DLRL_API Selection_impl :
        public virtual DDS::Selection,
        public LOCAL_REFCOUNTED_OBJECT
    {

        friend class DDS::ObjectHome_impl;

        friend DK_ObjectAdmin**
            (::ccpp_SelectionBridge_us_checkObjects) (
                DLRL_Exception* exception,
                void* userData,
                DK_SelectionAdmin* selection,
                DLRL_LS_object filterCriterion,
                DK_ObjectAdmin** objectArray,
                LOC_unsigned_long size,
                LOC_unsigned_long* passedAdminsArraySize);

        protected:
            DK_SelectionAdmin* selection;
            Selection_impl();

            virtual ~Selection_impl();

            virtual CORBA::ULong check_objects(
                DLRL_Exception* exception,
                DLRL_LS_object filterCriterion,
                const DDS::ObjectRootSeq& lsObjects,
                DK_ObjectAdmin** inputAdmins,
                DK_ObjectAdmin** passedAdmins) = 0;

        public:

            /**
             * This operation returns if the selection can be manually
             * refreshed using the refresh operation of the selection
             * (<code>false</code>) or if the selection is automatically
             * refreshed whenever the related {@link DDS::Cache_impl} is
             * refreshed (<code>true</code>).
             *
             * <P>Take note that it does not matter
             * if the related cache is in enabled or disabled update mode.</P>
             *
             * <P>The current implementation will always return false.</P>
             *
             * @throws DDS::AlreadyDeleted if the Selection is already deleted.
             * @return <code>true</code> if the Selection will be automatically
             * refreshed and <code>false</code> if it will not.
             */
            virtual CORBA::Boolean
            auto_refresh(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * This operation returns true if the Selection considers
             * change to the contained relations of it's member objects
             * as a modification to itself and false if it only takes
             * changes to it's member objects into account.
             *
             * <P>This feature is
             * only usefull when using the selection in combination with
             * a selection listener, which is currently unsupported</P>
             *
             * <P>The current implementation will always return false.</P>
             *
             * @throws DDS::AlreadyDeleted if the Selection is already deleted.
             * @return <code>true</code> if the Selection sees modifications
             * to its contained members as modifications to itself and
             * <code>false</code> otherwise.
             */
            virtual CORBA::Boolean
            concerns_contained(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * This operation returns the DDS::SelectionCriterion class
             * that belongs to the Selection.
             *
             * @throws DDS::AlreadyDeleted if the Selection is already deleted.
             * @return the criterion that belongs to the selection
             */
            virtual DDS::SelectionCriterion_ptr
            criterion(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * This operation updates the membership of the selection.
             *
             * <P>Any
             * objects that no longer pass the criterion are removed and
             * objects that now match the criterion are added. If this operation
             * is called the {@link DDS::Selection_impl#auto_refresh} returns
             * <code>true</code> then this operation is considered a no-op.</P>
             *
             * @throws DDS::AlreadyDeleted if the Selection is already deleted.
             */
            virtual void
            refresh(
                ) THROW_ORB_EXCEPTIONS;
    };
};

#endif /* CCPP_SELECTION_H */
