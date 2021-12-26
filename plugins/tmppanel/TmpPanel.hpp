#ifndef TMPPANEL_HPP_DEEE2CB2_2167_4B14_B8D6_08676A9F3CEB
#define TMPPANEL_HPP_DEEE2CB2_2167_4B14_B8D6_08676A9F3CEB
#pragma once

/*
TMPPANEL.HPP

Temporary panel header file

*/

#include "headers.hpp"

struct PluginPanel
{
	void clear()
	{
		Items.clear();
		StringData.clear();
		OwnerData.clear();
	}

	std::vector<PluginPanelItem> Items;
	// Lists for stable item addresses
	std::list<string> StringData;
	std::list<string> OwnerData;
	unsigned int OpenFrom;
};

struct shared_data
{
	PluginPanel CommonPanels[10];
	size_t CurrentCommonPanel{};
};

inline shared_data* SharedData;

inline PluginStartupInfo PsInfo;
inline FarStandardFunctions FSF;

#define NT_MAX_PATH 32768

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8_LO    0xBBEF
#define SIGN_UTF8_HI    0xBF

const wchar_t* GetMsg(int MsgId);
std::pair<string_view, string_view> ParseParam(string_view Str);
void GoToFile(const string& Target, bool AnotherPanel);
void WFD2FFD(const WIN32_FIND_DATA &wfd, PluginPanelItem &ffd, string* NameData);
string FormNtPath(string_view Path);
string ExpandEnvStrs(string_view Input);
string FindListFile(const string& Filename);
string GetFullPath(string_view Input);
bool IsTextUTF8(const char* Buffer,size_t Length);
bool GetFileInfoAndValidate(string_view FilePath, PluginPanelItem& FindData, string& NameData, bool Any);

#endif // TMPPANEL_HPP_DEEE2CB2_2167_4B14_B8D6_08676A9F3CEB
