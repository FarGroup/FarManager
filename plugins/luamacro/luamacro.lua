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

local function MacroInit (args)
  local Text = args[1]
  if Text and type(Text)=="string" then
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

local function MacroStep (args)
  local handle = args[1]
  local macro = MacroTable[handle]
  if macro then
    local status = co_status(macro.coro)
    if status == "suspended" then
      local ok, ret1, ret2, ret3 = co_resume(macro.coro, unpack(args, 2))
      if ok then
        status = co_status(macro.coro)
        if status == "suspended" and ret1 == PROPAGATE and ret2 ~= "exit" then
          macro.store[1] = ret3
          if ret2==F.MPRT_PLUGINCALL or ret2==F.MPRT_PLUGINMENU or
             ret2==F.MPRT_PLUGINCONFIG or ret2==F.MPRT_PLUGINCOMMAND then
            return ret2, ret3
          else
            return ret2, macro.store
          end
        else
          MacroTable[handle] = false
          LastMessage[1] = "OK"
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

local function MacroFinal (args)
  local handle = args[1]
  if MacroTable[handle] then
    MacroTable[handle] = false -- false, not nil!
    --far.Message("Final: closed handle "..handle)
    return 1
  else
    -- Far debug only: should not be here
    ErrMsg(("Final: handle %d does not exist"):format(handle))
  end
end

local function MacroParse (args)
  local text, onlyCheck, skipFile, title, buttons = unpack(args)
  local isFile = string.sub(text,1,1) == "@"
  if not (isFile and skipFile) then
    local chunk, msg = loadmacro(text)
    if not chunk then
      if not onlyCheck then
        far.Message(msg, title, buttons, "lw")
      end
      LastMessage[1] = msg -- keep alive from gc
      return F.MPRT_ERRORFINISH, LastMessage
    end
  end
  LastMessage[1] = "OK"
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

function export.Open (OpenFrom, Guid, Item)
  if     OpenFrom == F.MCT_MACROINIT    then return MacroInit(Item)
  elseif OpenFrom == F.MCT_MACROSTEP    then return MacroStep(Item)
  elseif OpenFrom == F.MCT_MACROFINAL   then return MacroFinal(Item)
  elseif OpenFrom == F.MCT_MACROPARSE   then return MacroParse(Item)
  elseif OpenFrom == F.OPEN_COMMANDLINE then return ProcessCommandLine(Item)
  end
end

local function ReadVarsConsts (region)
  while true do
    local sName,sValue,sType = far.MacroCallFar(0x80007,region)
    if not sName then break end
    if _G[sName] == nil then -- protect existing globals
      if     sType=="text"    then _G[sName]=sValue
      elseif sType=="real"    then _G[sName]=tonumber(sValue)
      elseif sType=="integer" then _G[sName]=bit64.new(sValue)
      end
    end
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

  ReadVarsConsts("consts")
  ReadVarsConsts("vars")
end
