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
package org.opensplice.cm;

/**
 * Represents a filter that can be used to filter the entities that are being
 * retrieved by the getOwnedEntities and getDependantEntities functions of
 * the Entity class.
 */
public class EntityFilter {
    private static final int _TOPIC         = 1;
    private static final int _QUERY         = 2;
    private static final int _PARTITION     = 4;
    private static final int _READER        = 5;
    private static final int _DATAREADER    = 6;
    private static final int _WRITER        = 8;
    private static final int _SUBSCRIBER    = 9;
    private static final int _PUBLISHER     = 10;
    private static final int _PARTICIPANT   = 11;
    private static final int _SERVICE       = 12;
    private static final int _SERVICESTATE  = 13;
    private static final int _ENTITY        = 14;
    private static final int _JOIN          = 15;
    private static final int _NETWORKREADER = 16;
    private static final int _GROUPQUEUE    = 17;
    private static final int _WAITSET       = 18;

    public static final EntityFilter TOPIC          = new EntityFilter(_TOPIC);
    public static final EntityFilter QUERY          = new EntityFilter(_QUERY);
    public static final EntityFilter PARTITION      = new EntityFilter(_PARTITION);
    public static final EntityFilter READER         = new EntityFilter(_READER);
    public static final EntityFilter DATAREADER     = new EntityFilter(_DATAREADER);
    public static final EntityFilter WRITER         = new EntityFilter(_WRITER);
    public static final EntityFilter SUBSCRIBER     = new EntityFilter(_SUBSCRIBER);
    public static final EntityFilter PUBLISHER      = new EntityFilter(_PUBLISHER);
    public static final EntityFilter PARTICIPANT    = new EntityFilter(_PARTICIPANT);
    public static final EntityFilter SERVICE        = new EntityFilter(_SERVICE);
    public static final EntityFilter SERVICESTATE   = new EntityFilter(_SERVICESTATE);
    public static final EntityFilter ENTITY         = new EntityFilter(_ENTITY);
    public static final EntityFilter JOIN           = new EntityFilter(_JOIN);
    public static final EntityFilter NETWORKREADER  = new EntityFilter(_NETWORKREADER);
    public static final EntityFilter GROUPQUEUE     = new EntityFilter(_GROUPQUEUE);
    public static final EntityFilter WAITSET        = new EntityFilter(_WAITSET);
    
    private EntityFilter(int rc){}
    
    /**
     * Converts the filter to its string representation.
     * 
     * @param filter The filter to convert.
     * @return The String representation og the supplied filter.
     */
    public static String getString(EntityFilter filter){
        String r = "ENTITY";
        
        if(filter.equals(EntityFilter.TOPIC)){
            r = "TOPIC";
        } else if(filter.equals(EntityFilter.QUERY)){
            r = "QUERY";
        } else if(filter.equals(EntityFilter.PARTITION)){
            r = "DOMAIN";
        } else if(filter.equals(EntityFilter.READER)){
            r = "READER";
        } else if(filter.equals(EntityFilter.DATAREADER)){
            r = "DATAREADER";
        } else if(filter.equals(EntityFilter.WRITER)){
            r = "WRITER";
        } else if(filter.equals(EntityFilter.SUBSCRIBER)){
            r = "SUBSCRIBER";
        } else if(filter.equals(EntityFilter.PUBLISHER)){
            r = "PUBLISHER";
        } else if(filter.equals(EntityFilter.PARTICIPANT)){
            r = "PARTICIPANT";
        } else if(filter.equals(EntityFilter.SERVICE)){
            r = "SERVICE";
        } else if(filter.equals(EntityFilter.SERVICESTATE)){
            r = "SERVICESTATE";
        } else if(filter.equals(EntityFilter.JOIN)){
            r = "JOIN";
        } else if(filter.equals(EntityFilter.NETWORKREADER)){
            r = "NETWORKREADER";
        } else if(filter.equals(EntityFilter.GROUPQUEUE)){
            r = "GROUPQUEUE";
        } else if(filter.equals(EntityFilter.WAITSET)){
            r = "WAITSET";
        }
        return r;
    }
}
