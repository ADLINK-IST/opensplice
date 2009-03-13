
package DDS;


public interface ReadConditionOperations  extends DDS.ConditionOperations
{
  int get_sample_state_mask ();
  int get_view_state_mask ();
  int get_instance_state_mask ();
  DDS.DataReader get_datareader ();
} // interface ReadConditionOperations
