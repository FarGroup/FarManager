#ifndef __FARCONST_HPP__
#define __FARCONST_HPP__
/*
farconst.hpp

содержит все enum, #define, etc
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

// типы рамок
enum {
  NO_BOX,
  SINGLE_BOX,
  SHORT_SINGLE_BOX,
  DOUBLE_BOX,
  SHORT_DOUBLE_BOX
};

enum {
  MSG_WARNING        =0x00000001,
  MSG_ERRORTYPE      =0x00000002,
  MSG_KEEPBACKGROUND =0x00000004,
  MSG_DOWN           =0x00000008,
  MSG_LEFTALIGN      =0x00000010,
  MSG_KILLSAVESCREEN =0x00000020,
};

// Работа с ассоциациями файлов
enum {
  FILETYPE_EXEC,       // Enter
  FILETYPE_VIEW,       // F3
  FILETYPE_EDIT,       // F4
  FILETYPE_ALTEXEC,    // Ctrl-PgDn
  FILETYPE_ALTVIEW,    // Alt-F3
  FILETYPE_ALTEDIT     // Alt-F4
};

enum DIZUPDATETYPE {
  DIZ_NOT_UPDATE,
  DIZ_UPDATE_IF_DISPLAYED,
  DIZ_UPDATE_ALWAYS
};

#define MAX_MSG 5000

enum {
  COLUMN_MARK           = 0x80000000,
  COLUMN_NAMEONLY       = 0x40000000,
  COLUMN_RIGHTALIGN     = 0x20000000,
  COLUMN_FORMATTED      = 0x10000000,
  COLUMN_COMMAS         = 0x08000000,
  COLUMN_THOUSAND       = 0x04000000,
  COLUMN_BRIEF          = 0x02000000,
  COLUMN_MONTH          = 0x01000000,
  COLUMN_FLOATSIZE      = 0x00800000,
  COLUMN_ECONOMIC       = 0x00400000,
  COLUMN_MINSIZEINDEX   = 0x00200000,
  COLUMN_SHOWBYTESINDEX = 0x00100000,
  COLUMN_FULLOWNER      = 0x00080000,

  //MINSIZEINDEX может быть только 0, 1, 2 или 3 (K,M,G,T)
  COLUMN_MINSIZEINDEX_MASK = 0x00000003,
};

//  +CASR_* Поведение Ctrl-Alt-Shift для AllCtrlAltShiftRule
enum {
  CASR_PANEL  = 0x0001,
  CASR_EDITOR = 0x0002,
  CASR_VIEWER = 0x0004,
  CASR_HELP   = 0x0008,
  CASR_DIALOG = 0x0010,
};

enum {
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
  DRIVE_USBDRIVE              =40,
  DRIVE_NOT_INIT              =255,
};

// для диалога GetNameAndPassword()
enum FlagsNameAndPassword{
  GNP_USELAST      = 0x00000001UL, // использовать последние введенные данные
};

enum {
    XC_QUIT                = (unsigned long) -777,
    XC_OPEN_ERROR          = 0,
    XC_MODIFIED            = 1,
    XC_NOT_MODIFIED        = 2,
    XC_LOADING_INTERRUPTED = 3,
    XC_EXISTS              = 4,
};

#define ADDSPACEFORPSTRFORMESSAGE 16

enum CHECKFOLDERCONST{ // for CheckFolder()
  CHKFLD_ERROR     = -2,
  CHKFLD_NOTACCESS = -1,
  CHKFLD_EMPTY     =  0,
  CHKFLD_NOTEMPTY  =  1,
  CHKFLD_NOTFOUND  =  2,
};

// для Opt.QuotedName
enum QUOTEDNAMETYPE{
  QUOTEDNAME_INSERT         = 0x00000001,            // кавычить при сбросе в командную строку, в диалогах и редакторе
  QUOTEDNAME_CLIPBOARD      = 0x00000002,            // кавычить при помещении в буфер обмена
};

enum ExcludeCmdHistoryType{
  EXCLUDECMDHISTORY_NOTWINASS    = 0x00000001,  // не помещать в историю команды ассоциаций Windows
  EXCLUDECMDHISTORY_NOTFARASS    = 0x00000002,  // не помещать в историю команды выполнения ассоциаций файлов
  EXCLUDECMDHISTORY_NOTPANEL     = 0x00000004,  // не помещать в историю команды выполнения с панели
  EXCLUDECMDHISTORY_NOTCMDLINE   = 0x00000008,  // не помещать в историю команды выполнения с ком.строки
};

enum GETDIRINFOFLAGS{
  GETDIRINFO_ENHBREAK           =0x00000001,
  GETDIRINFO_DONTREDRAWFRAME    =0x00000002,
  GETDIRINFO_SCANSYMLINK        =0x00000004,
  GETDIRINFO_SCANSYMLINKDEF     =0x00000008,
  GETDIRINFO_USEFILTER          =0x00000010,
};

enum CHECKEDPROPS_TYPE{
  CHECKEDPROPS_ISSAMEDISK,
  CHECKEDPROPS_ISDST_ENCRYPTION,
};

enum SETATTR_RET_CODES
{
  SETATTR_RET_ERROR,
  SETATTR_RET_OK,
  SETATTR_RET_SKIP,
  SETATTR_RET_SKIPALL,
};

#define DMOUSEBUTTON_LEFT   0x00000001
#define DMOUSEBUTTON_RIGHT  0x00000002

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8       0xBFBBEF

enum BOX_DEF_SYMBOLS
{
	BS_X_B0,          // 0xB0
	BS_X_B1,          // 0xB1
	BS_X_B2,          // 0xB2
	BS_V1,            // 0xB3
	BS_R_H1V1,        // 0xB4
	BS_R_H2V1,        // 0xB5
	BS_R_H1V2,        // 0xB6
	BS_RT_H1V2,       // 0xB7
	BS_RT_H2V1,       // 0xB8
	BS_R_H2V2,        // 0xB9
	BS_V2,            // 0xBA
	BS_RT_H2V2,       // 0xBB
	BS_RB_H2V2,       // 0xBC
	BS_RB_H1V2,       // 0xBD
	BS_RB_H2V1,       // 0xBE
	BS_RT_H1V1,       // 0xBF
	BS_LB_H1V1,       // 0xС0
	BS_B_H1V1,        // 0xС1
	BS_T_H1V1,        // 0xС2
	BS_L_H1V1,        // 0xС3
	BS_H1,            // 0xС4
	BS_C_H1V1,        // 0xС5
	BS_L_H2V1,        // 0xС6
	BS_L_H1V2,        // 0xС7
	BS_LB_H2V2,       // 0xС8
	BS_LT_H2V2,       // 0xС9
	BS_B_H2V2,        // 0xСA
	BS_T_H2V2,        // 0xСB
	BS_L_H2V2,        // 0xСC
	BS_H2,            // 0xСD
	BS_C_H2V2,        // 0xСE
	BS_B_H2V1,        // 0xСF
	BS_B_H1V2,        // 0xD0
	BS_T_H2V1,        // 0xD1
	BS_T_H1V2,        // 0xD2
	BS_LB_H1V2,       // 0xD3
	BS_LB_H2V1,       // 0xD4
	BS_LT_H2V1,       // 0xD5
	BS_LT_H1V2,       // 0xD6
	BS_C_H1V2,        // 0xD7
	BS_C_H2V1,        // 0xD8
	BS_RB_H1V1,       // 0xD9
	BS_LT_H1V1,       // 0xDA
	BS_X_DB,          // 0xDB
	BS_X_DC,          // 0xDC
	BS_X_DD,          // 0xDD
	BS_X_DE,          // 0xDE
	BS_X_DF,          // 0xDF
};

enum SetCPFlags
{
	SETCP_NOERROR    = 0x00000000,
	SETCP_WC2MBERROR = 0x00000001,
	SETCP_MB2WCERROR = 0x00000002,
	SETCP_OTHERERROR = 0x10000000,
};

#endif // __FARCONST_HPP__
