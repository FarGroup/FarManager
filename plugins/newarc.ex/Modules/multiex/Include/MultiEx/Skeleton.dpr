library Skeleton;

{$EXTENSION mxp}

uses
  PluginSystem;

exports
  mpGetInterfaceVersion,
  mpGetPluginInfo,
  mpGetFormatCount,
  mpGetFormatInfo,
  mpGetOptions,
  mpSetOption,
  mpOpenArchive,
  mpOpenArchiveBindStream,
  mpCloseArchive,
  mpIndexCount,
  mpIndexedInfo,
  mpFindInfo,
  mpFindFirstFile,
  mpFindNextFile,
  mpFindClose,
  mpIsFileAnArchive,
  mpIsStreamAnArchive,
  mpExportFileByNameToFile,
  mpExportFileByIndexToFile,
  mpExportFileByNameToStream,
  mpExportFileByIndexToStream,
  mpImportFileFromFile,
  mpImportFileFromStream,
  mpRemoveFileByName,
  mpRemoveFileByIndex,
  mpGetLastError,
  mpGetErrorText;

begin
end.
 