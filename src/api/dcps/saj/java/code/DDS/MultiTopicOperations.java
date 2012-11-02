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


public interface MultiTopicOperations  extends DDS.TopicDescriptionOperations
{
  String get_subscription_expression ();
  int get_expression_parameters (StringSeqHolder expression_parameters);
  int set_expression_parameters (String[] expression_parameters);
} // interface MultiTopicOperations
