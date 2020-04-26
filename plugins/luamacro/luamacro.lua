-- coding: utf-8
-- started: 2012-04-20

-- This plugin does not support reloading the default script on the fly.
if not (...) then return end

local function LOG (fmt, ...)
  win.OutputDebugString(fmt:format(...))
end

local F, Msg = far.Flags, nil
local bor = bit64.bor
local co_yield, co_resume, co_status = coroutine.yield, coroutine.resume, coroutine.status

local PROPAGATE={} -- a unique value, inaccessible to scripts.
local gmeta = { __index=_G }
local LastMessage
local strParseError = ""
local Shared
local TablePanelSort -- must be separate from LastMessage, otherwise Far crashes after a macro is called from CtrlF12.
local TableExecString -- must be separate from LastMessage, otherwise Far crashes
local utils, macrobrowser, panelsort, keymacro

local function ExpandEnv(str) return (str:gsub("%%(.-)%%", win.GetEnv)) end

local function pack (...)
  return { n=select("#",...), ... }
end

local function yield_resume (co, ...)
  local t1, t2 = ...
  if t1==true and t2==PROPAGATE then
    return co_resume(co, co_yield(select(2, ...)))
  end
  return ...
end

-- Override coroutine.resume for scripts, making it possible to call Keys(),
-- print(), Plugin.Call(), exit(), etc. from nested coroutines.
function coroutine.resume(co, ...) return yield_resume(co, co_resume(co, ...)) end

local ErrMsg = function(msg, title, buttons, flags)
  if type(msg)=="string" and not msg:utf8valid() and string.sub(msg,1,3)~="..." then
    local wstr = win.MultiByteToWideChar(msg, win.GetACP(), "e")
    msg = wstr and win.Utf16ToUtf8(wstr) or msg
  end
  return far.Message(msg, title or "LuaMacro", buttons, flags or "wl")
end

local function checkarg (arg, argnum, reftype)
  if type(arg) ~= reftype then
    error(("arg. #%d: %s expected, got %s"):format(argnum, reftype, type(arg)), 3)
  end
end

-------------------------------------------------------------------------------
-- Functions implemented via "returning a key" to Far
-------------------------------------------------------------------------------

function _G.Keys (...)
  for n=1,select("#",...) do
    local str=select(n,...)
    if type(str)=="string" then
      for key in str:gmatch("%S+") do
        local cnt,name = key:match("^(%d+)%*(.+)")
        if cnt then cnt = tonumber(cnt)
        else        cnt,name = 1,key
        end
        local lname = name:lower()
        if     lname == "disout" then keymacro.mmode(1,1)
        elseif lname == "enout"  then keymacro.mmode(1,0)
        else
          local R1,R2 = keymacro.TransformKey(name)
          for k=1,cnt do co_yield(PROPAGATE, F.MPRT_KEYS, R1, R2) end
        end
      end
    end
  end
end

function _G.print (...)
  local param = ""
  if select("#", ...)>0 then param = (...) end
  co_yield(PROPAGATE, F.MPRT_PRINT, tostring(param))
end

function _G.exit ()
  co_yield(PROPAGATE, "exit")
end

local function yieldcall (...)
  return co_yield(PROPAGATE, ...)
end

-------------------------------------------------------------------------------
-- END: Functions implemented via "returning a key" to Far
-------------------------------------------------------------------------------

local PluginInfo

function export.GetPluginInfo()
  local out = {
    Flags = bor(F.PF_PRELOAD,F.PF_FULLCMDLINE,F.PF_EDITOR,F.PF_VIEWER,F.PF_DIALOG),
    CommandPrefix = "lm:macro:lua:moon:luas:moons"..utils.GetPrefixes()[1],
    PluginMenuGuids = win.Uuid("EF6D67A2-59F7-4DF3-952E-F9049877B492"),
    PluginMenuStrings = { "Macro Browser" },
  }
  PluginInfo = out

  local mode = far.MacroGetArea()
  local area = utils.GetTrueAreaName(mode)
  local IsDiskMenuPossible = area=="Shell" or area=="Tree" or area=="QView" or area=="Info"
  for _,item in ipairs(utils.GetMenuItems()) do
    local flags = item.flags
    if flags.config then
      local ok, text = pcall(item.text, "Config", area)
      if ok then
        if type(text) == "string" then
          out.PluginConfigStrings = out.PluginConfigStrings or {}
          table.insert(out.PluginConfigStrings, text)
          out.PluginConfigGuids = out.PluginConfigGuids and out.PluginConfigGuids..item.guid or item.guid
        end
      else
        ErrMsg(text)
      end
    end
    if IsDiskMenuPossible and flags.disks then
      local ok, text = pcall(item.text, "Disks", area)
      if ok then
        if type(text) == "string" then
          out.DiskMenuStrings = out.DiskMenuStrings or {}
          table.insert(out.DiskMenuStrings, text)
          out.DiskMenuGuids = out.DiskMenuGuids and out.DiskMenuGuids..item.guid or item.guid
        end
      else
        ErrMsg(text)
      end
    end
    if flags.plugins and (flags[mode] or flags.common) then
      local ok, text = pcall(item.text, "Plugins", area)
      if ok then
        if type(text) == "string" then
          out.PluginMenuStrings = out.PluginMenuStrings or {}
          table.insert(out.PluginMenuStrings, text)
          out.PluginMenuGuids = out.PluginMenuGuids and out.PluginMenuGuids..item.guid or item.guid
        end
      else
        ErrMsg(text)
      end
    end
  end
  return out
end

local function GetFileParams (Text)
  local from,to = Text:find("^%s*@%s*")
  if from then
    local from2,to2,fname = Text:find("^\"([^\"]+)\"", to+1) -- test for quoted file name
    if not from2 then
      from2,to2,fname = Text:find("^(%S+)", to+1) -- test for unquoted file name
    end
    if from2 then
      local space,params = Text:match("^(%s*)(.*)", to2+1)
      if space~="" or params=="" then
        return ExpandEnv(fname), params
      end
    end
    error("Invalid macrosequence specification")
  end
end

local function loadmacro (Lang, Text, Env, ConvertPath)
  local _loadstring, _loadfile = loadstring, loadfile
  if Lang == "moonscript" then
    local ms = require "moonscript"
    _loadstring, _loadfile = ms.loadstring, ms.loadfile
  end

  local f1,f2,msg
  local fname,params = GetFileParams(Text)
  if fname then
    fname = ConvertPath and far.ConvertPath(fname, F.CPM_NATIVE) or fname
    f2,msg = _loadstring("return "..params)
    if f2 then
      f1,msg = _loadfile(fname)
    end
  else
    f1,msg = _loadstring(Text)
  end

  if f1 then
    strParseError = ""
    Env = Env or setmetatable({}, gmeta)
    Env._filename = fname
    setfenv(f1, Env)
    if f2 then setfenv(f2, Env) end
    return f1,f2
  else
    strParseError = msg
    return nil,msg
  end
end

local function postmacro (f, ...)
  if type(f) == "function" then
    keymacro.PostNewMacro(pack(f, ...), 0, nil, true)
    return true
  end
  return false
end

local function FixReturn (handle, ok, ...)
  local ret1, ret_type = ...
  if ok then
    local status = co_status(handle.coro)
    if status == "suspended" and ret1 == PROPAGATE and ret_type ~= "exit" then
      handle._store = pack(select(3, ...))
      return ret_type, handle._store
    else
      return F.MPRT_NORMALFINISH, pack(true, ...)
    end
  else
    local msg = type(ret1)=="string" and ret1 or "(error object is not a string)"
    msg = string.gsub(debug.traceback(handle.coro, msg), "\n\t", "\n   ")
    ErrMsg(msg)
    return F.MPRT_ERRORFINISH
  end
end

local function MacroStep (handle, ...)
  if handle then
    local status = co_status(handle.coro)
    if status == "suspended" then
      if handle.params then
        local params = handle.params
        handle.params = nil
        local tp = type(params)
        if tp == "function" then
          local tt = pack(xpcall(params, debug.traceback))
          if tt[1] then
            return FixReturn(handle, co_resume(handle.coro, unpack(tt,2,tt.n)))
          else
            ErrMsg(tt[2])
            return F.MPRT_ERRORFINISH
          end
        elseif tp == "table" then
          return FixReturn(handle, co_resume(handle.coro, params))
        end
      else
        return FixReturn(handle, co_resume(handle.coro, ...))
      end
    else
      ErrMsg("Step: called on macro in "..status.." status") -- debug only: should not be here
    end
  else
    ErrMsg("Step: handle does not exist") -- debug only: should not be here
  end
end

local function MacroParse (Lang, Text, onlyCheck, skipFile)
  local _loadstring, _loadfile = loadstring, loadfile
  if Lang == "moonscript" then
    local ms = require "moonscript"
    _loadstring, _loadfile = ms.loadstring, ms.loadfile
  end

  local ok,msg = true,nil
  local fname,params = GetFileParams(Text)
  if fname then
    ok,msg = _loadstring("return "..params)
    if ok and not skipFile then
      ok,msg = _loadfile(fname)
    end
  else
    ok,msg = _loadstring(Text)
  end

  if ok then
    strParseError = ""
    return F.MPRT_NORMALFINISH
  else
    strParseError = msg
    if not onlyCheck then
      far.Message(msg, Msg.MMacroParseErrorTitle, Msg.MOk, "lw")
    end
    return F.MPRT_ERRORPARSE
  end
end

local function GetLastParseError()
  LastMessage = pack(strParseError, tonumber(strParseError:match(":(%d+): ")) or 0, 0)
  return LastMessage
end

local function ExecString (lang, text, params, onlyCheck)
  if type(text)=="string" then
    local chunk, msg = loadmacro(lang, text)
    if chunk then
      TableExecString = pack(chunk(unpack(params,1,params.n)))
      return F.MPRT_NORMALFINISH, TableExecString
    else
      if not onlyCheck then ErrMsg(msg) end
      TableExecString = { msg }
      return F.MPRT_ERRORPARSE, TableExecString
    end
  end
end

local function About()
  -- LuaMacro
  local GInfo = export.GetGlobalInfo()
  local text = ("%s %d.%d.%d build %d"):format(GInfo.Title, unpack(GInfo.Version))

  -- Lua/LuaJIT
  text = text.."\n"..(jit and jit.version or _VERSION)

  -- MoonScript and LPeg
  local ok,lib = pcall(require, "moonscript.version")
  if ok then
    text = text.."\nMoonScript "..lib.version
    if lpeg then text = text.."\nLPeg "..lpeg.version() end
  end

  -- All together
  far.Message(text, "About", nil, "l")
end

local function ShowAndPass(...) far.Show(...) return ... end

local function ShowCmdLineHelp()
  local windir, fardir = win.GetEnv("WINDIR"), win.GetEnv("FARHOME")
  if windir and fardir then
    local suffix = win.GetEnv("FARLANG")=="Russian" and "ru" or "en"
    local topic = fardir.."\\Encyclopedia\\macroapi_manual."..suffix..".chm::/92.html"
    win.ShellExecute(nil, nil, windir.."\\hh.exe", topic)
  end
end

local function Open_CommandLine (strCmdLine)
  local prefix, text = strCmdLine:match("^%s*([^:%s]+):%s*(.-)%s*$")
  if not prefix then return end -- this can occur with Plugin.Command()
  prefix = prefix:lower()
  if prefix == "lm" or prefix == "macro" then
    if text=="" then ShowCmdLineHelp(); return;  end
    local cmd = text:match("%S*"):lower()
    if cmd == "load" then
      local paths = text:match("%S.*",5)
      paths = paths and paths:gsub([[^"(.+)"$]], "%1")
      far.MacroLoadAll(paths)
    elseif cmd == "save" then utils.WriteMacros()
    elseif cmd == "unload" then utils.UnloadMacros()
    elseif cmd == "about" then About()
    elseif cmd ~= "" then ErrMsg(Msg.CL_UnsupportedCommand .. cmd) end
  elseif prefix == "lua" or prefix == "moon" or prefix == "luas" or prefix == "moons" then
    if text=="" then ShowCmdLineHelp(); return;  end
    local show = false
    if text:find("^=") then
      show, text = true, text:sub(2)
    end
    local fname, params = GetFileParams(text)
    if show and not fname then
      text = "return "..text
    end
    local lang = (prefix=="lua" or prefix=="luas") and "lua" or "moonscript"
    local f1,f2 = loadmacro(lang, text, nil, true)
    if f1 then
      local ff1 = show and function(...) return ShowAndPass(f1(...)) end or f1
      if prefix=="lua" or prefix=="moon" then
        keymacro.PostNewMacro({ ff1,f2,HasFunction=true }, 0, nil, true)
      else
        f2 = f2 or function() end
        Shared.CmdLineResult = nil
        Shared.CmdLineResult = pack(ff1(f2()))
      end
    else
      ErrMsg(f2)
    end
  else
    local item = utils.GetPrefixes()[prefix]
    if item then return item.action(prefix, text) end
  end
end

local function PanelModuleExist(mod)
  for _,module in ipairs(utils.GetPanelModules()) do
    if mod == module then return true; end
  end
end

local function Open_LuaMacro (calltype, ...)
  if     calltype==F.MCT_KEYMACRO       then return keymacro.Dispatch(...)
  elseif calltype==F.MCT_MACROPARSE     then return MacroParse(...)
  elseif calltype==F.MCT_DELMACRO       then return utils.DelMacro(...)
  elseif calltype==F.MCT_ENUMMACROS     then return utils.EnumMacros(...)
  elseif calltype==F.MCT_GETMACRO       then return utils.GetMacroWrapper(...)
  elseif calltype==F.MCT_LOADMACROS     then
    local InitedRAM,Paths = ...
    keymacro.InitInternalVars(InitedRAM)
    return utils.LoadMacros(false,Paths)
  elseif calltype==F.MCT_RECORDEDMACRO  then return utils.ProcessRecordedMacro(...)
  elseif calltype==F.MCT_RUNSTARTMACRO  then return utils.RunStartMacro()
  elseif calltype==F.MCT_WRITEMACROS    then return utils.WriteMacros()
  elseif calltype==F.MCT_EXECSTRING     then return ExecString(...)
  elseif calltype==F.MCT_ADDMACRO       then return utils.AddMacroFromFAR(...)
  elseif calltype==F.MCT_PANELSORT      then
    if panelsort then
      TablePanelSort = { panelsort.SortPanelItems(...) }
      if TablePanelSort[1] then return TablePanelSort end
    end
  elseif calltype==F.MCT_GETCUSTOMSORTMODES then
    if panelsort then
      TablePanelSort = panelsort.GetSortModes()
      return TablePanelSort
    end
  elseif calltype==F.MCT_CANPANELSORT then
    return panelsort and panelsort.CanDoPanelSort(...)
  end
end

local CanCreatePanel = {
  [F.OPEN_LEFTDISKMENU]  = true;
  [F.OPEN_RIGHTDISKMENU] = true;
  [F.OPEN_FINDLIST]      = true;
  [F.OPEN_SHORTCUT]      = true;
--[F.OPEN_FILEPANEL]     = true; -- does it needed?
  [F.OPEN_PLUGINSMENU]   = true;
}

function export.Open (OpenFrom, guid, ...)
  if OpenFrom == F.OPEN_LUAMACRO then
    return Open_LuaMacro(guid, ...)

  elseif OpenFrom == F.OPEN_COMMANDLINE then
    local mod, obj = Open_CommandLine(...)
    return mod and obj and PanelModuleExist(mod) and { module=mod; object=obj }

  elseif OpenFrom == F.OPEN_ANALYSE then
    local info = ...
    local mod = info.Handle.module
    if type(mod.Open) == "function" then
      info.Handle = info.Handle.object
      local obj = mod.Open(OpenFrom, guid, info)
      return obj and { module=mod; object=obj }
    end

  elseif OpenFrom == F.OPEN_FINDLIST then
    for _,mod in ipairs(utils.GetPanelModules()) do
      if type(mod.Open) == "function" then
        local obj = mod.Open(OpenFrom, guid, ...)
        if obj then return { module=mod; object=obj } end
      end
    end

  elseif OpenFrom == F.OPEN_SHORTCUT then
    local info = ...
    if info.ShortcutData then
      local mod_guid, data = info.ShortcutData:match(
        "^(%x%x%x%x%x%x%x%x%-%x%x%x%x%-%x%x%x%x%-%x%x%x%x%-%x%x%x%x%x%x%x%x%x%x%x%x)/(.*)")
      if mod_guid then
        local mod = utils.GetPanelModules()[win.Uuid(mod_guid)]
        if mod and type(mod.Open) == "function" then
          info.ShortcutData = data
          local obj = mod.Open(OpenFrom, guid, info)
          return obj and { module=mod; object=obj }
        end
      end
    end

  elseif OpenFrom == F.OPEN_FROMMACRO then -- TODO: add panel modules support
    local argtable =  ...
    if argtable[1]=="argtest" then -- argtest: return received arguments
      return unpack(argtable, 2, argtable.n)
    elseif argtable[1]=="macropost" then -- test Mantis # 2222
      return far.MacroPost([[far.Message"macropost"]])
    end

  else
    local items = utils.GetMenuItems()
    if items[guid] then
      local mod, obj = items[guid].action(OpenFrom, ...)
      if CanCreatePanel[OpenFrom] and mod and obj and PanelModuleExist(mod) then
        return { module=mod; object=obj }
      end
    else
      macrobrowser()
    end

  end
end

-- TODO: when called from a module's panel, call that module's Configure()
function export.Configure (guid)
  local items = utils.GetMenuItems()
  if items[guid] then items[guid].action() end
end

local function Init()
  Shared = {
    ErrMsg            = ErrMsg,
    ExpandEnv         = ExpandEnv,
    GetLastParseError = GetLastParseError,
    MacroStep         = MacroStep,
    checkarg          = checkarg,
    loadmacro         = loadmacro,
    pack              = pack,
    yieldcall         = yieldcall,
  }
  Shared.MacroCallFar, far.MacroCallFar = far.MacroCallFar, nil
  Shared.FarMacroCallToLua, far.FarMacroCallToLua = far.FarMacroCallToLua, nil

  local ModuleDir = far.PluginStartupInfo().ModuleDir
  local function RunPluginFile (fname, param)
    local func,msg = assert(loadfile(ModuleDir..fname))
    return func(param)
  end

  Msg = RunPluginFile("lang.lua");
  Shared.Msg = Msg

  utils = RunPluginFile("utils.lua", Shared)
  Shared.utils = utils

  RunPluginFile("api.lua", Shared)
  mf.postmacro = postmacro
  mf.acall = function(f, ...)
    checkarg(f, 1, "function")
    return yieldcall("acall", f, ...)
  end

  keymacro = RunPluginFile("keymacro.lua", Shared)
  Shared.keymacro = keymacro
  mf.mmode, _G.mmode = keymacro.mmode, keymacro.mmode
  mf.akey, _G.akey = keymacro.akey, keymacro.akey
  mf.AddExitHandler = keymacro.AddExitHandler

  macrobrowser = RunPluginFile("mbrowser.lua", Shared)

  do -- force MoonScript to load lpeg.dll residing in %farhome%
    local cpath = package.cpath
    package.cpath = win.GetEnv("farhome").."\\?.dll"
    RunPluginFile("moonscript.lua")
    package.cpath = cpath
  end

  if bit and jit then
    RunPluginFile("winapi.lua")
    RunPluginFile("farapi.lua")

    panelsort = RunPluginFile("panelsort.lua", Shared)
    Shared.panelsort = panelsort
    Panel.LoadCustomSortMode = panelsort.LoadCustomSortMode
    Panel.SetCustomSortMode = panelsort.SetCustomSortMode
    Panel.CustomSortMenu = panelsort.CustomSortMenu
  end

  utils.FixInitialModules()
  utils.InitMacroSystem()
  local macros = win.GetEnv("farprofile").."\\Macros\\"
  local modules = macros .. "modules\\"
  package.path = modules.."?.lua;"..modules.."?\\init.lua;"..package.path
  package.moonpath = modules.."?.moon;"..modules.."?\\init.moon;"..package.moonpath
  package.cpath = macros..(win.IsProcess64bit() and "lib64" or "lib32").."\\?.dll;"..package.cpath

  if _G.IsLuaStateRecreated then
    _G.IsLuaStateRecreated = nil
    utils.LoadMacros()
  end
end

function export.Analyse(Data)
  for _,module in ipairs(utils.GetPanelModules()) do
    if type(module.Analyse) == "function" then
      local datacopy = {}; for k,v in pairs(Data) do datacopy[k]=v; end -- prevent modifying 'Data'
      local obj = module.Analyse(datacopy)
      if obj then
        return { module=module; object=obj }
      end
    end
  end
end

function export.GetOpenPanelInfo (wrapped_obj, handle, ...)
  local mod, obj = wrapped_obj.module, wrapped_obj.object
  if type(mod.GetOpenPanelInfo) == "function" then
    local op_info = mod.GetOpenPanelInfo(obj, handle, ...)
    if type(op_info) == "table" then
      if type(op_info.ShortcutData) == "string"
         and type(mod.Info) == "table"
         and type(mod.Info.Guid) == "string"
      then
        op_info._ModuleShortcutData = win.Uuid(mod.Info.Guid) .. "/" .. op_info.ShortcutData
      end
      return op_info
    end
  end
end

function export.MakeDirectory (wrapped_obj, ...)
  local func = wrapped_obj.module.MakeDirectory
  if type(func) == "function" then return func(wrapped_obj.object, ...)
  else return 1, "" -- suppress Far error message
  end
end

for _,name in ipairs {"ClosePanel","Compare","DeleteFiles","GetFiles","GetFindData",
      "ProcessHostFile","ProcessPanelEvent","ProcessPanelInput","PutFiles","SetDirectory",
      "SetFindList"} do
  export[name] =
    function(wrapped_obj, ...)
      local func = wrapped_obj.module[name]
      if type(func) == "function" then
        return func(wrapped_obj.object, ...)
      end
    end
end

local ok, msg = pcall(Init) -- pcall is used to handle RunPluginFile() failure in one place only
if not ok then
  export=nil; ErrMsg(msg)
end
