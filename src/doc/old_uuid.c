#define _GNU_SOURCE 1 // for asprintf
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vitalnix/compiler.h>

static int get_date_parts(const char *, unsigned int *, unsigned int *,
    unsigned int *);

//----------------------------------------------------------------------------
/*
    Scrambling in Eike Teiwes's 1997 scripts (posted in C here):
*/
char *tw0_encode(int day, int month, int year) {
    char *ret;

    day   = 26 * day + 113; 
    month = 73 * month + 113; 
    year -= 1900; 
    year  = 29 * (year - 75) + 113; 

    asprintf(&ret, "%d%d%d", day, month, year);
    return ret;
}

void tw0_decode(const char *code, int *day, int *month, int *year) {
    char tmp[4];
    tmp[3] = '\0';

    *day = *month = *year = 0;

    memcpy(tmp, &code[0], 3); *day   = strtol(tmp, NULL, 10);
    memcpy(tmp, &code[3], 3); *month = strtol(tmp, NULL, 10);
    memcpy(tmp, &code[6], 3); *year  = strtol(tmp, NULL, 10);

    *day   = (*day - 113) / 26;
    *month = (*month - 113) / 73;
    *year  = (*year - 113) / 29 + 75 + 1900;
    return;
}

/*
    DAXTRAQ (pre-Vitalnix) encoding
*/
char *dt4_encode(const char *date) {
    unsigned int year, month, day;
    char *s;

    if(!get_date_parts(date, &year, &month, &day))
        return NULL;
    asprintf(&s, "%04X%02X%02X", year, month, day);
    return s;
}

char *dt3_encode(const char *date) {
    unsigned int year, month, day;
    struct tm tm = {};
    char *s;

    if(!get_date_parts(date, &year, &month, &day))
        return NULL;

    tm.tm_year = year - 1900;
    tm.tm_mon  = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 12;
    asprintf(&s, "%08lX", static_cast(long, mktime(&tm)));
    return s;
}

/*
    VITALNIX 1.x/2.x ENCODING
*/
char *v0_encode(const char *date) {
    unsigned int year, month, day;
    int ret = 0;
    char *s;

    if(strchr(date, '-') != NULL)
        ret = sscanf(date, "%u-%u-%u", &year, &month, &day);
    else if(strchr(date, '.') != NULL)
        ret = sscanf(date, "%u.%u.%u", &day, &month, &year);
    else if(strchr(date, '/') != NULL)
        ret = sscanf(date, "%u/%u/%u", &month, &day, &year);
    if(ret != 3) {
        errno = EINVAL;
        return NULL;
    }

    asprintf(&s, "0_%03X%01X%02X", year, month, day);
    return s;
}

//-----------------------------------------------------------------------------
static int get_date_parts(const char *date, unsigned int *year,
  unsigned int *month, unsigned int *day)
{
    int ret = 0;
    if(strchr(date, '-') != NULL)
        ret = sscanf(date, "%u-%u-%u", year, month, day);
    else if(strchr(date, '.') != NULL)
        ret = sscanf(date, "%u.%u.%u", day, month, year);
    else if(strchr(date, '/') != NULL)
        ret = sscanf(date, "%u/%u/%u", month, day, year);
    if(ret != 3) {
        errno = EINVAL;
        return 0;
    }
    return 1;
}

//=============================================================================
