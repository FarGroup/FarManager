#include "ustring.h"
#include "luafar.h"

extern int IsLuaJIT();

/* Taken from Lua 5.1; modified to work with Unicode filenames. */
/* ------------------------------------------------------------ */
/*
** {======================================================
** Load functions
** =======================================================
*/

typedef struct LoadF
{
	int extraline;
	FILE *f;
	char buff[LUAL_BUFFERSIZE];
} LoadF;


static const char *getF(lua_State *L, void *ud, size_t *size)
{
	LoadF *lf = (LoadF *)ud;
	(void)L;

	if(lf->extraline)
	{
		lf->extraline = 0;
		*size = 1;
		return "\n";
	}

	if(feof(lf->f)) return NULL;

	*size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);
	return (*size > 0) ? lf->buff : NULL;
}


static int errfile(lua_State *L, const char *what, int fnameindex)
{
	const char *serr = strerror(errno);
	const char *filename = lua_tostring(L, fnameindex) + 1;
	lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
	lua_remove(L, fnameindex);
	return LUA_ERRFILE;
}

static int skipBOM (FILE *f) {
	const char *p = "\xEF\xBB\xBF";  /* Utf8 BOM mark */
	int c = getc(f);
	if (c != *(unsigned char *)p++)
		return c;
	do
	{
		if (getc(f) != *(unsigned char *)p++)
		{
			rewind(f);
			break;
		}
	} while (*p != '\0');
	return getc(f);  /* return next character */
}

int LF_LoadFile(lua_State *L, const wchar_t *filename)
{
	LoadF lf;
	int status, readstatus;
	int c;
	int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
	lf.extraline = 0;

	if(filename == NULL)
	{
		lua_pushliteral(L, "=stdin");
		lf.f = stdin;
	}
	else
	{
		//lua_pushfstring(L, "@%s", filename);
		lua_pushliteral(L, "@");
		push_utf8_string(L, filename, -1);
		lua_concat(L, 2);
		lf.f = _wfopen(filename, L"r");

		if(lf.f == NULL) return errfile(L, "open", fnameindex);
	}

	c = IsLuaJIT() ? getc(lf.f) : skipBOM(lf.f);

	if(c == '#')     /* Unix exec. file? */
	{
		lf.extraline = 1;

		while((c = getc(lf.f)) != EOF && c != '\n') ;   /* skip first line */

		if(c == '\n') c = getc(lf.f);
	}

	if(c == LUA_SIGNATURE[0] && filename)     /* binary file? */
	{
		lf.f = _wfreopen(filename, L"rb", lf.f);  /* reopen in binary mode */

		if(lf.f == NULL) return errfile(L, "reopen", fnameindex);

		/* skip eventual `#!...' */
		while((c = getc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;

		lf.extraline = 0;
	}

	ungetc(c, lf.f);
#if LUA_VERSION_NUM == 501
	status = lua_load(L, getF, &lf, lua_tostring(L, -1));
#else
	status = lua_load(L, getF, &lf, lua_tostring(L, -1), NULL);
#endif
	readstatus = ferror(lf.f);

	if(filename) fclose(lf.f);   /* close file (even in case of errors) */

	if(readstatus)
	{
		lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
		return errfile(L, "read", fnameindex);
	}

	lua_remove(L, fnameindex);
	return status;
}

/* }====================================================== */

// Taken from Lua 5.1
static int load_aux(lua_State *L, int status)
{
	if(status == 0)   /* OK? */
		return 1;
	else
	{
		lua_pushnil(L);
		lua_insert(L, -2);  /* put before error message */
		return 2;  /* return nil plus error message */
	}
}

// Taken from Lua 5.1 and modified
int luaB_loadfileW(lua_State *L)
{
	const wchar_t *fname = opt_utf8_string(L, 1, NULL);
	return load_aux(L, LF_LoadFile(L, fname));
}

// Taken from Lua 5.1 and modified
int luaB_dofileW (lua_State *L) {
  const wchar_t *fname = opt_utf8_string(L, 1, NULL);
  int n = lua_gettop(L);
  if (LF_LoadFile(L, fname) != 0) lua_error(L);
  lua_call(L, 0, LUA_MULTRET);
  return lua_gettop(L) - n;
}

// Taken from Lua 5.1 (luaL_gsub) and modified
const wchar_t *LF_Gsub(lua_State *L, const wchar_t *s, const wchar_t *p,
                       const wchar_t *r)
{
	const wchar_t *wild;
	size_t l = wcslen(p);
	size_t l2 = sizeof(wchar_t) * wcslen(r);
	luaL_Buffer b;
	luaL_buffinit(L, &b);

	while((wild = wcsstr(s, p)) != NULL)
	{
		luaL_addlstring(&b, (const char*)s, sizeof(wchar_t) *(wild - s));   /* push prefix */
		luaL_addlstring(&b, (const char*)r, l2);  /* push replacement in place of pattern */
		s = wild + l;  /* continue after `p' */
	}

	luaL_addlstring(&b, (const char*)s, sizeof(wchar_t) * wcslen(s));  /* push last suffix */
	luaL_addlstring(&b, "\0\0", 2);  /* push L'\0' */
	luaL_pushresult(&b);
	return (const wchar_t*) lua_tostring(L, -1);
}

