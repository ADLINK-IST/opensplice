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
 * The ObjectScope class represents an enumeration of the possible scopes
 * in which to view a DLRL object. I.E. Just the object itself (SIMPLE),
 * or the object itself and it's composite relations or the object itself
 * it's composite relations and it's non composite relations.
 */
public final class ObjectScope {

	private        int __value;
	private static int __size = 3;
	private static DDS.ObjectScope[] __array = new DDS.ObjectScope [__size];

	/**
     * This option indicates that the concerned object should be
     * viewed ignoring any (composite) relations. I.E. Only looking at the
     * state of the object itself.
     */
	public static final int _SIMPLE_OBJECT_SCOPE = 0;

    /**
     * A convenience wrapper for the _SIMPLE_OBJECT_SCOPE option of the 
     * ObjectScope enumeration.
     */
	public static final DDS.ObjectScope SIMPLE_OBJECT_SCOPE = new DDS.ObjectScope(_SIMPLE_OBJECT_SCOPE);

	/**
     * This option indicates that the concerned object should be
     * viewed ignoring only non-composite relations. I.E. The state
     * of the object itself and the state of it's composite related
     * objects is relevant.
     */
	public static final int _CONTAINED_OBJECTS_SCOPE = 1;
	
    /**
     * A convenience wrapper for the _CONTAINED_OBJECTS_SCOPE option of the 
     * ObjectScope enumeration.
     */
    public static final DDS.ObjectScope CONTAINED_OBJECTS_SCOPE = new DDS.ObjectScope(_CONTAINED_OBJECTS_SCOPE);

	/**
     * This option indicates that the concerned object should be
     * viewed taking all (composite) relations into account. I.E. The state
     * of the object itself, the state of it's composite related objects
     * and the state of it's non-composite related objects is relevant.
     */
	public static final int _RELATED_OBJECTS_SCOPE = 2;

    /**
     * A convenience wrapper for the _RELATED_OBJECTS_SCOPE option of the 
     * ObjectScope enumeration.
     */
	public static final DDS.ObjectScope RELATED_OBJECTS_SCOPE = new DDS.ObjectScope(_RELATED_OBJECTS_SCOPE);

    /**
     * This operation returns the value of this ObjectScope, which is either simple object scope,
     * contained objects scope or related objects scope.
     *
     * @return the value of this ObjectScope.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct ObjectScope object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of ObjectScope
     * @return The correct ObjectScope matching the specified value
     */
	public static DDS.ObjectScope from_int (int value){
		if (value >= 0 && value < __size){
			return __array[value];
		} else {
			throw new java.lang.RuntimeException ();
		}
	}

	protected ObjectScope (int value){
		__value = value;
		__array[__value] = this;
	}
} // class ObjectScope
