unit PluginSystem;

interface

uses
  // If you want to make it a little smaller and are not supporting IStreams
  // then remove (comment out) the ActiveX unit, and all functions that
  // use IStream.
  ActiveX,
  Common;

function mpGetInterfaceVersion(var Major, Minor: LongWord): LongBool; stdcall;
function mpGetPluginInfo(var Info: TPluginInfo): LongBool; stdcall;
function mpGetFormatCount(): LongWord; stdcall;
function mpGetFormatInfo(FormatIndex: LongInt; var FormatInfo: TFormatInfo): LongBool; stdcall;
function mpGetOptions(FormatIndex: LongInt; OptionType: LongInt): PChar; stdcall;
function mpSetOption(FormatIndex: LongInt; OptionType: LongInt; Name: PChar; Value: PChar): LongBool; stdcall;
function mpOpenArchive(var ArchiveHandle: LongInt; FormatIndex: LongInt; ArchiveName: PChar; Flags: Cardinal): LongBool; stdcall;
function mpOpenArchiveBindStream(var ArchiveHandle: LongInt; FormatIndex: LongInt; Stream: IStream; Flags: Cardinal): LongBool; stdcall;
function mpCloseArchive(ArchiveHandle: LongInt): LongBool; stdcall;
function mpIndexCount(ArchiveHandle: LongInt): LongInt; stdcall;
function mpIndexedInfo(ArchiveHandle: LongInt; Index: LongInt; Item: PChar): PChar; stdcall;
function mpFindInfo(Handle: LongInt; Field: PChar): PChar; stdcall;
function mpFindFirstFile(ArchiveHandle: LongInt; FileMask: PChar): LongInt; stdcall;
function mpFindNextFile(FindHandle: LongInt): LongBool; stdcall;
function mpFindClose(FindHandle: LongInt): LongBool; stdcall;
function mpIsFileAnArchive(FormatIndex: LongInt; Filename: PChar): LongBool; stdcall;
function mpIsStreamAnArchive(FormatIndex: LongInt; Stream: IStream): LongBool; stdcall;
function mpExportFileByNameToFile(ArchiveHandle: LongInt; ArchiveFile: PChar; ExternalFile: PChar): LongBool; stdcall;
function mpExportFileByIndexToFile(ArchiveHandle: LongInt; FileIndex: LongInt; ExternalFile: PChar): LongBool; stdcall;
function mpExportFileByNameToStream(ArchiveHandle: LongInt; ArchiveFile: PChar; Stream: IStream): LongBool; stdcall;
function mpExportFileByIndexToStream(ArchiveHandle: LongInt; FileIndex: LongInt; Stream: IStream): LongBool; stdcall;
function mpImportFileFromFile(ArchiveHandle: LongInt; ArchiveFile: PChar; ExternalFile: PChar): LongBool; stdcall;
function mpImportFileFromStream(ArchiveHandle: LongInt; ArchiveFile: PChar; Stream: IStream): LongBool; stdcall;
function mpRemoveFileByName(ArchiveHandle: LongInt; Filename: PChar): LongBool; stdcall;
function mpRemoveFileByIndex(ArchiveHandle: LongInt; FileIndex: LongInt): LongBool; stdcall;
function mpGetLastError(): Cardinal; stdcall;
function mpGetErrorText(Value: Cardinal): PChar; stdcall;

implementation

var
  LastError: Cardinal;

function mpGetInterfaceVersion(var Major, Minor: LongWord): LongBool;
begin
  // Interface version (used for when the interface upgrades to add new features)
  //   right now, only 1.0 is supported
  Major := InterfaceVersionMajor;
  Minor := InterfaceVersionMinor;
  Result := True;
end;

function mpGetPluginInfo(var Info: TPluginInfo): LongBool;
begin
  Result := False;
  if Info.Size = SizeOf(TPluginInfo) then
    with Info do
      begin
      Name := 'Plugin Name';
      Author := 'Author Name';
      URL := 'http://URL/to/Plugin/site/';
      Email := 'email address of author';
      Major := 1;
      Minor := 0; // Plugin internal version (not used except for plugin upgrade paths)
      Result := True;
      end
  else
    LastError := pERROR_INVALID_PARM_1;
end;

function mpGetFormatCount(): LongWord;
begin
  Result := 0;
end;

function mpGetFormatInfo(FormatIndex: LongInt; var FormatInfo: TFormatInfo): LongBool;
begin
  Result := False;
  LastError := pERROR_INVALID_FORMAT;
end;

function mpGetOptions(FormatIndex: LongInt; OptionType: LongInt): PChar;
begin
  Result := '';
  LastError := pERROR_UNSUPPORTED;
end;

function mpSetOption(FormatIndex: LongInt; OptionType: LongInt; Name: PChar; Value: PChar): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpOpenArchive(var ArchiveHandle: LongInt; FormatIndex: LongInt; ArchiveName: PChar; Flags: Cardinal): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpOpenArchiveBindStream(var ArchiveHandle: LongInt; FormatIndex: LongInt; Stream: IStream; Flags: Cardinal): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpCloseArchive(ArchiveHandle: LongInt): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpIndexCount(ArchiveHandle: LongInt): LongInt;
begin
  Result := 0;
  LastError := pERROR_UNSUPPORTED;
end;

function mpIndexedInfo(ArchiveHandle: LongInt; Index: LongInt; Item: PChar): PChar;
begin
  Result := '';
  LastError := pERROR_UNSUPPORTED;
end;

function mpFindInfo(Handle: LongInt; Field: PChar): PChar;
begin
  Result := '';
  LastError := pERROR_UNSUPPORTED;
end;

function mpFindFirstFile(ArchiveHandle: LongInt; FileMask: PChar): LongInt;
begin
  Result := -1;
  LastError := pERROR_UNSUPPORTED;
end;

function mpFindNextFile(FindHandle: LongInt): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpFindClose(FindHandle: LongInt): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpIsFileAnArchive(FormatIndex: LongInt; Filename: PChar): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpIsStreamAnArchive(FormatIndex: LongInt; Stream: IStream): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpExportFileByNameToFile(ArchiveHandle: LongInt; ArchiveFile: PChar; ExternalFile: PChar): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpExportFileByIndexToFile(ArchiveHandle: LongInt; FileIndex: LongInt; ExternalFile: PChar): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpExportFileByNameToStream(ArchiveHandle: LongInt; ArchiveFile: PChar; Stream: IStream): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpExportFileByIndexToStream(ArchiveHandle: LongInt; FileIndex: LongInt; Stream: IStream): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpImportFileFromFile(ArchiveHandle: LongInt; ArchiveFile: PChar; ExternalFile: PChar): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpImportFileFromStream(ArchiveHandle: LongInt; ArchiveFile: PChar; Stream: IStream): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpRemoveFileByName(ArchiveHandle: LongInt; Filename: PChar): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpRemoveFileByIndex(ArchiveHandle: LongInt; FileIndex: LongInt): LongBool;
begin
  Result := False;
  LastError := pERROR_UNSUPPORTED;
end;

function mpGetLastError(): Cardinal;
begin
  Result := LastError;
end;                      

function mpGetErrorText(Value: Cardinal): PChar;
begin
  // Plugin Specific Error Messages
  Result := '';
end;

end.
