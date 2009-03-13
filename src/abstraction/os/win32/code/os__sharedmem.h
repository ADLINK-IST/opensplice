
#ifndef OS_WIN32__SHAREDMEM_H
#define OS_WIN32__SHAREDMEM_H

#if defined (__cplusplus)
extern "C" {
#endif

/** \brief Initialize shared memory module
 */
void
os_sharedMemoryInit (
    void);

/** \brief Deinitialize shared memory module
 */
void
os_sharedMemoryExit (
    void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32__SHAREDMEM_H */
