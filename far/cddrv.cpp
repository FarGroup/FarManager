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

#include "headers.hpp"
#pragma hdrstop

#include "cddrv.hpp"
#include "drivemix.hpp"
#include "flink.hpp"
#include "pathmix.hpp"
#include "string_utils.hpp"

enum CDROM_DeviceCapabilities
{
	CAPABILITIES_NONE            = 0,

	//CD
	CAPABILITIES_READ_CDROM      = bit(0),
	CAPABILITIES_READ_CDR        = bit(1),
	CAPABILITIES_READ_CDRW       = bit(2),

	CAPABILITIES_WRITE_CDR       = bit(3),
	CAPABILITIES_WRITE_CDRW      = bit(4),

	//DVD
	CAPABILITIES_READ_DVDROM     = bit(5),
	CAPABILITIES_READ_DVDR       = bit(6),
	CAPABILITIES_READ_DVDRW      = bit(7),
	CAPABILITIES_READ_DVDRAM     = bit(8),

	CAPABILITIES_WRITE_DVDR      = bit(9),
	CAPABILITIES_WRITE_DVDRW     = bit(10),
	CAPABILITIES_WRITE_DVDRAM    = bit(11),

	//BlueRay
	CAPABILITIES_READ_BDROM      = bit(12),
	CAPABILITIES_WRITE_BDROM     = bit(13),

	//HD-DVD
	CAPABILITIES_READ_HDDVD      = bit(14),
	CAPABILITIES_WRITE_HDDVD     = bit(15),

	//GENERIC

	CAPABILITIES_GENERIC_CDROM   = CAPABILITIES_READ_CDROM | CAPABILITIES_READ_CDR | CAPABILITIES_READ_CDRW,
	CAPABILITIES_GENERIC_CDRW    = CAPABILITIES_WRITE_CDR | CAPABILITIES_WRITE_CDRW,
	CAPABILITIES_GENERIC_DVDROM  = CAPABILITIES_READ_DVDROM | CAPABILITIES_READ_DVDR | CAPABILITIES_READ_DVDRW | CAPABILITIES_READ_DVDRAM,
	CAPABILITIES_GENERIC_DVDRW   = CAPABILITIES_WRITE_DVDR | CAPABILITIES_WRITE_DVDRW,
	CAPABILITIES_GENERIC_DVDRAM  = CAPABILITIES_WRITE_DVDRAM,
	
	CAPABILITIES_GENERIC_BDROM   = CAPABILITIES_READ_BDROM,
	CAPABILITIES_GENERIC_BDRW    = CAPABILITIES_WRITE_BDROM,

	CAPABILITIES_GENERIC_HDDVD   = CAPABILITIES_READ_HDDVD,
	CAPABILITIES_GENERIC_HDDVDRW = CAPABILITIES_WRITE_HDDVD
};

static CDROM_DeviceCapabilities getCapsUsingProductId(const char* prodID)
{
	std::string productID;
	const auto Iterator = null_iterator(prodID);
	std::copy_if(Iterator, Iterator.end(), std::back_inserter(productID), is_alpha);

	static const std::pair<const char*, int> Capabilities[] =
	{
		{"CD", CAPABILITIES_GENERIC_CDROM},
		{"CDRW", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_CDRW},
		{"DVD", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM},
		{"DVDRW", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_DVDRW},
		{"DVDRAM", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_DVDRW|CAPABILITIES_GENERIC_DVDRAM},
		{"BDROM", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_DVDRAM},
		{"HDDVD", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_HDDVD},
	};

	return std::accumulate(ALL_CONST_RANGE(Capabilities), CAPABILITIES_NONE, [&productID](auto Value, const auto& i)
	{
		return static_cast<CDROM_DeviceCapabilities>(Value | (contains(productID, i.first)? i.second : 0));
	});
}

static void InitSCSIPassThrough(SCSI_PASS_THROUGH_WITH_BUFFERS& Sptwb)
{
	Sptwb = {};

	Sptwb.Spt.PathId = 0;
	Sptwb.Spt.TargetId = 1;
	Sptwb.Spt.Length = sizeof(Sptwb.Spt);
	Sptwb.Spt.SenseInfoLength = 24;
	Sptwb.Spt.SenseInfoOffset = static_cast<ULONG>(offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf));
	Sptwb.Spt.DataTransferLength = sizeof(Sptwb.DataBuf);
	Sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	Sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	Sptwb.Spt.TimeOutValue = 2;
}

static CDROM_DeviceCapabilities getCapsUsingMagic(const os::fs::file& Device)
{
	int caps = CAPABILITIES_NONE;

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	const auto sDataLength = sizeof(sptwb.DataBuf);

	InitSCSIPassThrough(sptwb);

	//MODE SENSE FIRST
	sptwb.Spt.Cdb[0] = SCSIOP_MODE_SENSE;
	sptwb.Spt.Cdb[1] = 0x08;                    // target shall not return any block descriptors
	sptwb.Spt.Cdb[2] = MODE_PAGE_CAPABILITIES;
	sptwb.Spt.Cdb[4] = 192;
	sptwb.Spt.CdbLength = 6;

	DWORD returned = 0;

	if ( Device.IoControl(
			IOCTL_SCSI_PASS_THROUGH,
			&sptwb,
			sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS),
			&sptwb,
			sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS),
			&returned
			) && (sptwb.Spt.ScsiStatus == 0) )
	{
		// Notes:
		// 1. The header of 6-byte MODE commands is 4 bytes long.
		// 2. No Block Descriptors returned before parameter page as was specified when building the Mode command.
		// 3. First two bytes of a parameter page are the Page Code and Page Length bytes.
		// Therefore, our useful data starts at the 7th byte in the data buffer.
		caps = CAPABILITIES_READ_CDROM;

		if ( sptwb.DataBuf[6] & 0x01 )
			caps |= CAPABILITIES_READ_CDR;

		if ( sptwb.DataBuf[6] & 0x02 )
			caps |= CAPABILITIES_READ_CDRW;

		if ( sptwb.DataBuf[6] & 0x08 )
			caps |= CAPABILITIES_READ_DVDROM;

		if ( sptwb.DataBuf[6] & 0x10 )
			caps |= (CAPABILITIES_READ_DVDR | CAPABILITIES_READ_DVDRW);

		if ( sptwb.DataBuf[6] & 0x20 )
			caps |= CAPABILITIES_READ_DVDRAM;

		if ( sptwb.DataBuf[7] & 0x01 )
			caps |= CAPABILITIES_WRITE_CDR;

		if ( sptwb.DataBuf[7] & 0x02 )
			caps |= CAPABILITIES_WRITE_CDRW;

		if ( sptwb.DataBuf[7] & 0x10 )
			caps |= (CAPABILITIES_WRITE_DVDR | CAPABILITIES_WRITE_DVDRW);

		if ( sptwb.DataBuf[7] & 0x20 )
			caps |= CAPABILITIES_WRITE_DVDRAM;
	}

	// GET CONFIGURATION NOW
	InitSCSIPassThrough(sptwb);

	sptwb.Spt.Cdb[0] = 0x46; //GET CONFIGURATION
	sptwb.Spt.Cdb[7] = static_cast<unsigned char>(sDataLength >> 8);	// Allocation length (MSB).
	sptwb.Spt.Cdb[8] = static_cast<unsigned char>(sDataLength & 0xff);	// Allocation length (LSB).
	sptwb.Spt.Cdb[9] = 0x00;
	sptwb.Spt.CdbLength = 10;

	returned = 0;

	if ( Device.IoControl(
			IOCTL_SCSI_PASS_THROUGH,
			&sptwb,
			sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS),
			&sptwb,
			sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS),
			&returned
			) && (sptwb.Spt.ScsiStatus == 0) )
	{
		const auto* ptr = sptwb.DataBuf;
		const auto* ptr_end = &sptwb.DataBuf[sDataLength];

		ptr += 8;

		enum MMC_Features
		{
			MMC_FEATUREPROFILE_LIST             = 0x0000,
			MMC_FEATURECORE                     = 0x0001,
			MMC_FEATURE_MORPHING                = 0x0002,
			MMC_FEATURE_REMOVABLE               = 0x0003,
			MMC_FEATURE_WRITE_PROTECT           = 0x0004,
			MMC_FEATURE_RANDOM_READ             = 0x0010,
			MMC_FEATURE_MULTIREAD               = 0x001D,
			MMC_FEATURE_CD_READ                 = 0x001E,
			MMC_FEATURE_DVD_READ                = 0x001F,
			MMC_FEATURE_RANDOM_WRITE            = 0x0020,
			MMC_FEATURE_INC_STREAM_WRITE        = 0x0021,
			MMC_FEATURE_SECTOR_ERASE            = 0x0022,
			MMC_FEATURE_FORMAT                  = 0x0023,
			MMC_FEATURE_HW_DEFECT_MANAGEMENT    = 0x0024,
			MMC_FEATURE_WRITE_ONCE              = 0x0025,
			MMC_FEATURE_RESTRICTED_OW           = 0x0026,
			MMC_FEATURE_CWRW_CAV_WRITE          = 0x0027,
			MMC_FEATURE_MRW                     = 0x0028,
			MMC_FEATURE_ENH_DEFECT_REPORT       = 0x0029,
			MMC_FEATURE_DVDPLUSRW               = 0x002A,
			MMC_FEATURE_DVDPLUSR                = 0x002B,
			MMC_FEATURE_RIGID_RESTRICTED_OW     = 0x002C,
			MMC_FEATURE_CD_TAO                  = 0x002D,
			MMC_FEATURE_CD_MASTERING            = 0x002E,
			MMC_FEATURE_DVDMINUSR_RW_WRITE      = 0x002F,
			MMC_FEATURE_DDCD_READ               = 0x0030,
			MMC_FEATURE_DDCDR_WRITE             = 0x0031,
			MMC_FEATURE_DDCDRW_WRITE            = 0x0032,
			MMC_FEATURE_CDRW_WRITE              = 0x0037,
			MMC_FEATURE_POWER_MANAGEMENT        = 0x0100,
			MMC_FEATURE_SMART                   = 0x0101,
			MMC_FEATURE_EMBEDDED_CHARGER        = 0x0102,
			MMC_FEATURE_CD_AUDIO_ANALOG         = 0x0103,
			MMC_FEATURE_MICROCODE_UPGRADE       = 0x0104,
			MMC_FEATURE_TIMEOUT                 = 0x0105,
			MMC_FEATURE_DVD_CSS                 = 0x0106,
			MMC_FEATURE_REALTIME_STREAM         = 0x0107,
			MMC_FEATURE_DRIVE_SN                = 0x0108,
			MMC_FEATURE_DISC_CTRL_BLOCKS        = 0x010A,
			MMC_FEATURE_DVD_CPRM                = 0x010B,
			MMC_FEATURE_FIRMWARE_INFO           = 0x010C,
			// MMC-5/MMC-6.
			MMC_FEATURE_LAYER_JUMP_REC          = 0x0033,
			MMC_FEATURE_BDR_POW                 = 0x0038,
			MMC_FEATURE_DVDPLUSRW_DL            = 0x003A,
			MMC_FEATURE_DVDPLUSR_DL             = 0x003B,
			MMC_FEATURE_BD_READ                 = 0x0040,
			MMC_FEATURE_BD_WRITE                = 0x0041,
			MMC_FEATURE_TSR                     = 0x0042,
			MMC_FEATURE_HDDVD_READ              = 0x0050,
			MMC_FEATURE_HDDVD_WRITE             = 0x0051,
			MMC_FEATURE_HYBRID_DISC             = 0x0080,
			MMC_FEATURE_AACS                    = 0x010D,
			MMC_FEATURE_VCPS                    = 0x0110
		};

		while ( ptr < ptr_end )
		{
			switch (static_cast<MMC_Features>((ptr[0] << 8) | ptr[1]))
			{
				default:
					break;

					//USEFULL ONLY IF MODE SENSE FAILED
				case MMC_FEATURE_CD_READ:
					caps |= CAPABILITIES_READ_CDROM; //useless junk
					break;

				case MMC_FEATURE_DVDPLUSRW:
					caps |= CAPABILITIES_READ_DVDRW; //if we have write support, it was determined by mode sense
					break;

				case MMC_FEATURE_DVDPLUSR:
					caps |= CAPABILITIES_READ_DVDR; //if we have write support, it was determined by mode sense
					break;

				case MMC_FEATURE_DVDPLUSRW_DL:
					caps |= CAPABILITIES_READ_DVDRW; //if we have write support, it was determined by mode sense
					break;

				case MMC_FEATURE_DVDPLUSR_DL:
					caps |= CAPABILITIES_READ_DVDR; //if we have write support, it was determined by mode sense
					break;

				//REALLY USEFUL
				case MMC_FEATURE_BD_READ:
					caps |= CAPABILITIES_READ_BDROM;
					break;

				case MMC_FEATURE_BD_WRITE:
					caps |= CAPABILITIES_WRITE_BDROM;
					break;

				case MMC_FEATURE_HDDVD_READ:
					caps |= CAPABILITIES_READ_HDDVD;
					break;

				case MMC_FEATURE_HDDVD_WRITE:
					caps |= CAPABILITIES_WRITE_HDDVD;
					break;
			}

			ptr += ptr[3];
			ptr += 4;
		}
	}

	return static_cast<CDROM_DeviceCapabilities>(caps);
}

static CDROM_DeviceCapabilities getCapsUsingDeviceProps(const os::fs::file& Device)
{
	STORAGE_DESCRIPTOR_HEADER hdr{};
	STORAGE_PROPERTY_QUERY query{ StorageDeviceProperty, PropertyStandardQuery };
	DWORD returnedLength;
	if (!Device.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), &hdr, sizeof(hdr), &returnedLength) || !hdr.Size)
		return CAPABILITIES_NONE;

	std::vector<char> Buffer(hdr.Size);
	if (!Device.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), Buffer.data(), static_cast<DWORD>(Buffer.size()), &returnedLength))
		return CAPABILITIES_NONE;

	const auto devDesc = reinterpret_cast<const PSTORAGE_DEVICE_DESCRIPTOR>(Buffer.data());

	if (!devDesc->ProductIdOffset || !Buffer[devDesc->ProductIdOffset])
		return CAPABILITIES_NONE;

	return getCapsUsingProductId(&Buffer[devDesc->ProductIdOffset]);
}

static CDROM_DeviceCapabilities GetDeviceCapabilities(const os::fs::file& Device)
{
	const auto caps = getCapsUsingMagic(Device);
	if (caps != CAPABILITIES_NONE)
		return caps;

	return getCapsUsingDeviceProps(Device);
}

static UINT GetDeviceTypeByCaps(CDROM_DeviceCapabilities caps)
{
	static const std::pair<int, int> DeviceCaps[] =
	{
		{DRIVE_BD_RW, CAPABILITIES_GENERIC_BDRW},
		{DRIVE_BD_ROM, CAPABILITIES_GENERIC_BDROM},
		{DRIVE_HDDVD_RW, CAPABILITIES_GENERIC_HDDVDRW},
		{DRIVE_HDDVD_ROM, CAPABILITIES_GENERIC_HDDVD},
		{DRIVE_DVD_RAM, CAPABILITIES_GENERIC_DVDRAM},
		{DRIVE_DVD_RW, CAPABILITIES_GENERIC_DVDRW},
		{DRIVE_CD_RWDVD, CAPABILITIES_GENERIC_CDRW|CAPABILITIES_GENERIC_DVDROM},
		{DRIVE_DVD_ROM, CAPABILITIES_GENERIC_DVDROM},
		{DRIVE_CD_RW, CAPABILITIES_GENERIC_CDRW},
		{DRIVE_CDROM, CAPABILITIES_GENERIC_CDROM},
	};

	const auto ItemIterator = std::find_if(CONST_RANGE(DeviceCaps, i)
	{
		return (caps & i.second) == i.second;
	});

	return ItemIterator == std::cend(DeviceCaps)? DRIVE_UNKNOWN : ItemIterator->first;
}

bool IsDriveTypeCDROM(UINT DriveType)
{
	return DriveType == DRIVE_CDROM || (DriveType >= DRIVE_CD_RW && DriveType <= DRIVE_HDDVD_RW);
}

bool DriveCanBeVirtual(UINT DriveType)
{
	return (DriveType == DRIVE_FIXED && IsWindows7OrGreater()) || (IsDriveTypeCDROM(DriveType) && IsWindows8OrGreater());
}

UINT FAR_GetDriveType(const string& RootDir, DWORD Detect)
{
	auto strRootDir = RootDir.empty()? GetPathRoot(os::GetCurrentDirectory()) : RootDir;
	AddEndSlash(strRootDir);

	UINT DrvType = GetDriveType(strRootDir.data());

	// анализ CD-привода
	if ((Detect&1) && DrvType == DRIVE_CDROM)
	{
		string VolumePath = strRootDir;
		DeleteEndSlash(VolumePath);

		if (starts_with(VolumePath, L"\\\\?\\"_sv))
		{
			VolumePath[2] = L'.';
		}
		else
		{
			constexpr auto UncDevicePrefix = L"\\\\.\\"_sv;
			VolumePath.insert(0, UncDevicePrefix.raw_data(), UncDevicePrefix.size());
		}

		if(const auto Device = os::fs::file(VolumePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
		{
			DrvType = GetDeviceTypeByCaps(GetDeviceCapabilities(Device));
		}

		if (DrvType == DRIVE_UNKNOWN) // фигня могла кака-нить произойти, посему...
			DrvType=DRIVE_CDROM;       // ...вертаем в зад сидюк.
	}

	if ( 0 != (Detect & 2) && DrvType == DRIVE_REMOVABLE )
	{
		// media have to be inserted!
		//
		string drive = HasPathPrefix(strRootDir) ? strRootDir : L"\\\\?\\"_sv + strRootDir;
		DeleteEndSlash(drive);

		DrvType = DRIVE_USBDRIVE; // default type if detection failed
		if (const auto Device = os::fs::file(drive, STANDARD_RIGHTS_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
		{
			DISK_GEOMETRY g;
			DWORD dwOutBytes;
			if (Device.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &g, sizeof(g), &dwOutBytes, nullptr))
				if (g.MediaType != FixedMedia && g.MediaType != RemovableMedia)
					DrvType = DRIVE_REMOVABLE;
		}
	}

	return DrvType;
}
