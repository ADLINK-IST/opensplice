//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#include <stddef.h>
#include "q_murmurhash3.h"

static os_uint32 rotl32(os_uint32 x, int r)
{
  return (x << r) | (x >> (32 - r));
}
#define ROTL32(x,y) rotl32(x,y)

static os_uint32 getblock(const os_uint32 * p, os_saddress i)
{
  return p[i];
}

static os_uint32 fmix(os_uint32 h)
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

/* Really
     http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp,
   MurmurHash3_x86_32 (public domain) */
os_uint32 murmurhash3(const void *key, size_t len, os_uint32 seed)
{
  const unsigned char *data = (const unsigned char *) key;
  const os_saddress nblocks = (os_saddress) (len / 4);

  os_uint32 h1 = seed;

  const os_uint32 c1 = 0xcc9e2d51;
  const os_uint32 c2 = 0x1b873593;

  //----------
  // body

  const os_uint32 *blocks = (const os_uint32 *) (data + nblocks * 4);
  const unsigned char *tail;
  os_saddress i;
  os_uint32 k1;

  for(i = -nblocks; i; i++)
  {
    os_uint32 k1 = getblock(blocks,i);

    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;

    h1 ^= k1;
    h1 = ROTL32(h1,13);
    h1 = h1 * 5+0xe6546b64;
  }

  //----------
  // tail

  tail = (const unsigned char *) (data + nblocks * 4);

  k1 = 0;

  switch (len & 3)
  {
    case 3: k1 ^= (os_uint32)tail[2] << 16;
    case 2: k1 ^= (os_uint32)tail[1] << 8;
    case 1: k1 ^= (os_uint32)tail[0]; k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization

  h1 ^= len;
  
  h1 = fmix(h1);
  
  return h1;
}

/* SHA1 not available (unoffical build.) */
