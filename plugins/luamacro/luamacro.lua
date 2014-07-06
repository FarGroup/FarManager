-- started: 2012-04-20

-- This plugin does not support reloading the default script on the fly.
if far.ReloadDefaultScript then return end

local function LOG (fmt, ...)
  local log = io.open("c:\\lua.log",--[["at"]]"a")
  if log then
    log:write("LUA: ", fmt:format(...), "\n")
    log:close()
  end
end

local F, Msg = far.Flags, nil
local bor = bit64.bor
local co_yield, co_resume, co_status, co_wrap =
  coroutine.yield, coroutine.resume, coroutine.status, coroutine.wrap

local PROPAGATE={} -- a unique value, inaccessible to scripts.
local gmeta = { __index=_G }
local LastMessage = {}
local TablePanelSort -- must be separate from LastMessage, otherwise Far crashes after a macro is called from CtrlF12.
local TableExecString -- must be separate from LastMessage, otherwise Far crashes
local utils, macrobrowser, panelsort, keymacro

local function ExpandEnv(str) return (str:gsub("%%(.-)%%", win.GetEnv)) end

local function pack (...)
  return { n=select("#",...), ... }
end

-- Override coroutine.resume for scripts, making it possible to call Keys(),
-- print(), Plugin.Call(), exit(), etc. from nested coroutines.
function coroutine.resume(co, ...)
  local t = pack(co_resume(co, ...))
  while t[1]==true and t[2]==PROPAGATE do
    t = pack(co_resume(co, co_yield(unpack(t, 2, t.n))))
  end
  return unpack(t, 1, t.n)
end

local ErrMsg = function(msg,title)
  if type(msg)=="string" and not msg:utf8valid() then
    local wstr = win.MultiByteToWideChar(msg, win.GetACP(), "e")
    msg = wstr and win.Utf16ToUtf8(wstr) or msg
  end
  far.Message(msg,title or "LuaMacro",nil,"wl")
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
        co_yield(PROPAGATE, F.MPRT_KEYS, key)
      end
    end
  end
end

function _G.print (...)
  local param = ""
  if select("#", ...)>0 then param = (...) end
  co_yield(PROPAGATE, F.MPRT_PRINT, tostring(param))
end

function _G.printf (fmt, ...)
  checkarg(fmt,1,"string")
  return _G.print(fmt:format(...))
end

function _G.exit ()
  co_yield(PROPAGATE, "exit")
end

local function yieldcall (ret_code, ...)
  return co_yield(PROPAGATE, ret_code, pack(...))
end

-------------------------------------------------------------------------------
-- END: Functions implemented via "returning a key" to Far
-------------------------------------------------------------------------------

local PluginInfo

function export.GetPluginInfo()
  PluginInfo = {
    Flags = bor(F.PF_PRELOAD,F.PF_FULLCMDLINE,F.PF_EDITOR,F.PF_VIEWER,F.PF_DIALOG),
    CommandPrefix = "lm:macro:lua:moon",
    PluginMenuGuids = win.Uuid("EF6D67A2-59F7-4DF3-952E-F9049877B492"),
    PluginMenuStrings = { "Macro Browser" },
  }
  local out = PluginInfo

  local wtype = far.AdvControl("ACTL_GETWINDOWTYPE").Type
  for _,item in ipairs(utils.GetMenuItems()) do
    local flags = item.flags
    local text = (flags.config or flags.disks or flags.plugins and flags[wtype]) and item.text(wtype)
    if type(text) == "string" then
      if flags.config then
        out.PluginConfigStrings = out.PluginConfigStrings or {}
        table.insert(out.PluginConfigStrings, text)
        out.PluginConfigGuids = out.PluginConfigGuids and out.PluginConfigGuids..item.guid or item.guid
      end
      if flags.disks then
        out.DiskMenuStrings = out.DiskMenuStrings or {}
        table.insert(out.DiskMenuStrings, text)
        out.DiskMenuGuids = out.DiskMenuGuids and out.DiskMenuGuids..item.guid or item.guid
      end
      if flags.plugins and flags[wtype] then
        out.PluginMenuStrings = out.PluginMenuStrings or {}
        table.insert(out.PluginMenuStrings, text)
        out.PluginMenuGuids = out.PluginMenuGuids and out.PluginMenuGuids..item.guid or item.guid
      end
    end
  end
  return out
end

local PatSplitMacroString = regex.new([[
^ \s* @ \s*
(?:
    (  " ( [^"]+ ) "  |  [^"\s]+  )
    (?:  \s+ (\S.*)   |  \s*      )
  |
    (.*)
)$
]], "sx")

local function SplitMacroString (Text)
  local c1,c2,c3,c4 = PatSplitMacroString:match(Text)
  if c1 then return c2 or c1, c3; end
  assert(not c4, "Invalid macrosequence specification")
end

local function loadmacro (Lang, Text, Env)
  local _loadstring, _loadfile = loadstring, loadfile
  if Lang == "moonscript" then
    local ms = require "moonscript"
    _loadstring, _loadfile = ms.loadstring, ms.loadfile
  end

  local f1,f2,msg
  local fname,params = SplitMacroString(Text)
  if fname then
    fname = ExpandEnv(fname)
    if params then
      f2,msg = _loadstring("return "..params)
      if not f2 then return nil,msg end
    end
    f1,msg = _loadfile(fname)
  else
    f1,msg = _loadstring(Text)
  end

  if f1 then
    Env = Env or setmetatable({_filename=fname}, gmeta)
    setfenv(f1, Env)
    if f2 then setfenv(f2, Env) end
    return f1,f2
  else
    return nil,msg
  end
end

local function postmacro (f, ...)
  if type(f) == "function" then
    return keymacro.PostNewMacro(pack(f, ...), "", 0, nil, true)
  end
  return false
end

local function MacroInit (Id, Lang, Text)
  local chunk, params
  if type(Id) == "table" then
    if Id.HasFunction then
      chunk, params = Id[1], Id[2] -- макросы, запускаемые посредством MSSC_POST или с командной строки
    else
      chunk, params = Id[1], function() return unpack(Id,2,Id.n) end -- макросы, запускаемые через mf.postmacro
    end
  else -- загруженные или "добавленные" макросы
    local mtable = utils.GetMacroById(Id)
    if mtable then
      chunk = mtable.action
      if not chunk then
        chunk, params = loadmacro(mtable.language or Lang, mtable.code)
      end
    end
  end
  if chunk then
    local macro = { coro=coroutine.create(chunk), params=params, store={} }
    return macro
  else
    ErrMsg(params)
  end
end

local function MacroStep (handle, ...)
  local macro = handle
  if macro then
    local status = co_status(macro.coro)
    if status == "suspended" then
      local ok, ret1, ret_type, ret_values
      if macro.params then
        local params = macro.params
        macro.params = nil
        ok, ret1, ret_type, ret_values = co_resume(macro.coro, params())
      else
        ok, ret1, ret_type, ret_values = co_resume(macro.coro, ...)
      end
      if ok then
        status = co_status(macro.coro)
        if status == "suspended" and ret1 == PROPAGATE and ret_type ~= "exit" then
          macro.store[1] = ret_values
          if ret_type==F.MPRT_PLUGINCALL or ret_type==F.MPRT_PLUGINMENU or ret_type==F.MPRT_PLUGINCONFIG or
             ret_type==F.MPRT_PLUGINCOMMAND or ret_type==F.MPRT_USERMENU then
            return ret_type, ret_values
          else
            return ret_type, macro.store
          end
        else
          LastMessage[1] = ""
          return F.MPRT_NORMALFINISH
        end
      else
        ret1 = type(ret1)=="string" and ret1 or "(error object is not a string)"
        ret1 = debug.traceback(macro.coro, ret1):gsub("\n\t","\n   ")
        ErrMsg(ret1)
        LastMessage[1] = ret1
        return F.MPRT_ERRORFINISH, LastMessage
      end
    else
      ErrMsg("Step: called on macro in "..status.." status")
    end
  else
    -- Far debug only: should not be here
    ErrMsg(("Step: handle %d does not exist"):format(handle))
  end
end

local function MacroParse (Lang, Text, onlyCheck, skipFile)
  local _loadstring, _loadfile = loadstring, loadfile
  if Lang == "moonscript" then
    local ms = require "moonscript"
    _loadstring, _loadfile = ms.loadstring, ms.loadfile
  end

  local ok,msg = true,nil
  local fname,params = SplitMacroString(Text)
  if fname then
    if params then
      ok,msg = _loadstring("return "..params)
    end
    if ok and not skipFile then
      ok,msg = _loadfile(ExpandEnv(fname))
    end
  else
    ok,msg = _loadstring(Text)
  end

  if not ok then
    if not onlyCheck then
      far.Message(msg, Msg.MMacroPErrorTitle, Msg.MOk, "lw")
    end
    LastMessage = pack(
      msg, -- keep alive from gc
      tonumber(msg:match(":(%d+): ")) or 0)
    return F.MPRT_ERRORPARSE, LastMessage
  end

  return F.MPRT_NORMALFINISH
end

local function ExecString (lang, text, params)
  if type(text)=="string" then
    local chunk, msg = loadmacro(lang, text)
    if chunk then
      TableExecString = pack(chunk(unpack(params,1,params.n)))
      return F.MPRT_NORMALFINISH, TableExecString
    else
      ErrMsg(msg)
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

local function ProcessCommandLine (CmdLine)
  local prefix, text = CmdLine:match("^%s*(%w+):%s*(.-)%s*$")
  prefix = prefix:lower()
  if prefix == "lm" or prefix == "macro" then
    local cmd = text:match("%S*"):lower()
    if cmd == "load" then far.MacroLoadAll()
    elseif cmd == "save" then utils.WriteMacros()
    elseif cmd == "unload" then utils.UnloadMacros()
    elseif cmd == "post" then -- DEPRECATED, to be removed on 2014-Oct-29.
      prefix, text = "lua", text:match("%S+%s*(.*)")
    elseif cmd == "about" then About()
    elseif cmd ~= "" then ErrMsg(Msg.CL_UnsupportedCommand .. cmd) end
  end
  if prefix == "lua" or prefix == "moon" then
    if text~="" then
      if text:find("^=") then
        text = "far.Show(" .. text:sub(2) .. ")"
      else
        local fname, params = SplitMacroString(text)
        if fname then
          fname = ExpandEnv(fname)
          fname = far.ConvertPath(fname, F.CPM_NATIVE)
          if fname:find("%s") then fname = '"'..fname..'"' end
          text = "@"..fname
          if params then text = text.." "..params end
        end
      end
      local f1,f2 = loadmacro(prefix=="lua" and "lua" or "moonscript", text)
      if f1 then keymacro.PostNewMacro({ f1,f2,HasFunction=true }, "", 0, nil, true)
      else ErrMsg(f2)
      end
    end
  end
end

function export.Open (OpenFrom, arg1, ...)
  if OpenFrom == F.OPEN_LUAMACRO then
    local calltype = arg1
    if     calltype==F.MCT_KEYMACRO       then return keymacro.Dispatch(...)
    elseif calltype==F.MCT_MACROPARSE     then return MacroParse(...)
    elseif calltype==F.MCT_DELMACRO       then return utils.DelMacro(...)
    elseif calltype==F.MCT_ENUMMACROS     then return utils.EnumMacros(...)
    elseif calltype==F.MCT_GETMACRO       then return utils.GetMacroWrapper(...)
    elseif calltype==F.MCT_LOADMACROS     then
      local InitedRAM = ...
      keymacro.InitInternalVars(InitedRAM)
      return utils.LoadMacros()
    elseif calltype==F.MCT_RECORDEDMACRO  then return utils.ProcessRecordedMacro(...)
    elseif calltype==F.MCT_RUNSTARTMACRO  then return utils.RunStartMacro()
    elseif calltype==F.MCT_WRITEMACROS    then return utils.WriteMacros()
    elseif calltype==F.MCT_EXECSTRING     then return ExecString(...)
    elseif calltype==F.MCT_ADDMACRO       then return utils.AddMacroFromFAR(...)
    elseif calltype==F.MCT_PANELSORT      then
      if panelsort then
        TablePanelSort = { panelsort.SortPanelItems(...) }
        if TablePanelSort[1] then return F.MPRT_NORMALFINISH, TablePanelSort end
      end
    elseif calltype==F.MCT_GETCUSTOMSORTMODES then
      if panelsort then
        TablePanelSort = panelsort.GetSortModes()
        return F.MPRT_NORMALFINISH, TablePanelSort
      end
    end

  elseif OpenFrom == F.OPEN_COMMANDLINE then
    local guid, cmdline =  arg1, ...
    return ProcessCommandLine(cmdline)

  elseif OpenFrom == F.OPEN_FROMMACRO then
    local guid, args =  arg1, ...
    if args[1]=="argtest" then -- argtest: return received arguments
      return unpack(args,2,args.n)
    elseif args[1]=="macropost" then -- test Mantis # 2222
      return far.MacroPost([[far.Message"macropost"]])
    end

  else
    local items = utils.GetMenuItems()
    if items[arg1] then
      items[arg1].action(OpenFrom, ...)
    else
      macrobrowser()
    end

  end
end

function export.Configure (guid)
  local items = utils.GetMenuItems()
  if items[guid] then items[guid].action() end
end

-- Add function unicode.utf8.cfind:
-- same as find, but offsets are in characters rather than bytes
local function AddCfindFunction()
  local usub, ssub = unicode.utf8.sub, string.sub
  local ulen, slen = unicode.utf8.len, string.len
  local ufind = unicode.utf8.find
  unicode.utf8.cfind = function(s, patt, init, plain)
    init = init and slen(usub(s, 1, init-1)) + 1
    local t = { ufind(s, patt, init, plain) }
    if t[1] == nil then return nil end
    return ulen(ssub(s, 1, t[1]-1)) + 1, ulen(ssub(s, 1, t[2])), unpack(t, 3)
  end
end

local function Init()
  local Shared = { ErrMsg=ErrMsg, pack=pack, checkarg=checkarg, loadmacro=loadmacro, yieldcall=yieldcall,
                   MacroInit=MacroInit, MacroStep=MacroStep }

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

  keymacro = RunPluginFile("keymacro.lua", Shared)
  Shared.keymacro = keymacro
  mf.mmode, _G.mmode = keymacro.mmode, keymacro.mmode

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

  utils.InitMacroSystem()
  AddCfindFunction()
  local modules = win.GetEnv("farprofile").."\\Macros\\modules\\"
  package.path = modules.."?.lua;"..modules.."?\\init.lua;"..package.path
  package.moonpath = modules.."?.moon;"..modules.."?\\init.moon;"..package.moonpath
  package.cpath = modules.."?.dll;"..package.cpath

  if _G.IsLuaStateRecreated then
    _G.IsLuaStateRecreated = nil
    utils.LoadMacros()
  end
end

local ok, msg = pcall(Init) -- pcall is used to handle RunPluginFile() failure in one place only
if not ok then
  export=nil; ErrMsg(msg)
end
