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
 * The CacheKind class represents an enumeration of the possible types 
 * (sub classes) of a CacheBase.
 */
public final class CacheKind {
	private        int __value;
	private static int __size = 2;
	private static DDS.CacheKind[] __array = new DDS.CacheKind [__size];


	/**
     * This option indicates that the specific CacheBase object is instantiated as a 
     * Cache class.
     */
	public static final int _CACHE_KIND = 0;

    /**
     * A convenience wrapper for the _CACHE_KIND option of the CacheKind enumeration.
     */
	public static final DDS.CacheKind CACHE_KIND = new DDS.CacheKind(_CACHE_KIND);

	/**
     * This option indicates that the specific CacheBase object is instantiated as a 
     * CacheAccess class.
     */
	public static final int _CACHEACCESS_KIND = 1;

    /**
     * A convenience wrapper for the _CACHEACCESS_KIND option of the CacheKind enumeration.
     */
	public static final DDS.CacheKind CACHEACCESS_KIND = new DDS.CacheKind(_CACHEACCESS_KIND);

    /**
     * This operation returns the value of this CacheKind, which is either of kind cache 
     * or of kind cache access.
     *
     * @return the value of this CacheKind.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct CacheKind object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of CacheKind
     * @return The correct CacheKind matching the specified value
     */
	public static DDS.CacheKind from_int (int value){
		if (value >= 0 && value < __size){
			return __array[value];
		} else {
			throw new java.lang.RuntimeException();
		}
	}

	protected CacheKind (int value){
		__value = value;
		__array[__value] = this;
	}

} // class DDS.CacheKind
