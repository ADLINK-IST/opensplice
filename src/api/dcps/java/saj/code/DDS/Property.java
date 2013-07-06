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


public final class Property
{
  public String name = null;
  public String value = null;

  public Property () {
  } // ctor

  public Property (String _name, String _value)   {
      name = _name;
      value = _value;
  } // ctor

} // class Property
