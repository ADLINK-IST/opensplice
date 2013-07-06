#ifndef Q_PCAP_H
#define Q_PCAP_H

#include <stdio.h>

struct msghdr;

FILE *new_pcap_file (const char *name);
int write_pcap_received (FILE *fp, os_int64 tstamp, const os_sockaddr_storage *dst, const struct msghdr *hdr, unsigned sz);
int write_pcap_sent (FILE *fp, os_int64 tstamp, const os_sockaddr_storage *src, const struct msghdr *hdr, unsigned sz);

#endif /* Q_PCAP_H */

/* SHA1 not available (unoffical build.) */
