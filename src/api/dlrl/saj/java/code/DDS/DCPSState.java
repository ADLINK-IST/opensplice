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
 * The DCPSState class represents an enumeration of the possible states in which
 * the publication and subscribtion infrastructure used by the DLRL can be.
 */
public final class DCPSState {
	private        int __value;
	private static int __size = 3;
	private static DDS.DCPSState[] __array = new DDS.DCPSState [__size];


	/**
     * This option indicates that the publication and subscribtion infrastructure
     * is in initial mode. I.E. the application using DLRL can still insert
     * information (in the form of registering ObjectHomes) to be used by the DLRL
     * to derive the publication and subscribtion infrastructure from.
     */
	public static final int _INITIAL = 0;

    /**
     * A convenience wrapper for the _INITIAL option of the DCPSState enumeration.
     */
	public static final DDS.DCPSState INITIAL = new DDS.DCPSState(_INITIAL);

	/**
     * This option indicates that the publication and subscribtion infrastructure
     * is in registered mode. I.E. the DLRL has used the information provided by the
     * application to derive the neccesary DCPS publication and subscribtion infrastructure.
     * The derived entities are not yet enabled though.
     */
	public static final int _REGISTERED = 1;

    /**
     * A convenience wrapper for the _REGISTERED option of the DCPSState enumeration.
     */
	public static final DDS.DCPSState REGISTERED = new DDS.DCPSState(_REGISTERED);

	/**
     * This option indicates that the publication and subscribtion infrastructure
     * is in enabled mode. I.E. the publication and subscribtion infrastructure is fully
     * up and running and the DLRL is ready to send/receive data.
     */
	public static final int _ENABLED = 2;

    /**
     * A convenience wrapper for the _ENABLED option of the DCPSState enumeration.
     */
	public static final DDS.DCPSState ENABLED = new DDS.DCPSState(_ENABLED);

    /**
     * This operation returns the value of this DCPSState, which is either initial,
     * registered or enabled.
     *
     * @return the value of this DCPSState.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct DCPSState object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of DCPSState
     * @return The correct DCPSState matching the specified value
     */
	public static DDS.DCPSState from_int (int value){
		if (value >= 0 && value < __size){
			return __array[value];
		} else {
			throw new java.lang.RuntimeException();
		}
	}

	protected DCPSState (int value){
		__value = value;
		__array[__value] = this;
	}

} // class DCPSState
