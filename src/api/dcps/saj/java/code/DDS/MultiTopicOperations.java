
package DDS;


public interface MultiTopicOperations  extends DDS.TopicDescriptionOperations
{
  String get_subscription_expression ();
  int get_expression_parameters (StringSeqHolder expression_parameters);
  int set_expression_parameters (String[] expression_parameters);
} // interface MultiTopicOperations
