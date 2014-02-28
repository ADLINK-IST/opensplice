#ifndef Q_NWIF_H
#define Q_NWIF_H

#include "os_socket.h"
#include "c_base.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define INET6_ADDRSTRLEN_EXTENDED (INET6_ADDRSTRLEN + 8) /* 13: '[' + ']' + ':' + PORT */
#define MAX_INTERFACES 32
struct nn_interface {
  os_sockaddr_storage addr;
  os_sockaddr_storage netmask;
  os_uint if_index;
  unsigned mc_capable: 1;
  unsigned point_to_point: 1;
  char *name;
};

char *sockaddr_to_string_no_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const os_sockaddr_storage *src);
char *sockaddr_to_string_with_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const os_sockaddr_storage *src);
void print_sockerror (const char *msg);
int make_socket
(
  os_socket * socket,
  unsigned short port,
  c_bool stream,
  c_bool reuse,
  const os_sockaddr_storage * mcip,
  const char * address
);
int find_own_ip (const char *requested_address);
unsigned short get_socket_port (os_socket socket);
int join_mcgroups (os_socket socket, const os_sockaddr_storage *mcip);
void sockaddr_set_port (os_sockaddr_storage *addr, unsigned short port);
unsigned short sockaddr_get_port (const os_sockaddr_storage *addr);
  
#if defined (__cplusplus)
}
#endif


#endif /* Q_NWIF_H */

/* SHA1 not available (unoffical build.) */
