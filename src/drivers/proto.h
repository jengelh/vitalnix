#ifndef _VITALNIX_DRIVERS_PROTO_H
#define _VITALNIX_DRIVERS_PROTO_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_CB_ALL(name) \
    DRIVER_CB_BASE(name), \
    DRIVER_CB_USER(name), \
    DRIVER_CB_GROUP(name)

#ifndef __cplusplus
#    define DRIVER_CB_BASE1(name) \
        .init           = name##_init, \
        .open           = name##_open, \
        .close          = name##_close, \
        .exit           = name##_exit
#    define DRIVER_CB_BASE(name) \
        DRIVER_CB_BASE1(name), \
        .modctl         = name##_modctl, \
        .lock           = name##_lock, \
        .unlock         = name##_unlock
#    define DRIVER_CB_USER(name) \
        .useradd        = name##_useradd, \
        .usermod        = name##_usermod, \
        .userdel        = name##_userdel, \
        .userinfo       = name##_userinfo, \
        .usertrav_init  = name##_usertrav_init, \
        .usertrav_walk  = name##_usertrav_walk, \
        .usertrav_free  = name##_usertrav_free
#    define DRIVER_CB_GROUP(name) \
        .groupadd       = name##_groupadd, \
        .groupmod       = name##_groupmod, \
        .groupdel       = name##_groupdel, \
        .groupinfo      = name##_groupinfo, \
        .grouptrav_init = name##_grouptrav_init, \
        .grouptrav_walk = name##_grouptrav_walk, \
        .grouptrav_free = name##_grouptrav_free
#endif // __cplusplus

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_DRIVERS_PROTO_H

//=============================================================================
