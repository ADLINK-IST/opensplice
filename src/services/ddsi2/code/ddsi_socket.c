#include "ddsi_socket.h"
#include "q_config.h"
#include "q_log.h"
#include "q_pcap.h"

os_ssize_t ddsi_socket_read (os_socket sock, unsigned char * buf, os_size_t len, c_bool udp)
{
  int err;
  os_ssize_t ret;
  struct msghdr msghdr;
  os_sockaddr_storage src;
  struct iovec msg_iov;
  socklen_t srclen = (socklen_t) sizeof (src);

  msg_iov.iov_base = (void*) buf;
  msg_iov.iov_len = len;

  memset (&msghdr, 0, sizeof (msghdr));

  msghdr.msg_name = &src;
  msghdr.msg_namelen = srclen;
  msghdr.msg_iov = &msg_iov;
  msghdr.msg_iovlen = 1;

  do
  {
    /* Returns -1 on error or 0 on shutdown */

    ret = recvmsg (sock, &msghdr, 0);
    if (ret == -1)
    {
      err = os_sockError ();
    }
  }
  while ((ret == -1) && ((err == os_sockEINTR) ||
    (err == os_sockEAGAIN) || (err == os_sockEWOULDBLOCK)));

  if (ret == -1)
  {
    /* For TCP if connection broken, can get a variety of expected
       errors, closing a UDP one can also cause some errors. Only one
       seen so far is NOTSOCK on Windows. */
    if (udp && err != os_sockENOTSOCK)
    {
      NN_ERROR3 ("UDP recvmsg sock %d: ret %d errno %d\n", sock, ret, err);
    }
  }
  else if ((ret > 0) && udp)
  {
    /* Check for udp packet truncation */

    if
    (
      (((os_size_t) ret) > len)
#if SYSDEPS_MSGHDR_FLAGS
      || (msghdr.msg_flags & MSG_TRUNC)
#endif
    )
    {
      char addrbuf[INET6_ADDRSTRLEN_EXTENDED];
      sockaddr_to_string_with_port (addrbuf, &src);
      NN_WARNING3 ("%s => %d truncated to %d\n", addrbuf, ret, len);
    }
  }

  return ret;
}

os_ssize_t ddsi_socket_write (os_socket sock, struct msghdr * msg, os_size_t len, c_bool udp)
{
  int err;
  os_ssize_t ret;
  int flags = 0;
  unsigned retry = 2;

#ifdef MSG_NOSIGNAL
  flags |= MSG_NOSIGNAL;
#endif

  if (!udp)
  {
    msg->msg_name = NULL;
    msg->msg_namelen = 0;
  }

  while (TRUE)
  {
    ret = sendmsg (sock, msg, flags);
    if (ret == -1)
    {
      err = os_sockError ();
      if (err != os_sockEINTR)
      {
        /* EPERM transient generated on some RedHat systems (out dated check?) */

        if (udp && (err == os_sockEPERM) && (retry-- > 0))
        {
          continue;
        }
        break;
      }
    }
    else
    {
      len -= ret;
      if (len == 0)
      {
        break;
      }
    }
  }

  if (udp && (ret > 0) && gv.pcap_fp)
  {
    os_sockaddr_storage sa;
    socklen_t alen = sizeof (sa);
    getsockname (sock, (struct sockaddr *) &sa, &alen);
    write_pcap_sent (gv.pcap_fp, now (), &sa, msg, ret);
  }

  return ret;
}

/* SHA1 not available (unoffical build.) */
