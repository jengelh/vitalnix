/*
 *	libvxutil/util.c - General functions
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libHX/arbtree.h>
#include <libHX/defs.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>
#define sizeof_z(s) (sizeof(s) - 1)

/* Functions */
static int vxutil_parse_date(const char *, unsigned int *, unsigned int *,
	unsigned int *);
static void vxutil_quote_base64(const char *, char *);
static size_t quoted_size(const char *, unsigned int);
static const char *surname_pointer(const char *);
static char *transform7(const char *, char *, size_t);

/* Variables */
static const char *const quote_match[] = {
	[VXQUOTE_SINGLE] = "'\\",
	[VXQUOTE_DOUBLE] = "\"\\",
	[VXQUOTE_XML]    = "\"&<>",
};

//-----------------------------------------------------------------------------
EXPORT_SYMBOL unsigned int vxutil_now_iday(void)
{
	return time(NULL) / 86400;
}

/**
 * vxutil_only_digits -
 * @p:	string to look at
 *
 * Returns whether the string in @p consists only of digits.
 */
EXPORT_SYMBOL bool vxutil_only_digits(const char *p)
{
	while (*p != '\0')
		if (!isdigit(*p++))
			return false;
	return true;
}

EXPORT_SYMBOL char *vxutil_propose_home(char *dest, size_t size,
    const char *base, const char *username, unsigned int level)
{
	if (*username == '\0')
		fprintf(stderr, "%s: username has zero length\n", __func__);
	if (level == 0)
		snprintf(dest, size, "%s/%s", base, username);
	else if (level == 1 || level == 10)
		snprintf(dest, size, "%s/%c/%s", base, *username, username);
	else if (level == 15)
		snprintf(dest, size, "%s/%c/%c/%s", base, username[0],
		         (username[1] == '\0') ? '_' : username[1], username);
	else
		snprintf(dest, size, "%s/%c/%c%c/%s", base, *username,
		         username[0], (username[1] == '\0') ? '_' : username[1],
		         username);

	return dest;
}

/**
 * vxutil_propose_lname -
 * @surname_8:		The surname (family name), UTF-8 encoded
 * @firstname_8:	The first name, UTF-8 encoded
 */
EXPORT_SYMBOL char *vxutil_propose_lname(char *dest, size_t size,
    const char *surname_8, const char *firstname_8)
{
	char surname_7[7], firstname_7[8];

	transform7(firstname_8, firstname_7, sizeof(firstname_7));

	if (surname_8 != NULL) {
		transform7(surname_pointer(surname_8), surname_7, sizeof(surname_7));
		snprintf(dest, size, "%c%s", *firstname_7, surname_7);
	} else {
		snprintf(dest, size, "%s", firstname_7);
	}

	if (strlen(dest) < 2 && size >= 3)
		/*
		 * In case the login name is too short, append a "0" (just
		 * cosmetics), it does not affect anything.
		 */
		strcat(dest, "0");

	return HX_strlower(dest);
}

EXPORT_SYMBOL char *vxutil_quote(const char *src, unsigned int type,
    char **free_me)
{
	char *ret, *ret_wp;

	if (type >= _VXQUOTE_MAX ||
	 (type <= VXQUOTE_XML && strpbrk(src, quote_match[type]) == NULL))
		return const_cast1(char *, src);

	/* Allocation and deallocation saving */
	ret = realloc(*free_me, quoted_size(src, type) + 1);
	if (ret == NULL)
		return NULL;
	*free_me = ret_wp = ret;

	if (type == VXQUOTE_BASE64) {
		vxutil_quote_base64(src, ret);
		return ret;
	}

	if (type == VXQUOTE_XML)
		while (*src != '\0') {
			size_t seg_len = strcspn(src, quote_match[type]);
			if (seg_len > 0) {
				memcpy(ret_wp, src, seg_len);
				ret_wp += seg_len;
				src    += seg_len;
				if (*src == '\0')
					break;
			}
			switch (*src++) {
				case '"':
					memcpy(ret_wp, "&quot;", 6);
					ret_wp += 6;
					break;
				case '&':
					memcpy(ret_wp, "&amp;", 5);
					ret_wp += 5;
					break;
				case '<':
					memcpy(ret_wp, "&lt;", 4);
					ret_wp += 4;
					break;
				case '>':
					memcpy(ret_wp, "&gt;", 4);
					ret_wp += 4;
					break;
			}
			*ret_wp = '\0';
			return ret;
		}

	while (*src != '\0') {
		size_t seg_len = strcspn(src, quote_match[type]);
		if (seg_len > 0) {
			memcpy(ret_wp, src, seg_len);
			ret_wp += seg_len;
			src    += seg_len;
			if (*src == '\0')
				break;
		}
		*ret_wp++ = '\\';
		*ret_wp++ = *src++;
	}

	*ret_wp = '\0';
	return ret;
}

EXPORT_SYMBOL int vxutil_replace_run(const char *fmt,
    const struct HXbtree *catalog)
{
	hxmc_t *cmd = NULL;
	int ret;
	HXformat_aprintf(catalog, &cmd, fmt);
	ret = system(cmd);
	HXmc_free(cmd);
	return ret;
}

EXPORT_SYMBOL char *vxutil_slurp_file(const char *fn)
{
	struct stat sb;
	char *dst;
	FILE *fp;

	if ((fp = fopen(fn, "rb")) == NULL)
		return NULL;
	if (fstat(fileno(fp), &sb) != 0)
		return NULL;
	if ((dst = calloc(1, sb.st_size + 1)) == NULL)
		return NULL;
	if (fread(dst, sb.st_size, 1, fp) < 1) {
		free(dst);
		return NULL;
	}

	fclose(fp);
	return dst;
}

EXPORT_SYMBOL int vxutil_string_iday(const char *s)
{
	unsigned int day = 0, month = 0, year = 0;
	struct tm td;
	int ret = 0;
	time_t sec;

	if ((ret = vxutil_parse_date(s, &day, &month, &year)) != 3)
		return ret;

	td.tm_mday = day;
	td.tm_mon  = month - 1;
	td.tm_year = year - 1900;
	if ((sec = mktime(&td)) == -1)
		return -1;

	return sec / 86400;
}

EXPORT_SYMBOL int vxutil_string_xday(const char *s)
{
	unsigned int day = 0, month = 0, year = 0;
	int ret = 0;
	if ((ret = vxutil_parse_date(s, &day, &month, &year)) <= 0)
		return ret;
	return ((year & 0xFFF) << 12) | ((month & 0xF) << 8) | (day & 0xFF);
}

EXPORT_SYMBOL bool vxutil_valid_username(const char *n)
{
	bool valid;

	if (*n == '\0')
		return false;
	if (!((*n >= 'A' && *n <= 'Z') || (*n >= 'a' && *n <= 'z') ||
	    *n == '_'))
		return false;

	while (*n != '\0') {
		if (*n == '$' && *(n+1) == '\0')
			/* Samba accounts */
			return true;

		valid = (*n >= 'A' && *n <= 'Z') || (*n >= 'a' && *n <= 'z') ||
		        (*n >= '0' && *n <= '9') || *n == '_' || *n == '.' ||
		        *n == '-';
		if (!valid)
			return false;
		++n;
	}

	return true;
}

//-----------------------------------------------------------------------------
static int vxutil_parse_date(const char *s, unsigned int *day,
    unsigned int *month, unsigned int *year)
{
	int ret = 0;

	if (strchr(s, '-') != NULL)
		/*
		 * ISO-8601 style YYYY-MM-DD,
		 * http://www.cl.cam.ac.uk/~mgk25/iso-time.html
		 * http://en.wikipedia.org/wiki/ISO-8601
		 */
		ret = sscanf(s, "%u-%u-%u", year, month, day);
	else if (strchr(s, '.') != NULL)
		/* European style DD.MM.YYYY */
		ret = sscanf(s, "%u.%u.%u", day, month, year);
	else if (strchr(s, '/') != NULL)
		/* American style MM/DD/YYYY */
		ret = sscanf(s, "%u/%u/%u", month, day, year);
	if (ret != 3)
		return -1;

	/*
	 * If any of the three are zero, the input date is illegal. If all
	 * three are zero, the equivalent meaning is "no date given".
	 */
	if (*day == 0 && *month == 0 && *year == 0)
		return 0;
	if (*day == 0 || *month == 0 || *year == 0)
		return -1;

	if (*year >= 100 && *year < 1000) {
		*year += 1900;
	} else if (*year < 100) {
		/*
		 * Fix two-digit year numbers. The range of two-digit years is
		 * always [now-50yrs ... now+50yrs].
		 */
		time_t now_sec = time(NULL);
		struct tm now_tm;
		int bp, nc;

		localtime_r(&now_sec, &now_tm);
		bp = (now_tm.tm_year + 50) % 100;
		nc = now_tm.tm_year - now_tm.tm_year % 100 +
		     ((bp < 50) ? 100 : 0);
		*year += 1900 + ((*year > bp) ? (nc - 100) : nc);
	}

	return 1;
}

/**
 * vxutil_quote_base64 -
 * @s:	string to encode
 * @d:	destination buffer
 *
 * Encode @src into BASE-64 according to RFC 4648 and write result to @dest,
 * which must be of appropriate size, plus one for a trailing NUL.
 */
static void vxutil_quote_base64(const char *s, char *d)
{
	static const char *a =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t len = strlen(s);

	while (len > 0) {
		if (len >= 3) {
			len -= 3;
			d[0] = a[(s[0] & 0xFC) >> 2];
			d[1] = a[((s[0] & 0x03) << 4) | ((s[1] & 0xF0) >> 4)];
			d[2] = a[((s[1] & 0x0F) << 2) | ((s[2] & 0xC0) >> 6)];
			d[3] = a[s[2] & 0x3F];
		} else if (len == 2) {
			len = 0;
			d[0] = a[(s[0] & 0xFC) >> 2];
			d[1] = a[((s[0] & 0x03) << 4) | ((s[1] & 0xF0) >> 4)];
			d[2] = a[(s[1] & 0x0F) << 2];
			d[3] = '=';
		} else if (len == 1) {
			len = 0;
			d[0] = a[(s[0] & 0xFC) >> 2];
			d[1] = a[(s[0] & 0x03) << 4];
			d[2] = '=';
			d[3] = '=';
		}
		s += 3;
		d += 4;
	}
	*d = '\0';
}

/**
 * quoted_size -
 * @s:		string to analyze
 * @type:	non-zero if double quoted
 *
 * Returns the size of the string @s when quoted.
 */
static size_t quoted_size(const char *s, unsigned int type)
{
	const char *p = s;
	size_t n = strlen(s);

	/* The order of division and multiplication is _important_ here. */
	if (type == VXQUOTE_BASE64)
		return (strlen(s) + 2) / 3 * 4;

	if (type == VXQUOTE_XML) {
		while ((p = strpbrk(p, quote_match[type])) != NULL) {
			switch (*p) {
				/* minus 2: \0 and the original char */
				case '"':
					n += sizeof("&quot;") - 2;
					break;
				case '&':
					n += sizeof("&amp;") - 2;
					break;
				case '<':
				case '>':
					n += sizeof("&lt;") - 2;
					break;
			}
			++p;
		}
		return n;
	}

	while ((p = strpbrk(p, quote_match[type])) != NULL) {
		++n;
		++p;
	}

	return n;
}

/**
 * surname_pointer -
 * @s:	string to analyze
 *
 * Return a pointer to the first word that begins with an uppercase character.
 * If there is none that matches this criteria, return a pointer to the second
 * word, if there is any. If that also does not exist, return the original
 * pointer (i.e. to the first word).
 *
 * Note that this only works for standard ASCII A-Z, and not umlauts, etc.
 *
 * Examples:
 *     van der Waals   => Waals
 *     van der waals   => der waals
 *     waals           => waals
 */
static const char *surname_pointer(const char *s)
{
	const char *p;

	while (isspace(*s))
		++s;
	p = s;

	while (*p != '\0') {
		if (isupper(*p))
			return p;
		while (!isspace(*p) && *p != '\0')
			++p;
		while (isspace(*p))
			++p;
	}
	if ((p = strchr(s, ' ')) != NULL) {
		while (isspace(*p))
			++p;
		return p;
	}
	return s;
}

static const struct stab {
	const char *in, *out;
	int is, os;
} subst_tab[] = {
#define E(s, d) {(s), (d), sizeof(s) - 1, sizeof(d) - 1}
	/* [U+00B5] */ E("µ", "m"),
	/* [U+00C0] */ E("À", "A"),
	/* [U+00C1] */ E("Á", "A"),
	/* [U+00C2] */ E("Â", "A"),
	/* [U+00C3] */ E("Ã", "A"),
	/* [U+00C4] */ E("Ä", "AE"),
	/* [U+00C5] */ E("Å", "A"),
	/* [U+00C6] */ E("Æ", "AE"),
	/* [U+00C7] */ E("Ç", "C"),
	/* [U+00C8] */ E("È", "E"),
	/* [U+00C9] */ E("É", "E"),
	/* [U+00CA] */ E("Ê", "E"),
	/* [U+00CB] */ E("Ë", "E"),
	/* [U+00CC] */ E("Ì", "I"),
	/* [U+00CD] */ E("Í", "I"),
	/* [U+00CE] */ E("Î", "I"),
	/* [U+00CF] */ E("Ï", "I"),
	/* [U+00D0] */ E("Ð", "TH"),
	/* [U+00D1] */ E("Ñ", "N"),
	/* [U+00D2] */ E("Ò", "O"),
	/* [U+00D3] */ E("Ó", "O"),
	/* [U+00D4] */ E("Ô", "O"),
	/* [U+00D5] */ E("Õ", "O"),
	/* [U+00D6] */ E("Ö", "OE"),
	/* [U+00D8] */ E("Ø", "O"),
	/* [U+00D9] */ E("Ù", "U"),
	/* [U+00DA] */ E("Ú", "U"),
	/* [U+00DB] */ E("Û", "U"),
	/* [U+00DC] */ E("Ü", "UE"),
	/* [U+00DD] */ E("Ý", "Y"),
	/* [U+00DE] */ E("Þ", "TH"),
	/* [U+00DF] */ E("ß", "ss"),
	/* [U+00E0] */ E("à", "a"),
	/* [U+00E1] */ E("á", "a"),
	/* [U+00E2] */ E("â", "a"),
	/* [U+00E3] */ E("ã", "a"),
	/* [U+00E4] */ E("ä", "ae"),
	/* [U+00E5] */ E("å", "a"),
	/* [U+00E6] */ E("æ", "ae"),
	/* [U+00E7] */ E("ç", "c"),
	/* [U+00E8] */ E("è", "e"),
	/* [U+00E9] */ E("é", "e"),
	/* [U+00EA] */ E("ê", "e"),
	/* [U+00EB] */ E("ë", "e"),
	/* [U+00EC] */ E("ì", "i"),
	/* [U+00ED] */ E("í", "i"),
	/* [U+00EE] */ E("î", "i"),
	/* [U+00EF] */ E("ï", "i"),
	/* [U+00F0] */ E("ð", "th"),
	/* [U+00F1] */ E("ñ", "n"),
	/* [U+00F2] */ E("ò", "o"),
	/* [U+00F3] */ E("ó", "o"),
	/* [U+00F4] */ E("ô", "o"),
	/* [U+00F5] */ E("õ", "o"),
	/* [U+00F6] */ E("ö", "oe"),
	/* [U+00F8] */ E("ø", "o"),
	/* [U+00F9] */ E("ù", "u"),
	/* [U+00FA] */ E("ú", "u"),
	/* [U+00FB] */ E("û", "u"),
	/* [U+00FC] */ E("ü", "ue"),
	/* [U+00FD] */ E("ý", "y"),
	/* [U+00FE] */ E("þ", "th"),
	/* [U+00FF] */ E("ÿ", "y"),
	/* [U+0100] */ E("Ā", "A"),
	/* [U+0101] */ E("ā", "a"),
	/* [U+0102] */ E("Ă", "A"),
	/* [U+0103] */ E("ă", "a"),
	/* [U+0104] */ E("Ą", "A"),
	/* [U+0105] */ E("ą", "a"),
	/* [U+0106] */ E("Ć", "C"),
	/* [U+0107] */ E("ć", "c"),
	/* [U+0108] */ E("Ĉ", "C"),
	/* [U+0109] */ E("ĉ", "c"),
	/* [U+010A] */ E("Ċ", "C"),
	/* [U+010B] */ E("ċ", "c"),
	/* [U+010C] */ E("Č", "C"),
	/* [U+010D] */ E("č", "c"),
	/* [U+010E] */ E("Ď", "D"),
	/* [U+010F] */ E("ď", "d"),
	/* [U+0110] */ E("Đ", "D"),
	/* [U+0111] */ E("đ", "d"),
	/* [U+0112] */ E("Ē", "E"),
	/* [U+0113] */ E("ē", "e"),
	/* [U+0114] */ E("Ĕ", "E"),
	/* [U+0115] */ E("ĕ", "e"),
	/* [U+0116] */ E("Ė", "E"),
	/* [U+0117] */ E("ė", "e"),
	/* [U+0118] */ E("Ę", "E"),
	/* [U+0119] */ E("ę", "e"),
	/* [U+011A] */ E("Ě", "E"),
	/* [U+011B] */ E("ě", "e"),
	/* [U+011C] */ E("Ĝ", "G"),
	/* [U+011D] */ E("ĝ", "g"),
	/* [U+011E] */ E("Ğ", "G"),
	/* [U+011F] */ E("ğ", "g"),
	/* [U+0120] */ E("Ġ", "G"),
	/* [U+0121] */ E("ġ", "g"),
	/* [U+0122] */ E("Ģ", "G"),
	/* [U+0123] */ E("ģ", "g"),
	/* [U+0124] */ E("Ĥ", "H"),
	/* [U+0125] */ E("ĥ", "h"),
	/* [U+0126] */ E("Ħ", "H"),
	/* [U+0127] */ E("ħ", "h"),
	/* [U+0128] */ E("Ĩ", "I"),
	/* [U+0129] */ E("ĩ", "i"),
	/* [U+012A] */ E("Ī", "I"),
	/* [U+012B] */ E("ī", "i"),
	/* [U+012C] */ E("Ĭ", "I"),
	/* [U+012D] */ E("ĭ", "i"),
	/* [U+012E] */ E("Į", "I"),
	/* [U+012F] */ E("į", "i"),
	/* [U+0130] */ E("İ", "I"),
	/* [U+0131] */ E("ı", "i"),
	/* [U+0132] */ E("Ĳ", "IJ"),
	/* [U+0133] */ E("ĳ", "ij"),
	/* [U+0134] */ E("Ĵ", "J"),
	/* [U+0135] */ E("ĵ", "j"),
	/* [U+0136] */ E("Ķ", "K"),
	/* [U+0137] */ E("ķ", "k"),
	/* [U+0138] */ E("ĸ", "k"),
	/* [U+0139] */ E("Ĺ", "L"),
	/* [U+013A] */ E("ĺ", "l"),
	/* [U+013B] */ E("Ļ", "L"),
	/* [U+013C] */ E("ļ", "l"),
	/* [U+013D] */ E("Ľ", "L"),
	/* [U+013E] */ E("ľ", "l"),
	/* [U+013F] */ E("Ŀ", "L"),
	/* [U+0140] */ E("ŀ", "l"),
	/* [U+0141] */ E("Ł", "L"),
	/* [U+0142] */ E("ł", "l"),
	/* [U+0143] */ E("Ń", "N"),
	/* [U+0144] */ E("ń", "n"),
	/* [U+0145] */ E("Ņ", "N"),
	/* [U+0146] */ E("ņ", "n"),
	/* [U+0147] */ E("Ň", "N"),
	/* [U+0148] */ E("ň", "n"),
	/* [U+0149] */ E("ŉ", "n"),
	/* [U+014A] */ E("Ŋ", "NG"),
	/* [U+014B] */ E("ŋ", "ng"),
	/* [U+014C] */ E("Ō", "O"),
	/* [U+014D] */ E("ō", "o"),
	/* [U+014E] */ E("Ŏ", "O"),
	/* [U+014F] */ E("ŏ", "o"),
	/* [U+0150] */ E("Ő", "O"),
	/* [U+0151] */ E("ő", "o"),
	/* [U+0152] */ E("Œ", "OE"),
	/* [U+0153] */ E("œ", "oe"),
	/* [U+0154] */ E("Ŕ", "R"),
	/* [U+0155] */ E("ŕ", "r"),
	/* [U+0156] */ E("Ŗ", "R"),
	/* [U+0157] */ E("ŗ", "r"),
	/* [U+0158] */ E("Ř", "R"),
	/* [U+0159] */ E("ř", "r"),
	/* [U+015A] */ E("Ś", "S"),
	/* [U+015B] */ E("ś", "s"),
	/* [U+015C] */ E("Ŝ", "S"),
	/* [U+015D] */ E("ŝ", "s"),
	/* [U+015E] */ E("Ş", "S"),
	/* [U+015F] */ E("ş", "s"),
	/* [U+0160] */ E("Š", "S"),
	/* [U+0161] */ E("š", "s"),
	/* [U+0162] */ E("Ţ", "T"),
	/* [U+0163] */ E("ţ", "t"),
	/* [U+0164] */ E("Ť", "T"),
	/* [U+0165] */ E("ť", "t"),
	/* [U+0166] */ E("Ŧ", "T"),
	/* [U+0167] */ E("ŧ", "t"),
	/* [U+0168] */ E("Ũ", "U"),
	/* [U+0169] */ E("ũ", "u"),
	/* [U+016A] */ E("Ū", "U"),
	/* [U+016B] */ E("ū", "u"),
	/* [U+016C] */ E("Ŭ", "U"),
	/* [U+016D] */ E("ŭ", "u"),
	/* [U+016E] */ E("Ů", "U"),
	/* [U+016F] */ E("ů", "u"),
	/* [U+0170] */ E("Ű", "U"),
	/* [U+0171] */ E("ű", "u"),
	/* [U+0172] */ E("Ų", "U"),
	/* [U+0173] */ E("ų", "u"),
	/* [U+0174] */ E("Ŵ", "W"),
	/* [U+0175] */ E("ŵ", "w"),
	/* [U+0176] */ E("Ŷ", "Y"),
	/* [U+0177] */ E("ŷ", "y"),
	/* [U+0178] */ E("Ÿ", "Y"),
	/* [U+0179] */ E("Ź", "Z"),
	/* [U+017A] */ E("ź", "z"),
	/* [U+017B] */ E("Ż", "Z"),
	/* [U+017C] */ E("ż", "z"),
	/* [U+017D] */ E("Ž", "Z"),
	/* [U+017E] */ E("ž", "z"),
	/* [U+017F] */ E("ſ", "s"),
	/* [U+0180] */ E("ƀ", "b"),
	/* [U+0181] */ E("Ɓ", "B"),
	/* [U+0182] */ E("Ƃ", "B"),
	/* [U+0183] */ E("ƃ", "b"),
	/* [U+0184] */ E("Ƅ", "b"),
	/* [U+0185] */ E("ƅ", "b"),
	/* [U+0186] */ E("Ɔ", "C"),
	/* [U+0187] */ E("Ƈ", "C"),
	/* [U+0188] */ E("ƈ", "c"),
	/* [U+0189] */ E("Ɖ", "TH"),
	/* [U+018A] */ E("Ɗ", "D"),
	/* [U+018B] */ E("Ƌ", "D"),
	/* [U+018C] */ E("ƌ", "d"),
	/* [U+018D] */ E("ƍ", "d"),
	/* [U+018E] */ E("Ǝ", "E"),
	/* [U+0190] */ E("Ɛ", "E"),
	/* [U+0191] */ E("Ƒ", "F"),
	/* [U+0192] */ E("ƒ", "f"),
	/* [U+0193] */ E("Ɠ", "G"),
	/* [U+0194] */ E("Ɣ", "G"),
	/* [U+0195] */ E("ƕ", "hv"),
	/* [U+0196] */ E("Ɩ", "I"),
	/* [U+0197] */ E("Ɨ", "I"),
	/* [U+0198] */ E("Ƙ", "K"),
	/* [U+0199] */ E("ƙ", "k"),
	/* [U+019A] */ E("ƚ", "l"),
	/* [U+019B] */ E("ƛ", "l"),
	/* [U+019C] */ E("Ɯ", "m"),
	/* [U+019D] */ E("Ɲ", "N"),
	/* [U+019E] */ E("ƞ", "n"),
	/* [U+019F] */ E("Ɵ", "O"),
	/* [U+01A0] */ E("Ơ", "O"),
	/* [U+01A1] */ E("ơ", "o"),
	/* [U+01A2] */ E("Ƣ", "GHA"),
	/* [U+01A3] */ E("ƣ", "gha"),
	/* [U+01A4] */ E("Ƥ", "P"),
	/* [U+01A5] */ E("ƥ", "p"),
	/* [U+01A6] */ E("Ʀ", "yr"),
	/* [U+01A7] */ E("Ƨ", "S"),
	/* [U+01A8] */ E("ƨ", "s"),
	/* [U+01A9] */ E("Ʃ", "ESH"),
	/* [U+01AC] */ E("Ƭ", "T"),
	/* [U+01AD] */ E("ƭ", "t"),
	/* [U+01AE] */ E("Ʈ", "T"),
	/* [U+01AF] */ E("Ư", "U"),
	/* [U+01B0] */ E("ư", "u"),
	/* [U+01B1] */ E("Ʊ", "U"),
	/* [U+01B2] */ E("Ʋ", "V"),
	/* [U+01B3] */ E("Ƴ", "Y"),
	/* [U+01B4] */ E("ƴ", "y"),
	/* [U+01B5] */ E("Ƶ", "Z"),
	/* [U+01B6] */ E("ƶ", "z"),
	/* [U+01B7] */ E("Ʒ", "EZH"),
	/* [U+01B8] */ E("Ƹ", "EZH"),
	/* [U+01B9] */ E("ƹ", "ezh"),
	/* [U+01BA] */ E("ƺ", "ezh"),
	/* [U+01C4] */ E("Ǆ", "DZ"),
	/* [U+01C5] */ E("ǅ", "Dz"),
	/* [U+01C6] */ E("ǆ", "dz"),
	/* [U+01C7] */ E("Ǉ", "LJ"),
	/* [U+01C8] */ E("ǈ", "Lj"),
	/* [U+01C9] */ E("ǉ", "lj"),
	/* [U+01CA] */ E("Ǌ", "NJ"),
	/* [U+01CB] */ E("ǋ", "Nj"),
	/* [U+01CC] */ E("ǌ", "nj"),
	/* [U+01CD] */ E("Ǎ", "A"),
	/* [U+01CE] */ E("ǎ", "a"),
	/* [U+01CF] */ E("Ǐ", "I"),
	/* [U+01D0] */ E("ǐ", "i"),
	/* [U+01D1] */ E("Ǒ", "O"),
	/* [U+01D2] */ E("ǒ", "o"),
	/* [U+01D3] */ E("Ǔ", "U"),
	/* [U+01D4] */ E("ǔ", "u"),
	/* [U+01D5] */ E("Ǖ", "U"),
	/* [U+01D6] */ E("ǖ", "u"),
	/* [U+01D7] */ E("Ǘ", "U"),
	/* [U+01D8] */ E("ǘ", "u"),
	/* [U+01D9] */ E("Ǚ", "U"),
	/* [U+01DA] */ E("ǚ", "u"),
	/* [U+01DB] */ E("Ǜ", "U"),
	/* [U+01DC] */ E("ǜ", "u"),
	/* [U+01DD] */ E("ǝ", "e"),
	/* [U+01DE] */ E("Ǟ", "A"),
	/* [U+01DF] */ E("ǟ", "a"),
	/* [U+01E0] */ E("Ǡ", "A"),
	/* [U+01E1] */ E("ǡ", "a"),
	/* [U+01E2] */ E("Ǣ", "AE"),
	/* [U+01E3] */ E("ǣ", "ae"),
	/* [U+01E4] */ E("Ǥ", "G"),
	/* [U+01E5] */ E("ǥ", "g"),
	/* [U+01E6] */ E("Ǧ", "G"),
	/* [U+01E7] */ E("ǧ", "g"),
	/* [U+01E8] */ E("Ǩ", "K"),
	/* [U+01E9] */ E("ǩ", "k"),
	/* [U+01EA] */ E("Ǫ", "O"),
	/* [U+01EB] */ E("ǫ", "o"),
	/* [U+01EC] */ E("Ǭ", "O"),
	/* [U+01ED] */ E("ǭ", "o"),
	/* [U+01EE] */ E("Ǯ", "EZH"),
	/* [U+01EF] */ E("ǯ", "ezh"),
	/* [U+01F0] */ E("ǰ", "j"),
	/* [U+01F1] */ E("Ǳ", "DZ"),
	/* [U+01F2] */ E("ǲ", "Dz"),
	/* [U+01F3] */ E("ǳ", "dz"),
	/* [U+01F4] */ E("Ǵ", "G"),
	/* [U+01F5] */ E("ǵ", "g"),
	/* [U+01F6] */ E("Ƕ", "Hv"),
	/* [U+01F8] */ E("Ǹ", "N"),
	/* [U+01F9] */ E("ǹ", "n"),
	/* [U+01FA] */ E("Ǻ", "A"),
	/* [U+01FB] */ E("ǻ", "a"),
	/* [U+01FC] */ E("Ǽ", "AE"),
	/* [U+01FD] */ E("ǽ", "ae"),
	/* [U+01FE] */ E("Ǿ", "O"),
	/* [U+01FF] */ E("ǿ", "o"),
	/* [U+0200] */ E("Ȁ", "A"),
	/* [U+0201] */ E("ȁ", "a"),
	/* [U+0202] */ E("Ȃ", "A"),
	/* [U+0203] */ E("ȃ", "a"),
	/* [U+0204] */ E("Ȅ", "E"),
	/* [U+0205] */ E("ȅ", "e"),
	/* [U+0206] */ E("Ȇ", "E"),
	/* [U+0207] */ E("ȇ", "e"),
	/* [U+0208] */ E("Ȉ", "I"),
	/* [U+0209] */ E("ȉ", "i"),
	/* [U+020A] */ E("Ȋ", "I"),
	/* [U+020B] */ E("ȋ", "i"),
	/* [U+020C] */ E("Ȍ", "O"),
	/* [U+020D] */ E("ȍ", "o"),
	/* [U+020E] */ E("Ȏ", "O"),
	/* [U+020F] */ E("ȏ", "o"),
	/* [U+0210] */ E("Ȑ", "R"),
	/* [U+0211] */ E("ȑ", "r"),
	/* [U+0212] */ E("Ȓ", "R"),
	/* [U+0213] */ E("ȓ", "r"),
	/* [U+0214] */ E("Ȕ", "U"),
	/* [U+0215] */ E("ȕ", "u"),
	/* [U+0216] */ E("Ȗ", "U"),
	/* [U+0217] */ E("ȗ", "u"),
	/* [U+0218] */ E("Ș", "S"),
	/* [U+0219] */ E("ș", "s"),
	/* [U+021A] */ E("Ț", "T"),
	/* [U+021B] */ E("ț", "t"),
	/* [U+021E] */ E("Ȟ", "EZH"),
	/* [U+021F] */ E("ȟ", "ezh"),
	/* [U+0220] */ E("Ƞ", "H"),
	/* [U+0222] */ E("Ȣ", "N"),
	/* [U+0223] */ E("ȣ", "d"),
	/* [U+0226] */ E("Ȧ", "Z"),
	/* [U+0227] */ E("ȧ", "z"),
	/* [U+0228] */ E("Ȩ", "A"),
	/* [U+0229] */ E("ȩ", "a"),
	/* [U+022A] */ E("Ȫ", "E"),
	/* [U+022B] */ E("ȫ", "e"),
	/* [U+022A] */ E("Ȫ", "O"),
	/* [U+022B] */ E("ȫ", "o"),
	/* [U+022C] */ E("Ȭ", "O"),
	/* [U+022D] */ E("ȭ", "o"),
	/* [U+022E] */ E("Ȯ", "O"),
	/* [U+022F] */ E("ȯ", "o"),
	/* [U+0230] */ E("Ȱ", "O"),
	/* [U+0231] */ E("ȱ", "o"),
	/* [U+0232] */ E("Ȳ", "Y"),
	/* [U+0233] */ E("ȳ", "y"),
	{NULL},
#undef E
};

/**
 * transform7 -
 * @src:	Source string to transform
 * @dest:	Destination buffer
 * @dsize:	Size of destination buffer
 *
 * Transform an UTF-8 string @src into a 7-bit clean string according to the
 * substitution table and put the result into @dest, which is of size @dsize.
 * (So it puts in at most @dsize-1 characters.) The result will always be
 * '\0'-terminated.
 */
char *transform7(const char *src, char *dest, size_t dsize)
{
	char *od = dest;
	--dsize;
	while (dsize && *src != '\0') {
		/*
		 * Yes, no digit handling. This function is mainly used as part
		 * to produce login names, which better should not have
		 * numbers. (Since we may potentially be adding one, anyhow.)
		 */
		if ((*src >= 'A' && *src <= 'Z') || (*src >= 'a' && *src <= 'z')) {
			*dest++ = *src++;
			--dsize;
		} else if (*signed_cast(const unsigned char *, src) & 0x80) {
			const struct stab *sp = subst_tab;
			int ok = 0;
			for (sp = subst_tab; sp->in != NULL; ++sp) {
				if (strncmp(src, sp->in, sp->is) != 0)
					continue;
				ok   = 1;
				src += sp->is;
				if (dsize >= sp->os) {
					strncpy(dest, sp->out, sp->os);
					dest  += sp->os;
					dsize -= sp->os;
				} else {
					/*
					 * Do not let "abcdeöa" through,
					 * because if there was	too few space,
					 * the "ö" gets skipped but @dsize is
					 * still big enough to let the "a" in.
					 */
					dsize = 0;
				}
				break;
			}
			/* Just skip things we can not transform */
			if (!ok)
				++src;
		} else {
			++src;
		}
	}
	*dest = '\0';
	return od;
}
