#ifndef OSPL_PROC_H
#define OSPL_PROC_H

#include <os_stdlib.h>

void
kill_descendents (
    pid_t pid,
    int signal
    );

#endif /* OSPL_PROC_H */
