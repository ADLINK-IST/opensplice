/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
/**
 * Always include os_socket.h before stdsoap2.h!!!!
 * This is needed for Windows platform, since the
 * current GSOAP version (2.7) still uses
 * Windows Sockets 1.1, which conflicts with our
 * requirement for Windows Sockets 2! Nevertheless
 * Windows Sockets 2 is backwards compatible, so
 * including the winsock2.h will not cause any problems.
 */ 
#include "os_socket.h"
#include "os_stdlib.h"
#include <stdsoap2.h>

#ifndef WITH_LEAN
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_LONG642s(struct soap *soap, LONG64 n)
{
    char llstr[36];
    (void)os_lltostr(n, llstr, sizeof(llstr), NULL);
    os_sprintf(soap->tmpbuf, "%s", llstr);

    return soap->tmpbuf;
}
#endif

#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2LONG64(struct soap *soap, const char *s, LONG64 *p)
{
    if (s) {
        *p = (LONG64)os_atoll (s);
    }
    return soap->error;
}
#endif

#ifndef WITH_LEAN
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_ULONG642s(struct soap *soap, ULONG64 n)
{
    char llstr[36];
    (void)os_ulltostr(n, llstr, sizeof(llstr), NULL);
    os_sprintf(soap->tmpbuf, "%s", llstr);

    return soap->tmpbuf;
}
#endif

#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2ULONG64(struct soap *soap, const char *s, ULONG64 *p)
{
    if (s) {
        *p = (ULONG64)os_atoll (s);
    }
    return soap->error;
}
#endif
