/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/************************************************************************
 * LOGICAL_NAME:    ExtDomainParticipantHelper.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for a Helper class of the extended 
 * DomainParticipant, that simulates the behavior of a Helper class with respect 
 * to narrowing an existing DomainParticipant into its extended representation.
 * 
 ***/

package chatroom;

import DDS.DomainParticipant;

public class ExtDomainParticipantHelper {
    public static ExtDomainParticipant narrow(
            DomainParticipant participant) {
        return new ExtDomainParticipant(participant);
    }
}
