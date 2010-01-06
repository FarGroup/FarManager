unit Common;

interface

uses
  DateUtils;

type
  TManagerPluginInfo = packed record
    // Version 1.0
    Size: LongInt;
    PluginName: PChar;
    PluginAuthor: PChar;
    PluginURL: PChar;
    PluginEmail: PChar;
    PluginFullPath: PChar;
    VersionMajor, VersionMinor: LongInt;
    InterfaceMajor, InterfaceMinor: LongInt;
    Supported: LongBool;
  end;

  TPluginInfo = packed record
    // Version 1.0
    Size: LongInt;
    Name: PChar;
    Author: PChar;
    URL: PChar;
    Email: PChar;
    Major: LongInt;
    Minor: LongInt;
  end;

  TFormatInfo = packed record
    // Version 1.0
    Size: LongInt;
    FileMask: PChar; // This is better than an Interface
    GameName: PChar; // This is the name of the game it supports
    Flags: Int64;    // Support flags
  end;

const
  SUPPORTFLAG_CREATE             = $0000000000000001;
  SUPPORTFLAG_IMPORT             = $0000000000000002;
  SUPPORTFLAG_EXPORT             = $0000000000000004;
  SUPPORTFLAG_DELETE             = $0000000000000008;
  SUPPORTFLAG_REPLACE            = $0000000000000010;
  SUPPORTFLAG_BYINDEX            = $0000000000000020;
  SUPPORTFLAG_BYNAME             = $0000000000000040;
  SUPPORTFLAG_HANDLEISTREAM      = $0000000000000080;
  SUPPORTFLAG_HANDLEFILE         = $0000000000000100;
  SUPPORTFLAG_TESTARCHIVE        = $0000000000000200;
  SUPPORTFLAG_EXPORTNAMEWILD     = $0000000000000400;
  SUPPORTFLAG_IMPORTNAMEWILD     = $0000000000000800;

  OPTIONTYPE_CREATE              = $00000001;
  OPTIONTYPE_EXPORT              = $00000002;
  OPTIONTYPE_IMPORT              = $00000003;
  OPTIONTYPE_FILEINFO            = $00000004;
  OPTIONTYPE_INVALIDFILENAMECHAR = $00000005;

  OPENFLAG_CREATENEW             = $00000001;
  OPENFLAG_OPENALWAYS            = $00000002;
  OPENFLAG_FOREXPORT             = $00000004;
  OPENFLAG_FORIMPORT             = $00000008;

  pERROR_OK                      = $00000000;
  pERROR_INVALID_PARM_1          = $00000001;
  pERROR_INVALID_PARM_2          = $00000002;
  pERROR_INVALID_PARM_3          = $00000003;
  pERROR_INVALID_PARM_4          = $00000004;
  pERROR_INVALID_PARM_5          = $00000005;
  pERROR_INVALID_PARM_6          = $00000006;
  pERROR_INVALID_PARM_7          = $00000007;
  pERROR_INVALID_PARM_8          = $00000008;
  pERROR_INVALID_PARM_9          = $00000009;
  pERROR_FILE_NOT_EXISTS         = $0000000A;
  pERROR_FILE_CANT_OPEN          = $0000000B;
  pERROR_INVALID_FORMAT          = $0000000C;
  pERROR_STREAM_READ             = $0000000D;
  pERROR_INVALID_HANDLE          = $0000000E;
  pERROR_INVALID_INDEX           = $0000000F;
  pERROR_CREATE_ERROR            = $00000010;
  pERROR_ARCHIVE_READ_ERROR      = $00000011;
  pERROR_NO_MATCHES              = $00000012;
  pERROR_ARCHIVE_CLOSED          = $00000013;
  pERROR_INVALID_OPTION          = $00000014;
  pERROR_WILDCARDS_NOT_ALLOWED   = $00000015;
  pERROR_INVALID_ARCHIVE         = $00000016;
  pERROR_FILE_EXISTS             = $00000017;
  pERROR_UNSUPPORTED             = $00000018;

  pERROR_SPECIFICPLUGIN          = $80000000;

  InterfaceVersionMajor          = 1;
  InterfaceVersionMinor          = 0;

implementation

end.
