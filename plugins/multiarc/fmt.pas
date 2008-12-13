(**************************************************************
* FMT.PAS
* Archive Support API for FAR Manager 1.75 and MultiArc plugin
* Revision: 1.11 21.03.2006 $
*
* Copyright (c) 1996-2000 Eugene Roshal
* Copyrigth (c) 2000-2006 FAR group
* Translated by WARP ItSelf
**************************************************************)

{$IFNDEF VIRTUALPASCAL}
   {$ALIGN OFF}
   {$MINENUMSIZE 4}
{$ENDIF}

unit Fmt;

interface

uses Windows;

const
   GETARC_EOF       = 0;
   GETARC_SUCCESS   = 1;
   GETARC_BROKEN    = 2;
   GETARC_UNEXPEOF  = 3;
   GETARC_READERROR = 4;


type
   PArcItemInfo = ^TArcItemInfo;
   TArcItemInfo = packed record
      HostOS : array [0..31] of Char;
      Description : array [0..255] of Char;
      Solid : Integer;
      Comment : Integer;
      Encrypted : Integer;
      DictSize : Integer;
      UnpVer : Integer;
      Chapter : Integer;
   end;

const
   AF_AVPRESENT    = $00000001;
   AF_IGNOREERRORS = $00000002;
   AF_HDRENCRYPTED = $00000080;

type
   PArcInfo = ^TArcInfo;
   TArcInfo = packed record
      SFXSize : Integer;
      Volume : Integer;
      Comment : Integer;
      Recovery : Integer;
      Lock : Integer;
      Flags : Cardinal;
      Reserved : Cardinal;
      Chapters : Integer;
   end;


implementation
end.
