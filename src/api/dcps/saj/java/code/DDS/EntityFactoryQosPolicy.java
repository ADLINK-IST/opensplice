
package DDS;


public final class EntityFactoryQosPolicy 
{
  public boolean autoenable_created_entities = true;

  public EntityFactoryQosPolicy ()
  {
  } // ctor

  public EntityFactoryQosPolicy (boolean _autoenable_created_entities)
  {
    autoenable_created_entities = _autoenable_created_entities;
  } // ctor

} // class EntityFactoryQosPolicy
