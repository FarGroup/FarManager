#pragma once

/*
drivemix.hpp

Misc functions for drive/disk info
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

enum
{
	// DRIVE_UNKNOWN            = 0,
	// DRIVE_NO_ROOT_DIR        = 1,
	// DRIVE_REMOVABLE          = 2,
	// DRIVE_FIXED              = 3,
	// DRIVE_REMOTE             = 4,
	// DRIVE_CDROM              = 5,
	// DRIVE_RAMDISK            = 6,

	DRIVE_SUBSTITUTE            =15,
	DRIVE_REMOTE_NOT_CONNECTED  =16,
	DRIVE_CD_RW                 =18,
	DRIVE_CD_RWDVD              =19,
	DRIVE_DVD_ROM               =20,
	DRIVE_DVD_RW                =21,
	DRIVE_DVD_RAM               =22,
	DRIVE_BD_ROM                =23,
	DRIVE_BD_RW                 =24,
	DRIVE_HDDVD_ROM             =25,
	DRIVE_HDDVD_RW              =26,
	DRIVE_USBDRIVE              =40,
	DRIVE_VIRTUAL               =41,
	DRIVE_NOT_INIT              =255,
};

enum CHECKEDPROPS_TYPE
{
	CHECKEDPROPS_ISSAMEDISK,
	CHECKEDPROPS_ISDST_ENCRYPTION,
};

bool IsDriveTypeRemote(UINT DriveType);
DWORD FarGetLogicalDrives();
int CheckDisksProps(const string& SrcPath, const string&DestPath, int CheckedType);
