#ifndef _VITALNIX_LIBVXMDFMT_STATICBUILD_H
#define _VITALNIX_LIBVXMDFMT_STATICBUILD_H 1

#include <vitalnix/compiler.h>
#include <vitalnix/libvxcore/loader.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REGISTER_MODULE(name, ptr) \
	static CONSTRUCTOR void register_libvxmdfmt_##name(void) \
	{ \
		vxcore_module_register("libvxmdfmt", #name, (ptr)); \
	} \
	static DESTRUCTOR void unregister_libvxmdfmt_##name(void) \
	{ \
		vxcore_module_unregister("libvxmdfmt", #name); \
	}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXMDFMT_STATICBUILD_H */
