/*
 * in_address.c
 *
 *  Created on: Jan 8, 2009
 *      Author: frehberg
 */
/* interface */
#include "in_address.h"

/* implementation */
#include <string.h> /* memcmp */
#include <assert.h>

#include "in_ddsiDefinitions.h"
#include "os_heap.h"
#include "in_report.h"
#include "in_ddsiDeserializer.h"
#include "in_ddsiSerializer.h"
#include "in__config.h"

/** \brief init */
os_boolean
in_addressInitFromString(in_address _this, const os_char *addressString)
{
	os_boolean result = FALSE;
	in_octet *addressDest = NULL;
	int addressFamily;
	int retval;

	if (addressString) {
		/* very likely IPv6 if containing ':' os_character */
		if (strchr(addressString, ':')!=NULL) {
			/* found ':' character, seems to be IPv6 string */
			addressFamily = AF_INET6;
			addressDest   = in_addressIPv6Ptr(_this);
		}
		else  {
			/* seems to be IPv4 string */
			addressFamily = AF_INET;
			/* the last 4 octets */
			addressDest   = in_addressIPv4Ptr(_this);
			/* init prefix with IPv4 mapping */
			in_addressInitIPv4Padding(_this);
		}
#if 0
		/* This code has been disabled due to win32 portability issues. IPv4/v6
		 * conversion shall be provided by abstraction layer in future. Until then
		 * IPv6 address conversion is not supported. */

		/* returns 1 on success, 0 if bad string format, -1 if bad family */
		retval = inet_pton(addressFamily, addressString, addressDest);
#else
		/* portable code, but does not cover IPv6 */
		if (addressFamily == AF_INET6) {
			retval = 0; /* error, IPv6 not supported */
		} else {
			in_addr_t networkByteOrderAddress = 0;
			/* Note, different return value semantic:
			 * returns INADDR_NONE on error */
		    networkByteOrderAddress = inet_addr(addressString);
		    if (networkByteOrderAddress == INADDR_NONE &&
		    	strncmp(addressString, "255.255.255.255", strlen("255.255.255.255"))!=0) {
		    	/* inet_addr returns INADDR_NONE, but as INADDR_NONE(0xffffffff) is
		    	 * a valid address we made sure that the string does not contain the well
		    	 * known IPv4 broadcast address */
		    	retval = 0; /* error */
		    } else {
		    	retval = 1; /* success */
		    	/* store IPv4 octets in same order in last 4 slots of array */
		    	*((in_addr_t*) addressDest) =  networkByteOrderAddress;
		    }
		}
#endif
		if (retval <= 0) {
			/* conversion failed, bad format string */
			result = FALSE;
			IN_REPORT_WARNING_2("in_addressInitFromString",
		    		            "invalid networking address %s (%s) specified.",
		    		            addressString,
		    		            (addressFamily==AF_INET ? "AF_INET" : "AF_INET6"));
		}
		else {
			result = TRUE;
		}
	}

	return result;
}

void
in_addressInitFromSockAddr(in_address _this, struct sockaddr_in *sockaddr)
{
	switch (sockaddr->sin_family) {
	case AF_INET:
		in_addressInitFromInAddr(_this,
						&((struct sockaddr_in*)sockaddr)->sin_addr);
				break;

	case AF_INET6:
		in_addressInitFromIn6Addr(_this,
						&((struct sockaddr_in6*)sockaddr)->sin6_addr);
		break;

	default:
		/* unknown address familiy, not supported */
		in_addressInitAny(_this); /* error state */
		IN_REPORT_WARNING_1("in_addressInitFromSockAddr",
	    		            "socket address family not supported %u.",
	    		            sockaddr->sin_family);
	}
}

/** \brief convert the string into IP address with default
 * \return TRUE in case either one has been converted successfully, otherwise FALSE
 * Able to handle IPv4 and IPv6 address strings */
os_boolean
in_addressInitFromStringWithDefault(
	in_address _this,
    const os_char *addressString,
    const os_char *addressOnError)
{
    os_boolean result = FALSE;

    if (!in_addressInitFromString(_this, addressString)) {
    	IN_REPORT_WARNING_2("in_addressInitFromStringWithDefault",
    	            "invalid networking address %s specified, "
    	            "switching to %s",
    	            addressString, addressOnError);
    	if (!in_addressInitFromString(_this, addressOnError)) {
    		IN_REPORT_WARNING_1("in_addressInitFromStringWithDefault",
    		            "invalid fallback networking address %s specified.",
    		            addressOnError);
    		in_addressInitAny(_this);
    	} else {
    		/* addressOnError is valid */
    		result = TRUE;
    	}
    } else {
    	/* primary addressString is valid */
    	result = TRUE;
    }

    return result;
}

/** \brief init posix IPv4 address */
void
in_addressInitFromInAddr(in_address _this, struct in_addr *addr)
{

	in_addressInitIPv4(_this, (in_octet*)addr);

	assert(in_addressIsIPv4Compatible(_this));
}


/** \brief init posix IPv6 address */
void
in_addressInitFromIn6Addr(in_address _this, struct in6_addr *addr)
{
	in_addressInitIPv6(_this, (in_octet*)addr);
}

/** \brief copy IP address to IPv4 socket address */
void
in_addressToInAddr(in_address _this, struct in_addr *addr)
{
	assert(in_addressIsUnspecified(_this) || in_addressIsIPv4Compatible(_this));

	memcpy(((in_octet*)(addr)),
			in_addressIPv4Ptr(_this),
			IN_IPV4_ADDRESS_SIZE);
}
/** \brief copy IP address to IPv6 socket address */
void
in_addressToIn6Addr(in_address _this, struct in6_addr *addr)
{
	memcpy(((in_octet*)(addr)),
			in_addressIPv6Ptr(_this),
			IN_IPV6_ADDRESS_SIZE);
}


in_addressType
in_addressGetType(in_address _this)
{
	in_addressType result = IN_ADDRESS_TYPE_UNICAST;
	if (in_addressIsUnspecified(_this)) {
		result = IN_ADDRESS_TYPE_UNKNOWN;
	} else if (in_addressIsIPv4Compatible(_this)) {
		/* is IPv4 address */

		/* We can not tell if IPv4 broadcast address, as this depends on
		 * the network mask being set, maybe we have got to introduce an attribute
		 * for the IPv4 netmask */
		in_addr_t *netAddr =  (in_addr_t*) in_addressIPv4Ptr(_this);
		os_uint hostAddr = ntohl(*netAddr);

		if (IN_CLASSD(hostAddr)) {
			result = IN_ADDRESS_TYPE_MULTICAST;
		} else if (hostAddr == INADDR_LOOPBACK) {
			result = IN_ADDRESS_TYPE_LOOPBACK;
		}
	}
	else {
		const struct in6_addr *ipv6 = (struct in6_addr*)_this;
		/* \TODO check for  "Anycast" */
		if (IN6_IS_ADDR_MULTICAST(ipv6)) {
			result = IN_ADDRESS_TYPE_MULTICAST;
		} else if (IN6_IS_ADDR_LOOPBACK(ipv6)) {
			result = IN_ADDRESS_TYPE_LOOPBACK;
		}
	}

	return result;
}

os_equality
in_addressCompare(in_address _this, in_address _other)
{
	os_equality result = OS_NE;

	int compResult =
		memcmp(in_addressPtr(_this),
			   in_addressPtr(_other),
			   IN_IP_ADDRESS_SIZE);

	switch (compResult) {
	case -1: result = OS_LT; break;
	case  0: result = OS_EQ; break;
	case  1: result = OS_GT; break;
	default:
		assert(!"never reach this");
	}

	return result;
}

static os_boolean
in_addressMatchesIPv4(in_address this, struct sockaddr_in *sockAddr)
{
	os_boolean result;
	struct in_addr* thisAsInAddr =
		/* IPv4 address is stored in vector in network byte order */
		(struct in_addr*) in_addressIPv4Ptr(this);

	assert(sockAddr->sin_family == AF_INET);

	result = in_addressIsIPv4Compatible(this) &&
		(memcmp(&sockAddr->sin_addr, thisAsInAddr, sizeof(struct in_addr))==0);

	return result;
}

static os_boolean
in_addressMatchesIPv6(in_address this, struct sockaddr_in6 *sockAddr)
{
	os_boolean result;
	struct in6_addr* thisAsIn6Addr =
		/* cast the vector to in6_addr */
		(struct in6_addr*) in_addressIPv6Ptr(this);

	assert(sockAddr->sin6_family == AF_INET6);

	result =
		(memcmp(&sockAddr->sin6_addr, thisAsIn6Addr, sizeof(struct in6_addr)) == 0);

	return result;
}


os_boolean
in_addressMatches(in_address this, struct sockaddr *sockAddr)
{
	os_boolean result = FALSE;

	switch (sockAddr->sa_family) {
	case AF_INET:
		result =
			in_addressMatchesIPv4(this,
					((struct sockaddr_in*)sockAddr));
		break;
	case AF_INET6:
		result =
			in_addressMatchesIPv6(this,
					((struct sockaddr_in6*)sockAddr));
				break;
	default:
		/* not supported yet */
		result = FALSE;
	}

	return result;
}

int /* Note: platform specific 'int' */
in_addressGetFamilyFromString(const os_char *addressString)
{
	int result;
	if (strchr(addressString, ':')!=NULL) {
		/* if address string contains ':' it must be ipv6 */
		result = AF_INET6;
	} else {
		/* either the string 'broadcast' or dotted IP address */
		result = AF_INET;
	}
	return result;
}

/** \brief compare operation, returns TRUE if equal, otherwise FALSE */
os_boolean
in_addressEqual(const in_address _this, const in_address other)
{
	/** TODO common operation, declare as macro for speedup, maybe
	 * use platform specific IPv6 comparison macros */
	os_boolean result;
	result =
		memcmp(_this, other, sizeof(*_this)) == 0;
	return result;
}

os_char*
in_addressToString(const in_address _this, os_char *buffer, os_uint buflen)
{
#if 0
	/* This code has been disabled due to win32 portability issues. IPv4/v6
	 * conversion shall be provided by abstraction layer in future. Until then the
	 * protable workarround below shall be used. */

	if (in_addressIsIPv4Compatible(_this)) {
		/* \TODO check return value for error */
		inet_ntop(AF_INET, in_addressIPv4Ptr(_this), buffer, buflen);
	} else {
		/* \TODO check return value for error */
		inet_ntop(AF_INET6, in_addressIPv6Ptr(_this), buffer, buflen);
	}
#else
	os_char *result = NULL; /* return value on error */

	if (in_addressIsIPv4Compatible(_this)) {
		if (buflen >= ((3*4) + 3 + 1)) {
			sprintf(buffer,
					"%d.%d.%d.%d",
					_this->octets[12],
					_this->octets[13],
					_this->octets[14],
					_this->octets[15]);
			result = buffer;
		}
	} else {
		/* simple but valid IPv6 address to string conversion */
		if (buflen >= ((16*2) + 7 + 1)) {
			sprintf(buffer,
					"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
					_this->octets[0],
					_this->octets[1],
					_this->octets[2],
					_this->octets[3],
					_this->octets[4],
					_this->octets[5],
					_this->octets[6],
					_this->octets[7],
					_this->octets[8],
					_this->octets[9],
					_this->octets[10],
					_this->octets[11],
					_this->octets[12],
					_this->octets[13],
					_this->octets[14],
					_this->octets[15]);
			result = buffer;
		}
	}
#endif

	return buffer;
}


void
in_addressInitAny(in_address _this)
{
	memset(in_addressPtr(_this), 0, IN_IPV6_ADDRESS_SIZE);
}

/** \brief parse 16 octets from CDR buffer, return 0 in case of error */
in_long
in_addressInitFromBuffer(in_address _this, in_ddsiDeserializer deserializer)
{
	return in_ddsiDeserializerParseOctets(deserializer,
			in_addressPtr(_this),
			IN_IPV6_ADDRESS_SIZE);
}

/** init the IPv4 padding octets */
void
in_addressInitIPv4Padding(in_address _this)
{
	memset(in_addressPtr(_this), 0, IN_IPV4_OFFSET);
}

/** init the IPv4 octets */
void
in_addressInitIPv4Addr(in_address _this, const in_octet *fourOctets)
{
	memcpy(in_addressIPv4Ptr(_this), fourOctets, IN_IPV4_ADDRESS_SIZE);
}

/** \brief init IPv4 address, concatinating
 *  two operations with comma-operator */
void
in_addressInitIPv4(in_address _this, const in_octet *octets)
{
	in_addressInitIPv4Padding(_this);
	in_addressInitIPv4Addr(_this, octets);
}

/** \brief init  IPv6 address */
void
in_addressInitIPv6(in_address _this, const in_octet *octets)
{
	/* in_octet[16] _a16; */
	memcpy(in_addressIPv6Ptr(_this), octets, IN_IPV6_ADDRESS_SIZE);
}

/** \brief copy IP address from _other to _this */
void
in_addressCopy(in_address _this, const in_address from)
{
	memcpy(in_addressPtr(_this), in_addressPtr(from), sizeof(*(_this)));
}

/** \brief verify the address is IPv4 compatible */
os_boolean
in_addressIsIPv4Compatible(const in_address _this)
{
	os_boolean result;
	struct in6_addr *asIn6Addr =
		(struct in6_addr*) _this;
	assert(_this);

	result = IN6_IS_ADDR_V4COMPAT(asIn6Addr);

	return result;
}

/** \brief verify if all octets 0 */
os_boolean
in_addressIsUnspecified(const in_address _this)
{
	os_boolean result;
	struct in6_addr *asIn6Addr =
		(struct in6_addr*) _this;
	assert(_this);

	result = IN6_IS_ADDR_UNSPECIFIED(asIn6Addr);

	return result;
}
