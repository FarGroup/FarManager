//---------------------------------------------------------------------------

#include <windows.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "lf_common.h"
#include "lf_flags.h"
#include "lf_luafar.h"
#include "lf_service.h"
#include "lf_string.h"
#include "lf_util.h"

static const char FAR_DN_STORAGE[] = "FAR_DN_STORAGE";

static intptr_t GetEnableFromLua (lua_State *L, int pos)
{
	intptr_t ret;
	if (lua_isnoneornil(L,pos)) //get state
		ret = -1;
	else if (lua_isnumber(L,pos))
		ret = lua_tointeger(L, pos);
	else
		ret = lua_toboolean(L, pos);
	return ret;
}

int Dialog_getvalue(lua_State *L, int pos, HANDLE *target)
{
	if (lua_type(L, pos) == LUA_TUSERDATA)
	{
		int equal;
		lua_getmetatable(L, pos);
		luaL_getmetatable(L, TYPE_DIALOG);
		equal = lua_rawequal(L, -1, -2);
		lua_pop(L, 2);
		if (equal && target)
		{
			*target = ((TDialogData*)lua_touserdata(L, pos))->hDlg;
			equal = *target != INVALID_HANDLE_VALUE;
		}
		return equal;
	}
	return 0;
}

// the table is on lua stack top
static flags_t GetItemFlags(lua_State* L, int flag_index, int item_index)
{
	int success;
	lua_pushinteger(L, flag_index);
	lua_gettable(L, -2);
	flags_t flags = GetFlagCombination(L, -1, &success);

	if (!success)
		return luaL_error(L, "unsupported flag in dialog item %d", item_index);

	lua_pop(L, 1);
	return flags;
}

static UINT64 GetDialogItemType(lua_State* L, int key, int item)
{
	int success;
	lua_pushinteger(L, key);
	lua_gettable(L, -2);
	flags_t iType = get_env_flag(L, -1, &success);

	if (!success)
	{
		const char* sType = lua_tostring(L, -1);
		return luaL_error(L, "%s - unsupported type in dialog item %d", sType, item);
	}

	lua_pop(L, 1);
	return iType;
}

// list table is on Lua stack top
struct FarList* CreateList(lua_State *L, int historyindex)
{
	int n = (int)lua_objlen(L,-1);
	struct FarList* list = (struct FarList*)lua_newuserdata(L,
	                       sizeof(struct FarList) + n*sizeof(struct FarListItem)); // +2
	int len = (int)lua_objlen(L, historyindex);
	lua_rawseti(L, historyindex, ++len);  // +1; put into "histories" table to avoid being gc'ed
	list->StructSize = sizeof(struct FarList);
	list->ItemsNumber = n;
	list->Items = (struct FarListItem*)(list+1);

	for (int i=0; i<n; i++)
	{
		struct FarListItem *p = list->Items + i;
		lua_pushinteger(L, i+1); // +2
		lua_gettable(L,-2);      // +2

		if (lua_type(L,-1) != LUA_TTABLE)
			luaL_error(L, "value at index %d is not a table", i+1);

		p->Text = NULL;
		lua_getfield(L, -1, "Text"); // +3

		if (lua_isstring(L,-1))
		{
			lua_pushvalue(L,-1);       // +4
			p->Text = check_utf8_string(L,-1,NULL); // +4
			lua_rawseti(L, historyindex, ++len);  // +3
		}

		lua_pop(L, 1);                 // +2
		p->Flags = CheckFlagsFromTable(L, -1, "Flags");
		lua_pop(L, 1);                 // +1
	}

	return list;
}

static void PushList (lua_State *L, const struct FarList *list)
{
	lua_createtable(L, (int)list->ItemsNumber, 0);
	for (int i=0; i < (int)list->ItemsNumber; i++)
	{
		lua_createtable(L,0,2);
		PutFlagsToTable(L, "Flags", list->Items[i].Flags);
		PutWStrToTable(L, "Text", list->Items[i].Text, -1);
		lua_rawseti(L,-2,i+1);
		if (list->Items[i].Flags & LIF_SELECTED)
			PutIntToTable(L, "SelectIndex", i+1);
	}
}

//	enum FARDIALOGITEMTYPES Type;            1
//	intptr_t X1,Y1,X2,Y2;                    2,3,4,5
//	union
//	{
//		intptr_t Selected;                     6
//		struct FarList *ListItems;             6
//		struct FAR_CHAR_INFO *VBuf;            6
//		intptr_t Reserved0;                    6
//	}
//#ifndef __cplusplus
//	Param
//#endif
//	;
//	const wchar_t *History;                  7
//	const wchar_t *Mask;                     8
//	FARDIALOGITEMFLAGS Flags;                9
//	const wchar_t *Data;                     10
//	size_t MaxLength;                        11  // terminate 0 not included (if == 0 string size is unlimited)
//	intptr_t UserData;                       12
//	intptr_t Reserved[2];


// item table is on Lua stack top
static void SetFarDialogItem(lua_State *L, struct FarDialogItem* Item, int itemindex,
                             int historyindex)
{
	memset(Item, 0, sizeof(struct FarDialogItem));
	Item->Type  = GetDialogItemType(L, 1, itemindex+1);
	Item->X1    = GetIntFromArray(L, 2);
	Item->Y1    = GetIntFromArray(L, 3);
	Item->X2    = GetIntFromArray(L, 4);
	Item->Y2    = GetIntFromArray(L, 5);
	Item->Flags = GetItemFlags(L, 9, itemindex+1);

	if (Item->Type==DI_LISTBOX || Item->Type==DI_COMBOBOX)
	{
		int SelectIndex;
		lua_rawgeti(L, -1, 6);             // +1

		if (lua_type(L,-1) != LUA_TTABLE)
			luaLF_SlotError(L, 6, "table");

		Item->Param.ListItems = CreateList(L, historyindex);
		SelectIndex = GetOptIntFromTable(L, "SelectIndex", -1);

		if (SelectIndex > 0 && SelectIndex <= (int)lua_objlen(L,-1))
			Item->Param.ListItems->Items[SelectIndex-1].Flags |= LIF_SELECTED;

		lua_pop(L,1);                      // 0
	}
	else if (Item->Type == DI_USERCONTROL)
	{
		lua_rawgeti(L, -1, 6);
		if (lua_type(L,-1) == LUA_TUSERDATA)
		{
			TFarUserControl* fuc = CheckFarUserControl(L, -1);
			Item->Param.VBuf = fuc->VBuf;
		}
		lua_pop(L,1);
	}
	else
		Item->Param.Selected = GetIntFromArray(L, 6);

	//---------------------------------------------------------------------------
	if (Item->Flags & DIF_HISTORY)
	{
		lua_rawgeti(L, -1, 7);                          // +1
		Item->History = opt_utf8_string(L, -1, NULL);   // +1
		int len = (int)lua_objlen(L, historyindex);
		lua_rawseti(L, historyindex, len+1);  // +0; put into "histories" table to avoid being gc'ed
	}

	//---------------------------------------------------------------------------
	lua_rawgeti(L, -1, 8);                       // +1
	Item->Mask = opt_utf8_string(L, -1, NULL);   // +1
	int len = (int)lua_objlen(L, historyindex);
	lua_rawseti(L, historyindex, len+1);  // +0; put into "histories" table to avoid being gc'ed
	//---------------------------------------------------------------------------
	Item->MaxLength = GetOptIntFromArray(L, 11, 0);
	lua_pushinteger(L, 10); // +1
	lua_gettable(L, -2);    // +1

	if (lua_isstring(L, -1))
	{
		Item->Data = check_utf8_string(L, -1, NULL);  // +1
		len = (int)lua_objlen(L, historyindex);
		lua_rawseti(L, historyindex, len+1);  // +0; put into "histories" table to avoid being gc'ed
	}
	else
		lua_pop(L, 1);

	//---------------------------------------------------------------------------
	lua_rawgeti(L, -1, 12);
	Item->UserData = lua_tointeger(L, -1);
	lua_pop(L, 1);
}

static void PushDlgItem(lua_State *L, const struct FarDialogItem* pItem, BOOL table_exist)
{
	if (! table_exist)
		lua_createtable(L, 12, 0);

	PutIntToArray(L, 1, pItem->Type);
	PutIntToArray(L, 2, pItem->X1);
	PutIntToArray(L, 3, pItem->Y1);
	PutIntToArray(L, 4, pItem->X2);
	PutIntToArray(L, 5, pItem->Y2);

	if ((pItem->Type == DI_LISTBOX || pItem->Type == DI_COMBOBOX) && pItem->Param.ListItems)
	{
		PushList(L, pItem->Param.ListItems);
		lua_rawseti(L, -2, 6);
	}
	else if (pItem->Type == DI_USERCONTROL)
	{
		lua_pushinteger(L, 6);
		lua_pushlightuserdata(L, pItem->Param.VBuf);
		lua_settable(L, -3);
	}
	else
		PutIntToArray(L, 6, pItem->Param.Selected);

	PutWStrToArray(L, 7, pItem->History, -1);
	PutWStrToArray(L, 8, pItem->Mask, -1);
	PutFlagsToArray(L, 9, pItem->Flags);
	lua_pushinteger(L, 10);
	push_utf8_string(L, pItem->Data, -1);
	lua_settable(L, -3);
	PutIntToArray(L, 11, pItem->MaxLength);
	lua_pushinteger(L, 12);
	lua_pushinteger(L, pItem->UserData);
	lua_rawset(L, -3);
}

static void PushDlgItemNum(lua_State *L, HANDLE hDlg, int numitem, int pos_table,
                           PSInfo *Info)
{
	struct FarGetDialogItem fgdi = { sizeof(struct FarGetDialogItem), 0, 0 };
	fgdi.Size = Info->SendDlgMessage(hDlg, DM_GETDLGITEM, numitem, &fgdi);

	if (fgdi.Size > 0)
	{
		BOOL table_exist;
		fgdi.Item = (struct FarDialogItem*) lua_newuserdata(L, fgdi.Size);
		Info->SendDlgMessage(hDlg, DM_GETDLGITEM, numitem, &fgdi);
		table_exist = lua_istable(L, pos_table);

		if (table_exist)
			lua_pushvalue(L, pos_table);

		PushDlgItem(L, fgdi.Item, table_exist);
		lua_remove(L, -2);
	}
	else
		lua_pushnil(L);
}

static int SetDlgItem(lua_State *L, HANDLE hDlg, int numitem, int pos_table,
                      PSInfo *Info)
{
	struct FarDialogItem DialogItem;
	lua_newtable(L);
	lua_replace(L,1);
	luaL_checktype(L, pos_table, LUA_TTABLE);
	lua_pushvalue(L, pos_table);
	SetFarDialogItem(L, &DialogItem, numitem, 1);
	lua_pushboolean(L, (int)Info->SendDlgMessage(hDlg, DM_SETDLGITEM, numitem, &DialogItem));
	return 1;
}

TDialogData* NewDialogData(lua_State* L, PSInfo *Info, HANDLE hDlg, BOOL isOwned)
{
	TDialogData *dd = (TDialogData*) lua_newuserdata(L, sizeof(TDialogData));
	dd->L        = GetPluginData(L)->MainLuaState;
	dd->Info     = Info;
	dd->hDlg     = hDlg;
	dd->isOwned  = isOwned;
	dd->wasError = FALSE;
	dd->isModal  = TRUE;
	dd->dataRef  = LUA_REFNIL;
	luaL_getmetatable(L, TYPE_DIALOG);
	lua_setmetatable(L, -2);

	if (isOwned)
	{
		lua_newtable(L);
		lua_setfenv(L, -2);
	}

	return dd;
}

TDialogData* CheckDialog(lua_State* L, int pos)
{
	return (TDialogData*)luaL_checkudata(L, pos, TYPE_DIALOG);
}

TDialogData* CheckValidDialog(lua_State* L, int pos)
{
	TDialogData* dd = CheckDialog(L, pos);
	luaL_argcheck(L, dd->hDlg != INVALID_HANDLE_VALUE, pos, "closed dialog");
	return dd;
}

HANDLE CheckDialogHandle(lua_State* L, int pos)
{
	return CheckValidDialog(L, pos)->hDlg;
}

int DialogHandleEqual(lua_State* L)
{
	TDialogData* dd1 = CheckDialog(L, 1);
	TDialogData* dd2 = CheckDialog(L, 2);
	lua_pushboolean(L, dd1->hDlg == dd2->hDlg);
	return 1;
}

int PushDMParams (lua_State *L, intptr_t Msg, intptr_t Param1)
{
	if (! ((Msg>DM_FIRST && Msg<=DM_GETDIALOGTITLE) || Msg==DM_USER))
		return 0;

	lua_pushinteger(L, Msg);             //+1

	// Param1
	switch(Msg)
	{
		case DM_CLOSE:
			lua_pushinteger(L, Param1<=0 ? Param1 : Param1+1);
			break;

		case DM_ENABLEREDRAW:
		case DM_GETDIALOGINFO:
		case DM_GETDIALOGTITLE:
		case DM_GETDLGDATA:
		case DM_GETDLGRECT:
		case DM_GETDROPDOWNOPENED:
		case DM_GETFOCUS:
		case DM_KEY:
		case DM_MOVEDIALOG:
		case DM_REDRAW:
		case DM_RESIZEDIALOG:
		case DM_SETDLGDATA:
		case DM_SETINPUTNOTIFY:
		case DM_SHOWDIALOG:
		case DM_USER:
			lua_pushinteger(L, Param1);
			break;

		default: // dialog element position
			lua_pushinteger(L, Param1+1);
			break;
	}

	return 1;
}

static int DoSendDlgMessage (lua_State *L, intptr_t Msg, int delta)
{
	typedef struct { void *Id; int Ref; } listdata_t;
	TPluginData *pluginData = GetPluginData(L);
	PSInfo *Info = pluginData->Info;
	intptr_t Param1=0, res=0, res_incr=0;
	void* Param2 = NULL;
	wchar_t buf[512];
	int pos2 = 2-delta, pos3 = 3-delta, pos4 = 4-delta;
	//---------------------------------------------------------------------------
	COORD coord;
	SMALL_RECT small_rect;
	//---------------------------------------------------------------------------
	lua_settop(L, pos4); //many cases below rely on top==pos4
	HANDLE hDlg = CheckDialogHandle(L, 1);
	if (delta == 0)
		Msg = (int) check_env_flag(L, 2);

	// Param1
	switch(Msg)
	{
		case DM_CLOSE:
			Param1 = luaL_optinteger(L,pos3,-1);
			if (Param1>0) --Param1;
			break;

		case DM_ENABLEREDRAW:
		case DM_SETINPUTNOTIFY:
			Param1 = GetEnableFromLua(L,pos3);
			break;

		case DM_GETDLGDATA:
		case DM_SETDLGDATA:
			break;

		case DM_GETDIALOGINFO:
		case DM_GETDIALOGTITLE:
		case DM_GETDLGRECT:
		case DM_GETDROPDOWNOPENED:
		case DM_GETFOCUS:
		case DM_KEY:
		case DM_MOVEDIALOG:
		case DM_REDRAW:
		case DM_RESIZEDIALOG:
		case DM_SHOWDIALOG:
		case DM_USER:
		// DN_*
		case DN_DRAGGED:
		case DN_DRAWDIALOG:
		case DN_DRAWDIALOGDONE:
			Param1 = luaL_optinteger(L,pos3,0);
			break;

		default: // dialog element position
			Param1 = luaL_optinteger(L,pos3,1) - 1;
			break;
	}

	// res_incr
	switch(Msg)
	{
		case DM_GETFOCUS:
		case DM_LISTADDSTR:
			res_incr=1;
			break;

		default:
			res_incr=0;
			break;
	}

	// Param2 and the rest
	switch(Msg)
	{
		default:
			luaL_argerror(L, pos2, "operation not implemented");
			break;

		case DM_CLOSE:
		case DM_EDITUNCHANGEDFLAG:
		case DM_GETCHECK:
		case DM_GETCOMBOBOXEVENT:
		case DM_GETCURSORSIZE:
		case DM_GETDROPDOWNOPENED:
		case DM_GETFOCUS:
		case DM_GETITEMDATA:
		case DM_LISTSORT:
		case DM_REDRAW:               // alias: DM_SETREDRAW
		case DM_SET3STATE:
		case DM_SETCURSORSIZE:
		case DM_SETDROPDOWNOPENED:
		case DM_SETFOCUS:
		case DM_SETITEMDATA:
		case DM_SETMAXTEXTLENGTH:     // alias: DM_SETTEXTLENGTH
		case DM_SETINPUTNOTIFY:
		case DM_SHOWDIALOG:
		case DM_USER:
		// DN_*
		case DN_BTNCLICK:
		case DN_DRAGGED:
		case DN_DRAWDIALOG:
		case DN_DRAWDIALOGDONE:
		case DN_DROPDOWNOPENED:
			Param2 = (void*)(intptr_t)luaL_optint(L,pos4,0);
			break;

		case DM_ENABLEREDRAW:
			break;

		case DM_ENABLE:
		case DM_SHOWITEM:
			Param2 = (void*)GetEnableFromLua(L, pos4);
			break;

		case DM_LISTGETDATASIZE:
			Param2 = (void*)(intptr_t)(luaL_optint(L,pos4,1) - 1);
			break;

		case DM_LISTADDSTR:
		case DM_ADDHISTORY:
		case DM_SETHISTORY:
		case DM_SETTEXTPTR:
			Param2 = (void*)opt_utf8_string(L, pos4, NULL);
			break;

		case DM_SETCHECK:
			res = lua_isboolean(L,pos4) ? (lua_toboolean(L,pos4) ? BSTATE_CHECKED : BSTATE_UNCHECKED)
				: check_env_flag(L, pos4);
			Param2 = (void*) res;
			break;

		case DM_GETCURSORPOS:
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &coord))
			{
				lua_createtable(L,0,2);
				PutNumToTable(L, "X", coord.X);
				PutNumToTable(L, "Y", coord.Y);
				return 1;
			}
			return lua_pushnil(L), 1;

		case DM_GETDIALOGINFO:
		{
			struct DialogInfo dlg_info = { sizeof(dlg_info) };
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &dlg_info))
			{
				lua_createtable(L,0,2);
				PutLStrToTable(L, "Id", (const char*)&dlg_info.Id, sizeof(dlg_info.Id));
				PutLStrToTable(L, "Owner", (const char*)&dlg_info.Owner, sizeof(dlg_info.Owner));
				return 1;
			}
			return lua_pushnil(L), 1;
		}

		case DM_GETDLGDATA: {
			TDialogData *dd = (TDialogData*) Info->SendDlgMessage(hDlg,Msg,0,0);
			lua_rawgeti(L, LUA_REGISTRYINDEX, dd->dataRef);
			return 1;
		}

		case DM_SETDLGDATA: {
			TDialogData *dd = (TDialogData*) Info->SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
			lua_rawgeti(L, LUA_REGISTRYINDEX, dd->dataRef);
			luaL_unref(L, LUA_REGISTRYINDEX, dd->dataRef);
			lua_pushvalue(L, pos3);
			dd->dataRef = luaL_ref(L, LUA_REGISTRYINDEX);
			return 1;
		}

		case DM_GETDLGRECT:
		case DM_GETITEMPOSITION:
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &small_rect))
			{
				lua_createtable(L,0,4);
				PutNumToTable(L, "Left", small_rect.Left);
				PutNumToTable(L, "Top", small_rect.Top);
				PutNumToTable(L, "Right", small_rect.Right);
				PutNumToTable(L, "Bottom", small_rect.Bottom);
				return 1;
			}
			return lua_pushnil(L), 1;

		case DM_GETEDITPOSITION:
		{
			struct EditorSetPosition esp = { sizeof(esp) };

			if (Info->SendDlgMessage(hDlg, Msg, Param1, &esp))
				return PushEditorSetPosition(L, &esp), 1;

			return lua_pushnil(L), 1;
		}

		case DM_GETSELECTION:
		{
			struct EditorSelect es = { sizeof(es) };

			if (Info->SendDlgMessage(hDlg, Msg, Param1, &es))
			{
				lua_createtable(L,0,5);
				PutNumToTable(L, "BlockType", (double) es.BlockType);
				PutNumToTable(L, "BlockStartLine", (double) es.BlockStartLine+1);
				PutNumToTable(L, "BlockStartPos", (double) es.BlockStartPos+1);
				PutNumToTable(L, "BlockWidth", (double) es.BlockWidth);
				PutNumToTable(L, "BlockHeight", (double) es.BlockHeight);
				return 1;
			}
			return lua_pushnil(L), 1;
		}

		case DM_SETSELECTION:
		{
			struct EditorSelect es = { sizeof(es) };
			luaL_checktype(L, pos4, LUA_TTABLE);

			if (FillEditorSelect(L, pos4, &es))
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &es));
			else
				lua_pushinteger(L,0);

			return 1;
		}

		case DM_GETTEXT:
		case DM_GETDIALOGTITLE:
		{
			struct FarDialogItemData fdid = { sizeof(fdid) };
			fdid.PtrLength = (size_t) Info->SendDlgMessage(hDlg, Msg, Param1, NULL);
			fdid.PtrData = (wchar_t*) malloc((fdid.PtrLength+1) * sizeof(wchar_t));
			size_t size = Info->SendDlgMessage(hDlg, Msg, Param1, &fdid);
			push_utf8_string(L, size?fdid.PtrData:L"", size);
			free(fdid.PtrData);
			return 1;
		}

		case DM_GETCONSTTEXTPTR:
		{
			wchar_t *ptr = (wchar_t*)Info->SendDlgMessage(hDlg, Msg, Param1, 0);
			push_utf8_string(L, ptr ? ptr:L"", -1);
			return 1;
		}

		case DM_SETTEXT:
		{
			struct FarDialogItemData fdid = { sizeof(fdid) };
			fdid.PtrLength = 0;
			fdid.PtrData = check_utf8_string(L, pos4, &fdid.PtrLength);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &fdid));
			return 1;
		}

		case DM_KEY:
		{
			size_t i, count;
			INPUT_RECORD *arr;
			if (lua_istable(L,pos4))
			{
				count = lua_objlen(L, pos4);
				arr = (INPUT_RECORD*)lua_newuserdata(L, count * sizeof(INPUT_RECORD));
				for(i=0; i<count; i++)
				{
					lua_pushinteger(L,i+1);
					lua_gettable(L,pos4);
					if (!lua_istable(L,-1))
					{
						luaL_error(L, "element #%d in argument #%d is not a table", i+1, pos4);
					}
					FillInputRecord(L, -1, arr+i);
					lua_pop(L,1);
				}
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, count, arr));
			}
			else if (lua_isstring(L,pos4))
			{
				wchar_t *str = check_utf8_string(L,pos4,NULL);
				wchar_t *p, *q;
				for (p=str,count=0; *p; count++)
				{
					while(iswspace(*p)) p++;
					if (*p == 0) break;
					while(*p && !iswspace(*p)) p++;
				}
				arr = (INPUT_RECORD*)lua_newuserdata(L, count * sizeof(INPUT_RECORD));
				for(i=0, p=str; i<count; i++)
				{
					while(iswspace(*p)) p++;
					q = p;
					while(*p && !iswspace(*p)) p++;
					*p++ = 0;
					if (!pluginData->FSF->FarNameToInputRecord(q, arr+i))
						luaL_argerror(L, pos4, "invalid key");
				}
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, count, arr));
			}
			else
				luaL_typerror(L, pos4, "table or string");

			return 1;
		}

		case DM_LISTADD:
		case DM_LISTSET:
		{
			luaL_checktype(L, pos4, LUA_TTABLE);
			lua_createtable(L,1,0); // "history table"
			lua_replace(L,1);
			lua_settop(L,pos4);
			Param2 = CreateList(L, 1);
			break;
		}

		case DM_LISTDELETE:
		{
			struct FarListDelete fld = { sizeof(fld) };
			if (lua_isnoneornil(L, pos4))
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, NULL));
			else
			{
				luaL_checktype(L, pos4, LUA_TTABLE);
				fld.StartIndex = GetOptIntFromTable(L, "StartIndex", 1) - 1;
				fld.Count = GetOptIntFromTable(L, "Count", 1);
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &fld));
			}
			return 1;
		}

		case DM_LISTFINDSTRING:
		{
			struct FarListFind flf = { sizeof(flf) };
			luaL_checktype(L, pos4, LUA_TTABLE);
			flf.StartIndex = GetOptIntFromTable(L, "StartIndex", 1) - 1;
			lua_getfield(L, pos4, "Pattern");
			flf.Pattern = check_utf8_string(L, -1, NULL);
			lua_getfield(L, pos4, "Flags");
			flf.Flags = get_env_flag(L, -1, NULL);
			res = Info->SendDlgMessage(hDlg, Msg, Param1, &flf);
			res < 0 ? lua_pushnil(L) : lua_pushinteger(L, res+1);
			return 1;
		}

		case DM_LISTGETCURPOS:
		{
			struct FarListPos flp = { sizeof(flp) };
			Info->SendDlgMessage(hDlg, Msg, Param1, &flp);
			lua_createtable(L,0,2);
			PutIntToTable(L, "SelectPos", flp.SelectPos+1);
			PutIntToTable(L, "TopPos", flp.TopPos+1);
			return 1;
		}

		case DM_LISTGETITEM:
		{
			struct FarListGetItem flgi = { sizeof(flgi) };
			flgi.ItemIndex = luaL_checkinteger(L, pos4) - 1;
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &flgi))
			{
				lua_createtable(L,0,2);
				PutFlagsToTable(L, "Flags", flgi.Item.Flags);
				PutWStrToTable(L, "Text", flgi.Item.Text, -1);
				return 1;
			}

			return lua_pushnil(L), 1;
		}

		case DM_LISTGETTITLES:
		{
			struct FarListTitles flt = { sizeof(flt) };
			flt.Title = buf;
			flt.Bottom = buf + ARRSIZE(buf)/2;
			flt.TitleSize = ARRSIZE(buf)/2;
			flt.BottomSize = ARRSIZE(buf)/2;
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &flt))
			{
				lua_createtable(L,0,2);
				PutWStrToTable(L, "Title", flt.Title, -1);
				PutWStrToTable(L, "Bottom", flt.Bottom, -1);
				return 1;
			}

			return lua_pushnil(L), 1;
		}

		case DM_LISTSETTITLES:
		{
			struct FarListTitles flt = { sizeof(flt) };
			luaL_checktype(L, pos4, LUA_TTABLE);
			lua_getfield(L, pos4, "Title");
			flt.Title = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			lua_getfield(L, pos4, "Bottom");
			flt.Bottom = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &flt));
			return 1;
		}

		case DM_LISTINFO:
		{
			struct FarListInfo fli = { sizeof(fli) };
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &fli))
			{
				lua_createtable(L,0,6);
				PutFlagsToTable(L, "Flags", fli.Flags);
				PutIntToTable(L, "ItemsNumber", fli.ItemsNumber);
				PutIntToTable(L, "SelectPos", fli.SelectPos+1);
				PutIntToTable(L, "TopPos", fli.TopPos+1);
				PutIntToTable(L, "MaxHeight", fli.MaxHeight);
				PutIntToTable(L, "MaxLength", fli.MaxLength);
				return 1;
			}
			return lua_pushnil(L), 1;
		}

		case DM_LISTINSERT:
		{
			struct FarListInsert flins = { sizeof(flins) };
			luaL_checktype(L, pos4, LUA_TTABLE);
			flins.Index = GetOptIntFromTable(L, "Index", 1) - 1;
			lua_getfield(L, pos4, "Text");
			flins.Item.Text = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			flins.Item.Flags = CheckFlagsFromTable(L, pos4, "Flags");
			res = Info->SendDlgMessage(hDlg, Msg, Param1, &flins);
			res < 0 ? lua_pushnil(L) : lua_pushinteger(L, res);
			return 1;
		}

		case DM_LISTUPDATE:
		{
			struct FarListUpdate flu = { sizeof(flu) };
			luaL_checktype(L, pos4, LUA_TTABLE);
			flu.Index = GetOptIntFromTable(L, "Index", 1) - 1;
			lua_getfield(L, pos4, "Text");
			flu.Item.Text = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			flu.Item.Flags = CheckFlagsFromTable(L, pos4, "Flags");
			lua_pushboolean(L, Info->SendDlgMessage(hDlg, Msg, Param1, &flu) != 0);
			return 1;
		}

		case DM_LISTSETCURPOS:
		{
			struct FarListPos flp = { sizeof(flp) };
			luaL_checktype(L, pos4, LUA_TTABLE);
			flp.SelectPos = GetOptIntFromTable(L, "SelectPos", 1) - 1;
			flp.TopPos = GetOptIntFromTable(L, "TopPos", 1) - 1;
			lua_pushinteger(L, 1 + Info->SendDlgMessage(hDlg, Msg, Param1, &flp));
			return 1;
		}

		case DM_LISTSETDATA:
		{
			listdata_t Data, *oldData;
			intptr_t Index;
			struct FarListItemData flid = { sizeof(flid) };

			luaL_checktype(L, pos4, LUA_TTABLE);
			Index = GetOptIntFromTable(L, "Index", 1) - 1;
			lua_getfenv(L, 1);
			lua_getfield(L, pos4, "Data");
			if (lua_isnil(L,-1)) // nil is not allowed
			{
				lua_pushinteger(L,0);
				return 1;
			}

			oldData = (listdata_t*)Info->SendDlgMessage(hDlg, DM_LISTGETDATA, Param1, (void*)Index);
			if (oldData &&
				sizeof(listdata_t) == Info->SendDlgMessage(hDlg, DM_LISTGETDATASIZE, Param1, (void*)Index) &&
				oldData->Id == pluginData)
			{
				luaL_unref(L, -2, oldData->Ref);
			}
			Data.Id = pluginData;
			Data.Ref = luaL_ref(L, -2);
			flid.Index = Index;
			flid.Data = &Data;
			flid.DataSize = sizeof(Data);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &flid));
			return 1;
		}

		case DM_LISTGETDATA:
		{
			intptr_t Index = luaL_checkinteger(L, pos4) - 1;
			listdata_t *Data = (listdata_t*)Info->SendDlgMessage(hDlg, Msg, Param1, (void*)Index);
			if (Data)
			{
				if (sizeof(listdata_t) == Info->SendDlgMessage(hDlg, DM_LISTGETDATASIZE, Param1, (void*)Index) &&
					Data->Id == pluginData)
				{
					lua_getfenv(L, 1);
					lua_rawgeti(L, -1, Data->Ref);
				}
				else
					lua_pushlightuserdata(L, Data);
			}
			else
				lua_pushnil(L);

			return 1;
		}

		case DM_GETDLGITEM:
			return PushDlgItemNum(L, hDlg, (int)Param1, pos4, Info), 1;

		case DM_SETDLGITEM:
			return SetDlgItem(L, hDlg, (int)Param1, pos4, Info);

		case DM_MOVEDIALOG:
		case DM_RESIZEDIALOG:
		case DM_SETCURSORPOS:
		{
			COORD *c;
			luaL_checktype(L, pos4, LUA_TTABLE);
			coord.X = GetOptIntFromTable(L, "X", 0);
			coord.Y = GetOptIntFromTable(L, "Y", 0);

			if (Msg == DM_SETCURSORPOS)
			{
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &coord));
				return 1;
			}
			c = (COORD*) Info->SendDlgMessage(hDlg, Msg, Param1, &coord);
			lua_createtable(L, 0, 2);
			PutIntToTable(L, "X", c->X);
			PutIntToTable(L, "Y", c->Y);
			return 1;
		}

		case DM_SETITEMPOSITION:
			luaL_checktype(L, pos4, LUA_TTABLE);
			small_rect.Left = GetOptIntFromTable(L, "Left", 0);
			small_rect.Top = GetOptIntFromTable(L, "Top", 0);
			small_rect.Right = GetOptIntFromTable(L, "Right", 0);
			small_rect.Bottom = GetOptIntFromTable(L, "Bottom", 0);
			Param2 = &small_rect;
			break;

		case DM_SETCOMBOBOXEVENT:
			Param2 = (void*)(intptr_t)OptFlags(L, pos4, 0);
			break;

		case DM_SETEDITPOSITION:
		{
			struct EditorSetPosition esp ={ sizeof(esp) };
			luaL_checktype(L, pos4, LUA_TTABLE);
			lua_settop(L, pos4);
			FillEditorSetPosition(L, &esp);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &esp));
			return 1;
		}

		case DN_CONTROLINPUT:
		{
			INPUT_RECORD rec;
			OptInputRecord(L, pluginData, pos4, &rec);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &rec));
			return 1;
		}
	}

	res = Info->SendDlgMessage(hDlg, Msg, Param1, Param2);
	lua_pushinteger(L, res + res_incr);
	return 1;
}

#define DlgMethod(name,msg) \
static int dlg_##name(lua_State *L) { return DoSendDlgMessage(L,msg,1); }

static int far_SendDlgMessage(lua_State *L) { return DoSendDlgMessage(L,0,0); }

DlgMethod( AddHistory,             DM_ADDHISTORY)
DlgMethod( Close,                  DM_CLOSE)
DlgMethod( EditUnchangedFlag,      DM_EDITUNCHANGEDFLAG)
DlgMethod( Enable,                 DM_ENABLE)
DlgMethod( EnableRedraw,           DM_ENABLEREDRAW)
DlgMethod( GetCheck,               DM_GETCHECK)
DlgMethod( GetComboboxEvent,       DM_GETCOMBOBOXEVENT)
DlgMethod( GetConstTextPtr,        DM_GETCONSTTEXTPTR)
DlgMethod( GetCursorPos,           DM_GETCURSORPOS)
DlgMethod( GetCursorSize,          DM_GETCURSORSIZE)
DlgMethod( GetDialogInfo,          DM_GETDIALOGINFO)
DlgMethod( GetDialogTitle,         DM_GETDIALOGTITLE)
DlgMethod( GetDlgData,             DM_GETDLGDATA)
DlgMethod( GetDlgItem,             DM_GETDLGITEM)
DlgMethod( GetDlgRect,             DM_GETDLGRECT)
DlgMethod( GetDropdownOpened,      DM_GETDROPDOWNOPENED)
DlgMethod( GetEditPosition,        DM_GETEDITPOSITION)
DlgMethod( GetFocus,               DM_GETFOCUS)
DlgMethod( GetItemData,            DM_GETITEMDATA)
DlgMethod( GetItemPosition,        DM_GETITEMPOSITION)
DlgMethod( GetSelection,           DM_GETSELECTION)
DlgMethod( GetText,                DM_GETTEXT)
DlgMethod( Key,                    DM_KEY)
DlgMethod( ListAdd,                DM_LISTADD)
DlgMethod( ListAddStr,             DM_LISTADDSTR)
DlgMethod( ListDelete,             DM_LISTDELETE)
DlgMethod( ListFindString,         DM_LISTFINDSTRING)
DlgMethod( ListGetCurPos,          DM_LISTGETCURPOS)
DlgMethod( ListGetData,            DM_LISTGETDATA)
DlgMethod( ListGetDataSize,        DM_LISTGETDATASIZE)
DlgMethod( ListGetItem,            DM_LISTGETITEM)
DlgMethod( ListGetTitles,          DM_LISTGETTITLES)
DlgMethod( ListInfo,               DM_LISTINFO)
DlgMethod( ListInsert,             DM_LISTINSERT)
DlgMethod( ListSet,                DM_LISTSET)
DlgMethod( ListSetCurPos,          DM_LISTSETCURPOS)
DlgMethod( ListSetData,            DM_LISTSETDATA)
DlgMethod( ListSetTitles,          DM_LISTSETTITLES)
DlgMethod( ListSort,               DM_LISTSORT)
DlgMethod( ListUpdate,             DM_LISTUPDATE)
DlgMethod( MoveDialog,             DM_MOVEDIALOG)
DlgMethod( Redraw,                 DM_REDRAW)
DlgMethod( ResizeDialog,           DM_RESIZEDIALOG)
DlgMethod( Set3State,              DM_SET3STATE)
DlgMethod( SetCheck,               DM_SETCHECK)
DlgMethod( SetComboboxEvent,       DM_SETCOMBOBOXEVENT)
DlgMethod( SetCursorPos,           DM_SETCURSORPOS)
DlgMethod( SetCursorSize,          DM_SETCURSORSIZE)
DlgMethod( SetDlgData,             DM_SETDLGDATA)
DlgMethod( SetDlgItem,             DM_SETDLGITEM)
DlgMethod( SetDropdownOpened,      DM_SETDROPDOWNOPENED)
DlgMethod( SetEditPosition,        DM_SETEDITPOSITION)
DlgMethod( SetFocus,               DM_SETFOCUS)
DlgMethod( SetHistory,             DM_SETHISTORY)
DlgMethod( SetInputNotify,         DM_SETINPUTNOTIFY)
DlgMethod( SetItemData,            DM_SETITEMDATA)
DlgMethod( SetItemPosition,        DM_SETITEMPOSITION)
DlgMethod( SetMaxTextLength,       DM_SETMAXTEXTLENGTH)
DlgMethod( SetSelection,           DM_SETSELECTION)
DlgMethod( SetText,                DM_SETTEXT)
DlgMethod( SetTextPtr,             DM_SETTEXTPTR)
DlgMethod( ShowDialog,             DM_SHOWDIALOG)
DlgMethod( ShowItem,               DM_SHOWITEM)
DlgMethod( User,                   DM_USER)

int PushDNParams (lua_State *L, intptr_t Msg, intptr_t Param1, void *Param2)
{
	// Param1
	switch(Msg)
	{
		case DN_CTLCOLORDIALOG:
		case DN_DRAGGED:
		case DN_DRAWDIALOG:
		case DN_DRAWDIALOGDONE:
		case DN_INPUT:
		case DN_RESIZECONSOLE:
			break;

		case DN_CLOSE:
		case DN_CONTROLINPUT:
		case DN_GOTFOCUS:
		case DN_KILLFOCUS:
			if (Param1 >= 0)
				++Param1;
			break;

		case DN_BTNCLICK:
		case DN_CTLCOLORDLGITEM:
		case DN_CTLCOLORDLGLIST:
		case DN_DRAWDLGITEM:
		case DN_DRAWDLGITEMDONE:
		case DN_DROPDOWNOPENED:
		case DN_EDITCHANGE:
		case DN_GETVALUE:
		case DN_HELP:
		case DN_HOTKEY:
		case DN_INITDIALOG:
		case DN_LISTCHANGE:
		case DN_LISTHOTKEY:
			++Param1; // dialog element position
			break;

		default:
			return FALSE;
	}

	lua_pushinteger(L, Msg);       //+1
	lua_pushinteger(L, Param1);    //+2

	// Param2
	switch(Msg)
	{
		case DN_CONTROLINPUT:   // TODO
		case DN_INPUT:          // TODO was: (Msg == DN_MOUSEEVENT)
		case DN_HOTKEY:
			PushInputRecord(L, (const INPUT_RECORD*)Param2);
			break;

		case DN_CTLCOLORDIALOG:
			PushFarColor(L, (struct FarColor*) Param2);
			break;

		case DN_CTLCOLORDLGITEM:
		case DN_CTLCOLORDLGLIST:
		{
			struct FarDialogItemColors* fdic = (struct FarDialogItemColors*) Param2;
			lua_createtable(L, (int)fdic->ColorsCount, 1);
			PutFlagsToTable(L, "Flags", fdic->Flags);
			for(int i=0; i < (int)fdic->ColorsCount; i++)
			{
				PushFarColor(L, &fdic->Colors[i]);
				lua_rawseti(L, -2, i+1);
			}
			break;
		}

		case DN_DRAWDLGITEM:
		case DN_EDITCHANGE:
			PushDlgItem(L, (struct FarDialogItem*)Param2, FALSE);
			break;

		case DN_GETVALUE:
		{
			struct FarGetValue *fgv = (struct FarGetValue*) Param2;
			lua_newtable(L);
			PutIntToTable(L, "GetType", fgv->Type);
			PutIntToTable(L, "ValType", fgv->Value.Type);
			PushFarMacroValue(L, &fgv->Value);
			lua_setfield(L, -2, "Value");
			break;
		}

		case DN_HELP:
			push_utf8_string(L, Param2 ? (wchar_t*)Param2 : L"", -1);
			break;

		case DN_LISTCHANGE:
		case DN_LISTHOTKEY:
			lua_pushinteger(L, (intptr_t)Param2+1);  // make list positions 1-based
			break;

		case DN_RESIZECONSOLE:
		{
			COORD* coord = (COORD*)Param2;
			lua_createtable(L, 0, 2);
			PutIntToTable(L, "X", coord->X);
			PutIntToTable(L, "Y", coord->Y);
			break;
		}

		default:
			lua_pushinteger(L, (intptr_t)Param2);  //+3
			break;
	}

	return TRUE;
}

intptr_t ProcessDNResult(lua_State *L, intptr_t Msg, void *Param2)
{
	intptr_t ret = 0;
	switch(Msg)
	{
		case DN_CTLCOLORDLGLIST:
		case DN_CTLCOLORDLGITEM:
			if ((ret = lua_istable(L,-1)) != 0)
			{
				struct FarDialogItemColors* fdic = (struct FarDialogItemColors*) Param2;
				size_t len = lua_objlen(L, -1);

				if (len > fdic->ColorsCount) len = fdic->ColorsCount;

				for(int i = 0; i < (int)len; i++)
				{
					lua_rawgeti(L, -1, i+1);
					GetFarColor(L, -1, &fdic->Colors[i]);
					lua_pop(L, 1);
				}
			}
			break;

		case DN_CTLCOLORDIALOG:
			ret = GetFarColor(L, -1, (struct FarColor*)Param2);
			break;

		case DN_HELP:
			if ((ret = (intptr_t)utf8_to_utf16(L, -1, NULL)) != 0)
			{
				lua_getfield(L, LUA_REGISTRYINDEX, FAR_DN_STORAGE);
				lua_pushvalue(L, -2);                // keep stack balanced
				lua_setfield(L, -2, "helpstring");   // protect from garbage collector
				lua_pop(L, 1);
			}
			break;

		case DN_GETVALUE:
			if ((ret = lua_istable(L,-1)) != 0)
			{
				struct FarMacroValue tempValue;
				struct FarGetValue *fgv = (struct FarGetValue*) Param2;
				lua_getfield(L, -1, "Value");
				ConvertLuaValue(L, -1, &tempValue);
				if (tempValue.Type == fgv->Value.Type)
				{
					fgv->Value = tempValue;
					if (fgv->Value.Type == FMVT_STRING)
					{
						lua_getfield(L, LUA_REGISTRYINDEX, FAR_DN_STORAGE);
						lua_pushvalue(L, -2);                   // keep stack balanced
						lua_setfield(L, -2, "getvaluestring");  // protect from garbage collector
						lua_pop(L, 1);
					}
				}
				else if (tempValue.Type==FMVT_DOUBLE && fgv->Value.Type==FMVT_INTEGER)
					fgv->Value.Value.Integer = (__int64)tempValue.Value.Double;
				else
					ret = 0;

				lua_pop(L, 1);
			}
			break;

		case DN_KILLFOCUS:
			ret = lua_tointeger(L, -1) - 1;
			break;

		default:
			ret = lua_isnumber(L, -1) ? lua_tointeger(L, -1) : (lua_Integer)lua_toboolean(L, -1);
			break;
	}
	return ret;
}

static intptr_t DoDlgProc(lua_State *L, PSInfo *Info, TDialogData *dd, HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
	if (!dd || dd->wasError)
		return Info->DefDlgProc(hDlg, Msg, Param1, Param2);

	lua_pushlightuserdata(L, dd);        //+1   retrieve the table
	lua_rawget(L, LUA_REGISTRYINDEX);    //+1
	lua_rawgeti(L, -1, 2);               //+2   retrieve the procedure
	lua_rawgeti(L, -2, 3);               //+3   retrieve the handle
	lua_remove(L, -3);                   //+2

	if (Msg == DN_INITDIALOG) {
		lua_pushinteger(L, Msg);                         //+3
		lua_pushinteger(L, Param1 + 1);                  //+4
		lua_rawgeti(L, LUA_REGISTRYINDEX, dd->dataRef);  //+5
	}
	else {
		if (!PushDNParams(L, Msg, Param1, Param2)) {     //+5
			lua_pop(L, 2);
			return Info->DefDlgProc(hDlg, Msg, Param1, Param2);
		}
	}

	if (pcall_msg(L, 4, 1))  //+2
	{
		dd->wasError = TRUE;
		Info->SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
		return Info->DefDlgProc(hDlg, Msg, Param1, Param2);
	}

	intptr_t ret = lua_isnil(L, -1) ?
		Info->DefDlgProc(hDlg, Msg, Param1, Param2) :
		ProcessDNResult(L, Msg, Param2);

	lua_pop(L, 1);
	return ret;
}

static void RemoveDialogFromRegistry(lua_State *L, TDialogData *dd)
{
	luaL_unref(dd->L, LUA_REGISTRYINDEX, dd->dataRef);
	dd->hDlg = INVALID_HANDLE_VALUE;
	lua_pushlightuserdata(L, dd);
	lua_pushnil(L);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

static __inline BOOL NonModal(TDialogData *dd)
{
	return dd && !dd->isModal;
}

intptr_t LF_DlgProc(lua_State *L, HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
	PSInfo *Info = GetPluginData(L)->Info;
	TDialogData *dd = (TDialogData*) Info->SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

	if (Msg == DN_INITDIALOG && dd->hDlg == INVALID_HANDLE_VALUE)
	{
		dd->hDlg = hDlg;
	}

	if (dd)
	{
		L = dd->L; // the dialog may be called from a lua_State other than the main one
	}

	intptr_t ret = DoDlgProc(L, Info, dd, hDlg, Msg, Param1, Param2);
	if (Msg == DN_CLOSE && ret && NonModal(dd))
	{
		Info->SendDlgMessage(hDlg, DM_SETDLGDATA, 0, 0);
		RemoveDialogFromRegistry(L, dd);
	}
	return ret;
}

static int far_DialogInit(lua_State *L)
{
	enum { POS_HISTORIES=1, POS_ITEMS=2 };
	struct FarDialogItem *Items;
	TPluginData *pd = GetPluginData(L);
	GUID Id = { 0 };

	if (lua_type(L,1) == LUA_TSTRING) {
		if (lua_objlen(L,1) >= sizeof(GUID))
			Id = *(const GUID*)lua_tostring(L, 1);
	}
	else if (!lua_isnoneornil(L,1))
		return luaL_typerror(L, 1, "optional string");

	intptr_t X1 = luaL_checkinteger(L, 2);
	intptr_t Y1 = luaL_checkinteger(L, 3);
	intptr_t X2 = luaL_checkinteger(L, 4);
	intptr_t Y2 = luaL_checkinteger(L, 5);
	const wchar_t *HelpTopic = opt_utf8_string(L, 6, NULL);

	luaL_checktype(L, 7, LUA_TTABLE);
	lua_newtable(L);  // create a "histories" table, to prevent history strings
	// from being garbage collected too early
	lua_replace(L, POS_HISTORIES);

	intptr_t ItemsNumber = lua_objlen(L, 7);
	Items = (struct FarDialogItem*)lua_newuserdata(L, ItemsNumber * sizeof(struct FarDialogItem));
	lua_replace(L, POS_ITEMS);

	for (intptr_t i=0; i < ItemsNumber; i++) {
		lua_pushinteger(L, i+1);
		lua_gettable(L, 7);
		if (lua_type(L, -1) == LUA_TTABLE) {
			SetFarDialogItem(L, Items+i, (int)i, POS_HISTORIES);
			lua_pop(L, 1);
		}
		else
			return luaL_error(L, "Items[%d] is not a table", (int)i+1);
	}

	// 8-th parameter (flags)
	flags_t Flags = OptFlags(L, 8, 0);
	TDialogData *dd = NewDialogData(L, pd->Info, INVALID_HANDLE_VALUE, TRUE);

	// 9-th parameter (DlgProc function)
	FARAPIDEFDLGPROC Proc = NULL;
	void *Param = NULL;

	if (lua_isfunction(L, 9))
	{
		Proc = pd->DlgProc;
		Param = dd;
		if (lua_gettop(L) >= 10) {
			lua_pushvalue(L,10);
			dd->dataRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	}

	// Put some values into the registry
	lua_pushlightuserdata(L, dd); // important: index it with dd
	lua_createtable(L, 3, 0);
	lua_pushvalue(L, POS_HISTORIES);  // store the "histories" table
	lua_rawseti(L, -2, 1);

	if (lua_isfunction(L, 9))
	{
		lua_pushvalue(L, 9);    // store the procedure
		lua_rawseti(L, -2, 2);
		lua_pushvalue(L, -3);   // store the handle
		lua_rawseti(L, -2, 3);
	}

	lua_rawset(L, LUA_REGISTRYINDEX);

	dd->hDlg = pd->Info->DialogInit(pd->PluginId, &Id, X1, Y1, X2, Y2, HelpTopic,
	                                Items, ItemsNumber, 0, Flags, Proc, Param);

	if (dd->hDlg == INVALID_HANDLE_VALUE)
	{
		RemoveDialogFromRegistry(L, dd);
		lua_pushnil(L);
	}
	else
	{
		dd->isModal = (Flags&FDLG_NONMODAL) == 0;
	}

	return 1;
}

static void free_dialog(TDialogData* dd)
{
	lua_State* L = dd->L;

	if (dd->isOwned && dd->isModal && dd->hDlg != INVALID_HANDLE_VALUE)
	{
		dd->Info->DialogFree(dd->hDlg);
		RemoveDialogFromRegistry(L, dd);
	}
}

static int far_DialogRun(lua_State *L)
{
	TDialogData* dd = CheckValidDialog(L, 1);
	intptr_t result = dd->Info->DialogRun(dd->hDlg);
	if (result >= 0) ++result;

	if (dd->wasError)
	{
		free_dialog(dd);
		luaL_error(L, "error occured in dialog procedure");
	}

	lua_pushinteger(L, (int)result);
	return 1;
}

static int far_DialogFree(lua_State *L)
{
	free_dialog(CheckDialog(L, 1));
	return 0;
}

static int dialog_tostring(lua_State *L)
{
	TDialogData* dd = CheckDialog(L, 1);

	if (dd->hDlg != INVALID_HANDLE_VALUE)
		lua_pushfstring(L, "%s (%p)", TYPE_DIALOG, dd->hDlg);
	else
		lua_pushfstring(L, "%s (closed)", TYPE_DIALOG);

	return 1;
}

static int dialog_rawhandle(lua_State *L)
{
	TDialogData* dd = CheckDialog(L, 1);

	if (dd->hDlg != INVALID_HANDLE_VALUE)
		lua_pushlightuserdata(L, dd->hDlg);
	else
		lua_pushnil(L);

	return 1;
}

static int far_GetDlgItem(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE hDlg = CheckDialogHandle(L,1);
	int numitem = (int)luaL_checkinteger(L,2) - 1;
	PushDlgItemNum(L, hDlg, numitem, 3, Info);
	return 1;
}

static int far_SetDlgItem(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE hDlg = CheckDialogHandle(L,1);
	int numitem = (int)luaL_checkinteger(L,2) - 1;
	return SetDlgItem(L, hDlg, numitem, 3, Info);
}

static int far_SubscribeDialogDrawEvents(lua_State *L)
{
	GetPluginData(L)->Flags |= PDF_DIALOGEVENTDRAWENABLE;
	return 0;
}

const luaL_Reg dialog_methods[] =
{
	{"__gc",                far_DialogFree},
	{"__tostring",          dialog_tostring},
	{"rawhandle",           dialog_rawhandle},
	{"send",                far_SendDlgMessage},

	PAIR( dlg, AddHistory),
	PAIR( dlg, Close),
	PAIR( dlg, EditUnchangedFlag),
	PAIR( dlg, Enable),
	PAIR( dlg, EnableRedraw),
	PAIR( dlg, GetCheck),
	PAIR( dlg, GetComboboxEvent),
	PAIR( dlg, GetConstTextPtr),
	PAIR( dlg, GetCursorPos),
	PAIR( dlg, GetCursorSize),
	PAIR( dlg, GetDialogInfo),
	PAIR( dlg, GetDialogTitle),
	PAIR( dlg, GetDlgData),
	PAIR( dlg, GetDlgItem),
	PAIR( dlg, GetDlgRect),
	PAIR( dlg, GetDropdownOpened),
	PAIR( dlg, GetEditPosition),
	PAIR( dlg, GetFocus),
	PAIR( dlg, GetItemData),
	PAIR( dlg, GetItemPosition),
	PAIR( dlg, GetSelection),
	PAIR( dlg, GetText),
	PAIR( dlg, Key),
	PAIR( dlg, ListAdd),
	PAIR( dlg, ListAddStr),
	PAIR( dlg, ListDelete),
	PAIR( dlg, ListFindString),
	PAIR( dlg, ListGetCurPos),
	PAIR( dlg, ListGetData),
	PAIR( dlg, ListGetDataSize),
	PAIR( dlg, ListGetItem),
	PAIR( dlg, ListGetTitles),
	PAIR( dlg, ListInfo),
	PAIR( dlg, ListInsert),
	PAIR( dlg, ListSet),
	PAIR( dlg, ListSetCurPos),
	PAIR( dlg, ListSetData),
	PAIR( dlg, ListSetTitles),
	PAIR( dlg, ListSort),
	PAIR( dlg, ListUpdate),
	PAIR( dlg, MoveDialog),
	PAIR( dlg, Redraw),
	PAIR( dlg, ResizeDialog),
	PAIR( dlg, Set3State),
	PAIR( dlg, SetCheck),
	PAIR( dlg, SetComboboxEvent),
	PAIR( dlg, SetCursorPos),
	PAIR( dlg, SetCursorSize),
	PAIR( dlg, SetDlgData),
	PAIR( dlg, SetDlgItem),
	PAIR( dlg, SetDropdownOpened),
	PAIR( dlg, SetEditPosition),
	PAIR( dlg, SetFocus),
	PAIR( dlg, SetHistory),
	PAIR( dlg, SetInputNotify),
	PAIR( dlg, SetItemData),
	PAIR( dlg, SetItemPosition),
	PAIR( dlg, SetMaxTextLength),
	PAIR( dlg, SetSelection),
	PAIR( dlg, SetText),
	PAIR( dlg, SetTextPtr),
	PAIR( dlg, ShowDialog),
	PAIR( dlg, ShowItem),
	PAIR( dlg, User),

	{NULL, NULL},
};

static const luaL_Reg dialog_funcs[] =
{
	PAIR( far, DialogFree),
	PAIR( far, DialogInit),
	PAIR( far, DialogRun),
	PAIR( far, GetDlgItem),
	PAIR( far, SendDlgMessage),
	PAIR( far, SetDlgItem),
	PAIR( far, SubscribeDialogDrawEvents),

	{NULL, NULL},
};

static const char far_Dialog[] =
"function far.Dialog (Id,X1,Y1,X2,Y2,HelpTopic,Items,Flags,DlgProc,Param)\n"
  "local hDlg = far.DialogInit(Id,X1,Y1,X2,Y2,HelpTopic,Items,Flags,DlgProc,Param)\n"
  "if hDlg == nil then return nil end\n"

  "local ret = far.DialogRun(hDlg)\n"
  "for i, item in ipairs(Items) do\n"
    "local newitem = far.GetDlgItem(hDlg, i)\n"
    "if type(item[6]) == 'table' then\n"
      "local pos = far.SendDlgMessage(hDlg, 'DM_LISTGETCURPOS', i, 0)\n"
      "item[6].SelectIndex = pos.SelectPos\n"
    "else\n"
      "item[6] = newitem[6]\n"
    "end\n"
    "item[10] = newitem[10]\n"
  "end\n"

  "far.DialogFree(hDlg)\n"
  "return ret\n"
"end";

int luaopen_dialog(lua_State *L)
{
	luaL_register(L, NULL, dialog_funcs);

	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, FAR_DN_STORAGE);

	luaL_newmetatable(L, TYPE_DIALOG);
	lua_pushvalue(L,-1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, DialogHandleEqual);
	lua_setfield(L, -2, "__eq");
	luaL_register(L, NULL, dialog_methods);

	(void) luaL_dostring(L, far_Dialog);

	return 0;
}
