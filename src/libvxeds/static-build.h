#ifndef _VITALNIX_LIBVXEDS_STATICBUILD_H
#define _VITALNIX_LIBVXEDS_STATICBUILD_H 1

#include <vitalnix/compiler.h>
#include <vitalnix/libvxcore/loader.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REGISTER_MODULE(name, ptr) \
    static CONSTRUCTOR void register_libvxeds_##name(void) { \
        vxcore_module_register("libvxeds", #name, (ptr)); \
        return; \
    } \
    static DESTRUCTOR void unregister_libvxeds_##name(void) { \
        vxcore_module_unregister("libvxeds", #name); \
        return; \
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXEDS_STATICBUILD_H

//=============================================================================
