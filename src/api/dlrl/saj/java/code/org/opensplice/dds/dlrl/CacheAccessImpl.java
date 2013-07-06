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
package org.opensplice.dds.dlrl;

/** 
 * Prismtech implementation of the {@link DDS.CacheAccess} interface. 
 */ 
public final class CacheAccessImpl implements DDS.CacheAccess { 

    private long admin;
	private boolean alive = true;//NOT IN DESIGN
	private DDS.CacheUsage purpose;//NOT IN DESIGN

	private CacheAccessImpl(){}//disallow usage of the default constructor
	
	//NOT IN DESIGN
	CacheAccessImpl(DDS.CacheUsage purpose){
		this.purpose = purpose;
	}

    /* see DDS.CacheAccess for javadoc */ 
    public final DDS.CacheUsage cache_usage() throws DDS.AlreadyDeleted {
		if(!alive){
			throw new DDS.AlreadyDeleted("The CacheAccess has already been deleted.");
		}
		return purpose;
    }

    /* see DDS.CacheAccess for javadoc */  
    //NOT IN DESIGN
    public final DDS.ObjectRoot[] get_invalid_objects() throws DDS.AlreadyDeleted{
        return jniGetInvalidObjects();
    }    

    /* see DDS.CacheAccess for javadoc */ 
    public final DDS.Cache owner() throws DDS.AlreadyDeleted {
        return jniOwner();
    }

    /* see DDS.CacheBase for javadoc */ 
    public final void refresh() throws DDS.DCPSError, DDS.PreconditionNotMet, DDS.AlreadyDeleted {
        jniRefresh();
    }

    /* see DDS.CacheAccess for javadoc */ 
    public final void write() throws DDS.DCPSError, DDS.PreconditionNotMet, DDS.AlreadyDeleted, DDS.InvalidObjects{
        jniWrite();
    }

    /* see DDS.CacheAccess for javadoc */ 
    public final void purge() throws DDS.AlreadyDeleted {
        jniPurge();
    }

    /* see DDS.CacheAccess for javadoc */ 
	public final int[] contained_types() throws DDS.AlreadyDeleted {
		return jniContainedTypes();
	}

    /* see DDS.CacheAccess for javadoc */ 
    public final String[] type_names() throws DDS.AlreadyDeleted{
        return jniTypeNames();
    }

    /* see DDS.CacheAccess for javadoc */ 
	public final DDS.Contract[] contracts() throws DDS.AlreadyDeleted {
		return jniContracts();
	}

    /* see DDS.CacheBase for javadoc */ 
    public final DDS.CacheKind kind() throws DDS.AlreadyDeleted{
        return DDS.CacheKind.CACHEACCESS_KIND;
    }

    /* see DDS.CacheAccess for javadoc */ 
	public final DDS.Contract create_contract (DDS.ObjectRoot object, DDS.ObjectScope scope, int depth) 
																		throws DDS.AlreadyDeleted {
		return jniCreateContract(object, scope.value(), depth);
	}

    /* see DDS.CacheAccess for javadoc */ 
	public final void delete_contract (DDS.Contract contract) throws DDS.AlreadyDeleted {
		jniDeleteContract(contract);
	}

    /* see DDS.CacheBase for javadoc */ 
    public final DDS.ObjectRoot[] objects() throws DDS.AlreadyDeleted{
        return jniObjects();
    }

    protected final void finalize(){
	    jniDeleteCacheAccess();
    }

    private native DDS.Cache jniOwner() throws DDS.AlreadyDeleted;
    private native void jniRefresh() throws DDS.DCPSError;
    private native void jniWrite() throws DDS.DCPSError, DDS.PreconditionNotMet, DDS.AlreadyDeleted, DDS.InvalidObjects;
    private native void jniPurge() throws DDS.AlreadyDeleted;
    private native DDS.ObjectRoot[] jniObjects() throws DDS.AlreadyDeleted;
    private native void jniDeleteCacheAccess();
	private native int[] jniContainedTypes() throws DDS.AlreadyDeleted;
	private native String[] jniTypeNames() throws DDS.AlreadyDeleted;
    private native DDS.Contract[] jniContracts() throws DDS.AlreadyDeleted;
	private native DDS.Contract jniCreateContract(DDS.ObjectRoot object, int scope, int depth)
														throws DDS.AlreadyDeleted;
	private native void jniDeleteContract(DDS.Contract contract) throws DDS.AlreadyDeleted;
    //NOT IN DESIGN
    private native DDS.ObjectRoot[] jniGetInvalidObjects() throws DDS.AlreadyDeleted;
}