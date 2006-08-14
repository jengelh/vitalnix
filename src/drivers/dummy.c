#include "drivers/static-build.h"
#include "libvxpdb/libvxpdb.h"

// Variables
static struct vxpdb_mvtable THIS_MODULE = {
    .name = "Dummy module",
    .desc = "That's All, Folks",
};

REGISTER_MODULE(dummy, &THIS_MODULE);

//eof
