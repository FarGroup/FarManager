unit dup5drv_data;

// $Id: dup5drv_data.pas,v 1.1.1.1 2004/05/08 10:26:52 elbereth Exp $
// $Source: /cvsroot/dragonunpacker/DragonUnPACKer/plugins/drivers/dup5drv_data.pas,v $
//
// The contents of this file are subject to the Mozilla Public License
// Version 1.1 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
// specific language governing rights and limitations under the License.
//
// The Original Code is dup5drv_data.pas, released May 8, 2004.
//
// The Initial Developer of the Original Code is Alexandre Devilliers
// (elbereth@users.sourceforge.net, http://www.elberethzone.net).
//
// ===========================================================
// Optional Data handling unit (Delphi 6 Source)
// Dragon UnPACKer v5.0.0                      Plugin SDK PR-2
// ===========================================================
// Stores Data for ReadFormat() and GetEntry().

interface

uses dup5drv_utils;

procedure FSE_add(Name: String; Offset, Size: Int64; DataX, DataY: integer);
function FSE_Read(): FormatEntry;

type FSE = ^element;
     element = record
        Name : String;
        Size : Int64;
        Offset : Int64;
        DataX : integer;
        DataY : integer;
        suiv : FSE;
     end;

implementation
var DataBloc: FSE;

procedure FSE_add(Name: String; Offset, Size: Int64; DataX, DataY: integer);
var nouvFSE: FSE;
begin

  new(nouvFSE);
  nouvFSE^.Name := Name;
  nouvFSE^.Offset := Offset;
  nouvFSE^.Size := Size;
  nouvFSE^.DataX := DataX;
  nouvFSE^.DataY := DataY;
  nouvFSE^.suiv := DataBloc;
  DataBloc := nouvFSE;

end;

function FSE_Read(): FormatEntry;
var a: FSE;
begin

  if DataBloc <> NIL then
  begin
    a := DataBloc;
    DataBloc := DataBloc^.suiv;
    FSE_Read.FileName := a^.Name;
    FSE_Read.Offset := a^.Offset;
    FSE_Read.Size := a^.Size;
    FSE_Read.DataX := a^.DataX;
    FSE_Read.DataY := a^.DataY;
    Dispose(a);
  end
  else
  begin
    FSE_Read.FileName := '';
    FSE_Read.Offset := 0;
    FSE_Read.Size := 0;
    FSE_Read.DataX := 0;
    FSE_Read.DataY := 0;
  end;

end;

end.
