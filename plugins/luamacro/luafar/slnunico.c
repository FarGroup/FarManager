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

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "ustring.h"

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

/* macro to `unsign' a character */
#define uchar(c)				((unsigned char)(c))


/* This function taken from:
 * Quylthulg Copyright (C) 2009 Kein-Hong Man <keinhong@gmail.com>
 */
static int grab_one_utf8(const char **ps)
{
	unsigned char v, ext;
	int c, min;
	v = (unsigned char)*((*ps)++);

	if (v <= 0x7F)
	{
		ext = 0; c = v; min = 0;
	}
	else if (v <= 0xC1)
	{
		return -1;              /* bad sequence */
	}
	else if (v <= 0xDF)
	{
		ext = 1; c = v & 0x1F; min = 0x80;
	}
	else if (v <= 0xEF)
	{
		ext = 2; c = v & 0x0F; min = 0x800;
	}
	else if (v <= 0xF4)
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

		if (v < 0x80 || v > 0xBF)
		{
			return -1;          /* bad sequence */
		}

		c = (c << 6) | (v & 0x3F);
	}

	if (c < min)
	{
		return -1;              /* bad sequence */
	}

	return c;
}


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"

/* }====================================================== */


/* maximum size of each formatted item (> len(format('%999.99f', -1e308))) */
#define MAX_ITEM	1100
/* valid flags in a format specification */
#define FLAGS  "-+ #0"
/*
** maximum size of each format specification (such as '%-099.99d')
** (+11 accounts for %999.99x plus margin of error)
*/
#define MAX_FORMAT (sizeof(FLAGS) + sizeof(LUA_INTFRMLEN) + 11)


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

	if ((size_t)(p - strfrmt) >= sizeof(FLAGS))
		luaL_error(L, "invalid format (repeated flags)");

	if (isdigit(uchar(*p))) p++;	/* skip width */
	if (isdigit(uchar(*p))) p++;
	if (isdigit(uchar(*p))) p++;	/* (3 digits at most) */

	if (*p == '.')
	{
		p++;
		*hasprecision = 1;

		if (isdigit(uchar(*p))) p++;	/* skip precision */

		if (isdigit(uchar(*p))) p++;	/* (2 digits at most) */
	}

	if (isdigit(uchar(*p)))
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
		if (*strfrmt != L_ESC)
			luaL_addchar(&b, *strfrmt++);
		else if (*++strfrmt == L_ESC)
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
					wchar_t *s = check_utf8_string(L, arg, &l);

					if (!hasprecision && l >= 1000)
					{
						/* no precision and string is too long to be formatted;
							 keep original string */
						push_utf8_string(L, s, l);
						luaL_addvalue(&b);
						continue;	/* skip the `addsize' at the end */
					}
					else
					{
						wchar_t buffW[MAX_ITEM];	/* to store the formatted item */
						lua_pushstring(L, form);
						_snwprintf(buffW, MAX_ITEM, check_utf8_string(L,-1,NULL), s);
						buffW[MAX_ITEM-1] = L'\0';
						lua_pop(L, 1);
						push_utf8_string(L, buffW, -1);
						luaL_addvalue(&b);
						continue;
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

static int utf8_valid(lua_State *L)
{
	size_t len, utf8len=0;
	int result=1;
	const char *s = luaL_checklstring(L, 1, &len);
	const char *t = s + len;
	while (s < t)
	{
		if (grab_one_utf8(&s) == -1)
		{
			result=0; break;
		}
		++utf8len;
	}
	lua_pushboolean(L, result);
	lua_pushinteger(L, utf8len);
	return 2;
}


static const luaL_Reg uniclib[] =
{
	{"format",    str_format},
	{"utf8valid", utf8_valid},
	{NULL, NULL}
};

/*
** Open library
*/
LUALIB_API int luaopen_unicode(lua_State *L)
{
	luaL_register(L, "utf8", uniclib);
	return 0;
}
