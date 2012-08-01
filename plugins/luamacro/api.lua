-- api.lua

local F=far.Flags
local band,bor = bit64.band,bit64.bor
local SendDlgMessage = far.SendDlgMessage
local MacroCallFar = far.MacroCallFar

-- local function range (x,a,b)
--   if a>b then a,b=b,a end
--   if x<a then x=a elseif x>b then x=b end
--   return x
-- end

local function CreateMeta()
  return { __metatable="access denied", __newindex=function() end }
end

local function checkarg (arg, argnum, reftype)
  if type(arg) ~= reftype then
    error(("arg. #%d: %s expected, got %s"):format(argnum, reftype, type(arg)), 3)
  end
end

local function UnsupportedProperty (s)
  error("property not supported: "..tostring(s), 3)
end

--------------------------------------------------------------------------------
-- Œ¡Ÿ»≈
--------------------------------------------------------------------------------

function IsUserAdmin()
  return win.GetEnv("FarAdminMode") == "1"
end

--------------------------------------------------------------------------------
-- Œ¡Ÿ»≈: Area
--------------------------------------------------------------------------------
local areas = {
  [F.MACROAREA_OTHER]                = "Other",
  [F.MACROAREA_SHELL]                = "Shell",
  [F.MACROAREA_VIEWER]               = "Viewer",
  [F.MACROAREA_EDITOR]               = "Editor",
  [F.MACROAREA_DIALOG]               = "Dialog",
  [F.MACROAREA_SEARCH]               = "Search",
  [F.MACROAREA_DISKS]                = "Disks",
  [F.MACROAREA_MAINMENU]             = "MainMenu",
  [F.MACROAREA_MENU]                 = "Menu",
  [F.MACROAREA_HELP]                 = "Help",
  [F.MACROAREA_INFOPANEL]            = "Info",
  [F.MACROAREA_QVIEWPANEL]           = "QView",
  [F.MACROAREA_TREEPANEL]            = "Tree",
  [F.MACROAREA_FINDFOLDER]           = "FindFolder",
  [F.MACROAREA_USERMENU]             = "UserMenu",
  [F.MACROAREA_SHELLAUTOCOMPLETION]  = "ShellAutoCompletion",
  [F.MACROAREA_DIALOGAUTOCOMPLETION] = "DialogAutoCompletion",
}
local areacodes = {}; for k,v in pairs(areas) do areacodes[v]=k end

function GetArea() return areas[far.MacroGetArea()] end

local meta = CreateMeta()
Area = setmetatable({}, meta)

function meta.__index (tb, s)
  local code = areacodes[s]
  if code then return MacroCallFar(code) ~= 0 end
  UnsupportedProperty(s)
end

--------------------------------------------------------------------------------

local function basicSerialize (o)
  local tp = type(o)
  if tp == "boolean" then
    return tostring(o)
  elseif tp == "number" then
    if o == math.modf(o) then return tostring(o) end
    return string.format("(%.17f * 2^%d)", math.frexp(o)) -- preserve accuracy
  elseif tp == "string" then
    return string.format("%q", o)
  end
  return nil, tp
end

local function serialize (o)
  local s, tp = basicSerialize(o)
  if s then return "return "..s end
  if tp ~= "table" then return nil end
  local t = { "return {" }
  for k,v in pairs(o) do
    local k2 = basicSerialize(k)
    if k2 then
      local v2 = basicSerialize(v)
      if v2 then
        t[#t+1] = string.format("  [%s] = %s,", k2, v2)
      end
    end
  end
  t[#t+1] = "}\n"
  return table.concat(t, "\n")
end

function mdelete (key, name)
  checkarg(key, 1, "string")
  checkarg(name, 2, "string")
  local obj = far.CreateSettings()
  local subkey = obj:OpenSubkey(0, key)
  if subkey then
    obj:Delete(subkey, name~="*" and name or nil)
  end
  obj:Free()
end

function msave (key, name, value)
  checkarg(key, 1, "string")
  checkarg(name, 2, "string")
  local str = serialize(value)
  if str then
    local obj = far.CreateSettings()
    local subkey = obj:CreateSubkey(0, key)
    obj:Set(subkey, name, F.FST_DATA, str)
    obj:Free()
  end
end

function mload (key, name)
  checkarg(key, 1, "string")
  checkarg(name, 2, "string")
  local obj = far.CreateSettings()
  local subkey = obj:OpenSubkey(0, key)
  local chunk = subkey and obj:Get(subkey, name, F.FST_DATA)
  obj:Free()
  if chunk then
    return assert(loadstring(chunk))()
  end
  return nil
end

function print (arg)
  arg = tostring(arg)
  local area = far.MacroGetArea()
  if area == F.MACROAREA_SHELL then
    panel.InsertCmdLine(nil, arg)
  elseif area == F.MACROAREA_EDITOR then
    editor.UndoRedo(Id, "EUR_BEGIN")
    editor.InsertText(nil, arg)
    editor.UndoRedo(Id, "EUR_END")
    editor.Redraw()
  elseif area == F.MACROAREA_DIALOG then
    local hDlg = far.AdvControl("ACTL_GETWINDOWINFO").Id
    local pos = SendDlgMessage(hDlg, "DM_GETFOCUS")
    if pos >= 0 then
      local item = SendDlgMessage(hDlg, "DM_GETDLGITEM", pos)
      if item then
        if item[1]==F.DI_EDIT or item[1]==F.DI_FIXEDIT or item[1]==F.DI_PSWEDIT then
          local text = SendDlgMessage(hDlg, "DM_GETTEXT", pos)
          local cursor = SendDlgMessage(hDlg, "DM_GETCURSORPOS", pos)
          SendDlgMessage(hDlg, "DM_SETTEXT", pos, text:sub(1,cursor.X)..arg..text:sub(cursor.X+1))
          cursor.X = cursor.X + arg:len()
          SendDlgMessage(hDlg, "DM_SETCURSORPOS", pos, cursor)
        end
      end
    end
  end
end

--------------------------------------------------------------------------------
-- «¿¬»—ﬂŸ»≈ Œ“  ŒÕ“≈ —“¿ »—œŒÀÕ≈Õ»ﬂ
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- ƒÀﬂ œ¿Õ≈À≈…
--------------------------------------------------------------------------------
local meta = CreateMeta()
APanel = setmetatable({}, meta)
PPanel = setmetatable({}, meta)

function meta.__index (tb, s)
  local pnum = tb==PPanel and 0 or 1
  local panelInfo = panel.GetPanelInfo(nil, pnum)
  if not panelInfo then return nil end
  ------------------------------------------------------------------------------
  if s == "Root" then
    local dir = panel.GetPanelDirectory(nil, pnum).Name
    return dir:lower() == far.GetPathRoot(dir):lower()
  elseif s == "Bof" then
    return panelInfo.CurrentItem == 1
  elseif s == "Eof" then
    return panelInfo.CurrentItem == panelInfo.ItemsNumber
  elseif s == "Empty" then
    return (panelInfo.ItemsNumber == 0) or (panelInfo.ItemsNumber == 1 and
      not not panel.GetPanelItem(nil,pnum,1).FileName:find("^%.%.[\\/]?$"))
  elseif s == "Selected" then
    if panelInfo.SelectedItemsNumber == 1 then
      local item = panel.GetSelectedPanelItem(nil,pnum,1)
      return item and 0 ~= band(item.Flags, F.PPIF_SELECTED)
    end
    return panelInfo.SelectedItemsNumber > 1
  elseif s == "Folder" then
    local item = panel.GetCurrentPanelItem(nil, pnum)
    return not not (item and item.FileAttributes:find"d")
  elseif s == "Plugin" then
    return 0 ~= band(panelInfo.Flags, F.PFLAGS_PLUGIN)
  ------------------------------------------------------------------------------
  elseif s == "FilePanel" then
    return panelInfo.PanelType == F.PTYPE_FILEPANEL
  elseif s == "Left" then
    return 0 ~= band(panelInfo.Flags, F.PFLAGS_PANELLEFT)
  elseif s == "Visible" then
    return 0 ~= band(panelInfo.Flags, F.PFLAGS_VISIBLE)
  ------------------------------------------------------------------------------
  elseif s == "ColumnCount" then
    local _,n = panel.GetColumnTypes(nil, pnum):gsub("," ,"")
    return n + 1
  elseif s == "ItemCount" then
    return panelInfo.ItemsNumber
  elseif s == "CurPos" then
    return panelInfo.CurrentItem
  elseif s == "SelCount" then
    return panelInfo.SelectedItemsNumber
  elseif s == "Current" then
    local item = panel.GetCurrentPanelItem(nil, pnum)
    return item and item.FileName or ""
  elseif s == "Path" then
    return (panel.GetPanelDirectory(nil, pnum).Name:gsub("\\$", ""))
  ------------------------------------------------------------------------------
  elseif s == "HostFile" then
    return panel.GetPanelHostFile(nil, pnum)
  elseif s == "Prefix" then
    return panel.GetPanelPrefix(nil, pnum)
  ------------------------------------------------------------------------------
  elseif s == "Height" then
    return panelInfo.PanelRect.bottom - panelInfo.PanelRect.top + 1
  elseif s == "Width" then
    return panelInfo.PanelRect.right - panelInfo.PanelRect.left + 1
  elseif s == "Type" then
    return panelInfo.PanelType
  else
    UnsupportedProperty(s)
  end
end

--------------------------------------------------------------------------------
-- ƒÀﬂ  ŒÃ¿ÕƒÕŒ… —“–Œ »
--------------------------------------------------------------------------------
local meta = CreateMeta()
CmdLine = setmetatable({}, meta)

function meta.__index (tb, s)
  if s == "Bof" then
    return panel.GetCmdLinePos()==1
  elseif s == "Eof" then
    return panel.GetCmdLinePos()==panel.GetCmdLine():len()+1
  elseif s == "Empty" then
    return panel.GetCmdLine()==""
  elseif s == "Selected" then
    local from,to = panel.GetCmdLineSelection()
    return to >= from
  elseif s == "ItemCount" then
    return panel.GetCmdLine():len()
  elseif s == "CurPos" then
    return panel.GetCmdLinePos()
  elseif s == "Value" then
    return panel.GetCmdLine()
  else
    UnsupportedProperty(s)
  end
end

--------------------------------------------------------------------------------
-- ƒÀﬂ –≈ƒ¿ “Œ–¿
--------------------------------------------------------------------------------
local meta = CreateMeta()
Editor = setmetatable({}, meta)

-- Taken from plugin LF4Ed.
local function GetSelectedText()
  local ei = editor.GetInfo()
  if ei and ei.BlockType ~= F.BTYPE_NONE then
    local t = {}
    local n = ei.BlockStartLine
    while true do
      local s = editor.GetString(nil, n, 1)
      if not s or s.SelStart == -1 then
        break
      end
      local sel = s.StringText:sub (s.SelStart+1, s.SelEnd)
      table.insert(t, sel)
      n = n + 1
    end
    editor.SetPosition(nil, ei)
    return table.concat(t, "\n")
  end
end

function meta.__index (tb, s)
  local info = assert(editor.GetInfo(), "no editor instance is open.")
  if s == "FileName" then
    return info.FileName
  elseif s == "Lines" then
    return info.TotalLines
  elseif s == "CurLine" then
    return info.CurLine+1
  elseif s == "CurPos" then
    return info.CurTabPos+1
  elseif s == "RealPos" then
    return info.CurPos+1
  elseif s == "Value" then
    return editor.GetString().StringText
  elseif s == "SelValue" then
    return GetSelectedText() or ""
  else
    UnsupportedProperty(s)
  end
end

--------------------------------------------------------------------------------
-- ƒÀﬂ ¬Õ”“–≈ÕÕ≈… œ–Œ√–¿ÃÃ€ œ–Œ—ÃŒ“–¿
--------------------------------------------------------------------------------
local meta = CreateMeta()
Viewer = setmetatable({}, meta)

function meta.__index (tb, s)
  local info = assert(viewer.GetInfo(), "no viewer instance is open.")
  if s == "FileName" then
    return info.FileName
  else
    UnsupportedProperty(s)
  end
end

--------------------------------------------------------------------------------
-- ƒÀﬂ ƒ»¿ÀŒ√Œ¬
--------------------------------------------------------------------------------
local meta = CreateMeta()
Dlg = setmetatable({}, meta)

function meta.__index (tb, s)
  local hDlg = far.AdvControl("ACTL_GETWINDOWINFO").Id
  assert(type(hDlg)=="userdata", "active window is not dialog.")
  local info = SendDlgMessage(hDlg, "DM_GETDIALOGINFO")
  if s == "ItemCount" then
    return info.ItemsNumber
  elseif s == "CurPos" then
    return info.FocusPos+1
  elseif s == "PrevPos" then
    return info.PrevFocusPos+1
  elseif s == "Id" then
    return win.Uuid(info.Id)
  elseif s == "ItemType" then
    local item = SendDlgMessage(hDlg, "DM_GETDLGITEM", info.FocusPos)
    return item and item[1] or -1
  else
    UnsupportedProperty(s)
  end
end

--------------------------------------------------------------------------------
-- ƒÀﬂ Ã≈Õﬁ » —œ»— Œ¬
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- ƒÀﬂ —»—“≈Ã€ œŒÃŒŸ»
--------------------------------------------------------------------------------
