/*
TMPMIX.CPP

Temporary panel miscellaneous utility functions

*/

#include "plugin.hpp"

#include "TmpPanel.hpp"
#include "guid.hpp"

#include <enum_tokens.hpp>
#include <scope_exit.hpp>
#include <smart_ptr.hpp>
#include <string_utils.hpp>

const wchar_t* GetMsg(int MsgId)
{
	return(PsInfo.GetMsg(&MainGuid, MsgId));
}

std::pair<string_view, string_view> ParseParam(string_view Str)
{
	if (!Str.starts_with(L'|'))
		return {};

	auto Param = Str.substr(1);
	const auto ParamEnd = Param.find(L'|');
	if (ParamEnd == Param.npos)
		return {};

	const auto Tail = trim_left(Param.substr(ParamEnd + 1));
	Param.remove_suffix(Param.size() - ParamEnd);

	return { Param, Tail };
}

void GoToFile(const string& Target, bool AnotherPanel)
{
	const auto CleanTarget = unquote(trim(string_view(Target)));
	const string Name = FSF.PointToName(CleanTarget.c_str());
	const string Dir = CleanTarget.substr(0, CleanTarget.size() - Name.size());

	const auto PanelHandle = AnotherPanel? PANEL_PASSIVE : PANEL_ACTIVE;

	if (!Dir.empty())
	{
		FarPanelDirectory dirInfo = { sizeof(dirInfo), Dir.c_str() };
		PsInfo.PanelControl(PanelHandle, FCTL_SETPANELDIRECTORY, 0, &dirInfo);
	}

	PanelInfo PInfo{ sizeof(PanelInfo) };
	PsInfo.PanelControl(PanelHandle, FCTL_GETPANELINFO, 0, &PInfo);

	PanelRedrawInfo PRI{ sizeof(PanelRedrawInfo) };
	PRI.CurrentItem = PInfo.CurrentItem;
	PRI.TopPanelItem = PInfo.TopPanelItem;

	for (size_t J = 0; J < PInfo.ItemsNumber; J++)
	{
		const size_t Size = PsInfo.PanelControl(PanelHandle, FCTL_GETPANELITEM, J, {});
		const block_ptr<PluginPanelItem> ppi(Size);
		FarGetPluginPanelItem gpi{ sizeof(gpi), Size, ppi.data() };
		PsInfo.PanelControl(PanelHandle, FCTL_GETPANELITEM, J, &gpi);

		if (!FSF.LStricmp(Name.c_str(), FSF.PointToName(ppi->FileName)))
		{
			PRI.CurrentItem = J;
			PRI.TopPanelItem = J;
			break;
		}
	}

	PsInfo.PanelControl(PanelHandle, FCTL_REDRAWPANEL, 0, &PRI);
}

void WFD2FFD(const WIN32_FIND_DATA& wfd, PluginPanelItem& ffd, string* NameData)
{
	ffd.FileAttributes = wfd.dwFileAttributes;
	ffd.CreationTime = wfd.ftCreationTime;
	ffd.LastAccessTime = wfd.ftLastAccessTime;
	ffd.LastWriteTime = wfd.ftLastWriteTime;
	ffd.FileSize = make_integer<unsigned long long>(wfd.nFileSizeLow, wfd.nFileSizeHigh);
	ffd.AllocationSize = 0;
	ffd.Reserved[0] = wfd.dwReserved0;
	ffd.Reserved[1] = wfd.dwReserved1;
	ffd.AlternateFileName = {};

	if (NameData)
	{
		*NameData = wfd.cFileName;
		ffd.FileName = NameData->c_str();
	}
}

static void ReplaceSlashToBackslash(string& Str)
{
	std::replace(ALL_RANGE(Str), L'/', L'\\');
}

string FormNtPath(string_view Path)
{
	if (Path.size() > 4 && Path[0] == L'\\' && Path[1] == L'\\')
	{
		if ((Path[2] == L'?' || Path[2] == L'.') && Path[3] == L'\\')
			return string(Path);

		string Str(Path.substr(2));
		ReplaceSlashToBackslash(Str);
		return L"\\\\?\\UNC\\"sv + Str;
	}

	string Str(Path);
	ReplaceSlashToBackslash(Str);
	return L"\\\\?\\"sv + Str;
}

string ExpandEnvStrs(const string_view Input)
{
	string Result(MAX_PATH, 0);
	const null_terminated_t C_Input(Input);

	for (;;)
	{
		const size_t Size = ExpandEnvironmentStrings(C_Input.c_str(), Result.data(), static_cast<DWORD>(Result.size()));
		if (!Size)
			return {};

		const auto CurrentSize = Result.size();
		Result.resize(Size - 1);
		if (Size - 1 <= CurrentSize)
			return Result;
	}
}

static string search_path(const string& Path, const string& Filename)
{
	for (string Result(MAX_PATH, 0);;)
	{
		const auto Size = SearchPath(Path.c_str(), Filename.c_str(), {}, static_cast<DWORD>(Result.size()), Result.data(), {});
		if (!Size)
			return {};

		const auto CurrentSize = Result.size();
		Result.resize(Size);
		if (Size <= CurrentSize)
			return Result;
	}
}

string FindListFile(const string& Filename)
{
	const auto FullPath = GetFullPath(Filename);
	const auto NtPath = FormNtPath(FullPath);

	if (GetFileAttributes(NtPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		return FullPath;

	{
		const auto NamePtr = FSF.PointToName(PsInfo.ModuleName);
		const string Path(PsInfo.ModuleName, NamePtr - PsInfo.ModuleName);

		if (auto Result = search_path(Path, Filename); !Result.empty())
			return Result;
	}

	for (const auto& Path: enum_tokens_with_quotes(ExpandEnvStrs(L"%FARHOME%;%PATH%"sv), L";"sv))
	{
		if (Path.empty())
			continue;

		if (auto Result = search_path(string(Path), Filename); !Result.empty())
			return Result;
	}

	return {};
}

string GetFullPath(string_view Input)
{
	string Result(MAX_PATH, 0);

	for (const null_terminated_t C_Input(Input);;)
	{
		const size_t Size = FSF.ConvertPath(CPM_FULL, C_Input.c_str(), Result.data(), Result.size());
		if (!Size)
			return {};

		const auto CurrentSize = Result.size();
		Result.resize(Size);
		if (Size <= CurrentSize)
			return Result;
	}
}

bool IsTextUTF8(const char* Buffer, size_t Length)
{
	bool Ascii = true;
	size_t Octets = 0;
	size_t LastOctetsPos = 0;
	const size_t MaxCharSize = 4;

	for (size_t i = 0; i < Length; i++)
	{
		BYTE c = Buffer[i];

		if (c & 0x80)
			Ascii = false;

		if (Octets)
		{
			if ((c & 0xC0) != 0x80)
				return false;

			Octets--;
		}
		else
		{
			LastOctetsPos = i;

			if (c & 0x80)
			{
				while (c & 0x80)
				{
					c <<= 1;
					Octets++;
				}

				Octets--;

				if (!Octets)
					return false;
			}
		}
	}

	return (!Octets || Length - LastOctetsPos < MaxCharSize) && !Ascii;
}

static bool isDevice(const string_view Filename, const string_view DevicePrefix)
{
	if (Filename.size() <= DevicePrefix.size())
		return false;

	if (FSF.LStrnicmp(Filename.data(), DevicePrefix.data(), DevicePrefix.size()))
		return false;

	const auto Tail = Filename.substr(DevicePrefix.size());
	return std::ranges::all_of(Tail, std::iswdigit);
}

bool GetFileInfoAndValidate(const string_view FilePath, PluginPanelItem& FindData, string& NameData, const bool Any)
{
	const auto ExpFilePath = ExpandEnvStrs(FilePath);
	const auto [ParamPart, FileNamePart] = ParseParam(ExpFilePath);
	const auto FileName = FileNamePart.empty()? ExpFilePath : FileNamePart;

	if (FileName.empty())
		return false;

	const auto FullPath = GetFullPath(FileName);
	const auto NtPath = FormNtPath(FullPath);

	if (FileName.starts_with(L"\\\\.\\") && FSF.LIsAlpha(FileName[4]) && FileName[5] == L':' && FileName[6] == 0)
	{
		FindData.FileAttributes = FILE_ATTRIBUTE_ARCHIVE;
		NameData = FileName;
		FindData.FileName = NameData.c_str();
		return true;
	}

	if (isDevice(FileName, L"\\\\.\\PhysicalDrive"sv) || isDevice(FileName, L"\\\\.\\cdrom"sv))
	{
		FindData.FileAttributes = FILE_ATTRIBUTE_ARCHIVE;
		NameData = FileName;
		FindData.FileName = NameData.c_str();
		return true;
	}

	const auto Attr = GetFileAttributes(NtPath.c_str());

	if (Attr == INVALID_FILE_ATTRIBUTES)
	{
		if (!Any)
			return false;

		FindData.FileAttributes = FILE_ATTRIBUTE_ARCHIVE;
		NameData = FileName;
		FindData.FileName = NameData.c_str();
		return true;
	}

	WIN32_FIND_DATA wfd{};
	const auto Find = FindFirstFile(NtPath.c_str(), &wfd);

	if (Find != INVALID_HANDLE_VALUE)
	{
		SCOPE_EXIT{ FindClose(Find); };

		WFD2FFD(wfd, FindData, {});
		NameData = FullPath;
		FindData.FileName = NameData.c_str();
		return true;
	}

	wfd.dwFileAttributes = Attr;
	const auto File = CreateFile(NtPath.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, {}, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_POSIX_SEMANTICS, {});
	if (File != INVALID_HANDLE_VALUE)
	{
		SCOPE_EXIT{ CloseHandle(File); };

		GetFileTime(File, &wfd.ftCreationTime, &wfd.ftLastAccessTime, &wfd.ftLastWriteTime);
		wfd.nFileSizeLow = GetFileSize(File, &wfd.nFileSizeHigh);
	}

	WFD2FFD(wfd, FindData, {});
	NameData = FullPath;
	FindData.FileName = NameData.c_str();
	return true;
}
