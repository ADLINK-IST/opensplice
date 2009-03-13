
package DDS;


public final class WriterDataLifecycleQosPolicy 
{
  public boolean autodispose_unregistered_instances = false;

  public WriterDataLifecycleQosPolicy ()
  {
  } // ctor

  public WriterDataLifecycleQosPolicy (boolean _autodispose_unregistered_instances)
  {
    autodispose_unregistered_instances = _autodispose_unregistered_instances;
  } // ctor

} // class WriterDataLifecycleQosPolicy
