local Shared = ...
local Msg, ErrMsg, pack, ExpandEnv = Shared.Msg, Shared.ErrMsg, Shared.pack, Shared.ExpandEnv

local F = far.Flags
local type = type
local string_find, string_sub = string.find, string.sub
local band, bor = bit64.band, bit64.bor
local MacroCallFar = far.MacroCallFar
local gmeta = { __index=_G }
local LastMessage = {}
--------------------------------------------------------------------------------
local TrueAreaNames = {
  [F.MACROAREA_OTHER]                = "Other",
  [F.MACROAREA_SHELL]                = "Shell",
  [F.MACROAREA_VIEWER]               = "Viewer",
  [F.MACROAREA_EDITOR]               = "Editor",
  [F.MACROAREA_DIALOG]               = "Dialog",
  [F.MACROAREA_SEARCH]               = "Search",
  [F.MACROAREA_DISKS]                = "Disks",
  [F.MACROAREA_MAINMENU]             = "MainMenu",
  [F.MACROAREA_MENU]                 = "Menu",
  [F.MACROAREA_HELP]                 = "Help",
  [F.MACROAREA_INFOPANEL]            = "Info",
  [F.MACROAREA_QVIEWPANEL]           = "QView",
  [F.MACROAREA_TREEPANEL]            = "Tree",
  [F.MACROAREA_FINDFOLDER]           = "FindFolder",
  [F.MACROAREA_USERMENU]             = "UserMenu",
  [F.MACROAREA_SHELLAUTOCOMPLETION]  = "ShellAutoCompletion",
  [F.MACROAREA_DIALOGAUTOCOMPLETION] = "DialogAutoCompletion",
  [F.MACROAREA_COMMON]               = "Common",
}

local AllAreaNames = {}
for k,v in pairs(TrueAreaNames) do
  local str = v:lower()
  AllAreaNames[k] = str
  AllAreaNames[str] = k
end

local SomeAreaNames = {
  "other", "viewer", "editor", "dialog", "menu", "help", "dialogautocompletion",
  "common" -- "common" должен идти последним
}

local function GetTrueAreaName(Mode) return TrueAreaNames[Mode] end
local function GetAreaName(Mode)     return AllAreaNames[Mode] end
local function GetAreaCode(Area)     return AllAreaNames[Area:lower()] end
--------------------------------------------------------------------------------

local MCODE_F_CHECKALL     = 0x80C64
local MCODE_F_GETOPTIONS   = 0x80C65
Shared.OnlyEditorViewerUsed = band(MacroCallFar(MCODE_F_GETOPTIONS),0x3) ~= 0

local Areas
local LoadedMacros
local LoadMacrosDone
local LoadingInProgress
local EnumState = {}
local Events
local EventGroups = {"dialogevent","editorevent","editorinput","exitfar","viewerevent", "consoleinput"}
local AddedMenuItems
local AddedPrefixes

package.nounload = {lpeg=true}
local initial_modules = {}

local function FixInitialModules()
  for k in pairs(package.loaded) do initial_modules[k]=true end
end

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

  function StringToFlags (str, filename)
    local flags = 0
    if type(str) == "string" then
      for word in str:gmatch("[^ |]+") do
        local f = MacroFlagsByStr[word:lower()]
        if f then
          flags = bor(flags, f)
        else
          local line1 = filename or "<not a file>"
          local btn = filename and "OK;Edit" or "OK"
          if 2 == ErrMsg(line1.."\nInvalid macro flag: "..word, nil, btn) then
            editor.Editor(filename,nil,nil,nil,nil,nil,nil,1,nil,65001)
          end
        end
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

local function ReadIniFile (filename)
  local fp = io.open(filename)
  if not fp then return nil end

  local currsect = 1
  local t = { [currsect]={} }
  local numline = 0

  if fp:read(3) ~= "\239\187\191" then fp:seek("set",0) end -- skip UTF-8 BOM
  for line in fp:lines() do
    numline = numline + 1
    local sect = line:match("^%s*%[([^%]]+)%]%s*$")
    if sect then
      t[sect] = t[sect] or {}
      currsect = sect
    else
      local id,val = line:match("^%s*(%w+)%s*=%s*(.-)%s*$")
      if id then
        t[currsect][id] = val
      elseif not (line:match("^%s*;") or line:match("^%s*$")) then
        fp:close()
        return nil, (("%s:%d: invalid line in ini-file"):format(filename,numline))
      end
    end
  end
  fp:close()
  return t
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
    if ret then
      if macros==Events.dialogevent or macros==Events.editorinput or macros==Events.commandline then
        return ret
      elseif macros==Events.consoleinput then
        if ret ~= 0 then return ret end
      end
    end
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

function export.ProcessEditorEvent (EditorID, Event, Param)
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

local function export_ProcessConsoleInput (Rec, Flags)
  return EV_Handler(Events.consoleinput, nil, Rec, Flags)
end

local ExpandKey do -- измеренное время исполнения на ключе "CtrlAltShiftF12" = ??? (Lua); 2.3uS (LuaJIT);
  local t={}

  ExpandKey = function (key)
    local ctrl,alt,rest
    key = key:lower()
    local start = 1
    for k=1,3 do
      local from,to,word = string_find(key, "^([lr]?ctrl)", start)
      if from then ctrl = ctrl or word
      else
        from,to,word = string_find(key, "^([lr]?alt)", start)
        if from then alt = alt or word
        else
          from,to,word = string_find(key, "^(shift)", start)
          if from then rest = rest or word
          else break
          end
        end
      end
      start = to+1
    end
    rest = (rest or "")..string_sub(key, start)
    ctrl, alt = ctrl or "", alt or ""

    if ctrl=="ctrl" then
      if alt=="alt" then
        t[1] = "lctrllalt"..rest; t[2] = "lctrlralt"..rest
        t[3] = "rctrllalt"..rest; t[4] = "rctrlralt"..rest
        return t,4
      else
        t[1] = "lctrl"..alt..rest
        t[2] = "rctrl"..alt..rest
        return t,2
      end
    else
      if alt=="alt" then
        t[1] = ctrl.."lalt"..rest
        t[2] = ctrl.."ralt"..rest
        return t,2
      else
        t[1] = ctrl..alt..rest
        return t,1
      end
    end
  end
end

local function AddRegularMacro (srctable, FileName)
  local macro = {}
  if type(srctable)=="table" and type(srctable.area)=="string" then
    macro.area = srctable.area
    macro.key = type(srctable.key)=="string" and srctable.key or "none"
    if not macro.key:find("%S") then macro.key = "none" end
  else
    return
  end

  local keyregex, ok = macro.key:match("^/(.+)/$"), nil
  if keyregex then
    ok, macro.keyregex = pcall(regex.new, "^("..keyregex..")$", "i")
    if not ok then
      ErrMsg("Invalid regex: "..macro.key)
      return
    end
  end

  if type(srctable.action)=="function" then
    macro.action = srctable.action
  elseif type(srctable.code)=="string" then
    local isMoonScript = srctable.language=="moonscript"
    if srctable.code:sub(1,1) == "@" then
      macro.code = srctable.code
      macro.language = isMoonScript and "moonscript" or "lua"
    else
      local f, msg = (isMoonScript and require("moonscript").loadstring or loadstring)(srctable.code)
      if f then
        macro.action = f
      else
        if FileName then ErrMsg(msg, isMoonScript and "MoonScript") end
        return
      end
    end
  else
    return
  end

  local arFound = {} -- prevent multiple inclusions, i.e. area="Editor Editor"
  for a in srctable.area:lower():gmatch("%S+") do
    local arTable = Areas[a]
    if arTable and not arFound[a] then
      if macro.keyregex then
        arTable[1] = arTable[1] or {}
        table.insert(arTable[1], macro)
      else
        local keyFound = {} -- prevent multiple inclusions
        for k in macro.key:lower():gmatch("%S+") do
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
    macro.flags = StringToFlags(srctable.flags, FileName)

    if type(srctable.description)=="string" then macro.description=srctable.description end
    if type(srctable.condition)=="function" then macro.condition=srctable.condition end
    if type(srctable.filemask)=="string" then macro.filemask=srctable.filemask end

    local priority = srctable.priority
    if type(priority)=="number" then
      macro.priority = priority>100 and 100 or priority<0 and 0 or priority
    end

    if FileName then
      macro.FileName = FileName
    else
      macro.guid = srctable.guid
      macro.callback = srctable.callback
      macro.callbackId = srctable.callbackId
      macro.language = srctable.language
    end

    macro.id = #LoadedMacros+1
    LoadedMacros[macro.id] = macro
    return macro
  end
end

local CharNames = { ["."]="Dot", ["<"]="Less", [">"]="More", ["|"]="Pipe", ["/"]="Slash",
                    [":"]="Colon", ["?"]="Question", ["*"]="Asterisk", ['"']="Quote" }

local function AddRecordedMacro (srctable, filename)
  local area = srctable.area:lower()
  if not (area and Areas[area]) then return end
  local arTable = Areas[area]

  local key = srctable.key
  -- check correspondence between (a) filename and (b) area_key
  if ("%s_%s"):format(area, (key:gsub(".",CharNames))):lower() ~=
     filename:gsub("^.*\\",""):sub(1,-5):lower() then return
  end

  if srctable.code and srctable.code:sub(1,1) ~= "@" then
    local f, msg = loadstring(srctable.code)
    if not f then ErrMsg(msg) return end
  end

  local macro = { FileName=filename }
  local t,n = ExpandKey(key)
  for i=1,n do
    local normkey = t[i]
    arTable[normkey] = arTable[normkey] or {}
    arTable[normkey].recorded = macro
  end

  for _,v in ipairs{"area","key","action","code","description"} do macro[v]=srctable[v] end

  macro.flags = StringToFlags(srctable.flags, filename)
  if type(macro.description)~="string" then macro.description=nil end

  macro.id = #LoadedMacros+1
  LoadedMacros[macro.id] = macro
end

local AddEvent_fields = {"group","action","description","priority","condition","filemask"}
local function AddEvent (srctable, FileName)
  local group = type(srctable)=="table" and type(srctable.group)=="string" and srctable.group:lower()
  if not (group and Events[group]) then return end

  if type(srctable.action)~="function" then return end

  local macro={}
  table.insert(Events[group], macro)

  for _,v in ipairs(AddEvent_fields) do macro[v]=srctable[v] end
  macro.FileName = FileName

  if type(macro.description)~="string" then macro.description=nil end
  if type(macro.condition)~="function" then macro.condition=nil end
  if type(macro.filemask)~="string" then macro.filemask=nil end

  if type(macro.priority)~="number" then macro.priority=50
  elseif macro.priority>100 then macro.priority=100 elseif macro.priority<0 then macro.priority=0
  end

  macro.id = #LoadedMacros+1
  LoadedMacros[macro.id] = macro
  return macro
end

local function AddMenuItem (srctable, FileName)
  if type(srctable)=="table" and
     type(srctable.menu)=="string" and
     type(srctable.guid)=="string" and
     (type(srctable.text)=="function" or type(srctable.text)=="string") and
     type(srctable.action)=="function"
  then
    local item = {}
    item.guid = win.Uuid(srctable.guid)
    if item.guid and #item.guid==16 and not AddedMenuItems[item.guid] then
      item.flags = {}
      for w in srctable.menu:lower():gmatch("%S+") do
        if w=="plugins" or w=="disks" or w=="config" then item.flags[w]=true end
      end
      if type(srctable.area)=="string" then
        for w in srctable.area:lower():gmatch("%S+") do
          if w == "common" then
            item.flags[w]=true
          else
            local code = GetAreaCode(w)
            if code then item.flags[code]=true end
          end
        end
      end
      local text = srctable.text
      item.text = type(text)=="function" and text or function() return text end
      item.action = srctable.action
      item.description = type(srctable.description)=="string" and srctable.description or ""
      item.FileName = FileName
      item.id = #AddedMenuItems + 1
      AddedMenuItems[item.id] = item
      AddedMenuItems[item.guid] = item
      return true
    end
  end
  return false
end

local function AddPrefixes (srctable, FileName)
  local result = 0
  if type(srctable)=="table" and
     type(srctable.prefixes)=="string" and
     type(srctable.action)=="function"
  then
    for prefix in srctable.prefixes:lower():gmatch("[^:]+") do
      if prefix:match("^%S+$") and not AddedPrefixes[prefix] then
        local item = {
          prefix = prefix,
          action = srctable.action,
          description = type(srctable.description)=="string" and srctable.description or "",
          FileName = FileName
        }
        AddedPrefixes[prefix] = item
        AddedPrefixes[1] = AddedPrefixes[1]..":"..prefix
        result = result + 1
      end
    end
  end
  return result
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
        if not macro.disabled and macro.area and macro.area:lower():find(area) then
          LastMessage = pack(macro.key, macro.description or "")
          return LastMessage
        end
      else
        EnumState.index = 0
        break
      end
    end
  end
end

local function GetMoonscriptLineNumber (filename, line)
  local line_tables = require("moonscript.line_tables")
  local errors = require("moonscript.errors")
  local cache = {}
  local table = line_tables["@"..filename]
  if table then
    return errors.reverse_line_number(filename, table, line, cache)
  end
end

-- ...nager\unicode_far\CommonProfile\Macros\scripts\test1.lua:9:
-- attempt to perform arithmetic on a nil value

local function ErrMsgLoad (msg, filename, isMoonScript, mode)
  local title = isMoonScript and mode=="compile" and "MoonScript" or "LuaMacro"

  if type(msg)~="string" and type(msg)~="number" then
    ErrMsg(filename..":\n<non-string error message>", title, nil, "w")
    return
  end

  if mode=="run" then
    local found = false
    local fname,line = msg:match("^(.-):(%d+):")
    if fname then
      line = tonumber(line)
      if string_sub(fname,1,3) ~= "..." then
        found = true
      else
        fname = string_sub(fname,4)
        -- for k=1,5 do
        --   if fname:utf8valid() then break end
        --   fname = string_sub(fname,2)
        -- end
        fname = fname:gsub("/", "\\")
        local middle = fname:match([=[^[^\\]*\[^\\]+\]=])
        if middle then
          local from = string_find(filename:lower(), middle:lower(), 1, true)
          if from then
            fname = string_sub(filename,1,from-1) .. fname
            local attr = win.GetFileAttr(fname)
            found = attr and not attr:find("d")
          end
        end
      end
    end
    if found then
      if 2 == ErrMsg(msg, title, "OK;Edit", "wl") then
        if isMoonScript then line = GetMoonscriptLineNumber(fname,line) end
        editor.Editor(fname,nil,nil,nil,nil,nil,nil,line or 1,nil,65001)
      end
    else
      ErrMsg(msg, title, "OK", "wl")
    end
  else
    if 2 == far.Message(msg, title, "OK;Edit", "wl") then
      local pattern = isMoonScript and "%[(%d+)%] >>" or "^[^\n]-:(%d+):"
      local line = tonumber(msg:match(pattern))
      if line and isMoonScript and mode=="run" then line = GetMoonscriptLineNumber(filename,line) end
      editor.Editor(filename,nil,nil,nil,nil,nil,nil,line or 1,nil,65001)
    end
  end
end

local function LoadMacros (unload, paths)
  if LoadingInProgress then return end
  LoadingInProgress = true

  if LoadMacrosDone then
    local ok, msg = pcall(export_ExitFAR)
    if not ok then ErrMsg(msg) end
    LoadMacrosDone = false
  end

  export.ExitFAR = nil
  export.ProcessDialogEvent = nil
  export.ProcessEditorInput = nil
  export.ProcessViewerEvent = nil
  export.ProcessConsoleInput = nil

  local allAreas = band(MacroCallFar(MCODE_F_GETOPTIONS),0x3) == 0
  local numerrors=0
  local newAreas = {}
  Events = {}
  EnumState = {}
  LoadedMacros = {}
  AddedMenuItems = {}
  AddedPrefixes = { [1]="" }
  if Shared.panelsort then Shared.panelsort.DeleteSortModes() end

  local AreaNames = allAreas and AllAreaNames or SomeAreaNames
  for _,name in pairs(AreaNames) do newAreas[name]={} end
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
    local DirMacros = win.GetEnv("farprofile").."\\Macros"
    if 0 == band(MacroCallFar(MCODE_F_GETOPTIONS),0x10) then -- not ReadOnlyConfig
      win.CreateDir(DirMacros.."\\scripts", true)
      win.CreateDir(win.GetEnv("farprofile").."\\Menus", true)
    end

    local moonscript = require "moonscript"

    local function LoadRegularFile (FindData, FullPath, macroinit)
      if FindData.FileAttributes:find("d") then return end
      if macroinit and #FullPath==#macroinit and far.LStricmp(FullPath,macroinit)==0 then
        return
      end
      local isMoonScript = string_find(FullPath, "[nN]", -1)
      local f, msg = (isMoonScript and moonscript.loadfile or loadfile)(FullPath)
      if not f then
        numerrors=numerrors+1
        ErrMsgLoad(msg,FullPath,isMoonScript,"compile")
        return
      end
      local env = {
        Macro = function(t) return not not AddRegularMacro(t,FullPath) end;
        Event = function(t) return not not AddEvent(t,FullPath) end;
        MenuItem = function(t) return AddMenuItem(t,FullPath) end;
        CommandLine = function(t) return AddPrefixes(t,FullPath) end;
        NoMacro=DummyFunc, NoEvent=DummyFunc, NoMenuItem=DummyFunc, NoCommandLine=DummyFunc }
      setmetatable(env,gmeta)
      setfenv(f, env)
      local ok, msg = pcall(f, FullPath)
      if ok then
        env.Macro, env.Event, env.MenuItem, env.CommandLine,
        env.NoMacro, env.NoEvent, env.NoMenuItem, env.NoCommandLine = nil
      else
        numerrors=numerrors+1
        ErrMsgLoad(msg,FullPath,isMoonScript,"run")
      end
    end

    local tempRecordedMacro
    local function ReadRecordedMacro (m)
      if tempRecordedMacro == nil then
        if type(m) == "table" then
          local t_action, t_code = type(m.action), type(m.code)
          tempRecordedMacro = type(m.area)=="string" and type(m.key)=="string" and
            (t_action=="function" and t_code=="nil" or t_action=="nil" and t_code=="string") and m
        else
          tempRecordedMacro = false
        end
      end
    end

    local function LoadRecordedFile (FindData, FullPath)
      if FindData.FileAttributes:find("d") then return end
      local f, msg = loadfile(FullPath)
      if not f then
        numerrors=numerrors+1; ErrMsg(msg); return
      end
      local env = setmetatable({ Macro=ReadRecordedMacro }, gmeta)
      setfenv(f, env)
      tempRecordedMacro = nil
      local ok, msg = pcall(f)
      if ok then
        env.Macro = nil
        if tempRecordedMacro==nil then -- support old format (support till 2015-05-28)
          ReadRecordedMacro(env)
        end
        if tempRecordedMacro then AddRecordedMacro(tempRecordedMacro, FullPath) end
      else
        numerrors=numerrors+1; ErrMsg(msg)
      end
    end

    if paths then
      paths = ExpandEnv(paths)
    else
      paths = DirMacros.."\\scripts"
      local cfg, msg = ReadIniFile(far.PluginStartupInfo().ModuleDir.."luamacro.ini")
      if cfg then
        if cfg.General then
          local p = cfg.General.MacroPath
          if p then paths = ExpandEnv(p) end
        end
      else
        if msg then ErrMsg(msg) end
      end
    end

    for p in paths:gmatch("[^;]+") do
      p = far.ConvertPath(p, F.CPM_FULL) -- needed for relative paths
      local macroinit = p:gsub("[\\/]*$", "\\_macroinit.lua")
      local info = win.GetFileInfo(macroinit)
      if info and not info.FileAttributes:find("d") then
        LoadRegularFile(info, macroinit, nil)
      else
        macroinit = nil
      end
      far.RecursiveSearch (p, "*.lua,*.moon", LoadRegularFile, bor(F.FRS_RECUR,F.FRS_SCANSYMLINK), macroinit)
    end

    far.RecursiveSearch (DirMacros.."\\internal", "*.lua", LoadRecordedFile, 0)

    export.ExitFAR = Events.exitfar[1] and export_ExitFAR
    export.ProcessDialogEvent = Events.dialogevent[1] and export_ProcessDialogEvent
    export.ProcessEditorInput = Events.editorinput[1] and export_ProcessEditorInput
    export.ProcessViewerEvent = Events.viewerevent[1] and export_ProcessViewerEvent
    export.ProcessConsoleInput = Events.consoleinput[1] and export_ProcessConsoleInput

    LoadMacrosDone = true
  end

  LoadingInProgress = nil
  return numerrors==0
end

local function InitMacroSystem()
  LoadMacros(true)
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
    fp:write(([[
Macro {
  description=%q;
  area=%q; key=%q;
  flags=%q;
  code=%q;
}
]]):format(macro.description, macro.area, macro.key, FlagsToString(macro.flags), macro.code))
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
    menuitems[i] = { text=descr, macro=macro }
  end

  table.sort(menuitems, function(a,b) return far.LStricmp(a.text, b.text) < 0 end)

  for i,item in ipairs(menuitems) do
    local ch = i<10 and i or i<36 and string.char(i+55)
    item.text = ch and (ch..". "..item.text) or item.text
  end

  local props, bkeys = {
      Title = Msg.UtExecuteMacroTitle, Bottom = Msg.UtExecuteMacroBottom,
      Flags = { FMENU_AUTOHIGHLIGHT=1, FMENU_WRAPMODE=1, FMENU_CHANGECONSOLETITLE=1 },
      Id = win.Uuid("165AA6E3-C89B-4F82-A0C5-C309243FD21B") }, { {BreakKey="A+F4"} }
  while true do
    local item, pos = far.Menu(props, menuitems, bkeys)
    if not item then
      return
    elseif item.macro then
      return item.macro
    elseif item.BreakKey == "A+F4" then
      props.SelectIndex = pos
      local m = menuitems[pos].macro
      if m.FileName then
        local startline = m.action and debug.getinfo(m.action,"S").linedefined
        editor.Editor(m.FileName,nil,nil,nil,nil,nil,nil,startline,nil,65001)
      end
    end
  end
end

local function GetMacro (argMode, argKey, argUseCommon, argCheckOnly)
  if LoadingInProgress then return end

  local area = GetAreaName(argMode)
  if not area then return end -- трюк используется в CheckForEscSilent() в Фаре

  local key = argKey:lower()
  do
    local alt
    local from,to,ctrl = string_find(key, "^(r?ctrl)")
    if ctrl=="ctrl" then ctrl="lctrl" end
    local start = to and to+1 or 1
    from,to,alt = string_find(key, "^(r?alt)", start)
    if alt=="alt" then alt="lalt" end
    start = to and to+1 or start
    key = (ctrl or "") .. (alt or "") .. string_sub(key, start)
  end

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
        CInfo[n] = m.priority or 50
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

local function GetMacroWrapper (argMode, argKey, argUseCommon)
  local macro,area = GetMacro(argMode, argKey, argUseCommon, true)
  if macro then
    LastMessage = pack(GetAreaCode(area), macro.code or "", macro.description or "", macro.flags)
    return LastMessage
  end
end

local function ProcessRecordedMacro (Mode, Key, code, flags, description)
  local Area = GetTrueAreaName(Mode)
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

local function AddMacroFromFAR (mode, key, lang, code, flags, description, guid, callback, callbackId)
  local area = GetTrueAreaName(mode)
  local m = AddRegularMacro { area=area, key=key, code=code, flags=flags, description=description,
                              guid=guid, callback=callback, callbackId=callbackId, language=lang }
  local action = m and m.action
  if action then
    local env = setmetatable({}, gmeta)
    setfenv(action, env)
  end
  return not not m
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

  for k=1,2 do
    if k==2 then mtable = Areas.common end
    for _,macros in pairs(mtable) do
      local m = macros.recorded
      if m and not m.disabled and m.flags and band(m.flags,0x8)~=0 and not m.autostartdone then
        m.autostartdone=true
        if MacroCallFar(MCODE_F_CHECKALL, mode, m.flags) then
          Shared.keymacro.PostNewMacro(m, m.flags, nil, true)
        end
      end
      for _,m in ipairs(macros) do
        if not m.disabled and m.flags and band(m.flags,0x8)~=0 and not m.autostartdone then
          m.autostartdone=true
          if MacroCallFar(MCODE_F_CHECKALL, mode, m.flags) then
            if not m.condition or m.condition() then
              Shared.keymacro.PostNewMacro(m, m.flags, nil, true)
            end
          end
        end
      end
    end
  end
  return true
end

local function GetMacroCopy (id)
  if LoadedMacros[id] then
    local t={}
    for k,v in pairs(LoadedMacros[id]) do t[k]=v end
    return t
  end
  return nil
end

return {
  AddMacroFromFAR = AddMacroFromFAR,
  CheckFileName = CheckFileName,
  DelMacro = DelMacro,
  EnumMacros = EnumMacros,
  FixInitialModules = FixInitialModules,
  FlagsToString = FlagsToString,
  GetAreaCode = GetAreaCode,
  GetMacro = GetMacro,
  GetMacroCopy = GetMacroCopy,
  GetMacroWrapper = GetMacroWrapper,
  GetMenuItems = function() return AddedMenuItems end,
  GetMoonscriptLineNumber = GetMoonscriptLineNumber,
  GetPrefixes = function() return AddedPrefixes end,
  GetTrueAreaName = GetTrueAreaName,
  InitMacroSystem = InitMacroSystem,
  LoadingInProgress = function() return LoadingInProgress end,
  LoadMacros = LoadMacros,
  ProcessRecordedMacro = ProcessRecordedMacro,
  RunStartMacro = RunStartMacro,
  UnloadMacros = InitMacroSystem,
  WriteMacros = WriteMacros,
}
