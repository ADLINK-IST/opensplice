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

#ifndef U__DOMAIN_H
#define U__DOMAIN_H

#include "u_participant.h"
#include "u_domain.h"
#include "u_entity.h"
#include "os.h"

/** \brief Protect against process termination during domain access.
 *
 * This method is used by all other classes within this component whenever
 * the access to the domain is required.
 * Once access to the domain is no longer required the method
 * u_domainUnprotect must be called.
 */
u_result
u_domainProtect(
    u_domain _this);

/** \brief Unprotect against process termination after domain access.
 *
 * This method is used by all other classes within this component to
 * release the protection against termination set by the method
 * u_domainProtect.
 */
u_result
u_domainUnprotect(
    u_domain _this);

c_long
u_domainProtectCount(
    u_domain _this);

u_result
u_domainAddParticipant (
    u_domain _this,
    u_participant p);

u_result
u_domainRemoveParticipant (
    u_domain _this,
    u_participant p);

u_result
u_domainDetachParticipants (
    u_domain _this);

c_bool
u_domainCheckHandleServer (
    u_domain _this,
    c_long serverId);

c_voidp
u_domainGetCopy (
    u_domain _this,
    u_entityCopy copy,
    void* copyArg);

c_address
u_domainHandleServer(
    u_domain _this);

c_voidp
u_domainAddress(
    u_domain _this);

#endif
