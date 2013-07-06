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
 * The CriterionKind class represents an enumeration of the possible types 
 * (sub classes) of a Criterion.
 */
public final class CriterionKind {
	private        int __value;
	private static int __size = 2;
	private static DDS.CriterionKind[] __array = new DDS.CriterionKind [__size];


	/**
     * This option indicates that the specific Criterion object is instantiated as a 
     * QueryCriterion class.
     */
	public static final int _QUERY = 0;

    /**
     * A convenience wrapper for the _QUERY option of the CriterionKind enumeration.
     */
	public static final DDS.CriterionKind QUERY = new DDS.CriterionKind(_QUERY);

	/**
     * This option indicates that the specific Criterion object is instantiated as a 
     * FilterCriterion class.
     */
	public static final int _FILTER = 1;

    /**
     * A convenience wrapper for the _FILTER option of the CriterionKind enumeration.
     */
	public static final DDS.CriterionKind FILTER = new DDS.CriterionKind(_FILTER);

    /**
     * This operation returns the value of this CriterionKind, which is either of type  filter 
     * or of type query.
     *
     * @return the value of this CriterionKind.
     */
	public int value (){
		return __value;
	}

    /**
     * Returns the correct CriterionKind object from the specified value or throws a
     * runtime exception is the value is out of range.
     *
     * @param value The value indicating the type of CriterionKind
     * @return The correct CriterionKind matching the specified value
     */
	public static DDS.CriterionKind from_int (int value){
		if (value >= 0 && value < __size){
			return __array[value];
		} else {
			throw new java.lang.RuntimeException();
		}
	}

	protected CriterionKind (int value){
		__value = value;
		__array[__value] = this;
	}

} // class DDS.CriterionKind
