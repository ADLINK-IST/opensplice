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

#ifndef D__PUBLISHER_H
#define D__PUBLISHER_H

#include "d__types.h"
#include "u_user.h"
#include "os_mutex.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_publisher){
    C_EXTENDS(d_object);
    d_admin         admin;
    u_publisher     publisher;
    c_bool          enabled;
    
    u_writer        statusRequestWriter;
    c_ulong         statusRequestNumber;
    u_writer        groupsRequestWriter;
    c_ulong         groupsRequestNumber;
    u_writer        sampleRequestWriter;
    c_ulong         sampleRequestNumber;
    u_writer        statusWriter;
    c_ulong         statusNumber;
    u_writer        newGroupWriter;
    c_ulong         newGroupNumber;
    u_writer        sampleChainWriter;
    c_ulong         sampleChainNumber;
    u_writer        nameSpacesRequestWriter;
    c_ulong         nameSpacesRequestNumber;
    u_writer        nameSpacesWriter;
    c_ulong         nameSpacesNumber;
    u_writer        deleteDataWriter;
    c_ulong         deleteDataNumber;
};

c_bool          d_publisherMessageWriterCopy            (d_message msgFrom,
                                                         d_message msgTo);

c_bool          d_publisherStatusWriterCopy             (c_type type, 
                                                         void *data, 
                                                         void *to);

c_bool          d_publisherNewGroupWriterCopy           (c_type type, 
                                                         void *data, 
                                                         void *to);
                                                 
c_bool          d_publisherGroupsRequestWriterCopy      (c_type type, 
                                                         void *data, 
                                                         void *to);

c_bool          d_publisherStatusRequestWriterCopy      (c_type type, 
                                                         void *data, 
                                                         void *to);
                                                     
c_bool          d_publisherSampleRequestWriterCopy      (c_type type, 
                                                         void *data, 
                                                         void *to);

c_bool          d_publisherSampleChainWriterCopy        (c_type type, 
                                                         void *data, 
                                                         void *to);

c_bool          d_publisherNameSpacesWriterCopy         (c_type type, 
                                                         void *data, 
                                                         void *to);
                                                     
c_bool          d_publisherNameSpacesRequestWriterCopy  (c_type type, 
                                                         void *data, 
                                                         void *to);

c_bool          d_publisherDeleteDataWriterCopy         (c_type type, 
                                                         void *data, 
                                                         void *to);

void            d_publisherInitMessage                  (d_publisher publisher,
                                                         d_message message);

void            d_publisherEnsureServicesAttached       (v_entity entity,
                                                         c_voidp args);

void            d_publisherDeinit                       (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /* D__PUBLISHER_H */
