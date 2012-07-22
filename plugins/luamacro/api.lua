-- api.lua

local F=far.Flags
local band = bit64.band

local function range (x,a,b)
  if a>b then a,b=b,a end
  if x<a then x=a elseif x>b then x=b end
  return x
end

local function checkarg (arg, argnum, reftype)
  if type(arg) ~= reftype then
    error(("arg. #%d: %s expected, got %s"):format(argnum, reftype, type(arg)), 3)
  end
end

APanel, PPanel = {}, {}
local meta = { __metatable="access denied" }

function meta.__index (tb, s)
  local pnum = tb==PPanel and 0 or 1
  local panelInfo = panel.GetPanelInfo(nil, pnum)
  if not panelInfo then return nil end
  ------------------------------------------------------------------------------
  if s     == "Root" then
    return not not panel.GetPanelDirectory(nil, pnum).Name:match("^[A-Za-z]:\\$") -- FIXME
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
  elseif s == "Path" then -- FIXME
    return (panel.GetPanelDirectory(nil, pnum).Name:gsub("\\$", ""))
  ------------------------------------------------------------------------------
  elseif s == "HostFile" then
    return panel.GetPanelHostFile(nil, pnum)
  elseif s == "Prefix" then
    return panel.GetPanelPrefix(nil, pnum)
  end
end

function meta.__newindex (tb, s, i)
  local pnum = tb==APanel and 1 or 0
  local panelInfo = panel.GetPanelInfo(nil, pnum)
  if not panelInfo then return end
  if s == "CurPos" then
    i = range(i, 1, panelInfo.ItemsNumber) -- out of range can crash Far
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

function mdelete (key, name)
  checkarg(key, 1, "string")
  if name then
    checkarg(name, 2, "string")
  end
  local obj = far.CreateSettings()
  local subkey = obj:OpenSubkey(0, key)
  if subkey then
    obj:Delete(subkey, name or nil)
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
