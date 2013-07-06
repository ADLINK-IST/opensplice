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
 * The MembershipState class represents an enumeration of the possible states
 * of a member of a Selection.
 */
public final class MembershipState {
	private        int __value;
	private static int __size = 3;
	private static DDS.MembershipState[] __array = new DDS.MembershipState [__size];

	/**
     * This option indicates that the concerned object has an undefined membership state
     * in relation to the selection. Meaning it could already be a member or be a newly
     * inserted member.
     */
	public static final int _UNDEFINED_MEMBERSHIP = 0;

    /**
     * A convenience wrapper for the _UNDEFINED_MEMBERSHIP option of the 
     * MembershipState enumeration.
     */
	public static final DDS.MembershipState UNDEFINED_MEMBERSHIP = new DDS.MembershipState(_UNDEFINED_MEMBERSHIP);

	/**
     * This option indicates that the concerned object is already a member of the selection
     */
	public static final int _ALREADY_MEMBER = 1;

    /**
     * A convenience wrapper for the _ALREADY_MEMBER option of the 
     * MembershipState enumeration.
     */
	public static final DDS.MembershipState ALREADY_MEMBER = new DDS.MembershipState(_ALREADY_MEMBER);

	/**
     * This option indicates that the concerned object is not yet a member of the selection
     */
	public static final int _NOT_MEMBER = 2;

    /**
     * A convenience wrapper for the _NOT_MEMBER option of the 
     * MembershipState enumeration.
     */
	public static final DDS.MembershipState NOT_MEMBER = new DDS.MembershipState(_NOT_MEMBER);

    /**
     * This operation returns the value of this MembershipState, which is either undefined,
     * already a member or not yet a member.
     *
     * @return the value of this MembershipState.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct MembershipState object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of MembershipState
     * @return The correct MembershipState matching the specified value
     */
	public static DDS.MembershipState from_int (int value){
		if (value >= 0 && value < __size){
			 return __array[value];
		} else {
			throw new java.lang.RuntimeException();
		}
	}

	protected MembershipState (int value){
		__value = value;
		__array[__value] = this;
	}
} // class MembershipState
