-- Restore the option "Create folders in uppercase" that was removed in Far 3.0.4184.

local ON = true -- set to false to turn the option OFF
local Pattern = regex.new("^\\U+$")
local F = far.Flags

local GUID = win.Uuid("fad00dbe-3fff-4095-9232-e1cc70c67737") -- "MakeFolderId"
local itempos = 3

local function Work (Event, FarDialogEvent)
  if Event == F.DE_DLGPROCINIT and FarDialogEvent.Msg == F.DN_CLOSE
                               and FarDialogEvent.Param1 >= 1 then
    local hDlg = FarDialogEvent.hDlg
    local DialogInfo = hDlg:send("DM_GETDIALOGINFO")
    if DialogInfo and DialogInfo.Id==GUID then
      local Item = hDlg:send("DM_GETDLGITEM", itempos)
      if Item and Item[1]==F.DI_EDIT and Pattern:match(Item[10]) then
        hDlg:send("DM_SETTEXT", itempos, Item[10]:upper())
      end
    end
  end
end

Event {
  description="Create folders in uppercase";
  group="DialogEvent"; condition = function() return ON end;
  action=Work;
}
