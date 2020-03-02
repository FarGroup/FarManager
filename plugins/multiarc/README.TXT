Archive support plugin considers all *.fmt DLL modules in Formats
directory as second level plugins.  Examples of such plugins
are supplied here. To reduce the size of the compiled DLLs,
the examples mainly use Win32 API functions and were compiled
without the standard C startup code.

Second-level archive support plugins must export the following functions:

----------------------------------------------------------------------------
Called when second-level plugin module is loaded. This function
is optional and may be omitted.

DWORD WINAPI _export LoadFormatModule(
  const char *ModuleName
);

Parameters:
  ModuleName - name of the second-level plugin module

Return value:
  Must be 0. In future it may be used to return second-level
  plugin information.
----------------------------------------------------------------------------
Checks if the given archive file is supported by the plugin or not.

BOOL WINAPI IsArchive(
  const char *Name,
  const unsigned char *Data,
  int DataSize
);

Parameters:
  Name     - archive name
  Data     - archive data
  DataSize - archive data size

Return value:
  TRUE if this archive type is supported.

----------------------------------------------------------------------------
Open archive and prepare to read it. Called after successful IsArchive.

BOOL WINAPI OpenArchive(
  const char *Name,
  int *Type
);

Parameters:
  Name - archive name
  Type - if the plugin supports several archive types, it must put the type
         here, otherwise set *Type to 0

Return value:
  TRUE if the archive was opened successfully.

----------------------------------------------------------------------------
Get the next archive item. Called after OpenArchive.

int WINAPI GetArcItem(
  struct PluginPanelItem *Item,
  struct ArcItemInfo *Info
);

Parameters:
  Item - this structure must be filled. Read its description in plugins.hlp.
  Info - additional item info, which should be filled if possible.

    struct ArcItemInfo
    {
      char HostOS[32];       - Host OS name or empty if unknown
      char Description[256]; - Item description or empty string
      int Solid;             - "Solid" flag
      int Comment;           - Set if file comment is present
      int Encrypted;         - Set if file is encrypted
      int DictSize;          - Dictionary size or 0 if unknown
      int UnpVer;            - Version to unpack (HighNumber*256+LowNumber)
                               or 0 if unknown
      int Chapter;           - chapter item
    };

  This structure is filled with zeros before it is passed to GetArcItem.

Return value:
  GETARC_EOF         End of archive
  GETARC_SUCCESS     Item successfully read
  GETARC_BROKEN      Archive broken
  GETARC_UNEXPEOF    Unexpected end of archive
  GETARC_READERROR   Read error

----------------------------------------------------------------------------
Close the archive. Called after the last GetArcItem call.

BOOL WINAPI CloseArchive(
  struct ArcInfo *Info
);

Parameters:
  Info - additional archive info, which should be filled if possible.

    struct ArcInfo
    {
      int SFXSize;     - SFX module size
      int Volume;      - Volume flag
      int Comment;     - Archive comment present
      int Recovery;    - Recovery record present
      int Lock;        - Archive is locked
      DWORD Flags;     - Additional archive information flags
      DWORD Reserved;  - Reserved
      int Chapters;    - count chapters in archive
    };

  'Flags' field can be combination of the following values:

   AF_AVPRESENT     Authenticity information present

   AF_IGNOREERRORS  Archiver commands exit code must be ignored
                    for this archive

  This structure is filled with zeros before it is passed to CloseArchive.

Return value:
  TRUE if successful.

----------------------------------------------------------------------------
Get the archive format name.

BOOL WINAPI GetFormatName(
  int Type,
  char *FormatName,
  char *DefaultExt
);

Parameters:
  Type       - archive type
  FormatName - format name. It will be used to save parameters
               in registry and select desired format
  DefaultExt - default file extension for this format (without dot).
               Used to increase format recognizing speed.

Return value:
  TRUE if successful. If specified Type value is greater than
  the maximum supported, FALSE must be returned.

----------------------------------------------------------------------------
Get the default archiver command strings.

BOOL WINAPI GetDefaultCommands(
  int Type,
  int Command,
  char *Dest
);

Parameters:
  Type    - archive type
  Command - command number
    0 - extract
    1 - extract without path
    2 - test
    3 - delete
    4 - comment
    5 - comment files
    6 - convert to SFX
    7 - lock
    8 - add recovery record
    9 - recover
   10 - add files
   11 - move files
   12 - add files and folders
   13 - move files and folders
   14 - mask to select all files

  Dest    - buffer to copy the command

Return value:
  TRUE if successful. If specified Type value is greater than the maximum
  supported, FALSE must be returned. If Type is supported, but the required
  command is absent, return TRUE and set Dest to an empty string.


----------------------------------------------------------------------------
Send a pointer to the PluginStartupInfo structure. Called when the
second-level plugin module is loaded (called after the LoadFormatModule
function). This function is optional and may be omitted.

void  WINAPI SetFarInfo(
  const struct PluginStartupInfo *Info
);

Parameters:
  Info    - plugin startup info:

Return value:
  None.

----------------------------------------------------------------------------
Returns the start position of the archive in the stream of data passed
to the IsArchive function. The function GetSFXPos is called
immediately after the archive has been recognized by the IsArchive
function. MultiArc selects the FMT module that has returned the lowest
offset (closer to the beginning of the file). This allows to handle
correctly the situation when, for example, an ARJ archive is stored
without compression in a ZIP archive called "N3gk8v1t.106". Previous
versions of MultiArc showed the ARJ archive, and not the ZIP archive,
when entering such file.

BOOL WINAPI GetSFXPos(void);

Parameters:
  None.

Return value:
  Offset of the archive beginning from the start of the file.


----------------------------------------------------------------------------
ATTENTION! IF YOU WRITE ARCHIVE FORMAT PLUGINS, KEEP IN MIND THAT THE CURRENT
STRUCTURE ALIGNMENT MAY BE DIFFERENT FROM 1, AND IF YOU NEED TO DESCRIBE THE
STRUCTURE OF YOUR ARCHIVE FILE PRECISELY, THEN SWITCH ALIGNMENT BEFORE YOUR
STRUCTURE DEFINITION EXPLICITLY! OTHERWISE THE FIELDS IN YOUR STRUCTURE MAY
HAVE THE WRONG SIZE.

For explanations about alignment, consult your compiler manual.
----------------------------------------------------------------------------
