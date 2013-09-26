-- encoding: UTF-8

local args = ...
local M, utils = args.M, args.utils

local F = far.Flags
local MCODE_F_POSTNEWMACRO = 0x80C64
local MCODE_F_CHECKALL     = 0x80C65
local ceil, max = math.ceil, math.max
local LStricmp = far.LStricmp -- consider win.CompareString ?
local Title = "Macro Browser"
local Message = function (str,title,...) return far.Message(str,title or Title,...) end

local areaCodes = {
  other="O", shell="S", viewer="V", editor="E", dialog="D", search="S",
  disks="D", mainmenu="M", menu="M", help="H", info="I", qview="Q", tree="T",
  findfolder="F", usermenu="U", shellautocompletion="S",
  dialogautocompletion="D", common="C",
}

local areaArr = {
  "other", "shell", "viewer", "editor", "dialog", "search",
  "disks", "mainmenu", "menu", "help", "info", "qview", "tree",
  "findfolder", "usermenu", "shellautocompletion",
  "dialogautocompletion", "common",
}
-- O S V E D S D M M H I Q T F U S D C
--   1     2 1 2 3 3             1 2

local function GetItems (fcomp, sortmark, onlyactive)
  local currArea = areaArr[1+far.MacroGetArea()]
  local events,macros,items={},{},{}
  local maxKeyW, maxKeyLen do
    local farRect = far.AdvControl("ACTL_GETFARRECT")
    local farWidth = farRect.Right - farRect.Left + 1
    maxKeyW = max(4, ceil(farWidth-36)/2)
    maxKeyLen = 0
  end
  for k=1,math.huge do
    local m = utils.GetMacroCopy(k)
    if not m then break end
    if m.area then
      if not m.disabled then
        local ars,s = {},""
        m.area:lower():gsub("[^ ]+", function(c) ars[c]=true end)
        if ars[currArea] or ars.common then m.active=true end
        if m.active or not onlyactive then
          for i,v in ipairs(areaArr) do
            s=s..(ars[v] and areaCodes[v] or ".")
          end
          m.codedArea=s
          m.description=m.description or "id="..m.id
          macros[#macros+1]=m
          local keylen = m.key:len()
          m.codedKey = keylen<=maxKeyW and m.key or m.key:sub(1,maxKeyW-3).."..."
          maxKeyLen = max(maxKeyLen, m.codedKey:len())
        end
      end
    else
      m.description=m.description or "id="..m.id
      events[#events+1]=m
    end
  end

  table.sort(macros, fcomp)
  table.sort(events, function(a,b) return a.group < b.group end)

  items[#items+1] = {
    separator=true,
    text=("%s [ %s ]"):format(onlyactive and M.MBSepActiveMacros or M.MBSepMacros, sortmark) }

  local fmt = ("%%s %%s │ %%-%ds │ %%s"):format(maxKeyLen)
  for i,m in ipairs(macros) do
    items[#items+1] = { text=fmt:format(m.active and "√" or " ", m.codedArea, m.codedKey, m.description), macro=m }
  end

  items[#items+1] = { separator=true, text=M.MBSepEvents }

  for i,m in ipairs(events) do
    items[#items+1] = { text=("%-19s │ %s"):format(
                        m.group, m.description), macro=m, }
  end

  return items
end

local function LocateFile (fname)
  local attr = win.GetFileAttr(fname)
  if attr and not attr:find"d" then
    local dir, name = fname:match("^(.*\\)([^\\]*)$")
    if panel.SetPanelDirectory(nil, 1, dir) then
      local pinfo = panel.GetPanelInfo(nil, 1)
      for i=1, pinfo.ItemsNumber do
        local item = panel.GetPanelItem(nil, 1, i)
        if item.FileName == name then
          local rect = pinfo.PanelRect
          local hheight = math.floor((rect.bottom - rect.top - 4) / 2)
          local topitem = pinfo.TopPanelItem
          panel.RedrawPanel(nil, 1, { CurrentItem = i,
            TopPanelItem = i>=topitem and i<topitem+hheight and topitem or
                           i>hheight and i-hheight or 0 })
          return true
        end
      end
    end
  end
  return false
end

local CmpFuncs = {
  ["C+F1"] = { function (a,b) return LStricmp(a.codedArea,b.codedArea) > 0 end,     -- CompArea
               function (a,b) return LStricmp(a.codedArea,b.codedArea) < 0 end, "1↑", "1↓" },
  ["C+F2"] = { function (a,b) return LStricmp(a.key,b.key) < 0 end,                 -- CompKey
               function (a,b) return LStricmp(a.key,b.key) > 0 end, "2↑", "2↓" },
  ["C+F3"] = { function (a,b) return LStricmp(a.description,b.description) < 0 end, -- CompDescr
               function (a,b) return LStricmp(a.description,b.description) > 0 end, "3↑", "3↓" },
}

local Data = mf.mload("LuaMacro", "MacroBrowser")
local SortKey = Data and Data.SortKey or "C+F1"
local InvSort = Data and Data.InvSort or 1
local ShowOnlyActive = Data and Data.ShowOnlyActive

local function ShowHelp()
  far.Message(
    ("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s"):format(
      M.MBHelpLine1, M.MBHelpLine2, M.MBHelpLine3, M.MBHelpLine4, M.MBHelpLine5,
      M.MBHelpLine6, M.MBHelpLine7, M.MBHelpLine8, M.MBHelpLine9, M.MBHelpLine10),
    Title, nil, "l")
end

local function ShowInfo (m)
  if m.area then
    local code = m.code and m.code:gsub("\r?\n"," ") or ""
    if code:len() > 50 then code = code:sub(1,47).."..." end

    local str = ([[
description │ %s
area        │ %s
key         │ %s
flags       │ %s
filemask    │ %s
priority    │ %s
condition   │ %s
action      │ %s
code        │ %s
%s
%s]]) :format(m.description or "id="..m.id,
              m.area,
              m.key,
              utils.FlagsToString(m.flags),
              m.filemask or "", m.priority or "",
              m.condition and tostring(m.condition) or "",
              m.action and tostring(m.action) or "",
              code,
              "\1",
              m.FileName or "<"..M.MBNoFileNameAvail..">")
    far.Message(str,M.MBTitleMacro,nil,"l")
  else
    local str = ([[
description │ %s
group       │ %s
filemask    │ %s
priority    │ %s
condition   │ %s
action      │ %s
%s
%s]]) :format(m.description or "",
              m.group,
              m.filemask or "",
              m.priority or "",
              m.condition and tostring(m.condition) or "",
              m.action and tostring(m.action) or "",
              "\1",
              m.FileName)
    far.Message(str,M.MBTitleEventHandler,nil,"l")
  end
end

local function MenuLoop()
--far.MacroLoadAll()

  local farRect = far.AdvControl("ACTL_GETFARRECT")
  local props = {
    Title = Title,
    Flags = {FMENU_SHOWAMPERSAND=1,FMENU_WRAPMODE=1,FMENU_CHANGECONSOLETITLE=1},
    MaxHeight = farRect.Bottom - farRect.Top - 6,
  }

  local bkeys = {
    {BreakKey="F1"},{BreakKey="F3"},{BreakKey="F4"},{BreakKey="A+F4"},{BreakKey="C+H"},{BreakKey="C+PRIOR"},
  }
  for k in pairs(CmpFuncs) do bkeys[#bkeys+1] = {BreakKey=k} end

  assert(CmpFuncs[SortKey][InvSort])

  while true do
    local items = GetItems(CmpFuncs[SortKey][InvSort], CmpFuncs[SortKey][InvSort+2], ShowOnlyActive)
    local item, pos = far.Menu(props, items, bkeys)
    if not item then break end
    props.SelectIndex = pos
    local BrKey = item.BreakKey
    ----------------------------------------------------------------------------
    if BrKey == nil then -- execute
      local m = item.macro
      if m.area then
        if m.active then
          local check = true
          local area = far.MacroGetArea()
          if m.filemask then
            local filename
            if     area==F.MACROAREA_EDITOR then filename=editor.GetFileName()
            elseif area==F.MACROAREA_VIEWER then filename=viewer.GetFileName()
            end
            check = filename and utils.CheckFileName(m.filemask, filename)
          end
          if check then
            if far.MacroCallFar(MCODE_F_CHECKALL, area, m.flags, m.callback, m.callbackId) then
              if not m.keyregex then
                local key1 = m.key:match("%S+")
                if (not m.condition or m.condition(key1)) then
                  far.MacroCallFar(MCODE_F_POSTNEWMACRO, m.id, m.code, m.flags, key1)
                  break
                else Message("condition() check failed")
                end
              else Message("cannot guess a key when the macro has regex key specification")
              end
            else Message("flags check failed")
            end
          else Message("filemask check failed")
          end
        else Message("attempt to execute macro from wrong area")
        end
      else Message("attempt to execute event")
      end
    ----------------------------------------------------------------------------
    elseif CmpFuncs[BrKey] then -- sort
      if BrKey == SortKey then InvSort = InvSort==1 and 2 or 1
      else SortKey, InvSort = BrKey, 1
      end
      props.SelectIndex = 1
    ----------------------------------------------------------------------------
    elseif BrKey=="F1" then
      ShowHelp()
    ----------------------------------------------------------------------------
    elseif BrKey=="F3" and items[pos] then
      ShowInfo(items[pos].macro)
    ----------------------------------------------------------------------------
    elseif BrKey=="C+H" then -- hide inactive macros
      ShowOnlyActive = not ShowOnlyActive
      props.SelectIndex = nil
    ----------------------------------------------------------------------------
    elseif (BrKey=="F4" or BrKey=="A+F4") and items[pos] then -- edit
      local m = items[pos].macro
      if m.FileName then
        local startline = m.action and debug.getinfo(m.action,"S").linedefined
        if BrKey=="A+F4" then -- modal editor
          editor.Editor(m.FileName,nil,nil,nil,nil,nil,nil,startline,nil,65001)
        elseif BrKey=="F4" then -- non-modal editor
          local a = far.MacroGetArea()
          if a==F.MACROAREA_SHELL or a==F.MACROAREA_EDITOR or a==F.MACROAREA_VIEWER then
            local flags = {EF_NONMODAL=1,EF_IMMEDIATERETURN=1}
            editor.Editor(m.FileName,nil,nil,nil,nil,nil,flags,startline,nil,65001)
            break
          end
        end
      else
        Message(M.MBNoFileNameAvail)
      end
    ----------------------------------------------------------------------------
    elseif BrKey=="C+PRIOR" and items[pos] then -- CtrlPgUp - locate the file in active panel
      local m = items[pos].macro
      if m.FileName then
        if LocateFile(m.FileName) then
          break
        else
          Message(M.MBFileNotFound)
        end
      else
        Message(M.MBNoFileNameAvail)
      end
    ----------------------------------------------------------------------------
    end
  end
  mf.msave("LuaMacro", "MacroBrowser",
    { SortKey=SortKey, InvSort=InvSort, ShowOnlyActive=ShowOnlyActive })
end

local function export_ProcessSynchroEvent (event,param)
  export.ProcessSynchroEvent = nil
  MenuLoop()
end

-- This function is needed to make far.MacroGetArea work properly:
-- otherwise it returns 8 (MACROAREA_MENU) when called from Plugins Menu.
local function RunMenuLoop()
  export.ProcessSynchroEvent = export_ProcessSynchroEvent
  far.AdvControl("ACTL_SYNCHRO",0)
end

return RunMenuLoop
