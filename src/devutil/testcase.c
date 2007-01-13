/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006 - 2007
    This code is released under version 2.1 of the GNU LGPL.

    A small testcase for some functions.
*/
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

//-----------------------------------------------------------------------------
static void test_1(void) {
    printf("--- TEST 1 --- XDAY conversion\n");
    printf("2006-03-04 = %lx (should be 7d6304)\n",
           vxutil_string_xday("2006-03-04"));
    return;
}

static void test_2(void) {
    char *r;
    vxutil_cryptpw("", NULL, CRYPW_BLOWFISH, &r);
    printf(
        "--- TEST 2 --- Blowfish encryption\n"
        "Output: %s\n", r);
    free(r);
    return;
}

static void test_3(const char *f) {
    struct vxeds_entry entry;
    void *handle;
    int ret, c = 0;

    printf("--- TEST 3: EDS reading\n");
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

    printf("--- TEST 4: Quoting\n");

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

    free(fm);
    return;
}

static void test_5(void) {
#define CHK(s, f) \
    vxutil_propose_lname(lname, sizeof(lname), (s), (f)); \
    printf(f " " s " = %s\n", lname);

    char lname[16];
    printf("--- TEST 5: Login name creation\n");

    CHK("van der Foobar", "Ölte");
    CHK("van Yksi", "Ölte");
    CHK("van Öksi", "Ären");
    CHK("Üxi", "Iren");
    CHK("Üxi", "Omur");
    CHK("Ji-Domo Den", "Ojiisan");
    CHK("Ji Go Ku", "Shoujo");
    CHK("Ji Go-Ku", "Ojiisan");
    CHK("Ji-Go Ku", "ōtōsan");
    CHK("Ji go Ku", "@erv");
    return;
#undef CHK
}

static void test_6(void)
{
    const char *table[] = {
        "",       "",
        "f",      "Zg==",
        "fo",     "Zm8=",
        "foo",    "Zm9v",
        "foob",   "Zm9vYg==",
        "fooba",  "Zm9vYmE=",
        "foobar", "Zm9vYmFy",
        "Turpakäräjiin", "VHVycGFrw6Ryw6RqaWlu",
    };
    char *s = NULL;
    int i;

    printf("--- TEST 6: BASE-64 encoding ---\n");
    printf("  Input \tOutput      \tExpected\n");
    for(i = 0; i < ARRAY_SIZE(table); i += 2)
        printf("> %-6s\t%-12s\t%s\n", table[i],
            vxutil_quote(table[i], VXQUOTE_BASE64, &s), table[i+1]);

    free(s);
    return;
}

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    test_1();
    test_2();
    test_3("daten.sdf");
    test_3("daten.xml");
    test_4();
    test_5();
    test_6();
    return 0;
}

//=============================================================================
