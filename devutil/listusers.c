/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
  -- License restrictions apply (LGPL v2.1)
  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <stdio.h>
#include <accdb.h>

int main(void) {
    struct adb_module *db = adb_load("*", NULL);
    struct adb_user r;
    void *i = NULL;

    db->open(db, 0);
    db->usertrav(db, &i, NULL);
    while(db->usertrav(db, &i, &r)) {
        printf("%s ", r.lname);
    }

    printf("\n");
    return 0;
}

//=============================================================================
