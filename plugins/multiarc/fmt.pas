(********************************************************
* FMT.PAS
* Archive Support API for FAR Manager 1.70 and MultiArc plugin
* Revision: 1.10 06.05.2003 $
*
* Copyright (c) 1996-2000 Eugene Roshal
* Copyrigth (c) 2000-<%YEAR%> FAR group
* Translated by Vasily V. Moshninov
*********************************************************)

{$ALIGN OFF}
{$MINENUMSIZE 4}

unit fmt;

interface
uses windows;

const
  GETARC_EOF       = 0;
  GETARC_SUCCESS   = 1;
  GETARC_BROKEN    = 2;
  GETARC_UNEXPEOF  = 3;
  GETARC_READERROR = 4;

type

  TArcItemInfo = packed record
    HostOS: packed array[0..31] of char;
    Description: packed array[0..255] of char;
    Solid: integer;
    Comment: integer;
    Encrypted: integer;
    DictSize: integer;
    UnpVer: integer;
    Chapter: integer;
  end; { TArcItemInfo record }
  PArcItemInfo = ^TArcItemInfo;


const
// ARCINFO_FLAGS
  AF_AVPRESENT    = 1;
  AF_IGNOREERRORS = 2;

type

  TArcInfo = packed record
    SFXSize: integer;
    Volume: integer;
    Comment: integer;
    Recovery: integer;
    Lock: integer;
    Flags: DWORD;
    Reserved: DWORD;
    Chapters: integer;
  end; { TArcInfo record }
  PArcInfo = ^TArcInfo;

implementation

end.
