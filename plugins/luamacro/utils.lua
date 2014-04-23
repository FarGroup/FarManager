local Shared = ...
local M, ErrMsg, pack = Shared.M, Shared.ErrMsg, Shared.pack

local F = far.Flags
local type = type
local band, bor = bit64.band, bit64.bor
local MacroCallFar = far.MacroCallFar
local gmeta = { __index=_G }
local LastMessage = {}
--------------------------------------------------------------------------------
-- Данный список должен в точности соответствовать enum FARMACROAREA, т.е. тот же смысл и порядок.
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

local function GetTrueAreaName(Mode) return TrueAreaNames[Mode+1] end
local function GetAreaName(Mode)     return AllAreaNames[Mode+1] end

local function GetAreaCode(Area)
  local code = AllAreaNames[Area:lower()]
  return code and code-1
end
--------------------------------------------------------------------------------

local MCODE_F_POSTNEWMACRO = 0x80C64
local MCODE_F_CHECKALL     = 0x80C65
local MCODE_F_GETOPTIONS   = 0x80C66

local Areas
local LoadedMacros
local LoadMacrosDone
local EnumState = {}
local Events
local EventGroups = {"dialogevent","editorevent","editorinput","exitfar","viewerevent"}

local AddMacro_filename
local AddMacro_fields = {"area","key","code","action","description","priority","condition","filemask"}
local AddMacro_fields2 = {"guid","callback","callbackId"}

package.nounload = {}
local initial_modules = {}
for k in pairs(package.loaded) do initial_modules[k]=true end

local function CheckFileName (mask, name)
  return far.ProcessName("PN_CMPNAMELIST", mask, name, "PN_SKIPPATH")
end

local StringToFlags, FlagsToString do
  local MacroFlagsByInt = {
    [0x00000001] = "EnableOutput",
    [0x00000002] = "NoSendKeysToPlugins",
    [0x00000008] = "RunAfterFARStart",
    [0x00000010] = "EmptyCommandLine",
    [0x00000020] = "NotEmptyCommandLine",
    [0x00000040] = "EVSelection",
    [0x00000080] = "NoEVSelection",
    [0x00000100] = "Selection",
    [0x00000200] = "PSelection",
    [0x00000400] = "NoSelection",
    [0x00000800] = "NoPSelection",
    [0x00001000] = "NoFilePanels",
    [0x00002000] = "NoFilePPanels",
    [0x00004000] = "NoPluginPanels",
    [0x00008000] = "NoPluginPPanels",
    [0x00010000] = "NoFolders",
    [0x00020000] = "NoPFolders",
    [0x00040000] = "NoFiles",
    [0x00080000] = "NoPFiles",
  }
  local MacroFlagsByStr={}
  for k,v in pairs(MacroFlagsByInt) do MacroFlagsByStr[v:lower()]=k end

  function StringToFlags (str)
    local flags = 0
    if type(str) == "string" then
      for word in str:lower():gmatch("[^ |]+") do
        local f = MacroFlagsByStr[word]
        if f then flags = bor(flags, f) end
      end
    end
    return flags
  end

  -- assume 52 bits at most
  function FlagsToString (flags)
    local str, bit = "", 1
    while flags >= bit do
      if band(flags,bit) ~= 0 then
        local s = MacroFlagsByInt[bit]
        if s then
          if str ~= "" then str = str.." " end
          str = str..s
        end
      end
      bit = bit * 2
    end
    return str
  end
end

local function EV_Handler (macros, filename, ...)
  -- Get current priorities.
  local indexes,priorities = {},{}
  for i,m in ipairs(macros) do
    indexes[i],priorities[i] = i, -1
    if not (m.filemask and filename) or CheckFileName(m.filemask, filename) then
      if m.condition then
        local pr = m.condition(...)
        if pr then
          if type(pr)=="number" then priorities[i] = pr<0 and 0 or pr>100 and 100 or pr
          else priorities[i] = m.priority
          end
        end
      else
        priorities[i] = m.priority
      end
    end
  end

  -- Sort by current priorities (stable sort).
  table.sort(indexes, function(i,j)
      return priorities[i]>priorities[j] or priorities[i]==priorities[j] and i<j
    end)

  -- Execute.
  for _,i in ipairs(indexes) do
    if priorities[i] < 0 then break end
    local ret = macros[i].action(...)
    if ret and (macros==Events.dialogevent or macros==Events.editorinput) then
      return ret
    end
    --MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags)
  end
end

local Subscriptions = {}
local SubscribeChangeEvent = editor.SubscribeChangeEvent

function editor.SubscribeChangeEvent (EditorID, Subscribe)
  if not EditorID or EditorID==F.CURRENT_EDITOR then
    local info = editor.GetInfo(nil)
    if not info then return false end
    EditorID = info.EditorID
  end

  local count = Subscriptions[EditorID]
  if count then
    local result = true
    if Subscribe then
      if count==0 then result=SubscribeChangeEvent(EditorID,true) end
      if result then Subscriptions[EditorID]=count+1 end
    else
      if count==1 then result=SubscribeChangeEvent(EditorID,false) end
      if result and count>0 then Subscriptions[EditorID]=count-1 end
    end
    return result
  end
  return false
end

local function export_ProcessEditorEvent (EditorID, Event, Param)
  if     Event==F.EE_READ  then Subscriptions[EditorID]=0
  elseif Event==F.EE_CLOSE then Subscriptions[EditorID]=nil
  end
  return EV_Handler(Events.editorevent, editor.GetFileName(nil), EditorID, Event, Param)
end

local function export_ProcessViewerEvent (ViewerID, Event, Param)
  return EV_Handler(Events.viewerevent, viewer.GetFileName(nil), ViewerID, Event, Param)
end

local function export_ExitFAR ()
  return EV_Handler(Events.exitfar)
end

local function export_ProcessDialogEvent (Event, Param)
  return EV_Handler(Events.dialogevent, nil, Event, Param)
end

local function export_ProcessEditorInput (Rec)
  return EV_Handler(Events.editorinput, editor.GetFileName(nil), Rec)
end

local ExpandKey do -- измеренное время исполнения на ключе "CtrlAltShiftF12" = 8.4 микросекунды.
  local PatExpandKey = regex.new("ctrl|alt|shift|lctrl|rctrl|lalt|ralt|.*")
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
  if type(srctable)~="table"
    or type(srctable.area)~="string"
    or type(srctable.key)~="string" then return
  end

  local keyregex = srctable.key:match("^/(.+)/$")
  if keyregex then
    if pcall(regex.new, keyregex) then
      keyregex = regex.new("^("..keyregex..")$", "i")
    else
      ErrMsg("Invalid regex: "..srctable.key)
      return
    end
  end

  if type(srctable.code)=="string" then
    if srctable.code:sub(1,1) ~= "@" then
      local f, msg = loadstring(srctable.code)
      if not f then ErrMsg(msg) return end
    end
  elseif type(srctable.action)~="function" then
    return
  end

  local macro = {}
  local arFound = {} -- prevent multiple inclusions, i.e. area="Editor Editor"
  for a in srctable.area:lower():gmatch("%S+") do
    local arTable = Areas[a]
    if arTable and not arFound[a] then
      if keyregex then
        arTable[1] = arTable[1] or {}
        table.insert(arTable[1], macro)
        macro.keyregex = keyregex
      else
        local keyFound = {} -- prevent multiple inclusions
        for k in srctable.key:lower():gmatch("%S+") do
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

    macro.flags = StringToFlags(srctable.flags)
    if type(macro.description)~="string" then macro.description=nil end
    if type(macro.condition)~="function" then macro.condition=nil end
    if type(macro.filemask)~="string" then macro.filemask=nil end

    if type(macro.priority)~="number" then macro.priority=nil
    elseif macro.priority>100 then macro.priority=100 elseif macro.priority<0 then macro.priority=0
    end

    macro.id = #LoadedMacros+1
    LoadedMacros[macro.id] = macro
    return macro.id
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

  local macro = { FileName=AddMacro_filename }
  local t,n = ExpandKey(key)
  for i=1,n do
    local normkey = t[i]
    arTable[normkey] = arTable[normkey] or {}
    arTable[normkey].recorded = macro
  end

  for _,v in ipairs{"area","key","code","description"} do macro[v]=srctable[v] end

  macro.flags = StringToFlags(srctable.flags)
  if type(macro.description)~="string" then macro.description=nil end

  macro.id = #LoadedMacros+1
  LoadedMacros[macro.id] = macro
end

local AddEvent_fields = {"group","action","description","priority","condition","filemask"}
local function AddEvent (srctable)
  local group = type(srctable)=="table" and type(srctable.group)=="string" and srctable.group:lower()
  if not (group and Events[group]) then return end

  if type(srctable.action)~="function" then return end

  local macro={}
  table.insert(Events[group], macro)

  for _,v in ipairs(AddEvent_fields) do macro[v]=srctable[v] end
  macro.FileName = AddMacro_filename

  if type(macro.description)~="string" then macro.description=nil end
  if type(macro.condition)~="function" then macro.condition=nil end
  if type(macro.filemask)~="string" then macro.filemask=nil end

  if type(macro.priority)~="number" then macro.priority=50
  elseif macro.priority>100 then macro.priority=100 elseif macro.priority<0 then macro.priority=0
  end

  macro.id = #LoadedMacros+1
  LoadedMacros[macro.id] = macro
  return macro.id
end

local function EnumMacros (strArea, resetEnum)
  local area = strArea:lower()
  if Areas[area] then
    if EnumState.area ~= area or resetEnum then
      EnumState.area, EnumState.index = area, 0
    end
    while true do
      EnumState.index = EnumState.index + 1
      local macro = LoadedMacros[EnumState.index]
      if macro then
        if macro.area and macro.area:lower():find(area) then
          local code = macro.code
          if not code then
            code = ("@%s (Id=%d)"):format(macro.FileName, macro.id)
            local len = code:len()
            if len > 62 then code = ("@...%s (Id=%d)"):format(macro.FileName:sub(len-58), macro.id) end
          end
          LastMessage = pack(macro.id, macro.key, macro.flags, code, macro.description or "")
          return F.MPRT_NORMALFINISH, LastMessage
        end
      else
        EnumState.index = 0
        break
      end
    end
  end
end

local function LoadMacros (allAreas, unload)
  if LoadMacrosDone then
    LoadMacrosDone = false
    export_ExitFAR()
  end

  local numerrors=0
  local newAreas = {}
  Events,Subscriptions = {},{}
  EnumState = {}
  LoadedMacros = {}
  if Shared.panelsort then Shared.panelsort.DeleteSortModes() end

  local AreaNames = allAreas and AllAreaNames or SomeAreaNames
  for _,name in ipairs(AreaNames) do newAreas[name]={} end
  for _,name in ipairs(EventGroups) do Events[name]={} end
  for k in pairs(package.loaded) do
    if initial_modules[k]==nil and not package.nounload[k] then
      package.loaded[k]=nil
    end
  end

  -- Copy macros loaded by MCTL_ADDMACRO to save them from destruction.
  if Areas then
    local IdUpdated = {}
    for a,areatable in pairs(Areas) do
      for k,macroarray in pairs(areatable) do
        for i,m in ipairs(macroarray) do
          if m.guid and not m.disabled then
            newAreas[a][k] = newAreas[a][k] or {}
            table.insert(newAreas[a][k], m)
            if not IdUpdated[m] then
              IdUpdated[m] = true
              m.id = #LoadedMacros+1
              LoadedMacros[m.id] = m
            end
          end
        end
      end
    end
  end
  Areas = newAreas

  if not unload then
    local DummyFunc = function() end
    local dir = win.GetEnv("farprofile").."\\Macros"
    if 0 == band(MacroCallFar(MCODE_F_GETOPTIONS),0x10) then -- not ReadOnlyConfig
      win.CreateDir(dir.."\\scripts", true)
      win.CreateDir(win.GetEnv("farprofile").."\\Menus", true)
    end

    local macroinit_name, macroinit_done = dir.."\\scripts\\_macroinit.lua", false
    local isStationary = true
    local moonscript = require "moonscript"

    local function LoadOneFile (FindData, FullPath)
      if macroinit_done and #FullPath==#macroinit_name and far.LStricmp(FullPath,macroinit_name)==0 then
        return
      end
      local loadfunc = string.sub(FullPath,-4):lower()==".lua" and loadfile or moonscript.loadfile
      local f, msg = loadfunc(FullPath)
      if not f then
        numerrors=numerrors+1; ErrMsg(msg); return
      end
      local env = isStationary and {Macro=AddMacro,Event=AddEvent,NoMacro=DummyFunc,NoEvent=DummyFunc} or {}
      if isStationary then setmetatable(env,gmeta) end
      setfenv(f, env)
      AddMacro_filename = FullPath
      local ok, msg = pcall(f, FullPath)
      if ok then
        if isStationary then
          env.Macro,env.Event,env.NoMacro,env.NoEvent = nil,nil,nil,nil
        else
          AddRecordedMacro(env)
        end
      else
        numerrors=numerrors+1; ErrMsg(msg)
      end
    end

    local info = win.GetFileInfo(macroinit_name)
    if info then
      LoadOneFile(info, macroinit_name)
      macroinit_done = true
    end
    far.RecursiveSearch (dir.."\\scripts", "*.lua,*.moon", LoadOneFile, bor(F.FRS_RECUR,F.FRS_SCANSYMLINK))

    isStationary = false
    far.RecursiveSearch (dir.."\\internal", "*.lua", LoadOneFile, 0)

    LoadMacrosDone = true
  end

  export.ExitFAR = Events.exitfar[1] and export_ExitFAR
  export.ProcessDialogEvent = Events.dialogevent[1] and export_ProcessDialogEvent
  export.ProcessEditorEvent = Events.editorevent[1] and export_ProcessEditorEvent
  export.ProcessEditorInput = Events.editorinput[1] and export_ProcessEditorInput
  export.ProcessViewerEvent = Events.viewerevent[1] and export_ProcessViewerEvent

  return numerrors==0
end

local function InitMacroSystem()
  local allAreas = band(MacroCallFar(MCODE_F_GETOPTIONS),0x3) == 0
  LoadMacros(allAreas,true)
end

local function WriteOneMacro (dir, macro, keyname, delete)
  local fname = ("%s\\%s_%s.lua"):format(dir, macro.area, (keyname:gsub(".", CharNames)))
  local attr = win.GetFileAttr(fname)
  if attr then
    win.SetFileAttr(fname, "")
    win.DeleteFile(fname)
  end

  if delete then return end

  -- operation "write"
  local fp, msg = io.open(fname, "w")
  if fp then
    fp:write(("area=%q\nkey=%q\nflags=%q\ndescription=%q\ncode=%q\n"):
      format(macro.area, macro.key, FlagsToString(macro.flags), macro.description, macro.code))
    fp:close()
    macro.FileName = fname
  end
end

local function WriteMacros()
  if 0 ~= band(MacroCallFar(MCODE_F_GETOPTIONS),0x10) then return end -- ReadOnlyConfig

  local dir = win.GetEnv("farprofile").."\\Macros\\internal"
  if not win.CreateDir(dir, true) then return end

  for areaname,area in pairs(Areas) do
    for keyname,macroarray in pairs(area) do
      local macro = macroarray.recorded
      if macro and macro.needsave then
        WriteOneMacro(dir, macro, macro.key, macro.disabled)
        macro.needsave = nil
        if macro.disabled then
          macroarray.recorded = nil
        end
      end
    end
  end
  return true
end

local function GetFromMenu (macrolist)
  local menuitems = {}
  for i,macro in ipairs(macrolist) do
    local descr = macro.description
    if not descr or descr=="" then
      descr = ("< No description: Id=%d >"):format(macro.id)
    end
    menuitems[i] = { text = descr }
  end

  local props, bkeys = {Title=M.UtExecuteMacroTitle,Bottom=M.UtExecuteMacroBottom}, {{BreakKey="A+F4"}}
  while true do
    local item, pos = far.Menu(props, menuitems, bkeys)
    if not item then
      return
    elseif item.BreakKey == nil then
      return macrolist[pos]
    elseif item.BreakKey == "A+F4" then
      props.SelectIndex = pos
      local m = macrolist[pos]
      if m.FileName then
        local startline = m.action and debug.getinfo(m.action,"S").linedefined
        editor.Editor(m.FileName,nil,nil,nil,nil,nil,nil,startline,nil,65001)
      end
    end
  end
end

local GetMacro_keypat = regex.new("^(r?ctrl)?(r?alt)?(.*)")

local function GetMacro (argMode, argKey, argUseCommon, argCheckOnly)
  local area = GetAreaName(argMode)
  if not area then return end -- трюк используется в CheckForEscSilent() в Фаре

  local key = GetMacro_keypat:gsub(argKey:lower(),
    function(a,b,c)
      return (a=="ctrl" and "lctrl" or a or "")..(b=="alt" and "lalt" or b or "")..c
    end)
  local Names = { area, argUseCommon and area~="common" and "common" or nil }

  -- First, check "keyboard-recorded" macros, they have the highest priority.
  for _,areaname in ipairs(Names) do
    local areatable = Areas[areaname]
    if areatable and areatable[key] then
      local m = areatable[key].recorded
      if m and not m.disabled and (argCheckOnly or MacroCallFar(MCODE_F_CHECKALL,argMode,m.flags,nil,nil)) then
        return m, areaname
      end
    end
  end

  -- Create collector table: keys are macros, values are indexes into CInfo.
  -- For each macro, CInfo stores 2 consecutive values: dynamic priority and found area.
  local Collector, CInfo = {}, {}

  -- Filter macros by filemask and flags. Put the "successful" ones in the collector.
  local filename = area=="editor" and editor.GetFileName() or area=="viewer" and viewer.GetFileName()

  local function ExamineMacro (m, areaname)
    local check = not (filename and m.filemask) or CheckFileName(m.filemask, filename)
    if check and MacroCallFar(MCODE_F_CHECKALL, GetAreaCode(area), m.flags, m.callback, m.callbackId) then
      if not Collector[m] then
        local n = #CInfo + 1
        Collector[m] = n
        CInfo[n] = m.priority or (areaname=="common" and 40) or 50
        CInfo[n+1] = areaname
      end
    end
  end

  for _,areaname in ipairs(Names) do
    local areatable = Areas[areaname]
    if areatable then
      local macros = areatable[key]
      if macros then
        for _,m in ipairs(macros) do
          if not m.disabled then
            if argCheckOnly then return m, areaname end
            ExamineMacro(m, areaname)
          end
        end
      end
      local macros_regex = areatable[1]
      if macros_regex then
        for _,m in ipairs(macros_regex) do
          if not m.disabled and m.keyregex:match(key) then
            if argCheckOnly then return m, areaname end
            ExamineMacro(m, areaname)
          end
        end
      end
    end
  end
  if not next(Collector) then return end

  -- Filter macros by condition() where available; update dynamic priorities.
  -- Calculate maximal priority and number of macros left in the container.
  local max_priority = -1
  local nummacros = 0
  for m,p in pairs(Collector) do
    if m.condition then
      local pr = m.condition(argKey) -- unprotected call
      if pr then
        if type(pr)=="number" then
          CInfo[p] = pr>100 and 100 or pr<0 and 0 or pr
        end
      else
        Collector[m] = nil
      end
    end
    if Collector[m] then
      nummacros = nummacros + 1
      if max_priority < CInfo[p] then max_priority = CInfo[p] end
    end
  end
  if nummacros == 0 then return end

  -- If only 1 macro is left, do return it.
  if nummacros == 1 then
    local m = next(Collector)
    return m, CInfo[Collector[m]+1]
  end

  -- Make an array with highest priority macros.
  local macrolist = {}
  for m,p in pairs(Collector) do
    if CInfo[p]==max_priority then macrolist[#macrolist+1]=m end
  end
  if #macrolist == 1 then
    local m = macrolist[1]
    return m, CInfo[Collector[m]+1]
  end

  -- Make order of macros in the menu consistent
  table.sort(macrolist, function(m1,m2) return Collector[m1] < Collector[m2] end)

  local m = GetFromMenu(macrolist)
  if m then return m, CInfo[Collector[m]+1] end
  return {}, nil

end

local function GetMacroWrapper (...)
  local macro,area = GetMacro(...)
  if macro then
    LastMessage = macro.id and pack(macro.id, GetAreaCode(area), macro.code or "",
      macro.description or "", macro.flags) or pack(0)
    return F.MPRT_NORMALFINISH, LastMessage
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
    needsave=true
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
     -- MCTL_ADDMACRO may be called during LoadMacros execution, hence AddMacro_filename should be restored.
    local fname = AddMacro_filename
    AddMacro_filename = nil
    AddMacro { area=area, key=key, code=code, flags=flags, description=description,
               guid=guid, callback=callback, callbackId=callbackId }
    AddMacro_filename = fname
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
          return true
        end
      end
    end
  end
end

local function RunStartMacro()
  if not LoadMacrosDone then return end

  local mode = far.MacroGetArea()
  local opt = band(MacroCallFar(MCODE_F_GETOPTIONS),0x3)
  local mtable = opt==1 and Areas.editor or opt==2 and Areas.viewer or Areas.shell

  for _,macros in pairs(mtable) do
    local m = macros.recorded
    if m and not m.disabled and m.flags and band(m.flags,0x8)~=0 and not m.autostartdone then
      m.autostartdone=true
      if MacroCallFar(MCODE_F_CHECKALL, mode, m.flags) then
        MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags)
      end
    end
    for _,m in ipairs(macros) do
      if not m.disabled and m.flags and band(m.flags,0x8)~=0 and not m.autostartdone then
        m.autostartdone=true
        if MacroCallFar(MCODE_F_CHECKALL, mode, m.flags) then
          if not m.condition or m.condition() then
            MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags)
          end
        end
      end
    end
  end
  return true
end

local function GetMacroById (id)
  return LoadedMacros[id]
end

local function GetMacroCopy (id)
  if LoadedMacros[id] then
    local t={}
    for k,v in pairs(LoadedMacros[id]) do t[k]=v end
    return t
  end
  return nil
end

InitMacroSystem()

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
  UnloadMacros = InitMacroSystem,
  WriteMacros = WriteMacros,
  GetMacroCopy = GetMacroCopy,
  CheckFileName = CheckFileName,
  FlagsToString = FlagsToString,
}
