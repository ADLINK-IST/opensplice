
package DDS;




public interface EntityOperations 
{

  //  get_listener();
  int enable ();
  DDS.StatusCondition get_statuscondition ();
  int get_status_changes ();
  long get_instance_handle ();
  
} // interface EntityOperations
