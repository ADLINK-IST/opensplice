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
 * The ReferenceScope class represents an enumeration of the possible scopes in
 * which to view a relation. I.E. just the relation itself (SIMPLE) or also the object 
 * at the end point of the relation (REFERENCED).
 */
public final class ReferenceScope {
  
	private        int __value;
	private static int __size = 2;
	private static DDS.ReferenceScope[] __array = new DDS.ReferenceScope [__size];

	/**
     * This option indicates that only the relation should be taken into account.
     * I.E. Has the relation or in case of a collection an element in the collection
     * been modified.
     */
	public static final int _SIMPLE_CONTENT_SCOPE = 0;

    /**
     * A convenience wrapper for the _SIMPLE_CONTENT_SCOPE option of the 
     * ReferenceScope enumeration.
     */
	public static final DDS.ReferenceScope SIMPLE_CONTENT_SCOPE = 
                                                new DDS.ReferenceScope(_SIMPLE_CONTENT_SCOPE);

	/**
     * This option indicates that not only the relation should be taken into account, but that
     * the state of the related objects should also be taken into acount. In case of a collection
     * this means the state of each element in the collection.
     */
	public static final int _REFERENCED_CONTENTS_SCOPE = 1;
    
    /**
     * A convenience wrapper for the _REFERENCED_CONTENTS_SCOPE option of the 
     * ReferenceScope enumeration.
     */
	public static final DDS.ReferenceScope REFERENCED_CONTENTS_SCOPE = 
                                                new DDS.ReferenceScope(_REFERENCED_CONTENTS_SCOPE);

    /**
     * This operation returns the value of this ReferenceScope, which is either simple 
     * content scope or referenced content scope.
     *
     * @return the value of this ReferenceScope.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct ReferenceScope object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of ReferenceScope
     * @return The correct ReferenceScope matching the specified value
     */
	public static DDS.ReferenceScope from_int (int value){
		if (value >= 0 && value < __size){
			return __array[value];
		} else {
			throw new java.lang.RuntimeException ();
		}
	}

	protected ReferenceScope (int value){
		__value = value;
		__array[__value] = this;
	}
} // class ReferenceScope
