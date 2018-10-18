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
#ifndef Q_HBCONTROL_H
#define Q_HBCONTROL_H

#if defined (__cplusplus)
extern "C" {
#endif

struct writer;

struct hbcontrol {
  nn_mtime_t t_of_last_write;
  nn_mtime_t t_of_last_hb;
  nn_mtime_t t_of_last_ackhb;
  nn_mtime_t tsched;
  unsigned hbs_since_last_write;
  unsigned last_packetid;
};

void writer_hbcontrol_init (struct hbcontrol *hbc);
os_int64 writer_hbcontrol_intv (const struct writer *wr, nn_mtime_t tnow);
void writer_hbcontrol_note_asyncwrite (struct writer *wr, nn_mtime_t tnow);
int writer_hbcontrol_ack_required (const struct writer *wr, nn_mtime_t tnow);
struct nn_xmsg *writer_hbcontrol_piggyback (struct writer *wr, nn_mtime_t tnow, unsigned packetid, int *hbansreq);
int writer_hbcontrol_must_send (const struct writer *wr, nn_mtime_t tnow);
struct nn_xmsg *writer_hbcontrol_create_heartbeat (struct writer *wr, nn_mtime_t tnow, int hbansreq, int issync);

#if defined (__cplusplus)
}
#endif

#endif /* Q_HBCONTROL_H */
