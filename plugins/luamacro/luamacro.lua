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

-- A unique value, inaccessible to scripts.
local PROPAGATE={}

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

local MacroTable = {}
local LastMessage = {}
local gmeta = { __index=_G }

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

local function PluginCall    (...) return co_yield(PROPAGATE, F.MPRT_PLUGINCALL,    pack(...)) end
local function PluginMenu    (...) return co_yield(PROPAGATE, F.MPRT_PLUGINMENU,    pack(...)) end
local function PluginConfig  (...) return co_yield(PROPAGATE, F.MPRT_PLUGINCONFIG,  pack(...)) end
local function PluginCommand (...) return co_yield(PROPAGATE, F.MPRT_PLUGINCOMMAND, pack(...)) end

function _G.exit ()
  co_yield(PROPAGATE, "exit")
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

local function MacroInit (Text)
  if type(Text)=="string" then
    local chunk, msg = loadmacro(Text)
    if chunk then
      local env = setmetatable({}, gmeta)
      setfenv(chunk, env)
      local macro = { coro=co_create(chunk), store={} }
      table.insert(MacroTable, macro)
      --far.Message("Init: created handle "..#MacroTable)
      return #MacroTable
    else
      ErrMsg(msg)
    end
  end
end

local function MacroStep (handle, ...)
  local macro = MacroTable[handle]
  if macro then
    local status = co_status(macro.coro)
    if status == "suspended" then
      local ok, ret1, ret_type, ret_values = co_resume(macro.coro, ...)
      if ok then
        status = co_status(macro.coro)
        if status == "suspended" and ret1 == PROPAGATE and ret_type ~= "exit" then
          macro.store[1] = ret_values
          if ret_type==F.MPRT_PLUGINCALL or ret_type==F.MPRT_PLUGINMENU or
             ret_type==F.MPRT_PLUGINCONFIG or ret_type==F.MPRT_PLUGINCOMMAND then
            return ret_type, ret_values
          else
            return ret_type, macro.store
          end
        else
          MacroTable[handle] = false
          LastMessage[1] = ""
          return F.MPRT_NORMALFINISH, LastMessage
        end
      else
        ErrMsg(ret1)
        MacroTable[handle] = false
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
  if MacroTable[handle] then
    MacroTable[handle] = false -- false, not nil!
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
    elseif op=="save" then far.MacroSaveAll()
    end
  end
end

function export.Open (OpenFrom, ...)
  if OpenFrom == F.OPEN_LUAMACRO then
    local calltype, handle, args = ...
    if     calltype==F.MCT_MACROINIT  then return MacroInit (args[1])
    elseif calltype==F.MCT_MACROSTEP  then return MacroStep (handle, unpack(args))
    elseif calltype==F.MCT_MACROFINAL then return MacroFinal(handle)
    elseif calltype==F.MCT_MACROPARSE then return MacroParse(unpack(args))
    end
  elseif OpenFrom == F.OPEN_COMMANDLINE then
    local guid, cmdline = ...
    return ProcessCommandLine(cmdline)
  end
end

local function ReadVarsConsts (region)
  while true do
    local sName,sValue,sType = far.MacroCallFar(0x80C65,region)
    if not sName then break end
    if _G[sName] == nil then -- protect existing globals
      if     sType=="text"    then _G[sName]=sValue
      elseif sType=="real"    then _G[sName]=tonumber(sValue)
      elseif sType=="integer" then _G[sName]=bit64.new(sValue)
      end
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
  local func,msg = loadfile(far.PluginStartupInfo().ModuleDir.."api.lua")
  if func then
    func { checkarg=checkarg, loadmacro=loadmacro }
    Plugin.Call=PluginCall
    Plugin.Menu=PluginMenu
    Plugin.Config=PluginConfig
    Plugin.Command=PluginCommand
  else
    ErrMsg(msg)
  end

  AddCfindFunction()
  ReadVarsConsts("consts")
  ReadVarsConsts("vars")
end
