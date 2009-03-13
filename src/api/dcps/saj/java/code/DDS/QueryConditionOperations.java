
package DDS;


public interface QueryConditionOperations  extends DDS.ReadConditionOperations
{
  String get_query_expression ();
  int get_query_parameters (DDS.StringSeqHolder query_parameters);
  int set_query_parameters (String[] query_parameters);
} // interface QueryConditionOperations
