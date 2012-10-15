/*
  Porting to Lua 5.2 and bug fixes: (C) Shmuel Zeigerman, 2010.
  MIT license.
*/

/*
*	Selene Unicode/UTF-8
*	This additions
*	Copyright (c) 2005 Malete Partner, Berlin, partner@malete.org
*	Available under "Lua 5.0 license", see http://www.lua.org/license.html#5
*	$Id: slnunico.c,v 1.5 2006/07/26 17:20:04 paul Exp $
*
*	contains code from
** lstrlib.c,v 1.109 2004/12/01 15:46:06 roberto Exp
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*
*	uses the udata table and a couple of expressions from Tcl 8.4.x UTF-8
* which comes with the following license.terms:

This software is copyrighted by the Regents of the University of
California, Sun Microsystems, Inc., Scriptics Corporation, ActiveState
Corporation and other parties.  The following terms apply to all files
associated with the software unless explicitly disclaimed in
individual files.

The authors hereby grant permission to use, copy, modify, distribute,
and license this software and its documentation for any purpose, provided
that existing copyright notices are retained in all copies and that this
notice is included verbatim in any distributions. No written agreement,
license, or royalty fee is required for any of the authorized uses.
Modifications to this software may be copyrighted by their authors
and need not follow the licensing terms described here, provided that
the new terms are clearly indicated on the first page of each file where
they apply.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
MODIFICATIONS.

GOVERNMENT USE: If you are acquiring this software on behalf of the
U.S. government, the Government shall have only "Restricted Rights"
in the software and related documentation as defined in the Federal
Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
are acquiring the software on behalf of the Department of Defense, the
software shall be classified as "Commercial Computer Software" and the
Government shall have only "Restricted Rights" as defined in Clause
252.227-7013 (c) (1) of DFARs.  Notwithstanding the foregoing, the
authors grant the U.S. Government and others acting in its behalf
permission to use and distribute the software in accordance with the
terms specified in this license.

(end of Tcl license terms)
*/

/*
According to http://ietf.org/rfc/rfc3629.txt we support up to 4-byte
(21 bit) sequences encoding the UTF-16 reachable 0-0x10FFFF.
Any byte not part of a 2-4 byte sequence in that range decodes to itself.
Ill formed (non-shortest) "C0 80" will be decoded as two code points C0 and 80,
not code point 0; see security considerations in the RFC.
However, UTF-16 surrogates (D800-DFFF) are accepted.

See http://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
for default grapheme clusters.
Lazy westerners we are (and lacking the Hangul_Syllable_Type data),
we care for base char + Grapheme_Extend, but not for Hangul syllable sequences.

For http://unicode.org/Public/UNIDATA/UCD.html#Grapheme_Extend
we use Mn (NON_SPACING_MARK) + Me (ENCLOSING_MARK),
ignoring the 18 mostly south asian Other_Grapheme_Extend (16 Mc, 2 Cf) from
http://www.unicode.org/Public/UNIDATA/PropList.txt
*/

/* Contains code from:
Quylthulg Copyright (C) 2009 Kein-Hong Man <keinhong@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lstrlib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary.
*/
#if !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif

/*
** length modifier for integer conversions ** in 'string.format' and
** integer type corresponding to the previous length
*/

#if defined(LUA_USELONGLONG)

#define LUA_INTFRMLEN           "ll"
#define LUA_INTFRM_T            long long

#else

#define LUA_INTFRMLEN           "l"
#define LUA_INTFRM_T            long

#endif

#ifndef SLN_UNICODENAME /* unless set it luaconf */
# define SLN_UNICODENAME "unicode"
#endif


#include "slnudata.c"
#define charinfo(c) (~0xFFFF&(c) ? 0 : GetUniCharInfo(c)) /* BMP only */
#define charcat(c) (UNICODE_CATEGORY_MASK & charinfo(c))
#define Grapheme_Extend(code) \
	(1 & (((1<<NON_SPACING_MARK)|(1<<ENCLOSING_MARK)) >> charcat(code)))

enum   /* operation modes */
{
	MODE_ASCII, /* single byte 7bit */
	MODE_LATIN, /* single byte 8859-1 */
	MODE_UTF8,	/* UTF-8 by code points */
	MODE_GRAPH	/* UTF-8 by grapheme clusters */
#define MODE_MBYTE(mode) (~1&(mode))
};


/* macro to `unsign' a character */
#define uchar(c)				((unsigned char)(c))

typedef const unsigned char cuc; /* it's just toooo long :) */


static int get_mode(lua_State *L)
{
#if LUA_VERSION_NUM == 501
	int mode;
	lua_getfield(L, LUA_ENVIRONINDEX, "mode");
	mode = (int)lua_tointeger(L, -1);
	lua_pop(L, 1);
#elif LUA_VERSION_NUM == 502
	int mode = (int)lua_tointeger(L, lua_upvalueindex(1));
#endif
	return mode;
}

/* This function taken from:
 * Quylthulg Copyright (C) 2009 Kein-Hong Man <keinhong@gmail.com>
 */
static int grab_one_utf8(const char **ps)
{
	unsigned char v, ext;
	int c, min;
	v = (unsigned char)*((*ps)++);

	if(v <= 0x7F)
	{
		ext = 0; c = v; min = 0;
	}
	else if(v <= 0xC1)
	{
		return -1;              /* bad sequence */
	}
	else if(v <= 0xDF)
	{
		ext = 1; c = v & 0x1F; min = 0x80;
	}
	else if(v <= 0xEF)
	{
		ext = 2; c = v & 0x0F; min = 0x800;
	}
	else if(v <= 0xF4)
	{
		ext = 3; c = v & 0x07; min = 0x10000;
	}
	else
	{
		return -1;              /* can't fit */
	}

	/* verify extended sequence */
	while(ext--)
	{
		v = (unsigned char)*((*ps)++);

		if(v < 0x80 || v > 0xBF)
		{
			return -1;          /* bad sequence */
		}

		c = (c << 6) | (v & 0x3F);
	}

	if(c < min)
	{
		return -1;              /* bad sequence */
	}

	return c;
}


static const char* advance(const char **s, int multibyte)
{
	const char *old = *s;

	if(!multibyte || grab_one_utf8(s) < 0)
		*s = old + 1;

	return old;
}

static void utf8_enco(luaL_Buffer *b, unsigned c)
{
	if(0x80 > c)
	{
		luaL_addchar(b, c);
		return;
	}

	if(0x800 > c)
		luaL_addchar(b, 0xC0|(c>>6));
	else
	{
		if(0x10000 > c)
			luaL_addchar(b, 0xE0|(c>>12));
		else
		{
			luaL_addchar(b, 0xF0|(c>>18));
			luaL_addchar(b, 0x80|(0x3F&(c>>12)));
		}

		luaL_addchar(b, 0x80|(0x3F&(c>>6)));
	}

	luaL_addchar(b, 0x80|(0x3F&c));
}	/* utf8_enco */


/* end must be > *pp */
static unsigned utf8_deco(const char **pp, const char *end)
{
	register cuc *p = (cuc*)*pp, * const e = (cuc*)end;
	unsigned first = *p, code;
	*pp = (const char*)++p; /* eat one */
	/* check ASCII, dangling cont., non-shortest or not continued */
	if(0xC2 > first || e == p || 0x80 != (0xC0&*p)) return first;

	code = 0x3F&*p++; /* save 1st cont. */
	/* check 2 byte (5+6 = 11 bit) sequence up to 0x7FF */
	if(0xE0 > first)    /* any >= C2 is valid */
	{
		code |= (0x1F&first)<<6;
		goto seq;
	}

	if(e != p && 0x80 == (0xC0&*p))    /* is continued */
	{
		code = code<<6 | (0x3F&*p++); /* save 2nd */

		if(0xF0 > first)    /* 3 byte (4+6+6 = 16 bit) seq -- want 2nd cont. */
		{
			if(0xF800&(code |= (0x0F&first)<<12)   /* >0x7FF: not non-shortest */
			        /* && 0xD800 != (0xD800 & code) -- nah, let surrogates pass */
			  )
				goto seq;
		}
		else if(e != p && 0x80 == (0xC0&*p)    /* check 3rd */
		        /* catch 0xF4 < first and other out-of-bounds */
		        && 0x110000 > (code = (0x0F&first)<<18 | code<<6 | (0x3F&*p++))
		        && 0xFFFF < code /* not a 16 bitty */
		       )
			goto seq;
	}

	return first;
seq:
	*pp = (const char*)p;
	return code;
}	/* utf8_deco */


/* reverse decode before pp > start */
static unsigned utf8_oced(const char **pp, const char *start)
{
	register cuc *p = (cuc*)*pp, * const s = (cuc*)start;
	unsigned last = *--p, code;
	*pp = (const char*)p; /* eat one */
	/* check non-continuer or at the edge */
	if(0x80 != (0xC0&last) || s == p) return last;

	code = 0x3F&last; /* save last cont. */

	if(0xC0 == (0xE0&*--p))    /* preceeded by 2 byte seq starter */
	{
		if(0xC2 <= *p) { code |= (0x1F&*p)<<6; goto seq; }
	}
	else if(0x80 == (0xC0&*p) && s<p)
	{
		code |= (0x3F&*p)<<6;

		if(0xE0 == (0xF0&*--p))    /* 3 byte starter */
		{
			if(0xF800&(code |= (0x0F&*p)<<12)) goto seq;
		}
		else if(0x80 == (0xC0&*p) && s<=--p    /* valid 4 byte ? */
		        && 0x110000 > (code |= (0x0F&*p)<<18 | (0x3F&p[1])<<12)
		        && 0xFFFF < code
		       )
			goto seq;
	}

	return last;
seq:
	*pp = (const char*)p;
	return code;
}	/* utf8_oced */


/* skip over Grapheme_Extend codes */
static void utf8_graphext(const char **pp, const char *end)
{
	const char *p = *pp;

	for(; p < end; *pp=p)
	{
		unsigned code = utf8_deco(&p, end);

		if(!Grapheme_Extend(code)) break;
	}
}	/* utf8_graphext */


static size_t utf8_count(const char **pp, ptrdiff_t bytes, int graph, size_t max)
{
	const char *const end = *pp+bytes;
	size_t count = 0;

	while(*pp < end && count != max)
	{
		unsigned code = utf8_deco(pp, end);
		count++;

		if(!graph) continue;

		if(Grapheme_Extend(code) && 1<count) count--;  /* uncount */
	}

	if(graph && count == max)  /* gather more extending */
		utf8_graphext(pp, end);

	return count;
}	/* utf8_count */



static int unic_len(lua_State *L)
{
	size_t l;
	const char *s = luaL_checklstring(L, 1, &l);
	int mode = get_mode(L);

	if(MODE_MBYTE(mode)) l = utf8_count(&s, l, mode-2, -1);

	lua_pushinteger(L, l);
	return 1;
}


static ptrdiff_t posrelat(ptrdiff_t pos, size_t len)
{
	/* relative string position: negative means back from end */
	return (pos>=0) ? pos : (ptrdiff_t)len+pos+1;
}


static int unic_sub(lua_State *L)
{
	size_t l;
	const char *s = luaL_checklstring(L, 1, &l), *p, *e=s+l;
	ptrdiff_t start = luaL_checkinteger(L, 2);
	ptrdiff_t end = luaL_optinteger(L, 3, -1);
	int mode = get_mode(L);

	if(MODE_MBYTE(mode)) { p=s; l = utf8_count(&p, l, mode-2, -1); }

	start = posrelat(start, l);
	end = posrelat(end, l);

	if(start < 1) start = 1;

	if(end > (ptrdiff_t)l) end = (ptrdiff_t)l;

	if(start > end)
		lua_pushliteral(L, "");
	else
	{
		l = end - --start; /* #units */

		if(!(MODE_MBYTE(mode)))  /* single byte */
			s += start;
		else
		{
			if(start) utf8_count(&s, e-s, mode-2, start);  /* skip */

			p = s;
			utf8_count(&p, e-p, mode-2, l);
			l = p-s;
		}

		lua_pushlstring(L, s, l);
	}

	return 1;
}


static int str_reverse(lua_State *L)    /* TODO? whatfor? */
{
	size_t l;
	luaL_Buffer b;
	const char *s = luaL_checklstring(L, 1, &l), *p = s+l, *q;
	int mode = get_mode(L), mb = MODE_MBYTE(mode);
	luaL_buffinit(L, &b);

	if(!mb)
		while(s < p--) luaL_addchar(&b, *p);
	else
	{
		unsigned code;

		while(s < p)
		{
			q = p;
			code = utf8_oced(&p, s);

			if(MODE_GRAPH == mode)
				while(Grapheme_Extend(code) && p>s) code = utf8_oced(&p, s);

			luaL_addlstring(&b, p, q-p);
		}
	}

	luaL_pushresult(&b);
	return 1;
}



static int unic_lower(lua_State *L)
{
	size_t l;
	luaL_Buffer b;
	const char *s = luaL_checklstring(L, 1, &l), * const e=s+l;
	int mode = get_mode(L), mb = MODE_MBYTE(mode);
	luaL_buffinit(L, &b);

	while(s < e)
	{
		unsigned c = mb ? utf8_deco(&s, e) : uchar(*s++);
		int info = charinfo(c);

		if(GetCaseType(info)&0x02 && (mode || !(0x80&c))) c += GetDelta(info);
		if(mb) utf8_enco(&b, c); else luaL_addchar(&b, c);
	}

	luaL_pushresult(&b);
	return 1;
}


static int unic_upper(lua_State *L)
{
	size_t l;
	luaL_Buffer b;
	const char *s = luaL_checklstring(L, 1, &l), * const e=s+l;
	int mode = get_mode(L), mb = MODE_MBYTE(mode);
	luaL_buffinit(L, &b);

	while(s < e)
	{
		unsigned c = mb ? utf8_deco(&s, e) : uchar(*s++);
		int info = charinfo(c);

		if(GetCaseType(info)&0x04 && (mode || !(0x80&c))) c -= GetDelta(info);
		if(mb) utf8_enco(&b, c); else luaL_addchar(&b, c);
	}

	luaL_pushresult(&b);
	return 1;
}


static int str_rep(lua_State *L)
{
	size_t l;
	luaL_Buffer b;
	const char *s = luaL_checklstring(L, 1, &l);
	int n = luaL_checkint(L, 2);
	luaL_buffinit(L, &b);

	while(n-- > 0)
		luaL_addlstring(&b, s, l);

	luaL_pushresult(&b);
	return 1;
}


static int unic_byte(lua_State *L)
{
	size_t l;
	ptrdiff_t posi, pose;
	const char *s = luaL_checklstring(L, 1, &l), *p, *e=s+l;
	int n, mode = get_mode(L), mb = MODE_MBYTE(mode);

	if(mb) { p=s; l = utf8_count(&p, l, mode-2, -1); }

	posi = posrelat(luaL_optinteger(L, 2, 1), l);
	pose = posrelat(luaL_optinteger(L, 3, posi), l);

	if(posi <= 0) posi = 1;

	if((size_t)pose > l) pose = l;

	if(0 >= (n = (int)(pose - --posi))) return 0;	/* empty interval */

	if(!mb)
		e = (s += posi) + n;
	else
	{
		if(posi) utf8_count(&s, e-s, mode-2, posi);  /* skip */

		p=s;
		utf8_count(&p, e-s, mode-2, n);
		e=p;
	}

	/* byte count is upper bound on #elements */
	luaL_checkstack(L, (int)(e-s), "string slice too long");

	for(n=0; s<e; n++)
		lua_pushinteger(L, mb ? utf8_deco(&s, e) : uchar(*s++));

	return n;
}


static int unic_char(lua_State *L)
{
	int i, n = lua_gettop(L);	/* number of arguments */
	int mode = get_mode(L), mb = MODE_MBYTE(mode);
	unsigned lim = mb ? 0x110000 : 0x100;
	luaL_Buffer b;
	luaL_buffinit(L, &b);

	for(i=1; i<=n; i++)
	{
		unsigned c = luaL_checkint(L, i);
		luaL_argcheck(L, lim > c, i, "invalid value");
		if(mb) utf8_enco(&b, c); else luaL_addchar(&b, c);
	}

	luaL_pushresult(&b);
	return 1;
}


static int writer(lua_State *L, const void* b, size_t size, void* B)
{
	(void)L;
	luaL_addlstring((luaL_Buffer*) B, (const char *)b, size);
	return 0;
}


static int str_dump(lua_State *L)
{
	luaL_Buffer b;
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_settop(L, 1);
	luaL_buffinit(L,&b);

	if(lua_dump(L, writer, &b) != 0)
		luaL_error(L, "unable to dump given function");

	luaL_pushresult(&b);
	return 1;
}



/*
** {======================================================
** PATTERN MATCHING
** =======================================================
* find/gfind(_aux) -> match, push_captures
* gsub -> match, add_s (-> push_captures)
* push_captures, add_s -> push_onecapture
* match ->
* 	start/end_capture -> match,
* 	match_capture, matchbalance, classend -> -,
* 	min/max_expand -> match, singlematch
* 	singlematch -> matchbracketclass, match_class,
* 	matchbracketclass -> match_class -> -,
*/


#define CAP_UNFINISHED	(-1)
#define CAP_POSITION	(-2)

typedef struct MatchState
{
	const char *src_init;	/* init of source string */
	const char *src_end;	/* end (`\0') of source string */
	lua_State *L;
	int level;	/* total number of captures (finished or unfinished) */
	int mode;
	int mb;
	struct
	{
		const char *init;
		ptrdiff_t len;
	} capture[LUA_MAXCAPTURES];
} MatchState;


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


static int check_capture(MatchState *ms, int l)
{
	l -= '1';

	if(l < 0 || l >= ms->level || ms->capture[l].len == CAP_UNFINISHED)
		return luaL_error(ms->L, "invalid capture index");

	return l;
}


static int capture_to_close(MatchState *ms)
{
	int level = ms->level;

	for(level--; level>=0; level--)
		if(ms->capture[level].len == CAP_UNFINISHED) return level;

	return luaL_error(ms->L, "invalid pattern capture");
}


static const char *classend(MatchState *ms, const char *p)
{
	switch(*p)
	{
		case L_ESC:

			if(!*++p) luaL_error(ms->L, "malformed pattern (ends with " LUA_QL("%%") ")");

			break;
		case '[':

			/* if (*p == '^') p++; -- no effect */
			do  	/* look for a `]' */
			{
				if(!*p) luaL_error(ms->L, "malformed pattern (missing " LUA_QL("]") ")");

				if(L_ESC == *(p++) && *p) p++;	/* skip escapes (e.g. `%]') */
			}
			while(']' != *p);

			break;
		default:

			if(!ms->mb) break;

			utf8_deco(&p, p+4);
			return p;
	}

	return p+1;
}	/* classend */


/*
 * The following macros are used for fast character category tests.  The
 * x_BITS values are shifted right by the category value to determine whether
 * the given category is included in the set.
 */

#define LETTER_BITS ((1 << UPPERCASE_LETTER) | (1 << LOWERCASE_LETTER) \
                     | (1 << TITLECASE_LETTER) | (1 << MODIFIER_LETTER) | (1 << OTHER_LETTER))

#define DIGIT_BITS (1 << DECIMAL_DIGIT_NUMBER)

#define NUMBER_BITS (1 << DECIMAL_DIGIT_NUMBER) \
	| (1 << LETTER_NUMBER) | (1 << OTHER_NUMBER)

#define SPACE_BITS ((1 << SPACE_SEPARATOR) | (1 << LINE_SEPARATOR) \
                    | (1 << PARAGRAPH_SEPARATOR))

#define CONNECTOR_BITS (1 << CONNECTOR_PUNCTUATION)

#define PUNCT_BITS ((1 << CONNECTOR_PUNCTUATION) | \
                    (1 << DASH_PUNCTUATION) | (1 << OPEN_PUNCTUATION) | \
                    (1 << CLOSE_PUNCTUATION) | (1 << INITIAL_QUOTE_PUNCTUATION) | \
                    (1 << FINAL_QUOTE_PUNCTUATION) | (1 << OTHER_PUNCTUATION))


/* character c matches class cl. undefined for cl not ascii */
static int match_class(int c, int cl, int mode)
{
	int msk, res;

	switch(0x20|cl /*tolower*/)
	{
		case 'a' : msk = LETTER_BITS; break;
		case 'c' : msk = 1<<CONTROL; break;
		case 'x' : /* hexdigits */

			if(0x40==(~0x3f&c)/*64-127*/ && 1&(0x7e/*a-f*/>>(0x1f&c))) goto matched;

		case 'd' : msk = 1<<DECIMAL_DIGIT_NUMBER; mode=0;/* ASCII only */ break;
		case 'l' : msk = 1<<LOWERCASE_LETTER; break;
		case 'n' : msk = NUMBER_BITS; break; /* new */
		case 'p' : msk = PUNCT_BITS; break;
		case 's' :
#define STDSPACE /* standard "space" controls 9-13 */ \
	(1<<9/*TAB*/|1<<10/*LF*/|1<<11/*VT*/|1<<12/*FF*/|1<<13/*CR*/)

			if(!(~0x1f & c) && 1&(STDSPACE >> c)) goto matched;

			msk = SPACE_BITS;
			break;
		case 'u' : msk = 1<<UPPERCASE_LETTER; break;
			/*
				this is not compatible to lua 5.1, where %w is just a letter or a digit
			case 'w' : msk = LETTER_BITS|NUMBER_BITS|CONNECTOR_BITS; break;
			*/
		case 'w' : msk = LETTER_BITS|NUMBER_BITS; break;

		case 'z' : if(!c) goto matched; msk = 0; break;

		default: return cl == c;
	}

	res = 1 & (msk >> charcat(c));

	if(!mode && 0x80&c) res = 0;

	if(0)
	{
matched:
		res = 1;
	}

	return 0x20&cl /*islower*/ ? res : !res;
}	/* match_class */


/* decode single byte or UTF-8 seq; advance *s */
static unsigned deco(const MatchState *ms, const char **s, const char *e)
{
	return ms->mb ? utf8_deco(s, e) : *(unsigned char*)(*s)++;
}

/* s must be < ms->src_end, p < ep */
static const char *singlematch(const MatchState *ms,
                               const char *s, const char *p, const char *ep)
{
	int neg = 0;
	unsigned c1, c2;
	unsigned c;
#ifdef OPTIMIZE_SIZE
	c = deco(ms, &s, ms->src_end);
#else

	if(!ms->mb || !(0x80&*s))
		c = *(unsigned char*)s++;
	else
		c = utf8_deco(&s, ms->src_end);

#endif

	switch(*p)
	{
		case L_ESC:

			if(match_class(c, uchar(p[1]), ms->mode))
			{
			case '.': /* the all class */
#ifndef OPTIMIZE_SIZE

				if(MODE_GRAPH != ms->mode) return s;  /* common fast path */

#endif
				goto matched_class;
			}

			s = 0;
			break;
		default:
#ifdef OPTIMIZE_SIZE
			c1 = deco(ms, &p, ep);
#else

			if(!ms->mb || !(0x80&*p))
				c1 = *(unsigned char*)p++;
			else
				c1 = utf8_deco(&p, ep);

#endif

			if(c != c1) s = 0;

			break;
		case '[': /* matchbracketclass */
			ep--; /* now on the ']' */

			if((neg = '^' == *++p)) p++;	/* skip the `^' */

			while(p < ep)
			{
				if(*p == L_ESC)
				{
					if(match_class(c, uchar(*++p), ms->mode)) goto matched_class_in_brack;

					p++;
					continue;
				}

				c1 = deco(ms, &p, ep);

				/* in lua-5.1 and 5.1.1 a trailing '-' is allowed
					dynasm.lua relies on this
				*/
				if(ep <= p + 1 || '-' != *p)
				{
					const char *op = p, *es;

					if(MODE_GRAPH == ms->mode) utf8_graphext(&p, ep);

					if(c != c1) continue;

					if(MODE_GRAPH != ms->mode) goto matched;

					/* comp grapheme extension */
					es = s;
					utf8_graphext(&es, ms->src_end);

					if(es-s == p-op && (es==s || !memcmp(s, op, es-s))) goto matched;

					continue;
				}

				++p;
				/* range c1-c2 -- no extend support in range bounds... */
				/* if (ep == ++p) break; see above */ /* bugger - trailing dash */
				c2 = deco(ms, &p, ep);

				if(c2 < c1) { unsigned swap=c1; c1=c2; c2=swap; }

				if(c1 <= c && c <= c2) goto matched_class_in_brack;  /* ...but extend match */
			}

			/* not matched */
			neg = !neg;
matched:

			if(neg) s = 0;

			/* matchbracketclass */
	}

	return s;
matched_class_in_brack: /* matched %something or range in [] */

	if(neg)
		s = 0;
	else
	{
matched_class: /* matched %something or . */

		if(MODE_GRAPH == ms->mode) utf8_graphext(&s, ms->src_end);
	}

	return s;
}


static const char *match(MatchState *ms, const char *s, const char *p);


static const char *matchbalance(MatchState *ms, const char *s,
                                const char *p)
{
	if(*p == 0 || *(p+1) == 0)
		luaL_error(ms->L, "unbalanced pattern");

	if(*s != *p) return NULL;
	else
	{
		int b = *p;
		int e = *(p+1);
		int cont = 1;

		while(++s < ms->src_end)
		{
			if(*s == e)
			{
				if(--cont == 0) return s+1;
			}
			else if(*s == b) cont++;
		}
	}

	return NULL;	/* string ends out of balance */
}


static const char *max_expand(MatchState *ms,
                              const char *s, const char *p, const char *ep)
{
	const char *sp = s, *es;

	while(sp<ms->src_end && (es = singlematch(ms, sp, p, ep)))
		sp = es;

	/* keeps trying to match with the maximum repetitions */
	while(sp>=s)
	{
		const char *res = match(ms, sp, ep+1);

		if(res || sp==s) return res;

		if(!ms->mb)
			sp--;	/* else didn't match; reduce 1 repetition to try again */
		else
		{
			unsigned code = utf8_oced(&sp, s);

			if(MODE_GRAPH == ms->mode)
				while(Grapheme_Extend(code) && sp>s) code = utf8_oced(&sp, s);
		}
	}

	return NULL;
}


static const char *min_expand(MatchState *ms,
                              const char *s, const char *p, const char *ep)
{
	do
	{
		const char *res = match(ms, s, ep+1);

		if(res) return res;

		if(s >= ms->src_end) break;
	}
	while((s = singlematch(ms, s, p, ep)));    /* try with one more repetition */

	return NULL;
}


static const char *start_capture(MatchState *ms, const char *s,
                                 const char *p, int what)
{
	const char *res;
	int level = ms->level;

	if(level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");

	ms->capture[level].init = s;
	ms->capture[level].len = what;
	ms->level = level+1;

	if((res=match(ms, s, p)) == NULL)	/* match failed? */
		ms->level--;	/* undo capture */

	return res;
}


static const char *end_capture(MatchState *ms, const char *s,
                               const char *p)
{
	int l = capture_to_close(ms);
	const char *res;
	ms->capture[l].len = s - ms->capture[l].init;	/* close capture */

	if((res = match(ms, s, p)) == NULL)	/* match failed? */
		ms->capture[l].len = CAP_UNFINISHED;	/* undo capture */

	return res;
}


static const char *match_capture(MatchState *ms, const char *s, int l)
{
	size_t len;
	l = check_capture(ms, l);
	len = ms->capture[l].len;

	if((size_t)(ms->src_end-s) >= len &&
	        memcmp(ms->capture[l].init, s, len) == 0)
		return s+len;
	else return NULL;
}


static const char *match(MatchState *ms, const char *s, const char *p)
{
init: /* using goto's to optimize tail recursion */

	switch(*p)
	{
		case '(':  	/* start capture */
		{
			if(*(p+1) == ')')	/* position capture? */
				return start_capture(ms, s, p+2, CAP_POSITION);
			else
				return start_capture(ms, s, p+1, CAP_UNFINISHED);
		}
		case ')':  	/* end capture */
		{
			return end_capture(ms, s, p+1);
		}
		case L_ESC:
		{
			switch(*(p+1))
			{
				case 'b':  	/* balanced string? */
				{
					s = matchbalance(ms, s, p+2);

					if(s == NULL) return NULL;

					p+=4; goto init;	/* else return match(ms, s, p+4); */
				}
#if 0 /* TODO */
				case 'f':  	/* frontier? */
				{
					const char *ep; char previous;
					p += 2;

					if(*p != '[')
						luaL_error(ms->L, "missing " LUA_QL("[") " after "
						           LUA_QL("%%f") " in pattern");

					luaL_error(ms->L, "missing `[' after `%%f' in pattern");
					ep = classend(ms, p);	/* points to what is next */
					/* with UTF-8, getting the previous is more complicated */
					previous = (s == ms->src_init) ? '\0' : *(s-1);

					/* use singlematch to apply all necessary magic */
					if(singlematch(uchar(previous), p, ep-1) ||
					        !singlematch(uchar(*s), p, ep-1)) return NULL;

					p=ep; goto init;	/* else return match(ms, s, ep); */
				}
#endif
				default:
				{
					if(isdigit(uchar(*(p+1))))  	/* capture results (%0-%9)? */
					{
						s = match_capture(ms, s, uchar(*(p+1)));

						if(s == NULL) return NULL;

						p+=2; goto init;	/* else return match(ms, s, p+2) */
					}

					goto dflt;	/* case default */
				}
			}
		}
		case '\0':  	/* end of pattern */
		{
			return s;	/* match succeeded */
		}
		case '$':
		{
			if(*(p+1) == '\0')	/* is the `$' the last char in pattern? */
				return (s == ms->src_end) ? s : NULL;	/* check end of string */
			else goto dflt; /* ??? */
		}
default: dflt:  	/* it is a pattern item */
			{
				const char *ep = classend(ms, p);	/* points to what is next */
				const char *es = 0;

				if(s < ms->src_end) es = singlematch(ms, s, p, ep);

				switch(*ep)
				{
					case '?':  	/* optional */
					{
						const char *res;

						if(es && (res=match(ms, es, ep+1))) return res;

						p=ep+1; goto init;	/* else return match(ms, s, ep+1); */
					}
					case '*':  	/* 0 or more repetitions */
					{
						return max_expand(ms, s, p, ep);
					}
					case '+':  	/* 1 or more repetitions */
					{
						return (es ? max_expand(ms, es, p, ep) : NULL);
					}
					case '-':  	/* 0 or more repetitions (minimum) */
					{
						return min_expand(ms, s, p, ep);
					}
					default:
					{
						if(!es) return NULL;

						s=es; p=ep; goto init;	/* else return match(ms, s+1, ep); */
					}
				}
			}
	}
}



static const char *lmemfind(const char *s1, size_t l1,
                            const char *s2, size_t l2)
{
	if(l2 == 0) return s1;	/* empty strings are everywhere */
	else if(l2 > l1) return NULL;	/* avoids a negative `l1' */
	else
	{
		const char *init;	/* to search for a `*s2' inside `s1' */
		l2--;	/* 1st char will be checked by `memchr' */
		l1 = l1-l2;	/* `s2' cannot be found after that */

		while(l1 > 0 && (init = (const char *)memchr(s1, *s2, l1)) != NULL)
		{
			init++;	 /* 1st char is already checked */

			if(memcmp(init, s2+1, l2) == 0)
				return init-1;
			else  	/* correct `l1' and `s1' to try again */
			{
				l1 -= init-s1;
				s1 = init;
			}
		}

		return NULL;	/* not found */
	}
}


static void push_onecapture(MatchState *ms, int i, const char *s,
                            const char *e)
{
	if(i >= ms->level)
	{
		if(i == 0)   /* ms->level == 0, too */
			lua_pushlstring(ms->L, s, e - s);  /* add whole match */
		else
			luaL_error(ms->L, "invalid capture index");
	}
	else
	{
		ptrdiff_t l = ms->capture[i].len;

		if(l == CAP_UNFINISHED) luaL_error(ms->L, "unfinished capture");

		if(l == CAP_POSITION)
			lua_pushinteger(ms->L, ms->capture[i].init - ms->src_init + 1);
		else
			lua_pushlstring(ms->L, ms->capture[i].init, l);
	}
}


static int push_captures(MatchState *ms, const char *s, const char *e)
{
	int i;
	int nlevels = (ms->level == 0 && s) ? 1 : ms->level;
	luaL_checkstack(ms->L, nlevels, "too many captures");

	for(i = 0; i < nlevels; i++)
		push_onecapture(ms, i, s, e);

	return nlevels;  /* number of strings pushed */
}


static int unic_find_aux(lua_State *L, int find)
{
	size_t l1, l2;
	const char *s = luaL_checklstring(L, 1, &l1);
	const char *p = luaL_checklstring(L, 2, &l2);
	ptrdiff_t init = posrelat(luaL_optinteger(L, 3, 1), l1) - 1;

	if(init < 0) init = 0;
	else if((size_t)(init) > l1) init = (ptrdiff_t)l1;

	if(find && (lua_toboolean(L, 4) ||	/* explicit request? */
	            strpbrk(p, SPECIALS) == NULL))  	/* or no special characters? */
	{
		/* do a plain search */
		const char *s2 = lmemfind(s+init, l1-init, p, l2);

		if(s2)
		{
			lua_pushinteger(L, s2-s+1);
			lua_pushinteger(L, s2-s+l2);
			return 2;
		}
	}
	else
	{
		MatchState ms;
		int anchor = (*p == '^') ? (p++, 1) : 0;
		const char *s1=s+init;
		ms.L = L;
		ms.src_init = s;
		ms.src_end = s+l1;
		ms.mode = get_mode(L);
		ms.mb = MODE_MBYTE(ms.mode);

		do
		{
			const char *res;
			ms.level = 0;

			if((res=match(&ms, s1, p)) != NULL)
			{
				if(find)
				{
					lua_pushinteger(L, s1-s+1);  /* start */
					lua_pushinteger(L, res-s);   /* end */
					return push_captures(&ms, NULL, 0) + 2;
				}
				else
					return push_captures(&ms, s1, res);
			}
		}
		while(advance(&s1, ms.mb) < ms.src_end && !anchor);
	}

	lua_pushnil(L);	/* not found */
	return 1;
}

static int unic_find(lua_State *L)
{
	return unic_find_aux(L, 1);
}


static int unic_match(lua_State *L)
{
	return unic_find_aux(L, 0);
}



static int gmatch_aux(lua_State *L)
{
	MatchState *ms = (MatchState*)lua_tostring(L, lua_upvalueindex(4));
	const char *p = lua_tostring(L, lua_upvalueindex(2));
	const char *src;

	for(src = ms->src_init + (size_t)lua_tointeger(L, lua_upvalueindex(3));
	        src <= ms->src_end;
	        advance(&src, ms->mb))
	{
		const char *e;
		ms->level = 0;

		if((e = match(ms, src, p)) != NULL)
		{
			const char *q = e;

			if(e == src) advance(&q, ms->mb);  /* empty match? go at least one position */

			lua_pushinteger(L, q - ms->src_init);
			lua_replace(L, lua_upvalueindex(3));
			return push_captures(ms, src, e);
		}
	}

	return 0;	/* not found */
}



static int gmatch(lua_State *L)
{
	size_t len;
	MatchState ms;
	ms.L = L;
	ms.src_init = luaL_checklstring(L, 1, &len);
	ms.src_end = ms.src_init + len;
	ms.mode = get_mode(L);
	ms.mb = MODE_MBYTE(ms.mode);
	luaL_checkstring(L, 2);
	lua_settop(L, 2);
	lua_pushinteger(L, 0);
	lua_pushlstring(L, (char*)&ms, sizeof(MatchState));
	lua_pushcclosure(L, gmatch_aux, 4);
	return 1;
}

static int gfind_nodef(lua_State *L)
{
	return luaL_error(L, LUA_QL("string.gfind") " was renamed to "
	                  LUA_QL("string.gmatch"));
}


static void add_s(MatchState *ms, luaL_Buffer *b,
                  const char *s, const char *e)
{
	size_t l, i;
	const char *news = lua_tolstring(ms->L, 3, &l);

	for(i = 0; i < l; i++)
	{
		if(news[i] != L_ESC)
			luaL_addchar(b, news[i]);
		else
		{
			i++;  /* skip ESC */

			if(!isdigit(uchar(news[i])))
				luaL_addchar(b, news[i]);
			else if(news[i] == '0')
				luaL_addlstring(b, s, e - s);
			else
			{
				push_onecapture(ms, news[i] - '1', s, e);
				luaL_addvalue(b);  /* add capture to accumulated result */
			}
		}
	}
}

static void add_value(MatchState *ms, luaL_Buffer *b, const char *s,
                      const char *e)
{
	lua_State *L = ms->L;

	switch(lua_type(L, 3))
	{
		case LUA_TNUMBER:
		case LUA_TSTRING:
		{
			add_s(ms, b, s, e);
			return;
		}
		case LUA_TFUNCTION:
		{
			int n;
			lua_pushvalue(L, 3);
			n = push_captures(ms, s, e);
			lua_call(L, n, 1);
			break;
		}
		case LUA_TTABLE:
		{
			push_onecapture(ms, 0, s, e);
			lua_gettable(L, 3);
			break;
		}
		default:
		{
			luaL_argerror(L, 3, "string/function/table expected");
			return;
		}
	}

	if(!lua_toboolean(L, -1))     /* nil or false? */
	{
		lua_pop(L, 1);
		lua_pushlstring(L, s, e - s);  /* keep original text */
	}
	else if(!lua_isstring(L, -1))
		luaL_error(L, "invalid replacement value (a %s)", luaL_typename(L, -1));

	luaL_addvalue(b);  /* add result to accumulator */
}

static int unic_gsub(lua_State *L)
{
	size_t srcl;
	const char *src = luaL_checklstring(L, 1, &srcl);
	const char *p = luaL_checkstring(L, 2);
	int max_s = luaL_optint(L, 4, srcl+1);
	int anchor = (*p == '^') ? (p++, 1) : 0;
	int n = 0;
	MatchState ms;
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	ms.L = L;
	ms.src_init = src;
	ms.src_end = src+srcl;
	ms.mode = get_mode(L);
	ms.mb = MODE_MBYTE(ms.mode);

	while(n < max_s)
	{
		const char *e;
		ms.level = 0;
		e = match(&ms, src, p);

		if(e)
		{
			n++;
			add_value(&ms, &b, src, e);
		}

		if(e && e>src)  /* non empty match? */
			src = e;	/* skip it */
		else if(src < ms.src_end)
		{
			const char *old = advance(&src, ms.mb);
			luaL_addlstring(&b, old, src-old);
		}
		else break;

		if(anchor) break;
	}

	luaL_addlstring(&b, src, ms.src_end-src);
	luaL_pushresult(&b);
	lua_pushinteger(L, n);	/* number of substitutions */
	return 2;
}

/* }====================================================== */


/* maximum size of each formatted item (> len(format('%99.99f', -1e308))) */
#define MAX_ITEM	512
/* valid flags in a format specification */
#define FLAGS  "-+ #0"
/*
** maximum size of each format specification (such as '%-099.99d')
** (+10 accounts for %99.99x plus margin of error)
*/
#define MAX_FORMAT (sizeof(FLAGS) + sizeof(LUA_INTFRMLEN) + 10)


static void addquoted(lua_State *L, luaL_Buffer *b, int arg)
{
	size_t l;
	const char *s = luaL_checklstring(L, arg, &l);
	luaL_addchar(b, '"');

	while(l--)
	{
		switch(*s)
		{
			case '"': case '\\': case '\n':
			{
				luaL_addchar(b, '\\');
				luaL_addchar(b, *s);
				break;
			}
			case '\r':
			{
				luaL_addlstring(b, "\\r", 2);
				break;
			}
			case '\0':
			{
				luaL_addlstring(b, "\\000", 4);
				break;
			}
			default:
			{
				luaL_addchar(b, *s);
				break;
			}
		}

		s++;
	}

	luaL_addchar(b, '"');
}


static const char *scanformat(lua_State *L, const char *strfrmt, char *form,
                              int *hasprecision)
{
	const char *p = strfrmt;

	while(strchr(FLAGS, *p)) p++;	/* skip flags */

	if((size_t)(p - strfrmt) >= sizeof(FLAGS))
		luaL_error(L, "invalid format (repeated flags)");

	if(isdigit(uchar(*p))) p++;	/* skip width */

	if(isdigit(uchar(*p))) p++;	/* (2 digits at most) */

	if(*p == '.')
	{
		p++;
		*hasprecision = 1;

		if(isdigit(uchar(*p))) p++;	/* skip precision */

		if(isdigit(uchar(*p))) p++;	/* (2 digits at most) */
	}

	if(isdigit(uchar(*p)))
		luaL_error(L, "invalid format (width or precision too long)");

	form[0] = L_ESC;
	strncpy(form+1, strfrmt, p-strfrmt+1);
	form[p-strfrmt+2] = 0;
	return p;
}

static void addintlen(char *form)
{
	size_t l = strlen(form);
	char spec = form[l - 1];
	strcpy(form + l - 1, LUA_INTFRMLEN);
	form[l + sizeof(LUA_INTFRMLEN) - 2] = spec;
	form[l + sizeof(LUA_INTFRMLEN) - 1] = '\0';
}

static int str_format(lua_State *L)
{
	int arg = 1;
	size_t sfl;
	const char *strfrmt = luaL_checklstring(L, arg, &sfl);
	const char *strfrmt_end = strfrmt+sfl;
	luaL_Buffer b;
	luaL_buffinit(L, &b);

	while(strfrmt < strfrmt_end)
	{
		if(*strfrmt != L_ESC)
			luaL_addchar(&b, *strfrmt++);
		else if(*++strfrmt == L_ESC)
			luaL_addchar(&b, *strfrmt++);	/* %% */
		else   /* format item */
		{
			char form[MAX_FORMAT];	/* to store the format (`%...') */
			char buff[MAX_ITEM];	/* to store the formatted item */
			int hasprecision = 0;
			arg++;
			strfrmt = scanformat(L, strfrmt, form, &hasprecision);

			switch(*strfrmt++)
			{
				case 'c':
				{
#ifdef LUA_USE_SNPRINTF
					snprintf(buff, MAX_ITEM, form,
					         (int) luaL_checknumber(L, arg));
#else
					sprintf(buff, form, (int) luaL_checknumber(L, arg));
#endif
					break;
				}
				case 'd': case 'i':
				{
					addintlen(form);
#ifdef LUA_USE_SNPRINTF
					snprintf(buff, MAX_ITEM, form,
					         (LUA_INTFRM_T) luaL_checknumber(L, arg));
#else
					sprintf(buff, form,
					        (LUA_INTFRM_T) luaL_checknumber(L, arg));
#endif
					break;
				}
				case 'o': case 'u': case 'x': case 'X':
				{
					addintlen(form);
#ifdef LUA_USE_SNPRINTF
					snprintf(buff, MAX_ITEM, form,
					         (unsigned LUA_INTFRM_T) luaL_checknumber(L, arg));
#else
					sprintf(buff, form,
					        (unsigned LUA_INTFRM_T) luaL_checknumber(L, arg));
#endif
					break;
				}
				case 'e': case 'E': case 'f':
				case 'g': case 'G':
				{
#ifndef LUA_NUMBER_DOUBLE
					luaL_argerror(L, 1, "double formatting not supported");
#else
#	ifdef __dietlibc__
#		warning "double formatting is broken in dietlibc"
#	endif
#	ifdef LUA_USE_SNPRINTF
					snprintf(buff, MAX_ITEM, form,
					         (double) luaL_checknumber(L, arg));
#	else
					sprintf(buff, form, (double) luaL_checknumber(L, arg));
#	endif
#endif
					break;
				}
				case 'q':
				{
					addquoted(L, &b, arg);
					continue;	/* skip the `addsize' at the end */
				}
				case 's':
				{
					size_t l;
					const char *s = luaL_checklstring(L, arg, &l);

					if(!hasprecision && l >= 100)
					{
						/* no precision and string is too long to be formatted;
							 keep original string */
						lua_pushvalue(L, arg);
						luaL_addvalue(&b);
						continue;	/* skip the `addsize' at the end */
					}
					else
					{
#ifdef LUA_USE_SNPRINTF
						snprintf(buff, MAX_ITEM, form, s);
#else
						sprintf(buff, form, s);
#endif
						break;
					}
				}
				default:  	/* also treat cases `pnLlh' */
				{
					return luaL_error(L, "invalid option " LUA_QL("%%%c") " to "
					                  LUA_QL("format"), *(strfrmt - 1));
				}
			}

			luaL_addlstring(&b, buff, strlen(buff));
		}
	}

	luaL_pushresult(&b);
	return 1;
}

#ifdef WANT_EXT_MATCH
static struct { const char *k; int v; } unicflags[] =
{
	{ "ASCII", MODE_ASCII }
	,{ "LATIN", MODE_LATIN }
	,{ "UTF8",  MODE_UTF8 }
	,{ "GRAPH", MODE_GRAPH }
};
#define unicflags_sz ( sizeof( unicflags ) / sizeof( unicflags[0] ) )

/*
	allow direkt match calls from c
*/
int ext_uni_match(void *state, const char *s, size_t n,
                  const char *p, int init, int mode)
{
	lua_State *L = state;
	MatchState ms;
	int anchor = (*p == '^') ? (p++, 1) : 0;
	const char *s1;
	int i = posrelat(init, n) - 1;

	if(i < 0) i = 0;
	else if((size_t)(i) > n) i = (ptrdiff_t)n;

	s1 = s + i;
	ms.L = L;
	ms.src_init = s;
	ms.src_end = s + n;
	ms.mode = mode;
	ms.mb = MODE_MBYTE(mode);

	do
	{
		const char *res;
		ms.level = 0;

		if((res=match(&ms, s1, p)) != NULL)
			return 1;
	}
	while(s1++ < ms.src_end && !anchor);

	return 0;
}
#endif

static const luaL_Reg uniclib[] =
{
	{"byte", unic_byte}, /* no cluster ! */
	{"char", unic_char},
	{"dump", str_dump},
	{"find", unic_find}, /* cluster */
	{"format", str_format},
	{"gfind", gfind_nodef},
	{"gmatch", gmatch}, /* cluster */
	{"gsub", unic_gsub}, /* cluster */
	{"len", unic_len}, /* cluster/byte opt. */
	{"lower", unic_lower},
	{"match", unic_match}, /* cluster */
	{"rep", str_rep},
	{"reverse", str_reverse},
	{"sub", unic_sub}, /* cluster/byte opt. */
	{"upper", unic_upper},
	{NULL, NULL}
};

#if defined( SLNUNICODE_AS_STRING ) && defined( STRING_WITH_METAT )
static void createmetatable(lua_State *L)
{
	lua_newtable(L);  /* create metatable for strings */
	lua_pushliteral(L, "");  /* dummy string */
	lua_pushvalue(L, -2);
	lua_setmetatable(L, -2);  /* set string metatable */
	lua_pop(L, 1);  /* pop dummy string */
	lua_pushvalue(L, -2);  /* string library... */
	lua_setfield(L, -2, "__index");  /* ...is the __index metamethod */
	lua_pop(L, 1);  /* pop metatable */
}
#endif

/*
** Register separate library
*/
static void register_lib(lua_State *L, int mode, const char *name)
{
#if LUA_VERSION_NUM == 501
	lua_newtable(L);
	lua_pushinteger(L, mode);
	lua_setfield(L, -2, "mode");
	lua_pushvalue(L, -1);
	lua_replace(L, LUA_ENVIRONINDEX);
	luaL_register(L, name, uniclib);
#elif LUA_VERSION_NUM == 502
	lua_createtable(L, 0, 16);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, name + sizeof(SLN_UNICODENAME));
	lua_pushinteger(L, mode);
	luaL_setfuncs(L, uniclib, 1);
#endif
	lua_pop(L, 1);
}

/*
** Open string library
*/
LUALIB_API int luaopen_unicode(lua_State *L)
{
	/* register unicode itself so require("unicode") works */
#if LUA_VERSION_NUM == 501
	luaL_register(L, SLN_UNICODENAME,
	              uniclib + (sizeof uniclib/sizeof uniclib[0] - 1)); /* empty func list */
#else
	lua_createtable(L, 0, 0);
	lua_pushvalue(L, -1);
	lua_setglobal(L, SLN_UNICODENAME);
	luaL_setfuncs(L, uniclib + (sizeof uniclib/sizeof uniclib[0] - 1), 0); /* empty func list */
#endif
	register_lib(L, MODE_ASCII, SLN_UNICODENAME ".ascii");
#ifdef SLNUNICODE_AS_STRING
#ifdef STRING_WITH_METAT
	createmetatable(L);
#endif
	lua_setglobal(L, "string");
#endif
	register_lib(L, MODE_LATIN, SLN_UNICODENAME ".latin1");
	register_lib(L, MODE_GRAPH, SLN_UNICODENAME ".grapheme");
	register_lib(L, MODE_UTF8, SLN_UNICODENAME ".utf8");
#ifdef WANT_EXT_MATCH
	{
		unsigned i;
		const char *ln = SLN_UNICODENAME ".mode";
		luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", 1);
		lua_getfield(L, -1, ln);

		if(!lua_istable(L, -1))
		{
			lua_pop(L, 1);
#if LUA_VERSION_NUM < 502

			if(luaL_findtable(L, LUA_GLOBALSINDEX, ln, unicflags_sz))
				luaL_error(L, "name conflict for module " LUA_QS, ln);

#else
			lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

			if(luaL_findtable(L, -1, ln, unicflags_sz))
				luaL_error(L, "name conflict for module " LUA_QS, ln);

			lua_remove(L, -2);
#endif
			lua_pushvalue(L, -1);
			lua_setfield(L, -3, ln);
		}

		lua_remove(L, -2);

		for(i = 0; unicflags_sz > i; ++i)
		{
			lua_pushnumber(L, unicflags[i].v);
			lua_setfield(L, -2, unicflags[i].k);
		}
	}
#endif
	return 1;
}
