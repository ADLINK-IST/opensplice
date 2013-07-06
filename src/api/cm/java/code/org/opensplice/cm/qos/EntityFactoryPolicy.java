/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.cm.qos;

/**
 * Controls the behavior of the entity when acting as a factory for other 
 * entities. In other words, configures the side-effects of the 
 * create_xx and delete_xx operations.
 * 
 * @date Jan 10, 2005 
 */
public class EntityFactoryPolicy {
    /**
     * Specifies whether the entity acting as a factory automatically enables 
     * the instances it creates. If autoenable_created_entities==true
     * the factory will automatically enable each created Entity otherwise it 
     * will not. By default, true.
     */
    public boolean autoenable_created_entities;
    
    public static final EntityFactoryPolicy DEFAULT = new EntityFactoryPolicy(true);
    
    /**
     * Constructs a new entity factory policy.
     *  
     *
     * @param _autoenable_created_entities Whether or not to auto enable 
     *                                     entities.
     */
    public EntityFactoryPolicy(boolean _autoenable_created_entities){
        autoenable_created_entities = _autoenable_created_entities;
    }
    
    public EntityFactoryPolicy copy(){
        return new EntityFactoryPolicy(this.autoenable_created_entities);
    }
}
