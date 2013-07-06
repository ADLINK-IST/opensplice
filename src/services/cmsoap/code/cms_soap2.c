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
    llstr[35] = '\0';
    os_sprintf(soap->tmpbuf, "%s", os_lltostr(n, &llstr[35]));

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
        soap->error = SOAP_TYPE;
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
    llstr[35] = '\0';
    os_sprintf(soap->tmpbuf, "%s", os_ulltostr(n, &llstr[35]));

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
        soap->error = SOAP_TYPE;
    }
    return soap->error;
}
#endif
