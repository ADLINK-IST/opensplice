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
 */
#ifndef V_MESSAGEEXT_H
#define V_MESSAGEEXT_H

#include "kernelModule.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif

OS_API void v_messageExtFree (v_messageExt pmsg);
OS_API v_messageExt v_messageExtCopyToExtType (c_type xmsgType, const struct v_message_s *vmsg);
OS_API v_message v_messageExtConvertFromExtType (c_type msgType, v_messageExt pmsg);
OS_API void v_messageExtConvertHeaderFromExt (v_message vmsg, const struct v_messageExt_s *xmsg);
OS_API void v_messageExtConvertHeaderToExt (v_messageExt xmsg, const struct v_message_s *vmsg);
OS_API void v_messageExtConvertHeaderFromExtNoAllocTime (v_message vmsg, const struct v_messageExt_s *xmsg);
OS_API void v_messageExtConvertHeaderToExtNoAllocTime (v_messageExt xmsg, const struct v_message_s *vmsg);
OS_API void v_messageEOTExtConvertFromExtNoAllocTime (v_messageEOT vmsgEOT, const struct v_messageEOTExt_s *xmsgEOT);
OS_API v_message v_messageEOTExtConvertFromExtType (v_messageEOTExt xmsg_eot);
OS_API void v_messageEOTExtConvertToExtNoAllocTime (v_messageEOTExt xmsgEOT, const struct v_messageEOT_s *vmsgEOT);
OS_API v_messageEOTExt v_messageEOTExtCopyToExtType (const struct v_messageEOT_s *vmsg);
OS_API void v_messageEOTExtFree (v_messageEOTExt xmsg);
OS_API void v_messageExtTypeFree (c_type xmsgType);
OS_API c_type v_messageExtTypeNew (v_topic topic);

struct v_messageExtCdrInfo;
struct sd_cdrControl;
struct sd_cdrInfo;

OS_API struct v_messageExtCdrInfo *v_messageExtCdrInfoNew(c_type topicMessageType, const struct sd_cdrControl *control);
OS_API void v_messageExtCdrInfoFree(struct v_messageExtCdrInfo *xci);
OS_API int v_messageExtCdrSerializeNoAllocTime (int (*f) (const struct sd_cdrInfo *ci, void *serdata, const void *data), const struct v_messageExtCdrInfo *xci, void *serdata, const struct v_message_s *vmsg);
OS_API int v_messageExtCdrDeserializeNoAllocTime (int (*f) (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src), v_message *dst, const struct v_messageExtCdrInfo *xci, os_uint32 sz, const void *src);

#if defined (__cplusplus)
}
#endif

#undef OS_API

#endif
