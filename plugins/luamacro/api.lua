-- api.lua

local F=far.Flags

local function checkarg (arg, argnum, reftype)
  if type(arg) ~= reftype then
    error(("arg. #%d: %s expected, got %s"):format(argnum, reftype, type(arg)), 3)
  end
end

APanel, PPanel = {}, {}
local meta = { __metatable="access denied" }

function meta.__index (tb, s)
  local pnum = tb==PPanel and 0 or 1
  if     s == "CurPos" then
    return panel.GetPanelInfo(nil, pnum).CurrentItem
  elseif s == "ColumnCount" then
    local _,n = panel.GetColumnTypes(nil, pnum):gsub("," ,"")
    return n + 1
  elseif s == "ItemCount" then
    return panel.GetPanelInfo(nil, pnum).ItemsNumber
  elseif s == "SelCount" then
    return panel.GetPanelInfo(nil, pnum).SelectedItemsNumber
  elseif s == "Current" then
    return panel.GetCurrentPanelItem(nil, pnum).FileName
  elseif s == "Path" then
    return (panel.GetPanelDirectory(nil, pnum):gsub("\\$", ""))
  elseif s == "Root" then
    return not not panel.GetPanelDirectory(nil, pnum):match("^[A-Za-z]:\\$")
  elseif s == "Folder" then
    return not not panel.GetCurrentPanelItem(nil, pnum).FileAttributes:find"d"
  elseif s == "Plugin" then
    return 0 ~= bit64.band(panel.GetPanelInfo(nil, pnum).Flags, F.PFLAGS_PLUGIN)
  elseif s == "FilePanel" then
    return panel.GetPanelInfo(nil, pnum).PanelType == F.PTYPE_FILEPANEL
  elseif s == "Left" then
    return 0 ~= bit64.band(panel.GetPanelInfo(nil, pnum).Flags, F.PFLAGS_PANELLEFT)
  elseif s == "Visible" then
    return 0 ~= bit64.band(panel.GetPanelInfo(nil, pnum).Flags, F.PFLAGS_VISIBLE)
  end
end

function meta.__newindex (tb, s, i)
  local pnum = tb==APanel and 1 or 0
  if s == "CurPos" then
    panel.RedrawPanel(nil, pnum, { CurrentItem = i })
  end
end

setmetatable(APanel, meta)
setmetatable(PPanel, meta)

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

function MacroArea() return areas[far.MacroGetArea()] or "Unknown" end

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

function msave (key, name, value)
  checkarg(key, 1, "string")
  checkarg(name, 2, "string")
  local str = serialize(value)
  if str then
    local obj = far.CreateSettings()
    local subkey = obj:CreateSubkey(0, key, "description here")
    obj:Set(subkey, name, F.FST_DATA, str)
    obj:Free()
  end
end

function mload (key, name)
  checkarg(key, 1, "string")
  checkarg(name, 2, "string")
  local obj = far.CreateSettings()
  local subkey = obj:CreateSubkey(0, key, "description here")
  local value = obj:Get(subkey, name, F.FST_DATA)
  obj:Free()
  if value then
    return assert(loadstring(value))()
  end
end
