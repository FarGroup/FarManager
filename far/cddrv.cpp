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

	if ( strstr(productID, "CD") )
		caps |= CAPABILITIES_GENERIC_CDROM;

	if ( strstr(productID, "CDRW") )
		caps |= (CAPABILITIES_GENERIC_CDROM | CAPABILITIES_GENERIC_CDRW);

	if ( strstr(productID, "DVD") )
		caps |= (CAPABILITIES_GENERIC_CDROM | CAPABILITIES_GENERIC_DVDROM);

	if ( strstr(productID, "DVDRW") )
		caps |= (CAPABILITIES_GENERIC_CDROM | CAPABILITIES_GENERIC_DVDROM | CAPABILITIES_GENERIC_DVDRW);

	if ( strstr(productID, "DVDRAM") )
		caps |= (CAPABILITIES_GENERIC_CDROM | CAPABILITIES_GENERIC_DVDROM | CAPABILITIES_GENERIC_DVDRW | CAPABILITIES_GENERIC_DVDRAM);

	if ( strstr(productID, "BDROM") )
		caps |= (CAPABILITIES_GENERIC_CDROM | CAPABILITIES_GENERIC_DVDROM | CAPABILITIES_GENERIC_BDROM);

	if ( strstr(productID, "HDDVD") )
		caps |= (CAPABILITIES_GENERIC_CDROM | CAPABILITIES_GENERIC_DVDROM | CAPABILITIES_GENERIC_HDDVD);

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
	PSTORAGE_DEVICE_DESCRIPTOR      devDesc;
	BOOL                            status;
	char                            outBuf[1024];
	DWORD                           returnedLength;
	STORAGE_PROPERTY_QUERY          query;
	query.PropertyId = StorageDeviceProperty;
	query.QueryType = PropertyStandardQuery;
	status = Device.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), &outBuf, sizeof(outBuf), &returnedLength);

	if (status)
	{
		devDesc = (PSTORAGE_DEVICE_DESCRIPTOR) outBuf;

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

CDROM_DeviceCapabilities GetDeviceCapabilities(File& Device)
{
	CDROM_DeviceCapabilities caps = CAPABILITIES_NONE;

	caps = getCapsUsingMagic(Device);

	if ( caps == CAPABILITIES_NONE )
		caps = getCapsUsingDeviceProps(Device);

	return caps;
}

UINT GetDeviceTypeByCaps(CDROM_DeviceCapabilities caps)
{
	if ( caps & CAPABILITIES_GENERIC_BDRW )
		return DRIVE_BD_RW;

	if ( caps & CAPABILITIES_GENERIC_BDROM )
		return DRIVE_BD_ROM;

	if ( caps & CAPABILITIES_GENERIC_HDDVDRW )
		return DRIVE_HDDVD_RW;

	if ( caps & CAPABILITIES_GENERIC_HDDVD )
		return DRIVE_HDDVD_ROM;

	if (caps & CAPABILITIES_GENERIC_DVDRAM )
		return DRIVE_DVD_RAM;

	if (caps & CAPABILITIES_GENERIC_DVDRW)
		return DRIVE_DVD_RW;

	if ((caps & CAPABILITIES_GENERIC_CDRW) && (caps & CAPABILITIES_GENERIC_DVDROM))
		return DRIVE_CD_RWDVD;

	if (caps & CAPABILITIES_GENERIC_DVDROM)
		return DRIVE_DVD_ROM;

	if (caps & CAPABILITIES_GENERIC_CDRW)
		return DRIVE_CD_RW;

	if (caps & CAPABILITIES_GENERIC_CDROM)
		return DRIVE_CDROM;

	return DRIVE_UNKNOWN;
}

bool IsDriveTypeCDROM(UINT DriveType)
{
	return DriveType == DRIVE_CDROM || (DriveType >= DRIVE_CD_RW && DriveType <= DRIVE_HDDVD_RW);
}

bool DriveCanBeVirtual(UINT DriveType)
{
	return DriveType == DRIVE_FIXED || (WinVer >= 0x0602 && IsDriveTypeCDROM(DriveType));
}

UINT FAR_GetDriveType(const wchar_t *RootDir, CDROM_DeviceCapabilities *Caps, DWORD Detect)
{
	string strRootDir;

	if (!RootDir || !*RootDir)
	{
		string strCurDir;
		apiGetCurrentDirectory(strCurDir);
		GetPathRoot(strCurDir, strRootDir);
	}
	else
	{
		strRootDir = RootDir;
	}

	AddEndSlash(strRootDir);

	CDROM_DeviceCapabilities caps = CAPABILITIES_NONE;

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
			caps = GetDeviceCapabilities(Device);
			Device.Close();

			DrvType = GetDeviceTypeByCaps(caps);
		}

		if (DrvType == DRIVE_UNKNOWN) // фигня могла кака-нить произойти, посему...
			DrvType=DRIVE_CDROM;       // ...вертаем в зад сидюк.
	}
	if (Caps)
		*Caps=caps;

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
			if ( DeviceIoControl(hDevice,IOCTL_DISK_GET_DRIVE_GEOMETRY,nullptr,0,&g,(DWORD)sizeof(g),&dwOutBytes,NULL) )
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
