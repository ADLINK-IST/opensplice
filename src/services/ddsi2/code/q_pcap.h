#ifndef Q_PCAP_H
#define Q_PCAP_H

#include <stdio.h>

#if defined (__cplusplus)
extern "C" {
#endif

struct msghdr;

FILE * new_pcap_file (const char *name);

void write_pcap_received
(
  FILE * fp,
  os_int64 tstamp,
  const os_sockaddr_storage * src,
  const os_sockaddr_storage * dst,
  unsigned char * buf,
  os_size_t sz
);

void write_pcap_sent
(
  FILE * fp,
  os_int64 tstamp,
  const os_sockaddr_storage * src,
  const struct msghdr * hdr,
  os_size_t sz
);

#if defined (__cplusplus)
}
#endif

#endif /* Q_PCAP_H */

/* SHA1 not available (unoffical build.) */
