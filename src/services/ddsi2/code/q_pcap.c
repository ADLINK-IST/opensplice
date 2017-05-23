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
#include <stdio.h>
#include <assert.h>

#include "os_mutex.h"

#include "q_log.h"
#include "q_time.h"
#include "q_config.h"
#include "q_globals.h"
#include "q_bswap.h"
#include "sysdeps.h"
#include "q_pcap.h"

/* pcap format info taken from http://wiki.wireshark.org/Development/LibpcapFileFormat */

#define LINKTYPE_RAW 101 /* Raw IP; the packet begins with an IPv4 or IPv6 header, with the "version" field of the header indicating whether it's an IPv4 or IPv6 header. */

typedef struct pcap_hdr_s {
  os_uint32 magic_number;   /* magic number */
  unsigned short version_major;  /* major version number */
  unsigned short version_minor;  /* minor version number */
  os_int32  thiszone;       /* GMT to local correction */
  os_uint32 sigfigs;        /* accuracy of timestamps */
  os_uint32 snaplen;        /* max length of captured packets, in octets */
  os_uint32 network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
  os_int32 ts_sec;          /* timestamp seconds (orig: unsigned) */
  os_int32 ts_usec;         /* timestamp microseconds (orig: unsigned) */
  os_uint32 incl_len;       /* number of octets of packet saved in file */
  os_uint32 orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

typedef struct ipv4_hdr_s {
  unsigned char version_hdrlength;
  unsigned char diffserv_congestion;
  unsigned short totallength;
  unsigned short identification;
  unsigned short flags_fragment_offset;
  unsigned char ttl;
  unsigned char proto;
  unsigned short checksum;
  os_uint32 srcip;
  os_uint32 dstip;
} ipv4_hdr_t;

typedef struct udp_hdr_s {
  unsigned short srcport;
  unsigned short dstport;
  unsigned short length;
  unsigned short checksum;
} udp_hdr_t;

static const ipv4_hdr_t ipv4_hdr_template = {
  (4 << 4) | 5, /* IPv4, minimum header length */
  0 | 0,        /* no diffserv, congestion */
  0,            /* total length will be overridden */
  0,            /* not fragmenting, so irrelevant */
  0,            /* not fragmenting */
  0,            /* TTL: received has it at 128, sent at 255 */
  17,           /* UDP */
  0,            /* checksum will be overridden */
  0,            /* srcip: set per packet */
  0             /* dstip: set per packet */
};

#define IPV4_HDR_SIZE 20
#define UDP_HDR_SIZE 8

FILE *new_pcap_file (const char *name)
{
  FILE *fp;
  pcap_hdr_t hdr;

  if ((fp = fopen (name, "wb")) == NULL)
  {
    NN_WARNING1 ("packet capture disabled: file %s could not be opened for writing\n", name);
    return NULL;
  }

  hdr.magic_number = 0xa1b2c3d4;
  hdr.version_major = 2;
  hdr.version_minor = 4;
  hdr.thiszone = 0;
  hdr.sigfigs = 0;
  hdr.snaplen = 65535;
  hdr.network = LINKTYPE_RAW;
  fwrite (&hdr, sizeof (hdr), 1, fp);

  return fp;
}

static void write_data (FILE *fp, const struct msghdr *msghdr, os_size_t sz)
{
  size_t i, n = 0;
  for (i = 0; i < (size_t) msghdr->msg_iovlen && n < sz; i++)
  {
    os_size_t m1 = msghdr->msg_iov[i].iov_len;
    os_size_t m = (n + m1 <= sz) ? m1 : sz - n;
    fwrite (msghdr->msg_iov[i].iov_base, m, 1, fp);
    n += m;
  }
  assert (n == sz);
}

static unsigned short calc_ipv4_checksum (const unsigned short *x)
{
  os_uint32 s = 0;
  int i;
  for (i = 0; i < 10; i++)
  {
    s += x[i];
  }
  s = (s & 0xffff) + (s >> 16);
  return (unsigned short) ~s;
}

void write_pcap_received
(
  FILE * fp,
  nn_wctime_t tstamp,
  const os_sockaddr_storage * src,
  const os_sockaddr_storage * dst,
  unsigned char * buf,
  os_size_t sz
)
{
  if (!config.useIpv6)
  {
    pcaprec_hdr_t pcap_hdr;
    union {
      ipv4_hdr_t ipv4_hdr;
      unsigned short x[10];
    } u;
    udp_hdr_t udp_hdr;
    os_size_t sz_ud = sz + UDP_HDR_SIZE;
    os_size_t sz_iud = sz_ud + IPV4_HDR_SIZE;
    os_mutexLock (&gv.pcap_lock);
    wctime_to_sec_usec_32 (&pcap_hdr.ts_sec, &pcap_hdr.ts_usec, tstamp);
    pcap_hdr.incl_len = pcap_hdr.orig_len = (os_uint32) sz_iud;
    fwrite (&pcap_hdr, sizeof (pcap_hdr), 1, fp);
    u.ipv4_hdr = ipv4_hdr_template;
    u.ipv4_hdr.totallength = toBE2u ((unsigned short) sz_iud);
    u.ipv4_hdr.ttl = 128;
    u.ipv4_hdr.srcip = ((os_sockaddr_in*) src)->sin_addr.s_addr;
    u.ipv4_hdr.dstip = ((os_sockaddr_in*) dst)->sin_addr.s_addr;
    u.ipv4_hdr.checksum = calc_ipv4_checksum (u.x);
    fwrite (&u.ipv4_hdr, sizeof (u.ipv4_hdr), 1, fp);
    udp_hdr.srcport = ((os_sockaddr_in*) src)->sin_port;
    udp_hdr.dstport = ((os_sockaddr_in*) dst)->sin_port;
    udp_hdr.length = toBE2u ((unsigned short) sz_ud);
    udp_hdr.checksum = 0; /* don't have to compute a checksum for UDPv4 */
    fwrite (&udp_hdr, sizeof (udp_hdr), 1, fp);
    fwrite (buf, sz, 1, fp);
    os_mutexUnlock (&gv.pcap_lock);
  }
}

void write_pcap_sent
(
  FILE * fp,
  nn_wctime_t tstamp,
  const os_sockaddr_storage * src,
  const struct msghdr * hdr,
  os_size_t sz
)
{
  if (!config.useIpv6)
  {
    pcaprec_hdr_t pcap_hdr;
    union {
      ipv4_hdr_t ipv4_hdr;
      unsigned short x[10];
    } u;
    udp_hdr_t udp_hdr;
    os_size_t sz_ud = sz + UDP_HDR_SIZE;
    os_size_t sz_iud = sz_ud + IPV4_HDR_SIZE;
    os_mutexLock (&gv.pcap_lock);
    wctime_to_sec_usec_32 (&pcap_hdr.ts_sec, &pcap_hdr.ts_usec, tstamp);
    pcap_hdr.incl_len = pcap_hdr.orig_len = (os_uint32) sz_iud;
    fwrite (&pcap_hdr, sizeof (pcap_hdr), 1, fp);
    u.ipv4_hdr = ipv4_hdr_template;
    u.ipv4_hdr.totallength = toBE2u ((unsigned short) sz_iud);
    u.ipv4_hdr.ttl = 255;
    u.ipv4_hdr.srcip = ((os_sockaddr_in*) src)->sin_addr.s_addr;
    u.ipv4_hdr.dstip = ((os_sockaddr_in*) hdr->msg_name)->sin_addr.s_addr;
    u.ipv4_hdr.checksum = calc_ipv4_checksum (u.x);
    fwrite (&u.ipv4_hdr, sizeof (u.ipv4_hdr), 1, fp);
    udp_hdr.srcport = ((os_sockaddr_in*) src)->sin_port;
    udp_hdr.dstport = ((os_sockaddr_in*) hdr->msg_name)->sin_port;
    udp_hdr.length = toBE2u ((unsigned short) sz_ud);
    udp_hdr.checksum = 0; /* don't have to compute a checksum for UDPv4 */
    fwrite (&udp_hdr, sizeof (udp_hdr), 1, fp);
    write_data (fp, hdr, sz);
    os_mutexUnlock (&gv.pcap_lock);
  }
}

/* SHA1 not available (unoffical build.) */
