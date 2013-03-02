-- started: 2012-04-20

local function LOG (fmt, ...)
  local log = io.open("c:\\lua.log","at")
  if log then
    log:write("LUA: ", fmt:format(...), "\n")
    log:close()
  end
end

local F = far.Flags
local co_create, co_yield, co_resume, co_status, co_wrap =
  coroutine.create, coroutine.yield, coroutine.resume, coroutine.status, coroutine.wrap

local PROPAGATE={} -- a unique value, inaccessible to scripts.
local gmeta = { __index=_G }
local RunningMacros = {}
local LastMessage = {}
local utils

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
  Flags = F.PF_PRELOAD,
  CommandPrefix = "lm",
}
function export.GetPluginInfo()
  return PluginInfo
end

local function loadmacro (Text)
  if string.sub(Text,1,1) == "@" then
    Text = string.sub(Text,2):gsub("%%(.-)%%", win.GetEnv)
    return loadfile(Text)
  else
    return loadstring(Text)
  end
end

local function MacroInit (Id, Text)
  local chunk, msg
  if Id == 0 then -- Id==0 может быть только для "одноразовых" макросов, запускаемых посредством MSSC_POST.
    chunk, msg = loadmacro(Text)
  else
    local mtable = utils.GetMacroById(Id)
    if mtable then
      chunk = mtable.action
      if not chunk then chunk, msg = loadmacro(mtable.code) end
    end
  end
  if chunk then
    if Id == 0 then
      local env = setmetatable({}, gmeta)
      setfenv(chunk, env)
    end
    local macro = { coro=co_create(chunk), store={} }
    table.insert(RunningMacros, macro)
    return #RunningMacros
  else
    ErrMsg(msg)
  end
end

local function MacroStep (handle, ...)
  local macro = RunningMacros[handle]
  if macro then
    local status = co_status(macro.coro)
    if status == "suspended" then
      local ok, ret1, ret_type, ret_values = co_resume(macro.coro, ...)
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
        --ErrMsg(ret1)
        ErrMsg(debug.traceback(macro.coro, ret1))
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

local function ProcessCommandLine (CmdLine)
  local op, text = CmdLine:match("(%S+)%s*(.*)")
  if op then
    local op = op:lower()
    if     op=="post"  and text~="" then far.MacroPost(text, F.KMFLAGS_DISABLEOUTPUT)
    elseif op=="check" and text~="" then far.MacroCheck(text)
    elseif op=="load" then far.MacroLoadAll()
    elseif op=="save" then utils.WriteMacros()
    elseif op=="unload" then utils.UnloadMacros()
    end
  end
end

function export.Open (OpenFrom, ...)
  if OpenFrom == F.OPEN_LUAMACRO then
    local calltype, handle, args = ...
    if     calltype==F.MCT_MACROINIT      then return MacroInit (unpack(args))
    elseif calltype==F.MCT_MACROSTEP      then return MacroStep (handle, unpack(args))
    elseif calltype==F.MCT_MACROFINAL     then return MacroFinal(handle)
    elseif calltype==F.MCT_MACROPARSE     then return MacroParse(unpack(args))
    elseif calltype==F.MCT_DELMACRO       then return utils.DelMacro(unpack(args))
    elseif calltype==F.MCT_ENUMMACROS     then return utils.EnumMacros(unpack(args))
    elseif calltype==F.MCT_GETMACRO       then return utils.GetMacroWrapper(args)
    elseif calltype==F.MCT_LOADMACROS     then return utils.LoadMacros(unpack(args))
    elseif calltype==F.MCT_PROCESSMACRO   then return utils.ProcessMacroFromFAR(unpack(args))
    elseif calltype==F.MCT_RUNSTARTMACRO  then return utils.RunStartMacro()
    elseif calltype==F.MCT_WRITEMACROS    then return utils.WriteMacros()
    end

  elseif OpenFrom == F.OPEN_COMMANDLINE then
    local guid, cmdline = ...
    return ProcessCommandLine(cmdline)

  elseif OpenFrom == F.OPEN_FROMMACRO then
    local guid, args = ...
    if args[1]=="argtest" then -- argtest: return received arguments
      return unpack(args,2)
    elseif args[1]=="macropost" then -- this test fails (Mantis # 2222)
      return far.MacroPost([[far.Message"macropost"]])
    end
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
  local func,msg = loadfile(far.PluginStartupInfo().ModuleDir.."utils.lua")
  if func then
    utils = func { ErrMsg=ErrMsg, pack=pack }
  else
    export={}; ErrMsg(msg); return
  end

  local func,msg = loadfile(far.PluginStartupInfo().ModuleDir.."api.lua")
  if func then
    func { utils=utils, checkarg=checkarg, loadmacro=loadmacro, yieldcall=yieldcall }
  else
    export={}; ErrMsg(msg); return
  end

  AddCfindFunction()
  package.path = win.GetEnv("farprofile").."\\Macros\\modules\\?.lua;"..package.path
end
