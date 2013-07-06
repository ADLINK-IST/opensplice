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
package org.opensplice.dds.dlrl;

/** 
 * Prismtech implementation of the {@link DDS.FilterCriterion} interface. 
 */ 
public abstract class FilterCriterionImpl implements DDS.FilterCriterion { 

    /* see DDS.FilterCriterion for javadoc */ 
    public final DDS.CriterionKind kind (){
        return DDS.CriterionKind.FILTER;
    }
}