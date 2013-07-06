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
 * <P>The CacheBase defines the common interface for all cache-like classes.
 * I.E Cache and CacheAccess. It defines operations to retrieve the usage of the cache,
 * to refresh the cache and to retrieve all DLRL objects contained within the cache.</P>
 */
public interface CacheBase {

    /**
     * Returns the usage mode of the CacheBase, which can be READ_ONLY,
     * WRITE_ONLY or READ_WRITE.
     *
     * @return the usage mode of the CacheBase.
     * @throws DDS.AlreadyDeleted if the CacheBase is already deleted.
     */
	DDS.CacheUsage cache_usage () throws DDS.AlreadyDeleted;

	/**
	 * <P>If the kind() operation indicates this CacheBase
     * represents a CACHE_KIND:
     * Applies waiting modifications in the DCPS onto the DLRL objects, when
	 * updates_enabled is <code>false</code>. No Listeners are
	 * triggered in that case. When updates_enabled is <code>true</code>, this
	 * operation is considered a no-op. This operation may not be called during
     * any listener callback if the kind() operation indicates this CacheBase
     * represents a CACHE_KIND, as it would result in a deadlock.</P>
	 * <P>One precondition must be satisfied when invoking the refresh method:
	 * the Cache must be in a readable mode (READ_ONLY or READ_WRITE, but
	 * not WRITE_ONLY). If this precondition is not met, a PreconditionNotMet
	 * Exception is thrown.</P>
     *
	 * <P>This operation is not supported for if the kind() operation indicates
     * this CacheBase represents a CACHE_ACCESS_KIND.</P>
	 *
	 * @throws DDS.DCPSError if an unexpected error occured in the DCPS.
	 * @throws DDS.AlreadyDeleted if the CacheBase is already deleted.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 */
	void refresh () throws DDS.DCPSError, DDS.AlreadyDeleted, DDS.PreconditionNotMet;

	/**
	 * Returns the list of all objects that are available in this CacheBase.
	 *
	 * @return the list of available objects.
	 * @throws DDS.AlreadyDeleted if the CacheBase is already deleted.
     * @throws DDS.DCPSError if an unexpected error occured in the DCPS.
	 */
    DDS.ObjectRoot[] objects() throws DDS.AlreadyDeleted, DDS.DCPSError;

	/**
	 * Returns the kind which indicates the eventual sub
     * class of this CacheBase.
	 *
	 * @return the kind of this CacheBase
	 * @throws DDS.AlreadyDeleted if the CacheBase is already deleted.
	 */
    DDS.CacheKind kind() throws DDS.AlreadyDeleted;
} // interface CacheBase
