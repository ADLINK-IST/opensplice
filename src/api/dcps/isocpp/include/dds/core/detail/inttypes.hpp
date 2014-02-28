/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_CORE_DETAIL_INTTYPES_HPP_
#define OSPL_DDS_CORE_DETAIL_INTTYPES_HPP_

/**
 * @file
 */

// Implementation

/* (from spec:) This implementation-defined header stands in for the C99 header files
 * inttypes.h. Under toolchains that support inttypes.h, this header can
 * simply include that one. Under toolchains that do not, this header must
 * provide equivalent definitions.
 */
#if defined _MSC_VER && (_MSC_VER <= 1500)
// VS 2005 & 2008 confirmed to have no stdint.h; predecessors presumed likewise
#include <dds/core/detail/old_win_stdint.h>
#else
#include <stdint.h>
#endif

// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_INTTYPES_HPP_ */
