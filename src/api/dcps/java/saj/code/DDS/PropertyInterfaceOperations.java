/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

package DDS;

public interface PropertyInterfaceOperations  {
  int set_property (Property a_property);
  int get_property (PropertyHolder a_property);
} // interface PropertyInterfaceOperations
