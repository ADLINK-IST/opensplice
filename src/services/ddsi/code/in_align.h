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
#ifndef IN_ALIGN_H_
#define IN_ALIGN_H_

#include "in_commonTypes.h" /* P2UI, UI2P, UI */

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif


/** \brief round up/ceil ptr  to multiple of boundary */
#define IN_ALIGN_PTR_CEIL(_address,_boundary) \
    UI2P( (P2UI(_address) + (UI(_boundary)-1)) & (~(UI(_boundary)-1)) )

/** \brief floor ptr to next smaller multiple of boundary */
#define IN_ALIGN_PTR_FLOOR(_address,_boundary) \
    UI2P( P2UI(_address) & (~(UI(_boundary)-1)) )

/** \brief floor length to next smaller multiple of boundary */
#define IN_ALIGN_UINT_FLOOR(_val,_boundary) \
    (P2UI(_val) & (~(UI(_boundary)-1)))

/** \brief round up/ceil length to multiple of boundary */
#define IN_ALIGN_UINT_CEIL(_val,_boundary) \
	( (P2UI(_val) + (UI(_boundary)-1)) & (~(UI(_boundary)-1)) )

#define IN_ALIGN_UINT_PAD(_val,_boundary) \
	(IN_ALIGN_UINT_CEIL(_val,_boundary)-(_val))

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif


#endif /* IN_ALIGN_H_ */
