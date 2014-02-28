#ifndef _DDSI_SOCKET_H_
#define _DDSI_SOCKET_H_

#include "ddsi_tran.h"

os_ssize_t ddsi_socket_read (os_socket sock, unsigned char * buf, os_size_t len, c_bool udp);
os_ssize_t ddsi_socket_write (os_socket sock, struct msghdr * msg, os_size_t len, c_bool udp);

#endif

/* SHA1 not available (unoffical build.) */
