-- started: 2012-04-20

local function LOG (fmt, ...)
  local log = io.open("c:\\lua.log","at")
  if log then
    log:write("LUA: ", fmt:format(...), "\n")
    log:close()
  end
end

local F = far.Flags
local MacroCallFar = far.MacroCallFar
local co_create, co_yield, co_resume, co_status, co_wrap =
  coroutine.create, coroutine.yield, coroutine.resume, coroutine.status, coroutine.wrap

local MCODE_F_GETMACRODATA = 0x80C64
local MCODE_F_POSTNEWMACRO = 0x80C65
local MCODE_F_CHECKALL     = 0x80C66
local MCODE_F_NORMALIZEKEY = 0x80C67

local PROPAGATE={} -- a unique value, inaccessible to scripts.
local gmeta = { __index=_G }
local RunningMacros = {}
local LastMessage = {}
local Areas, AreaNames
local LoadedMacros
local EnumState = {}

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
  if Id == 0 then -- Id==0 может быть только для "одноразовых" макросов, запускаемых посредством MSSC_POST.
    chunk, msg = loadmacro(Text)
  else
    local mtable = LoadedMacros[Id]
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

local LoadMacros, EnumMacros, WriteMacros, GetMacro, ProcessMacroFromFAR, DelMacro, RunStartMacro -- functions

local function UnloadMacros()
  LoadMacros(true,true)
end

local function ProcessCommandLine (CmdLine)
  local op, text = CmdLine:match("(%S+)%s*(.*)")
  if op then
    local op = op:lower()
    if     op=="post"  and text~="" then far.MacroPost(text, F.KMFLAGS_DISABLEOUTPUT)
    elseif op=="check" and text~="" then far.MacroCheck(text)
    elseif op=="load" then far.MacroLoadAll()
    elseif op=="save" then WriteMacros()
    elseif op=="unload" then UnloadMacros()
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
    elseif calltype==F.MCT_LOADMACROS     then return LoadMacros(unpack(args))
    elseif calltype==F.MCT_ENUMMACROS     then return EnumMacros(unpack(args))
    elseif calltype==F.MCT_WRITEMACROS    then return WriteMacros()
    elseif calltype==F.MCT_GETMACRO       then return GetMacro(unpack(args))
    elseif calltype==F.MCT_PROCESSMACRO   then return ProcessMacroFromFAR(unpack(args))
    elseif calltype==F.MCT_DELMACRO       then return DelMacro(unpack(args))
    elseif calltype==F.MCT_RUNSTARTMACRO  then return RunStartMacro()
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
    local area,key,usecommon = MacroCallFar(MCODE_F_GETMACRODATA, str)
    if not mode then return -2 end

    local _,ret = GetMacro(area,key,usecommon,false,true)
    if ret==nil or ret[1]==0 then return -2 end

    local id = ret[1]
    local macro = LoadedMacros[id]
    if macro.action then
      -- setfenv(macro.action, getfenv(2))
      macro.action()
      return 0
    else
      str = macro.code
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
  UnloadMacros()
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
local AddMacro_fields = {"area","key","code","action","flags","description","priority","condition"}
local AddMacro_fields2 = {"guid","callback","callbackId"}

local function AddMacro (srctable)
  local ok
  local area = type(srctable)=="table" and type(srctable.area)=="string" and srctable.area:lower()
  if not area then return end

  local key = type(srctable.key)=="string" and srctable.key:lower()
  if not key then return end

  local keyregex = key:find("^/.+/$")
  if keyregex then
    ok, keyregex = pcall(regex.new, "(" .. key:sub(2,-2) .. ")", "i")
    if not ok then ErrMsg(("Invalid regex: %s"):format(srctable.key)); return; end
  end

  local IsCode = type(srctable.code)=="string"
  if IsCode then
    if srctable.code:sub(1,1) ~= "@" then
      local f, msg = loadstring(srctable.code)
      if not f then ErrMsg(msg) return end
    end
  elseif type(srctable.action)~="function" then
    return
  end

  local macro={}
  local arFound = {} -- prevent multiple inclusions, i.e. area="Editor Editor"
  for a in area:gmatch("%S+") do
    local arTable = Areas[a]
    if arTable and not arFound[a] then
      if keyregex then
        arTable[1] = arTable[1] or {}
        table.insert(arTable[1], macro)
        macro.keyregex = keyregex
      else
        local keyFound = {} -- prevent multiple inclusions
        for k in key:gmatch("%S+") do
          local normkey = (MacroCallFar(MCODE_F_NORMALIZEKEY, k) or k):lower()
          if not keyFound[normkey] then
            arTable[normkey] = arTable[normkey] or {}
            table.insert(arTable[normkey], macro)
            keyFound[normkey] = true
          end
        end
      end
      arFound[a] = true
    end
  end

  if next(arFound) then
    for _,v in ipairs(AddMacro_fields) do macro[v]=srctable[v] end
    if AddMacro_filename then
      macro.FileName = AddMacro_filename
    else
      for _,v in ipairs(AddMacro_fields2) do macro[v]=srctable[v] end
    end

    if type(macro.flags)~="string" then macro.flags="" end
    if type(macro.description)~="string" then macro.description=nil end
    if type(macro.condition)~="function" then macro.condition=nil end

    if type(macro.priority)~="number" then macro.priority=50
    elseif macro.priority>100 then macro.priority=100 elseif macro.priority<0 then macro.priority=0
    end

    macro.id = #LoadedMacros+1
    LoadedMacros[macro.id] = macro
  end
end

local function AddRecordedMacro (srctable)
  local area = type(srctable)=="table" and type(srctable.area)=="string" and srctable.area:lower()
  if not (area and Areas[area]) then return end

  local key = srctable.key
  if type(key) ~= "string" then return end
  key = (MacroCallFar(MCODE_F_NORMALIZEKEY, key) or key):lower()

  if type(srctable.code)=="string" then
    if srctable.code:sub(1,1) ~= "@" then
      local f, msg = loadstring(srctable.code)
      if not f then ErrMsg(msg) return end
    end
  else
    return
  end

  local macro = { priority=200 }
  Areas[area][key] = Areas[area][key] or {}
  Areas[area][key].recorded = macro

  for _,v in ipairs{"area","key","code","flags","description"} do macro[v]=srctable[v] end

  if type(macro.flags)~="string" then macro.flags="" end
  if type(macro.description)~="string" then macro.description=nil end

  macro.id = #LoadedMacros+1
  LoadedMacros[macro.id] = macro
end

function EnumMacros (strArea, resetEnum)
  local area = strArea:lower()
  if Areas[area] then
    if EnumState.area ~= area or resetEnum then
      EnumState.area, EnumState.key, EnumState.index = area, nil, 0
    end

    if EnumState.key == nil or EnumState.index ~= 0 and Areas[area][EnumState.key][EnumState.index] == nil then
      EnumState.key, EnumState.index = next(Areas[area],EnumState.key), 0
    end

    if EnumState.key then
      local macrotable = Areas[area][EnumState.key]
      local macro
      if EnumState.index == 0 then
        macro = macrotable.recorded
        if not macro then
          EnumState.index = 1
          macro = macrotable[1]
        end
      else
        macro = macrotable[EnumState.index]
      end

      if macro then
        EnumState.index = EnumState.index + 1
        local code = macro.code
        if not code then
          code = ("@%s (Id=%d)"):format(macro.FileName, macro.id)
          local len = code:len()
          if len > 62 then code = ("@...%s (Id=%d)"):format(macro.FileName:sub(len-58), macro.id) end
        end
        LastMessage = pack(macro.id, macro.key, macro.flags, code, macro.description or "")
        return F.MPRT_COMMONCASE, LastMessage
      end
    end
  end
end

local AllAreaNames = {
  "other", "shell", "viewer", "editor", "dialog", "search", "disks", "mainmenu", "menu", "help",
  "info", "qview", "tree", "findfolder", "usermenu", "shellautocompletion", "dialogautocompletion",
  "common" -- "common" должен идти последним
}

local SomeAreaNames = {
    "other", "viewer", "editor", "dialog", "menu", "help", "dialogautocompletion",
    "common" -- "common" должен идти последним
}

function LoadMacros (allAreas, unload)
  Areas = {}
  EnumState = {}
  LoadedMacros = {}
  AreaNames = allAreas and AllAreaNames or SomeAreaNames
  for _,name in ipairs(AreaNames) do Areas[name]={} end

  if not unload then
    local dir = win.GetEnv("farprofile").."\\Macros"
    for k=1,2 do
      local root = k==1 and dir.."\\scripts" or dir.."\\internal"
      local flags = k==1 and F.FRS_RECUR or 0
      far.RecursiveSearch (root, "*.lua",
        function (FindData, FullPath)
          local f, msg = loadfile(FullPath)
          if not f then
            ErrMsg(msg) return
          end
          local env = k==1 and {Macro=AddMacro} or {}
          setfenv(f, env)
          AddMacro_filename = FullPath
          local ok, msg = pcall(f)
          if ok then
            if k==1 then
              env.Macro = nil
              setmetatable(env, gmeta)
            else
              AddRecordedMacro(env)
            end
          else
            ErrMsg(msg)
          end
        end, flags)
    end
  end

  LastMessage = pack()
  return F.MPRT_COMMONCASE, LastMessage
end

local CharNames = { ["."]="Dot", ["<"]="Less", [">"]="More", ["|"]="Pipe", ["/"]="Slash",
                    [":"]="Colon", ["?"]="Question", ["*"]="Asterisk", ['"']="Quote" }

local function WriteOneMacro (macro, keyname)
  local dir = win.GetEnv("farprofile").."\\Macros\\internal"
  if not win.CreateDir(dir,true) then return end

  local fname = ("%s\\%s_%s"):format(dir, macro.area, keyname:gsub(".", CharNames)..".lua")
  local attr = win.GetFileAttr(fname)
  if attr then
    win.SetFileAttr(fname, "")
    win.DeleteFile(fname)
  end

  if macro.disabled then -- operation "delete"
    LastMessage[1] = ""
    return F.MPRT_NORMALFINISH, LastMessage
  end

  -- operation "write"
  local fp, msg = io.open(fname, "w")
  if fp then
    fp:write(("area=%q\nkey=%q\nflags=%q\ndescription=%q\ncode=%q\n"):
      format(macro.area, macro.key, macro.flags, macro.description, macro.code))
    fp:close()
    LastMessage[1] = ""
    return F.MPRT_NORMALFINISH, LastMessage
  end
end

function WriteMacros()
  for areaname,area in pairs(Areas) do
    for keyname,macroarray in pairs(area) do
      local macro = macroarray.recorded
      if macro and macro.needsave then
        local normkey = MacroCallFar(MCODE_F_NORMALIZEKEY, keyname) or keyname
        WriteOneMacro(macro,normkey)
        if macro.disabled then
          LoadedMacros[macroarray.recorded.id] = false
          macroarray.recorded = nil
        else
          macro.needsave = nil
        end
      end
    end
  end
end

local function GetTopMacros (area, macrolist, checkonly)
  local topmacros = { n=0 }
  local max_priority = -1
  for _,macro in ipairs(macrolist) do
    local pr = macro.priority
    if not checkonly then
      if MacroCallFar(MCODE_F_CHECKALL, area, macro.flags, macro.callback, macro.callbackId) then
        if macro.condition then
          pr = macro.condition() -- unprotected call
          pr = (type(pr)=="number") and (pr>100 and 100 or pr<0 and 0 or pr) or (pr and macro.priority)
        end
      else
        pr = nil
      end
    end
    if pr then
      if pr > max_priority then
        topmacros.n = 1
        topmacros[1] = macro
        max_priority = pr
        if pr > 100 then
          break
        end
      elseif pr == max_priority then
        topmacros.n = topmacros.n + 1
        topmacros[topmacros.n] = macro
      end
    end
  end
  return topmacros.n > 0 and topmacros
end

local function GetFinalMacro (macrolist, area, checkonly)
  local selected_index = 1
  if macrolist.n > 1 and not checkonly then
    local menuitems = {}
    for i=1,macrolist.n do
      local macro = macrolist[i]
      local descr = macro.description
      if not descr or descr=="" then
        descr = ("< No description: Id=%d >"):format(macro.id)
      end
      menuitems[i] = { text = descr }
    end

    local item, pos = far.Menu({Title="Execute a macro"}, menuitems)
    if not item then
      LastMessage = pack(0)
      return F.MPRT_COMMONCASE, LastMessage
    end

    selected_index = pos
  end

  local macro = macrolist[selected_index]
  LastMessage = pack(macro.id, area, macro.code or "", macro.description or "",
                     macro.flags, macro.guid, macro.callback, macro.callbackId)
  return F.MPRT_COMMONCASE, LastMessage
end

function GetMacro (Area, Key, UseCommon, StrictKeys, CheckOnly)
  if not Areas then return end -- macros were not loaded

  Area,Key = Area:lower(),Key:lower()

  local Names = Area=="" and AreaNames or { Area, UseCommon and "common" or nil }

  for _,areaname in ipairs(Names) do
    local areatable = Areas[areaname]
    local macros = areatable[Key]
    local macros_regex = areatable[1]
    local macrolist = {}

    if macros then
      if macros.recorded and not macros.recorded.disabled then -- must come first
        macrolist[#macrolist+1] = macros.recorded
      end
      for _,v in ipairs(macros) do -- must come second
        if not v.disabled then macrolist[#macrolist+1]=v end
      end
    end
    if macros_regex then
      for _,v in ipairs(macros_regex) do
        if v.keyregex:match(Key) == Key then
          macrolist[#macrolist+1] = v
        end
      end
    end

    if next(macrolist) then
      local trylist = GetTopMacros(areaname, macrolist, CheckOnly)
      if trylist then return GetFinalMacro(trylist, areaname, CheckOnly) end
    end
  end

  if StrictKeys then return end

  local keylist = {}
  if Key:find("rctrl") then keylist[#keylist+1] = Key:gsub("rctrl","ctrl",1) end
  if Key:find("ralt") then keylist[#keylist+1] = Key:gsub("ralt","alt",1) end
  if #keylist==2 then keylist[#keylist+1] = keylist[1]:gsub("ralt","alt",1) end
  if #keylist==0 then return end

  for _,areaname in ipairs(Names) do
    local areatable = Areas[areaname]
    local macrolist = {}

    for _,cur_key in ipairs(keylist) do
      local macros = areatable[cur_key]
      if macros and macros.recorded and not macros.recorded.disabled then -- must come first
        macrolist[#macrolist+1] = macros.recorded
      end
    end
    for _,cur_key in ipairs(keylist) do
      local macros = areatable[cur_key]
      if macros then -- must come second
        for _,v in ipairs(macros) do
          if not v.disabled then macrolist[#macrolist+1]=v end
        end
      end
    end

    if #macrolist > 0 then
      macrolist.n = #macrolist
      local trylist = GetTopMacros(areaname, macrolist, CheckOnly)
      if trylist then return GetFinalMacro(trylist, areaname, CheckOnly) end
    end
  end
end

local function ProcessRecordedMacro (Area, Key, code, flags, description)
  local area, key = Area:lower(), Key:lower()

  if code == "" then -- удаление
    local m = Areas[area][key] and Areas[area][key].recorded or
              Areas["common"][key] and Areas["common"][key].recorded
    if m then
      m.disabled,m.needsave = true,true
    end
    return
  end

  local macro = {
    area=Area, key=Key, code=code, flags=flags, description=description,
    needsave=true, priority=200
  }

  Areas[area][key] = Areas[area][key] or {}
  if Areas[area][key].recorded then -- модификация
    macro.id = Areas[area][key].recorded.id
  else -- добавление
    macro.id = #LoadedMacros+1
  end

  Areas[area][key].recorded = macro
  LoadedMacros[macro.id] = macro
end

function ProcessMacroFromFAR (area, key, code, flags, description, guid, callback, callbackId)
  if guid then -- MCTL_ADDMACRO
    AddMacro_filename = nil
    AddMacro { area=area, key=key, code=code, flags=flags, description=description,
               guid=guid, callback=callback, callbackId=callbackId }
  else
    ProcessRecordedMacro(area, key, code, flags, description)
  end
end

function DelMacro (guid, callbackId) -- MCTL_DELMACRO
  for _,areatable in pairs(Areas) do
    for _,macroarray in pairs(areatable) do
      for _,m in ipairs(macroarray) do
        if m.guid and m.guid[1]==guid[1] and m.callbackId==callbackId and not m.disabled then
          m.disabled = true
          LastMessage = pack(true)
          return F.MPRT_COMMONCASE, LastMessage
        end
      end
    end
  end
end

function RunStartMacro()
  for _,macros in pairs(Areas.shell) do
    local m = macros.recorded
    if m and not m.disabled and m.flags and m.flags:lower():find("runafterfarstart") then
      if MacroCallFar(MCODE_F_CHECKALL, "Shell", m.flags) then
        MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags)
      end
    end
    for _,m in ipairs(macros) do
      if not m.disabled and m.flags and m.flags:lower():find("runafterfarstart") then
        if MacroCallFar(MCODE_F_CHECKALL, "Shell", m.flags) then
          if not m.condition or m.condition() then
            MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags)
          end
        end
      end
    end
  end
end
