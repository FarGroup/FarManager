#include "NetFavorites.hpp"
#include "Network.hpp"
#include "NetCommon.hpp"
#include "NetLng.hpp"
#include "guid.hpp"
#include <PluginSettings.hpp>
#include <string>
#include <vector>

const wchar_t SZ_FAVORITES[] = L"Favorites";
const wchar_t SZ_USERNAME[] = L"UserName";
const wchar_t SZ_USERPASS[] = L"Password";
static wchar_t szFavProv[] = L"Far Favorites Provider";

std::vector<std::wstring> SplitPath(const wchar_t* lpRemoteName)
{
	// lpRemoteName = Favorite\\share \\machine\share or \\machine
	// result  {Favorite , machine, share} or {Favorite , machine}
	// machine name must be in uppercase
	std::vector<std::wstring> result;
	result.reserve(3);
	std::wstring remote_name{lpRemoteName};
	result.emplace_back(SZ_FAVORITES);
	if (remote_name.find(SZ_FAVORITES) == 0)
	{
		remote_name = remote_name.substr(std::size(SZ_FAVORITES));
	}

	size_t i = 0;
	while (remote_name[i] == L'\\')
	{
		i++;
	}

	std::wstring machine_name{remote_name.substr(i)};
	auto const pos = machine_name.find(L'\\');
	if (pos != std::string::npos)
	{
		auto share_name = machine_name.substr(pos + 1);
		machine_name = machine_name.substr(0, pos);
		CharUpperBuffW(machine_name.data(), static_cast<DWORD>(machine_name.size()));
		result.push_back(machine_name);
		result.push_back(share_name);
	}
	else
	{
		CharUpperBuffW(machine_name.data(), static_cast<DWORD>(machine_name.size()));
		result.push_back(machine_name);
	}

	return result;
}

bool EnumFavorites(const LPNETRESOURCE pNR, NetResourceList* pList)
{
	if (!pNR)
	{
		NETRESOURCE tmp{};
		tmp.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
		tmp.lpRemoteName = {};
		tmp.lpProvider = szFavProv;
		pList->Push(tmp);
		return false;
	}
	else if (lstrcmp(pNR->lpProvider, szFavProv) == 0)
	{
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		size_t favorites_root = settings.CreateSubKey(0, SZ_FAVORITES);

		FarSettingsEnum fse{};
		if (settings.Enum(favorites_root, &fse))
		{
			for (size_t i = 0; i < fse.Count; i++)
			{
				if (fse.Items[i].Type == FST_SUBKEY)
				{
					wchar_t szSrc[MAX_PATH] = L"\\\\";
					lstrcat(szSrc, fse.Items[i].Name);
					NETRESOURCE tmp{};

					if (Opt.FavoritesFlags & FAVORITES_CHECK_RESOURCES)
					{
						tmp.lpProvider = {};
						tmp.lpRemoteName = szSrc;
						tmp.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
						pList->Push(tmp);
					}
					else if (NetBrowser::GetResourceInfo(szSrc, &tmp))
					{
						pList->Push(tmp);
						NetResourceList::DeleteNetResource(tmp);
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool CheckFavoriteItem(const LPNETRESOURCE pNR)
{
	return pNR && lstrcmp(pNR->lpProvider, szFavProv) == 0;
}

bool InFavoriteExists(const wchar_t* lpRemoteName)
{
	if (!lpRemoteName)
	{
		return false;
	}
	auto split_path = SplitPath(lpRemoteName);

	PluginSettings settings(MainGuid, PsInfo.SettingsControl);
	size_t level_root = 0;
	for (const auto& piece: split_path)
	{
		level_root = settings.OpenSubKey(level_root, piece.c_str());
		if (level_root == 0)
			return false;
	}
	return true;
}

void WriteFavoriteItem(FAVORITEITEM* lpFavItem)
{
	auto split_path = SplitPath(lpFavItem->lpRemoteName);
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);
	size_t level_root = 0;
	for (const auto& piece: split_path)
	{
		level_root = settings.CreateSubKey(level_root, piece.c_str());
		if (level_root == 0)
			return;
	}
	settings.Set(level_root, SZ_USERNAME, lpFavItem->lpUserName);
	settings.Set(level_root, SZ_USERPASS, lpFavItem->lpPassword);
}

bool ReadFavoriteItem(FAVORITEITEM* lpFavItem)
{
	if (lpFavItem)
	{
		auto split_path = SplitPath(lpFavItem->lpRemoteName);
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		size_t level_root = 0;
		for (const auto& piece: split_path)
		{
			level_root = settings.OpenSubKey(level_root, piece.c_str());
			if (level_root == 0)
				return false;
		}

		lpFavItem->lpUserName = _wcsdup(settings.Get(level_root, SZ_USERNAME, L""));
		lpFavItem->lpPassword = _wcsdup(settings.Get(level_root, SZ_USERPASS, L""));
		return true;
	}
	return false;
}

bool GetFavoritesParent(NETRESOURCE& SrcRes, LPNETRESOURCE lpParent)
{
	if (!lpParent)
		return false;

	if (SrcRes.dwDisplayType == RESOURCEDISPLAYTYPE_SHARE)
		return false;

	if (lstrcmp(SrcRes.lpProvider, szFavProv) == 0)
	{
		return false;
	}

	if (InFavoriteExists(SrcRes.lpRemoteName))
	{
		wchar_t res[MAX_PATH];
		lstrcpy(res, GetMsg(MFavorites));

		NETRESOURCE nr{};
		nr.lpProvider = szFavProv;
		nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
		nr.lpRemoteName = res;
		NetResourceList::CopyNetResource(*lpParent, nr);

		return true;
	}

	return false;
}

bool GetFavoriteResource(const wchar_t* SrcName, LPNETRESOURCE DstNetResource)
{
	NETRESOURCE nr = {0};

	wchar_t szKey[MAX_PATH];
	if (InFavoriteExists(SrcName))
	{
		auto p1 = FSF.PointToName(SrcName);
		if (DstNetResource)
		{
			lstrcpy(szKey, L"\\\\");
			lstrcat(szKey, p1);
			nr.lpRemoteName = CharUpper(szKey);
			nr.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
			NetResourceList::CopyNetResource(*DstNetResource, nr);
		}

		return true;
	}

	return false;
}

bool RemoveFromFavorites(const wchar_t* SrcName)
{
	auto split_path = SplitPath(SrcName);
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	size_t level_root = 0;
	for (const auto& piece: split_path)
	{
		level_root = settings.CreateSubKey(level_root, piece.c_str());
		if (level_root == 0)
			return false;
	}
	settings.DeleteSubKey(level_root);
	return true;
}
