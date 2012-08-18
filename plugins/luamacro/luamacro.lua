-- started: 2012-04-20

local function LOG (fmt, ...)
  local log = io.open("c:\\lua.log","at")
  if log then
    log:write("LUA: ", fmt:format(...), "\n")
    log:close()
  end
end

local F = far.Flags

local co_create, co_yield, co_resume, co_status =
  coroutine.create, coroutine.yield, coroutine.resume, coroutine.status

local ErrMsg = function(msg) far.Message(msg, "LuaMacro", nil, "w") end

local macros = {}
local LastError
local gmeta = { __index=_G }

--FIXME: function duplicated in luamacro.lua and api.lua.
local function checkarg (arg, argnum, reftype)
  if type(arg) ~= reftype then
    error(("arg. #%d: %s expected, got %s"):format(argnum, reftype, type(arg)), 3)
  end
end

function _G.Keys (...)
  for n=1,select("#",...) do
    local str=select(n,...)
    if type(str)=="string" then
      for key in str:gmatch("%S+") do co_yield(key) end
    end
  end
end

function _G.print (str)
  str = tostring(str)
  co_yield("print:"..str)
end

function _G.printf (fmt, ...)
  checkarg(fmt,1,"string")
  return _G.print(fmt:format(...))
end

local PluginInfo = {
  Flags = F.PF_PRELOAD,
}
function export.GetPluginInfo()
  return PluginInfo
end

local function MacroInit (args)
  local Text, AKey = args[1], args[2]
  if Text and Text.Type==F.FMVT_STRING then
    Text = Text.Value
    local chunk, msg
    if string.sub(Text,1,1) == "@" then
      Text = string.sub(Text,2):gsub("%%(.-)%%", win.GetEnv)
      chunk, msg = loadfile(Text)
    else
      chunk, msg = loadstring(Text)
    end
    if chunk then
      local env = setmetatable({}, gmeta)
      setfenv(chunk, env)
      local macro = { coro=co_create(chunk), step=0 }
      if AKey and AKey.Type==F.FMVT_STRING then env.AKey = AKey.Value end
      table.insert(macros, macro)
      return #macros
    else
      ErrMsg(msg)
    end
  end
end

local function MacroStep (handle)
  local macro = macros[handle]
  if macro then
    local status = co_status(macro.coro)
    if status == "suspended" then
      local ok, ret = co_resume(macro.coro)
      if ok then
        macro.step = macro.step + 1
        status = co_status(macro.coro)
        if status == "suspended" and type(ret) == "string" then
          macro.lastkey = win.Utf8ToUtf16(ret).."\0\0" -- keep alive from gc
        else
          macro.lastkey = "\0\0"
        end
        return macro.lastkey
      else
        ErrMsg(ret)
      end
    else
      ErrMsg("Step: called on macro in "..status.." status")
    end
  else
    -- Far debug only: should not be here
    ErrMsg("Step: handle does not exist")
  end
end

local function MacroFinal (handle)
  if macros[handle] then
    macros[handle] = false -- false, not nil!
    return 1
  else
    -- Far debug only: should not be here
    ErrMsg("Final: handle does not exist")
  end
end

local function MacroParse (args)
  local text, onlyCheck, title, buttons = args[1], args[2], args[3], args[4]
  if string.sub(text.Value,1,1) ~= "@" then
    local chunk, msg = loadstring(text.Value)
    if not chunk then
      if onlyCheck.Value == 0 then
        far.Message(msg, title.Value, buttons.Value, "lw")
      end
      LastError = win.Utf8ToUtf16(msg).."\0\0" -- keep alive from gc
      return LastError
    end
  end
  LastError = win.Utf8ToUtf16("OK\0")
  return LastError
end

function export.Open (OpenFrom, Guid, Item)
  if     OpenFrom == F.OPEN_MACROINIT  then return MacroInit(Item)
  elseif OpenFrom == F.OPEN_MACROSTEP  then return MacroStep(Item)
  elseif OpenFrom == F.OPEN_MACROFINAL then return MacroFinal(Item)
  elseif OpenFrom == F.OPEN_MACROPARSE then return MacroParse(Item)
  end
end

do
  local func,msg = loadfile(far.PluginStartupInfo().ModuleDir.."api.lua")
  if func then func() else ErrMsg(msg) end
end
