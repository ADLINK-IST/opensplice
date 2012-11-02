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


public interface QueryConditionOperations  extends DDS.ReadConditionOperations
{
  String get_query_expression ();
  int get_query_parameters (DDS.StringSeqHolder query_parameters);
  int set_query_parameters (String[] query_parameters);
} // interface QueryConditionOperations
