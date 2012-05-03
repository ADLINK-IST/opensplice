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
#ifndef NW_CONFIGURATION_H
#define NW_CONFIGURATION_H

#include "c_typebase.h"
#include "nw_commonTypes.h" /* for NW_CLASS */
#include "nw_configurationDefs.h"
#include "u_service.h"
#include "u_cfElement.h"

#ifndef OSPL_ENV_RELEASE
#define NW_DEBUGGING
#else
/* Profiling tests, do not forget to switch them off... */
#define NW_PROFILING
/* Currently, the BIT functionalities are also compiled in the release version */
#define NW_TRACING
#define NW_LOOPBACK
#endif

#ifdef NW_DEBUGGING
#define NW_TRACING
#define NW_LOOPBACK
#define NW_PROFILING
#endif

#define NW_ATTACH_TIMEOUT (30)

void nw_configurationInitialize(
    u_service service,
    const char *serviceName,
    const char *URI);

void nw_configurationFinalize(void);


/* Convenience function to determine if report is needed */
c_bool nw_configurationLevelIsInteresting(c_ulong level);

#ifdef NW_LOOPBACK
/* Loopback can be used for testing purposes only */
c_bool nw_configurationUseLoopback();
c_bool nw_configurationUseComplementPartitions();
#endif

c_bool nw_configurationLoseSentMessage();
c_bool nw_configurationLoseReceivedMessage();

#ifdef NW_DEBUGGING
c_bool nw_configurationNoPacking();
#endif

c_bool nw_configurationGetIsIPv6(void);

void nw_configurationSetIsIPv6(c_bool isIPv6);

v_qos nw_configurationGetQos(void);
c_bool nw_configurationIsDiscoveryChannel(u_cfElement channel);

/* Getting parameter values by name */
c_bool nw_configurationGetBoolParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_bool defaultValue);

c_long nw_configurationGetLongParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_long defaultValue);

c_ulong nw_configurationGetULongParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_ulong defaultValue);

c_size nw_configurationGetSizeParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_size defaultValue);

c_float nw_configurationGetFloatParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           c_float defaultValue);

/* Do not forget to os_free the result after use */
c_string nw_configurationGetStringParameter(
           const c_char *parameterPath,
           const c_char *parameterName,
           const c_char *defaultValue);


/* Getting attribute values by name */
c_bool nw_configurationGetBoolAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_bool defaultValueNoElmt,
           c_bool defaultValueNoAttr);

c_ulong nw_configurationGetULongAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_ulong defaultValueNoElmt,
           c_ulong defaultValueNoAttrib);

c_size nw_configurationGetSizeAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_size defaultValueNoElmt,
           c_size defaultValueNoAttrib);

c_float nw_configurationGetFloatAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           c_float defaultValueNoElmt,
           c_float defaultValueNoAttrib);

c_string nw_configurationGetStringAttribute(
           const c_char *parameterPath,
           const c_char *attribyteName,
           const c_char *defaultValueNoElmt,
           const c_char *defaultValueNoAttrib);

/* return true if the element specified by 'path' has at least 1 (direct) child
 * element with the tag name specified in 'tagName'
 */
c_bool
nw_configurationElementHasChildElementWithName(
    const c_char* path,
    const c_char* tagName);

c_iter
nw_configurationGetElements(
    const c_char* path);

/* Convenience class */
NW_CLASS(nw_nameList);

nw_nameList nw_configurationGetChildElementPaths(
           const c_char *parameterPath,
           const c_char *attribName);

int      nw_nameListGetCount(
           nw_nameList nameList);

const c_char *nw_nameListGetName(
           nw_nameList nameList,
           c_ulong index);

void nw_nameListFree(
           nw_nameList nameList);

#endif

/* Domain parameters */
c_float nw_configurationGetDomainLeaseExpiryTime();
c_float nw_configurationGetDomainLeaseUpdateTime();
c_string nw_configurationGetDomainRole();


