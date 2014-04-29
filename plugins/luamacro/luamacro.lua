-- started: 2012-04-20

local function LOG (fmt, ...)
  local log = io.open("c:\\lua.log","at")
  if log then
    log:write("LUA: ", fmt:format(...), "\n")
    log:close()
  end
end

local MCODE_F_POSTNEWMACRO = 0x80C64
local F, M = far.Flags, nil
local bor = bit64.bor
local co_yield, co_resume, co_status, co_wrap =
  coroutine.yield, coroutine.resume, coroutine.status, coroutine.wrap

local PROPAGATE={} -- a unique value, inaccessible to scripts.
local gmeta = { __index=_G }
local RunningMacros = {}
local PostedMacros = {}
local LastMessage = {}
local TablePanelSort -- must be separate from LastMessage, otherwise Far crashes after a macro is called from CtrlF12.
local TableExecString -- must be separate from LastMessage, otherwise Far crashes
local utils, macrobrowser, panelsort

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

local ErrMsg = function(msg,title) far.Message(msg,title or "LuaMacro",nil,"wl") end

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

function _G.print (str)
  co_yield(PROPAGATE, F.MPRT_PRINT, tostring(str))
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

local PluginInfo = {
  Flags = bor(F.PF_PRELOAD,F.PF_FULLCMDLINE,F.PF_EDITOR,F.PF_VIEWER,F.PF_DIALOG),
  CommandPrefix = "lm:macro:lua:moon",
  PluginMenuGuids = win.Uuid("EF6D67A2-59F7-4DF3-952E-F9049877B492"),
  PluginMenuStrings = { "Macro Browser" },
}
function export.GetPluginInfo()
  return PluginInfo
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
  local f1,f2,msg
  local fname,params = SplitMacroString(Text)
  local _loadstring, _loadfile = loadstring, loadfile
  if Lang == "moonscript" then
    local ms = require "moonscript"
    _loadstring, _loadfile = ms.loadstring, ms.loadfile
  end

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
    local Id = #PostedMacros+1
    PostedMacros[Id] = pack(f, ...)
    return far.MacroCallFar(MCODE_F_POSTNEWMACRO, -Id, "", 0)
  end
  return false
end

local function MacroInit (Id, Lang, Text)
  local chunk, params
  if Id == 0 then -- Id==0 может быть только для "одноразовых" макросов, запускаемых посредством MSSC_POST.
    chunk, params = loadmacro(Lang, Text)
  elseif Id < 0 then -- Id<0 может быть только для макросов, запускаемых посредством mf.postmacro.
    local mtable = PostedMacros[-Id]
    PostedMacros[-Id] = false
    chunk = mtable[1]
    params = function() return unpack(mtable, 2, mtable.n) end
  else
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
    table.insert(RunningMacros, macro)
    return #RunningMacros
  else
    ErrMsg(params)
  end
end

local function MacroStep (handle, ...)
  local macro = RunningMacros[handle]
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
          RunningMacros[handle] = false
          LastMessage[1] = ""
          return F.MPRT_NORMALFINISH
        end
      else
        ret1 = type(ret1)=="string" and ret1 or "(error object is not a string)"
        ret1 = debug.traceback(macro.coro, ret1):gsub("\n\t","\n   ")
        ErrMsg(ret1)
        RunningMacros[handle] = false
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

local function MacroFinal (handle)
  if RunningMacros[handle] then
    RunningMacros[handle] = false -- false, not nil!
    --LOG("Final: closed handle "..handle)
    return 1
  else
    -- Far debug only: should not be here
    ErrMsg(("Final: handle %d does not exist"):format(handle))
  end
end

local function MacroParse (lang, text, onlyCheck, skipFile, title, buttons)
  local isFile = string.sub(text,1,1) == "@"
  if not (isFile and skipFile) then
    local chunk, msg = loadmacro(lang, text)
    if not chunk then
      if not onlyCheck then
        far.Message(msg, title, buttons, "lw")
      end
      LastMessage = pack(
        msg, -- keep alive from gc
        tonumber(msg:match(":(%d+): ")) or 0)
      return F.MPRT_ERRORPARSE, LastMessage
    end
  end
  LastMessage[1] = ""
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

local function ProcessCommandLine (CmdLine)
  local op, text = CmdLine:match("^%s*(%w+:)%s*(.-)%s*$")
  op = op:lower()
  if op == "lm:" or op == "macro:" then
    local cmd = text:match("%S*"):lower()
    if cmd == "load" then far.MacroLoadAll()
    elseif cmd == "save" then utils.WriteMacros()
    elseif cmd == "unload" then utils.UnloadMacros()
    elseif cmd == "post" then -- DEPRECATED, to be removed on 2014-Oct-29.
      op, text = "lua:", text:match("%S+%s*(.*)")
    elseif cmd ~= "" then ErrMsg(M.CL_UnsupportedCommand .. cmd) end
  end
  if op == "lua:" or op == "moon:" then
    if text~="" then
      local fname, params = SplitMacroString(text)
      if fname then
        fname = ExpandEnv(fname)
        fname = far.ConvertPath(fname, F.CPM_NATIVE)
        if fname:find("%s") then fname = '"'..fname..'"' end
        text = "@"..fname
        if params then text = text.." "..params end
      end
      far.MacroPost(text, op=="lua:" and "KMFLAGS_LUA" or "KMFLAGS_MOONSCRIPT")
    end
  end
end

function export.Open (OpenFrom, arg1, arg2, ...)
  if OpenFrom == F.OPEN_LUAMACRO then
    local calltype, handle = arg1, arg2
    if     calltype==F.MCT_MACROINIT      then return MacroInit (...)
    elseif calltype==F.MCT_MACROSTEP      then return MacroStep (handle, ...)
    elseif calltype==F.MCT_MACROFINAL     then return MacroFinal(handle)
    elseif calltype==F.MCT_MACROPARSE     then return MacroParse(...)
    elseif calltype==F.MCT_DELMACRO       then return utils.DelMacro(...)
    elseif calltype==F.MCT_ENUMMACROS     then return utils.EnumMacros(...)
    elseif calltype==F.MCT_GETMACRO       then return utils.GetMacroWrapper(...)
    elseif calltype==F.MCT_LOADMACROS     then return utils.LoadMacros(...)
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
    local guid, cmdline =  arg1, arg2
    return ProcessCommandLine(cmdline)

  elseif OpenFrom == F.OPEN_FROMMACRO then
    local guid, args =  arg1, arg2
    if args[1]=="argtest" then -- argtest: return received arguments
      return unpack(args,2,args.n)
    elseif args[1]=="macropost" then -- test Mantis # 2222
      return far.MacroPost([[far.Message"macropost"]])
    end

  else
    macrobrowser()

  end
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

do
  local Shared = { ErrMsg=ErrMsg, pack=pack, checkarg=checkarg, loadmacro=loadmacro, yieldcall=yieldcall }

  local ModuleDir = far.PluginStartupInfo().ModuleDir
  local function RunPluginFile (fname, param)
    local func,msg = loadfile(ModuleDir..fname)
    if func then return func(param) or true
    else export=nil; ErrMsg(msg)
    end
  end

  M = RunPluginFile("lang.lua");
  if not M then return end
  Shared.M = M

  utils = RunPluginFile("utils.lua", Shared)
  if not utils then return end
  Shared.utils = utils

  if not RunPluginFile("api.lua", Shared) then return end
  mf.postmacro = postmacro

  macrobrowser = RunPluginFile("mbrowser.lua", Shared)
  if not macrobrowser then return end

  if not RunPluginFile("moonscript.lua", Shared) then return end

  if bit and jit then
    if not RunPluginFile("winapi.lua") then return end
    if not RunPluginFile("farapi.lua") then return end

    panelsort = RunPluginFile("panelsort.lua", Shared)
    if not panelsort then return end
    Shared.panelsort = panelsort
    Panel.LoadCustomSortMode = panelsort.LoadCustomSortMode
    Panel.SetCustomSortMode = panelsort.SetCustomSortMode
    Panel.CustomSortMenu = panelsort.CustomSortMenu
  end

  AddCfindFunction()
  local modules = win.GetEnv("farprofile").."\\Macros\\modules"
  package.path = ("%s\\?.lua;%s\\?\\init.lua;%s"):format(modules,modules,package.path)
end
