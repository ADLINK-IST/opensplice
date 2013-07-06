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

public final class InvalidSampleVisibilityQosPolicyKind
{
	private int value = -1;
	public static final int _NO_INVALID_SAMPLES = 0;
	public static final InvalidSampleVisibilityQosPolicyKind NO_INVALID_SAMPLES = new InvalidSampleVisibilityQosPolicyKind(_NO_INVALID_SAMPLES);
	public static final int _MINIMUM_INVALID_SAMPLES = 1;
	public static final InvalidSampleVisibilityQosPolicyKind MINIMUM_INVALID_SAMPLES = new InvalidSampleVisibilityQosPolicyKind(_MINIMUM_INVALID_SAMPLES);
	public static final int _ALL_INVALID_SAMPLES = 2;
	public static final InvalidSampleVisibilityQosPolicyKind ALL_INVALID_SAMPLES = new InvalidSampleVisibilityQosPolicyKind(_ALL_INVALID_SAMPLES);
	public int value()
	{
		return value;
	}
	public static InvalidSampleVisibilityQosPolicyKind from_int(int value)
	{
		switch (value) {
			case _NO_INVALID_SAMPLES: return NO_INVALID_SAMPLES;
			case _MINIMUM_INVALID_SAMPLES: return MINIMUM_INVALID_SAMPLES;
			case _ALL_INVALID_SAMPLES: return ALL_INVALID_SAMPLES;
			default: throw new org.omg.CORBA.BAD_PARAM();
		}
	}
	protected InvalidSampleVisibilityQosPolicyKind(int i)
	{
		value = i;
	}
}
