#ifdef DDSI_INCLUDE_ENCRYPTION

/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef Q_SECURITY_H
#define Q_SECURITY_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Generic class */
C_CLASS(q_securityEncoderSet);
C_CLASS(q_securityDecoderSet);

/* Set of supported ciphers */
typedef enum 
{
  Q_CIPHER_UNDEFINED,
  Q_CIPHER_NULL,
  Q_CIPHER_BLOWFISH,
  Q_CIPHER_AES128,
  Q_CIPHER_AES192,
  Q_CIPHER_AES256,
  Q_CIPHER_NONE,
  Q_CIPHER_MAX
} q_cipherType;

void ddsi_security_plugin (void);

#if defined (__cplusplus)
}
#endif

#endif
#endif
