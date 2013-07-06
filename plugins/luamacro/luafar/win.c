#include <windows.h>
#include "reg.h"
#include "util.h"
#include "ustring.h"
#include "compat52.h"

#define DIM(buff) (sizeof(buff)/sizeof(buff[0]))

extern const char* VirtualKeyStrings[256];
extern void pushFileTime(lua_State *L, const FILETIME *ft);
extern void NewVirtualKeyTable(lua_State* L, BOOL twoways);
extern BOOL dir_exist(const wchar_t* path);

// os.getenv does not always work correctly, hence the following.
static int win_GetEnv(lua_State *L)
{
	const wchar_t* name = check_utf8_string(L, 1, NULL);
	wchar_t buf[256], *p;
	DWORD res, size;
	res = GetEnvironmentVariableW(name, buf, DIM(buf));

	if(res == 0) return 0;

	if(res < DIM(buf)) return push_utf8_string(L, buf, -1), 1;

	size = res + 1;
	p = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));
	res = GetEnvironmentVariableW(name, p, size);

	if(res > 0 && res < size)
		return push_utf8_string(L, p, -1), 1;

	return 0;
}

static int win_SetEnv(lua_State *L)
{
	const wchar_t* name = check_utf8_string(L, 1, NULL);
	const wchar_t* value = opt_utf8_string(L, 2, NULL);
	BOOL res = SetEnvironmentVariableW(name, value);
	return lua_pushboolean(L, res), 1;
}

static HKEY CheckHKey(lua_State *L, int pos)
{
	const char *str = luaL_checkstring(L, pos);

	if(!strcmp(str, "HKLM")) return HKEY_LOCAL_MACHINE;

	if(!strcmp(str, "HKCC")) return HKEY_CURRENT_CONFIG;

	if(!strcmp(str, "HKCR")) return HKEY_CLASSES_ROOT;

	if(!strcmp(str, "HKCU")) return HKEY_CURRENT_USER;

	if(!strcmp(str, "HKU"))  return HKEY_USERS;

	luaL_argerror(L, pos, "must be 'HKLM', 'HKCC', 'HKCR', 'HKCU' or 'HKU'");
	return 0;
}

// SetRegKey (Root, Key, ValueName, DataType, ValueData)
//   Root:       root, [string], one of "HKLM", "HKCC", "HKCR", "HKCU", "HKU"
//   Key:        registry key, [string]
//   ValueName:  registry value name, [string]
//   DataType:   "string","expandstring","multistring","dword" or "binary", [string]
//   ValueData:  registry value data, [string | number | lstring]
// Returns:
//   nothing.
static int win_SetRegKey(lua_State *L)
{
	HKEY hRoot           = CheckHKey(L, 1);
	wchar_t* Key         = (wchar_t*)check_utf8_string(L, 2, NULL);
	wchar_t* ValueName   = (wchar_t*)check_utf8_string(L, 3, NULL);
	const char* DataType = luaL_checkstring(L, 4);
	size_t len;

	if(!strcmp("string", DataType))
	{
		SetRegKeyStr(hRoot, Key, ValueName,
		             (wchar_t*)check_utf8_string(L, 5, NULL));
	}
	else if(!strcmp("dword", DataType))
	{
		SetRegKeyDword(hRoot, Key, ValueName, (DWORD)luaL_checkinteger(L, 5));
	}
	else if(!strcmp("binary", DataType))
	{
		BYTE *data = (BYTE*)luaL_checklstring(L, 5, &len);
		SetRegKeyArr(hRoot, Key, ValueName, data, (DWORD)len);
	}
	else if(!strcmp("expandstring", DataType))
	{
		const wchar_t* data = check_utf8_string(L, 5, NULL);
		HKEY hKey = CreateRegKey(hRoot, Key);
		RegSetValueExW(hKey, ValueName, 0, REG_EXPAND_SZ, (BYTE*)data, 1+(DWORD)wcslen(data));
		RegCloseKey(hKey);
	}
	else if(!strcmp("multistring", DataType))
	{
		const char* data = luaL_checklstring(L, 5, &len);
		HKEY hKey = CreateRegKey(hRoot, Key);
		RegSetValueExW(hKey, ValueName, 0, REG_MULTI_SZ, (BYTE*)data, (DWORD)len);
		RegCloseKey(hKey);
	}
	else
		luaL_argerror(L, 5, "unsupported value type");

	return 0;
}

// ValueData, DataType = GetRegKey (Root, Key, ValueName)
//   Root:       [string], one of "HKLM", "HKCC", "HKCR", "HKCU", "HKU"
//   Key:        registry key, [string]
//   ValueName:  registry value name, [string]
//   ValueData:  registry value data, [string | number | lstring]
//   DataType:   "string", "expandstring", "multistring", "dword"
//                    or "binary", [string]
static int win_GetRegKey(lua_State *L)
{
	HKEY hKey;
	DWORD datatype, datasize;
	char *data;
	LONG ret;
	HKEY hRoot = CheckHKey(L, 1);
	wchar_t* Key = (wchar_t*)check_utf8_string(L, 2, NULL);
	const wchar_t* ValueName = check_utf8_string(L, 3, NULL);
	hKey = OpenRegKey(hRoot, Key);

	if(hKey == NULL)
	{
		lua_pushnil(L);
		lua_pushstring(L, "OpenRegKey failed.");
		return 2;
	}

	RegQueryValueExW(hKey, ValueName, NULL, &datatype, NULL, &datasize);
	data = (char*) malloc(datasize);
	ret = RegQueryValueExW(hKey, ValueName, NULL, &datatype, (BYTE*)data, &datasize);
	RegCloseKey(hKey);

	if(ret != ERROR_SUCCESS)
	{
		lua_pushnil(L);
		lua_pushstring(L, "RegQueryValueEx failed.");
	}
	else
	{
		switch(datatype)
		{
			case REG_BINARY:
				lua_pushlstring(L, data, datasize);
				lua_pushstring(L, "binary");
				break;
			case REG_DWORD:
				lua_pushinteger(L, *(int*)data);
				lua_pushstring(L, "dword");
				break;
			case REG_SZ:
				push_utf8_string(L, (wchar_t*)data, -1);
				lua_pushstring(L, "string");
				break;
			case REG_EXPAND_SZ:
				push_utf8_string(L, (wchar_t*)data, -1);
				lua_pushstring(L, "expandstring");
				break;
			case REG_MULTI_SZ:
				push_utf8_string(L, (wchar_t*)data, datasize/sizeof(wchar_t));
				lua_pushstring(L, "multistring");
				break;
			default:
				lua_pushnil(L);
				lua_pushstring(L, "unsupported value type");
				break;
		}
	}

	free(data);
	return 2;
}

// Result = DeleteRegKey (Root, Key)
//   Root:      [string], one of "HKLM", "HKCC", "HKCR", "HKCU", "HKU"
//   Key:       registry key, [string]
//   Result:    TRUE if success, FALSE if failure, [boolean]
static int win_DeleteRegKey(lua_State *L)
{
	HKEY hRoot = CheckHKey(L, 1);
	const wchar_t* Key = check_utf8_string(L, 2, NULL);
	long res = RegDeleteKeyW(hRoot, Key);
	lua_pushboolean(L, res==ERROR_SUCCESS);
	return 1;
}

// Result = EnumRegKey (Root, Key, Index)
//   Root:      [string], one of "HKLM", "HKCC", "HKCR", "HKCU", "HKU"
//   Key:       registry key, [string]
//   Index:     integer
//   Result:    string or nil
static int win_EnumRegKey(lua_State *L)
{
	HKEY hKey;
	LONG ret;
	HKEY hRoot = CheckHKey(L, 1);
	wchar_t* Key = (wchar_t*)check_utf8_string(L, 2, NULL);
	DWORD dwIndex = (DWORD)luaL_checkinteger(L, 3);
	wchar_t Name[512];
	DWORD NameSize = DIM(Name);
	FILETIME LastWriteTime;

	if(RegOpenKeyExW(hRoot, Key, 0, KEY_ENUMERATE_SUB_KEYS, &hKey)!=ERROR_SUCCESS)
	{
		lua_pushnil(L);
		lua_pushstring(L, "RegOpenKeyExW failed.");
		return 2;
	}

	ret = RegEnumKeyEx(
		hKey,             // handle of key to enumerate
		dwIndex,          // index of subkey to enumerate
		Name,             // address of buffer for subkey name
		&NameSize,        // address for size of subkey buffer
		NULL,             // reserved
		NULL,             // address of buffer for class string
		NULL,             // address for size of class buffer
		&LastWriteTime);  // address for time key last written to

	RegCloseKey(hKey);

	if (ret == ERROR_SUCCESS)
		push_utf8_string(L, Name, NameSize);
	else
		lua_pushnil(L);

	return 1;
}

// Result = EnumRegValue (Root, Key, Index)
//   Root:      [string], one of "HKLM", "HKCC", "HKCR", "HKCU", "HKU"
//   Key:       registry key, [string]
//   Index:     integer
//   Result:    string or nil
static int win_EnumRegValue(lua_State *L)
{
	HKEY hKey;
	LONG ret;
	HKEY hRoot = CheckHKey(L, 1);
	wchar_t* Key = (wchar_t*)check_utf8_string(L, 2, NULL);
	DWORD dwIndex = (DWORD)luaL_checkinteger(L, 3);
	wchar_t Name[512];
	DWORD NameSize = DIM(Name);
	DWORD Type;

	if(RegOpenKeyExW(hRoot, Key, 0, KEY_QUERY_VALUE, &hKey)!=ERROR_SUCCESS)
	{
		lua_pushnil(L);
		lua_pushstring(L, "RegOpenKeyExW failed.");
		return 2;
	}

	ret = RegEnumValue(
    hKey,             // handle of key to query
    dwIndex,          // index of value to query
    Name,             // address of buffer for value string
    &NameSize,        // address for size of value buffer
    NULL,             // reserved
    &Type,            // address of buffer for type code
    NULL,             // address of buffer for value data
    NULL              // address for size of data buffer
   );

	RegCloseKey(hKey);

	if (ret == ERROR_SUCCESS)
		push_utf8_string(L, Name, NameSize);
	else
		lua_pushnil(L);

	return 1;
}

// Based on "CheckForEsc" function, by Ivan Sintyurin (spinoza@mail.ru)
static WORD ExtractKey()
{
	INPUT_RECORD rec;
	DWORD ReadCount;
	HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);

	while(PeekConsoleInput(hConInp,&rec,1,&ReadCount), ReadCount)
	{
		ReadConsoleInput(hConInp,&rec,1,&ReadCount);

		if(rec.EventType==KEY_EVENT && rec.Event.KeyEvent.bKeyDown)
			return rec.Event.KeyEvent.wVirtualKeyCode;
	}

	return 0;
}

// result = ExtractKey()
// -- general purpose function; not FAR dependent
static int win_ExtractKey(lua_State *L)
{
	WORD vKey = ExtractKey() & 0xff;

	if(vKey && VirtualKeyStrings[vKey])
		lua_pushstring(L, VirtualKeyStrings[vKey]);
	else
		lua_pushnil(L);

	return 1;
}

static void PushWinFindData(lua_State *L, const WIN32_FIND_DATAW *FData)
{
	lua_createtable(L, 0, 7);
	PutAttrToTable(L,                          FData->dwFileAttributes);
	PutNumToTable(L, "FileSize", FData->nFileSizeLow + 65536.*65536.*FData->nFileSizeHigh);
	PutFileTimeToTable(L, "LastWriteTime",     FData->ftLastWriteTime);
	PutFileTimeToTable(L, "LastAccessTime",    FData->ftLastAccessTime);
	PutFileTimeToTable(L, "CreationTime",      FData->ftCreationTime);
	PutWStrToTable(L, "FileName",              FData->cFileName, -1);
	PutWStrToTable(L, "AlternateFileName",     FData->cAlternateFileName, -1);
}

static int win_GetFileInfo(lua_State *L)
{
	WIN32_FIND_DATAW fd;
	const wchar_t *fname = check_utf8_string(L, 1, NULL);
	HANDLE h = FindFirstFileW(fname, &fd);

	if(h == INVALID_HANDLE_VALUE)
		lua_pushnil(L);
	else
	{
		PushWinFindData(L, &fd);
		FindClose(h);
	}

	return 1;
}

static void pushSystemTime(lua_State *L, const SYSTEMTIME *st)
{
	lua_createtable(L, 0, 8);
	PutIntToTable(L, "wYear", st->wYear);
	PutIntToTable(L, "wMonth", st->wMonth);
	PutIntToTable(L, "wDayOfWeek", st->wDayOfWeek);
	PutIntToTable(L, "wDay", st->wDay);
	PutIntToTable(L, "wHour", st->wHour);
	PutIntToTable(L, "wMinute", st->wMinute);
	PutIntToTable(L, "wSecond", st->wSecond);
	PutIntToTable(L, "wMilliseconds", st->wMilliseconds);
}

static int win_GetSystemTimeAsFileTime(lua_State *L)
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	pushFileTime(L, &ft);
	return 1;
}

static int win_FileTimeToSystemTime(lua_State *L)
{
	FILETIME ft;
	SYSTEMTIME st;
	long long llFileTime = 10000 * (long long) luaL_checknumber(L, 1);
	ft.dwLowDateTime = llFileTime & 0xFFFFFFFF;
	ft.dwHighDateTime = llFileTime >> 32;

	if(! FileTimeToSystemTime(&ft, &st))
		return SysErrorReturn(L);

	pushSystemTime(L, &st);
	return 1;
}

static int win_SystemTimeToFileTime(lua_State *L)
{
	FILETIME ft;
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_settop(L, 1);
	st.wYear         = GetOptIntFromTable(L, "wYear", 0);
	st.wMonth        = GetOptIntFromTable(L, "wMonth", 0);
	st.wDayOfWeek    = GetOptIntFromTable(L, "wDayOfWeek", 0);
	st.wDay          = GetOptIntFromTable(L, "wDay", 0);
	st.wHour         = GetOptIntFromTable(L, "wHour", 0);
	st.wMinute       = GetOptIntFromTable(L, "wMinute", 0);
	st.wSecond       = GetOptIntFromTable(L, "wSecond", 0);
	st.wMilliseconds = GetOptIntFromTable(L, "wMilliseconds", 0);

	if(! SystemTimeToFileTime(&st, &ft))
		return SysErrorReturn(L);

	pushFileTime(L, &ft);
	return 1;
}

static int win_FileTimeToLocalFileTime(lua_State *L)
{
	FILETIME ft, local_ft;
	long long llFileTime = 10000 * (long long) luaL_checknumber(L, 1);
	ft.dwLowDateTime = llFileTime & 0xFFFFFFFF;
	ft.dwHighDateTime = llFileTime >> 32;

	if(FileTimeToLocalFileTime(&ft, &local_ft))
		pushFileTime(L, &local_ft);
	else
		return SysErrorReturn(L);

	return 1;
}

static int win_CompareString(lua_State *L)
{
	DWORD dwFlags = 0;
	size_t len1, len2;
	int result;
	const wchar_t *ws1  = check_utf8_string(L, 1, &len1);
	const wchar_t *ws2  = check_utf8_string(L, 2, &len2);
	const char *sLocale = luaL_optstring(L, 3, "");
	const char *sFlags  = luaL_optstring(L, 4, "");
	LCID Locale = LOCALE_USER_DEFAULT;

	if(!strcmp(sLocale, "s")) Locale = LOCALE_SYSTEM_DEFAULT;
	else if(!strcmp(sLocale, "n")) Locale = LOCALE_NEUTRAL;

	if(strchr(sFlags, 'c')) dwFlags |= NORM_IGNORECASE;

	if(strchr(sFlags, 'k')) dwFlags |= NORM_IGNOREKANATYPE;

	if(strchr(sFlags, 'n')) dwFlags |= NORM_IGNORENONSPACE;

	if(strchr(sFlags, 's')) dwFlags |= NORM_IGNORESYMBOLS;

	if(strchr(sFlags, 'w')) dwFlags |= NORM_IGNOREWIDTH;

	if(strchr(sFlags, 'S')) dwFlags |= SORT_STRINGSORT;

	result = CompareStringW(Locale, dwFlags, ws1, (int)len1, ws2, (int)len2) - 2;
	(result == -2) ? lua_pushnil(L) : lua_pushinteger(L, result);
	return 1;
}

static int win_wcscmp(lua_State *L)
{
	const wchar_t *ws1  = check_utf8_string(L, 1, NULL);
	const wchar_t *ws2  = check_utf8_string(L, 2, NULL);
	int insens = lua_toboolean(L, 3);
	lua_pushinteger(L, (insens ? _wcsicmp : wcscmp)(ws1, ws2));
	return 1;
}

static int win_GetVirtualKeys(lua_State *L)
{
	NewVirtualKeyTable(L, TRUE);
	return 1;
}

static int win_GetConsoleScreenBufferInfo(lua_State* L)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

	if(!GetConsoleScreenBufferInfo(h, &info))
		return lua_pushnil(L), 1;

	lua_createtable(L, 0, 11);
	PutIntToTable(L, "SizeX",              info.dwSize.X);
	PutIntToTable(L, "SizeY",              info.dwSize.Y);
	PutIntToTable(L, "CursorPositionX",    info.dwCursorPosition.X);
	PutIntToTable(L, "CursorPositionY",    info.dwCursorPosition.Y);
	PutIntToTable(L, "Attributes",         info.wAttributes);
	PutIntToTable(L, "WindowLeft",         info.srWindow.Left);
	PutIntToTable(L, "WindowTop",          info.srWindow.Top);
	PutIntToTable(L, "WindowRight",        info.srWindow.Right);
	PutIntToTable(L, "WindowBottom",       info.srWindow.Bottom);
	PutIntToTable(L, "MaximumWindowSizeX", info.dwMaximumWindowSize.X);
	PutIntToTable(L, "MaximumWindowSizeY", info.dwMaximumWindowSize.Y);
	return 1;
}

static int win_CopyFile(lua_State *L)
{
	const wchar_t* src = check_utf8_string(L, 1, NULL);
	const wchar_t* trg = check_utf8_string(L, 2, NULL);
	BOOL fail_if_exists = FALSE; // default = overwrite the target

	if(lua_gettop(L) > 2)
		fail_if_exists = lua_toboolean(L,3);

	if(CopyFileW(src, trg, fail_if_exists))
		return lua_pushboolean(L, 1), 1;

	return SysErrorReturn(L);
}

static int win_MoveFile(lua_State *L)
{
	const wchar_t* src = check_utf8_string(L, 1, NULL);
	const wchar_t* trg = check_utf8_string(L, 2, NULL);
	const char* sFlags = luaL_optstring(L, 3, NULL);
	int flags = 0;

	if(sFlags)
	{
		if(strchr(sFlags, 'c')) flags |= MOVEFILE_COPY_ALLOWED;
		else if(strchr(sFlags, 'd')) flags |= MOVEFILE_DELAY_UNTIL_REBOOT;
		else if(strchr(sFlags, 'r')) flags |= MOVEFILE_REPLACE_EXISTING;
		else if(strchr(sFlags, 'w')) flags |= MOVEFILE_WRITE_THROUGH;
	}

	if(MoveFileExW(src, trg, flags))
		return lua_pushboolean(L, 1), 1;

	return SysErrorReturn(L);
}

static int win_DeleteFile(lua_State *L)
{
	if(DeleteFileW(check_utf8_string(L, 1, NULL)))
		return lua_pushboolean(L, 1), 1;

	return SysErrorReturn(L);
}

static BOOL mkdir(const wchar_t* aPath)
{
  wchar_t **Ends = NULL;
  int posexist=-1, posfail=-1, num_ends=0, i;
	wchar_t *Path = _wcsdup(aPath), *pos;

	// Replace / with \ and count "ends".
	for(pos=Path; *pos; )
	{
		if (*pos==L'\\' || *pos==L'/')
		{
			num_ends++;
			do *pos++ = L'\\'; while (*pos==L'\\' || *pos==L'/');
		}
		else pos++;
	}
	if (pos > Path && pos[-1] != L'\\') num_ends++;

	// Acquire positions of "ends".
	Ends = (wchar_t**) malloc(num_ends*sizeof(wchar_t*));
	for(pos=Path,i=0; *pos; )
	{
		if (*pos==L'\\')
		{
			do pos++; while (*pos==L'\\');
			Ends[i++] = pos;
		}
		else pos++;
	}
	if (pos > Path && pos[-1] != L'\\') Ends[i] = pos;

  // Find end position of the longest existing directory in the given path.
	for (i=num_ends-1; i>=0; i--)
	{
		DWORD attr;
		wchar_t tempchar = *Ends[i];
		*Ends[i] = 0;
		attr = GetFileAttributesW(Path);
		*Ends[i] = tempchar;
		if (attr != 0xFFFFFFFFU)
		{
			if (attr & FILE_ATTRIBUTE_DIRECTORY)
			{
				posexist = i; break;
			}
			free(Ends); free(Path);
			return FALSE;
		}
	}

  // Create directories one by one. Store "failed end position" when failed.
	for (i=posexist+1; i<num_ends; i++)
	{
		BOOL result;
		wchar_t tempchar = *Ends[i];
		*Ends[i] = 0;
		result = CreateDirectoryW(Path,NULL);
		*Ends[i] = tempchar;
		if (!result)
		{
			posfail = i; break;
		}
	}

  // In case of failure, remove the directories we already created, don't leave garbage.
	if (posfail >= 0)
	{
		for (i = posfail-1; i>posexist; i--)
		{
			*Ends[i] = 0; RemoveDirectoryW(Path);
		}
	}

  free(Ends); free(Path);
	return posfail < 0;
}

static int win_CreateDir(lua_State *L)
{
	BOOL result, opt_tolerant, opt_original;
	const wchar_t* path = check_utf8_string(L, 1, NULL);
	const char* flags = "";

	if (lua_type(L,2) == LUA_TSTRING)
		flags = lua_tostring(L,2);
	else if (lua_toboolean(L,2))
		flags = "t";

	opt_tolerant = strchr(flags,'t') != NULL;
	opt_original = strchr(flags,'o') != NULL;

	if(dir_exist(path))
	{
		if (opt_tolerant) return lua_pushboolean(L,1), 1;

		return lua_pushnil(L), lua_pushliteral(L, "directory already exists"), 2;
	}

	result = opt_original ? CreateDirectoryW(path,NULL) : mkdir(path);
	if(result)
		return lua_pushboolean(L, 1), 1;

	return SysErrorReturn(L);
}

static int win_RemoveDir(lua_State *L)
{
	if(RemoveDirectoryW(check_utf8_string(L, 1, NULL)))
		return lua_pushboolean(L, 1), 1;

	return SysErrorReturn(L);
}

static int win_ShellExecute(lua_State *L)
{
	HWND hwnd = lua_isuserdata(L, 1) ? lua_touserdata(L, 1) : NULL;
	const wchar_t* lpOperation = opt_utf8_string(L, 2, NULL);
	const wchar_t* lpFile = check_utf8_string(L, 3, NULL);
	const wchar_t* lpParameters = opt_utf8_string(L, 4, NULL);
	const wchar_t* lpDirectory = opt_utf8_string(L, 5, NULL);
	INT nShowCmd = (INT)luaL_optinteger(L, 6, SW_SHOWNORMAL);
	HINSTANCE hinst = ShellExecuteW(
	                      hwnd,           // handle to parent window
	                      lpOperation,    // pointer to string that specifies operation to perform
	                      lpFile,         // pointer to filename or folder name string
	                      lpParameters,   // pointer to string that specifies executable-file parameters
	                      lpDirectory,    // pointer to string that specifies default directory
	                      nShowCmd        // whether file is shown when opened
	                  );
	lua_pushinteger(L, (INT_PTR)hinst);
	return 1;
}

const luaL_Reg win_funcs[] =
{
	{"CompareString",       win_CompareString},
	{"CopyFile",            win_CopyFile},
	{"CreateDir",           win_CreateDir},
	{"DeleteFile",          win_DeleteFile},
	{"DeleteRegKey",        win_DeleteRegKey},
	{"EnumRegKey",          win_EnumRegKey},
	{"EnumRegValue",        win_EnumRegValue},
	{"ExtractKey",          win_ExtractKey},
	{"FileTimeToLocalFileTime", win_FileTimeToLocalFileTime},
	{"FileTimeToSystemTime",win_FileTimeToSystemTime},
	{"GetConsoleScreenBufferInfo", win_GetConsoleScreenBufferInfo},
	{"GetEnv",              win_GetEnv},
	{"GetFileInfo",         win_GetFileInfo},
	{"GetRegKey",           win_GetRegKey},
	{"GetSystemTimeAsFileTime", win_GetSystemTimeAsFileTime},
	{"GetVirtualKeys",      win_GetVirtualKeys},
	{"MoveFile",            win_MoveFile},
	{"RemoveDir",           win_RemoveDir},
	{"RenameFile",          win_MoveFile}, // alias
	{"SetEnv",              win_SetEnv},
	{"SetRegKey",           win_SetRegKey},
	{"ShellExecute",        win_ShellExecute},
	{"SystemTimeToFileTime",win_SystemTimeToFileTime},
	{"wcscmp",              win_wcscmp},

	{"EnumSystemCodePages", ustring_EnumSystemCodePages},
	{"GetACP",              ustring_GetACP},
	{"GetCPInfo",           ustring_GetCPInfo},
	{"GetDriveType",        ustring_GetDriveType},
	{"GetFileAttr",         ustring_GetFileAttr},
	{"GetLogicalDriveStrings",ustring_GetLogicalDriveStrings},
	{"GetOEMCP",            ustring_GetOEMCP},
	{"GlobalMemoryStatus",  ustring_GlobalMemoryStatus},
	{"MultiByteToWideChar", ustring_MultiByteToWideChar },
	{"OemToUtf8",           ustring_OemToUtf8},
	{"SHGetFolderPath",     ustring_SHGetFolderPath},
	{"SearchPath",          ustring_SearchPath},
	{"SetFileAttr",         ustring_SetFileAttr},
	{"Sleep",               ustring_Sleep},
	{"Utf16ToUtf8",         ustring_Utf16ToUtf8},
	{"Utf8ToOem",           ustring_Utf8ToOem},
	{"Utf8ToUtf16",         ustring_Utf8ToUtf16},
	{"Uuid",                ustring_Uuid},
	{"WideCharToMultiByte", ustring_WideCharToMultiByte},
	{"subW",                ustring_sub},
	{"lenW",                ustring_len},

	{NULL, NULL}
};

LUALIB_API int luaopen_win(lua_State *L)
{
	luaL_register(L, "win", win_funcs);
	return 1;
}
