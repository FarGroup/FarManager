local args = ...
local ErrMsg, pack = args.ErrMsg, args.pack

local F = far.Flags
local MacroCallFar = far.MacroCallFar
local gmeta = { __index=_G }
local LastMessage = {}
--------------------------------------------------------------------------------
-- Данный список должен в точности соответствовать enum MACROMODEAREA, т.е. тот же смысл и порядок.
local TrueAreaNames = {
 "Other", "Shell", "Viewer", "Editor", "Dialog", "Search", "Disks", "MainMenu", "Menu", "Help",
 "Info", "QView", "Tree", "FindFolder", "UserMenu", "ShellAutoCompletion", "DialogAutoCompletion",
 "Common",
}

local AllAreaNames = {}
for i,v in ipairs(TrueAreaNames) do AllAreaNames[i]=v:lower() end
for i=1,#AllAreaNames do local str=AllAreaNames[i]; AllAreaNames[str]=i; end

local SomeAreaNames = {
  "other", "viewer", "editor", "dialog", "menu", "help", "dialogautocompletion",
  "common" -- "common" должен идти последним
}

local function GetTrueAreaName(Mode) return TrueAreaNames[Mode+1] or "" end
local function GetAreaName(Mode)     return AllAreaNames[Mode+1] or "" end
local function GetAreaCode(Area)     return (AllAreaNames[Area:lower()] or 0) - 1; end
--------------------------------------------------------------------------------

local MCODE_F_POSTNEWMACRO = 0x80C64
local MCODE_F_CHECKALL     = 0x80C65
local MCODE_F_GETOPTIONS   = 0x80C66

local Areas, AreaNames
local LoadedMacros
local EnumState = {}

local AddMacro_filename
local AddMacro_fields = {"area","key","code","action","flags","description","priority","condition"}
local AddMacro_fields2 = {"guid","callback","callbackId"}

local ExpandKey do -- измеренное время исполнения на ключе "CtrlAltShiftF12" = 9.4 микросекунды.
  local PatExpandKey = regex.new("Ctrl|Alt|Shift|LCtrl|RCtrl|LAlt|RAlt|.*", "i")
  local lctrl,rctrl,lalt,ralt,rest
  local t={}

  local function ExpandKeyCallback (elem)
    if     elem=="ctrl"  then lctrl,rctrl="lctrl","rctrl"
    elseif elem=="alt"   then lalt,ralt="lalt","ralt"
    elseif elem=="shift" then rest="shift"
    elseif elem=="lctrl" then lctrl=elem
    elseif elem=="rctrl" then rctrl=elem
    elseif elem=="lalt"  then lalt=elem
    elseif elem=="ralt"  then ralt=elem
    else rest=rest..elem
    end
  end

  ExpandKey = function (key)
    lctrl,rctrl,lalt,ralt,rest = nil,nil,nil,nil,""
    PatExpandKey:gsub(key:lower(), ExpandKeyCallback)
    local n = (lctrl and rctrl and 2 or 1) * (lalt and ralt and 2 or 1)
    if n==1 then
      t[1] = (lctrl or rctrl or "")..(lalt or ralt or "")..rest
    elseif n==2 then
      t[1] = (lctrl or rctrl or "")..(lalt or ralt or "")..rest
      t[2] = (rctrl or lctrl or "")..(ralt or lalt or "")..rest
    else
      t[1] = "lctrllalt"..rest; t[2] = "lctrlralt"..rest
      t[3] = "rctrllalt"..rest; t[4] = "rctrlralt"..rest
    end
    return t,n
  end
end

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
          local t,n = ExpandKey(k)
          for i=1,n do
            local normkey = t[i]
            if not keyFound[normkey] then
              arTable[normkey] = arTable[normkey] or {}
              table.insert(arTable[normkey], macro)
              keyFound[normkey] = true
            end
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

    if type(macro.priority)~="number" then macro.priority=nil
    elseif macro.priority>100 then macro.priority=100 elseif macro.priority<0 then macro.priority=0
    end

    macro.id = #LoadedMacros+1
    LoadedMacros[macro.id] = macro
  end
end

local CharNames = { ["."]="Dot", ["<"]="Less", [">"]="More", ["|"]="Pipe", ["/"]="Slash",
                    [":"]="Colon", ["?"]="Question", ["*"]="Asterisk", ['"']="Quote" }

local function AddRecordedMacro (srctable)
  local area = type(srctable)=="table" and type(srctable.area)=="string" and srctable.area:lower()
  if not (area and Areas[area]) then return end
  local arTable = Areas[area]

  local key = srctable.key
  if type(key) ~= "string" or
        -- check correspondence between (a) filename and (b) area_key
        ("%s_%s"):format(area, (key:gsub(".",CharNames))):lower() ~=
        AddMacro_filename:gsub("^.*\\",""):sub(1,-5):lower() then
    return
  end

  if type(srctable.code) ~= "string" then return end
  if srctable.code:sub(1,1) ~= "@" then
    local f, msg = loadstring(srctable.code)
    if not f then ErrMsg(msg) return end
  end

  local macro = { priority=200 }
  local t,n = ExpandKey(key)
  for i=1,n do
    local normkey = t[i]
    arTable[normkey] = arTable[normkey] or {}
    arTable[normkey].recorded = macro
  end

  for _,v in ipairs{"area","key","code","flags","description"} do macro[v]=srctable[v] end

  if type(macro.flags)~="string" then macro.flags="" end
  if type(macro.description)~="string" then macro.description=nil end

  macro.id = #LoadedMacros+1
  LoadedMacros[macro.id] = macro
end

local function EnumMacros (strArea, resetEnum)
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

local function LoadMacros (allAreas, unload)
  Areas = {}
  EnumState = {}
  LoadedMacros = {}
  AreaNames = allAreas and AllAreaNames or SomeAreaNames
  for _,name in ipairs(AreaNames) do Areas[name]={} end

  if not unload then
    local NoMacro = function() end
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
          local env = k==1 and {Macro=AddMacro, NoMacro=NoMacro} or {}
          if k==1 then setmetatable(env,gmeta) end
          setfenv(f, env)
          AddMacro_filename = FullPath
          local ok, msg = pcall(f)
          if ok then
            if k==1 then
              env.Macro,env.NoMacro = nil,nil
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

local function UnloadMacros()
  local allAreas = bit64.band(MacroCallFar(MCODE_F_GETOPTIONS),0x1) == 0
  LoadMacros(allAreas,true)
end

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
    LastMessage = pack()
    return F.MPRT_NORMALFINISH, LastMessage
  end

  -- operation "write"
  local fp, msg = io.open(fname, "w")
  if fp then
    fp:write(("area=%q\nkey=%q\nflags=%q\ndescription=%q\ncode=%q\n"):
      format(macro.area, macro.key, macro.flags, macro.description, macro.code))
    fp:close()
    LastMessage = pack()
    return F.MPRT_NORMALFINISH, LastMessage
  end
end

local function WriteMacros()
  for areaname,area in pairs(Areas) do
    for keyname,macroarray in pairs(area) do
      local macro = macroarray.recorded
      if macro and macro.needsave then
        WriteOneMacro(macro,macro.key)
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
    local priority = macro.priority or (area=="common" and 40) or 50
    local pr = priority
    if not checkonly then
      if MacroCallFar(MCODE_F_CHECKALL, GetAreaCode(area), macro.flags, macro.callback, macro.callbackId) then
        if macro.condition then
          pr = macro.condition() -- unprotected call
          pr = (type(pr)=="number") and (pr>100 and 100 or pr<0 and 0 or pr) or (pr and priority)
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

local function GetFromMenu (macrolist)
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
  if item then return macrolist[pos] end
end

local GetMacro_keypat = regex.new("^(r?ctrl)?(r?alt)?(.*)")

local function GetMacro (Mode, Key, UseCommon, CheckOnly)
  if not Areas then return end -- macros were not loaded
  if Mode >= #TrueAreaNames then return end -- трюк используется в CheckForEscSilent() в Фаре

  local Area,Key = GetAreaName(Mode),Key:lower()
  Key = GetMacro_keypat:gsub(Key,
    function(a,b,c)
      return (a=="ctrl" and "lctrl" or a or "")..(b=="alt" and "lalt" or b or "")..c
    end)
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

    if macrolist[1] then
      local toplist = GetTopMacros(areaname, macrolist, CheckOnly)
      if toplist then
        local macro = (CheckOnly or toplist.n==1) and toplist[1] or GetFromMenu(toplist)
        if macro then return macro, areaname; end
      end
    end
  end
end

local function GetMacroWrapper (args)
  local macro,area = GetMacro(unpack(args))
  if macro then
    LastMessage = pack(macro.id, GetAreaCode(area), macro.code or "", macro.description or "",
                       macro.flags, macro.guid, macro.callback, macro.callbackId)
    return F.MPRT_COMMONCASE, LastMessage
  end
end

local function ProcessRecordedMacro (Area, Key, code, flags, description)
  local area, key = Area:lower(), Key:lower()

  local keys,numkeys = ExpandKey(Key)

  if code == "" then -- удаление
    for i=1,numkeys do
      local k = keys[i]
      local m = Areas[area][k] and Areas[area][k].recorded or
                Areas["common"][k] and Areas["common"][k].recorded
      if m then
        m.disabled,m.needsave = true,true
        break
      end
    end
    return
  end

  local macro = {
    area=Area, key=Key, code=code, flags=flags, description=description,
    needsave=true, priority=200
  }
  local existing = Areas[area][keys[1]] and Areas[area][keys[1]].recorded
  macro.id = existing and existing.id or #LoadedMacros+1
  LoadedMacros[macro.id] = macro

  for i=1,numkeys do
    local k = keys[i]
    Areas[area][k] = Areas[area][k] or {}
    Areas[area][k].recorded = macro
  end
end

local function ProcessMacroFromFAR (mode, key, code, flags, description, guid, callback, callbackId)
  local area = GetTrueAreaName(mode)
  if guid then -- MCTL_ADDMACRO
    AddMacro_filename = nil
    AddMacro { area=area, key=key, code=code, flags=flags, description=description,
               guid=guid, callback=callback, callbackId=callbackId }
  else
    ProcessRecordedMacro(area, key, code, flags, description)
  end
end

local function DelMacro (guid, callbackId) -- MCTL_DELMACRO
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

local function RunStartMacro()
  if not Areas then return end -- macros not loaded
  for _,macros in pairs(Areas.shell) do
    local m = macros.recorded
    if m and not m.disabled and m.flags and m.flags:lower():find("runafterfarstart") then
      if MacroCallFar(MCODE_F_CHECKALL, GetAreaCode("Shell"), m.flags) then
        MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags)
      end
    end
    for _,m in ipairs(macros) do
      if not m.disabled and m.flags and m.flags:lower():find("runafterfarstart") then
        if MacroCallFar(MCODE_F_CHECKALL, GetAreaCode("Shell"), m.flags) then
          if not m.condition or m.condition() then
            MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags)
          end
        end
      end
    end
  end
  LastMessage = pack(true)
  return F.MPRT_COMMONCASE, LastMessage
end

local function GetMacroById (id)
  return LoadedMacros[id]
end

return {
  DelMacro = DelMacro,
  EnumMacros = EnumMacros,
  GetAreaCode = GetAreaCode,
  GetMacro = GetMacro,
  GetMacroById = GetMacroById,
  GetMacroWrapper = GetMacroWrapper,
  GetTrueAreaName = GetTrueAreaName,
  LoadMacros = LoadMacros,
  ProcessMacroFromFAR = ProcessMacroFromFAR,
  RunStartMacro = RunStartMacro,
  UnloadMacros = UnloadMacros,
  WriteMacros = WriteMacros,
}
