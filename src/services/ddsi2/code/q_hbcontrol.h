#ifndef Q_HBCONTROL_H
#define Q_HBCONTROL_H

struct writer;

struct hbcontrol {
  os_int64 t_of_last_write;
  os_int64 t_of_last_hb;
  os_int64 t_of_last_ackhb;
  os_int64 tsched;
  unsigned hbs_since_last_write;
  unsigned last_packetid;
};

void writer_hbcontrol_init (struct hbcontrol *hbc);
os_int64 writer_hbcontrol_intv (const struct writer *wr, os_int64 tnow);
void writer_hbcontrol_note_asyncwrite (struct writer *wr, os_int64 tnow);
int writer_hbcontrol_ack_required (const struct writer *wr, os_int64 tnow);
struct nn_xmsg *writer_hbcontrol_piggyback (struct writer *wr, os_int64 tnow, unsigned packetid, int *hbansreq);
int writer_hbcontrol_must_send (const struct writer *wr, os_int64 tnow);
struct nn_xmsg *writer_hbcontrol_create_heartbeat (struct writer *wr, os_int64 tnow, int hbansreq);

#endif /* Q_HBCONTROL_H */

/* SHA1 not available (unoffical build.) */
