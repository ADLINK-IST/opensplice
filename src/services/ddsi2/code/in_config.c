/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "in_config.h"
#include "os_stdlib.h"
#include "u_participant.h"
#include "u_cfElement.h"
#include "u_cfData.h"

static in_config configInstance = NULL;

static void
in_configSetNetworkAddress(
    in_config config,
    const c_char* value);

static void
in_configSetTracingOutputFile(
    in_config config,
    const c_char* value);

static void
in_configSetTracingTimestamps(
    in_config  config,
    c_bool useTimestamp);

static void
in_configSetAllowMulticast(
    in_config  config,
    c_bool allowMulticast);

static void
in_configSetEnableMulticastLoopback (
	in_config config,
	c_bool enabledMulticastLoopback);

static void
in_configAddPeer(
    in_config config,
    const c_char* peer);

static void
in_configSetTracingRelativeTimestamps(
    in_config config,
    u_cfElement element,
    const c_char* timestampsPath,
    const c_char* absoluteName);

static void
in_configSetTracingVerbosity(
    in_config config,
    const c_char* value);

static void
in_configSetDomainId (
	in_config config,
	int domainId);

static void
in_configSetParticipantIndex (
	in_config config,
	const c_char *value);

static void
in_configSetCoexistWithNativeNetworking(
    in_config  config,
    c_bool coexistWithNativeNetworking);

static void
in_configEnableUndocFeatures(
    in_config config,
    const c_char* value);

static void
in_configValueString(
    in_config configuration,
    u_cfElement  element,
    const char * tag,
    void  (* const setAction)(in_config config, const c_char * str) );

static void
in_configValueInt(
    in_config configuration,
    u_cfElement  element,
    const char * tag,
    void  (* const setAction)(in_config config, const int str) );

static void
in_configValueBoolean(
    in_config configuration,
    u_cfElement  element,
    const char * tag,
    void         (* const setAction)(in_config config, c_bool str) );

static void
in_doPrint(
    const char* format,
    va_list args);

in_config
in_configNew(
    u_participant participant,
    const c_char* serviceName)
{
    in_config config;
    u_cfElement root, found, element;
    c_bool success;
    c_iter iter;
    c_char* attrValue;

    if(participant){
        config = in_config(os_malloc(OS_SIZEOF(in_config)));

        if(config){
            root = u_participantGetConfiguration(participant);

            if(root){
                iter = u_cfElementXPath(root, "DDSI2Service");

                element = u_cfElement(c_iterTakeFirst(iter));
                found = NULL;

                while(element){
                    success = u_cfElementAttributeStringValue(element, "name", &attrValue);

                    if(success == TRUE){
                        if(strcmp(serviceName, attrValue) == 0){
                            if(found){
                                u_cfElementFree(found);
                            }
                            found = element;
                            element = NULL;
                        }
                       os_free(attrValue);
                    }
                    if(element){
                        u_cfElementFree(element);
                    }
                    element = u_cfElement(c_iterTakeFirst(iter));
                }
                config->tracingOutputFile           = NULL;
                config->tracingOutputFileName       = NULL;
                config->tracingVerbosityLevel       = IN_LEVEL_WARNING;
                config->tracingTimestamps           = TRUE;
                config->tracingRelativeTimestamps   = FALSE;
                config->networkAddressString        = NULL;
                config->peers                       = NULL;
                config->allowMulticast              = TRUE;
		config->enableMulticastLoopback     = TRUE;
		config->domainId                    = 0;
		config->participantIndex            = -1;
		config->coexistWithNativeNetworking = FALSE;
		config->enableUndocFeatures         = NULL;

                if(found){
                    element = found;
                    in_configValueString (config, element, "General/NetworkInterfaceAddress/#text", in_configSetNetworkAddress);
                    in_configValueBoolean(config, element, "General/AllowMulticast/#text", in_configSetAllowMulticast);
                    in_configValueBoolean(config, element, "General/EnableMulticastLoopback/#text", in_configSetEnableMulticastLoopback);
                    in_configValueBoolean(config, element, "General/CoexistWithNativeNetworking/#text", in_configSetCoexistWithNativeNetworking);
                    in_configValueString (config, element, "General/EnableUndocFeatures/#text", in_configEnableUndocFeatures);
                    in_configValueString (config, element, "Discovery/Peer/#text", in_configAddPeer);
		    in_configValueInt (config, element, "Discovery/DomainId/#text", in_configSetDomainId);
		    in_configValueString (config, element, "Discovery/ParticipantIndex/#text", in_configSetParticipantIndex);
                    in_configValueString (config, element, "Tracing/Verbosity/#text", in_configSetTracingVerbosity);
                    in_configValueString (config, element, "Tracing/Verbosity/#text", in_configSetTracingVerbosity);
                    in_configValueString (config, element, "Tracing/OutputFile/#text", in_configSetTracingOutputFile);
                    in_configValueBoolean(config, element, "Tracing/Timestamps/#text", in_configSetTracingTimestamps);
                    in_configSetTracingRelativeTimestamps(config, element, "Tracing/Timestamps", "absolute");
                }
                u_cfElementFree(root);
            } else {
                in_printf(IN_LEVEL_CONFIG, "No configuration found.\n");
            }
        }
        configInstance = config;
    } else {
        config = NULL;
    }
    return config;
}

void
in_configFree(
    in_config config)
{
    assert(config);

    if(config){
        os_free(config);
    }
    if (config == configInstance)
      configInstance = NULL;
}

static void
in_doPrint(
    const char* format,
    va_list args)
{
    char description[16384];

    if(configInstance->tracingOutputFile){
        os_vsnprintf(description, sizeof(description)-1, format, args);
        description [sizeof(description)-1] = '\0';
        fprintf(configInstance->tracingOutputFile, "%s", description);
        fflush(configInstance->tracingOutputFile);
    }
}


int
in_printfRtps(
    const char* eventText,
    va_list args)
{
    if(configInstance){
      in_doPrint(eventText, args);
    }
    return 0;
}

void
in_printf(
    in_level level,
    const char * eventText,
    ...)
{
    va_list args;

    if(configInstance){
        if(level >= configInstance->tracingVerbosityLevel){
            va_start (args, eventText);
            in_doPrint(eventText, args);
            va_end (args);
        }
    }
}

static void
in_configSetNetworkAddress(
    in_config config,
    const c_char* value)
{
    if(config->networkAddressString){
        os_free(config->networkAddressString);
    }
    if (os_strcasecmp (value, "AUTO") != 0)
    {
      config->networkAddressString = os_strdup(value);
    }
}

static void
in_configSetTracingOutputFile(
    in_config config,
    const c_char* value)
{
    if(value){
        if(config->tracingOutputFileName){
            if( (os_strcasecmp(config->tracingOutputFileName, "stdout") != 0) &&
                (os_strcasecmp(config->tracingOutputFileName, "stderr") != 0))
            {
                if(config->tracingOutputFile){
                    fclose(config->tracingOutputFile);
                    config->tracingOutputFile = NULL;
                }
            }
            os_free(config->tracingOutputFileName);
            config->tracingOutputFileName = NULL;
        }

        if (os_strcasecmp(value, "stdout") == 0) {
            config->tracingOutputFileName = os_strdup("stdout");
            config->tracingOutputFile = stdout; /* default */
        } else if (os_strcasecmp(value, "stderr") == 0) {
            config->tracingOutputFileName = os_strdup("stderr");
            config->tracingOutputFile = stderr;
        } else {
            char * filename = os_fileNormalize(value);
            config->tracingOutputFile = fopen(filename, "a");
            config->tracingOutputFileName = os_strdup(filename);
        }
    }
}

static void
in_configSetTracingTimestamps(
    in_config  config,
    c_bool useTimestamp)
{
    if (config) {
        config->tracingTimestamps = useTimestamp;
    }
}

static void
in_configSetTracingRelativeTimestamps(
    in_config config,
    u_cfElement element,
    const c_char* timestampsPath,
    const c_char* absoluteName)
{
    u_cfElement timestampsElement;
    c_iter elements;
    c_bool success, absolute;

    elements = u_cfElementXPath(element, timestampsPath);

    if(elements){
        timestampsElement = u_cfElement(c_iterTakeFirst(elements));

        while(timestampsElement){
            success = u_cfElementAttributeBoolValue(timestampsElement, absoluteName, &absolute);

            if(success == TRUE){
                config->tracingRelativeTimestamps = (!absolute);
            }
            u_cfElementFree(timestampsElement);
            timestampsElement = u_cfElement(c_iterTakeFirst(elements));
        }
        c_iterFree(elements);
    }
}

static void
in_configSetTracingVerbosity(
    in_config config,
    const c_char* value)
{
    if(value && config){
        if(os_strcasecmp(value, "SEVERE") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_SEVERE;
        } else if(os_strcasecmp(value, "WARNING") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_WARNING;
        } else if(os_strcasecmp(value, "INFO") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_INFO;
        } else if(os_strcasecmp(value, "CONFIG") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_CONFIG;
        } else if(os_strcasecmp(value, "FINE") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_FINE;
        } else if(os_strcasecmp(value, "FINER") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_FINER;
        } else if(os_strcasecmp(value, "FINEST") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_FINEST;
        } else if(os_strcasecmp(value, "NONE") == 0){
            config->tracingVerbosityLevel = IN_LEVEL_NONE;
        }
    }
}

static void
in_configSetEnableMulticastLoopback(
    in_config  config,
    c_bool enableMulticastLoopback)
{
    config->enableMulticastLoopback = enableMulticastLoopback;
}

static void
in_configSetCoexistWithNativeNetworking(
    in_config  config,
    c_bool coexistWithNativeNetworking)
{
    config->coexistWithNativeNetworking = coexistWithNativeNetworking;
}

static void
in_configSetAllowMulticast(
    in_config  config,
    c_bool allowMulticast)
{
    config->allowMulticast = allowMulticast;
}

static void
in_configSetDomainId(
    in_config  config,
    int domainId)
{
    config->domainId = domainId;
}

static void
in_configSetParticipantIndex(
    in_config  config,
    const c_char *value)
{
  if (os_strcasecmp (value, "AUTO") == 0)
    config->participantIndex = -1;
  else
  {
    char *endptr;
    config->participantIndex = (int) strtoul (value, &endptr, 0);
    if (!(*value != '\0' && *endptr == '\0') ||
	config->participantIndex < 0 ||
	config->participantIndex >= 120)
    {
      in_printf(IN_LEVEL_CONFIG, "ddsi2: %s: invalid network id, defaulting to AUTO\n", value);
      config->participantIndex = -1;
    }
  }
}

static void
in_configEnableUndocFeatures(
    in_config config,
    const c_char* value)
{
    if(config->enableUndocFeatures){
        os_free(config->enableUndocFeatures);
    }
    config->enableUndocFeatures = os_strdup(value);
}

static void
in_configAddPeer(
    in_config config,
    const c_char* peer)
{
    c_char* tmp;

    if(config->peers){
        tmp = os_malloc(strlen(config->peers) + strlen(peer) + 2);
        os_sprintf(tmp, "%s,%s", config->peers, peer);
        os_free(config->peers);
        config->peers = tmp;
    } else {
        config->peers = os_strdup(peer);
    }
}

static void
in_configValueString(
    in_config configuration,
    u_cfElement  element,
    const char * tag,
    void  (* const setAction)(in_config config, const c_char * str) )
{
    c_iter   iter;
    u_cfData data;
    c_bool   found;
    c_char *   str;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while (data) {
        found = u_cfDataStringValue(data, &str);

        if (found == TRUE) {
            setAction(configuration, str);
            os_free(str);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

static void
in_configValueInt (
    in_config configuration,
    u_cfElement  element,
    const char * tag,
    void         (* const setAction)(in_config config, int str) )
{
    c_iter   iter;
    u_cfData data;
    c_bool   found;
    c_long    v;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while (data) {
        found = u_cfDataLongValue(data, &v);
        /* QAC EXPECT 2100; */
        if (found == TRUE) {
            setAction(configuration, (int) v);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}


static void
in_configValueBoolean(
    in_config configuration,
    u_cfElement  element,
    const char * tag,
    void         (* const setAction)(in_config config, c_bool str) )
{
    c_iter   iter;
    u_cfData data;
    c_bool   found;
    c_bool   b;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while (data) {
        found = u_cfDataBoolValue(data, &b);
        /* QAC EXPECT 2100; */
        if (found == TRUE) {
            setAction(configuration, b);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

