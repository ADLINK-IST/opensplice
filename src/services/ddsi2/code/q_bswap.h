#ifndef NN_BSWAP_H
#define NN_BSWAP_H

#include "os_abstract.h"

#include "q_inline.h"
#include "q_rtps.h" /* for nn_guid_t, nn_guid_prefix_t */
#include "q_protocol.h" /* for nn_sequence_number_t */

#define bswap2(x) ((short) bswap2u ((unsigned short) (x)))
#define bswap4(x) ((int) bswap4u ((unsigned) (x)))
#define bswap8(x) ((long long) bswap8u ((unsigned long long) (x)))

#if NN_HAVE_C99_INLINE && !defined SUPPRESS_BSWAP_INLINES
#include "q_bswap_template.c"
#else
#if defined (__cplusplus)
extern "C" {
#endif
unsigned short bswap2u (unsigned short x);
unsigned bswap4u (unsigned x);
unsigned long long bswap8u (unsigned long long x);
void bswapSN (nn_sequence_number_t *sn);
#if defined (__cplusplus)
}
#endif
#endif

#ifdef PA_LITTLE_ENDIAN
#define toBE2(x) bswap2 (x)
#define toBE2u(x) bswap2u (x)
#define toBE4(x) bswap4 (x)
#define toBE4u(x) bswap4u (x)
#define toBE8(x) bswap8 (x)
#define toBE8u(x) bswap8u (x)
#define fromBE2(x) bswap2 (x)
#define fromBE2u(x) bswap2u (x)
#define fromBE4(x) bswap4 (x)
#define fromBE4u(x) bswap4u (x)
#define fromBE8(x) bswap8 (x)
#define fromBE8u(x) bswap8u (x)
#else
#define toBE2u(x) (x)
#define toBE4(x) (x)
#define toBE4u(x) (x)
#define toBE8(x) (x)
#define toBE8u(x) (x)
#define fromBE2(x) (x)
#define fromBE2u(x) (x)
#define fromBE4(x) (x)
#define fromBE4u(x) (x)
#define fromBE8(x) (x)
#define fromBE8u(x) (x)
#endif

#if defined (__cplusplus)
extern "C" {
#endif

nn_guid_prefix_t nn_hton_guid_prefix (nn_guid_prefix_t p);
nn_guid_prefix_t nn_ntoh_guid_prefix (nn_guid_prefix_t p);
nn_entityid_t nn_hton_entityid (nn_entityid_t e);
nn_entityid_t nn_ntoh_entityid (nn_entityid_t e);
nn_guid_t nn_hton_guid (nn_guid_t g);
nn_guid_t nn_ntoh_guid (nn_guid_t g);

void bswap_sequence_number_set_hdr (nn_sequence_number_set_t *snset);
void bswap_sequence_number_set_bitmap (nn_sequence_number_set_t *snset);
void bswap_fragment_number_set_hdr (nn_fragment_number_set_t *fnset);
void bswap_fragment_number_set_bitmap (nn_fragment_number_set_t *fnset);

#if defined (__cplusplus)
}
#endif

#endif /* NN_BSWAP_H */

/* SHA1 not available (unoffical build.) */
