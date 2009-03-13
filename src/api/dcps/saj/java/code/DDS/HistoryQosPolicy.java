
package DDS;


public final class HistoryQosPolicy 
{
  public DDS.HistoryQosPolicyKind kind = null;
  public int depth = (int)0;

  public HistoryQosPolicy ()
  {
  } // ctor

  public HistoryQosPolicy (DDS.HistoryQosPolicyKind _kind, int _depth)
  {
    kind = _kind;
    depth = _depth;
  } // ctor

} // class HistoryQosPolicy
