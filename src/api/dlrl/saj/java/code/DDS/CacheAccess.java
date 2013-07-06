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
package DDS;

/**
 * <P>This class encapsulates the access to a set of objects. It
 * offers methods to refresh and write objects attached to it.
 * CacheAccess objects can be created in read mode, in order
 * to provide a consistent access to a subset of the Cache
 * without blocking the incoming updates or in write mode in
 * order to provide support for concurrent
 * modifications/updates threads.</P>
 *
 * The current implementation only supports writeable cache accesses.
 */
public interface CacheAccess extends DDS.CacheBase{

    /**
     * Returns the owning Cache object at which this CacheAccess
     * was created.
     *
	 * @return the owner Cache object of this CacheAccess object
	 * @throws DDS.AlreadyDeleted if the CacheAccess is already deleted.
     */
    DDS.Cache owner () throws DDS.AlreadyDeleted;

    /**
     * <P>Publishes all changes made to ObjectRoots in the scope of this
     * CacheAccess into the DCPS layer. Any object marked for destruction
     * will be actually destroyed after this operation and accessing such
     * ObjectRoots will raise an AlreadyDeleted exception.</P>
     * <P>This operation will raise a PreconditionNotMet exception if the CacheAccess is
     * created with an usage of READ_ONLY. When this exception occurs nothing is done
     * within the CacheAccess.</P>
     * <P>This operation will raise an InvalidObjects exception if one of the
     * objects contained within the CacheAccess has an invalid relation.
     * Relations are invalid when they point to a NULL pointer and are classified as mandatory
     * or when they point to an object which is marked for destruction in the next write operation.
     * Relations in this context also imply collections of an ObjectRoot.
     * Such collections may also contain invalid elements (IE elements
     * which represent ObjectRoots which are marked for destruction in the next write
     * operation). If such elements are contained within a collection, then the collection
     * is seen as invalid. Naturally NULL pointers can not be contained within a collection.
     * When this exception occurs nothing will be done in the CacheAccess, to find out
     * which objects have invalid objects the (@link #get_invalid_objects)
     * operation can be used. This operation will return all objects which have invalid relations.
     * The ObjectRoot provides utility operations to determine which relations are invalid.</P>
     * <P>This operation will raise a DCPSError exception if an error occurs
     * while trying to write the changes to DCPS. This exception is unrecoverable.</P>
     *
     * @throws DDS.DCPSError If an error occurred while writing the changes
     * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
     * @throws DDS.AlreadyDeleted if the CacheAccess is already deleted.
     * @throws DDS.InvalidObjects If invalid objects are detected which prevent the write operation to continue
     */
    void write () throws DDS.DCPSError, DDS.PreconditionNotMet, DDS.AlreadyDeleted, DDS.InvalidObjects;


    /**
     * <P>This is a utility function designed to easily retrieve objects within the
     * CacheAccess which can cause a write to fail. It is recommended to only use
     * this operation if a write operation fails due to invalid objects, as the write operation itself
     * performs the check for invalid objects as well, making it unneccesary to
     * perform the check before the write in application context. Invalid objects are defined as
     * ObjectRoots which have relations to other ObjectRoots that are marked for destruction
     * in the next write() operation. Or ObjectRoots which have relations which are null
     * pointers, but which the Object Model defined as mandatory relations I.E. a cardinality of 1 instead
     * of 0..1. </P>
     *
	 * @throws DDS.AlreadyDeleted if the CacheAccess is already deleted.
     * @return An array with all ObjectRoots which have invalid relations. Or a zero length array
     * if no invalid objects are found.
     */
     //NOT IN DESIGN
    DDS.ObjectRoot[] get_invalid_objects() throws DDS.AlreadyDeleted;

    /**
     * <P>Detach all ObjectRoots and Contracts (including the contracted DLRL Objects themselves) from the
     * CacheAccess.</P>
     * <P>If the CacheAccess is writeable then the CacheAccess will unregister itself for each purged ObjectRoot
     * at the respective DCPS data writer entity. If the CacheAccess was the last writeable CacheAccess
     * registered for that ObjectRoot instance within the scope of the owning Cache then an explicit unregister
     * is performed for that instance on DCPS topic level. The default QoS settings for data writers
     * (specifically auto dispose unregistered entities) created by DLRL will ensure an explicit dispose is also
     * propagated throughout the system.</P>
     * <P>However if the QoS for auto dispose unregistered entities is set to false, then the unregister will still be
     * performed but dependant on the fact if no other writers of that instance exist the instance reaches a
     * instance state of NOT_ALIVE_NO_WRITERS. If other writers of that instance do exist, then the instance
     * state wont change in this scenario.</P>
     *
	 * @throws DDS.AlreadyDeleted if the CacheAccess is already deleted.
     */
    void purge () throws DDS.AlreadyDeleted, DDS.DCPSError;

    /**
     * A list of indexes that represents the indexes of the ObjectHomes for which the
     * CacheAccess contains at least one object.
     *
	 * @throws DDS.AlreadyDeleted if the CacheAccess is already deleted.
     */
    int[] contained_types() throws DDS.AlreadyDeleted;

    /**
     * A list of names that represent the names of the ObjectHomes for which the
     * CacheAccess contains at least one object. Whenever possible use the
     * {@link #contained_types} instead, as it is more performance efficient then
     * using String identifiers.
     *
	 * @throws DDS.AlreadyDeleted if the CacheAccess is already deleted.
     */
    String[] type_names() throws DDS.AlreadyDeleted;

    /**
     * The current implementation does not yet support the notion of a
     * Contract, therefore this operation is not yet supported.
     */
    DDS.Contract[] contracts() throws DDS.AlreadyDeleted;

    /**
     * The current implementation does not yet support the notion of a
     * Contract, therefore this operation is not yet supported.
     */
    DDS.Contract create_contract (DDS.ObjectRoot object, DDS.ObjectScope scope, int depth) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;

    /**
     * The current implementation does not yet support the notion of a
     * Contract, therefore this operation is not yet supported.
     */
    void delete_contract (DDS.Contract a_contract) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;

} // interface CacheAccess
