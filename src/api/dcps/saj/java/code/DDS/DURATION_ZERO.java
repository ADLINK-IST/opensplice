/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

package DDS;

public interface DURATION_ZERO
{
  public static final DDS.Duration_t value = new DDS.Duration_t(
      DURATION_ZERO_SEC.value, DURATION_ZERO_NSEC.value);
}
