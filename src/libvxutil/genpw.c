/*
 *	libvxutil/genpw.c - Password generator
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <ctype.h>
#include <stdio.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Functions */
static void genpw_jp(char *, int, long);
static void genpw_zh(char *, int, long);
static void genpw_random(char *, int, long);

/* Variables */
/*
 * Of course this is not the true jp alphabet, but it is a table which has
 * mostly unambiguous syllables.
 */
static const char *const jp_table[] = {
	/*  0 */ "a",  "i",   "u",   "e",  "o",
	/*  5 */ "ka", "ki",  "ku",  "ke", "ko",
	/* 10 */ "ga", "gi",  "gu",  "ge", "go",
	/* 15 */ "sa", "shi", "su",  "se", "so",
	/* 20 */ "za", "ji",  "zu",  "ze", "zo",
	/* 25 */ "ta", "chi", "tsu", "te", "to",
	/* 30 */ "da", /* ji, zu */  "de", "do",
	/* 33 */ "na", "ni",  "nu",  "ne", "no",
	/* 38 */ "ha", "hi",  "fu",  "he", "ho",
	/* 43 */ "ba", "bi",  "bu",  "be", "bo",
	/* 48 */ "va", "vi",  "vu",  "ve", "vo",
	/* 53 */ "pa", "pi",  "pu",  "pe", "po",
	/* 58 */ "ma", "mi",  "mu",  "me", "mo",
	/* 63 */ "ra", "ri",  "ru",  "re", "ro",
	/* 68 */ "ya",        "yu",        "yo",
	/* 71 */ "wa",                     "wo",
	/* 73 */ "n", NULL,
};
static const char *const zh_table[] = {
	/*
	 * Some possible ambigiuousities have been left in (ju vs yu),
	 * others have been taken out (c* vs k*).
	 */
	"a", "ai", "an", "ang", "ao",
	"ba", "bai", "ban", "bang", "bao", "bei", "ben", "beng", "bi", "bian",
	"biao", "bie", "bin", "bing", "bo", "bu",
	/* "ca", "cai", "can", "cang", "cao", "ce", "cei", "cen", "ceng", */
	"cha", /* "chai", */ "chan", "chang", "chao", "che", "chen", "cheng",
	"chi", "chong", "chou", "chu", "chua", /* "chuai", */ "chuan",
	"chuang", /* "chui", */ "chun", "chuo", /* "ci", "cong", "cou", "cu",
	"cuan", "cui", "cun", "cuo", */
	"da", "dai", "dan", "dang", "dao", "de", "dei", "den", "deng", "di",
	"dian", "diao", "die", "ding", "diu", "dong", "dou", "du", "duan",
	"dui", "dun", "duo",
	"e", "ei", "en", "er",
	"fa", "fan", "fang", "fei", "fen", "feng", "fo", "fou", "fu",
	"ga", "gai", "gan", "gang", "gao", "ge", "gei", "gen", "geng", "gong",
	"gou", "gu", "gua", "guai", "guan", "guang", "gui", "gun", "guo",
	"ha", "hai", "han", "hang", "hao", "he", "hei", "hen", "heng",
	/* "hm", "hng", */ "hong", "hou", "hu", "hua", "huai", "huan", "huang",
	"hui", "hun", "huo",
	"ji", "jia", "jian", "jiang", "jiao", "jie", "jin", "jing", "jiong",
	"jiu", "ju", "juan", "jue", "jun",
	"ka", "kai", "kan", "kang", "kao", "ke", "kei", "ken", "keng", "kong",
	"kou", "ku", "kua", "kuai", "kuan", "kuang", "kui", "kun", "kuo",
	"la", "lai", "lan", "lang", "lao", "le", "lei", "leng", "li", "lia",
	"lian", "liang", "liao", "lie", "lin", "ling", "liu", "long", "lou",
	"lu", "luo", "luan", "lun",
	/* "m", */ "ma", "mai", "man", "mang", "mao", "mei", "men", "meng",
	"mi", "mian", "miao", "mie", "min", "ming", "miu", "mo", "mou", "mu",
	/* "n", */ "na", "nai", "nan", "nang", "nao", "ne", "nei", "nen",
	"neng", /* "ng", */ "ni", "nian", "niao", "nie", "nin", "ning", "niu",
	"nong", "nou", "nu", "nuo", "nuan",
	"o", "ou",
	"pa", "pai", "pan", "pang", "pao", "pei", "pen", "peng", "pi", "pian",
	"piao", "pie", "pin", "ping", "po", "pou", "pu",
	"qi", "qia", "qian", "qiang", "qiao", "qie", "qin", "qing", "qiong",
	"qiu", "qu", "quan", "que", "qun",
	"ran", "rang", "rao", "ren", "reng", "ri", "rong", "rou", "ru", "rua",
	"ruan", "rui", "run", "ruo",
	"sa", "sai", "san", "sang", "sao", "se", "sei", "sen", "seng", "sha",
	"shai", "shan", "shang", "shao", "she", "shei", "shen", "sheng", "shi",
	"shou", "shu", "shua", "shuai", "shuan", "shuang", "shui", "shun",
	"shuo", "si", "song", "sou", "su", "suan", "sui", "sun", "suo",
	"ta", "tai", "tan", "tang", "tao", "te", "teng", "ti", "tian", "tiao",
	"tie", "ting", "tong", "tou", "tu", "tuan", "tui", "tun", "tuo",
	"wa", "wai", "wan", "wang", "wei", "wen", "weng", "wo", "wu",
	"xi", "xia", "xian", "xiang", "xiao", "xie", "xin", "xing", "xiong",
	"xiu", "xu", "xuan", "xue", "xun",
	"ya", "yan", "yang", "yao", "ye", "yi", "yin", "ying", "yong", "you",
	"yu", "yuan", "yue", "yun",
	"za", "zai", "zan", "zang", "zao", "ze", "zei", "zen", "zeng", "zha",
	"zhai", "zhan", "zhang", "zhao", "zhe", "zhei", "zhen", "zheng", "zhi",
	"zhong", "zhou", "zhu", "zhua", "zhuai", "zhuan", "zhuang", "zhui",
	"zhun", "zhuo", "zi", "zong", "zou", "zu", "zuan", "zui", "zun", "zuo",
	NULL,
};

#define jpt_size (ARRAY_SIZE(jp_table) - 1)
#define zht_size (ARRAY_SIZE(zh_table) - 1)

//-----------------------------------------------------------------------------
EXPORT_SYMBOL void vxutil_genpw(char *plain, size_t len, long flags)
{
	long flad = flags & ~(GENPW_JP | GENPW_ZH);
	if (flags & GENPW_ZH)
		genpw_zh(plain, len, flad);
	else if (flags & GENPW_JP)
		genpw_jp(plain, len, flad);
	else
		genpw_random(plain, len, flad);
	return;
}

//-----------------------------------------------------------------------------
static void genpw_jp(char *plain, int size, long flags)
{
	long saved_flags  = flags;
	char *saved_plain = plain;
	int saved_size    = size;
	int prev          = -1;

	plain += size--;
	*--plain = '\0';

	while (size > 0) {
		const char *key;
		size_t ksz;
		int knum;

		if ((flags & (GENPW_1DIGIT | GENPW_O1DIGIT)) && HX_irand(0, 5) == 0) {
			*--plain = '0' + HX_irand(0, 10);
			--size;
			flags &= ~(GENPW_1DIGIT | GENPW_O1DIGIT);
			continue;
		}

		knum = HX_irand(0, jpt_size);
		if (prev == knum)
			continue;
		key = jp_table[knum];
		ksz = strlen(key);
		if (ksz > size)
			continue;
		if (size == 1 && knum == jpt_size - 1)
			/* no (standalone) "N" at the front */
			continue;

		plain -= ksz;
		size  -= ksz;
		strncpy(plain, key, ksz);
		prev   = knum;

		if ((flags & (GENPW_1CASE | GENPW_O1CASE)) && HX_irand(0, 10) == 0) {
			*plain = toupper(*plain);
			flags &= ~(GENPW_1CASE | GENPW_O1CASE);
		}
	}

	/*
	 * If the flags are still set, then these characteristics
	 * are not in the password. Redo it all.
	 */
	if (flags & (GENPW_1CASE | GENPW_1DIGIT))
		genpw_jp(saved_plain, saved_size, saved_flags);

	return;
}

static void genpw_zh(char *plain, int size, long flags)
{
	char *saved_plain = plain;
	long saved_flags  = flags;
	int saved_size    = size;
	int prev          = -1;

	plain   += size--;
	*--plain = '\0';

	while (size > 0) {
		const char *key;
		size_t ksz;
		int knum;

		if ((flags & (GENPW_1DIGIT | GENPW_O1DIGIT)) &&
		    HX_irand(0, 5) == 0) {
			*--plain = '0' + HX_irand(0, 10);
			--size;
			flags &= ~(GENPW_1DIGIT | GENPW_O1DIGIT);
			continue;
		}

		knum = HX_irand(0, zht_size);
		if (prev == knum)
			continue;
		key = zh_table[knum];
		ksz = strlen(key);
		if (ksz > size)
			continue;
		if (prev >= 0 && *key == *zh_table[prev])
			/*
			 * Avoid two words beginning with the same char. I did
			 * this to not have something like "sousuo".
			 */
			continue;

		plain -= ksz;
		size  -= ksz;
		strncpy(plain, key, ksz);
		prev   = knum;

		if ((flags & (GENPW_1CASE | GENPW_O1CASE)) && HX_irand(0, 10) == 0) {
			*plain = toupper(*plain);
			flags &= ~(GENPW_1CASE | GENPW_O1CASE);
		}
	}

	/*
	 * If the flags are still set, then these characteristics
	 * are not in the password. Redo it all.
	 */
	if (flags & (GENPW_1CASE | GENPW_1DIGIT))
		genpw_zh(saved_plain, saved_size, saved_flags);

	return;
}

static void genpw_random(char *plain, int size, long flags)
{
	char *saved_plain = plain;
	long saved_flags  = flags;
	int saved_size    = size;

	while (size-- > 0) {
		/* 20% probability for a digit */
		if ((flags & (GENPW_1DIGIT | GENPW_O1DIGIT)) &&
		    HX_irand(0, 5) == 0) {
			*plain++ = '0' + HX_irand(0, 10);
			flags &= ~(GENPW_1DIGIT | GENPW_O1DIGIT);
			continue;
		}
		*plain++ = 'a' + HX_irand(0, 26);
		if ((flags & (GENPW_1CASE | GENPW_O1CASE)) &&
		    HX_irand(0, 10) == 0) {
			*plain = toupper(*plain);
			flags &= ~(GENPW_1CASE | GENPW_O1CASE);
			/*continue;*/
		}
	}

	*plain++ = '\0';
	if (flags & (GENPW_1CASE | GENPW_1DIGIT))
		genpw_random(saved_plain, saved_size, saved_flags);

	return;
}

//=============================================================================
