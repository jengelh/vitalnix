#ifndef _VITALNIX_GENERIC_COMPILER_H
#define _VITALNIX_GENERIC_COMPILER_H 1

#define CONSTRUCTOR     __attribute__((constructor))
#define DESTRUCTOR      __attribute__((destructor))

#if defined(__GNUC__) && __GNUC__ >= 4
#    define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

// Code annotations
#define const_cast(type, expr)          ((type)(expr))
#define signed_cast(type, expr)         ((type)(expr))
#define static_cast(type, expr)         ((type)(expr))
#define reinterpret_cast(type, expr)    ((type)(expr))

#endif // _VITALNIX_GENERIC_COMPILER_H

//=============================================================================
