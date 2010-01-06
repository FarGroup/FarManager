library drv_none;

// $Id: drv_none1.dpr,v 1.1.1.1 2004/05/08 10:26:54 elbereth Exp $
// $Source: /cvsroot/dragonunpacker/DragonUnPACKer/plugins/drivers/none/drv_none1.dpr,v $
//
// The contents of this file are subject to the Mozilla Public License
// Version 1.1 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
// specific language governing rights and limitations under the License.
//
// The Original Code is drv_none1.dpr, released May 8, 2004.
//
// The Initial Developer of the Original Code is Alexandre Devilliers
// (elbereth@users.sourceforge.net, http://www.elberethzone.net).
//

{ Example Driver Plugin (Delphi Source)
  Dragon UnPACKer v5.0.0                             DDK PR-2
  ===========================================================

  Most functions are preset to work easily.
  Some functions need to be implemented:
  GetDriverInfo
  ReadFormat
  IsFormat

  But you can also recode everything if you wish.

  All structures needed are in the dup5drv_utils unit.
  The included dup5drv_data unit is used by the current
  implementation of the functions, but if you want to do it
  another way you don't need this unit.

  Rename the compiled DLL to .D5D and put it in the
  Data\Drivers\ subdir of Dragon UnPACKer 5.
}

uses
  SysUtils,
  Classes,
  Windows,
  dup5drv_utils in 'dup5drv_utils.pas',
  dup5drv_data in 'dup5drv_data.pas';

{$R *.res}

var FHandle: Integer = 0;
    CurFormat: Integer = 0;
    DrvInfo: CurrentDriverInfo;
    ErrInfo: ErrorInfo;

{ ----------------------------------------------------------
    function - DUDIVersion()
  parameters - none
     returns - Byte
  ----------------------------------------------------------

  DO NOT CHANGE THIS FUNCTION.
  Dragon UnPACKer 5 won't recognize your driver if this
  function is missing or if return value is not 1.
}
function DUDIVersion: Byte; stdcall;
begin
  DUDIVersion := 1;
end;

{ ----------------------------------------------------------
    function - GetNumVersion()
  parameters - none
     returns - Integer
  ----------------------------------------------------------

  This is the numeric representation of the version of your
  driver plugin.

  It must follow the following rules:
  1) Must be 5 numbers at least (ex: 10000)
     If less than 5, it will be left padded with zeroes (ex: 50 -> 00050)
  2) There is 5 values:
     Major version (at least 1 number)
     Minor version (1 number)
     Revision (1 number)
     Type (1 number)
     Type Revision (1 number, last)

     Ex: 10000 would be translated as v1.0.0 Alpha
         50011 would be translated as v5.0.0 Beta 1

     Possible types:
     0 for Alpha
     1 for Beta
     2 for RC
     3 for Gold
     4 for Final/Retail (non-test version)
     5 for Final/Retail (non-test version)
     6 for Final/Retail (non-test version)
     7 for Fix
     8 for Patch
     9 for Special

     For all types but 4, 5 and 6 the Type Revision will be interpreted as a
     number (for ex: 29683 will be interpreted as v2.9.6 Patch 3).

     For types 4, 5 and 6 the Type Revision will be a Release letter:
     Type: 4  Type Revision: 0=No release letter
                             1=A  2=B  3=C  4=D  5=E  6=F  7=G  8=H  9=I
     Type: 5  Type Revision: 0=J  1=K  2=L  3=M  4=N  5=O  6=P  7=Q  8=R  9=S
     Type: 6  Type Revision: 0=T  1=U  2=V  3=W  4=X  5=Y  6=Z
                             7=+  8=++ 9=+++

     Ex: 50042 = v5.0.0 Release B (or v5.0.0b)

     For every new release you must use a higher integer number.
     Ex: 1st release 10040
         2nd release must be higher than 10040 (ex: 10041 or 10140)
     This is to allow version management (ask the user if he really wants to
     install an older driver for example).
}
function GetNumVersion: Integer; stdcall;
begin

  GetNumVersion := 10000;

end;

{ ----------------------------------------------------------
    function - GetDriverInfo()
  parameters - none
     returns - DriverInfo record
  ----------------------------------------------------------

  This function is called by DUP5 for file associations and
  for Open dialog box file types.
}
function GetDriverInfo: DriverInfo; stdcall;
begin

  GetDriverInfo.Name := 'My Game MYID Driver';
  GetDriverInfo.Author := 'My Name';
  GetDriverInfo.Version := '1.0.0';
  GetDriverInfo.Comment := 'My Comment';
  // Number of formats defined in Formats array (see below)
  GetDriverInfo.NumFormats := 1;
  GetDriverInfo.Formats[1].Extensions := '*.myext';
  GetDriverInfo.Formats[1].Name := 'My Game (*.MYEXT)';
//Example of multiple games with same extension:
//GetDriverInfo.Formats[2].Extensions := '*.myext';
//GetDriverInfo.Formats[2].Name := 'My Game (*.MYEXT)|My 2nd Game (*.MYEXT)|My 3rd Game (*.MYEXT)';

end;

{ ----------------------------------------------------------
    function - ExtractFile()                     Facultative
  parameters - OutPutFile: ShortString
               EntryNam: ShortString
               Offset: Int64
               Size: Int64
               DataX: Integer
               DataY: Integer
     returns - Boolean
  ----------------------------------------------------------

  This function is called by DUP5 to extract files only if
  ExtractInternal is set to TRUE in CurrentDriverInfo.

  Use this function for extraction of crypted/compressed entries or entries
  that cannot be extracted only by copying from a start offset with a given
  size.

  NOTE: Include only if needed.
}
{function ExtractFile(outputfile: ShortString; entrynam: ShortString; Offset: Int64; Size: Int64; DataX: integer; DataY: integer): boolean; stdcall;
var Buf: PByteArray;
    BufEnd: PByteArray;
    fil: Integer;
begin

  fil := FileCreate(outputfile,fmOpenRead or fmShareExclusive);

  FileSeek(FHandle,Offset,0);

  GetMem(Buf,DataX);
  FileRead(FHandle,Buf^,DataX);
  FileWrite(fil,Buf^,DataX);

  FreeMem(Buf);
  FileClose(fil);

  ExtractFile := true;

end;}

{ ----------------------------------------------------------
    function - IsFormat()
  parameters - fil: ShortString
               Deeper: Boolean
     returns - Boolean
  ----------------------------------------------------------

  This function is called by DUP5 to know if your driver can
  open the file stored in Fil variable (full path to file).
  The Deeper "switch" is here to indicate if you only need
  to test by extension (deeper=false) or open file to check
  header (deeper=true).
}
function IsFormat(fil: ShortString; Deeper: Boolean): Boolean; stdcall;
var ext: string;
begin

  // Put file extension uppercase form in ext var
  ext := ExtractFileExt(fil);
  if ext <> '' then
    ext := copy(ext,2,length(ext)-1);
  ext := UpperCase(ext);

  if ext = 'MYEXT' then
    IsFormat := True
  else
    IsFormat := False;

end;

{ ----------------------------------------------------------
    function - CloseFormat()
  parameters - none
     returns - nothing
  ----------------------------------------------------------

  This function is called by DUP5 to close an opened file.
  You should free all allocated stuff in ReadFile()
  function. And your driver must be able to handle a new
  call to ReadFile() just after a call to this function.
}
procedure CloseFormat; stdcall;
begin

  // This is an example
  // Change it the way you need
  DrvInfo.Sch := '';
  DrvInfo.ID := '';
  DrvInfo.FileHandle := 0;

  if FHandle <> 0 then
    FileClose(FHandle);

  FHandle := 0;

end;

{ ----------------------------------------------------------
    function - GetCurrentDriverInfo()
  parameters - none
     returns - CurrentDriverInfo record
  ----------------------------------------------------------

  This function is called by DUP5 to get current file format
  information (directory handling, file handle, internal
  extraction).
  It is called just after ReadFile().

  NOTE: You don't need to change this function if you want
        to use the same driver model as this example.
}
function GetCurrentDriverInfo(): CurrentDriverInfo; stdcall;
begin

  GetCurrentDriverInfo := DrvInfo;

end;

{ ----------------------------------------------------------
    function - GetEntry()
  parameters - none
     returns - FormatEntry record
  ----------------------------------------------------------

  This function is called by DUP5 to retrieve entries of the
  file parsed by ReadFile() function.
  It is called just after ReadFile(). It is called N times
  (with N equal to the value returned by ReadFile).

  NOTE: You don't need to change this function if you want
        to use the same driver model as this example.
}
function GetEntry(): FormatEntry; stdcall;
begin

  GetEntry := FSE_Read;

end;

{ ----------------------------------------------------------
    function - GetErrorInfo()
  parameters - none
     returns - ErrorInfo record
  ----------------------------------------------------------

  This function is called by DUP5 when ReadFile() function
  returns an integer value of -3 (returns format info and
  game info).

  NOTE: You don't need to change this function if you want
        to use the same driver model as this example.
}
function GetErrorInfo(): ErrorInfo; stdcall;
begin

  GetErrorInfo := ErrInfo;

end;

{ ----------------------------------------------------------
    function - ReadFormat()
  parameters - fil: ShortString
               percent: TPercentCallback
               Deeper: boolean
     returns - Integer value
  ----------------------------------------------------------

  This function opens file pointed by fil var and if:
  1) The file is good format:
     File is parsed and entries are stored (for future call
     by DUP5 to GetEntry() function).
     Integer value will have the total number of stored
     entries.
     DriverInfo must be setup for future call to
     GetCurrentDriverInfo() function by DUP5.
     CurrentDriverInfo structure:
       ID     - File ID (Appears on the status of DUP5)
       Sch    - Char used as separator for directories (\ or /)
                If left blank, then no directory parsing will be done.
       FileHandle - File Handle (Integer);
       ExtractInternal - If your Driver include an ExtractFile
                         function set this to TRUE
                         else set it to FALSE (DUP5 will handle
                         the file extraction).
  2) The file could not be opened:
     Integer value is set to -2.
  3) The file does not have expected format:
     Integer value is set to -3.
     Error information should be stored (for future call by
     DUP5 to GetErrorInfo() function).
  4) The file extension is unrecognized:
     Integer value is set to -1.
     (This should normally never happen if IsFormat()
     function is well coded).

 Deeper switch have same function than in isFormat().
 percent is a procedure of TPercentCallback type (only 1 Byte parameter).
         It allows your plugin to use the percent bar of DUP5.
         Values can be from 0 to 100.

 IMPORTANT:
 You must not close the file handle because DUP5 will use
 it for Extraction if your driver does not have ExtractFile
 function.
 File handle is an integer, meaning you must use FileOpen,
 FileClose, FileRead, FileWrite, etc..
 If you don't want to use those functions you must include
 an ExtractFile function to your driver.
 ----------------------------------------------------------
}
function ReadFormat(fil: ShortString; percent: TPercentCallback; Deeper: boolean): Integer; stdcall;
var ext: string;
begin

  // Put file extension in uppercase form in ext var
  ext := ExtractFileExt(fil);
  if ext <> '' then
    ext := copy(ext,2,length(ext)-1);
  ext := UpperCase(ext);

  if ext = 'MY_EXT' then
  begin
    { Here you add code that will read the file and store
      entries somewhere (if you use dup5drv_data do as
      follow).

      Use FSE_Add to store every entry.

      FSE_Add arguments:
        Name = Name of entry
      Offset = Offset of entry in file (first offset is 0)
        Size = Size of entry in file (in bytes)
       DataX = Any 32bit signed value you wish to store
       DataY = Any 32bit signed value you wish to store

      You must also setup the CurrentDriverInfo:
      DrvInfo.ID := 'MYID';
      DrvInfo.Sch := '';
      DrvInfo.FileHandle := FileHandle;
      DrvInfo.ExtractInternal := False;

      If the file is not of the expected format you must
      set the return value to -3 and store info needed by
      GetErrorInfo(). If you use the predefined driver
      structure do as follow:

      ErrInfo.Format := 'My Format';
      ErrInfo.Games := 'My Game';

      To use the percent bar:
      percent(value);
      Ex: percent(50); will set the percent bar to 50%
    }
  end
  else
    ReadFormat := -1;  // Unknown extension, return errorcode

end;

{ ----------------------------------------------------------
    function - AboutBox()
  parameters - hwnd: Integer
               DLNGstr: TLanguageCallback
     returns - Integer value
  ----------------------------------------------------------

  This is called by DUP5 when a user press the About button
  in the driver list while your driver is selected.

  hwnd is the DUP5 windows handle
  DLNGstr is the Language string callback (to get translated
  strings).
}
procedure AboutBox(hwnd: Integer; DLNGstr: TLanguageCallBack); stdcall;
begin

  MessageBoxA(hwnd, PChar('That is an About BOX!'+#10+
                          'Wahou!')
                        , 'About My Driver...', MB_OK);

end;


// Exported functions and procedures
exports
  CloseFormat,
  DUDIVersion,
//ExtractFile,   // Export only if your program uses internal
                 // extraction.
  GetCurrentDriverInfo,
  GetDriverInfo,
  GetEntry,
  GetErrorInfo,
  GetNumVersion,
  IsFormat,
  AboutBox,      // If you don't want to use about box then comment out this
                 // line
  ReadFormat;

begin
end.
