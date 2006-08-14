#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "libvxeds/libvxeds.h"
#include "libvxutil/libvxutil.h"

//-----------------------------------------------------------------------------
static void test_1(void) {
    printf("--- TEST 1 ---\n");
    printf("2006-03-04 in days since 1970: %ld (should be 13210)\n",
        vxutil_string_iday("2006-03-04"));
    return;
}

static void test_2(void) {
    char *r;
    vxutil_cryptpw("", NULL, CRYPW_BLOWFISH, &r);
    printf("--- TEST 2 ---\n" "%s\n", r);
    free(r);
    return;
}

static void test_3(const char *f) {
    struct vxeds_entry entry;
    void *handle;
    int ret, c = 0;

    printf("--- TEST 3 ---\n");
    if((ret = vxeds_open(f, NULL, &handle)) <= 0) {
        printf("eds_open: %s\n", strerror(-ret));
        return;
    }
    while((ret = vxeds_read(handle, &entry)) > 0) {
        printf("\r%d", ++c);
        vxeds_free_entry(&entry);
    }
    printf(" entries, code %d (%s)\n", ret, strerror(-ret));
    vxeds_close(handle);
    return;
}

static void test_4(void) {
    char *fm = NULL, *ret;
    const char *s;

    printf("--- TEST 4 ---\n");

    s = "Good ''keg o' ol' \\\"whisky\\\"";
    ret = vxutil_quote(s, 0, &fm);
    printf("Unquoted:\t%s (%p)\n" "Quoted:\t\t'%s' (%p)\n", s, s, ret, ret);

    s = "'Good keg'";
    ret = vxutil_quote(s, 0, &fm);
    printf("Unquoted:\t%s (%p)\n" "Quoted:\t\t'%s' (%p)\n", s, s, ret, ret);

    s = "Good ''keg o' ol' \\\"whisky\\\"";
    ret = vxutil_quote(s, 1, &fm);
    printf("Unquoted:\t%s (%p)\n" "Quoted:\t\t\"%s\" (%p)\n", s, s, ret, ret);

    s = "noquote";
    ret = vxutil_quote(s, 1, &fm);
    printf("Unquoted:\t%s (%p)\n" "Quoted:\t\t\"%s\" (%p)\n", s, s, ret, ret);
    printf("fm is %p (should be NULL)\n", fm);

    return;
}

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    test_1();
    test_2();
    test_3("daten3.sdf");
    test_3("daten.xml");
    test_4();
    return 0;
}

//=============================================================================
