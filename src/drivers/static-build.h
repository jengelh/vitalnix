#ifndef _VITALNIX_DRIVERS_STATICBUILD_H
#define _VITALNIX_DRIVERS_STATICBUILD_H 1

#include <vitalnix/compiler.h>
#include <vitalnix/libvxcore/loader.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REGISTER_MODULE(name, ptr) \
    static CONSTRUCTOR void register_libvxpdb_##name(void) { \
        vxcore_module_register("libvxpdb", #name, (ptr)); \
        vxpdb_fix_vtable(ptr); \
        return; \
    } \
    static DESTRUCTOR void unregister_libvxpdb_##name(void) { \
        vxcore_module_unregister("libvxpdb", #name); \
        return; \
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_DRIVERS_STATICBUILD_H

//=============================================================================
