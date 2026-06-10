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

[[nodiscard]]
static DWORD cr_to_win32_error(int const Cr)
{
	switch (Cr)
	{
	case CR_SUCCESS:                          return ERROR_SUCCESS;                               // The operation completed successfully
	case CR_DEFAULT:                          return ERROR_CALL_NOT_IMPLEMENTED;                  // The operation is not implemented for this request
	case CR_OUT_OF_MEMORY:                    return ERROR_NOT_ENOUGH_MEMORY;                     // Not enough memory is available to process this command
	case CR_INVALID_POINTER:                  return ERROR_INVALID_USER_BUFFER;                   // A required pointer parameter is invalid
	case CR_INVALID_FLAG:                     return ERROR_INVALID_FLAGS;                         // The ulFlags parameter specified is invalid for this operation
	case CR_INVALID_DEVNODE:                  return SPAPI_E_NO_SUCH_DEVINST;                     // The device instance handle parameter is not valid
	case CR_INVALID_RES_DES:                  return ERROR_INVALID_PARAMETER;                     // The supplied resource descriptor parameter is invalid
	case CR_INVALID_LOG_CONF:                 return ERROR_INVALID_PARAMETER;                     // The supplied logical configuration parameter is invalid
	case CR_INVALID_ARBITRATOR:               return ERROR_INVALID_PARAMETER;                     // The arbitrator's registration identifier is invalid or there is already such a global arbitrator
	case CR_INVALID_NODELIST:                 return ERROR_INVALID_PARAMETER;                     // The nodelist header is invalid
	case CR_DEVNODE_HAS_REQS:                 return ERROR_ALREADY_EXISTS;                        // The device instance already has requirements
	case CR_INVALID_RESOURCEID:               return ERROR_MRM_INVALID_RESOURCE_IDENTIFIER;       // The RESOURCEID parameter does not contain a valid RESOURCEID
	case CR_DLVXD_NOT_FOUND:                  return ERROR_FILE_NOT_FOUND;                        // Dynamically loadable VxD was not found
	case CR_NO_SUCH_DEVNODE:                  return SPAPI_E_NO_SUCH_DEVINST;                     // The specified device instance handle does not correspond to a present device
	case CR_NO_MORE_LOG_CONF:                 return ERROR_NO_MORE_ITEMS;                         // There are no more logical configurations available
	case CR_NO_MORE_RES_DES:                  return ERROR_NO_MORE_ITEMS;                         // There are no more resource descriptions available
	case CR_ALREADY_SUCH_DEVNODE:             return SPAPI_E_DEVINST_ALREADY_EXISTS;              // This device instance already exists
	case CR_INVALID_RANGE_LIST:               return ERROR_INVALID_PARAMETER;                     // The supplied range list parameter is invalid
	case CR_INVALID_RANGE:                    return ERROR_INVALID_PARAMETER;                     // The supplied range parameter is invalid
	case CR_FAILURE:                          return ERROR_GEN_FAILURE;                           // A general internal error occurred
	case CR_NO_SUCH_LOGICAL_DEV:              return ERROR_NO_SUCH_DEVICE;                        // The logical device was not found in ISAPNP conversion
	case CR_CREATE_BLOCKED:                   return ERROR_DEVICE_INSTALL_BLOCKED;                // The device is disabled for this configuration
	case CR_NOT_SYSTEM_VM:                    return ERROR_INVALID_FUNCTION;                      // This routine must be called from the system VM
	case CR_REMOVE_VETOED:                    return ERROR_DEVICE_IN_USE;                         // A service or application refused to allow removal of this device
	case CR_APM_VETOED:                       return ERROR_SET_POWER_STATE_VETOED;                // The APM request has been vetoed
	case CR_INVALID_LOAD_TYPE:                return ERROR_INVALID_PARAMETER;                     // The load type is invalid
	case CR_BUFFER_SMALL:                     return ERROR_INSUFFICIENT_BUFFER;                   // An output parameter was too small to hold all the data available
	case CR_NO_ARBITRATOR:                    return ERROR_INVALID_PARAMETER;                     // The resource has no arbitrator
	case CR_NO_REGISTRY_HANDLE:               return ERROR_INVALID_HANDLE;                        // The operation does not produce registry entry
	case CR_REGISTRY_ERROR:                   return SPAPI_E_PNP_REGISTRY_ERROR;                  // A required entry in the registry is missing or an attempt to write to the registry failed
	case CR_INVALID_DEVICE_ID:                return SPAPI_E_INVALID_DEVINST_NAME;                // The specified Device ID is not a valid Device ID
	case CR_INVALID_DATA:                     return ERROR_INVALID_DATA;                          // One or more parameters were invalid
	case CR_INVALID_API:                      return ERROR_INVALID_FUNCTION;                      // This routine cannot be called from ring 3
	case CR_DEVLOADER_NOT_READY:              return ERROR_NOT_READY;                             // The device loader is not ready
	case CR_NEED_RESTART:                     return ERROR_SUCCESS_RESTART_REQUIRED;              // The system must be restarted for the operation to be completed
	case CR_NO_MORE_HW_PROFILES:              return ERROR_NO_MORE_ITEMS;                         // There are no more hardware profiles available
	case CR_DEVICE_NOT_THERE:                 return ERROR_DEVICE_NOT_CONNECTED;                  // The driver could not find the device
	case CR_NO_SUCH_VALUE:                    return ERROR_NOT_FOUND;                             // The specified value does not exist in the registry
	case CR_WRONG_TYPE:                       return ERROR_INVALID_PARAMETER;                     // The registry value has wrong type
	case CR_INVALID_PRIORITY:                 return ERROR_INVALID_PARAMETER;                     // The specified priority is invalid for this operation
	case CR_NOT_DISABLEABLE:                  return SPAPI_E_NOT_DISABLEABLE;                     // This device cannot be disabled
	case CR_FREE_RESOURCES:                   return ERROR_INVALID_PARAMETER;                     // The resources have been freed
	case CR_QUERY_VETOED:                     return ERROR_PLUGPLAY_QUERY_VETOED;                 // A service or application refused to query this device
	case CR_CANT_SHARE_IRQ:                   return ERROR_IRQ_BUSY;                              // The IRQ cannot be shared
	case CR_NO_DEPENDENT:                     return ERROR_CALL_NOT_IMPLEMENTED;                  // This routine is not implemented in this version of the operating system
	case CR_SAME_RESOURCES:                   return ERROR_INVALID_PARAMETER;                     // The two resources represent the same resource
	case CR_NO_SUCH_REGISTRY_KEY:             return SPAPI_E_KEY_DOES_NOT_EXIST;                  // The specified key does not exist in the registry
	case CR_INVALID_MACHINENAME:              return SPAPI_E_INVALID_MACHINENAME;                 // The specified machine name does not meet the UNC naming conventions
	case CR_REMOTE_COMM_FAILURE:              return SPAPI_E_REMOTE_COMM_FAILURE;                 // A general remote communication error occurred
	case CR_MACHINE_UNAVAILABLE:              return SPAPI_E_MACHINE_UNAVAILABLE;                 // The machine selected for remote communication is not available at this time
	case CR_NO_CM_SERVICES:                   return SPAPI_E_NO_CONFIGMGR_SERVICES;               // The Plug and Play service or another required service is not available
	case CR_ACCESS_DENIED:                    return ERROR_ACCESS_DENIED;                         // Access denied
	case CR_CALL_NOT_IMPLEMENTED:             return ERROR_CALL_NOT_IMPLEMENTED;                  // This routine is not implemented in this version of the operating system
	case CR_INVALID_PROPERTY:                 return SPAPI_E_INVALID_REG_PROPERTY;                // The specified property type is invalid for this operation
	case CR_DEVICE_INTERFACE_ACTIVE:          return SPAPI_E_DEVICE_INTERFACE_ACTIVE;             // Device interface is active
	case CR_NO_SUCH_DEVICE_INTERFACE:         return SPAPI_E_NO_SUCH_DEVICE_INTERFACE;            // No such device interface
	case CR_INVALID_REFERENCE_STRING:         return SPAPI_E_INVALID_REFERENCE_STRING;            // Invalid reference string
	case CR_INVALID_CONFLICT_LIST:            return ERROR_INVALID_PARAMETER;                     // Invalid conflict list
	case CR_INVALID_INDEX:                    return ERROR_INVALID_PARAMETER;                     // Invalid index
	case CR_INVALID_STRUCTURE_SIZE:           return ERROR_INVALID_PARAMETER;                     // Invalid structure size
	default:                                  return ERROR_GEN_FAILURE;
	}

#if NUM_CR_RESULTS > 0x0000003C
	COMPILER_WARNING("Update the mapping");
#endif
}

[[nodiscard]]
static auto cr_error(int const Cr)
{
	return far::format(L"CR 0x{:0>8X} ({})"sv, Cr, os::format_error(cr_to_win32_error(Cr)));
}

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
			if (const auto Result = Reset? CM_Get_Child(&m_Current, m_Root, 0) : CM_Get_Sibling(&m_Current, m_Current, 0); Result != CR_SUCCESS)
			{
				if (Result != CR_NO_SUCH_DEVINST)
					LOGWARNING(L"{}: {}"sv, Reset? L"CM_Get_Child" : L"CM_Get_Sibling"sv, cr_error(Result));
				return false;
			}

			Value = m_Current;
			return true;
		}

		DEVINST m_Root;
		mutable DEVINST m_Current{};
	};
}

[[nodiscard]]
static bool GetDeviceProperty(DEVINST hDevInst, DWORD Property, DWORD& Value)
{
	ULONG Size = sizeof(Value);
	if (const auto Result = CM_Get_DevNode_Registry_Property(hDevInst, Property, nullptr, &Value, &Size, 0); Result != CR_SUCCESS)
	{
		if (Result != CR_NO_SUCH_VALUE)
			LOGWARNING(L"CM_Get_DevNode_Registry_Property: {}"sv, cr_error(Result));
		return false;
	}

	return true;
}

[[nodiscard]]
static bool GetDeviceProperty(DEVINST hDevInst, DWORD Property, string& Value)
{
	return os::detail::ApiDynamicStringReceiver(Value, [&](std::span<wchar_t> Buffer) -> size_t
	{
		auto Size = static_cast<ULONG>(Buffer.size());
		const auto Result = CM_Get_DevNode_Registry_Property(hDevInst, Property, nullptr, Buffer.data(), &Size, 0);
		if (Result == CR_SUCCESS)
			return Size / sizeof(wchar_t) - 1;
		if (Result == CR_BUFFER_SMALL)
			return Size / sizeof(wchar_t);

		if (Result != CR_NO_SUCH_VALUE)
			LOGWARNING(L"CM_Get_DevNode_Registry_Property: {}"sv, cr_error(Result));

		return 0;
	});
}

[[nodiscard]]
static bool GetDevicePropertyRecursive(DEVINST hDevInst, DWORD Property, string& Value)
{
	if (GetDeviceProperty(hDevInst, Property, Value))
		return true;

	DEVINST hDevChild;
	if (const auto Result = CM_Get_Child(&hDevChild, hDevInst, 0); Result != CR_SUCCESS)
	{
		if (Result != CR_NO_SUCH_DEVINST)
			LOGWARNING(L"CM_Get_Child: {}"sv, cr_error(Result));
		return false;
	}

	return GetDeviceProperty(hDevChild, Property, Value);
}

[[nodiscard]]
static bool IsChildDeviceHotplug(DEVINST hDevInst)
{
	DEVINST hDevChild;
	if (const auto Result = CM_Get_Child(&hDevChild, hDevInst, 0); Result != CR_SUCCESS)
	{
		if (Result != CR_NO_SUCH_DEVINST)
			LOGWARNING(L"CM_Get_Child: {}"sv, cr_error(Result));
		return false;
	}

	DWORD Capabilities = 0;
	return GetDeviceProperty(hDevChild, CM_DRP_CAPABILITIES, Capabilities) &&
		!(Capabilities&CM_DEVCAP_SURPRISEREMOVALOK) &&
		(Capabilities&CM_DEVCAP_UNIQUEID);
}

[[nodiscard]]
static bool IsDeviceHotplug(DEVINST hDevInst, bool const IncludeSafeToRemove)
{
	DWORD Capabilities = 0;
	if (!GetDeviceProperty(hDevInst, CM_DRP_CAPABILITIES, Capabilities))
		return false;

	ULONG Status = 0, Problem = 0;
	if (const auto Result = CM_Get_DevNode_Status(&Status, &Problem, hDevInst, 0); Result != CR_SUCCESS)
	{
		LOGWARNING(L"CM_Get_DevNode_Status: {}"sv, cr_error(Result));
		return false;
	}

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
	wchar_t DeviceId[MAX_DEVICE_ID_LEN];
	if (const auto Result = CM_Get_Device_ID(hDevInst, DeviceId, static_cast<ULONG>(std::size(DeviceId)), 0); Result != CR_SUCCESS)
	{
		LOGWARNING(L"CM_Get_Device_ID: {}"sv, cr_error(Result));
		return {};
	}

	wchar_t_ptr_n<os::default_buffer_size> DeviceInterfaceList;

	auto InterfaceId = GUID_DEVINTERFACE_VOLUME;

	for (;;)
	{
		ULONG Size;

		if (const auto Result = CM_Get_Device_Interface_List_Size(&Size, &InterfaceId, DeviceId, CM_GET_DEVICE_INTERFACE_LIST_PRESENT); Result != CR_SUCCESS)
		{
			LOGWARNING(L"CM_Get_Device_Interface_List_Size: {}"sv, cr_error(Result));
			return {};
		}

		DeviceInterfaceList.reset(Size);
		const auto Result = CM_Get_Device_Interface_List(&InterfaceId, DeviceId, DeviceInterfaceList.data(), Size, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
		if (Result == CR_SUCCESS)
			break;

		if (Result == CR_BUFFER_SMALL)
			continue;

		LOGWARNING(L"CM_Get_Device_Interface_List: {}"sv, cr_error(Result));
		return {};
	}

	device_paths DevicePaths;

	for (const auto& i: enum_substrings(DeviceInterfaceList))
	{
		string strMountPoint(i);
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
	wchar_t DeviceId[MAX_DEVICE_ID_LEN];
	if (const auto Result = CM_Get_Device_ID(hDevInst, DeviceId, static_cast<ULONG>(std::size(DeviceId)), 0); Result != CR_SUCCESS)
	{
		LOGWARNING(L"CM_Get_Device_ID: {}"sv, cr_error(Result));
		return {};
	}

	wchar_t_ptr_n<os::default_buffer_size> DeviceIdList;

	for (;;)
	{
		ULONG Size = 0;
		if (const auto Result = CM_Get_Device_ID_List_Size(&Size, DeviceId, CM_GETIDLIST_FILTER_REMOVALRELATIONS); Result != CR_SUCCESS)
		{
			if (Result != CR_NO_SUCH_VALUE)
				LOGWARNING(L"CM_Get_Device_ID_List_Size: {}"sv, cr_error(Result));
			return {};
		}

		DeviceIdList.reset(Size);

		const auto Result = CM_Get_Device_ID_List(DeviceId, DeviceIdList.data(), Size, CM_GETIDLIST_FILTER_REMOVALRELATIONS);
		if (Result == CR_SUCCESS)
			break;

		if (Result == CR_BUFFER_SMALL)
			continue;

		LOGWARNING(L"CM_Get_Device_ID_List: {}"sv, cr_error(Result));
		return {};
	}

	device_paths DevicePaths;
	for (const auto& i: enum_substrings(DeviceIdList))
	{
		DEVINST hRelationDevInst;
		if (const auto Result = CM_Locate_DevNode(&hRelationDevInst, const_cast<DEVINSTID>(i.data()), 0); Result != CR_SUCCESS)
		{
			LOGWARNING(L"CM_Locate_DevNode: {}"sv, cr_error(Result));
			continue;
		}

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
	std::vector<DeviceInfo> Info;

	DEVINST Root;
	if (const auto Result = CM_Locate_DevNode(&Root, nullptr, CM_LOCATE_DEVNODE_NORMAL); Result != CR_SUCCESS)
	{
		LOGWARNING(L"CM_Locate_DevNode: {}"sv, cr_error(Result));
		return Info;
	}

	GetHotplugDevicesInfo(Root, Info, IncludeSafeToRemove);
	return Info;
}

static void RemoveHotplugDriveDevice(const DeviceInfo& Info, bool const Confirm)
{
	string strFriendlyName;
	if (GetDevicePropertyRecursive(Info.DevInst, CM_DRP_FRIENDLYNAME, strFriendlyName))
		inplace::trim(strFriendlyName);

	string strDescription;
	if (GetDevicePropertyRecursive(Info.DevInst, CM_DRP_DEVICEDESC, strDescription))
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
		return;

	auto VetoType = PNP_VetoTypeUnknown;
	wchar_t VetoName[MAX_PATH];
	const auto Result = CM_Request_Device_Eject(Info.DevInst, &VetoType, VetoName, static_cast<ULONG>(std::size(VetoName)), 0);
	if (Result != CR_SUCCESS || VetoType != PNP_VetoTypeUnknown)   //M$ баг, если есть VetoName, то даже при ошибке возвращается CR_SUCCESS
	{
		LOGWARNING(L"CM_Request_Device_Eject: {}"sv, cr_error(Result));
		throw far_exception(
		{
			{ Result == CR_SUCCESS? ERROR_DRIVE_LOCKED : cr_to_win32_error(Result), STATUS_SUCCESS, source_location::current() },
			far::format(L"CM_Request_Device_Eject: 0x{:0>8X}"sv, Result)
		});
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
}

void RemoveHotplugDrive(string_view const Path, bool const Confirm)
{
	// Removing VHD disk as hotplug is a very bad idea.
	// Currently OS removes the device but doesn't close the file handle, rendering the file completely unavailable until reboot.
	if (auto IsVhd = false; detach_vhd(Path, IsVhd) || IsVhd)
		return;

	const auto PathType = ParsePath(Path);
	if (PathType != root_type::win32nt_drive_letter && PathType != root_type::volume)
		return;

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

	if (ItemIterator != Info.cend())
		RemoveHotplugDriveDevice(*ItemIterator, Confirm);
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
			menu_item_ex ListItem;
			string strDescription;
			if (GetDevicePropertyRecursive(i.DevInst, CM_DRP_DEVICEDESC, strDescription) && !strDescription.empty())
			{
				inplace::trim(strDescription);
				ListItem.Name = strDescription;
			}

			string strFriendlyName;
			if (GetDevicePropertyRecursive(i.DevInst, CM_DRP_FRIENDLYNAME, strFriendlyName) && !strFriendlyName.empty())
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
	HotPlugList->EnableAutoHighlight();
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
				if (HotPlugList->empty())
					break;

				const auto I = HotPlugList->GetSelectPos();

				try
				{
					RemoveHotplugDriveDevice(Info[I], Global->Opt->Confirm.RemoveHotPlug);
				}
				catch (far_exception const& e)
				{
					Message(MSG_WARNING, e,
						msg(lng::MError),
						{
							msg(lng::MChangeCouldNotEjectHotPlugMedia2),
							HotPlugList->at(I).Name
						},
						{ lng::MOk });

					break;
				}

				FillMenu();
				break;
			}

			default:
				KeyProcessed = 0;
		}
		return KeyProcessed;
	});
}
