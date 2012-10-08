-- api.lua

local args = (...) or {}
local F=far.Flags
local band,bor = bit64.band,bit64.bor
local MacroCallFar = far.MacroCallFar

local checkarg = args.checkarg

local function SetProperties (namespace, proptable)
  local meta = { __metatable="access denied", __newindex=function() end }
  meta.__index = function(tb,nm)
    if nm == "properties" then return proptable end -- to allow introspection
    local f = proptable[nm]
    if f then return f() end
    error("property not supported: "..tostring(nm), 2)
  end
  setmetatable(namespace, meta)
  return namespace
end
--------------------------------------------------------------------------------

-- "mf" ("macrofunctions") namespace
mf = {
  abs             = function(...) return MacroCallFar(0x80C01, ...) end,
  akey            = function(...) return MacroCallFar(0x80C02, ...) end,
  asc             = function(...) return MacroCallFar(0x80C03, ...) end,
  atoi            = function(...) return MacroCallFar(0x80C04, ...) end,
  beep            = function(...) return MacroCallFar(0x80C48, ...) end,
  chr             = function(...) return MacroCallFar(0x80C06, ...) end,
  clip            = function(...) return MacroCallFar(0x80C05, ...) end,
  date            = function(...) return MacroCallFar(0x80C07, ...) end,
  env             = function(...) return MacroCallFar(0x80C0D, ...) end,
  fattr           = function(...) return MacroCallFar(0x80C0E, ...) end,
  fexist          = function(...) return MacroCallFar(0x80C0F, ...) end,
  float           = function(...) return MacroCallFar(0x80C41, ...) end,
  flock           = function(...) return MacroCallFar(0x80C31, ...) end,
  fmatch          = function(...) return MacroCallFar(0x80C4D, ...) end,
  fsplit          = function(...) return MacroCallFar(0x80C10, ...) end,
  index           = function(...) return MacroCallFar(0x80C12, ...) end,
  int             = function(...) return MacroCallFar(0x80C13, ...) end,
  itoa            = function(...) return MacroCallFar(0x80C14, ...) end,
  key             = function(...) return MacroCallFar(0x80C15, ...) end,
  lcase           = function(...) return MacroCallFar(0x80C16, ...) end,
  len             = function(...) return MacroCallFar(0x80C17, ...) end,
  max             = function(...) return MacroCallFar(0x80C18, ...) end,
  min             = function(...) return MacroCallFar(0x80C1D, ...) end,
  mmode           = function(...) return MacroCallFar(0x80C44, ...) end,
  mod             = function(...) return MacroCallFar(0x80C1E, ...) end,
  msgbox          = function(...) return MacroCallFar(0x80C21, ...) end,
  prompt          = function(...) return MacroCallFar(0x80C34, ...) end,
  replace         = function(...) return MacroCallFar(0x80C33, ...) end,
  rindex          = function(...) return MacroCallFar(0x80C2A, ...) end,
  size2str        = function(...) return MacroCallFar(0x80C59, ...) end,
  sleep           = function(...) return MacroCallFar(0x80C2B, ...) end,
  string          = function(...) return MacroCallFar(0x80C2C, ...) end,
  strpad          = function(...) return MacroCallFar(0x80C5F, ...) end,
  strwrap         = function(...) return MacroCallFar(0x80C5A, ...) end,
  substr          = function(...) return MacroCallFar(0x80C2D, ...) end,
  testfolder      = function(...) return MacroCallFar(0x80C42, ...) end,
  trim            = function(...) return MacroCallFar(0x80C40, ...) end,
  ucase           = function(...) return MacroCallFar(0x80C2E, ...) end,
  waitkey         = function(...) return MacroCallFar(0x80C2F, ...) end,
  xlat            = function(...) return MacroCallFar(0x80C30, ...) end,
}

mf.iif = function(Expr, res1, res2)
  if Expr and Expr~="" then return res1 else return res2 end
end
--------------------------------------------------------------------------------

Object = {
  CheckHotkey = function(...) return MacroCallFar(0x80C19, ...) end,
  GetHotkey   = function(...) return MacroCallFar(0x80C1A, ...) end,
}

SetProperties(Object, {
  Bof        = function() return MacroCallFar(0x80413) end,
  CurPos     = function() return MacroCallFar(0x80827) end,
  Empty      = function() return MacroCallFar(0x80415) end,
  Eof        = function() return MacroCallFar(0x80414) end,
  Height     = function() return MacroCallFar(0x80829) end,
  ItemCount  = function() return MacroCallFar(0x80826) end,
  RootFolder = function() return MacroCallFar(0x80417) end,
  Selected   = function() return MacroCallFar(0x80416) end,
  Title      = function() return MacroCallFar(0x80828) end,
  Width      = function() return MacroCallFar(0x8082A) end,
})
--------------------------------------------------------------------------------

local prop_Area = {
  Current    = function() return MacroCallFar(0x80805) end,
  -- Note: 0x80400 is subtracted from opcodes here.
  Dialog     = function() return MacroCallFar(0x04) end,
  DialogAutoCompletion = function() return MacroCallFar(0x10) end,
  Disks      = function() return MacroCallFar(0x06) end,
  Editor     = function() return MacroCallFar(0x03) end,
  FindFolder = function() return MacroCallFar(0x0D) end,
  Help       = function() return MacroCallFar(0x09) end,
  Info       = function() return MacroCallFar(0x0A) end,
  MainMenu   = function() return MacroCallFar(0x07) end,
  Menu       = function() return MacroCallFar(0x08) end,
  Other      = function() return MacroCallFar(0x00) end,
  QView      = function() return MacroCallFar(0x0B) end,
  Search     = function() return MacroCallFar(0x05) end,
  Shell      = function() return MacroCallFar(0x01) end,
  ShellAutoCompletion = function() return MacroCallFar(0x0F) end,
  Tree       = function() return MacroCallFar(0x0C) end,
  UserMenu   = function() return MacroCallFar(0x0E) end,
  Viewer     = function() return MacroCallFar(0x02) end,
}

local prop_APanel = {
  Bof         = function() return MacroCallFar(0x80418) end,
  ColumnCount = function() return MacroCallFar(0x8081E) end,
  CurPos      = function() return MacroCallFar(0x80816) end,
  Current     = function() return MacroCallFar(0x80806) end,
  DriveType   = function() return MacroCallFar(0x8081A) end,
  Empty       = function() return MacroCallFar(0x8041C) end,
  Eof         = function() return MacroCallFar(0x8041A) end,
  FilePanel   = function() return MacroCallFar(0x80426) end,
  Filter      = function() return MacroCallFar(0x8042E) end,
  Folder      = function() return MacroCallFar(0x80428) end,
  Format      = function() return MacroCallFar(0x80824) end,
  Height      = function() return MacroCallFar(0x8081C) end,
  HostFile    = function() return MacroCallFar(0x80820) end,
  ItemCount   = function() return MacroCallFar(0x80814) end,
  Left        = function() return MacroCallFar(0x8042A) end,
  LFN         = function() return MacroCallFar(0x8042C) end,
  OPIFlags    = function() return MacroCallFar(0x80818) end,
  Path        = function() return MacroCallFar(0x8080A) end,
  Path0       = function() return MacroCallFar(0x8080C) end,
  Plugin      = function() return MacroCallFar(0x80424) end,
  Prefix      = function() return MacroCallFar(0x80822) end,
  Root        = function() return MacroCallFar(0x80420) end,
  SelCount    = function() return MacroCallFar(0x80808) end,
  Selected    = function() return MacroCallFar(0x8041E) end,
  Type        = function() return MacroCallFar(0x80812) end,
  UNCPath     = function() return MacroCallFar(0x8080E) end,
  Visible     = function() return MacroCallFar(0x80422) end,
  Width       = function() return MacroCallFar(0x80810) end,
}

local prop_PPanel = {
  Bof         = function() return MacroCallFar(0x80419) end,
  ColumnCount = function() return MacroCallFar(0x8081F) end,
  CurPos      = function() return MacroCallFar(0x80817) end,
  Current     = function() return MacroCallFar(0x80807) end,
  DriveType   = function() return MacroCallFar(0x8081B) end,
  Empty       = function() return MacroCallFar(0x8041D) end,
  Eof         = function() return MacroCallFar(0x8041B) end,
  FilePanel   = function() return MacroCallFar(0x80427) end,
  Filter      = function() return MacroCallFar(0x8042F) end,
  Folder      = function() return MacroCallFar(0x80429) end,
  Format      = function() return MacroCallFar(0x80825) end,
  Height      = function() return MacroCallFar(0x8081D) end,
  HostFile    = function() return MacroCallFar(0x80821) end,
  ItemCount   = function() return MacroCallFar(0x80815) end,
  Left        = function() return MacroCallFar(0x8042B) end,
  LFN         = function() return MacroCallFar(0x8042D) end,
  OPIFlags    = function() return MacroCallFar(0x80819) end,
  Path        = function() return MacroCallFar(0x8080B) end,
  Path0       = function() return MacroCallFar(0x8080D) end,
  Plugin      = function() return MacroCallFar(0x80425) end,
  Prefix      = function() return MacroCallFar(0x80823) end,
  Root        = function() return MacroCallFar(0x80421) end,
  SelCount    = function() return MacroCallFar(0x80809) end,
  Selected    = function() return MacroCallFar(0x8041F) end,
  Type        = function() return MacroCallFar(0x80813) end,
  UNCPath     = function() return MacroCallFar(0x8080F) end,
  Visible     = function() return MacroCallFar(0x80423) end,
  Width       = function() return MacroCallFar(0x80811) end,
}

local prop_CmdLine = {
  Bof       = function() return MacroCallFar(0x80430) end,
  Empty     = function() return MacroCallFar(0x80432) end,
  Eof       = function() return MacroCallFar(0x80431) end,
  Selected  = function() return MacroCallFar(0x80433) end,
  CurPos    = function() return MacroCallFar(0x8083C) end,
  ItemCount = function() return MacroCallFar(0x8083B) end,
  Value     = function() return MacroCallFar(0x8083D) end,
}

local prop_Drv = {
  ShowMode = function() return MacroCallFar(0x8083F) end,
  ShowPos  = function() return MacroCallFar(0x8083E) end,
}

local prop_Help = {
  FileName = function() return MacroCallFar(0x80840) end,
  SelTopic = function() return MacroCallFar(0x80842) end,
  Topic    = function() return MacroCallFar(0x80841) end,
}

local prop_Mouse = {
  X          = function() return MacroCallFar(0x80001) end,
  Y          = function() return MacroCallFar(0x80002) end,
  Button     = function() return MacroCallFar(0x80003) end,
  CtrlState  = function() return MacroCallFar(0x80004) end,
  EventFlags = function() return MacroCallFar(0x80005) end,
}

local prop_Viewer = {
  FileName = function() return MacroCallFar(0x80839) end,
  State    = function() return MacroCallFar(0x8083A) end,
}
--------------------------------------------------------------------------------

Dlg = {
  GetValue = function(...) return MacroCallFar(0x80C08, ...) end,
  SetFocus = function(...) return MacroCallFar(0x80C57, ...) end,
}

SetProperties(Dlg, {
  CurPos     = function() return MacroCallFar(0x80835) end,
  Id         = function() return MacroCallFar(0x80837) end,
  Owner      = function() return MacroCallFar(0x80838) end,
  ItemCount  = function() return MacroCallFar(0x80834) end,
  ItemType   = function() return MacroCallFar(0x80833) end,
  PrevPos    = function() return MacroCallFar(0x80836) end,
})
--------------------------------------------------------------------------------

Editor = {
  DelLine  = function(...) return MacroCallFar(0x80C60, ...) end,
  GetStr   = function(...) return MacroCallFar(0x80C61, ...) end,
  InsStr   = function(...) return MacroCallFar(0x80C62, ...) end,
  Pos      = function(...) return MacroCallFar(0x80C0C, ...) end,
  Sel      = function(...) return MacroCallFar(0x80C09, ...) end,
  Set      = function(...) return MacroCallFar(0x80C0A, ...) end,
  SetStr   = function(...) return MacroCallFar(0x80C63, ...) end,
  SetTitle = function(...) return MacroCallFar(0x80C45, ...) end,
  Undo     = function(...) return MacroCallFar(0x80C0B, ...) end,
}

SetProperties(Editor, {
  CurLine  = function() return MacroCallFar(0x8082D) end,
  CurPos   = function() return MacroCallFar(0x8082E) end,
  FileName = function() return MacroCallFar(0x8082B) end,
  Lines    = function() return MacroCallFar(0x8082C) end,
  RealPos  = function() return MacroCallFar(0x8082F) end,
  SelValue = function() return MacroCallFar(0x80832) end,
  State    = function() return MacroCallFar(0x80830) end,
  Value    = function() return MacroCallFar(0x80831) end,
})
--------------------------------------------------------------------------------

Menu = {
  Filter     = function(...) return MacroCallFar(0x80C55, ...) end,
  FilterStr  = function(...) return MacroCallFar(0x80C56, ...) end,
  GetValue   = function(...) return MacroCallFar(0x80C46, ...) end,
  ItemStatus = function(...) return MacroCallFar(0x80C47, ...) end,
  Select     = function(...) return MacroCallFar(0x80C1B, ...) end,
  Show       = function(...) return MacroCallFar(0x80C1C, ...) end,
}

SetProperties(Menu, {
  Id         = function() return MacroCallFar(0x80844) end,
  Value      = function() return MacroCallFar(0x80843) end,
})
--------------------------------------------------------------------------------

Far = {
  Cfg_Get        = function(...) return MacroCallFar(0x80C58, ...) end,
  DisableHistory = function(...) return MacroCallFar(0x80C4C, ...) end,
  KbdLayout      = function(...) return MacroCallFar(0x80C49, ...) end,
  KeyBar_Show    = function(...) return MacroCallFar(0x80C4B, ...) end,
  Window_Scroll  = function(...) return MacroCallFar(0x80C4A, ...) end,
}

SetProperties(Far, {
  FullScreen     = function() return MacroCallFar(0x80411) end,
  Height         = function() return MacroCallFar(0x80801) end,
  IsUserAdmin    = function() return MacroCallFar(0x80412) end,
  PID            = function() return MacroCallFar(0x80804) end,
  Title          = function() return MacroCallFar(0x80802) end,
  UpTime         = function() return MacroCallFar(0x80803) end,
  Width          = function() return MacroCallFar(0x80800) end,
})
--------------------------------------------------------------------------------

BM = {
  Add   = function(...) return MacroCallFar(0x80C35, ...) end,
  Back  = function(...) return MacroCallFar(0x80C3D, ...) end,
  Clear = function(...) return MacroCallFar(0x80C36, ...) end,
  Del   = function(...) return MacroCallFar(0x80C37, ...) end,
  Get   = function(...) return MacroCallFar(0x80C38, ...) end,
  Goto  = function(...) return MacroCallFar(0x80C39, ...) end,
  Next  = function(...) return MacroCallFar(0x80C3A, ...) end,
  Pop   = function(...) return MacroCallFar(0x80C3B, ...) end,
  Prev  = function(...) return MacroCallFar(0x80C3C, ...) end,
  Push  = function(...) return MacroCallFar(0x80C3E, ...) end,
  Stat  = function(...) return MacroCallFar(0x80C3F, ...) end,
}
--------------------------------------------------------------------------------

Plugin = {
--Call     = function(...) return MacroCallFar(0x80C50, ...) end,
--Command  = function(...) return MacroCallFar(0x80C52, ...) end,
--Config   = function(...) return MacroCallFar(0x80C4F, ...) end,
  Exist    = function(...) return MacroCallFar(0x80C54, ...) end,
  Load     = function(...) return MacroCallFar(0x80C51, ...) end,
--Menu     = function(...) return MacroCallFar(0x80C4E, ...) end,
  Unload   = function(...) return MacroCallFar(0x80C53, ...) end,
}
--------------------------------------------------------------------------------

Panel = {
  FAttr     = function(...) return MacroCallFar(0x80C22, ...) end,
  FExist    = function(...) return MacroCallFar(0x80C24, ...) end,
  Item      = function(...) return MacroCallFar(0x80C28, ...) end,
  Select    = function(...) return MacroCallFar(0x80C27, ...) end,
  SetPath   = function(...) return MacroCallFar(0x80C23, ...) end,
  SetPos    = function(...) return MacroCallFar(0x80C25, ...) end,
  SetPosIdx = function(...) return MacroCallFar(0x80C26, ...) end,
}
--------------------------------------------------------------------------------

Area    = SetProperties({}, prop_Area)
APanel  = SetProperties({}, prop_APanel)
PPanel  = SetProperties({}, prop_PPanel)
CmdLine = SetProperties({}, prop_CmdLine)
Drv     = SetProperties({}, prop_Drv)
Help    = SetProperties({}, prop_Help)
Mouse   = SetProperties({}, prop_Mouse)
Viewer  = SetProperties({}, prop_Viewer)
--------------------------------------------------------------------------------

mf.eval = function (str, mode)
  if type(str) ~= "string" then return -1 end
  mode = mode or 0
  if not (mode==0 or mode==1 or mode==2 or mode==3) then return -1 end

  if mode == 2 then
    str = MacroCallFar(0x80006, str)
    if not str then return -2 end
  end

  local chunk, msg
  if string.sub(str,1,1) == "@" then
    str = string.sub(str,2):gsub("%%(.-)%%", win.GetEnv)
    chunk, msg = loadfile(str)
  else
    chunk, msg = loadstring(str)
  end

  if chunk then
    if mode==1 then return 0 end
    if mode==3 then return "" end
    setfenv(chunk, getfenv(2))
    chunk()
    return 0
  else
    far.Message(msg, "LuaMacro", nil, "wl")
    return mode==3 and msg or 11
  end
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
end

local function int64Serialize (o)
  if bit64.type(o) then
    return "bit64.new(\"" .. tostring(o) .. "\")"
  end
end

local function serialize (o)
  local s = basicSerialize(o) or int64Serialize(o)
  if s then return "return "..s end
  if type(o) == "table" then
    local t = { "return {" }
    for k,v in pairs(o) do
      local k2 = basicSerialize(k)
      if k2 then
        local v2 = basicSerialize(v) or int64Serialize(v)
        if v2 then
          t[#t+1] = string.format("  [%s] = %s,", k2, v2)
        end
      end
    end
    t[#t+1] = "}\n"
    return table.concat(t, "\n")
  end
end

function mf.mdelete (key, name)
  checkarg(key, 1, "string")
  checkarg(name, 2, "string")
  local obj = far.CreateSettings()
  local subkey = obj:OpenSubkey(0, key)
  if subkey then
    obj:Delete(subkey, name~="*" and name or nil)
  end
  obj:Free()
end

function mf.msave (key, name, value)
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

function mf.mload (key, name)
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
--------------------------------------------------------------------------------

_G.band, _G.bnot, _G.bor, _G.bxor, _G.lshift, _G.rshift =
  bit64.band, bit64.bnot, bit64.bor, bit64.bxor, bit64.lshift, bit64.rshift

_G.akey, _G.eval, _G.mmode, _G.msgbox, _G.prompt =
  mf.akey, mf.eval, mf.mmode, mf.msgbox, mf.prompt

mf.band, mf.bnot, mf.bor, mf.bxor, mf.lshift, mf.rshift =
  bit64.band, bit64.bnot, bit64.bor, bit64.bxor, bit64.lshift, bit64.rshift

mf.Keys, mf.exit, mf.print, mf.printf =
  _G.Keys, _G.exit, _G.print, _G.printf
--------------------------------------------------------------------------------
