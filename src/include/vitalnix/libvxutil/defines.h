#ifndef _VITALNIX_LIBVXUTIL_DEFINES_H
#define _VITALNIX_LIBVXUTIL_DEFINES_H 1

#ifndef _WIN32
#	define stricmp strcasecmp
#	define strnicmp strncasecmp
#endif

#define __STRINGIFY_EXPAND(s)   #s
#define __STRINGIFY(s)          __STRINGIFY_EXPAND(s)
#define MAXSNLEN                256
#define MAXFNLEN                256
#define MAXLNLEN                1024

#endif /* _VITALNIX_LIBVXUTIL_DEFINES_H */
