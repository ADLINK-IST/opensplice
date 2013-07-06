#pragma once
#ifndef DLRL_TUTORIAL_IF_H
#define DLRL_TUTORIAL_IF_H

#include "os_if.h"

#ifdef DLRL_TUTORIAL_EXPORT
#define DLRLTUT_API OS_API_EXPORT
#else
#define DLRLTUT_API OS_API_IMPORT
#endif

#endif /* DLRLTUTORIAL_IF_H */
