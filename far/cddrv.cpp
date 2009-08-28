/*
cddrv.cpp

про сидюк
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

static CDROM_DeviceCaps getCapsUsingMediaType(HANDLE hDevice)
{

    UCHAR               buffer[2048];   // Must be big enough hold DEVICE_MEDIA_INFO
    ULONG               returned;

    if(!DeviceIoControl(hDevice, IOCTL_STORAGE_GET_MEDIA_TYPES_EX, NULL, 0,
                        buffer, sizeof(buffer), &returned, FALSE))
    {
       return CDDEV_CAPS_NONE;
    }

    switch(((PGET_MEDIA_TYPES)buffer)->DeviceType)
    {
        case FILE_DEVICE_CD_ROM:
            return CDDEV_CAPS_GENERIC_CD;

        case FILE_DEVICE_DVD:
            return CDDEV_CAPS_GENERIC_DVD;

        default:
            return CDDEV_CAPS_NONE;
    }
}

static CDROM_DeviceCaps getCapsUsingProductId(const char *prodID)
{
    char productID[1024];
    int idx = 0;

    for(int i = 0; prodID[i]; i++)
    {
        char c = prodID[i];
        if(c >= 'A' && c <= 'Z')
            productID[idx++] = c;
        else if(c >= 'a' && c <= 'z')
            productID[idx++] = c - 'a' + 'A';
    }

    productID[idx] = 0;


    int caps = CDDEV_CAPS_NONE;

    if(strstr(productID, "CD"))
    {
        caps |= CDDEV_CAPS_GENERIC_CD;
    }

    if(strstr(productID, "CDRW"))
    {
        caps |= CDDEV_CAPS_GENERIC_CDRW;
    }

    if(strstr(productID, "DVD"))
    {
        caps |= CDDEV_CAPS_GENERIC_DVD;
    }

    if(strstr(productID, "DVDRW"))
    {
        caps |= CDDEV_CAPS_GENERIC_DVDRW;
    }

    if(strstr(productID, "DVDRAM"))
    {
        caps |= CDDEV_CAPS_GENERIC_DVDRAM;
    }

    return (CDROM_DeviceCaps)caps;
}


static CDROM_DeviceCaps getCapsUsingSCSIPassThrough(HANDLE hDevice)
{
		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb={0};
    sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.Spt.PathId = 0;
    sptwb.Spt.TargetId = 1;
    sptwb.Spt.Lun = 0;
    sptwb.Spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.Spt.SenseInfoLength = 24;
    sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.Spt.DataTransferLength = 192;
    sptwb.Spt.TimeOutValue = 2;
    sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf);
    sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,SenseBuf);
		ULONG length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,DataBuf) + sptwb.Spt.DataTransferLength;

    // If device supports SCSI-3, then we can get the CD drive capabilities, i.e. ability to
    // read/write to CD-ROM/R/RW or/and read/write to DVD-ROM/R/RW.
    // Use the previous spti structure, only modify the command to "mode sense"

    sptwb.Spt.Cdb[0] = SCSIOP_MODE_SENSE;
    sptwb.Spt.Cdb[1] = 0x08;                    // target shall not return any block descriptors
    sptwb.Spt.Cdb[2] = MODE_PAGE_CAPABILITIES;
    sptwb.Spt.Cdb[4] = 192;
		DWORD returned;
		BOOL status = DeviceIoControl(hDevice,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    if(status)
    {
        if(!sptwb.Spt.ScsiStatus)
        {
            // Notes:
            // 1. The header of 6-byte MODE commands is 4 bytes long.
            // 2. No Block Descriptors returned before parameter page as was specified when building the Mode command.
            // 3. First two bytes of a parameter page are the Page Code and Page Length bytes.
            // Therefore, our useful data starts at the 7th byte in the data buffer.

            int caps = CDDEV_CAPS_READ_CDROM;

            if(sptwb.DataBuf[6] & 0x01)
                caps |= CDDEV_CAPS_READ_CDR;

            if(sptwb.DataBuf[6] & 0x02)
                caps |= CDDEV_CAPS_READ_CDRW;

            if(sptwb.DataBuf[6] & 0x08)
                caps |= CDDEV_CAPS_READ_DVDROM;

            if(sptwb.DataBuf[6] & 0x10)
                caps |= CDDEV_CAPS_READ_DVDR | CDDEV_CAPS_READ_DVDRW;

            if(sptwb.DataBuf[6] & 0x20)
                caps |= CDDEV_CAPS_READ_DVDRAM;

            if(sptwb.DataBuf[7] & 0x01)
                caps |= CDDEV_CAPS_WRITE_CDR;

            if(sptwb.DataBuf[7] & 0x02)
                caps |= CDDEV_CAPS_WRITE_CDRW;

            if(sptwb.DataBuf[7] & 0x10)
                caps |= CDDEV_CAPS_WRITE_DVDR | CDDEV_CAPS_WRITE_DVDRW;

            if(sptwb.DataBuf[7] & 0x20)
                caps |= CDDEV_CAPS_WRITE_DVDRAM;

            if(caps != CDDEV_CAPS_NONE)
                return (CDROM_DeviceCaps)caps;
        }
    }

#if 0
/* $ 24.07.2004 VVM Выключим этот кусок.
    Тормозит и портит болванки при записи на SCSI/IDE писалках
*/

    sptwb.Spt.Cdb[0] = SCSIOP_INQUIRY;
    sptwb.Spt.Cdb[1] = 0;
    sptwb.Spt.Cdb[2] = 0;
    sptwb.Spt.Cdb[4] = 192;

    status = DeviceIoControl(hDevice,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    if(status)
    {
        if(!sptwb.Spt.ScsiStatus)
        {
            char productID[17];
            int idx = 0;

            for(int i = 16; i <= 31; i++)
            {
                productID[idx++] = sptwb.DataBuf[i];
            }

            productID[idx] = 0;

            return getCapsUsingProductId(productID);
        }
    }
#endif
    return CDDEV_CAPS_NONE;
}

static CDROM_DeviceCaps getCapsUsingDeviceProps(HANDLE hDevice)
{
    PSTORAGE_DEVICE_DESCRIPTOR      devDesc;
    BOOL                            status;
    char                            outBuf[512];
    DWORD                           returnedLength;
    STORAGE_PROPERTY_QUERY          query;

    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    status = DeviceIoControl(
                        hDevice,
                        IOCTL_STORAGE_QUERY_PROPERTY,
                        &query,
                        sizeof( STORAGE_PROPERTY_QUERY ),
                        &outBuf,
                        512,
                        &returnedLength,
                        NULL
                        );
    if(status)
    {
        devDesc = (PSTORAGE_DEVICE_DESCRIPTOR) outBuf;

        if(devDesc->ProductIdOffset && outBuf[devDesc->ProductIdOffset])
        {
            char productID[1024];
            int idx = 0;

            for(DWORD i = devDesc->ProductIdOffset; outBuf[i] && i < returnedLength; i++)
            {
                productID[idx++] = outBuf[i];
            }
            productID[idx] = 0;

            return getCapsUsingProductId(productID);
        }
    }

    return CDDEV_CAPS_NONE;
}


CDROM_DeviceCaps GetCDDeviceCaps(HANDLE hDevice)
{
    CDROM_DeviceCaps caps;
    if((caps = getCapsUsingSCSIPassThrough(hDevice)) !=  CDDEV_CAPS_NONE)
        return caps;

    if((caps = getCapsUsingDeviceProps(hDevice)) != CDDEV_CAPS_NONE)
        return caps;

    return getCapsUsingMediaType(hDevice);
}

UINT GetCDDeviceTypeByCaps(CDROM_DeviceCaps caps)
{
    if(caps & CDDEV_CAPS_WRITE_DVDRAM)
        return DRIVE_DVD_RAM;

    if(caps & CDDEV_CAPS_WRITE_DVDRW)
        return DRIVE_DVD_RW;

    if((caps & CDDEV_CAPS_WRITE_CDRW) && (caps & CDDEV_CAPS_READ_DVDROM))
        return DRIVE_CD_RWDVD;

    if(caps & CDDEV_CAPS_READ_DVDROM)
        return DRIVE_DVD_ROM;

    if(caps & CDDEV_CAPS_WRITE_CDRW)
        return DRIVE_CD_RW;

    if(caps & CDDEV_CAPS_READ_CDROM)
        return DRIVE_CDROM;

    return DRIVE_UNKNOWN;
}

bool IsDriveTypeCDROM(UINT DriveType)
{
  return DriveType == DRIVE_CDROM || (DriveType >= DRIVE_CD_RW && DriveType <= DRIVE_DVD_RAM);
}


UINT FAR_GetDriveType(const wchar_t *RootDir,CDROM_DeviceCaps *Caps,DWORD Detect)
{
  if(RootDir && !*RootDir)
    RootDir=NULL;

  wchar_t LocalName[4]=L" :";
  LocalName[0]=RootDir?*RootDir:0;

  CDROM_DeviceCaps caps=CDDEV_CAPS_NONE;
	UINT DrvType = GetDriveType(RootDir);

  // анализ CD-привода
  if ((Detect&1) && RootDir && IsLocalPath(RootDir) && DrvType == DRIVE_CDROM)
  {
    wchar_t szVolumeName[20]=L"\\\\.\\ :";
    szVolumeName[4]=*RootDir;

    HANDLE hDevice = apiCreateFile (szVolumeName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0);

    if (hDevice != INVALID_HANDLE_VALUE)
    {
      CDROM_DeviceCaps caps=GetCDDeviceCaps(hDevice);
      DrvType=GetCDDeviceTypeByCaps(caps);
      CloseHandle(hDevice);
    }

    if(DrvType == DRIVE_UNKNOWN) // фигня могла кака-нить произойти, посему...
      DrvType=DRIVE_CDROM;       // ...вертаем в зад сидюк.
  }

//  if((Detect&2) && IsDriveUsb(*LocalName,NULL)) //DrvType == DRIVE_REMOVABLE
//    DrvType=DRIVE_USBDRIVE;

//  if((Detect&4) && GetSubstName(DrvType,LocalName,NULL,0))
//    DrvType=DRIVE_SUBSTITUTE;

  if(Caps)
    *Caps=caps;

  return DrvType;
}
