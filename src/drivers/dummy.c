#include "drivers/static-build.h"
#include <vitalnix/libvxpdb/libvxpdb.h>

// Variables
static struct vxpdb_driver THIS_MODULE = {
    .name = "Dummy module",
    .desc = "That's All, Folks",
};

REGISTER_MODULE(dummy, &THIS_MODULE);

//eof
