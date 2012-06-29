-- started: 2012-04-20

local F = far.Flags
F.OPEN_MACROINIT, F.OPEN_MACROSTEP, F.OPEN_MACROFINAL = 100,101,102

local co_create, co_yield, co_resume, co_status =
  coroutine.create, coroutine.yield, coroutine.resume, coroutine.status

local ErrMsg = function(msg) far.Message(msg, "LuaMacro", nil, "w") end

local macros = {}

function far.Keys (str)
  assert(type(str) == "string", "arg. #1 to far.Keys must be string")
  for key in str:gmatch("%S+") do co_yield(key) end
end

local PluginInfo = {
  Flags = F.PF_PRELOAD,
}
function export.GetPluginInfo()
  return PluginInfo
end

local function MacroInit (args)
  local Text, AKey, Flags = args[1], args[2], args[3]
  if Text and Text.Type==F.FMVT_STRING then
    Text = Text.Value
    local chunk, msg
    if Text:sub(1,1) == "@" then chunk, msg = loadfile(Text:sub(2))
    else                         chunk, msg = loadstring(Text)
    end
    if chunk then
      local macro = { coro=co_create(chunk), step=0 }
      if AKey and AKey.Type==F.FMVT_STRING then macro.AKey = AKey.Value end
      if Flags and Flags.Type==F.FMVT_INTEGER then macro.Flags = Flags.Value end
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
      local ok, ret
      if macro.step == 0 then
        ok, ret = co_resume(macro.coro, macro.AKey, macro.Flags)
      else
        ok, ret = co_resume(macro.coro)
      end
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
      ErrMsg("Step: step called on finished macro")
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

function export.Open (OpenFrom, Guid, Item)
  if     OpenFrom == F.OPEN_MACROINIT  then return MacroInit(Item)
  elseif OpenFrom == F.OPEN_MACROSTEP  then return MacroStep(Item)
  elseif OpenFrom == F.OPEN_MACROFINAL then return MacroFinal(Item)
  end
end
