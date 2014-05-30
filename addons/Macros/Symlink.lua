local F = far.Flags
local guid = win.Uuid("84CFDB2D-416F-4E29-813C-24F96BF77533")
local src, link_name, target_path, abs_path, symlink = "", "", "", false, true

local Items = {
--[[ 1]] {F.DI_DOUBLEBOX, 3,1, 65,9, 0, 0,0, 0, "Create Link"},
--[[ 2]] {F.DI_TEXT,      5,2, 14,2, 0, 0,0, 0, "&Link name:"},
--[[ 3]] {F.DI_EDIT,      5,3, 63,3, 0, 0,0, 0, ""},
--[[ 4]] {F.DI_TEXT,      5,4, 11,4, 0, 0,0, 0, "Target:"},
--[[ 5]] {F.DI_EDIT,      5,5, 63,5, 0, 0,0, F.DIF_READONLY, ""},
--[[ 6]] {F.DI_CHECKBOX,  5,6, 21,6, 0, 0,0, 0, "&Absolute Path"},
--[[ 7]] {F.DI_CHECKBOX, 31,6, 44,6, 0, 0,0, 0, "&Symlink"},
--[[ 8]] {F.DI_TEXT,     -1,7,  0,0, 0, 0,0, F.DIF_SEPARATOR,""},
--[[ 9]] {F.DI_BUTTON,    0,8,  0,0, 0, 0,0, F.DIF_DEFAULTBUTTON+F.DIF_CENTERGROUP,"Ok"},
--[[10]] {F.DI_BUTTON,    0,8,  0,0, 0, 0,0, F.DIF_CENTERGROUP,"Cancel"}
}

local function mk_target(lname, spath, absp)
  local same, pos = 0, 0
  if not absp then
    repeat
      same = pos; pos = lname:cfind("\\",pos+1)
    until not pos or lname:sub(1,pos):lower() ~= spath:sub(1,pos):lower()
  end
  if same <= 0 or same >= lname:len() then return spath else
    local target = ""; pos = same
    while lname:cfind("\\",pos+1) do target = target .. "..\\"; pos = lname:cfind("\\",pos+1) end
    return target .. spath:sub(same+1)
  end
end

local function dlg_proc(hDlg, Msg, Param1, Param2)
  if Msg == F.DN_INITDIALOG then
    src = APanel.Current
    if APanel.Path ~= "" then src = APanel.Path .. (src == ".." and "" or "\\" .. src) end
    link_name = PPanel.Path .. "\\" .. mf.fsplit(src, 4+8)
    hDlg:send(F.DM_SETTEXT,  3, link_name)
    hDlg:send(F.DM_SETCHECK, 6, abs_path and F.BSTATE_CHECKED or F.BSTATE_UNCHECKED)
    hDlg:send(F.DM_SETCHECK, 7, symlink and F.BSTATE_CHECKED or F.BSTATE_UNCHECKED)
  elseif Msg == F.DN_BTNCLICK and Param1 == 6 then   -- [x] Abs
    abs_path = Param2 ~= 0
  elseif Msg == F.DN_BTNCLICK and Param1 == 7 then   -- [x] Symlink
    symlink = Param2 ~= 0
  elseif Msg == F.DN_EDITCHANGE and Param1 == 3 then -- link name changed 
    link_name = hDlg:send(F.DM_GETTEXT, 3)
  else
    return
  end
  target_path = mk_target(link_name, src, abs_path or not symlink)
  hDlg:send(F.DM_SETTEXT, 5, target_path)
  return true
end

Macro {
  area="Shell"; key="AltShiftF6"; flags="NoPluginPPanels"; description="Make symlink";
  condition = function()
    return PPanel.FilePanel and PPanel.Visible and (not APanel.Plugin or band(APanel.OPIFlags,F.OPIF_REALNAMES)~=0)
  end;
  action = function()
   if (9 == far.Dialog(guid,-1,-1,69,11,nil,Items,nil,dlg_proc) and link_name ~= "") then
     local attr = win.GetFileAttr(src)
     local dir = attr and attr:find("d")
     local link_type = dir and F.LINK_SYMLINKDIR or F.LINK_SYMLINKFILE
     if not symlink then link_type = dir and F.LINK_JUNCTION or F.LINK_HARDLINK end
     far.MkLink(target_path, link_name, link_type, F.MLF_SHOWERRMSG+F.MLF_HOLDTARGET)
   end
  end;
}
