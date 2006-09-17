#ifndef _VITALNIX_GENERIC_COMPILER_H
#define _VITALNIX_GENERIC_COMPILER_H 1

#define CONSTRUCTOR     __attribute__((constructor))
#define DESTRUCTOR      __attribute__((destructor))

#ifdef HAVE_VISIBILITY
#    define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#    define EXPORT_SYMBOL
#endif

// Code annotations
#define const_cast(type, expr)          ((type)(expr))
#define signed_cast(type, expr)         ((type)(expr))
#define static_cast(type, expr)         ((type)(expr))
#define reinterpret_cast(type, expr)    ((type)(expr))

#endif // _VITALNIX_GENERIC_COMPILER_H

//=============================================================================
