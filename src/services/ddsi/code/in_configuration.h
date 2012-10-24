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
#ifndef IN_CONFIGURATION_H
#define IN_CONFIGURATION_H

#include "c_typebase.h"
#include "in_commonTypes.h" /* for OS_CLASS */
#include "in_configurationDefs.h"
#include "u_service.h"
#include "u_cfElement.h"

#ifndef OSPL_ENV_RELEASE
#define IN_DEBUGGING
#else
/* Profiling tests, do not forget to switch them off... */
#define IN_PROFILING
/* Currently, the BIT functionalities are also compiled in the release version */
#define IN_TRACING
#define IN_LOOPBACK
#endif

#ifdef IN_DEBUGGING
#define IN_TRACING
#define IN_LOOPBACK
#define IN_PROFILING
#endif

#define IN_ATTACH_TIMEOUT (30)
void in_configurationInitialize(
    u_service service,
    const char *serviceName,
    const char *URI);

void in_configurationFinalize(void);


/* Convenience function to determine if report is needed */
c_bool in_configurationLevelIsInteresting(c_ulong level);

#ifdef IN_LOOPBACK
/* Loopback can be used for testing purposes only */
c_bool in_configurationUseLoopback();
c_bool in_configurationUseComplementPartitions();
#endif

#ifdef IN_DEBUGGING
c_bool in_configurationLoseSentMessage();
c_bool in_configurationLoseReceivedMessage();
c_bool in_configurationNoPacking();
#endif

v_qos in_configurationGetQos(void);
c_bool in_configurationIsDiscoveryChannel(u_cfElement channel);

/* Getting parameter values by name */
c_bool in_configurationGetBoolParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_bool defaultValue);

c_long in_configurationGetLongParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_long defaultValue);

c_ulong in_configurationGetULongParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_ulong defaultValue);

c_ulong in_configurationGetSizeParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_ulong defaultValue);

c_float in_configurationGetFloatParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_float defaultValue);

/* Do not forget to os_free the result after use */
c_string in_configurationGetStringParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           const c_char *defaultValue);


/* Getting attribute values by name */
c_bool in_configurationGetBoolAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_bool defaultValueNoElmt,
           c_bool defaultValueNoAttr);

c_ulong in_configurationGetULongAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_ulong defaultValueNoElmt,
           c_ulong defaultValueNoAttrib);

c_ulong in_configurationGetSizeAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_ulong defaultValueNoElmt,
           c_ulong defaultValueNoAttrib);

c_float in_configurationGetFloatAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_float defaultValueNoElmt,
           c_float defaultValueNoAttrib);

c_string in_configurationGetStringAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           const c_char *defaultValueNoElmt,
           const c_char *defaultValueNoAttrib);

/* return true if the element specified by 'path' has at least 1 (direct) child
 * element with the tag name specified in 'tagName'
 */
c_bool
in_configurationElementHasChildElementWithName(
    const c_char* path,
    const c_char* tagName);

c_iter
in_configurationGetElements(
    const c_char* path);

/* Convenience class */
OS_CLASS(in_nameList);

in_nameList in_configurationGetChildElementPaths(
           const c_char *parameterPath,
           const c_char *attribName);

int      in_nameListGetCount(
           in_nameList nameList);

const c_char *in_nameListGetName(
           in_nameList nameList,
           c_ulong index);

void in_nameListFree(
           in_nameList nameList);
#endif


/* Domain parameters */
c_float in_configurationGetDomainLeaseExpiryTime();
c_float in_configurationGetDomainLeaseUpdateTime();


