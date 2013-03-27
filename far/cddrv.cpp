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

enum CDROM_DeviceCapabilities
{
	CAPABILITIES_NONE			= 0x00000000,

	//CD
	CAPABILITIES_READ_CDROM		= 0x00000001,
	CAPABILITIES_READ_CDR		= 0x00000002,
	CAPABILITIES_READ_CDRW		= 0x00000004,

	CAPABILITIES_WRITE_CDR		= 0x00000008,
	CAPABILITIES_WRITE_CDRW		= 0x00000010,

	//DVD
	CAPABILITIES_READ_DVDROM	= 0x00000020,
	CAPABILITIES_READ_DVDR		= 0x00000040,
	CAPABILITIES_READ_DVDRW		= 0x00000080,
	CAPABILITIES_READ_DVDRAM	= 0x00000100,

	CAPABILITIES_WRITE_DVDR		= 0x00000200,
	CAPABILITIES_WRITE_DVDRW	= 0x00000400,
	CAPABILITIES_WRITE_DVDRAM	= 0x00000800,

	//BlueRay
	CAPABILITIES_READ_BDROM		= 0x00001000,
	CAPABILITIES_WRITE_BDROM	= 0x00002000,

	//HD-DVD
	CAPABILITIES_READ_HDDVD		= 0x00004000,
	CAPABILITIES_WRITE_HDDVD	= 0x00008000,

	//GENERIC

	CAPABILITIES_GENERIC_CDROM		= CAPABILITIES_READ_CDROM | CAPABILITIES_READ_CDR | CAPABILITIES_READ_CDRW,
	CAPABILITIES_GENERIC_CDRW		= CAPABILITIES_WRITE_CDR | CAPABILITIES_WRITE_CDRW,
	CAPABILITIES_GENERIC_DVDROM		= CAPABILITIES_READ_DVDROM | CAPABILITIES_READ_DVDR | CAPABILITIES_READ_DVDRW | CAPABILITIES_READ_DVDRAM,
	CAPABILITIES_GENERIC_DVDRW		= CAPABILITIES_WRITE_DVDR | CAPABILITIES_WRITE_DVDRW,
	CAPABILITIES_GENERIC_DVDRAM		= CAPABILITIES_WRITE_DVDRAM,
	
	CAPABILITIES_GENERIC_BDROM		= CAPABILITIES_READ_BDROM,
	CAPABILITIES_GENERIC_BDRW 		= CAPABILITIES_WRITE_BDROM,

	CAPABILITIES_GENERIC_HDDVD		= CAPABILITIES_READ_HDDVD,
	CAPABILITIES_GENERIC_HDDVDRW	= CAPABILITIES_WRITE_HDDVD
};

enum MMC_Features
{
	MMC_FEATUREPROFILE_LIST				= 0x0000,
	MMC_FEATURECORE						= 0x0001,
	MMC_FEATURE_MORPHING				= 0x0002,
	MMC_FEATURE_REMOVABLE				= 0x0003,
	MMC_FEATURE_WRITE_PROTECT			= 0x0004,
	MMC_FEATURE_RANDOM_READ				= 0x0010,
	MMC_FEATURE_MULTIREAD				= 0x001D,
	MMC_FEATURE_CD_READ					= 0x001E,
	MMC_FEATURE_DVD_READ				= 0x001F,
	MMC_FEATURE_RANDOM_WRITE			= 0x0020,
	MMC_FEATURE_INC_STREAM_WRITE		= 0x0021,
	MMC_FEATURE_SECTOR_ERASE			= 0x0022,
	MMC_FEATURE_FORMAT					= 0x0023,
	MMC_FEATURE_HW_DEFECT_MANAGEMENT	= 0x0024,
	MMC_FEATURE_WRITE_ONCE				= 0x0025,
	MMC_FEATURE_RESTRICTED_OW			= 0x0026,
	MMC_FEATURE_CWRW_CAV_WRITE			= 0x0027,
	MMC_FEATURE_MRW						= 0x0028,
	MMC_FEATURE_ENH_DEFECT_REPORT		= 0x0029,
	MMC_FEATURE_DVDPLUSRW				= 0x002A,
	MMC_FEATURE_DVDPLUSR				= 0x002B,
	MMC_FEATURE_RIGID_RESTRICTED_OW		= 0x002C,
	MMC_FEATURE_CD_TAO					= 0x002D,
	MMC_FEATURE_CD_MASTERING			= 0x002E,
	MMC_FEATURE_DVDMINUSR_RW_WRITE		= 0x002F,
	MMC_FEATURE_DDCD_READ				= 0x0030,
	MMC_FEATURE_DDCDR_WRITE				= 0x0031,
	MMC_FEATURE_DDCDRW_WRITE			= 0x0032,
	MMC_FEATURE_CDRW_WRITE				= 0x0037,
	MMC_FEATURE_POWER_MANAGEMENT		= 0x0100,
	MMC_FEATURE_SMART					= 0x0101,
	MMC_FEATURE_EMBEDDED_CHARGER		= 0x0102,
	MMC_FEATURE_CD_AUDIO_ANALOG			= 0x0103,
	MMC_FEATURE_MICROCODE_UPGRADE		= 0x0104,
	MMC_FEATURE_TIMEOUT					= 0x0105,
	MMC_FEATURE_DVD_CSS					= 0x0106,
	MMC_FEATURE_REALTIME_STREAM			= 0x0107,
	MMC_FEATURE_DRIVE_SN				= 0x0108,
	MMC_FEATURE_DISC_CTRL_BLOCKS		= 0x010A,
	MMC_FEATURE_DVD_CPRM				= 0x010B,
	MMC_FEATURE_FIRMWARE_INFO			= 0x010C,
	// MMC-5/MMC-6.
	MMC_FEATURE_LAYER_JUMP_REC			= 0x0033,
	MMC_FEATURE_BDR_POW					= 0x0038,
	MMC_FEATURE_DVDPLUSRW_DL			= 0x003A,
	MMC_FEATURE_DVDPLUSR_DL				= 0x003B,
	MMC_FEATURE_BD_READ					= 0x0040,
	MMC_FEATURE_BD_WRITE				= 0x0041,
	MMC_FEATURE_TSR						= 0x0042,
	MMC_FEATURE_HDDVD_READ				= 0x0050,
	MMC_FEATURE_HDDVD_WRITE				= 0x0051,
	MMC_FEATURE_HYBRID_DISC				= 0x0080,
	MMC_FEATURE_AACS					= 0x010D,
	MMC_FEATURE_VCPS					= 0x0110
};

static CDROM_DeviceCapabilities getCapsUsingProductId(const char* prodID)
{
	char productID[1024];
	int idx = 0;

	for (int i = 0; prodID[i]; i++)
	{
		char c = prodID[i];

		if (c >= 'A' && c <= 'Z')
			productID[idx++] = c;
		else if (c >= 'a' && c <= 'z')
			productID[idx++] = c - 'a' + 'A';
	}

	productID[idx] = 0;

	int caps = CAPABILITIES_NONE;

	struct capability_item
	{
		const char* Id;
		int Capability;
	};

	static std::array<capability_item, 7> Capabilities =
	{{
		{"CD", CAPABILITIES_GENERIC_CDROM},
		{"CDRW", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_CDRW},
		{"DVD", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM},
		{"DVDRW", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_DVDRW},
		{"DVDRAM", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_DVDRW|CAPABILITIES_GENERIC_DVDRAM},
		{"BDROM", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_DVDRAM},
		{"HDDVD", CAPABILITIES_GENERIC_CDROM|CAPABILITIES_GENERIC_DVDROM|CAPABILITIES_GENERIC_HDDVD},
	}};

	std::for_each(CONST_RANGE(Capabilities, i)
	{
		if (strstr(productID, i.Id) )
			caps |= i.Capability;
	});

	return (CDROM_DeviceCapabilities)caps;
}

static void InitSCSIPassThrough(SCSI_PASS_THROUGH_WITH_BUFFERS* pSptwb)
{
	ClearStruct(*pSptwb);

	pSptwb->Spt.PathId = 0;
	pSptwb->Spt.TargetId = 1;
	pSptwb->Spt.Length = sizeof(SCSI_PASS_THROUGH);
	pSptwb->Spt.SenseInfoLength = 24;
	pSptwb->Spt.SenseInfoOffset = static_cast<ULONG>(offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf));
	pSptwb->Spt.DataTransferLength = sizeof(pSptwb->DataBuf);
	pSptwb->Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	pSptwb->Spt.DataIn = SCSI_IOCTL_DATA_IN;
	pSptwb->Spt.TimeOutValue = 2;

	ClearArray(pSptwb->DataBuf);
	ClearArray(pSptwb->Spt.Cdb);
}

static CDROM_DeviceCapabilities getCapsUsingMagic(File& Device)
{
	int caps = CAPABILITIES_NONE;

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	unsigned short sDataLength = sizeof(sptwb.DataBuf);

	InitSCSIPassThrough(&sptwb);

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
	InitSCSIPassThrough(&sptwb);

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
		unsigned char *ptr = sptwb.DataBuf;
		unsigned char *ptr_end = &sptwb.DataBuf[sDataLength];

		ptr += 8;

		while ( ptr < ptr_end )
		{
			unsigned short feature_code = (static_cast<unsigned short>(ptr[0]) << 8) | ptr[1];

			switch ( feature_code )
			{
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

	return (CDROM_DeviceCapabilities)caps;
}

static CDROM_DeviceCapabilities getCapsUsingDeviceProps(File& Device)
{
	char outBuf[1024];
	DWORD returnedLength;
	STORAGE_PROPERTY_QUERY query = {StorageDeviceProperty, PropertyStandardQuery};

	if (Device.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), outBuf, sizeof(outBuf), &returnedLength))
	{
		PSTORAGE_DEVICE_DESCRIPTOR devDesc = reinterpret_cast<PSTORAGE_DEVICE_DESCRIPTOR>(outBuf);

		if (devDesc->ProductIdOffset && outBuf[devDesc->ProductIdOffset])
		{
			char productID[1024];
			int idx = 0;

			for (DWORD i = devDesc->ProductIdOffset; outBuf[i] && i < returnedLength; i++)
			{
				productID[idx++] = outBuf[i];
			}

			productID[idx] = 0;
			return getCapsUsingProductId(productID);
		}
	}

	return CAPABILITIES_NONE;
}

static CDROM_DeviceCapabilities GetDeviceCapabilities(File& Device)
{
	CDROM_DeviceCapabilities caps = CAPABILITIES_NONE;

	caps = getCapsUsingMagic(Device);

	if ( caps == CAPABILITIES_NONE )
		caps = getCapsUsingDeviceProps(Device);

	return caps;
}

static UINT GetDeviceTypeByCaps(CDROM_DeviceCapabilities caps)
{
	struct device_caps
	{
		int Device;
		int Caps;
	};

	static std::array<device_caps, 10> DeviceCaps =
	{{
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
	}};

	auto Item = std::find_if(CONST_RANGE(DeviceCaps, i)
	{
		return (caps & i.Caps) == i.Caps;
	});
	
	return Item == DeviceCaps.cend()? DRIVE_UNKNOWN : Item->Device;
}

bool IsDriveTypeCDROM(UINT DriveType)
{
	return DriveType == DRIVE_CDROM || (DriveType >= DRIVE_CD_RW && DriveType <= DRIVE_HDDVD_RW);
}

bool DriveCanBeVirtual(UINT DriveType)
{
	return DriveType == DRIVE_FIXED || (Global->WinVer() >= 0x0602 && IsDriveTypeCDROM(DriveType));
}

UINT FAR_GetDriveType(const string& RootDir, DWORD Detect)
{
	string strRootDir(RootDir);

	if (strRootDir.IsEmpty())
	{
		string strCurDir;
		apiGetCurrentDirectory(strCurDir);
		GetPathRoot(strCurDir, strRootDir);
	}

	AddEndSlash(strRootDir);

	UINT DrvType = GetDriveType(strRootDir);

	// анализ CD-привода
	if ((Detect&1) && DrvType == DRIVE_CDROM)
	{
		string VolumePath = strRootDir;
		DeleteEndSlash(VolumePath);

		if (VolumePath.IsSubStrAt(0, L"\\\\?\\"))
			VolumePath.Replace(0, 4, L"\\\\.\\");
		else
			VolumePath.Insert(0, L"\\\\.\\");

		File Device;
		if(Device.Open(VolumePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
		{
			CDROM_DeviceCapabilities caps = GetDeviceCapabilities(Device);
			Device.Close();

			DrvType = GetDeviceTypeByCaps(caps);
		}

		if (DrvType == DRIVE_UNKNOWN) // фигня могла кака-нить произойти, посему...
			DrvType=DRIVE_CDROM;       // ...вертаем в зад сидюк.
	}

	if ( 0 != (Detect & 2) && DrvType == DRIVE_REMOVABLE )
	{
		// media have to be inserted!
		//
		string drive = HasPathPrefix(strRootDir) ? strRootDir : L"\\\\?\\" + strRootDir;
		DeleteEndSlash(drive, false);

		HANDLE hDevice = ::CreateFileW(
			drive.CPtr(),
			GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr
		);
		if ( INVALID_HANDLE_VALUE != hDevice )
		{
			DISK_GEOMETRY g;
			DWORD dwOutBytes;
			if ( DeviceIoControl(hDevice,IOCTL_DISK_GET_DRIVE_GEOMETRY,nullptr,0,&g,(DWORD)sizeof(g),&dwOutBytes,nullptr) )
				if ( g.MediaType == FixedMedia || g.MediaType == RemovableMedia )
					DrvType = DRIVE_USBDRIVE;
			CloseHandle(hDevice);
		}
	}
//	if((Detect&2) && IsDriveUsb(*LocalName,nullptr)) //DrvType == DRIVE_REMOVABLE
//		DrvType=DRIVE_USBDRIVE;
//	if((Detect&4) && GetSubstName(DrvType,LocalName,nullptr,0))
//		DrvType=DRIVE_SUBSTITUTE;

	return DrvType;
}
