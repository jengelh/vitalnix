#ifndef _VITALNIX_GENERIC_COMPILER_H
#define _VITALNIX_GENERIC_COMPILER_H 1

#include <vitalnix/config.h>

#define CONSTRUCTOR     __attribute__((constructor))
#define DESTRUCTOR      __attribute__((destructor))

#ifdef HAVE_VISIBILITY_HIDDEN
#	define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#	define EXPORT_SYMBOL
#endif

#endif /* _VITALNIX_GENERIC_COMPILER_H */
