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


public final class DataReaderViewQos 
{
  
  public DDS.ViewKeyQosPolicy view_keys = null;

  /**
   * Returns the default DataReaderViewQos.
   */
  public DataReaderViewQos ()
  {
  } // ctor

  public DataReaderViewQos (DDS.ViewKeyQosPolicy _view_keys)
  {
      view_keys = _view_keys;
  } // ctor

} // class DataReaderViewQos
