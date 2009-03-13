
package DDS;


public interface ContentFilteredTopicOperations  extends DDS.TopicDescriptionOperations
{
  String get_filter_expression ();
  int get_expression_parameters (StringSeqHolder expression_parameters);
  int set_expression_parameters (String[] expression_parameters);
  DDS.Topic get_related_topic ();
} // interface ContentFilteredTopicOperations
