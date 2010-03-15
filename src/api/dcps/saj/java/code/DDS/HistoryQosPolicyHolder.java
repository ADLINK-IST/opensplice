/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

package DDS;

public final class HistoryQosPolicyHolder
{

    public DDS.HistoryQosPolicy value = null;

    public HistoryQosPolicyHolder () { }

    public HistoryQosPolicyHolder (DDS.HistoryQosPolicy initialValue)
    {
        value = initialValue;
    }

}
