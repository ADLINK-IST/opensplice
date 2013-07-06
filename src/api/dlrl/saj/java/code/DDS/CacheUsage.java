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
 * The CacheUsage class represents an enumeration of the possible usages of a CacheBase.
 */
public final class CacheUsage {
	private        int __value;
	private static int __size = 3;
	private static DDS.CacheUsage[] __array = new DDS.CacheUsage [__size];

	/**
     * This option indicates that the specific CacheBase is operating as a read
     * only cache. This means that the specific CacheBase can be used to 
     * read objects from either DCPS (in case of a Cache) or from the owning Cache
     * (in case of a CacheAccess). The user of the CacheBase may not however 
     * create/modify/delete Objects within that specific CacheBase.
     */
    public static final int _READ_ONLY = 0;

    /**
     * A convenience wrapper for the _READ_ONLY option of the CacheUsage enumeration.
     */
	public static final DDS.CacheUsage READ_ONLY = new DDS.CacheUsage(_READ_ONLY);

	/**
     * This option indicates that the specific CacheBase is operating as a write
     * only cache. This means that the specific CacheBase can not be used to 
     * read objects from either DCPS (in case of a Cache) or from the owning Cache
     * (in case of a CacheAccess). The user of the cache may create/modify/delete
     * Objects however.
     */
	public static final int _WRITE_ONLY = 1;

    /**
     * A convenience wrapper for the _WRITE_ONLY option of the CacheUsage enumeration.
     */
    public static final DDS.CacheUsage WRITE_ONLY = new DDS.CacheUsage(_WRITE_ONLY);

	/**
     * This option indicates that the specific CacheBase is operating as both a read 
     * and a write cache. This means that the specific CacheBase can be used to 
     * read objects from either DCPS (in case of a Cache) or from the owning Cache
     * (in case of a CacheAccess). The user of the CacheBase may also create/modify/delete
     * Objects within that specific CacheBase.
     */
	public static final int _READ_WRITE = 2;

    /**
     * A convenience wrapper for the _READ_WRITE option of the CacheUsage enumeration.
     */
	public static final DDS.CacheUsage READ_WRITE = new DDS.CacheUsage(_READ_WRITE);

    /**
     * This operation returns the value of this CacheUsage, which is either read only mode, 
     * write only mode or read and write mode.
     *
     * @return the value of this CacheUsage.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct CacheUsage object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of CacheUsage
     * @return The correct CacheUsage matching the specified value
     */
	public static DDS.CacheUsage from_int (int value){
		if (value >= 0 && value < __size){
		  return __array[value];
		} else {
		  throw new java.lang.RuntimeException();
		}
	}

	protected CacheUsage (int value){
		__value = value;
		__array[__value] = this;
	}
} // class CacheUsage
