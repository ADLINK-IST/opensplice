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
/** \file services/serialization/include/sd__serializerXML.h
 *  \brief Protected method of the XML serializer class, to be
 *         used only by the class and its descendants.
 */

#ifndef SD__SERIALIZERXML_H
#define SD__SERIALIZERXML_H

#include "sd_serializerXML.h"
#include "sd__deepwalkMeta.h" /* for sd_errorInfo */

c_ulong           sd_XMLSerType(
                      c_type type,
                      c_object object,
                      c_char *dataPtr);

void              sd_XMLDeserType(
                      c_type type,
                      c_object *objectPtr,
                      c_char **dataPtrPtr,
                      sd_errorInfo *errorInfo);                      
                      
c_ulong           sd_printCharData(
                      c_string dst,
                      c_string src);
                      
void              sd_scanCharData(
                      c_char **dst,
                      c_char **src,
                      sd_errorInfo *errorInfo);
                      
c_char *          sd_peekTaggedCharData(
                      c_char *src,
                      c_char *tagName);                      
                      
c_char *          sd_serializerXMLToString(
                      sd_serializer serializer,
                      sd_serializedData serData);

sd_serializedData sd_serializerXMLFromString(
                      sd_serializer serializer,
                      const c_char *str);
                      
                      
void              sd_XMLDeserCallbackPre(
                      const c_char *name,
                      c_type type,
                      c_object *objectPtr,
                      void *actionArg,
                      sd_errorInfo *errorInfo,
                      void *userData);
                      
void              sd_XMLDeserCallbackPost(
                      const c_char *name,
                      c_type type,
                      c_object *objectPtr,
                      void *actionArg,
                      sd_errorInfo *errorInfo,
                      void *userData);
                      
                      

#endif /* SD__SERIALIZERXML_H */
