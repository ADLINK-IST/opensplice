#ifndef NN_UNUSED_H
#define NN_UNUSED_H

#ifdef __GNUC__
#define UNUSED_ARG(x) x __attribute__ ((unused))
#else
#define UNUSED_ARG(x) x
#endif

#ifndef NDEBUG
#define UNUSED_ARG_NDEBUG(x) x
#else
#define UNUSED_ARG_NDEBUG(x) UNUSED_ARG (x)
#endif

#endif /* NN_UNUSED_H */

/* SHA1 not available (unoffical build.) */
