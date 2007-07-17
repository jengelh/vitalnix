/*
 *	libvxutil/util.c - General functions
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2003 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>
#define sizeof_z(s) (sizeof(s) - 1)

/* Functions */
static int vxutil_parse_date(const char *, int *, int *, int *);
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
EXPORT_SYMBOL long vxutil_now_iday(void)
{
	return time(NULL) / 86400;
}

EXPORT_SYMBOL int vxutil_only_digits(const char *p)
{
	/* Returns whether the string in P consists only of digits */
	while (*p != '\0')
		if (!isdigit(*p++))
			return 0;
	return 1;
}

EXPORT_SYMBOL char *vxutil_propose_home(char *dest, size_t size,
    const char *base, const char *username, unsigned int level)
{
	if (*username == '\0')
		fprintf(stderr, "%s: username has zero length\n", __FUNCTION__);
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

/*
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
		return const_cast(char *, src);

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
	hmc_t *cmd = NULL;
	int ret;
	HXformat_aprintf(catalog, &cmd, fmt);
	ret = system(cmd);
	hmc_free(cmd);
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

EXPORT_SYMBOL long vxutil_string_iday(const char *s)
{
	int day = 0, month = 0, year = 0, ret = 0;
	struct tm td;
	long sec;

	if ((ret = vxutil_parse_date(s, &day, &month, &year)) != 3)
		return ret;

	td.tm_mday = day;
	td.tm_mon  = month - 1;
	td.tm_year = year - 1900;
	if ((sec = mktime(&td)) == -1)
		return -1;

	return (unsigned long)sec / 86400;
}

EXPORT_SYMBOL long vxutil_string_xday(const char *s)
{
	int day = 0, month = 0, year = 0, ret = 0;
	if ((ret = vxutil_parse_date(s, &day, &month, &year)) <= 0)
		return ret;
	return ((year & 0xFFF) << 12) | ((month & 0xF) << 8) | (day & 0xFF);
}

EXPORT_SYMBOL int vxutil_valid_username(const char *n)
{
	if (*n == '\0')
		return 0;
	if (!((*n >= 'A' && *n <= 'Z') || (*n >= 'a' && *n <= 'z') ||
		*n == '_'))
		return 0;

	while (*n != '\0') {
		int valid;

		if (*n == '$' && *(n+1) == '\0')
			/* Samba accounts */
			return 1;

		valid = (*n >= 'A' && *n <= 'Z') || (*n >= 'a' && *n <= 'z') ||
		        (*n >= '0' && *n <= '9') || *n == '_' || *n == '.' ||
		        *n == '-';
		if (!valid)
			return 0;
		++n;
	}

	return 1;
}

//-----------------------------------------------------------------------------
static int vxutil_parse_date(const char *s, int *day, int *month, int *year)
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

/*
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
	return;
}

/*
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
				/*
				 * minus 1 because the original
				 * character will disappear
				 */
				case '"':
					n += sizeof_z("&quot;");
					break;
				case '&':
					n += sizeof_z("&amp;");
				case '<':
				case '>':
					n += sizeof_z("&lt;"); /* and &gt; */
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

/*
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
	while (isspace(*s)) ++s;
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

/*
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
	static const struct stab {
		const char *in, *out;
		int is, os;
	} subst_tab[] = {
#define E(s, d) {(s), (d), sizeof(s) - 1, sizeof(d) - 1}
		E("ß", "ss"),
		E("à", "a"), E("á", "a"), E("â", "a"), E("ã", "a"), E("ä", "ae"), E("å", "a"), E("æ", "ae"),
		E("è", "e"), E("é", "e"), E("ê", "e"), E("ë", "e"),
		E("ì", "i"), E("í", "i"), E("î", "i"), E("ï", "i"),
		E("ò", "o"), E("ó", "o"), E("ô", "o"), E("õ", "o"), E("ö", "oe"), E("ø", "o"),
		E("ù", "u"), E("ú", "u"), E("û", "u"), E("ü", "ue"),
		E("À", "A"), E("Á", "A"), E("Â", "A"), E("Ã", "A"), E("Ä", "Ae"), E("Å", "A"), E("Æ", "Ae"),
		E("È", "E"), E("É", "E"), E("Ê", "E"), E("Ë", "E"),
		E("Ì", "I"), E("Í", "I"), E("Î", "I"), E("Ï", "I"),
		E("Ò", "O"), E("Ó", "O"), E("Ô", "O"), E("Õ", "O"), E("Ö", "Oe"), E("Ø", "O"),
		E("Ù", "U"), E("Ú", "U"), E("Û", "U"), E("Ü", "Ue"),
		E("ç", "c"), E("Ç", "C"),
		E("Ñ", "N"), E("ñ", "n"),
		{NULL},
#undef E
	};

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
		} else if (*signed_cast(unsigned char *, src) & 0x80) {
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

//=============================================================================
