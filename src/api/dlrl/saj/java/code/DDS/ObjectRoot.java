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
 * <P>The ObjectRoot is the abstract root for any DLRL class. ObjectRoots are used to represent
 * either objects
 * that are in the Cache or clones that are attached to a CacheAccess. A Cache object
 * and its clones have the same OID in common. The OID represents an Object IDentifier that
 * uniquely identifies a DLRL object in a system. An Object in the Cache and its clones
 * in the CacheAccess share the same OID because they represent the same DLRL object in the
 * system (but maybe at different moments in time).</P>
 * <P>DLRL Objects located in the Cache can only be instantiated and modified when samples
 * arrive in the DCPS: Cache objects represent the system state. DLRL Objects located in a
 * readable CacheAccess can represent the system state of a certain moment in time (a so
 * called snapshot), while DLRL Objects in a writable CacheAccess can represent the system
 * state as the application intends it to become (he can instantiate and modify objects
 * in such a CacheAccess directly, and write these modifications into the system).
 * DLRL Objects in a READ_WRITE CacheAccess can represent both at the same time: an object
 * can be cloned in a CacheAccess (representing the system state at the moment of cloning),
 * and can then be modified by the application to represent the intended changes to the
 * system.<P>
 * <P>A DLRL Object has two separate lifecycle states: one representing the changes introduced
 * by incoming modifications by the DCPS (a so called read_state), and one representing
 * modifications that have been made by the local application (a so called write_state). Since
 * a DLRL Object in a Cache or a READ_ONLY CacheAccess can only be modified by the DCPS and
 * not by an application, it has no write_state (it is set to VOID). Since an object in a
 * WRITE_ONLY CacheAccess cannot be updated by the DCPS, it has no read_state (it is set to
 * VOID). Changes introduced by the DCPS will always be reflected in the read_state, changes
 * introduced by the application will always be reflected in the write_state.</P>
 * <P>The lifecycle for the read_state is as follows: when an object instance appears in a Cache or
 * CacheAccess for the first time, its read_state is set to NEW. When in a subsequent update round
 * (either introduced by incoming data in the DCPS or by a manual refresh of the Cache or
 * CacheAccess in which it resides) its value does not get changed, its read_state changes into
 * NOT_MODIFIED. When it does get modified in a subsequent update round, its read_state changes to
 * MODIFIED. When in a subsequent update round the object gets deleted, its read_state changes
 * into DELETED. The following update round any object with a read_state of DELETED will be
 * cleaned by the DLRL from it's administration, any attempt to access the object will result
 * in an AlreadyDeleted exception</P>
 * <P>The lifecycle for the write_state is as follows: when an object instance appears in a
 * CacheAccess for the first time as a result of an update round, its write_state is set to
 * NOT_MODIFIED. When it appears in the CacheAccess as a result of a local object creation, its
 * write_state is set to NEW. In each subsequent update round (introduced by manual refresh of the
 * CacheAccess in which it resides) its write_state is reset to NOT_MODIFIED. When the object does
 * get modified by means of a local modification by the application, its write_state changes to
 * MODIFIED. It changes back to NOT_MODIFIED is the application writes the contents of the
 * CacheAccess into the system. When an application destroys the object, its write_state changes
 * into DELETED. Only when the contents of the CacheAccess are subsequently written into the system
 * will the object be removed from the CacheAccess. Once an object has been removed from the CacheAccess
 * any attempt to access that object may raise a runtime exception of AlreadyDeleted.</P>
 * <P>An object in a Cache or READ_ONLY CacheAccess cannot be modified or destroyed by the local
 * application. Any attempt to do so will result in a PreconditionNotMet being raised.</P>
 */
public abstract class ObjectRoot {

    private long admin;
	private boolean isAlive = true;//NOT IN DESIGN
	private boolean isRegistered = false;//NOT IN DESIGN
	private int writeState = DDS.ObjectState._OBJECT_VOID;//NOT IN DESIGN
	private DDS.CacheAccess access = null;//NOT IN DESIGN
    private DDS.DLRLOid oid = null;//NOT IN DESIGN
    private String name = null;//NOT IN DESIGN
    
    protected boolean prevTopicValid = false;//NOT IN DESIGN

	/**
	 * Returns the Object IDentifier (OID) of this DLRL Object.
	 *
	 * @return the OID of the DLRL Object.
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 */
	public final DDS.DLRLOid oid () throws DDS.AlreadyDeleted{
        /* only cache valid oids
         * TODO migrate this to the kernel, well that the oid attribute is fully managed from the kernel/jni layer
         * and this operation only return the oid
         */
        if(oid != null){
            if(!isAlive){
                throw new DDS.AlreadyDeleted("Unable to retrieve the oid. The object in question has already been deleted!");
            }
            return (DDS.DLRLOid)oid.clone();
        } else {
		    DLRLOid tmpOid = jniOid();

            if(tmpOid.systemId != 0 || tmpOid.localId != 0 || tmpOid.serial != 0){
                oid = tmpOid;
            }
            return (DDS.DLRLOid)tmpOid.clone();
        }
	}

    protected static final void copyOidFrom(DDS.ObjectRoot sourceHolder, DDS.DLRLOid target){
        if(sourceHolder.oid == null){
            sourceHolder.oid = sourceHolder.jniOid();
        }
        target.systemId = sourceHolder.oid.systemId;
        target.localId = sourceHolder.oid.localId;
        target.serial = sourceHolder.oid.serial;
        /* following code is for backwards compatibility and to be removed once the value array is no longer supported*/
        target.value[0] = sourceHolder.oid.value[0];
        target.value[1] = sourceHolder.oid.value[1];
        target.value[2] = sourceHolder.oid.value[2];
    }

    /**
     * Returns a reference to the corresponding {@link DDS.ObjectHome}, which is
     * the manager of all instances of a specific type.
     *
     * @return the corresponding {@link DDS.ObjectHome}.
     * @throws DDS.AlreadyDeleted if the current Object is already deleted.
     */
    public final DDS.ObjectHome object_home() throws DDS.AlreadyDeleted {
        return jniObjectHome();
    }

    protected static final String getMainTopicName(ObjectRoot source){
        if(source.name == null){
            source.name = source.jniMaintopicName();
        }
        return source.name;
    }
	/**
	 * Returns the index of the ObjectHome to which this DLRL Object belongs.
	 *
	 * @return the home index.
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 */
     //NOT IN DESIGN
    public final int home_index() throws DDS.AlreadyDeleted{
        return jniHomeIndex();
    }

	/**
	 * Returns an array containing the names of each relation which is seen
     * as invalid by the DLRL. A Relation is invalid when it is a NIL pointer
     * but was modeled as a mandatory relation or when the relation points to an
     * object that is marked to be deleted in the next write() operation.
     * This operation is intended for use when writing object changes and
     * calling the write() operation on the CacheAccess, if an exception is raised
     * then this utility function can be used to determine what is not correct.
     * This operation will always return a zero length array for objects in
     * a Cache.
	 *
	 * @return the home index.
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 */
    //NOT IN DESIGN
    public final String[] get_invalid_relations(){
        return jniGetInvalidRelations();
    }

	/**
	 * Returns the read state of this object. For unregistered objects and
     * for objects in a CacheAccess that were not inserted by means of a refresh
	 * of the contracts of the CacheAccess this state will be set to VOID.
	 *
	 * @return the read state of the DLRL object.
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 */
	public final DDS.ObjectState read_state () throws DDS.AlreadyDeleted{
        return DDS.ObjectState.from_int(jniReadState());
	}

	/**
	 * Returns the write state of this object. For unregistered objects and
     * for objects in a Cache or READ_ONLY CacheAccess
	 * this state will be set to VOID.
	 *
	 * @return the write state of the DLRL object.
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 */
	public final DDS.ObjectState write_state () throws DDS.AlreadyDeleted{
        if(!isAlive){
            throw new DDS.AlreadyDeleted("Unable to retrieve the write state. The object in question has already been deleted!");
        }
        return DDS.ObjectState.from_int(writeState);
	}

	/**
	 * Returns the CacheBase in which this DLRL object is contained.
	 *
	 * @return the Cache or CacheAccess in which the DLRL object is contained.
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 */
	public final DDS.CacheBase owner () throws DDS.AlreadyDeleted{
		return jniOwner();
	}

	/**
	 * Marks an object for destruction. This is only allowed in a writeable CacheAccess.
	 * When invoked on an object located in any other type of CacheBase, a PreconditionNotMet
	 * will be raised. The object is only deleted after the destruction is commited by means
	 * of a write operation on the CacheAccess in which it is located.
	 *
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 * @throws DDS.PreconditionNotMet if the object is not in a writeable CacheAccess.
	 */
	public final void destroy () throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniDestroy();
	}

	/**
	 * Returns whether the object, or one of the objects in its related ObjectScope, has
	 * been modified in the last update round. If the read_state() of this ObjectRoot
     * is OBJECT_NEW, then false is always returned.
	 *
	 * @param scope the scope of related objects that will be checked for applied
	 * modifications.
	 * @return whether the object (or a related object) has been modified in the last update round.
	 * @throws DDS.AlreadyDeleted if the object is already deleted.
	 */
	public final boolean is_modified (DDS.ObjectScope scope) throws DDS.AlreadyDeleted{
		return jniIsModified(scope.value());
	}

	//NOT IN DESIGN
	protected final void validateAndRegisterObjectChange(boolean isImmutable) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
		if(writeState == DDS.ObjectState._OBJECT_NOT_MODIFIED){
            //among other things this operation checks if the object is located in a writeable access.
			jniStateHasChanged(isImmutable);
		}
        if(!isAlive){
            throw new DDS.AlreadyDeleted("Unable to make changes to the object. The object in question has already been deleted!");
        }
        if(isImmutable && isRegistered){
            throw new DDS.PreconditionNotMet("Unable to change immutable value because the object in question is already registered!");
        }

	}

	//NOT IN DESIGN
	private native void jniStateHasChanged(boolean isImmutable) throws DDS.AlreadyDeleted;
	//NOT IN DESIGN
	protected native void jniChangeRelationship(int index,DDS.ObjectRoot object) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
	//NOT IN DESIGN
	protected native void jniChangeCollection(int index, DDS.StrMap collection) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
	//NOT IN DESIGN
	protected native void jniChangeCollection(int index, DDS.IntMap collection) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
	//NOT IN DESIGN
	protected native void jniChangeCollection(int index, DDS.Set collection) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;

	private native DDS.DLRLOid jniOid() throws DDS.AlreadyDeleted;
    private native int jniReadState() throws DDS.AlreadyDeleted;
    private native int jniWriteState() throws DDS.AlreadyDeleted;
    private native DDS.CacheBase jniOwner() throws DDS.AlreadyDeleted;
    private native void jniDestroy() throws DDS.AlreadyDeleted;
    private native boolean jniIsModified(int scope) throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot jniGet(int relationIndex);
	protected native DDS.ObjectHome jniObjectHome() throws DDS.AlreadyDeleted;
	private native void jniDeleteObjectRoot();
    private native String jniMaintopicName() throws DDS.AlreadyDeleted;
    //NOT IN DESIGN
    private native int jniHomeIndex();
    //NOT IN DESIGN
    private native String[] jniGetInvalidRelations();

    protected final void finalize(){
	    jniDeleteObjectRoot();
    }



} // class ObjectRoot
