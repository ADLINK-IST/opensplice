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
/* Feature macros:

   - ENCRYPTION: support for encryption
     requires: NETWORK_PARTITIONS

   - SSM: support for source-specific multicast
     requires: NETWORK_PARTIITONS
     also requires platform support; SSM is silently disabled if the
     platform doesn't support it

   - BANDWIDTH_LIMITING: transmit-side bandwidth limiting
     requires: NETWORK_CHANNELS (for now, anyway)

   - IPV6: support for IPV6
     requires: platform support (which itself is not part of DDSI)

   - NETWORK_PARTITIONS: support for multiple network partitions

   - NETWORK_CHANNELS: support for multiple network channels

*/

#ifdef DDSI_INCLUDE_ENCRYPTION
  #ifndef DDSI_INCLUDE_NETWORK_PARTITIONS
    #error "ENCRYPTION requires NETWORK_PARTITIONS"
  #endif
#endif

#ifdef DDSI_INCLUDE_SSM
  #ifndef DDSI_INCLUDE_NETWORK_PARTITIONS
    #error "SSM requires NETWORK_PARTITIONS"
  #endif

  #include "os_socket.h"
  #ifndef OS_SOCKET_HAS_SSM
    #error "OS_SOCKET_HAS_SSM should be defined"
  #elif ! OS_SOCKET_HAS_SSM
    #undef DDSI_INCLUDE_SSM
  #endif
#endif

#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
  #ifndef DDSI_INCLUDE_NETWORK_CHANNELS
    #error "BANDWIDTH_LIMITING requires NETWORK_CHANNELS"
  #endif
#endif
