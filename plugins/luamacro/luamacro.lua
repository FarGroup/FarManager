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
local Areas
local LoadedMacros
local Enum_LastIndexes = {}

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

local function MacroInit (Id, Text)
  local chunk, msg
  if Id == 0 then
    chunk, msg = loadmacro(Text)
  else
    local mtable = LoadedMacros[Id]
    if mtable then chunk = mtable.action
    end
  end
  if chunk then
    if Id == 0 then
      local env = setmetatable({}, gmeta)
      setfenv(chunk, env)
    end
    local macro = { coro=co_create(chunk), store={} }
    table.insert(RunningMacros, macro)
    --far.Message("Init: created handle "..#RunningMacros)
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
          if ret_type==F.MPRT_PLUGINCALL or ret_type==F.MPRT_PLUGINMENU or
             ret_type==F.MPRT_PLUGINCONFIG or ret_type==F.MPRT_PLUGINCOMMAND then
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

local function UnloadAllMacros()
  far.MacroCallFar(0x80C65)
  Areas = nil
  LoadedMacros = nil
  Enum_LastIndexes = {}
end

local function ProcessCommandLine (CmdLine)
  local op, text = CmdLine:match("(%S+)%s*(.*)")
  if op then
    local op = op:lower()
    if     op=="post"  and text~="" then far.MacroPost(text, F.KMFLAGS_DISABLEOUTPUT)
    elseif op=="check" and text~="" then far.MacroCheck(text)
    elseif op=="load" then far.MacroLoadAll()
    elseif op=="save" then far.MacroSaveAll()
    elseif op=="unload" then UnloadAllMacros()
    end
  end
end

local LoadMacros, EnumMacros, WriteMacro -- functions

function export.Open (OpenFrom, ...)
  if OpenFrom == F.OPEN_LUAMACRO then
    local calltype, handle, args = ...
    if     calltype==F.MCT_MACROINIT  then return MacroInit (unpack(args))
    elseif calltype==F.MCT_MACROSTEP  then return MacroStep (handle, unpack(args))
    elseif calltype==F.MCT_MACROFINAL then return MacroFinal(handle)
    elseif calltype==F.MCT_MACROPARSE then return MacroParse(unpack(args))
    elseif calltype==F.MCT_LOADMACROS then return LoadMacros(unpack(args))
    elseif calltype==F.MCT_ENUMMACROS then return EnumMacros(unpack(args))
    elseif calltype==F.MCT_WRITEMACRO then return WriteMacro(unpack(args))
    end

  elseif OpenFrom == F.OPEN_COMMANDLINE then
    local guid, cmdline = ...
    return ProcessCommandLine(cmdline)

  elseif OpenFrom == F.OPEN_FROMMACRO then
    local guid, args = ...
    if args[1]=="argtest" then return unpack(args,2) end -- argtest: return received arguments
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

function _G.eval (str, mode)
  if type(str) ~= "string" then return -1 end
  mode = mode or 0
  if not (mode==0 or mode==1 or mode==2 or mode==3) then return -1 end

  if mode == 2 then
    local ret = far.MacroCallFar(0x80C64, str)
    local tp = type(ret)
    if tp == "string" then
      str = ret
    elseif tp == "number" then
      local chunk = LoadedMacros[ret].action
      setfenv(chunk, getfenv(2))
      chunk()
      return 0
    else
      return -2
    end
  end

  local chunk, msg = loadmacro(str)
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

function export.ExitFAR()
  UnloadAllMacros()
end

do
  local func,msg = loadfile(far.PluginStartupInfo().ModuleDir.."api.lua")
  if func then
    func { checkarg=checkarg, loadmacro=loadmacro }
    Plugin.Call=PluginCall
    Plugin.Menu=PluginMenu
    Plugin.Config=PluginConfig
    Plugin.Command=PluginCommand
    mf.eval = _G.eval
  else
    ErrMsg(msg)
  end

  AddCfindFunction()
  package.path = win.GetEnv("farprofile").."\\Macros\\modules\\?.lua;"..package.path
end

--------------------------------------------------------------------------------
-- 2012-12-04 Переходим от базы к файлам.
--------------------------------------------------------------------------------
local AddMacro_filename
local function AddMacro (macrotable)
  local area = type(macrotable)=="table" and type(macrotable.area)=="string" and macrotable.area:lower()
  if area and Areas[area] and type(macrotable.key)=="string" and type(macrotable.action)=="function" then
    if type(macrotable.flags)~="string" then macrotable.flags="" end
    if type(macrotable.description)~="string" then macrotable.description="" end
    macrotable.FileName = AddMacro_filename
    local key = macrotable.key:lower()
    macrotable.id = Areas[area][key] and Areas[area][key].id or #LoadedMacros+1
    LoadedMacros[macrotable.id] = macrotable
    Areas[area][key] = macrotable
  end
end

function EnumMacros (strArea)
  local area = strArea:lower()
  if Areas[area] then
    local k,v = next(Areas[area], Enum_LastIndexes[area])
    Enum_LastIndexes[area] = k
    if k then
      local sequence = ("@%s (Id=%d)"):format(v.FileName, v.id)
      local len = sequence:len()
      if len > 62 then sequence = ("@...%s (Id=%d)"):format(v.FileName:sub(len-58), v.id) end
      LastMessage = pack(v.id, v.key, v.flags, sequence, v.description)
      return F.MPRT_COMMONCASE, LastMessage
    end
  end
end

function LoadMacros (LoadAll)
  local dir = win.GetEnv("farprofile").."\\Macros"

  Areas = LoadAll and {
    other={}, shell={}, viewer={}, editor={}, dialog={}, search={}, disks={},
    mainmenu={}, menu={}, help={}, info={}, qview ={}, tree={}, findfolder={},
    usermenu={}, shellautocompletion={}, dialogautocompletion={}, common={},
  }
  or {
    other={}, viewer={}, editor={}, dialog={}, menu={}, help={},
    dialogautocompletion={}, common={},
  }
  Enum_LastIndexes = {}
  LoadedMacros = {}

  for k=1,2 do
    local root, mask, flags

    if k==1 then
      root, mask, flags = dir.."\\scripts", "*.lua", F.FRS_RECUR
    else
      root, mask, flags = dir.."\\internal", "*.lua", 0
    end

    far.RecursiveSearch (root, mask,
      function (FindData, FullPath)
        local f, msg = loadfile(FullPath)
        if not f then
          ErrMsg(msg) return
        end
        local env = {Macro=AddMacro}
        setfenv(f, env)
        AddMacro_filename = FullPath
        local ok, msg = pcall(f)
        if ok then
          env.Macro = nil
          setmetatable(env, gmeta)
        else
          ErrMsg(msg)
        end
      end, flags)
  end

  LastMessage = pack()
  return F.MPRT_COMMONCASE, LastMessage
end

local CharNames = { ["."]="Dot", ["<"]="Less", [">"]="More", ["|"]="Pipe", ["/"]="Slash",
                    [":"]="Colon", ["?"]="Question", ["*"]="Asterisk", ['"']="Quote" }

function WriteMacro (operation, area, keyname, flags, code, description)
  local dir = win.GetEnv("farprofile").."\\Macros\\internal"
  if not win.CreateDir(dir,true) then return end

  local fname = ("%s\\%s_%s"):format(dir, area, keyname:gsub(".", CharNames)..".lua")
  local attr = win.GetFileAttr(fname)
  if attr then
    win.SetFileAttr(fname, "")
    win.DeleteFile(fname)
  end

  if operation == 0 then -- operation "delete"
    LastMessage[1] = ""
    return F.MPRT_NORMALFINISH, LastMessage
  end

  if operation == 1 then -- operation "write"
    local fp, msg = io.open(fname, "w")
    if fp then
      if code:sub(1,1)=="@" then
        code = ("far.MacroPost(%q,%q,%q)"):format(code,"KMFLAGS_DISABLEOUTPUT",keyname)
      end
      fp:write(("Macro {\narea=%q; key=%q; flags=%q; description=%q; action=function()\n%s\nend;\n}\n"):
        format(area, keyname, flags, description, code))
      fp:close()
      LastMessage[1] = ""
      return F.MPRT_NORMALFINISH, LastMessage
    end
  end
end
