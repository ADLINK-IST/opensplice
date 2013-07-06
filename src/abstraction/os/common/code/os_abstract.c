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
pa_endianNess
pa_getEndianNess (
    void)
{
#ifdef PA_LITTLE_ENDIAN
    return pa_endianLittle;
#else
    return pa_endianBig;
#endif
}
