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
#ifndef GAPI_TOPICDESCRIPTION_H
#define GAPI_TOPICDESCRIPTION_H

#include "gapi_common.h"
#include "gapi_expression.h"
#include "gapi_entity.h"

#include "u_user.h"

#define _TopicDescription(o) ((_TopicDescription)(o))

#define gapi_topicDescriptionClaim(h,r) \
        (_TopicDescription(gapi_objectClaim(h,OBJECT_KIND_TOPICDESCRIPTION,r)))

#define gapi_topicDescriptionClaimNB(h,r) \
        (_TopicDescription(gapi_objectClaimNB(h,OBJECT_KIND_TOPICDESCRIPTION,r)))

#define _TopicDescriptionFromHandle(h) \
        (_TopicDescription(gapi_objectPeek(h,OBJECT_KIND_TOPICDESCRIPTION)))

#define _TopicDescriptionAlloc() \
        (_TopicDescription(_ObjectAlloc(OBJECT_KIND_TOPICDESCRIPTION, \
                                         C_SIZEOF(_TopicDescription), \
                                         NULL)))

C_STRUCT(_TopicDescription) {
    C_EXTENDS(_Entity);
    gapi_string             type_name;
    gapi_string             topic_name;
    gapi_long               useCount;
    q_expr                  expr;
    gapi_unsigned_long      messageOffset;
    gapi_unsigned_long      userdataOffset;
    gapi_unsigned_long      allocSize;
    gapi_topicAllocBuffer   allocBuffer;
};

_TopicDescription
_TopicDescriptionNew (
    const gapi_char *topic_name,
    const gapi_char *type_name,
    _DomainParticipant participant);

gapi_returnCode_t
_TopicDescriptionInit (
    _TopicDescription _this,
    const gapi_char *topic_name,
    const gapi_char *type_name,
    const gapi_char *expression,
    _DomainParticipant participant);

void
_TopicDescriptionDispose (
    _TopicDescription _this);

gapi_boolean
_TopicDescriptionHasType (
    _TopicDescription _this,
    const gapi_char *typeName);

gapi_returnCode_t
_TopicDescriptionFree (
    _TopicDescription _this);

gapi_boolean
_TopicDescriptionPrepareDelete (
    _TopicDescription _this);

gapi_string
_TopicDescriptionGetTypeName (
    _TopicDescription _this);

gapi_string
_TopicDescriptionGetName (
    _TopicDescription _this);

q_expr
_TopicDescriptionGetExpr (
    _TopicDescription topicDescription);

void
_TopicDescriptionIncUse (
    _TopicDescription _this);

void
_TopicDescriptionDecUse (
    _TopicDescription _this);

gapi_unsigned_long
_TopicDescriptionMessageOffset (
    _TopicDescription _this);

gapi_unsigned_long
_TopicDescriptionUserdataOffset (
    _TopicDescription _this);

gapi_unsigned_long
_TopicDescriptionAllocSize (
    _TopicDescription _this);

gapi_topicAllocBuffer
_TopicDescriptionAllocBuffer (
    _TopicDescription _this);

#if 0
gapi_expression
_TopicDescriptionCreateExpression (
    const char *topicName);
#endif

void
_TopicDescriptionCopyContext (
    _TopicDescription _this,
    _TopicDescription to);

_TypeSupport
_TopicDescriptionGetTypeSupport (
    _TopicDescription _this);

#endif
