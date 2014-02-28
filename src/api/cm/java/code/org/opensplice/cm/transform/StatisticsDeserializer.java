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
package org.opensplice.cm.transform;

import org.opensplice.cm.Entity;
import org.opensplice.cm.statistics.Statistics;

/**
 * 
 * 
 * @date May 12, 2005 
 */
public interface StatisticsDeserializer {
    public Statistics deserializeStatistics(Object serialized, Entity entity) throws TransformationException;
    public Statistics[] deserializeStatistics(Object serialized, Entity[] entities) throws TransformationException;
}
