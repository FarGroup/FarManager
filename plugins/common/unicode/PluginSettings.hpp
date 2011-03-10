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
		if (SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,(INT_PTR)&settings))
			handle = settings.Handle;
	}

	~PluginSettings()
	{
	    SettingsControl(handle,SCTL_FREE,0,0);
	}

	int CreateSubKey(int Root, const wchar_t *Name)
	{
		FarSettingsValue value={Root,Name};
		return SettingsControl(handle,SCTL_SUBKEY,0,(INT_PTR)&value);
	}

	const wchar_t *Get(int Root, const wchar_t *Name, const wchar_t *Default)
	{
		FarSettingsItem item={Root,Name,FST_STRING};
		if (SettingsControl(handle,SCTL_GET,0,(INT_PTR)&item))
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
		FarSettingsItem item={Root,Name,FST_QWORD};
		if (SettingsControl(handle,SCTL_GET,0,(INT_PTR)&item))
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

	bool Set(int Root, const wchar_t *Name, const wchar_t *Value)
	{
		FarSettingsItem item={Root,Name,FST_STRING};
		item.String=Value;
		return SettingsControl(handle,SCTL_SET,0,(INT_PTR)&item)!=FALSE;
	}

	bool Set(int Root, const wchar_t *Name, unsigned __int64 Value)
	{
		FarSettingsItem item={Root,Name,FST_QWORD};
		item.Number=Value;
		return SettingsControl(handle,SCTL_SET,0,(INT_PTR)&item)!=FALSE;
	}

	bool Set(int Root, const wchar_t *Name, __int64 Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, int Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, unsigned int Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, DWORD Value) { return Set(Root,Name,(unsigned __int64)Value); }
	bool Set(int Root, const wchar_t *Name, bool Value) { return Set(Root,Name,Value?1ull:0ull); }
};

#endif
