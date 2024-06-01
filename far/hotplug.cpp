/*
hotplug.cpp

Отключение Hotplug-устройств
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "hotplug.hpp"

// Internal:
#include "lang.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "notification.hpp"
#include "flink.hpp"
#include "strmix.hpp"
#include "drivemix.hpp"
#include "global.hpp"
#include "keyboard.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/enum_substrings.hpp"
#include "common/function_ref.hpp"
#include "common/keep_alive.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

/*
A device is considered a HotPlug device if the following are TRUE:
- does NOT have problem CM_PROB_DEVICE_NOT_THERE
- does NOT have problem CM_PROB_HELD_FOR_EJECT
- does NOT have problem CM_PROB_DISABLED
- has Capability CM_DEVCAP_REMOVABLE
- does NOT have Capability CM_DEVCAP_SURPRISEREMOVALOK
- does NOT have Capability CM_DEVCAP_DOCKDEVICE
*/

namespace
{
	class [[nodiscard]] enum_child_devices: public enumerator<enum_child_devices, DEVINST>
	{
		IMPLEMENTS_ENUMERATOR(enum_child_devices);

	public:
		explicit enum_child_devices(DEVINST Root):
			m_Root(Root)
		{
		}

	private:
		[[nodiscard, maybe_unused]]
		bool get(bool Reset, DEVINST& Value) const
		{
			if ((Reset? CM_Get_Child(&m_Current, m_Root, 0) : CM_Get_Sibling(&m_Current, m_Current, 0)) != CR_SUCCESS)
				return false;

			Value = m_Current;
			return true;
		}

		DEVINST m_Root;
		mutable DEVINST m_Current{};
	};
}

namespace detail
{
	struct devinfo_handle_closer
	{
		void operator()(HDEVINFO Handle) const noexcept
		{
			if (!SetupDiDestroyDeviceInfoList(Handle))
				LOGWARNING(L"SetupDiDestroyDeviceInfoList(): {}"sv, os::last_error());
		}
	};
}

class [[nodiscard]] dev_info: noncopyable
{
	using devinfo_handle = os::detail::handle_t<detail::devinfo_handle_closer>;

public:
	explicit dev_info(DEVINST DevInst)
	{
		wchar_t szDeviceID[MAX_DEVICE_ID_LEN];
		if (CM_Get_Device_ID(DevInst, szDeviceID, static_cast<ULONG>(std::size(szDeviceID)), 0) != CR_SUCCESS)
			return;

		m_info.reset(SetupDiGetClassDevs(&GUID_DEVINTERFACE_VOLUME, szDeviceID, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT));
		if (m_info)
		{
			m_id = szDeviceID;
		}
	}

	[[nodiscard]]
	explicit operator bool() const noexcept { return m_info != nullptr; }

	[[nodiscard]]
	bool OpenDeviceInfo(SP_DEVINFO_DATA& info_data) const
	{
		return SetupDiOpenDeviceInfo(m_info.native_handle(), m_id.c_str(), nullptr, 0, &info_data) != FALSE;
	}

	[[nodiscard]]
	bool GetDeviceRegistryProperty(SP_DEVINFO_DATA& info_data, DWORD Property, PDWORD PropertyRegDataType, PBYTE PropertyBuffer, DWORD PropertyBufferSize, PDWORD RequiredSize) const
	{
		return SetupDiGetDeviceRegistryProperty(m_info.native_handle(), &info_data, Property, PropertyRegDataType, PropertyBuffer, PropertyBufferSize, RequiredSize) != FALSE;
	}

	[[nodiscard]]
	bool EnumDeviceInterfaces(const UUID& InterfaceClassUuid, DWORD MemberIndex, SP_DEVICE_INTERFACE_DATA& DeviceInterfaceData) const
	{
		return SetupDiEnumDeviceInterfaces(m_info.native_handle(), nullptr, &InterfaceClassUuid, MemberIndex, &DeviceInterfaceData) != FALSE;
	}

	[[nodiscard]]
	auto DeviceInterfacesEnumerator(UUID const& InterfaceClassUuid) const
	{
		using value_type = SP_DEVICE_INTERFACE_DATA;
		return inline_enumerator<value_type>([this, InterfaceClassUuid = keep_alive(FWD(InterfaceClassUuid)), Index = 0uz](const bool Reset, value_type& Value) mutable
		{
			if (Reset)
				Index = 0;

			Value.cbSize = sizeof(Value);
			return SetupDiEnumDeviceInterfaces(m_info.native_handle(), nullptr, &InterfaceClassUuid, static_cast<DWORD>(Index++), &Value) != FALSE;
		});
	}

	[[nodiscard]]
	string GetDevicePath(SP_DEVICE_INTERFACE_DATA& DeviceInterfaceData) const
	{
		string result;
		DWORD RequiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(m_info.native_handle(), &DeviceInterfaceData, nullptr, 0, &RequiredSize, nullptr);
		if(RequiredSize)
		{
			const block_ptr<SP_DEVICE_INTERFACE_DETAIL_DATA> DData(RequiredSize);
			DData->cbSize = sizeof(*DData);
			if(SetupDiGetDeviceInterfaceDetail(m_info.native_handle(), &DeviceInterfaceData, DData.data(), RequiredSize, nullptr, nullptr))
			{
				result = DData->DevicePath;
			}
		}
		return result;
	}

private:
	devinfo_handle m_info;
	string m_id;
};

[[nodiscard]]
static bool GetDevicePropertyImpl(DEVINST hDevInst, function_ref<bool(dev_info const&, SP_DEVINFO_DATA&)> const Receiver)
{
	dev_info const Info(hDevInst);
	if (!Info)
		return false;

	SP_DEVINFO_DATA DeviceInfoData{ sizeof(DeviceInfoData) };
	if (!Info.OpenDeviceInfo(DeviceInfoData))
		return false;

	return Receiver(Info, DeviceInfoData);
}

[[nodiscard]]
static bool GetDeviceProperty(DEVINST hDevInst, DWORD Property, DWORD& Value)
{
	return GetDevicePropertyImpl(hDevInst, [&](const dev_info& Info, SP_DEVINFO_DATA& DeviceInfoData)
	{
		return Info.GetDeviceRegistryProperty(DeviceInfoData, Property, nullptr, std::bit_cast<BYTE*>(&Value), sizeof(Value), nullptr);
	});
}

[[nodiscard]]
static bool GetDeviceProperty(DEVINST hDevInst, DWORD Property, string& Value)
{
	return GetDevicePropertyImpl(hDevInst, [&](const dev_info& Info, SP_DEVINFO_DATA& DeviceInfoData)
	{
		return os::detail::ApiDynamicStringReceiver(Value, [&](std::span<wchar_t> Buffer)
		{
			DWORD RequiredSize = 0;
			if (Info.GetDeviceRegistryProperty(DeviceInfoData, Property, nullptr, std::bit_cast<BYTE*>(Buffer.data()), static_cast<DWORD>(Buffer.size()), &RequiredSize))
				return RequiredSize / sizeof(wchar_t) - 1;
			return RequiredSize / sizeof(wchar_t);
		});
	});
}

[[nodiscard]]
static bool GetDevicePropertyRecursive(DEVINST hDevInst, DWORD Property, string& Value)
{
	DEVINST hDevChild;
	return GetDeviceProperty(hDevInst, Property, Value) || (CM_Get_Child(&hDevChild, hDevInst, 0) == CR_SUCCESS && GetDeviceProperty(hDevChild, Property, Value));
}

[[nodiscard]]
static bool IsChildDeviceHotplug(DEVINST hDevInst)
{
	DEVINST hDevChild;
	if (CM_Get_Child(&hDevChild, hDevInst, 0) != CR_SUCCESS)
		return false;

	DWORD Capabilities = 0;
	return GetDeviceProperty(hDevChild, SPDRP_CAPABILITIES, Capabilities) &&
		!(Capabilities&CM_DEVCAP_SURPRISEREMOVALOK) &&
		(Capabilities&CM_DEVCAP_UNIQUEID);
}

[[nodiscard]]
static bool IsDeviceHotplug(DEVINST hDevInst, bool const IncludeSafeToRemove)
{
	DWORD Capabilities = 0;
	if (!GetDeviceProperty(hDevInst, SPDRP_CAPABILITIES, Capabilities))
		return false;

	ULONG Status = 0, Problem = 0;
	if (CM_Get_DevNode_Status(&Status, &Problem, hDevInst, 0) != CR_SUCCESS)
		return false;

	return (Problem != CM_PROB_DEVICE_NOT_THERE) &&
	       (Problem != CM_PROB_HELD_FOR_EJECT) && //возможно, надо проверять на наличие проблем вообще
	       (Problem != CM_PROB_DISABLED) &&
	       (Capabilities & CM_DEVCAP_REMOVABLE) &&
	       (IncludeSafeToRemove || !(Capabilities & CM_DEVCAP_SURPRISEREMOVALOK) || IsChildDeviceHotplug(hDevInst)) &&
	       !(Capabilities & CM_DEVCAP_DOCKDEVICE);
}

[[nodiscard]]
static DWORD DriveMaskFromVolumeName(string_view const VolumeName)
{
	DWORD Result = 0;
	string strCurrentVolumeName;
	const os::fs::enum_drives Enumerator(os::fs::get_logical_drives());
	const auto ItemIterator = std::ranges::find_if(Enumerator, [&](const auto& i)
	{
		return os::fs::GetVolumeNameForVolumeMountPoint(os::fs::drive::get_win32nt_root_directory(i), strCurrentVolumeName) && starts_with_icase(strCurrentVolumeName, VolumeName);
	});
	if (ItemIterator != Enumerator.cend() && os::fs::drive::is_standard_letter(*ItemIterator))
	{
		Result = bit(os::fs::drive::get_number(*ItemIterator));
	}
	return Result;
}

struct device_paths
{
	os::fs::drives_set Disks;
	std::list<string> Volumes;

	void append(device_paths&& rhs)
	{
		Disks |= rhs.Disks;
		Volumes.splice(Volumes.end(), rhs.Volumes);
	}
};

[[nodiscard]]
static device_paths get_device_paths_impl(DEVINST hDevInst)
{
	dev_info const Info(hDevInst);
	if (!Info)
		return {};

	device_paths DevicePaths;

	for (auto& i: Info.DeviceInterfacesEnumerator(GUID_DEVINTERFACE_VOLUME))
	{
		auto strMountPoint = Info.GetDevicePath(i);
		if (strMountPoint.empty())
			continue;

		AddEndSlash(strMountPoint);
		string strVolumeName;
		if (!os::fs::GetVolumeNameForVolumeMountPoint(strMountPoint, strVolumeName))
			continue;

		DevicePaths.Disks |= DriveMaskFromVolumeName(strVolumeName);
		DevicePaths.Volumes.emplace_back(strVolumeName);
	}

	return DevicePaths;
}

[[nodiscard]]
static device_paths get_relation_device_paths(DEVINST hDevInst)
{
	wchar_t szDeviceID[MAX_DEVICE_ID_LEN];
	if (CM_Get_Device_ID(hDevInst, szDeviceID, static_cast<ULONG>(std::size(szDeviceID)), 0) != CR_SUCCESS)
		return {};

	DWORD dwSize = 0;
	if (CM_Get_Device_ID_List_Size(&dwSize, szDeviceID, CM_GETIDLIST_FILTER_REMOVALRELATIONS) != CR_SUCCESS || !dwSize)
		return {};

	const wchar_t_ptr_n<os::default_buffer_size> DeviceIdList(dwSize);
	if (CM_Get_Device_ID_List(szDeviceID, DeviceIdList.data(), dwSize, CM_GETIDLIST_FILTER_REMOVALRELATIONS) != CR_SUCCESS)
		return {};

	device_paths DevicePaths;
	for (const auto& i: enum_substrings(DeviceIdList))
	{
		DEVINST hRelationDevInst;
		if (CM_Locate_DevNode(&hRelationDevInst, const_cast<DEVINSTID>(i.data()), 0) == CR_SUCCESS)
			DevicePaths.append(get_device_paths_impl(hRelationDevInst));
	}

	return DevicePaths;
}

[[nodiscard]]
static device_paths get_device_paths(DEVINST hDevInst)
{
	device_paths DevicePaths;

	DevicePaths.append(get_device_paths_impl(hDevInst));
	DevicePaths.append(get_relation_device_paths(hDevInst));

	for (const auto& i: enum_child_devices(hDevInst))
	{
		/*
		Only get the drive letter for this device if it is not a hotplug device.
		If it is a hotplug device then it will have its own subtree that contains its drive letters.
		*/
		if (!IsDeviceHotplug(i, false))
			DevicePaths.append(get_device_paths(i));
	}

	return DevicePaths;
}

struct DeviceInfo
{
	DEVINST DevInst;
	device_paths DevicePaths;
};

static void GetHotplugDevicesInfo(DEVINST hDevInst, std::vector<DeviceInfo>& Info, bool const IncludeSafeToRemove)
{
	if (IsDeviceHotplug(hDevInst, IncludeSafeToRemove))
	{
		Info.emplace_back(DeviceInfo{ hDevInst, get_device_paths(hDevInst) });
	}

	for (const auto& i: enum_child_devices(hDevInst))
	{
		GetHotplugDevicesInfo(i, Info, IncludeSafeToRemove);
	}
}

[[nodiscard]]
static auto GetHotplugDevicesInfo(bool const IncludeSafeToRemove)
{
	std::vector<DeviceInfo> Result;

	DEVINST Root;
	if (CM_Locate_DevNode(&Root, nullptr, CM_LOCATE_DEVNODE_NORMAL) == CR_SUCCESS)
	{
		GetHotplugDevicesInfo(Root, Result, IncludeSafeToRemove);
	}

	return Result;
}

[[nodiscard]]
static bool RemoveHotplugDriveDevice(const DeviceInfo& Info, bool const Confirm, bool& Cancelled)
{
	string strFriendlyName;
	if (GetDevicePropertyRecursive(Info.DevInst, SPDRP_FRIENDLYNAME, strFriendlyName))
		inplace::trim(strFriendlyName);

	string strDescription;
	if (GetDevicePropertyRecursive(Info.DevInst, SPDRP_DEVICEDESC, strDescription))
		inplace::trim(strDescription);

	auto MessageResult = message_result::first_button;

	if (Confirm)
	{
		const auto Separator = L", "sv;

		auto DisksStr = join(Separator, os::fs::enum_drives(Info.DevicePaths.Disks) | std::views::transform([](wchar_t const Drive){ return os::fs::drive::get_device_path(Drive); }));

		if (DisksStr.empty())
			DisksStr = join(Separator, Info.DevicePaths.Volumes);

		std::vector<string> MessageItems;
		MessageItems.reserve(6);

		MessageItems.emplace_back(msg(lng::MChangeHotPlugDisconnectDriveQuestion));
		MessageItems.emplace_back(strDescription);

		if (!strFriendlyName.empty() && !equal_icase(strDescription, strFriendlyName))
			MessageItems.emplace_back(strFriendlyName);

		if (!DisksStr.empty())
			MessageItems.emplace_back(far::vformat(msg(lng::MHotPlugDisks), DisksStr));

		MessageResult = Message(MSG_WARNING,
			msg(lng::MChangeHotPlugDisconnectDriveTitle),
			std::move(MessageItems),
			{ lng::MHRemove, lng::MHCancel });
	}

	if (MessageResult != message_result::first_button)
	{
		Cancelled = true;
		return false;
	}

	PNP_VETO_TYPE pvtVeto = PNP_VetoTypeUnknown;
	wchar_t VetoName[MAX_PATH];

	const auto crResult = CM_Request_Device_Eject(Info.DevInst, &pvtVeto, VetoName, static_cast<ULONG>(std::size(VetoName)), 0);
	if (crResult != CR_SUCCESS || pvtVeto != PNP_VetoTypeUnknown)   //M$ баг, если есть VetoName, то даже при ошибке возвращается CR_SUCCESS
	{
		SetLastError(pvtVeto == PNP_VetoTypeUnknown? ERROR_UNABLE_TO_UNLOAD_MEDIA : ERROR_DRIVE_LOCKED);
		return false;
	}

	Message(0,
		msg(lng::MChangeHotPlugDisconnectDriveTitle),
		{
			msg(lng::MChangeHotPlugNotify1),
			strDescription,
			strFriendlyName,
			msg(lng::MChangeHotPlugNotify2)
		},
		{ lng::MOk });

	return true;
}

[[nodiscard]]
bool RemoveHotplugDrive(string_view const Path, bool const Confirm, bool& Cancelled)
{
	// Removing VHD disk as hotplug is a very bad idea.
	// Currently OS removes the device but doesn't close the file handle, rendering the file completely unavailable until reboot.
	if (auto IsVhd = false; detach_vhd(Path, IsVhd))
	{
		return true;
	}
	else if (IsVhd)
	{
		Cancelled = true;
		return false;
	}

	const auto PathType = ParsePath(Path);
	if (PathType != root_type::win32nt_drive_letter && PathType != root_type::volume)
	{
		Cancelled = true;
		return false;
	}

	// Some USB drives always have CM_DEVCAP_SURPRISEREMOVALOK flag set
	const auto Info = GetHotplugDevicesInfo(true);

	const auto ItemIterator = [&]
	{
		if (PathType == root_type::win32nt_drive_letter)
		{
			const auto DiskNumber = os::fs::drive::get_number(Path[L"\\\\?\\"sv.size()]);
			return std::ranges::find_if(Info, [&](DeviceInfo const& i){ return i.DevicePaths.Disks[DiskNumber]; });
		}

		return std::ranges::find_if(Info, [&](DeviceInfo const& i)
		{
			return std::ranges::any_of(i.DevicePaths.Volumes, [&](string const& VolumeName)
			{
				return equal_icase(VolumeName, Path);
			});
		});
	}();

	if (ItemIterator == Info.cend())
	{
		Cancelled = true;
		return false;
	}

	return RemoveHotplugDriveDevice(*ItemIterator, Confirm, Cancelled);
}

void ShowHotplugDevices()
{
	const auto HotPlugList = VMenu2::create(msg(lng::MHotPlugListTitle), {});
	std::vector<DeviceInfo> Info;

	const auto FillMenu = [&]()
	{
		HotPlugList->clear();
		Info = GetHotplugDevicesInfo(false);

		if (Info.empty())
			return;

		for (const auto& i: Info)
		{
			MenuItemEx ListItem;
			string strDescription;
			if (GetDevicePropertyRecursive(i.DevInst, SPDRP_DEVICEDESC, strDescription) && !strDescription.empty())
			{
				inplace::trim(strDescription);
				ListItem.Name = strDescription;
			}

			string strFriendlyName;
			if (GetDevicePropertyRecursive(i.DevInst, SPDRP_FRIENDLYNAME, strFriendlyName) && !strFriendlyName.empty())
			{
				inplace::trim(strFriendlyName);

				if (!strDescription.empty())
				{
					if (!equal_icase(strDescription, strFriendlyName))
					{
						append(ListItem.Name, L" \""sv, strFriendlyName, L"\""sv);
					}
				}
				else
				{
					ListItem.Name = strFriendlyName;
				}
			}

			if (ListItem.Name.empty())
			{
				ListItem.Name = L"UNKNOWN"sv;
			}
			HotPlugList->AddItem(ListItem);
		}
	};

	FillMenu();
	HotPlugList->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND | VMENU_AUTOHIGHLIGHT);
	HotPlugList->SetPosition({ -1, -1, 0, 0 });
	HotPlugList->AssignHighlights();
	HotPlugList->SetBottomTitle(KeysToLocalizedText(KEY_DEL, KEY_CTRLR));

	SCOPED_ACTION(listener)(update_devices, [&]
	{
		HotPlugList->ProcessKey(Manager::Key(KEY_CTRLR));
	});

	HotPlugList->Run([&](const Manager::Key& RawKey)
	{
		int KeyProcessed = 1;

		switch (RawKey())
		{
			case KEY_F1:
			{
				help::show(L"HotPlugList"sv);
				break;
			}
			case KEY_CTRLR:
			{
				FillMenu();
				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				if (!HotPlugList->empty())
				{
					const auto I = HotPlugList->GetSelectPos();

					bool Cancelled = false;
					if (RemoveHotplugDriveDevice(Info[I], Global->Opt->Confirm.RemoveHotPlug, Cancelled))
					{
						FillMenu();
					}
					else if (!Cancelled)
					{
						SetLastError(ERROR_DRIVE_LOCKED); // ... "The disk is in use or locked by another process."
						const auto ErrorState = os::last_error();

						Message(MSG_WARNING, ErrorState,
							msg(lng::MError),
							{
								msg(lng::MChangeCouldNotEjectHotPlugMedia2),
								HotPlugList->at(I).Name
							},
							{ lng::MOk });
					}
				}

				break;
			}

			default:
				KeyProcessed = 0;
		}
		return KeyProcessed;
	});
}
