
package DDS;


public interface StatusConditionOperations  extends DDS.ConditionOperations
{
  int get_enabled_statuses ();
  int set_enabled_statuses (int mask);
  DDS.Entity get_entity ();
} // interface StatusConditionOperations
