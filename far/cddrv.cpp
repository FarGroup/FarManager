/*
cddrv.cpp

про сидюк
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
#include "cddrv.hpp"

// Internal:
#include "pathmix.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/null_iterator.hpp"
#include "common/string_utils.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

enum cdrom_device_capabilities
{
	CAPABILITIES_NONE     = 0,

	CAPABILITIES_CDROM    = 0_bit,
	CAPABILITIES_CDR      = 1_bit,
	CAPABILITIES_CDRW     = 2_bit,

	CAPABILITIES_DVDROM   = 3_bit,
	CAPABILITIES_DVDR     = 4_bit,
	CAPABILITIES_DVDRW    = 5_bit,
	CAPABILITIES_DVDRAM   = 6_bit,

	CAPABILITIES_BDROM    = 7_bit,
	CAPABILITIES_BDR      = 8_bit,
	CAPABILITIES_BDRW     = 9_bit,

	CAPABILITIES_HDDVDROM = 10_bit,
	CAPABILITIES_HDDVDR   = 11_bit,
	CAPABILITIES_HDDVDRW  = 12_bit,
	CAPABILITIES_HDDVDRAM = 13_bit,
};

static auto operator | (cdrom_device_capabilities const This, cdrom_device_capabilities const Rhs)
{
	return static_cast<cdrom_device_capabilities>(as_underlying_type(This) | Rhs);
}

static auto& operator |= (cdrom_device_capabilities& This, cdrom_device_capabilities const Rhs)
{
	return This = This | Rhs;
}

template<typename T, size_t N, size_t... I>
static auto write_value_to_big_endian_impl(unsigned char (&Dest)[N], T const Value, std::index_sequence<I...>)
{
	(..., (Dest[N - I - 1] = (Value >> (8 * I) & 0xFF)));
}

template<typename T, size_t N>
static auto write_value_to_big_endian(unsigned char (&Dest)[N], T const Value)
{
	return write_value_to_big_endian_impl(Dest, Value, std::make_index_sequence<N>{});
}

template<typename T, size_t N, size_t... I>
static auto read_value_from_big_endian_impl(unsigned char const (&Src)[N], std::index_sequence<I...>)
{
	static_assert(sizeof(T) >= N);
	return T((... | (T(Src[I]) << (8 * (N - I - 1)))));
}

template<typename T, size_t N>
static auto read_value_from_big_endian(unsigned char const (&Src)[N])
{
	return read_value_from_big_endian_impl<T>(Src, std::make_index_sequence<N>{});
}

template<typename T>
static auto& edit_as(void* const Buffer)
{
	static_assert(std::is_trivially_copyable_v<T>);

	return *static_cast<T*>(Buffer);
}

template<typename T>
static auto view_as_if(span<unsigned char const> const Buffer)
{
	static_assert(std::is_trivially_copyable_v<T>);

	return Buffer.size() >= sizeof(T)? view_as<T const*>(Buffer.data()) : nullptr;
}

struct SCSI_PASS_THROUGH_WITH_BUFFERS: SCSI_PASS_THROUGH
{
	UCHAR SenseBuf[32];
	UCHAR DataBuf[512];
};

static void InitSCSIPassThrough(SCSI_PASS_THROUGH_WITH_BUFFERS& Spt)
{
	Spt = {};

	Spt.Length = sizeof(SCSI_PASS_THROUGH);
	Spt.PathId = 0;
	Spt.TargetId = 1;
	Spt.SenseInfoLength = sizeof(Spt.SenseBuf);
	Spt.DataIn = SCSI_IOCTL_DATA_IN;
	Spt.DataTransferLength = sizeof(Spt.DataBuf);
	Spt.TimeOutValue = 2;

WARNING_PUSH()
WARNING_DISABLE_GCC("-Winvalid-offsetof")
WARNING_DISABLE_CLANG("-Winvalid-offsetof")

	Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	Spt.SenseInfoOffset = static_cast<ULONG>(offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf));

WARNING_POP()
}

// http://www.13thmonkey.org/documentation/SCSI/
// https://doc.xdevs.com/doc/Seagate/INF-8090.PDF

static auto profile_to_capabilities(FEATURE_PROFILE_TYPE const Profile)
{
WARNING_PUSH()
WARNING_DISABLE_MSC(4063) // case 'identifier' is not a valid value for switch of enum 'enumeration'
WARNING_DISABLE_GCC("-Wswitch")
WARNING_DISABLE_CLANG("-Wswitch")

	switch (Profile)
	{
	case ProfileCdrom:                     return CAPABILITIES_CDROM;     // 0008h | CD-ROM                                | Read only Compact Disc capable
	case ProfileCdRecordable:              return CAPABILITIES_CDR;       // 0009h | CD-R                                  | Write once Compact Disc capable
	case ProfileCdRewritable:              return CAPABILITIES_CDRW;      // 000Ah | CD-RW                                 | Re-writable Compact Disc capable
	case ProfileDvdRom:                    return CAPABILITIES_DVDROM;    // 0010h | DVD-ROM                               | Read only DVD
	case ProfileDvdRecordable:             return CAPABILITIES_DVDR;      // 0011h | DVD-R Sequential Recording            | Write once DVD using Sequential recording
	case ProfileDvdRam:                    return CAPABILITIES_DVDRAM;    // 0012h | DVD-RAM                               | Re-writable DVD
	case ProfileDvdRewritable:             return CAPABILITIES_DVDRW;     // 0013h | DVD-RW Restricted Overwrite           | Re-recordable DVD using Restricted Overwrite
	case ProfileDvdRWSequential:           return CAPABILITIES_DVDRW;     // 0014h | DVD-RW Sequential recording           | Re-recordable DVD using Sequential recording
	case ProfileDvdDashRDualLayer:         return CAPABILITIES_DVDRW;     // 0015h | DVD-R Dual Layer Sequential Recording | Dual Layer DVD-R using Sequential recording
	case ProfileDvdDashRLayerJump:         return CAPABILITIES_DVDRW;     // 0016h | DVD-R Dual Layer Jump Recording       | Dual Layer DVD-R using Layer Jump recording
	case 0x17:                             return CAPABILITIES_DVDRW;     // 0017h | DVD-RW Dual Layer                     | Re-recordable DVD for Dual Layer
	case 0x18:                             return CAPABILITIES_DVDR;      // 0018h | DVD-Download Disc Recording           | Write once DVD for CSS managed recording
	case ProfileDvdPlusRW:                 return CAPABILITIES_DVDRW;     // 001Ah | DVD+RW                                | DVD+ReWritable
	case ProfileDvdPlusR:                  return CAPABILITIES_DVDR;      // 001Bh | DVD+R                                 | DVD+Recordable
	case ProfileDDCdrom:                   return CAPABILITIES_CDROM;     // 0020h | DDCD-ROM                              | Read only DDCD
	case ProfileDDCdRecordable:            return CAPABILITIES_CDR;       // 0021h | DDCD-R                                | Write only DDCD
	case ProfileDDCdRewritable:            return CAPABILITIES_CDRW;      // 0022h | DDCD-RW                               | Re-Write only DDCD
	case ProfileDvdPlusRWDualLayer:        return CAPABILITIES_DVDRW;     // 002Ah | DVD+RW Dual Layer                     | DVD+Rewritable Dual Layer
	case ProfileDvdPlusRDualLayer:         return CAPABILITIES_DVDR;      // 002Bh | DVD+R Dual Layer                      | DVD+Recordable Dual Layer
	case ProfileBDRom:                     return CAPABILITIES_BDROM;     // 0040h | BD-ROM                                | Blu-ray Disc ROM
	case ProfileBDRSequentialWritable:     return CAPABILITIES_BDR;       // 0041h | BD-R SRM                              | Blu-ray Disc Recordable – Sequential Recording Mode
	case ProfileBDRRandomWritable:         return CAPABILITIES_BDR;       // 0042h | BD-R RRM                              | Blu-ray Disc Recordable – Random Recording Mode
	case ProfileBDRewritable:              return CAPABILITIES_BDRW;      // 0043h | BD-RE                                 | Blu-ray Disc Rewritable
	case ProfileHDDVDRom:                  return CAPABILITIES_HDDVDROM;  // 0050h | HD DVD-ROM                            | Read-only HD DVD
	case ProfileHDDVDRecordable:           return CAPABILITIES_HDDVDR;    // 0051h | HD DVD-R                              | Write-once HD DVD
	case ProfileHDDVDRam:                  return CAPABILITIES_HDDVDRAM;  // 0052h | HD DVD-RAM                            | Rewritable HD DVD
	case ProfileHDDVDRewritable:           return CAPABILITIES_HDDVDRW;   // 0053h | HD DVD-RW                             | Re-recordable HD DVD
	case ProfileHDDVDRDualLayer:           return CAPABILITIES_HDDVDR;    // 0058h | HD DVD-R Dual Layer                   | Write once HD DVD Dual Layer
	case ProfileHDDVDRWDualLayer:          return CAPABILITIES_HDDVDRW;   // 005Ah | HD DVD-RW Dual Layer                  | Re-recordable HD DVD Dual Layer
	default:                               return CAPABILITIES_NONE;
	}

WARNING_POP()
}

static auto capatibilities_from_scsi_configuration(const os::fs::file& Device)
{

	SCSI_PASS_THROUGH_WITH_BUFFERS Spt;
	InitSCSIPassThrough(Spt);

#if !IS_MICROSOFT_SDK()
	// GCC headers incorrectly reserve only one bit for RequestType
	struct CDB_FIXED
	{
		struct
		{
			UCHAR OperationCode;
			UCHAR RequestType : 2;
			UCHAR Reserved1 : 6;
			UCHAR StartingFeature[2];
			UCHAR Reserved2[3];
			UCHAR AllocationLength[2];
			UCHAR Control;
		}
		GET_CONFIGURATION;
	};
#define CDB CDB_FIXED
#endif

	auto& GetConfiguration = edit_as<CDB>(Spt.Cdb).GET_CONFIGURATION;

#if !IS_MICROSOFT_SDK()
#undef CDB
#endif

	GetConfiguration.OperationCode = SCSIOP_GET_CONFIGURATION;
	GetConfiguration.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE;
	write_value_to_big_endian(GetConfiguration.StartingFeature, FeatureProfileList);
	write_value_to_big_endian(GetConfiguration.AllocationLength, sizeof(Spt.DataBuf));
	Spt.CdbLength = CDB10GENERIC_LENGTH;

	if (!Device.IoControl(IOCTL_SCSI_PASS_THROUGH, &Spt, sizeof(SCSI_PASS_THROUGH), &Spt, sizeof(Spt)) || Spt.ScsiStatus != SCSISTAT_GOOD)
		return CAPABILITIES_NONE;

	span Buffer(Spt.DataBuf, Spt.DataTransferLength);

	const auto ConfigurationHeader = view_as_if<GET_CONFIGURATION_HEADER>(Buffer);
	if (!ConfigurationHeader || Buffer.size() < sizeof(ConfigurationHeader->DataLength) + read_value_from_big_endian<size_t>(ConfigurationHeader->DataLength))
		return CAPABILITIES_NONE;

	Buffer.pop_front(sizeof(*ConfigurationHeader));

	const auto FeatureList = view_as_if<FEATURE_DATA_PROFILE_LIST>(Buffer);
	if (!FeatureList)
		return CAPABILITIES_NONE;

	if (read_value_from_big_endian<FEATURE_NUMBER>(FeatureList->Header.FeatureCode) != FeatureProfileList)
		return CAPABILITIES_NONE;

	const span Profiles(FeatureList->Profiles, FeatureList->Header.AdditionalLength / sizeof(*FeatureList->Profiles));

	return std::accumulate(ALL_CONST_RANGE(Profiles), CAPABILITIES_NONE, [](auto const Value, auto const& i)
	{
		return Value | profile_to_capabilities(read_value_from_big_endian<FEATURE_PROFILE_TYPE>(i.ProfileNumber));
	});
}

static auto capatibilities_from_scsi_mode_sense(const os::fs::file& Device)
{
	SCSI_PASS_THROUGH_WITH_BUFFERS Spt;
	InitSCSIPassThrough(Spt);

	auto& ModeSense = edit_as<CDB>(Spt.Cdb).MODE_SENSE;
	ModeSense.OperationCode = SCSIOP_MODE_SENSE;
	ModeSense.Dbd = true;
	ModeSense.Pc = MODE_SENSE_CURRENT_VALUES;
	ModeSense.PageCode = MODE_PAGE_CAPABILITIES;
	ModeSense.AllocationLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(CDVD_CAPABILITIES_PAGE);
	Spt.CdbLength = CDB6GENERIC_LENGTH;

	if (!Device.IoControl(IOCTL_SCSI_PASS_THROUGH, &Spt, sizeof(SCSI_PASS_THROUGH), &Spt, sizeof(Spt)) || Spt.ScsiStatus != SCSISTAT_GOOD)
		return CAPABILITIES_NONE;

	const auto& CapsPage = view_as<CDVD_CAPABILITIES_PAGE>(Spt.DataBuf + sizeof(MODE_PARAMETER_HEADER));

	auto caps = CAPABILITIES_CDROM;

	if (CapsPage.CDRRead)
		caps |= CAPABILITIES_CDROM;

	if (CapsPage.CDERead)
		caps |= CAPABILITIES_CDROM;

	if (CapsPage.DVDROMRead)
		caps |= CAPABILITIES_DVDROM;

	if (CapsPage.DVDRRead)
		caps |= CAPABILITIES_DVDROM;

	if (CapsPage.DVDRAMRead)
		caps |= CAPABILITIES_DVDRAM;

	if (CapsPage.CDRWrite)
		caps |= CAPABILITIES_CDR;

	if (CapsPage.CDEWrite)
		caps |= CAPABILITIES_CDRW;

	if (CapsPage.DVDRWrite)
		caps |= CAPABILITIES_DVDR;

	if (CapsPage.DVDRAMWrite)
		caps |= CAPABILITIES_DVDRAM;

	return caps;
}

static auto product_id_to_capatibilities(const char* const ProductId)
{
	string ProductIdFiltered;
	const auto Iterator = null_iterator(ProductId);
	std::copy_if(Iterator, Iterator.end(), std::back_inserter(ProductIdFiltered), isalpha);

	static const struct
	{
		string_view Pattern;
		std::initializer_list<string_view> AntipatternsBefore, AntipatternsAfter;
		cdrom_device_capabilities Capabilities;
	}
	Capabilities[]
	{
		{ L"CDROM"sv,     {},        {},                            CAPABILITIES_CDROM    },
		{ L"CDR"sv,       {},        { L"OM"sv, L"W"sv },           CAPABILITIES_CDR      },
		{ L"CDRW"sv,      {},        {},                            CAPABILITIES_CDRW     },
		{ L"DVDROM"sv,    {L"HD"sv}, {},                            CAPABILITIES_DVDROM   },
		{ L"DVDR"sv,      {L"HD"sv}, { L"OM"sv, L"W"sv, L"AM"sv },  CAPABILITIES_DVDR     },
		{ L"DVDRW"sv,     {L"HD"sv}, {},                            CAPABILITIES_DVDRW    },
		{ L"DVDRAM"sv,    {L"HD"sv}, {},                            CAPABILITIES_DVDRAM   },
		{ L"BDROM"sv,     {},        {},                            CAPABILITIES_BDROM    },
		{ L"BDR"sv,       {},        { L"OM"sv, L"W"sv },           CAPABILITIES_BDR      },
		{ L"BDRW"sv,      {},        {},                            CAPABILITIES_BDRW     },
		{ L"HDDVDROM"sv,  {},        {},                            CAPABILITIES_HDDVDROM },
		{ L"HDDVDR"sv,    {},        { L"OM"sv, L"W"sv, L"AM"sv },  CAPABILITIES_HDDVDR   },
		{ L"HDDVDRW"sv,   {},        {},                            CAPABILITIES_HDDVDRW  },
		{ L"HDDVDRAM"sv,  {},        {},                            CAPABILITIES_HDDVDRAM },
	};

	return std::accumulate(ALL_CONST_RANGE(Capabilities), CAPABILITIES_NONE, [Id = string_view(ProductIdFiltered)](auto const Value, auto const& i)
	{
		const auto Pos = Id.find(i.Pattern);
		if (Pos == i.Pattern.npos)
			return Value;

		if (
			const auto Prefix = Id.substr(0, Pos);
			std::any_of(ALL_CONST_RANGE(i.AntipatternsBefore),
				[&](string_view const Str){ return ends_with(Prefix, Str); })
		)
			return Value;

		if (
			const auto Suffix = Id.substr(Pos + i.Pattern.size());
			std::any_of(ALL_CONST_RANGE(i.AntipatternsAfter),
				[&](string_view const Str){ return starts_with(Suffix, Str); })
		)
			return Value;

		return Value | i.Capabilities;
	});
}

static auto capatibilities_from_product_id(const os::fs::file& Device)
{
	STORAGE_DESCRIPTOR_HEADER DescriptorHeader{};
	STORAGE_PROPERTY_QUERY PropertyQuery{ StorageDeviceProperty, PropertyStandardQuery };

	if (!Device.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, &PropertyQuery, sizeof(PropertyQuery), &DescriptorHeader, sizeof(DescriptorHeader)) || !DescriptorHeader.Size)
		return CAPABILITIES_NONE;

	const char_ptr_n<os::default_buffer_size> Buffer(DescriptorHeader.Size);
	if (!Device.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, &PropertyQuery, sizeof(PropertyQuery), Buffer.data(), static_cast<DWORD>(Buffer.size())))
		return CAPABILITIES_NONE;

	const auto& DeviceDescriptor = view_as<STORAGE_DEVICE_DESCRIPTOR>(Buffer.data());

	if (!DeviceDescriptor.ProductIdOffset || !Buffer[DeviceDescriptor.ProductIdOffset])
		return CAPABILITIES_NONE;

	return product_id_to_capatibilities(&Buffer[DeviceDescriptor.ProductIdOffset]);
}

static auto get_device_capabilities(const os::fs::file& Device)
{
	for (const auto& Handler:
	{
		// Most relevant
		capatibilities_from_scsi_configuration,
		// Legacy
		capatibilities_from_scsi_mode_sense,
		// Trust your eyes
		capatibilities_from_product_id
	})
	{
		if (const auto Result = Handler(Device); Result != CAPABILITIES_NONE)
			return Result;
	}

	return CAPABILITIES_NONE;
}

static auto get_cd_type(cdrom_device_capabilities const caps)
{
	static const std::pair<cd_type, int> DeviceCaps[]
	{
		{ cd_type::hddvdram,     CAPABILITIES_HDDVDRAM },
		{ cd_type::hddvdrw,      CAPABILITIES_HDDVDRW },
		{ cd_type::hddvdr,       CAPABILITIES_HDDVDR },
		{ cd_type::hddvdrom,     CAPABILITIES_HDDVDROM },
		{ cd_type::bdrw,         CAPABILITIES_BDRW },
		{ cd_type::bdr,          CAPABILITIES_BDR },
		{ cd_type::bdrom,        CAPABILITIES_BDROM },
		{ cd_type::dvdram,       CAPABILITIES_DVDRAM },
		{ cd_type::dvdrw,        CAPABILITIES_DVDRW },
		{ cd_type::cdrwdvd,      CAPABILITIES_CDRW | CAPABILITIES_DVDROM },
		{ cd_type::dvdrom,       CAPABILITIES_DVDROM },
		{ cd_type::cdrw,         CAPABILITIES_CDRW },
		{ cd_type::cdrom,        CAPABILITIES_CDROM },
	};

	const auto ItemIterator = std::find_if(CONST_RANGE(DeviceCaps, i)
	{
		return (caps & i.second) == i.second;
	});

	return ItemIterator == std::cend(DeviceCaps)? cd_type::cdrom : ItemIterator->first;
}

bool DriveCanBeVirtual(unsigned DriveType)
{
	return (DriveType == DRIVE_FIXED && IsWindows7OrGreater()) || (DriveType == DRIVE_CDROM && IsWindows8OrGreater());
}

cd_type get_cdrom_type(string_view RootDir)
{
	string VolumePath(RootDir);
	DeleteEndSlash(VolumePath);

	if (starts_with(VolumePath, L"\\\\?\\"sv))
	{
		VolumePath[2] = L'.';
	}
	else
	{
		VolumePath.insert(0, L"\\\\.\\"sv);
	}

	if (const auto Device = os::fs::file(VolumePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
	{
		if (const auto Capabilities = get_device_capabilities(Device); Capabilities != CAPABILITIES_NONE)
			return get_cd_type(Capabilities);
	}

	// TODO: try WMI

	return cd_type::cdrom;
}

bool is_removable_usb(string_view RootDir)
{
	// media has to be inserted
	string drive(HasPathPrefix(RootDir)? RootDir : L"\\\\?\\"sv + RootDir);
	DeleteEndSlash(drive);

	const auto Device = os::fs::file(drive, STANDARD_RIGHTS_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
	if (!Device)
		return false;

	DISK_GEOMETRY DiskGeometry;
	if (!Device.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &DiskGeometry, sizeof(DiskGeometry)))
		return false;

	return DiskGeometry.MediaType == FixedMedia || DiskGeometry.MediaType == RemovableMedia;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("product_id_to_capatibilities")
{
	static const struct
	{
		const char* Src;
		cdrom_device_capabilities Result;
	}
	Tests[]
	{
		{ "CD-ROM",      CAPABILITIES_CDROM     },
		{ "CD+R",        CAPABILITIES_CDR       },
		{ "CD+-RW Foo",  CAPABILITIES_CDRW      },
		{ "DVD ROM",     CAPABILITIES_DVDROM    },
		{ "DVD_R_Bar",   CAPABILITIES_DVDR      },
		{ "123+DVD+RW",  CAPABILITIES_DVDRW     },
		{ "DVD+RAM",     CAPABILITIES_DVDRAM    },
		{ "BD_ROM",      CAPABILITIES_BDROM     },
		{ "UberBDR",     CAPABILITIES_BDR       },
		{ "HDBD/RW",     CAPABILITIES_BDRW      },
		{ "HD-DVD-ROM",  CAPABILITIES_HDDVDROM  },
		{ "HDDVDR",      CAPABILITIES_HDDVDR    },
		{ "HDDVD RW",    CAPABILITIES_HDDVDRW   },
		{ "HD/DVD+RAM",  CAPABILITIES_HDDVDRAM  },
	};

	for (const auto& i : Tests)
	{
		REQUIRE(i.Result == product_id_to_capatibilities(i.Src));
	}
}
#endif
