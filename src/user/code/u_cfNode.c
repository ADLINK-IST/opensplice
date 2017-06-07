/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "vortex_os.h"

#include "u_user.h"

#include "u__cfNode.h"
#include "u__cfElement.h"
#include "u__cfAttribute.h"
#include "u__cfData.h"
#include "u__participant.h"
#include "u__observable.h"
#include "u__entity.h"
#include "v_public.h"
#include "v_cfNode.h"
#include "v_configuration.h"

#include "os_stdlib.h"
#include "os_report.h"


u_result
u_cfNodeReadClaim(
    const u_cfNode node,
    v_cfNode* kernelNode)
{
    v_participant kp;
    v_configuration config;
    u_result r;

    assert(node != NULL);
    assert(kernelNode != NULL);

    *kernelNode = NULL;
    r = u_observableReadClaim(u_observable(node->participant),(v_public*)&kp, C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        r = u_handleClaim(node->configuration, &config);
        if (r == U_RESULT_OK) {
            if(config != NULL){
                *kernelNode = v_configurationGetNode(config, node->id);
                if (*kernelNode == NULL) {
                    r = U_RESULT_INTERNAL_ERROR;
                    u_observableRelease(u_observable(node->participant), C_MM_RESERVATION_ZERO);
                }
            } else {
                r = U_RESULT_INTERNAL_ERROR;
                u_observableRelease(u_observable(node->participant), C_MM_RESERVATION_ZERO);
                OS_REPORT(OS_ERROR, "u_cfNodeReadClaim", r,
                          "Internal error");
            }
        } else {
            u_observableRelease(u_observable(node->participant), C_MM_RESERVATION_ZERO);
            OS_REPORT(OS_ERROR, "u_cfNodeReadClaim", r,
                      "Could not claim configuration data");
        }
    } else {
        OS_REPORT(OS_ERROR, "u_cfNodeReadClaim", r,
                  "Could not protect kernel access, "
                  "Therefore failed to claim configuration data");
    }
    return r;
}

void
u_cfNodeRelease(
    const u_cfNode node)
{
    assert(node != NULL);
    u_handleRelease(node->configuration);
    u_observableRelease(u_observable(node->participant), C_MM_RESERVATION_ZERO);
}

void
u_cfNodeInit(
    u_cfNode _this,
    const u_participant participant,
    const v_cfNode kNode)
{
    v_configuration config;

    assert(_this != NULL);
    assert(participant != NULL);
    assert(kNode != NULL);

    config = v_cfNodeConfiguration(kNode);
    _this->configuration = u_handleNew(v_public(config));
    _this->participant = participant;
    _this->kind = v_cfNodeKind(kNode);
    _this->id = kNode->id;
}

void
u_cfNodeDeinit(
    u_cfNode node)
{
    OS_UNUSED_ARG(node);
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
    const u_cfNode node)
{
    assert(node != NULL);

    return node->participant;
}

v_cfKind
u_cfNodeKind(
    const u_cfNode node)
{
    assert(node != NULL);

    return node->kind;
}

c_char *
u_cfNodeName(
    const u_cfNode node)
{
    v_cfNode kNode;
    const c_char *vname;
    c_char *name;
    u_result result;

    assert(node != NULL);

    name = NULL;
    result = u_cfNodeReadClaim(node, &kNode);
    if (result == U_RESULT_OK) {
        vname = v_cfNodeGetName(kNode);
        if (vname != NULL) {
            name = os_strdup(vname);
        }
        u_cfNodeRelease(node);
    }
    return name;
}
