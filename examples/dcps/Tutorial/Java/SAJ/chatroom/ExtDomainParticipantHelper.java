/************************************************************************
 *  
 * Copyright (c) 2007
 * PrismTech Ltd.
 * All rights Reserved.
 * 
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
