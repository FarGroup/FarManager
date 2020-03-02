/* lregex.cpp */

#include "luafar.h"
#include "ustring.h"
#include "compat52.h"

#define TYPE_REGEX "far_regex"

enum { OP_FIND, OP_MATCH, OP_EXEC };

typedef struct
{
	HANDLE hnd;
} TFarRegex;

FARAPIREGEXPCONTROL GetRegExpControl(lua_State *L)
{
	return GetPluginData(L)->Info->RegExpControl;
}

TFarRegex* CheckFarRegex(lua_State *L, int pos)
{
	TFarRegex* fr = (TFarRegex*)luaL_checkudata(L, pos, TYPE_REGEX);
	luaL_argcheck(L, fr->hnd != INVALID_HANDLE_VALUE, pos, "attempt to access freed regex");
	return fr;
}

int method_gc(lua_State *L)
{
	TFarRegex* fr = CheckFarRegex(L, 1);

	if(fr->hnd != INVALID_HANDLE_VALUE)
	{
		FARAPIREGEXPCONTROL RegExpControl = GetRegExpControl(L);
		RegExpControl(fr->hnd, RECTL_FREE, 0, 0);
		fr->hnd = INVALID_HANDLE_VALUE;
	}

	return 0;
}

int method_tostring(lua_State *L)
{
	TFarRegex* fr = CheckFarRegex(L, 1);
	lua_pushfstring(L, "%s (%p)", TYPE_REGEX, fr);
	return 1;
}

const wchar_t* check_regex_pattern(lua_State *L, int pos_pat, int pos_cflags)
{
	const char* pat = luaL_checkstring(L, pos_pat);

	if(*pat != '/')
	{
		const char* cflags = pos_cflags ? luaL_optstring(L, pos_cflags, NULL) : NULL;
		lua_pushliteral(L, "/");
		lua_pushvalue(L, pos_pat);
		lua_pushliteral(L, "/");

		if(cflags) lua_pushvalue(L, pos_cflags);

		lua_concat(L, 3 + (cflags?1:0));
		lua_replace(L, pos_pat);
	}

	return check_utf8_string(L, pos_pat, NULL);
}

TFarRegex* push_far_regex(lua_State *L, FARAPIREGEXPCONTROL RegExpControl, const wchar_t* pat)
{
	TFarRegex* fr = (TFarRegex*)lua_newuserdata(L, sizeof(TFarRegex));

	if(!RegExpControl(NULL, RECTL_CREATE, 0, &fr->hnd))
		luaL_error(L, "RECTL_CREATE failed");

	if(!RegExpControl(fr->hnd, RECTL_COMPILE, 0, (void*)pat))
		luaL_error(L, "invalid regular expression");

//(void)RegExpControl(fr->hnd, RECTL_OPTIMIZE, 0, 0); // very slow operation
	luaL_getmetatable(L, TYPE_REGEX);
	lua_setmetatable(L, -2);
	return fr;
}

int _regex_gmatch_closure(lua_State *L, int is_wide)
{
	TFarRegex* fr = (TFarRegex*)lua_touserdata(L, lua_upvalueindex(1));
	struct RegExpSearch* pData = (struct RegExpSearch*)lua_touserdata(L, lua_upvalueindex(2));
	FARAPIREGEXPCONTROL RegExpControl = GetRegExpControl(L);

	if((pData->Position <= pData->Length) && RegExpControl(fr->hnd, RECTL_SEARCHEX, 0, pData))
	{
		int i, skip = pData->Count>1 ? 1 : 0;

		for(i=skip; i<pData->Count; i++)
		{
			if(pData->Match[i].start >= 0 && pData->Match[i].end >= pData->Match[i].start)
			{
				if(is_wide)
					push_utf16_string(L, pData->Text+pData->Match[i].start, pData->Match[i].end-pData->Match[i].start);
				else
					push_utf8_string(L, pData->Text+pData->Match[i].start, pData->Match[i].end-pData->Match[i].start);
			}
			else
				lua_pushboolean(L, 0);
		}

		if(pData->Position < pData->Match[0].end)
			pData->Position = pData->Match[0].end;
		else
			pData->Position++;

		return (int)pData->Count - skip;
	}

	return lua_pushnil(L), 1;
}

int regex_gmatch_closure(lua_State *L) { return _regex_gmatch_closure(L, 0); }
int regex_gmatch_closureW(lua_State *L) { return _regex_gmatch_closure(L, 1); }


int _Gmatch(lua_State *L, int is_wide)
{
	size_t len;
	const wchar_t* Text = is_wide ? check_utf16_string(L, 1, &len) : check_utf8_string(L, 1, &len);
	const wchar_t* pat = check_regex_pattern(L, 2, 3);
	FARAPIREGEXPCONTROL RegExpControl = GetRegExpControl(L);
	TFarRegex* fr = push_far_regex(L, RegExpControl, pat); // upvalue 1
	struct RegExpSearch* pData = (struct RegExpSearch*)lua_newuserdata(L, sizeof(struct RegExpSearch)); // upvalue 2
	memset(pData, 0, sizeof(struct RegExpSearch));
	pData->Text = Text;
	pData->Position = 0;
	pData->Length = len;
	pData->Count = RegExpControl(fr->hnd, RECTL_BRACKETSCOUNT, 0, 0);
	/* upvalues 3 and 4 must be kept to prevent values from being garbage-collected */
	pData->Match = (struct RegExpMatch*)lua_newuserdata(L, pData->Count*sizeof(struct RegExpMatch)); // upvalue 3
	pData->Match[0].end = -1;
	lua_pushvalue(L, 1); // upvalue 4
	lua_pushcclosure(L, is_wide ? regex_gmatch_closureW : regex_gmatch_closure, 4);
	return 1;
}

int func_Gmatch(lua_State *L) { return _Gmatch(L, 0); }
int func_GmatchW(lua_State *L) { return _Gmatch(L, 1); }

int rx_find_match(lua_State *L, int operation, int is_function, int is_wide)
{
	size_t len;
	FARAPIREGEXPCONTROL RegExpControl = GetRegExpControl(L);
	TFarRegex* fr;
	struct RegExpSearch data;
	memset(&data, 0, sizeof(data));

	if(is_function)
	{
		if(is_wide)
			data.Text = check_utf16_string(L, 1, &len);
		else
			data.Text = check_utf8_string(L, 1, &len);

		fr = push_far_regex(L, RegExpControl, check_regex_pattern(L, 2, 4));
		lua_replace(L, 2);
	}
	else
	{
		fr = CheckFarRegex(L, 1);

		if(is_wide)
			data.Text = check_utf16_string(L, 2, &len);
		else
			data.Text = check_utf8_string(L, 2, &len);
	}

	data.Length = len;
	data.Position = luaL_optinteger(L, 3, 1);

	if(data.Position > 0 && --data.Position > data.Length)
		data.Position = data.Length;

	if(data.Position < 0 && (data.Position += data.Length) < 0)
		data.Position = 0;

	data.Count = RegExpControl(fr->hnd, RECTL_BRACKETSCOUNT, 0, 0);
	data.Match = (struct RegExpMatch*)lua_newuserdata(L, data.Count*sizeof(struct RegExpMatch));

	if(RegExpControl(fr->hnd, RECTL_SEARCHEX, 0, &data))
	{
		int i;
		int skip = (operation == OP_FIND || operation == OP_EXEC || data.Count>1) ? 1 : 0;

		if(operation == OP_FIND || operation == OP_EXEC)
		{
			lua_pushinteger(L, data.Match[0].start+1);
			lua_pushinteger(L, data.Match[0].end);
		}
		if (operation == OP_EXEC)
		{
			lua_createtable(L, 2*(int)data.Count, 0);
			for(i=skip; i<data.Count; i++)
			{
				int k = (i-skip)*2 + 1;
				if(data.Match[i].start >= 0 && data.Match[i].end >= data.Match[i].start)
				{
					lua_pushinteger(L, data.Match[i].start+1);
					lua_rawseti(L, -2, k);
					lua_pushinteger(L, data.Match[i].end);
					lua_rawseti(L, -2, k+1);
				}
				else
				{
					lua_pushboolean(L, 0);
					lua_rawseti(L, -2, k);
					lua_pushboolean(L, 0);
					lua_rawseti(L, -2, k+1);
				}
			}
		}
		else
		{
			i = (int)data.Count - skip + 1;
			if (!lua_checkstack(L, i))
				luaL_error(L, "cannot add %d stack slots", i);
			for(i=skip; i<data.Count; i++)
			{
				if(data.Match[i].start >= 0 && data.Match[i].end >= data.Match[i].start)
				{
					if(is_wide)
						push_utf16_string(L, data.Text+data.Match[i].start, data.Match[i].end-data.Match[i].start);
					else
						push_utf8_string(L, data.Text+data.Match[i].start, data.Match[i].end-data.Match[i].start);
				}
				else
					lua_pushboolean(L, 0);
			}
		}
		switch (operation)
		{
			case OP_FIND:  return 2 + (int)data.Count - skip;
			case OP_MATCH: return 0 + (int)data.Count - skip;
			case OP_EXEC:  return 3;
			default:       return 0;
		}
	}

	return lua_pushnil(L), 1;
}

int method_bracketscount(lua_State *L)
{
	TFarRegex* fr = CheckFarRegex(L, 1);
	FARAPIREGEXPCONTROL RegExpControl = GetRegExpControl(L);
	lua_pushinteger(L, RegExpControl(fr->hnd, RECTL_BRACKETSCOUNT, 0, 0));
	return 1;
}

int rx_gsub(lua_State *L, int is_function, int is_wide)
{
	size_t len, flen;
	TFarRegex* fr;
	FARAPIREGEXPCONTROL RegExpControl = GetRegExpControl(L);
	const wchar_t *s, *f;
	int max_rep_capture, ftype, n, matches, reps;
	luaL_Buffer out;
	struct RegExpSearch data;
	memset(&data, 0, sizeof(data));

	if(is_function)
	{
		if(is_wide)
			data.Text = check_utf16_string(L, 1, &len);
		else
			data.Text = check_utf8_string(L, 1, &len);

		fr = push_far_regex(L, RegExpControl, check_regex_pattern(L, 2, 5));
		lua_replace(L, 2);
	}
	else
	{
		fr = CheckFarRegex(L, 1);

		if(is_wide)
			data.Text = check_utf16_string(L, 2, &len);
		else
			data.Text = check_utf8_string(L, 2, &len);
	}

	data.Length = len;
	s = data.Text;
	f = NULL;
	flen = 0;
	max_rep_capture = 0;
	ftype = lua_type(L, 3);

	if(ftype == LUA_TSTRING)
	{
		const wchar_t* p;
		f = check_utf8_string(L, 3, &flen);

		for(p=f; *p; p++)
		{
			if(*p == L'%')
			{
				int n, ch;

				if((ch = *++p) == 0) break;

				n = (ch >= L'0' && ch <= L'9') ? ch - L'0' :
				    (ch >= L'A' && ch <= L'Z') ? ch - L'A' + 10 :
				    (ch >= L'a' && ch <= L'z') ? ch - L'a' + 10 : -1;

				if(max_rep_capture < n) max_rep_capture = n;
			}
		}
	}
	else if(ftype != LUA_TTABLE && ftype != LUA_TFUNCTION)
		luaL_argerror(L, 3, "string or table or function");

	if(lua_isnoneornil(L, 4)) n = -1;
	else
	{
		n = (int)luaL_checkinteger(L, 4);

		if(n < 0) n = 0;
	}

	lua_settop(L, 3);
	data.Count = RegExpControl(fr->hnd, RECTL_BRACKETSCOUNT, 0, 0);

	if((ftype == LUA_TSTRING) &&
	        !(max_rep_capture == 1 && data.Count == 1) &&
	        (data.Count <= max_rep_capture))
		luaL_error(L, "replace string: invalid capture index");

	data.Match = (struct RegExpMatch*)lua_newuserdata(L, data.Count*sizeof(struct RegExpMatch));
	data.Match[0].end = -1;
	matches = reps = 0;
	luaL_buffinit(L, &out);

	while(n < 0 || reps < n)
	{
		int rep;
		intptr_t from, to;
		intptr_t prev_end = data.Match[0].end;

		if(!RegExpControl(fr->hnd, RECTL_SEARCHEX, 0, &data))
			break;

		if(data.Match[0].end == prev_end)
		{
			if(data.Position < data.Length)
			{
				luaL_addlstring(&out, (const char*)(s+data.Position), sizeof(wchar_t));
				data.Position++;
				continue;
			}

			break;
		}

		matches++;
		rep = 0;
		from = data.Match[0].start;
		to = data.Match[0].end;
		luaL_addlstring(&out, (const char*)(s + data.Position),
		                (from - data.Position) * sizeof(wchar_t));

		if(ftype == LUA_TSTRING)
		{
			size_t i, start = 0;

			for(i=0; i<flen; i++)
			{
				if(f[i] == L'%')
				{
					if(++i < flen)
					{
						int ch = f[i];
						int n = (ch >= L'0' && ch <= L'9') ? ch - L'0' :
						        (ch >= L'A' && ch <= L'Z') ? ch - L'A' + 10 :
						        (ch >= L'a' && ch <= L'z') ? ch - L'a' + 10 : -1;

						if(n >= 0)
						{
							if(n==1 && data.Count==1) n = 0;

							luaL_addlstring(&out, (const char*)(f+start), (i-1-start)*sizeof(wchar_t));

							if(data.Match[n].start >= 0)
							{
								luaL_addlstring(&out, (const char*)(s + data.Match[n].start),
								                (data.Match[n].end - data.Match[n].start) * sizeof(wchar_t));
							}
						}
						else   // delete the percent sign
						{
							luaL_addlstring(&out, (const char*)(f+start), (i-1-start)*sizeof(wchar_t));
							luaL_addlstring(&out, (const char*)(f+i), sizeof(wchar_t));
						}

						start = i+1;
					}
					else
					{
						luaL_addlstring(&out, (const char*)(f+start), (i-1-start)*sizeof(wchar_t));
						start = flen;
						break;
					}
				}
			}

			rep++;
			luaL_addlstring(&out, (const char*)(f+start), (flen-start)*sizeof(wchar_t));
		}
		else if(ftype == LUA_TTABLE)
		{
			int n = data.Count==1 ? 0:1;

			if(data.Match[n].start >= 0)
			{
				if(is_wide)
					push_utf16_string(L, s + data.Match[n].start, (data.Match[n].end - data.Match[n].start));
				else
					push_utf8_string(L, s + data.Match[n].start, (data.Match[n].end - data.Match[n].start));

				lua_gettable(L, 3);

				if(lua_isstring(L, -1))
				{
					if(!is_wide)
					{
						size_t len;
						const wchar_t* ws = check_utf8_string(L, -1, &len);
						lua_pushlstring(L, (const char*)ws, len*sizeof(wchar_t));
						lua_remove(L, -2);
					}

					luaL_addvalue(&out);
					rep++;
				}
				else if(lua_toboolean(L,-1))
					luaL_error(L, "invalid replacement type");
				else
					lua_pop(L, 1);
			}
		}
		else   // if (ftype == LUA_TFUNCTION)
		{
			intptr_t i, skip = data.Count==1 ? 0:1;
			lua_checkstack(L, (int)(data.Count+1-skip));
			lua_pushvalue(L, 3);

			for(i=skip; i<data.Count; i++)
			{
				if(data.Match[i].start >= 0 && data.Match[i].end >= data.Match[i].start)
				{
					if(is_wide)
						push_utf16_string(L, s + data.Match[i].start, (data.Match[i].end - data.Match[i].start));
					else
						push_utf8_string(L, s + data.Match[i].start, (data.Match[i].end - data.Match[i].start));
				}
				else
					lua_pushboolean(L, 0);
			}

			if(lua_pcall(L, (int)(data.Count-skip), 1, 0) == 0)
			{
				if(lua_isstring(L, -1))
				{
					if(!is_wide)
					{
						size_t len;
						const wchar_t* ws = check_utf8_string(L, -1, &len);
						lua_pushlstring(L, (const char*)ws, len*sizeof(wchar_t));
						lua_remove(L, -2);
					}

					luaL_addvalue(&out);
					rep++;
				}
				else if(lua_toboolean(L,-1))
					luaL_error(L, "invalid return type");
				else
					lua_pop(L, 1);
			}
			else
				luaL_error(L, lua_tostring(L, -1));
		}

		if(rep)
			reps++;
		else
			luaL_addlstring(&out, (const char*)(s+from), (to-from)*sizeof(wchar_t));

		if(data.Position < to)
			data.Position = to;
		else if(data.Position < data.Length)
		{
			luaL_addlstring(&out, (const char*)(s + data.Position), sizeof(wchar_t));
			data.Position++;
		}
		else
			break;
	}

	luaL_addlstring(&out, (const char*)(s + data.Position),
	                (data.Length - data.Position) * sizeof(wchar_t));
	luaL_pushresult(&out);

	if(!is_wide)
	{
		push_utf8_string(L, (const wchar_t*)lua_tostring(L, -1), lua_objlen(L, -1) / sizeof(wchar_t));
	}

	lua_pushinteger(L, matches);
	lua_pushinteger(L, reps);
	return 3;
}

int func_New(lua_State *L)
{
	const wchar_t* pat = check_regex_pattern(L, 1, 2);
	push_far_regex(L, GetRegExpControl(L), pat);
	return 1;
}

int method_find(lua_State *L)  { return rx_find_match(L, OP_FIND, 0, 0); }
int func_find(lua_State *L)  { return rx_find_match(L, OP_FIND, 1, 0); }
int method_findW(lua_State *L)  { return rx_find_match(L, OP_FIND, 0, 1); }
int func_findW(lua_State *L)  { return rx_find_match(L, OP_FIND, 1, 1); }

int method_match(lua_State *L)  { return rx_find_match(L, OP_MATCH, 0, 0); }
int func_match(lua_State *L)  { return rx_find_match(L, OP_MATCH, 1, 0); }
int method_matchW(lua_State *L)  { return rx_find_match(L, OP_MATCH, 0, 1); }
int func_matchW(lua_State *L)  { return rx_find_match(L, OP_MATCH, 1, 1); }

int method_exec(lua_State *L)  { return rx_find_match(L, OP_EXEC, 0, 0); }
int func_exec(lua_State *L)  { return rx_find_match(L, OP_EXEC, 1, 0); }
int method_execW(lua_State *L)  { return rx_find_match(L, OP_EXEC, 0, 1); }
int func_execW(lua_State *L)  { return rx_find_match(L, OP_EXEC, 1, 1); }

int method_gsub(lua_State *L)  { return rx_gsub(L, 0, 0); }
int func_gsub(lua_State *L)  { return rx_gsub(L, 1, 0); }
int method_gsubW(lua_State *L)  { return rx_gsub(L, 0, 1); }
int func_gsubW(lua_State *L)  { return rx_gsub(L, 1, 1); }

const luaL_Reg regex_methods[] =
{
	{"find",          method_find},
	{"gsub",          method_gsub},
	{"match",         method_match},
	{"exec",          method_exec},

	{"findW",         method_findW},
	{"gsubW",         method_gsubW},
	{"matchW",        method_matchW},
	{"execW",         method_execW},

	{"bracketscount", method_bracketscount},
	{"__gc",          method_gc},
	{"__tostring",    method_tostring},
	{NULL, NULL}
};

const luaL_Reg regex_functions[] =
{
	{"new",           func_New},

	{"find",          func_find},
	{"gmatch",        func_Gmatch},
	{"gsub",          func_gsub},
	{"match",         func_match},
	{"exec",          func_exec},

	{"findW",         func_findW},
	{"gmatchW",       func_GmatchW},
	{"gsubW",         func_gsubW},
	{"matchW",        func_matchW},
	{"execW",         func_execW},
	{NULL, NULL}
};

int luaopen_regex(lua_State *L)
{
	const char *libname;
	luaL_newmetatable(L, TYPE_REGEX);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, regex_methods);
	libname = lua_isstring(L, 1) ? lua_tostring(L, 1) : "regex";
	luaL_register(L, libname, regex_functions);
	return 1;
}
