// lf_editor.c

#include <windows.h>

#include <lua.h>
#include <lauxlib.h>

#include "lf_flags.h"
#include "lf_luafar.h"
#include "lf_service.h"
#include "lf_string.h"
#include "lf_util.h"

void PushEditorSetPosition(lua_State *L, const struct EditorSetPosition *esp)
{
	lua_createtable(L, 0, 6);
	PutIntToTable(L, "CurLine",       esp->CurLine + 1);
	PutIntToTable(L, "CurPos",        esp->CurPos + 1);
	PutIntToTable(L, "CurTabPos",     esp->CurTabPos + 1);
	PutIntToTable(L, "TopScreenLine", esp->TopScreenLine + 1);
	PutIntToTable(L, "LeftPos",       esp->LeftPos + 1);
	PutIntToTable(L, "Overtype",      esp->Overtype);
}

void FillEditorSetPosition(lua_State *L, struct EditorSetPosition *esp)
{
	esp->CurLine   = GetOptIntFromTable(L, "CurLine", 0) - 1;
	esp->CurPos    = GetOptIntFromTable(L, "CurPos", 0) - 1;
	esp->CurTabPos = GetOptIntFromTable(L, "CurTabPos", 0) - 1;
	esp->TopScreenLine = GetOptIntFromTable(L, "TopScreenLine", 0) - 1;
	esp->LeftPos   = GetOptIntFromTable(L, "LeftPos", 0) - 1;
	esp->Overtype  = GetOptIntFromTable(L, "Overtype", -1);
}

static int push_editor_filename(lua_State *L, intptr_t Id)
{
	PSInfo *Info = GetPluginData(L)->Info;
	size_t size = Info->EditorControl(Id, ECTL_GETFILENAME, 0, 0);

	if (!size) return 0;

	wchar_t* fname = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));
	size = Info->EditorControl(Id, ECTL_GETFILENAME, size, fname);

	if (size)
	{
		push_utf8_string(L, fname, -1);
		lua_remove(L, -2);
		return 1;
	}

	lua_pop(L,1);
	return 0;
}

static int editor_GetFileName(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);

	if (!push_editor_filename(L, EditorId)) lua_pushnil(L);

	return 1;
}

static int editor_GetInfo(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorInfo ei = { sizeof(ei) };

	if (!Info->EditorControl(EditorId, ECTL_GETINFO, 0, &ei))
		return lua_pushnil(L), 1;

	lua_createtable(L, 0, 18);
	PutNumToTable(L, "EditorID", (double)ei.EditorID);

	if (push_editor_filename(L, EditorId))
		lua_setfield(L, -2, "FileName");

	PutNumToTable(L, "WindowSizeX", (double) ei.WindowSizeX);
	PutNumToTable(L, "WindowSizeY", (double) ei.WindowSizeY);
	PutNumToTable(L, "TotalLines", (double) ei.TotalLines);
	PutNumToTable(L, "CurLine", (double) ei.CurLine + 1);
	PutNumToTable(L, "CurPos", (double) ei.CurPos + 1);
	PutNumToTable(L, "CurTabPos", (double) ei.CurTabPos + 1);
	PutNumToTable(L, "TopScreenLine", (double) ei.TopScreenLine + 1);
	PutNumToTable(L, "LeftPos", (double) ei.LeftPos + 1);
	PutNumToTable(L, "Overtype", (double) ei.Overtype);
	PutNumToTable(L, "BlockType", (double) ei.BlockType);
	PutNumToTable(L, "BlockStartLine", (double) ei.BlockStartLine + 1);
	PutNumToTable(L, "Options", (double) ei.Options);
	PutNumToTable(L, "TabSize", (double) ei.TabSize);
	PutNumToTable(L, "BookmarkCount", (double) ei.BookmarkCount);
	PutNumToTable(L, "SessionBookmarkCount", (double) ei.SessionBookmarkCount);
	PutNumToTable(L, "CurState", (double) ei.CurState);
	PutNumToTable(L, "CodePage", (double) ei.CodePage);
	PutRECTToTable(L, "WindowArea", ei.WindowArea);
	PutRECTToTable(L, "ClientArea", ei.ClientArea);
	return 1;
}

/* t-rex:
 * Для тех кому плохо доходит описываю:
 * Редактор в фаре это двух связный список, указатель на текущюю строку
 * изменяется только при ECTL_SETPOSITION, при использовании любой другой
 * ECTL_* для которой нужно задавать номер строки если этот номер не -1
 * (т.е. текущаая строка) то фар должен найти эту строку в списке (а это
 * занимает дофига времени), поэтому если надо делать несколько ECTL_*
 * (тем более когда они делаются на последовательность строк
 * i,i+1,i+2,...) то перед каждым ECTL_* надо делать ECTL_SETPOSITION а
 * сами ECTL_* вызывать с -1.
 */
static BOOL FastGetString(intptr_t EditorId, intptr_t string_num,
                          struct EditorGetString *egs, PSInfo *Info)
{
	struct EditorSetPosition esp = { sizeof(esp) };
	esp.CurLine   = string_num;
	esp.CurPos    = -1;
	esp.CurTabPos = -1;
	esp.TopScreenLine = -1;
	esp.LeftPos   = -1;
	esp.Overtype  = -1;

	if (!Info->EditorControl(EditorId, ECTL_SETPOSITION, 0, &esp))
		return FALSE;

	egs->StringNumber = string_num;
	return Info->EditorControl(EditorId, ECTL_GETSTRING, 0, egs) != 0;
}

// EditorGetString (EditorId, line_num, [mode])
//
//   line_num:  number of line in the Editor, a 1-based integer.
//
//   mode:      0 = returns: table LineInfo;        changes current position: no
//              1 = returns: table LineInfo;        changes current position: yes
//              2 = returns: StringText,StringEOL;  changes current position: yes
//              3 = returns: StringText,StringEOL;  changes current position: no
//
//   return:    either table LineInfo or StringText,StringEOL - depending on `mode` argument.
//
static int _EditorGetString(lua_State *L, int is_wide)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t line_num = luaL_optinteger(L, 2, 0) - 1;
	intptr_t mode = luaL_optinteger(L, 3, 0);
	BOOL res = 0;
	struct EditorGetString egs = { sizeof(egs) };

	if (mode == 0 || mode == 3 || mode == 4)
	{
		egs.StringNumber = line_num;
		res = Info->EditorControl(EditorId, ECTL_GETSTRING, 0, &egs) != 0;
	}
	else if (mode == 1 || mode == 2)
		res = FastGetString(EditorId, line_num, &egs, Info);

	if (res)
	{
		if (mode == 2 || mode == 3)
		{
			if (is_wide)
			{
				push_utf16_string(L, egs.StringText, egs.StringLength);
				push_utf16_string(L, egs.StringEOL, -1);
			}
			else
			{
				push_utf8_string(L, egs.StringText, egs.StringLength);
				push_utf8_string(L, egs.StringEOL, -1);
			}

			return 2;
		}
		else if (mode == 4)
		{
			lua_pushinteger(L, egs.SelStart+1);
			lua_pushinteger(L, egs.SelEnd);
			lua_pushinteger(L, egs.StringLength);
			return 3;
		}
		else
		{
			lua_createtable(L, 0, 6);
			PutNumToTable(L, "StringNumber", (double)egs.StringNumber+1);
			PutNumToTable(L, "StringLength", (double)egs.StringLength);
			PutNumToTable(L, "SelStart", (double)egs.SelStart+1);
			PutNumToTable(L, "SelEnd", (double)egs.SelEnd);

			if (is_wide)
			{
				push_utf16_string(L, egs.StringText, egs.StringLength);
				lua_setfield(L, -2, "StringText");
				push_utf16_string(L, egs.StringEOL, -1);
				lua_setfield(L, -2, "StringEOL");
			}
			else
			{
				PutWStrToTable(L, "StringText",  egs.StringText, egs.StringLength);
				PutWStrToTable(L, "StringEOL",   egs.StringEOL, -1);
			}
		}

		return 1;
	}

	return lua_pushnil(L), 1;
}

static int editor_GetString(lua_State *L) { return _EditorGetString(L, 0); }
static int editor_GetStringW(lua_State *L) { return _EditorGetString(L, 1); }

static int _EditorSetString(lua_State *L, int is_wide)
{
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSetString ess = { sizeof(ess) };
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	size_t len;
	ess.StringNumber = luaL_optinteger(L, 2, 0) - 1;

	if (is_wide)
	{
		ess.StringText = check_utf16_string(L, 3, &len);
		ess.StringEOL = opt_utf16_string(L, 4, NULL);

		if (ess.StringEOL)
		{
			lua_pushvalue(L, 4);
			lua_pushliteral(L, "\0\0");
			lua_concat(L, 2);
			ess.StringEOL = (wchar_t*) lua_tostring(L, -1);
		}
	}
	else
	{
		ess.StringText = check_utf8_string(L, 3, &len);
		ess.StringEOL = opt_utf8_string(L, 4, NULL);
	}

	ess.StringLength = len;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_SETSTRING, 0, &ess) != 0);
	return 1;
}

static int editor_SetString(lua_State *L) { return _EditorSetString(L, 0); }
static int editor_SetStringW(lua_State *L) { return _EditorSetString(L, 1); }

static int editor_InsertString(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	int indent = lua_toboolean(L, 2);
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_INSERTSTRING, 0, &indent) != 0);
	return 1;
}

static int editor_DeleteString(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETESTRING, 0, 0) != 0);
	return 1;
}

static int _EditorInsertText(lua_State *L, int is_wide)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t* text;
	if (is_wide)
	{
		size_t len;
		const char *s = luaL_checklstring(L,2,&len);
		int needZero = 0;
		if (len % sizeof(wchar_t))
		{
			if (s[len-1] && --len)
				needZero = 1;
		}
		else
			needZero = len && (s[len-2] || s[len-1]);

		if (needZero)
		{
			lua_pushlstring(L, s, len);
			lua_pushlstring(L, "\0", 1);
			lua_concat(L, 2);
			text = (const wchar_t*)lua_tostring(L, -1);
		}
		else
			text = len ? (const wchar_t*)s : L"";
	}
	else
	{
		text = check_utf8_string(L,2,NULL);
	}
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_INSERTTEXT, 0, (void*)text) != 0);
	return 1;
}

static int editor_InsertText(lua_State *L) { return _EditorInsertText(L, 0); }
static int editor_InsertTextW(lua_State *L) { return _EditorInsertText(L, 1); }

static int editor_DeleteChar(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETECHAR, 0, 0) != 0);
	return 1;
}

static int editor_DeleteBlock(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETEBLOCK, 0, 0) != 0);
	return 1;
}

static int editor_UndoRedo(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	struct EditorUndoRedo eur = { sizeof(eur) };
	eur.Command = check_env_flag(L, 2);
	lua_pushboolean(L, GetPluginData(L)->Info->EditorControl(EditorId, ECTL_UNDOREDO, 0, &eur) != 0);
	return 1;
}

static int editor_SetKeyBar(lua_State *L)
{
	return SetKeyBar(L, TRUE);
}

static int editor_SetParam(lua_State *L)
{
	wchar_t buf[256];
	int tp;
	intptr_t result;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSetParameter esp = { sizeof(esp) };
	esp.Type = check_env_flag(L,2);
	//-----------------------------------------------------
	tp = lua_type(L,3);

	if (tp == LUA_TNUMBER)
		esp.Param.iParam = lua_tointeger(L,3);
	else if (tp == LUA_TBOOLEAN)
		esp.Param.iParam = lua_toboolean(L,3);
	else if (tp == LUA_TSTRING)
		esp.Param.wszParam = check_utf8_string(L,3,NULL);

	//-----------------------------------------------------
	if (esp.Type == ESPT_GETWORDDIV)
	{
		esp.Param.wszParam = buf;
		esp.Size = ARRSIZE(buf);
	}

	//-----------------------------------------------------
	esp.Flags = GetFlagCombination(L, 4, NULL);
	//-----------------------------------------------------
	result = Info->EditorControl(EditorId, ECTL_SETPARAM, 0, &esp);
	lua_pushboolean(L, result != 0);

	if (result && esp.Type == ESPT_GETWORDDIV)
	{
		push_utf8_string(L,buf,-1); return 2;
	}

	return 1;
}

static int editor_SetPosition(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSetPosition esp = { sizeof(esp) };

	if (lua_istable(L, 2))
	{
		lua_settop(L, 2);
		FillEditorSetPosition(L, &esp);
	}
	else
	{
		esp.CurLine   = luaL_optinteger(L,2,0) - 1;
		esp.CurPos    = luaL_optinteger(L,3,0) - 1;
		esp.CurTabPos = luaL_optinteger(L,4,0) - 1;
		esp.TopScreenLine = luaL_optinteger(L,5,0) - 1;
		esp.LeftPos   = luaL_optinteger(L,6,0) - 1;
		esp.Overtype  = luaL_optinteger(L,7,-1);
	}

	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_SETPOSITION, 0, &esp));
	return 1;
}

static int editor_Redraw(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_REDRAW, 0, 0));
	return 1;
}

static int editor_ExpandTabs(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t line_num = luaL_optinteger(L, 2, 0) - 1;
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_EXPANDTABS, 0, &line_num));
	return 1;
}

static int PushBookmarks(lua_State *L, int command)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	size_t size = GetPluginData(L)->Info->EditorControl(EditorId, command, 0, NULL);
	if (size)
	{
		struct EditorBookmarks *ebm = (struct EditorBookmarks*)lua_newuserdata(L, size);
		ebm->StructSize = sizeof(*ebm);
		ebm->Size = size;
		if (GetPluginData(L)->Info->EditorControl(EditorId, command, 0, ebm))
		{
			lua_createtable(L, (int)ebm->Count, 0);
			for(int i=0; i < (int)ebm->Count; i++)
			{
				lua_pushinteger(L, i+1);
				lua_createtable(L, 0, 4);
				PutNumToTable(L, "Line", (double) ebm->Line[i] + 1);
				PutNumToTable(L, "Cursor", (double) ebm->Cursor[i] + 1);
				PutNumToTable(L, "ScreenLine", (double) ebm->ScreenLine[i] + 1);
				PutNumToTable(L, "LeftPos", (double) ebm->LeftPos[i] + 1);
				lua_rawset(L, -3);
			}
			return 1;
		}
	}
	return lua_pushnil(L), 1;
}

static int editor_GetBookmarks(lua_State *L)
{
	return PushBookmarks(L, ECTL_GETBOOKMARKS);
}

static int editor_GetSessionBookmarks(lua_State *L)
{
	return PushBookmarks(L, ECTL_GETSESSIONBOOKMARKS);
}

static int editor_AddSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_ADDSESSIONBOOKMARK, 0, 0) != 0);
	return 1;
}

static int editor_ClearSessionBookmarks(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_CLEARSESSIONBOOKMARKS, 0, 0) != 0);
	return 1;
}

static int editor_DeleteSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	intptr_t num = luaL_optinteger(L, 2, 0) - 1;
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETESESSIONBOOKMARK, 0, (void*)num) != 0);
	return 1;
}

static int editor_NextSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_NEXTSESSIONBOOKMARK, 0, 0) != 0);
	return 1;
}

static int editor_PrevSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_PREVSESSIONBOOKMARK, 0, 0) != 0);
	return 1;
}

static int editor_SetTitle(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t* text = opt_utf8_string(L, 2, NULL);
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_SETTITLE, 0, (void*)text));
	return 1;
}

static int editor_GetTitle(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t size = Info->EditorControl(EditorId, ECTL_GETTITLE, 0, NULL);
	if (size)
	{
		wchar_t* buf = (wchar_t*)lua_newuserdata(L, size*sizeof(wchar_t));
		if (size == Info->EditorControl(EditorId, ECTL_GETTITLE, size, buf))
		{
			push_utf8_string(L, buf, -1);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int editor_Quit(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_QUIT, 0, 0));
	return 1;
}

int FillEditorSelect(lua_State *L, int pos_table, struct EditorSelect *es)
{
	int success;
	lua_getfield(L, pos_table, "BlockType");
	es->BlockType = (int) get_env_flag(L, -1, &success);

	if (!success)
	{
		lua_pop(L,1);
		return 0;
	}

	lua_pushvalue(L, pos_table);
	es->BlockStartLine = GetOptIntFromTable(L, "BlockStartLine", 0) - 1;
	es->BlockStartPos  = GetOptIntFromTable(L, "BlockStartPos", 0) - 1;
	es->BlockWidth     = GetOptIntFromTable(L, "BlockWidth", -1);
	es->BlockHeight    = GetOptIntFromTable(L, "BlockHeight", -1);
	lua_pop(L,2);
	return 1;
}

static int editor_Select(lua_State *L)
{
	int success = TRUE;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSelect es = { sizeof(es) };

	if (lua_istable(L, 2))
		success = FillEditorSelect(L, 2, &es);
	else
	{
		es.BlockType = (int) check_env_flag(L, 2);
		es.BlockStartLine = luaL_optinteger(L, 3, 0) - 1;
		es.BlockStartPos  = luaL_optinteger(L, 4, 0) - 1;
		es.BlockWidth     = luaL_optinteger(L, 5, -1);
		es.BlockHeight    = luaL_optinteger(L, 6, -1);
	}

	lua_pushboolean(L, success && Info->EditorControl(EditorId, ECTL_SELECT, 0, &es));
	return 1;
}

// This function is that long because FAR API does not supply needed
// information directly.
static int editor_GetSelection(lua_State *L)
{
	intptr_t BlockStartPos, h, from, to;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorInfo EI = { sizeof(EI) };
	struct EditorGetString egs = { sizeof(egs) };
	struct EditorSetPosition esp = { sizeof(esp) };
	Info->EditorControl(EditorId, ECTL_GETINFO, 0, &EI);

	if (EI.BlockType == BTYPE_NONE || !FastGetString(EditorId, EI.BlockStartLine, &egs, Info))
		return lua_pushnil(L), 1;

	lua_createtable(L, 0, 5);
	PutIntToTable(L, "BlockType", EI.BlockType);
	PutIntToTable(L, "StartLine", EI.BlockStartLine+1);
	BlockStartPos = egs.SelStart;
	PutIntToTable(L, "StartPos", BlockStartPos+1);
	// binary search for a non-block line
	h = 100; // arbitrary small number
	from = EI.BlockStartLine;

	for(to = from+h; to < EI.TotalLines; to = from + (h*=2))
	{
		if (!FastGetString(EditorId, to, &egs, Info))
			return lua_pushnil(L), 1;

		if (egs.SelStart < 0)
			break;
	}

	if (to >= EI.TotalLines)
		to = EI.TotalLines - 1;

	// binary search for the last block line
	while(from != to)
	{
		intptr_t curr = (from + to + 1) / 2;

		if (!FastGetString(EditorId, curr, &egs, Info))
			return lua_pushnil(L), 1;

		if (egs.SelStart < 0)
		{
			if (curr == to)
				break;

			to = curr;      // curr was not selected
		}
		else
		{
			from = curr;    // curr was selected
		}
	}

	if (!FastGetString(EditorId, from, &egs, Info))
		return lua_pushnil(L), 1;

	PutIntToTable(L, "EndLine", from+1);
	PutIntToTable(L, "EndPos", egs.SelEnd);
	// restore current position, since FastGetString() changed it
	esp.CurLine       = EI.CurLine;
	esp.CurPos        = EI.CurPos;
	esp.CurTabPos     = EI.CurTabPos;
	esp.TopScreenLine = EI.TopScreenLine;
	esp.LeftPos       = EI.LeftPos;
	esp.Overtype      = EI.Overtype;
	Info->EditorControl(EditorId, ECTL_SETPOSITION, 0, &esp);
	return 1;
}

static int _EditorTabConvert(lua_State *L, int Operation)
{
	intptr_t EditorId = luaL_optinteger(L,1,CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorConvertPos ecp = { sizeof(ecp) };
	ecp.StringNumber = luaL_optinteger(L,2,0) - 1;
	ecp.SrcPos = luaL_checkinteger(L,3) - 1;

	if (Info->EditorControl(EditorId, Operation, 0, &ecp))
		lua_pushinteger(L, ecp.DestPos+1);
	else
		lua_pushnil(L);

	return 1;
}

static int editor_TabToReal(lua_State *L)
{
	return _EditorTabConvert(L, ECTL_TABTOREAL);
}

static int editor_RealToTab(lua_State *L)
{
	return _EditorTabConvert(L, ECTL_REALTOTAB);
}

static int editor_AddColor(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	intptr_t EditorId;
	struct EditorColor ec = { sizeof(ec) };
	ec.ColorItem = 0;
	EditorId        = luaL_optinteger(L, 1, CURRENT_EDITOR);
	ec.StringNumber = luaL_optinteger(L, 2, 0) - 1;
	ec.StartPos     = luaL_checkinteger(L, 3) - 1;
	ec.EndPos       = luaL_checkinteger(L, 4) - 1;
	ec.Flags        = OptFlags(L, 5, 0);
	luaL_argcheck(L, GetFarColor(L, 6, &ec.Color), 6, "table or number expected");
	ec.Priority     = (unsigned) luaL_optnumber(L, 7, EDITOR_COLOR_NORMAL_PRIORITY);
	GetOptGuid(L, 8, &ec.Owner, pd->PluginId);
	lua_pushboolean(L, pd->Info->EditorControl(EditorId, ECTL_ADDCOLOR, 0, &ec) != 0);
	return 1;
}

static int editor_DelColor(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	intptr_t EditorId;
	struct EditorDeleteColor edc = { sizeof(edc) };
	EditorId         = luaL_optinteger(L, 1, CURRENT_EDITOR);
	edc.StringNumber = luaL_optinteger(L, 2, 0) - 1;
	edc.StartPos     = luaL_optinteger(L, 3, 0) - 1;
	GetOptGuid(L, 4, &edc.Owner, pd->PluginId);
	lua_pushboolean(L, pd->Info->EditorControl(EditorId, ECTL_DELCOLOR, 0, &edc) != 0);
	return 1;
}

static int editor_GetColor(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t EditorId;
	struct EditorColor ec = { sizeof(ec) };
	EditorId        = luaL_optinteger(L, 1, CURRENT_EDITOR);
	ec.StringNumber = luaL_optinteger(L, 2, 0) - 1;
	ec.ColorItem    = luaL_checkinteger(L, 3);

	if (Info->EditorControl(EditorId, ECTL_GETCOLOR, 0, &ec))
	{
		lua_createtable(L, 0, 6);
		PutNumToTable(L, "StartPos", (double)ec.StartPos+1);
		PutNumToTable(L, "EndPos", (double)ec.EndPos+1);
		PutNumToTable(L, "Priority", (double)ec.Priority);
		PutFlagsToTable(L, "Flags",  ec.Flags);
		PushFarColor(L, &ec.Color);
		lua_setfield(L, -2, "Color");
		PutLStrToTable(L, "Owner", (const void*)&ec.Owner, sizeof(ec.Owner));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int editor_SaveFile(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSaveFile esf = { sizeof(esf) };
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	esf.FileName = opt_utf8_string(L, 2, NULL);
	esf.FileEOL = opt_utf8_string(L, 3, NULL);
	esf.CodePage = luaL_optinteger(L, 4, CP_DEFAULT);
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_SAVEFILE, 0, &esf));
	return 1;
}

static int editor_ReadInput(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	INPUT_RECORD ir;

	if (Info->EditorControl(EditorId, ECTL_READINPUT, 0, &ir))
		PushInputRecord(L, &ir);
	else
		lua_pushnil(L);

	return 1;
}

static int editor_ProcessInput(lua_State *L)
{
	INPUT_RECORD ir;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	luaL_checktype(L, 2, LUA_TTABLE);
	FillInputRecord(L, 2, &ir);
	lua_pushboolean(L, GetPluginData(L)->Info->EditorControl(EditorId, ECTL_PROCESSINPUT, 0, &ir) != 0);
	return 1;
}

static int editor_SubscribeChangeEvent(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	struct EditorSubscribeChangeEvent data = { sizeof(data) };
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	int command = lua_toboolean(L, 2) ? ECTL_SUBSCRIBECHANGEEVENT : ECTL_UNSUBSCRIBECHANGEEVENT;

	data.PluginId = *pd->PluginId;
	lua_pushboolean(L, pd->Info->EditorControl(EditorId, command, 0, &data) != 0);

	return 1;
}

static int editor_Editor(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t* FileName = check_utf8_string(L, 1, NULL);
	const wchar_t* Title    = opt_utf8_string(L, 2, NULL);
	intptr_t X1 = luaL_optinteger(L, 3, 0);
	intptr_t Y1 = luaL_optinteger(L, 4, 0);
	intptr_t X2 = luaL_optinteger(L, 5, -1);
	intptr_t Y2 = luaL_optinteger(L, 6, -1);
	flags_t  Flags = OptFlags(L,7,0);
	intptr_t StartLine = luaL_optinteger(L, 8, -1);
	intptr_t StartChar = luaL_optinteger(L, 9, -1);
	intptr_t CodePage  = luaL_optinteger(L, 10, CP_DEFAULT);
	intptr_t ret = Info->Editor(FileName, Title, X1, Y1, X2, Y2, Flags, StartLine, StartChar, CodePage);
	lua_pushinteger(L, (int)ret);
	return 1;
}

static const luaL_Reg editor_funcs[] =
{
	PAIR( editor, AddColor),
	PAIR( editor, AddSessionBookmark),
	PAIR( editor, ClearSessionBookmarks),
	PAIR( editor, DelColor),
	PAIR( editor, DeleteBlock),
	PAIR( editor, DeleteChar),
	PAIR( editor, DeleteSessionBookmark),
	PAIR( editor, DeleteString),
	PAIR( editor, Editor),
	PAIR( editor, ExpandTabs),
	PAIR( editor, GetBookmarks),
	PAIR( editor, GetColor),
	PAIR( editor, GetFileName),
	PAIR( editor, GetInfo),
	PAIR( editor, GetSelection),
	PAIR( editor, GetSessionBookmarks),
	PAIR( editor, GetString),
	PAIR( editor, GetStringW),
	PAIR( editor, GetTitle),
	PAIR( editor, InsertString),
	PAIR( editor, InsertText),
	PAIR( editor, InsertTextW),
	PAIR( editor, NextSessionBookmark),
	PAIR( editor, PrevSessionBookmark),
	PAIR( editor, ProcessInput),
	PAIR( editor, Quit),
	PAIR( editor, ReadInput),
	PAIR( editor, RealToTab),
	PAIR( editor, Redraw),
	PAIR( editor, SaveFile),
	PAIR( editor, Select),
	PAIR( editor, SetKeyBar),
	PAIR( editor, SetParam),
	PAIR( editor, SetPosition),
	PAIR( editor, SetString),
	PAIR( editor, SetStringW),
	PAIR( editor, SetTitle),
	PAIR( editor, SubscribeChangeEvent),
	PAIR( editor, TabToReal),
	PAIR( editor, UndoRedo),

	{NULL, NULL},
};

int luaopen_editor(lua_State *L)
{
	luaL_register(L, "editor", editor_funcs);
	lua_pop(L, 1);
	return 0;
}
