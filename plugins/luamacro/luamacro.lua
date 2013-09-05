-- started: 2012-04-20

local function LOG (fmt, ...)
  local log = io.open("c:\\lua.log","at")
  if log then
    log:write("LUA: ", fmt:format(...), "\n")
    log:close()
  end
end

local F, M = far.Flags, nil
local bor = bit64.bor
local co_create, co_yield, co_resume, co_status, co_wrap =
  coroutine.create, coroutine.yield, coroutine.resume, coroutine.status, coroutine.wrap

local PROPAGATE={} -- a unique value, inaccessible to scripts.
local gmeta = { __index=_G }
local RunningMacros = {}
local LastMessage = {}
local LastSortModes -- must be separate from LastMessage, otherwise Far crashes after a macro is called from CtrlF12.
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

local ErrMsg = function(msg) far.Message(msg, "LuaMacro", nil, "wl") end

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
  Flags = bor(F.PF_PRELOAD,F.PF_EDITOR,F.PF_VIEWER,F.PF_DIALOG),
  CommandPrefix = "lm",
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

local function loadmacro (Text)
  local fname, params = SplitMacroString(Text)
  if fname then
    if params then
      local f2, msg = loadstring("return "..params)
      if not f2 then return nil, msg end
      local f1, msg = loadfile(ExpandEnv(fname))
      if not f1 then return nil, msg end
      return f1, f2
    else
      return loadfile(ExpandEnv(fname))
    end
  else
    return loadstring(Text)
  end
end

local function MacroInit (Id, Text)
  local chunk, params
  if Id == 0 then -- Id==0 может быть только для "одноразовых" макросов, запускаемых посредством MSSC_POST.
    chunk, params = loadmacro(Text)
  else
    local mtable = utils.GetMacroById(Id)
    if mtable then
      chunk = mtable.action
      if not chunk then chunk, params = loadmacro(mtable.code) end
    end
  end
  if chunk then
    if Id == 0 then
      local env = setmetatable({}, gmeta)
      setfenv(chunk, env)
    end
    if params then setfenv(params, getfenv(chunk)) end
    local macro = { coro=co_create(chunk), params=params, store={} }
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
          return F.MPRT_NORMALFINISH, LastMessage
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
    --far.Message("Final: closed handle "..handle)
    return 1
  else
    -- Far debug only: should not be here
    ErrMsg(("Final: handle %d does not exist"):format(handle))
  end
end

local function MacroParse (text, onlyCheck, skipFile, title, buttons)
  local isFile = string.sub(text,1,1) == "@"
  if not (isFile and skipFile) then
    local chunk, msg = loadmacro(text)
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
  return F.MPRT_NORMALFINISH, LastMessage
end

local function ExecString (text, flags, params)
  if type(text)=="string" then
    local chunk, msg = loadmacro(text)
    if chunk then
      local env = setmetatable({},{__index=_G})
      LastMessage = pack(setfenv(chunk, env)(unpack(params,1,params.n)))
      return F.MPRT_COMMONCASE, LastMessage
    else
      far.Message(msg, "LuaMacro", nil, "wl")
      LastMessage = pack(msg)
      return F.MPRT_ERRORPARSE, LastMessage
    end
  end
end

local function ProcessCommandLine (CmdLine)
  local op, text = CmdLine:match("(%S+)%s*(.-)%s*$")
  if op then
    local op = op:lower()
    if op=="post" and text~="" then
      local fname, params = SplitMacroString(text)
      if fname then
        fname = ExpandEnv(fname)
        fname = far.ConvertPath(fname, F.CPM_NATIVE)
        if fname:find("%s") then fname = '"'..fname..'"' end
        text = "@"..fname
        if params then text = text.." "..params end
      end
      far.MacroPost(text)
    elseif op=="check" and text~="" then far.MacroCheck(text)
    elseif op=="load" then far.MacroLoadAll()
    elseif op=="save" then utils.WriteMacros()
    elseif op=="unload" then utils.UnloadMacros()
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
    elseif calltype==F.MCT_PROCESSMACRO   then return utils.ProcessMacroFromFAR(...)
    elseif calltype==F.MCT_RUNSTARTMACRO  then return utils.RunStartMacro()
    elseif calltype==F.MCT_WRITEMACROS    then return utils.WriteMacros()
    elseif calltype==F.MCT_EXECSTRING     then return ExecString(...)
    elseif calltype==F.MCT_PANELSORT      then
      if panelsort then
        LastMessage = pack(panelsort.SortPanelItems(...))
        if LastMessage[1] then return F.MPRT_COMMONCASE, LastMessage end
      end
    elseif calltype==F.MCT_GETCUSTOMSORTMODES then
      if panelsort then
        LastSortModes = panelsort.GetSortModes()
        return F.MPRT_COMMONCASE, LastSortModes
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

  macrobrowser = RunPluginFile("mbrowser.lua", Shared)
  if not macrobrowser then return end

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
