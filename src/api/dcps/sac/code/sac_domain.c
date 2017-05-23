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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_object.h"
#include "sac_objManag.h"
#include "sac_entity.h"
#include "u_domain.h"
#include "sac_report.h"

#define DDS_DomainClaim(_this, domain) \
        DDS_Object_claim(DDS_Object(_this), DDS_DOMAIN, (_Object *)domain)

#define DDS_DomainClaimRead(_this, domain) \
        DDS_Object_claim(DDS_Object(_this), DDS_DOMAIN, (_Object *)domain)

#define DDS_DomainRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_DomainCheck(_this, domain) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_DOMAIN, (_Object *)domain)

#define _Domain_get_user_entity(_this) \
        u_domain(_Entity_get_user_entity(_Entity(_Domain(_this))))

static DDS_ReturnCode_t
_Domain_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result;

    if (_this != NULL) {
        _Entity e = (_Entity)_this;
        (void) u_domainClose((u_domain)(e->uEntity));
        e->uEntity = NULL;
        result = _Entity_deinit(_this);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    return result;
}

DDS_ReturnCode_t
DDS_DomainFree (
    DDS_Domain _this)
{
    DDS_ReturnCode_t result;
    result = DDS__free(_this);
    return result;
}

DDS_Domain
DDS_DomainNew (
    const DDS_DomainId_t domainId)
{
    _Domain _this = NULL;
    u_domain uDomain;
    DDS_long timeout = 1;

    SAC_REPORT_STACK();
    if (u_domainOpen(&uDomain, NULL, domainId, timeout) != U_RESULT_OK) {
        goto err_open;
    }
    if (DDS_Object_new(DDS_DOMAIN, _Domain_deinit, (_Object *)&_this) != DDS_RETCODE_OK) {
        goto err_objNew;
    }
    if (DDS_Entity_init(_this, u_entity(uDomain)) != DDS_RETCODE_OK) {
        goto err_objInit;
    }
    _this->domainId = u_domainId(uDomain);

    SAC_REPORT_FLUSH(_this, FALSE);
    return (DDS_Domain)_this;

/* Error handling */
err_objInit:
    DDS__free(_this);
err_objNew:
    (void) u_domainClose(uDomain);
err_open:
    SAC_REPORT_FLUSH(_this, TRUE);
    return NULL;
}

DDS_DomainId_t
DDS_Domain_get_domain_id(
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    DDS_DomainId_t domainId = DDS_DOMAIN_ID_INVALID;
    _Domain domain;

    SAC_REPORT_STACK();
    if (_this != NULL) {
        result = DDS_DomainCheck(_this, &domain);
        if (result == DDS_RETCODE_OK) {
            domainId = domain->domainId;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "participant = 0x%x", _this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return domainId;
}

/*     DDS_ReturnCode_t
 *     create_persistent_snapshot(
 *         in String partition_expression,
 *         in String topic_expression,
 *         in String URI);
 */
DDS_ReturnCode_t
DDS_Domain_create_persistent_snapshot (
    DDS_Domain _this,
    const DDS_char *partition_expression,
    const DDS_char *topic_expression,
    const DDS_char *URI)
{
    DDS_ReturnCode_t result;
    _Domain domain;
    u_result uResult;

    SAC_REPORT_STACK();
    if((_this != NULL) &&
       (partition_expression != NULL) &&
       (topic_expression != NULL) &&
       (URI != NULL))
    {
        result = DDS_DomainCheck(_this, &domain);
        if(result == DDS_RETCODE_OK) {
            uResult = u_domainCreatePersistentSnapshot(
                            _Domain_get_user_entity(domain),
                            partition_expression,
                            topic_expression,
                            URI);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "domain = 0x%x, partition_expression = 0x%x, "
                   "topic_expression = 0x%x, URI = 0x%x",
                   _this, partition_expression, topic_expression, URI);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

