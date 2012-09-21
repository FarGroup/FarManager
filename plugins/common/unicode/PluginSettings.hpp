#ifndef __PLUGINSETTINGS_HPP__
#define __PLUGINSETTINGS_HPP__

#include "plugin.hpp"

class PluginSettings
{
	HANDLE handle;
	FARAPISETTINGSCONTROL SettingsControl;

public:

	PluginSettings(const GUID &guid, FARAPISETTINGSCONTROL SettingsControl)
	{
		this->SettingsControl = SettingsControl;
		handle = INVALID_HANDLE_VALUE;

		FarSettingsCreate settings={sizeof(FarSettingsCreate),guid,handle};
		if (SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
			handle = settings.Handle;
	}

	~PluginSettings()
	{
	    SettingsControl(handle,SCTL_FREE,0,0);
	}

	int CreateSubKey(int Root, const wchar_t *Name)
	{
		FarSettingsValue value={sizeof(FarSettingsValue),Root,Name};
		return (int)SettingsControl(handle,SCTL_CREATESUBKEY,0,&value);
	}

	int OpenSubKey(int Root, const wchar_t *Name)
	{
		FarSettingsValue value={sizeof(FarSettingsValue),Root,Name};
		return (int)SettingsControl(handle,SCTL_OPENSUBKEY,0,&value);
	}

	bool DeleteSubKey(int Root)
	{
		FarSettingsValue value={sizeof(FarSettingsValue),Root,nullptr};
		return (int)SettingsControl(handle,SCTL_DELETE,0,&value) ? true : false;
	}

	bool DeleteValue(int Root, const wchar_t *Name)
	{
		FarSettingsValue value={sizeof(FarSettingsValue),Root,Name};
		return (int)SettingsControl(handle,SCTL_DELETE,0,&value) ? true : false;
	}

	const wchar_t *Get(int Root, const wchar_t *Name, const wchar_t *Default)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_STRING};
		if (SettingsControl(handle,SCTL_GET,0,&item))
		{
			return item.String;
		}
		return Default;
	}

	void Get(int Root, const wchar_t *Name, wchar_t *Value, size_t Size, const wchar_t *Default)
	{
		lstrcpyn(Value, Get(Root,Name,Default), (int)Size);
	}

	unsigned __int64 Get(int Root, const wchar_t *Name, unsigned __int64 Default)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_QWORD};
		if (SettingsControl(handle,SCTL_GET,0,&item))
		{
			return item.Number;
		}
		return Default;
	}

	__int64      Get(int Root, const wchar_t *Name, __int64 Default) { return (__int64)Get(Root,Name,(unsigned __int64)Default); }
	int          Get(int Root, const wchar_t *Name, int Default)  { return (int)Get(Root,Name,(unsigned __int64)Default); }
	unsigned int Get(int Root, const wchar_t *Name, unsigned int Default) { return (unsigned int)Get(Root,Name,(unsigned __int64)Default); }
	DWORD        Get(int Root, const wchar_t *Name, DWORD Default) { return (DWORD)Get(Root,Name,(unsigned __int64)Default); }
	bool         Get(int Root, const wchar_t *Name, bool Default) { return Get(Root,Name,Default?1ull:0ull)?true:false; }

	size_t Get(int Root, const wchar_t *Name, void *Value, size_t Size)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_DATA};
		if (SettingsControl(handle,SCTL_GET,0,&item))
		{
			Size = (item.Data.Size>Size)?Size:item.Data.Size;
			memcpy(Value,item.Data.Data,Size);
			return Size;
		}
		return 0;
	}

	bool Set(int Root, const wchar_t *Name, const wchar_t *Value)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_STRING};
		item.String=Value;
		return SettingsControl(handle,SCTL_SET,0,&item)!=FALSE;
	}

	bool Set(int Root, const wchar_t *Name, unsigned __int64 Value)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_QWORD};
		item.Number=Value;
		return SettingsControl(handle,SCTL_SET,0,&item)!=FALSE;
	}

	bool Set(int Root, const wchar_t *Name, __int64 Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, int Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, unsigned int Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, DWORD Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, bool Value) { return Set(Root,Name,Value?1ull:0ull); }

	bool Set(int Root, const wchar_t *Name, const void *Value, size_t Size)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_DATA};
		item.Data.Size=Size;
		item.Data.Data=Value;
		return SettingsControl(handle,SCTL_SET,0,&item)!=FALSE;
	}
};

#endif
