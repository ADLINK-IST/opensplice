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
//NOT IN DESIGN
package org.opensplice.dds.dlrl;

public final class Initializer{

	static{
	    System.loadLibrary("dlrlsaj");
        jniInitializeAll();
        DDS.CacheFactory.get_instance();
	}

    private native static void jniInitializeAll();
}