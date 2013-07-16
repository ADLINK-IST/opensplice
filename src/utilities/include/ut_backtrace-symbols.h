#ifndef UT_BACKTRACE_SYMBOLS_H_
#define UT_BACKTRACE_SYMBOLS_H_

#if defined (__cplusplus)
extern "C" {
#endif

char **backtrace_symbols_extension(void *const *buffer, int size);

void
backtrace_symbols_fd_extension(void *const *buffer, int size, int fd);

#if defined (__cplusplus)
}
#endif

#endif /* UT_BACKTRACE_SYMBOLS_H_ */
