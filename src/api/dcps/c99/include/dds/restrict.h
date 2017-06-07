#ifndef DDS_RESTRICT_H
#define DDS_RESTRICT_H

#if defined(__cplusplus)
    #define restrict
#elif !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901)
    #define restrict
#endif

#endif
