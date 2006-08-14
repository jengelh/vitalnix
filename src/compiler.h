#ifndef _VITALNIX_GENERIC_COMPILER_H
#define _VITALNIX_GENERIC_COMPILER_H 1

#define CONSTRUCTOR     __attribute__((constructor))
#define DESTRUCTOR      __attribute__((destructor))

#if defined(__GNUC__) && __GNUC__ >= 4
#    define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

#endif // _VITALNIX_GENERIC_COMPILER_H

//=============================================================================
