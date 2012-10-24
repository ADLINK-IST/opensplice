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

public interface DURATION_INFINITE
{
  public static final DDS.Duration_t value = new DDS.Duration_t(
      DURATION_INFINITE_SEC.value, DURATION_INFINITE_NSEC.value );
}
