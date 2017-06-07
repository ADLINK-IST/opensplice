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




/* SHA1 not available (unoffical build.) */
