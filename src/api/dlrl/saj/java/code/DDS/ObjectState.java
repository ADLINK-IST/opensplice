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
 * The ObjectState class represents an enumeration of the possible life cycle states
 * of a DLRL object.
 */
public final class ObjectState {
	private        int __value;
	private static int __size = 5;
	private static DDS.ObjectState[] __array = new DDS.ObjectState [__size];

	/**
     * This option indicates that the concerned object's life cycle state
     * is not tracked.
     */
	public static final int _OBJECT_VOID = 0;

    /**
     * A convenience wrapper for the _OBJECT_VOID option of the 
     * ObjectState enumeration.
     */
	public static final DDS.ObjectState OBJECT_VOID = new DDS.ObjectState(_OBJECT_VOID);

	/**
     * This option indicates that the concerned object is newly created.
     */
	public static final int _OBJECT_NEW = 1;

    /**
     * A convenience wrapper for the _OBJECT_NEW option of the 
     * ObjectState enumeration.
     */
	public static final DDS.ObjectState OBJECT_NEW = new DDS.ObjectState(_OBJECT_NEW);

	/**
     * This option indicates that the concerned object has not been
     * modified.
     */
	public static final int _OBJECT_NOT_MODIFIED = 2;

    /**
     * A convenience wrapper for the _OBJECT_NOT_MODIFIED option of the 
     * ObjectState enumeration.
     */
	public static final DDS.ObjectState OBJECT_NOT_MODIFIED = new DDS.ObjectState(_OBJECT_NOT_MODIFIED);

	/**
     * This option indicates that the concerned object has been
     * modified.
     */
	public static final int _OBJECT_MODIFIED = 3;

    /**
     * A convenience wrapper for the _OBJECT_MODIFIED option of the 
     * ObjectState enumeration.
     */
	public static final DDS.ObjectState OBJECT_MODIFIED = new DDS.ObjectState(_OBJECT_MODIFIED);

	/**
     * This option indicates that the concerned object has been
     * deleted.
     */
	public static final int _OBJECT_DELETED = 4;

    /**
     * A convenience wrapper for the _OBJECT_DELETED option of the 
     * ObjectState enumeration.
     */
	public static final DDS.ObjectState OBJECT_DELETED = new DDS.ObjectState(_OBJECT_DELETED);

    /**
     * This operation returns the value of this ObjectState, which is either void, new, 
     * not modified, modified or deleted.
     *
     * @return the value of this ObjectState.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct ObjectState object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of ObjectState
     * @return The correct ObjectState matching the specified value
     */
	public static DDS.ObjectState from_int (int value){
		if (value >= 0 && value < __size){
		  return __array[value];
		} else {
		  throw new java.lang.RuntimeException();
		}
	}

	protected ObjectState (int value){
		__value = value;
		__array[__value] = this;
	}
} // class ObjectState
