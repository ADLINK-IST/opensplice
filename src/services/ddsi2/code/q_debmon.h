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
#ifndef Q_DEBMON_H
#define Q_DEBMON_H

struct debug_monitor;
typedef int (*debug_monitor_cpf_t) (ddsi_tran_conn_t conn, const char *fmt, ...);
typedef int (*debug_monitor_plugin_t) (ddsi_tran_conn_t conn, debug_monitor_cpf_t cpf, void *arg);

struct debug_monitor *new_debug_monitor (int port);
void add_debug_monitor_plugin (struct debug_monitor *dm, debug_monitor_plugin_t fn, void *arg);
void free_debug_monitor (struct debug_monitor *dm);

#endif /* defined(__ospli_osplo__q_debmon__) */

/* SHA1 not available (unoffical build.) */
