/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_ADDRESS_H_
#define IN_ADDRESS_H_

#include <assert.h>

#include "os_socket.h"
#include "os_iterator.h" /* for os_equality */
#include "in_commonTypes.h"
#include "in_ddsiDeserializer.h"

/** \brief Enough space for IPv6 address strings */
#define IN_ADDRESS_STRING_LEN (INET6_ADDRSTRLEN)

#define IN_IPV4_ADDRESS_SIZE (4)
#define IN_IPV6_ADDRESS_SIZE (16)
#define IN_IPV4_OFFSET     (IN_IPV6_ADDRESS_SIZE-IN_IPV4_ADDRESS_SIZE)
#define IN_IP_ADDRESS_SIZE  IN_IPV6_ADDRESS_SIZE

typedef in_octet in_addressIPv4[IN_IPV4_ADDRESS_SIZE];
typedef in_octet in_addressIPv6[IN_IPV6_ADDRESS_SIZE];

/** \brief IPv6/v4 address
 *
 * The DDSi Spec 2.1 declares the IPv4/IPv6 address type as follows:
 *
 * In case of IPv4 the leading 12 octets of the address must be zero.
 * The last 4 octets are used to store the IPv4 address. The mapping
 * between the dot-notation “a.b.c.d” of an IPv4 address and its
 * representation in the address field of a Locator_t is:
 *  address = (0,0,0,0,0,0,0,0,0,0,0,0,a,b,c,d}
 *
 *  IPv6 addresses typically use a shorthand hexadecimal notation that
 *  maps one-to-one to the 16 octets in the address field. For example
 *  the representation of the IPv6 address “FF00:4501:0:0:0:0:0:32” is:
 *  address = (0xff,0,0x45,0x01,0,0,0,0,0,0,0,0,0,0,0,0x32}
 *  */

OS_STRUCT(in_address)
{
	in_octet octets[IN_IPV6_ADDRESS_SIZE];
};

/** \brief IPv6/v4 address
 *
 * The DDSi Spec 2.1 declares the IPv4/IPv6 address type as follows:
 *
 * In case of IPv4 the leading 12 octets of the address must be zero.
 * The last 4 octets are used to store the IPv4 address. The mapping
 * between the dot-notation “a.b.c.d” of an IPv4 address and its
 * representation in the address field of a Locator_t is:
 *  address = (0,0,0,0,0,0,0,0,0,0,0,0,a,b,c,d}
 *
 *  IPv6 addresses typically use a shorthand hexadecimal notation that
 *  maps one-to-one to the 16 octets in the address field. For example
 *  the representation of the IPv6 address “FF00:4501:0:0:0:0:0:32” is:
 *  address = (0xff,0,0x45,0x01,0,0,0,0,0,0,0,0,0,0,0,0x32}
 *  */

typedef enum {
	IN_ADDRESS_INVALID,
	IN_ADDRESS_V4,
	IN_ADDRESS_V6,
	IN_ADDRESS_SENTINEL
} in_addressKind;

#define IN_ADDRESS_ANY ((in_address)NULL)

/** \brief macro to access the IPv4 relevant octets */
#define in_addressIPv4Ptr(_addr) ((_addr)->octets + IN_IPV4_OFFSET)

/** \brief macro to access the IPv6 relevant octets */
#define in_addressIPv6Ptr(_addr) ((_addr)->octets)

/** \brief macro to access the IPv6 relevant octets */
#define in_addressPtr(_addr) ((_addr)->octets)

/** \brief  */
/** \brief The address categories being supported */
typedef enum {
    IN_ADDRESS_TYPE_UNKNOWN,
    IN_ADDRESS_TYPE_LOOPBACK,
    IN_ADDRESS_TYPE_BROADCAST,
    IN_ADDRESS_TYPE_MULTICAST,
    IN_ADDRESS_TYPE_UNICAST,
    IN_ADDRESS_TYPE_SENTINEL
} in_addressType;


/** \brief verify leading octets are zeroed out */
os_boolean
in_addressValidIPv4(in_address _this);

/** \brief init
 * \param addressString must be valid dotted expression
 * \return FALSE if bad string, otherwise TRUE */
os_boolean
in_addressInitFromString(
		in_address _this,
		const os_char *addressString);


/** \brief convert the string into IP address with default
 * \return TRUE in case either one has been converted successfully, otherwise FALSE
 * Able to handle IPv4 and IPv6 address strings */
os_boolean
in_addressInitFromStringWithDefault(
		in_address _this,
		const os_char* addressString,
		const os_char* addressOnError);

/** \brief init the IP address from socket attributes
 *
 * Here the user does not need to specify the address family
 * (such as AF_INET, or AF_INET6), as this information can be read
 * from socket parameter.  */
void
in_addressInitFromSockAddr(in_address _this, struct sockaddr_in *addr);


/** \brief init posix IPv4 address */
void
in_addressInitFromInAddr(in_address _this, struct in_addr *addr);

/** \brief init posix IPv6 address */
void
in_addressInitFromIn6Addr(in_address _this, struct in6_addr *addr);

/** \brief copy IP address to IPv4 socket address */
void
in_addressToInAddr(in_address _this, struct in_addr *addr);

/** \brief copy IP address to IPv6 socket address */
void
in_addressToIn6Addr(in_address _this, struct in6_addr *addr);

/** \brief get type of address, such as multicast, or loopback.
 * \return one of IN_ADDRESS_TYPE_MULTICAST, IN_ADDRESS_TYPE_LOOPBACK, otherwise IN_ADDRESS_TYPE_UNKNOWN.
 * IPv4 broadcast can not be detected, as this belongs to the network configuration */
in_addressType
in_addressGetType(in_address _this);

/** \brief compare IP address,
 * \return return one of SO_LT, OS_EQ, or OS_GT */
os_equality
in_addressCompare(in_address _this, in_address _other);

/** \brief compare with IP address of socket
 *
 * Verify if the socket belongs to same host */
os_boolean
in_addressMatches(in_address _this, struct sockaddr* sockAddr);

/** \brief return the address family, either AF_INET or AF_INET6 */
int /* Note: platform specific 'int' */
in_addressGetFamilyFromString(const char *addressString);

/** \brief compare operation, returns TRUE if equal, otherwise FALSE */
os_boolean
in_addressEqual(const in_address _this, const in_address other);

/** \brief convert IP address into string,
 *
 * The buffer length must equal IN_ADDRESS_STRING_LEN */
os_char*
in_addressToString(const in_address _this, os_char *buf, os_uint buflen);

/** \brief init */
void
in_addressInitAny(in_address _this);

/** \brief parse 16 octets from CDR buffer, return -1 in case of error */
in_long
in_addressInitFromBuffer(
		in_address _this,
		in_ddsiDeserializer deserializer);

/** init the IPv4 padding octets */
void
in_addressInitIPv4Padding(in_address _this);

/** init the IPv4 octets */
void
in_addressInitIPv4Addr(in_address _this, const in_octet *fourOctets);

/** \brief init IPv4 address, concatinating
 *  two operations with comma-operator */
void
in_addressInitIPv4(in_address _this, const in_octet *octets);

/** \brief init  IPv6 address */
void
in_addressInitIPv6(in_address _this, const in_octet *octets);

/** \brief copy IP address from _other to _this */
void
in_addressCopy(in_address _this, const in_address from);

/** \brief verify the address is IPv4 compatible */
os_boolean
in_addressIsIPv4Compatible(const in_address _this);

/** \brief verify if all octets 0 */
os_boolean
in_addressIsUnspecified(const in_address _this);


#endif /* IN_ADDRESS_H_ */
