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

#include "os.h"

#include "u_user.h"

#include "u__cfNode.h"
#include "u__cfElement.h"
#include "u__cfAttribute.h"
#include "u__cfData.h"
#include "u__participant.h"
#include "u__entity.h"
#include "v_public.h"
#include "v_cfNode.h"
#include "v_configuration.h"

#include "os_stdlib.h"
#include "os_report.h"


u_result
u_cfNodeReadClaim(
    u_cfNode node,
    v_cfNode* kernelNode)
{
    v_participant kp;
    v_configuration config;
    u_result r;

    *kernelNode = NULL;
    if (node != NULL || kernelNode == NULL) {
        r = u_entityReadClaim(u_entity(node->participant),(v_entity*)&kp);
        if (r == U_RESULT_OK) {
            r = u_handleClaim(node->configuration, &config);
            if (r == U_RESULT_OK) {
                if(config != NULL){
                    *kernelNode = v_configurationGetNode(config, node->id);
                    if (*kernelNode == NULL) {
                        r = U_RESULT_INTERNAL_ERROR;
                        u_entityRelease(u_entity(node->participant));
                    }
                } else {
                    r = U_RESULT_INTERNAL_ERROR;
                    u_entityRelease(u_entity(node->participant));
                    OS_REPORT(OS_ERROR, "u_cfNodeReadClaim", 0,
                              "Internal error");
                }
            } else {
                u_entityRelease(u_entity(node->participant));
                OS_REPORT(OS_ERROR, "u_cfNodeReadClaim", 0,
                          "Could not claim configuration data");
            }
        } else {
            OS_REPORT(OS_ERROR, "u_cfNodeReadClaim", 0,
                      "Could not protect kernel access, "
                      "Therefore failed to claim configuration data");
        }
    } else {
        r = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_cfNodeReadClaim", 0,
                  "No configuration data specified to claim");
    }
    return r;
}

u_result
u_cfNodeRelease(
    u_cfNode node)
{
    u_result r;

    if (node != NULL) {
        u_handleRelease(node->configuration);
        r = u_entityRelease(u_entity(node->participant));
        if (r != U_RESULT_OK) {
            OS_REPORT(OS_ERROR, "u_cfNodeRelease", 0,
                      "Release Participant failed.");
        }
    } else {
        r = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_cfNodeRelease", 0,
                  "No configuration data specified to release");
    }
    return r;
}

void
u_cfNodeInit(
    u_cfNode _this,
    u_participant participant,
    v_cfNode kNode)
{
    v_configuration config;

    if (_this != NULL) {
        config = v_cfNodeConfiguration(kNode);
        _this->configuration = u_handleNew(v_public(config));
        _this->participant = participant;
        _this->kind = v_cfNodeKind(kNode);
        _this->id = kNode->id;
    } else {
        OS_REPORT(OS_ERROR, "u_cfNodeInit", 0, "This reference is NIL");
    }
}

void
u_cfNodeDeinit(
    u_cfNode node)
{
}

void
u_cfNodeFree (
    u_cfNode node)
{
    switch(u_cfNodeKind(node)) {
    case V_CFELEMENT:
        u_cfElementFree(u_cfElement(node));
    break;
    case V_CFATTRIBUTE:
        u_cfAttributeFree(u_cfAttribute(node));
    break;
    case V_CFDATA:
        u_cfDataFree(u_cfData(node));
    break;
    default:
    break;
    }
}

u_participant
u_cfNodeParticipant(
    u_cfNode node)
{
    u_participant p;

    if (node == NULL) {
        p = NULL;
    } else {
        p = node->participant;
    }
    return p;
}

v_cfKind
u_cfNodeKind(
    u_cfNode node)
{
    assert(node != NULL);

    return node->kind;
}

c_char *
u_cfNodeName(
    u_cfNode node)
{
    v_cfNode kNode;
    const c_char *vname;
    c_char *name;
    c_ulong length;
    u_result result;

    name = NULL;
    if (node != NULL) {
        result = u_cfNodeReadClaim(node, &kNode);
        if (result == U_RESULT_OK) {
            vname = v_cfNodeGetName(kNode);
            length = (c_ulong)strlen(vname);
            name = os_malloc(length + 1U);
            os_strncpy(name, vname, length);
            name[length] = 0;
            u_cfNodeRelease(node);
        }
    }
    return name;
}
