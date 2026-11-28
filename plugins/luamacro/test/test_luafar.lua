-- test_luafar.lua
local function LoadMod(name)
  return assert(loadfile(far.PluginStartupInfo().ModuleDir.."test\\"..name..".lua")) ()
end

local asrt = LoadMod "assert"
local sd   = LoadMod "simpledialog"

local LF = {}
local F = far.Flags
local band, bor, bnot = bit64.band, bit64.bor, bit64.bnot
local luamacroId = far.GetPluginId()
local TKEY_BINARY = "__binary"

local TmpFileName = win.JoinPath(assert(win.GetEnv"TEMP" or win.GetEnv"TMP"), "tmp.tmp")

local function WriteTmpFile(...)
  local fp = assert(io.open(TmpFileName,"w"))
  fp:write(...)
  fp:close()
end

local function DeleteTmpFile()
  win.DeleteFile(TmpFileName)
end

local function test_ACTL()
  asrt.eq    ( actl.Commit(), 1)
  local arr = asrt.table ( actl.GetArrayColor())
  asrt.range ( #arr, 147)
  asrt.table ( actl.GetColor("COL_DIALOGBOXTITLE"))
  asrt.table ( actl.GetCursorPos())
  asrt.udata ( actl.GetFarHwnd())
  asrt.str   ( actl.GetFarManagerVersion())
  asrt.num   ( actl.GetFarManagerVersion(true))
  asrt.table ( actl.GetFarRect())
  asrt.eq    ( actl.GetWindowCount(), 2)
  asrt.table ( actl.GetWindowInfo(2))
  asrt.func  ( actl.ProgressNotify)
  asrt.func  ( actl.Quit)
  asrt.func  ( actl.RedrawAll)
  asrt.func  ( actl.SetArrayColor)
  asrt.func  ( actl.SetCurrentWindow)
  asrt.func  ( actl.SetCursorPos)
  asrt.func  ( actl.SetProgressState)
  asrt.func  ( actl.SetProgressValue)
  asrt.func  ( actl.Synchro)
  asrt.func  ( actl.WaitKey)
end

function LF.test_AdvControl()
  LF.test_AdvControl_Window()
  LF.test_AdvControl_Colors()
  LF.test_AdvControl_Synchro()
  LF.test_AdvControl_Misc()
  test_ACTL()
end

function LF.test_far_GetMsg()
  asrt.str (far.GetMsg(0))
end

local function test_far_MacroExecute()
  local i1 = bit64.new("0x8765876587658765")
  local a1,a2,a3,a4,a5,a6 = "foo", false, 5, nil, i1, {[TKEY_BINARY]="bar"}
  local function test(code, flags)
    local r = far.MacroExecute(code, flags, a1, a2, a3, a4, a5, a6)
    asrt.table (r)
    asrt.eq    (r.n,  6)
    asrt.eq    (r[1], a1)
    asrt.eq    (r[2], a2)
    asrt.eq    (r[3], a3)
    asrt.eq    (r[4], a4)
    asrt.eq    (r[5], a5)
    asrt.table (r[6])
    asrt.eq    (r[6][TKEY_BINARY], a6[TKEY_BINARY])
  end
  test("return ...", nil)
  test("return ...", "KMFLAGS_LUA")
  test("...", "KMFLAGS_MOONSCRIPT")
end

local function test_far_MacroAdd()
  local area, key, descr = "MACROAREA_SHELL", "CtrlA", "Test MacroAdd"

  local Id = far.MacroAdd(area, nil, key, [[A = { b=5 }]], descr)
  asrt.istrue(far.MacroDelete(asrt.udata(Id)))

  Id = far.MacroAdd(-1, nil, key, [[A = { b=5 }]], descr)
  asrt.isnil(Id) -- bad area

  Id = far.MacroAdd(area, nil, key, [[A = { b:5 }]], descr)
  asrt.isnil(Id) -- bad code

  Id = far.MacroAdd(area, "KMFLAGS_MOONSCRIPT", key, [[A = { b:5 }]], descr)
  asrt.istrue(far.MacroDelete(asrt.udata(Id)))

  Id = far.MacroAdd(area, "KMFLAGS_MOONSCRIPT", key, [[A = { b=5 }]], descr)
  asrt.isnil(Id) -- bad code

  Id = far.MacroAdd(area, nil, key, [[@c:\myscript 5+6,"foo"]], descr)
  asrt.istrue(far.MacroDelete(asrt.udata(Id)))

  Id = far.MacroAdd(area, nil, key, [[@c:\myscript 5***6,"foo"]], descr)
  asrt.istrue(far.MacroDelete(asrt.udata(Id))) -- with @ there is no syntax check till the macro runs

  Id = far.MacroAdd(nil, nil, key, [[@c:\myscript]])
  asrt.istrue(far.MacroDelete(asrt.udata(Id))) -- check default area (MACROAREA_COMMON)

  Id = far.MacroAdd(area,nil,key,[[Keys"F7" assert(Dlg.Id=="FAD00DBE-3FFF-4095-9232-E1CC70C67737") Keys"Esc"]],descr)
  asrt.eq(0, mf.eval("Shell/"..key, 2))
  asrt.istrue(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) return false end)
  asrt.eq(-2, mf.eval("Shell/"..key, 2))
  asrt.istrue(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) error"deliberate error" end)
  asrt.eq(-2, mf.eval("Shell/"..key, 2))
  asrt.istrue(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) return id==Id end)
  asrt.eq(0, mf.eval("Shell/"..key, 2))
  asrt.istrue(far.MacroDelete(Id))

end

local function test_far_MacroCheck()
  asrt.istrue(far.MacroCheck([[A = { b=5 }]]))
  asrt.istrue(far.MacroCheck([[A = { b=5 }]], "KMFLAGS_LUA"))

  asrt.isfalse(far.MacroCheck([[A = { b:5 }]], "KMFLAGS_SILENTCHECK"))

  asrt.istrue(far.MacroCheck([[A = { b:5 }]], "KMFLAGS_MOONSCRIPT"))

  asrt.isfalse(far.MacroCheck([[A = { b=5 }]], {KMFLAGS_MOONSCRIPT=1,KMFLAGS_SILENTCHECK=1} ))

  WriteTmpFile [[A = { b=5 }]] -- valid Lua, invalid MoonScript
  asrt.istrue (far.MacroCheck("@"..TmpFileName, "KMFLAGS_LUA"))
  asrt.istrue (far.MacroCheck("@"..TmpFileName.." 5+6,'foo'", "KMFLAGS_LUA")) -- valid file arguments
  asrt.isfalse(far.MacroCheck("@"..TmpFileName.." 5***6,'foo'", "KMFLAGS_SILENTCHECK")) -- invalid file arguments
  asrt.isfalse(far.MacroCheck("@"..TmpFileName, {KMFLAGS_MOONSCRIPT=1,KMFLAGS_SILENTCHECK=1}))

  WriteTmpFile [[A = { b:5 }]] -- invalid Lua, valid MoonScript
  asrt.isfalse (far.MacroCheck("@"..TmpFileName, "KMFLAGS_SILENTCHECK"))
  asrt.istrue  (far.MacroCheck("@"..TmpFileName, "KMFLAGS_MOONSCRIPT"))
  DeleteTmpFile()

  asrt.isfalse (far.MacroCheck([[@//////]], "KMFLAGS_SILENTCHECK"))
end

local function test_far_MacroGetArea()
  asrt.eq(far.MacroGetArea(), F.MACROAREA_SHELL)
end

local function test_far_MacroGetLastError()
  asrt.istrue  (far.MacroCheck("a=1"))
  asrt.eq    (far.MacroGetLastError().ErrSrc, "")
  asrt.isfalse (far.MacroCheck("a=", "KMFLAGS_SILENTCHECK"))
  assert       (far.MacroGetLastError().ErrSrc:len() > 0)
end

local function test_far_MacroGetState()
  local st = far.MacroGetState()
  assert(st==F.MACROSTATE_EXECUTING or st==F.MACROSTATE_EXECUTING_COMMON)
end

function LF.test_MacroControl()
  test_far_MacroAdd()
  test_far_MacroCheck()
  test_far_MacroExecute()
  test_far_MacroGetArea()
  test_far_MacroGetLastError()
  test_far_MacroGetState()
end

function LF.test_RegexControl()
  local L = win.Utf8ToUtf16
  local pat = "([bc]+)"
  local pat2 = "([bc]+)|(zz)"
  local rep = "%1%1"
  local R = regex.new(pat)
  local R2 = regex.new(pat2)

  local fr,to,cap
  local str, nfound, nrep

  asrt.eq(R:bracketscount(), 2)

  fr,to,cap = regex.find("abc", pat)
  assert(fr==2 and to==3 and cap=="bc")
  fr,to,cap = regex.findW(L"abc", pat)
  assert(fr==2 and to==3 and cap==L"bc")

  fr,to,cap = R:find("abc")
  assert(fr==2 and to==3 and cap=="bc")
  fr,to,cap = R:findW(L"abc")
  assert(fr==2 and to==3 and cap==L"bc")

  fr,to,cap = regex.exec("abc", pat2)
  assert(fr==2 and to==3 and #cap==4 and cap[1]==2 and cap[2]==3 and cap[3]==false and cap[4]==false)
  fr,to,cap = regex.execW(L"abc", pat2)
  assert(fr==2 and to==3 and #cap==4 and cap[1]==2 and cap[2]==3 and cap[3]==false and cap[4]==false)

  fr,to,cap = R2:exec("abc")
  assert(fr==2 and to==3 and #cap==4 and cap[1]==2 and cap[2]==3 and cap[3]==false and cap[4]==false)
  fr,to,cap = R2:execW(L"abc")
  assert(fr==2 and to==3 and #cap==4 and cap[1]==2 and cap[2]==3 and cap[3]==false and cap[4]==false)

  assert(regex.match("abc", pat)=="bc")
  assert(regex.matchW(L"abc", pat)==L"bc")

  assert(R:match("abc")=="bc")
  assert(R:matchW(L"abc")==L"bc")

  str, nfound, nrep = regex.gsub("abc", pat, rep)
  assert(str=="abcbc" and nfound==1 and nrep==1)
  str, nfound, nrep = regex.gsubW(L"abc", pat, rep)
  assert(str==L"abcbc" and nfound==1 and nrep==1)

  str, nfound, nrep = R:gsub("abc", rep)
  assert(str=="abcbc" and nfound==1 and nrep==1)
  str, nfound, nrep = R:gsubW(L"abc", rep)
  assert(str==L"abcbc" and nfound==1 and nrep==1)

  local t = {}
  for cap in regex.gmatch("abc", ".") do t[#t+1]=cap end
  assert(#t==3 and t[1]=="a" and t[2]=="b" and t[3]=="c")
  for cap in regex.gmatchW(L"abc", ".") do t[#t+1]=cap end
  assert(#t==6 and t[4]==L"a" and t[5]==L"b" and t[6]==L"c")

  t, R = {}, regex.new(".")
  for cap in R:gmatch("abc") do t[#t+1]=cap end
  assert(#t==3 and t[1]=="a" and t[2]=="b" and t[3]=="c")
  for cap in R:gmatchW(L"abc") do t[#t+1]=cap end
  assert(#t==6 and t[4]==L"a" and t[5]==L"b" and t[6]==L"c")

  str, nfound, nrep = regex.gsub(";a;", "a*", "ITEM")
  assert(str=="ITEM;ITEM;ITEM" and nfound==3 and nrep==3)
  str, nfound, nrep = regex.gsub(";a;", "a*?", "ITEM")
  assert(str=="ITEM;ITEMaITEM;ITEM" and nfound==4 and nrep==4)

  -- Mantis 3336 (https://bugs.farmanager.com/view.php?id=3336)
  local c1,c2,c3
  fr,to,c1 = regex.find("{}", "\\{(.)?\\}")
  assert(fr==1 and to==2 and c1==false)
  fr,to,c1,c2,c3 = regex.find("bbb", "(b)?b(b)?(b)?b")
  assert(fr==1 and to==3 and c1=="b" and c2==false and c3==false)

  -- Mantis 1388 (https://bugs.farmanager.com/view.php?id=1388)
  c1,c2 = regex.match("123", "(\\d+)A|(\\d+)")
  assert(c1==false and c2=="123")

  -- Issue #609 (https://github.com/FarGroup/FarManager/issues/609)
  c1 = regex.match("88", "(8)+")
  asrt.eq(c1, "8")

  -- test regex flags
  LoadMod("test_regex_flags")()
end

function LF.test_utf8_len()
  asrt.eq ((""):len(), 0)
  asrt.eq (("FOo БАр"):len(), 7)
end

function LF.test_utf8_sub()
  local text = "abcdабвг"
  local len = assert(text:len()==8) and 8

  for _,start in ipairs{-len*3, 0, 1} do
    asrt.eq (text:sub(start, -len*4), "")
    asrt.eq (text:sub(start, -len*3), "")
    asrt.eq (text:sub(start, -len*2), "")
    asrt.eq (text:sub(start, -len-1 + 0), "")
    asrt.eq (text:sub(start,          0), "")
    asrt.eq (text:sub(start, -len-1 + 1), "a")
    asrt.eq (text:sub(start,          1), "a")
    asrt.eq (text:sub(start, -len-1 + 6), "abcdаб")
    asrt.eq (text:sub(start,          6), "abcdаб")
    asrt.eq (text:sub(start, len*1), text)
    asrt.eq (text:sub(start, len*2), text)
  end

  for _,start in ipairs{3, -6} do
    asrt.eq (text:sub(start, -len*2), "")
    asrt.eq (text:sub(start,      0), "")
    asrt.eq (text:sub(start,      1), "")
    asrt.eq (text:sub(start, start-1), "")
    asrt.eq (text:sub(start,      -6), "c")
    asrt.eq (text:sub(start, start+0), "c")
    asrt.eq (text:sub(start,      -5), "cd")
    asrt.eq (text:sub(start, start+3), "cdаб")
    asrt.eq (text:sub(start,      -3), "cdаб")
    asrt.eq (text:sub(start, len), "cdабвг")
    asrt.eq (text:sub(start, 2*len), "cdабвг")
  end

  for _,start in ipairs{len+1, 2*len} do
    for _,fin in ipairs{-2*len, -1*len, -1, 0, 1, len-1, len, 2*len} do
      asrt.eq(text:sub(start,fin), "")
    end
  end

  for _,start in ipairs{-2*len,-len-1,-len,-len+1,-1,0,1,len-1,len,len+1} do
    asrt.eq(text:sub(start), text:sub(start,len))
  end

  asrt.isfalse(pcall(text.sub, text))
  asrt.isfalse(pcall(text.sub, text, {}))
  asrt.isfalse(pcall(text.sub, text, nil))
end

function LF.test_utf8_lower_upper()
  asrt.eq ((""):lower(), "")
  asrt.eq (("abc"):lower(), "abc")
  asrt.eq (("ABC"):lower(), "abc")

  asrt.eq ((""):upper(), "")
  asrt.eq (("abc"):upper(), "ABC")
  asrt.eq (("ABC"):upper(), "ABC")

  local russian_abc = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя"
  local part1, part2 = russian_abc:sub(1,33), russian_abc:sub(34)
  asrt.eq (part1:lower(), part2)
  asrt.eq (part2:lower(), part2)
  asrt.eq (part1:upper(), part1)
  asrt.eq (part2:upper(), part1)

  local noletters = "1234567890~@#$%^&*()_+-=[]{}|/\\';.,"
  asrt.eq (noletters:lower(), noletters)
  asrt.eq (noletters:upper(), noletters)
end

function LF.test_utf8()
  LF.test_utf8_len()
  LF.test_utf8_sub()
  LF.test_utf8_lower_upper()
end

---------------------------------------------------------------------------------------------------
-- ACTL_GETWINDOWCOUNT, ACTL_GETWINDOWTYPE, ACTL_GETWINDOWINFO, ACTL_SETCURRENTWINDOW, ACTL_COMMIT
---------------------------------------------------------------------------------------------------
function LF.test_AdvControl_Window()
  local num, t

  num = far.AdvControl("ACTL_GETWINDOWCOUNT")
  assert(num == 2)
  mf.acall(far.Show); mf.acall(far.Show)
  assert(far.AdvControl("ACTL_GETWINDOWTYPE").Type == F.WTYPE_VMENU)
  assert(num+2 == far.AdvControl("ACTL_GETWINDOWCOUNT"))
  Keys("Esc Esc")
  asrt.eq(num, far.AdvControl("ACTL_GETWINDOWCOUNT"))

  -- Get information about 2 available windows
  t = asrt.table(far.AdvControl("ACTL_GETWINDOWINFO", 1))
  assert(t.Type==F.WTYPE_DESKTOP and t.Id==0 and t.Pos==1 and t.Flags==0 and #t.TypeName>0 and
         t.Name=="")

  t = asrt.table(far.AdvControl("ACTL_GETWINDOWINFO", 2))
  assert(t.Type==F.WTYPE_PANELS and t.Id==0 and t.Pos==2 and t.Flags==F.WIF_CURRENT and
         #t.TypeName>0 and #t.Name>0)
  asrt.eq(far.AdvControl("ACTL_GETWINDOWTYPE").Type, F.WTYPE_PANELS)

  -- Set "Desktop" as the current window
  asrt.eq (1, far.AdvControl("ACTL_SETCURRENTWINDOW", 1))
  asrt.eq (1, far.AdvControl("ACTL_COMMIT"))
  t = asrt.table(far.AdvControl("ACTL_GETWINDOWINFO", 2)) -- "z-order": the window that was #1 is now #2
  assert(t.Type==0 and t.Id==0 and t.Pos==2 and t.Flags==F.WIF_CURRENT and #t.TypeName>0 and
         t.Name=="")
  asrt.eq (far.AdvControl("ACTL_GETWINDOWTYPE").Type, F.WTYPE_DESKTOP)
  t = asrt.table(far.AdvControl("ACTL_GETWINDOWINFO", 1))
  assert(t.Type==F.WTYPE_PANELS and t.Id==0 and t.Pos==1 and t.Flags==0 and #t.TypeName>0 and
         #t.Name>0)

  -- Restore "Panels" as the current window
  asrt.eq (1, far.AdvControl("ACTL_SETCURRENTWINDOW", 1))
  asrt.eq (1, far.AdvControl("ACTL_COMMIT"))
  asrt.eq (far.AdvControl("ACTL_GETWINDOWTYPE").Type, F.WTYPE_PANELS)
end

function LF.test_AdvControl_Colors()
  local allcolors = asrt.table(far.AdvControl("ACTL_GETARRAYCOLOR"))
  assert(#allcolors > 140)
  for n=1,#allcolors do
    local color = assert(far.AdvControl("ACTL_GETCOLOR", n-1))
    assert(color.Flags and color.ForegroundColor and color.BackgroundColor)
    for k,v in pairs(color) do
      asrt.eq(allcolors[n][k], v)
    end
  end
  asrt.isnil(far.AdvControl("ACTL_GETCOLOR", #allcolors))
  asrt.isnil(far.AdvControl("ACTL_GETCOLOR", -1))

  -- change the colors
  local arr, elem = {StartIndex=0; Flags=0}, {Flags=100; ForegroundColor=101; BackgroundColor=102}
  for n=1,#allcolors do arr[n]=elem end
  asrt.eq(far.AdvControl("ACTL_SETARRAYCOLOR", nil, arr), 1)
  for n=1,#allcolors do
    local color = asrt.table(far.AdvControl("ACTL_GETCOLOR", n-1))
    for k,v in pairs(elem) do
      asrt.eq(color[k], v)
    end
  end

  -- restore the colors
  assert(far.AdvControl("ACTL_SETARRAYCOLOR", nil, allcolors))
end

function LF.test_AdvControl_Synchro()
  local pass = 0
  local oldProcessSynchroEvent = export.ProcessSynchroEvent
  export.ProcessSynchroEvent =
    function(event,param)
      asrt.istrue(pass==0 and param==123 or pass==1 and param==-456)
      pass = pass + 1
    end
  far.AdvControl("ACTL_SYNCHRO", 123)
  far.AdvControl("ACTL_SYNCHRO", -456)
  for _ = 1,2 do
    mf.acall(far.Show); Keys"Esc"
  end
  export.ProcessSynchroEvent = oldProcessSynchroEvent
  asrt.eq(pass, 2)
end

function LF.test_AdvControl_Misc()
  local t

  asrt.udata(far.AdvControl("ACTL_GETFARHWND"))

  asrt.eq(far.AdvControl("ACTL_GETFARMANAGERVERSION"):sub(1,1), "3")
  asrt.eq(far.AdvControl("ACTL_GETFARMANAGERVERSION",true), 3)

  t = far.AdvControl("ACTL_GETFARRECT")
  assert(t.Left>=0 and t.Top>=0 and t.Right>t.Left and t.Bottom>t.Top)

  asrt.eq(0, far.AdvControl("ACTL_SETCURSORPOS", nil, {X=-1,Y=0}))
  for k=0,2 do
    asrt.eq(1, far.AdvControl("ACTL_SETCURSORPOS", nil, {X=k,Y=k+1}))
    t = asrt.table(far.AdvControl("ACTL_GETCURSORPOS"))
    assert(t.X==k and t.Y==k+1)
  end

  asrt.istrue(mf.acall(far.AdvControl, "ACTL_WAITKEY", nil, "F1"))
  Keys("F1")
  asrt.istrue(mf.acall(far.AdvControl, "ACTL_WAITKEY"))
  Keys("F2")
end

function LF.test_gmatch_coro()
  local function yieldFirst(it)
    return coroutine.wrap(function()
      coroutine.yield(it())
    end)
  end

  local it = ("1 2 3"):gmatch("(%d+)")
  local head = yieldFirst(it)
  asrt.eq (head(), "1")
end

function LF.test_PluginsControl()
  local mod = assert(far.PluginStartupInfo().ModuleName)
  local hnd1 = far.FindPlugin("PFM_MODULENAME", mod)
  asrt.udata(hnd1)
  local hnd2 = far.FindPlugin("PFM_GUID", far.PluginStartupInfo().PluginGuid)
  asrt.udata(hnd2)

  local info = far.GetPluginInformation(hnd1)
  asrt.table(info)
  asrt.table(info.GInfo)
  asrt.table(info.PInfo)
  asrt.eq(mod, info.ModuleName)
  asrt.num(info.Flags)
  assert(0 ~= band(info.Flags, F.FPF_LOADED))
  assert(0 == band(info.Flags, F.FPF_ANSI))

  local pluglist = far.GetPlugins()
  asrt.table(pluglist)
  assert(#pluglist >= 1)
  for _,plug in ipairs(pluglist) do
    asrt.udata(plug)
  end

  asrt.func(far.LoadPlugin)
  asrt.func(far.ForcedLoadPlugin)
  asrt.func(far.UnloadPlugin)
end

function LF.test_far_timer()
  local N = 0
  local timer = far.Timer(50, function(hnd)
      N = N+1
      if N==3 then hnd:Close() end
    end)
  while not timer.Closed do Keys("foobar") end
  asrt.eq (N, 3)
end

function LF.test_win_functions()
  asrt.func( win.CompareString)
  asrt.func( win.CopyFile)
  asrt.func( win.CreateDir)
  asrt.func( win.DeleteFile)
  asrt.func( win.DeleteRegKey)
  asrt.func( win.DeleteRegValue)
  asrt.func( win.EnumRegKey)
  asrt.func( win.EnumRegValue)
  asrt.func( win.EnumSystemCodePages)
  asrt.func( win.ExtractKey)
  asrt.func( win.ExtractKeyEx)
  asrt.func( win.FileTimeToLocalFileTime)
  asrt.func( win.FileTimeToSystemTime)
  asrt.func( win.GetACP)
  asrt.func( win.GetCPInfo)
  asrt.func( win.GetConsoleCP)
  asrt.func( win.GetConsoleOutputCP)
  asrt.func( win.GetConsoleScreenBufferInfo)
  asrt.func( win.GetCurrentDir)
  asrt.func( win.GetDriveType)
  asrt.func( win.GetEnv)
  asrt.func( win.GetEnvironmentStrings)
  asrt.func( win.GetFileAttr)
  asrt.func( win.GetFileInfo)
  asrt.func( win.GetFileTimes)
  asrt.func( win.GetKeyState)
  asrt.func( win.GetLocalTime)
  asrt.func( win.GetLogicalDriveStrings)
  asrt.func( win.GetOEMCP)
  asrt.func( win.GetRegKey)
  asrt.func( win.GetSystemTime)
  asrt.func( win.GetSystemTimeAsFileTime)
  asrt.func( win.GetVirtualKeys)
  asrt.func( win.GlobalMemoryStatus)
  asrt.func( win.IsProcess64bit)
  asrt.func( win.IsWinVersion)
  asrt.func( win.JoinPath)
  asrt.func( win.MoveFile)
  asrt.func( win.MultiByteToWideChar)
  asrt.func( win.OemToUtf8)
  asrt.func( win.OutputDebugString)
  asrt.func( win.RemoveDir)
  asrt.func( win.RenameFile)
  asrt.func( win.ReplaceFile)
  asrt.func( win.SHGetFolderPath)
  asrt.func( win.SearchPath)
  asrt.func( win.SetConsoleCP)
  asrt.func( win.SetConsoleOutputCP)
  asrt.func( win.SetCurrentDir)
  asrt.func( win.SetEnv)
  asrt.func( win.SetFileAttr)
  asrt.func( win.SetFileTimes)
  asrt.func( win.SetRegKey)
  asrt.func( win.ShellExecute)
  asrt.func( win.Sleep)
  asrt.func( win.SystemTimeToFileTime)
  asrt.func( win.Utf16ToUtf8)
  asrt.func( win.Utf8ToOem)
  asrt.func( win.Utf8ToUtf16)
  asrt.func( win.Uuid)
  asrt.func( win.WideCharToMultiByte)
  asrt.func( win.WriteConsole)
  asrt.func( win.lenW)
  asrt.func( win.subW)
  asrt.func( win.system)
  asrt.func( win.wcscmp)
end

function LF.test_win_CompareString()
  assert(win.CompareString("a","b") < 0)
  assert(win.CompareString("b","a") > 0)
  assert(win.CompareString("b","b") == 0)
end

function LF.test_win_Uuid()
  local uuid = win.Uuid()
  asrt.eq(#uuid, 16)
  asrt.neq(uuid, ("\0"):rep(16))

  local u1 = win.Uuid(uuid)
  asrt.eq(#u1, 36)
  asrt.eq(win.Uuid(u1), uuid)

  local u2 = win.Uuid("L")
  asrt.eq(#u2, 36)
  asrt.eq(u2, u2:match("^[0-9a-f%-]+$"))

  local u3 = win.Uuid("U")
  asrt.eq(#u3, 36)
  asrt.eq(u3, u3:match("^[0-9A-F%-]+$"))
end

function LF.test_win()
  LF.test_win_functions()

  LF.test_win_CompareString()
  LF.test_win_Uuid()

  asrt.table(win.EnumSystemCodePages())
  asrt.num(win.GetACP())
  asrt.str(win.GetCurrentDir())
  asrt.num(win.GetOEMCP())

  local dir = asrt.str(win.GetEnv("FARHOME"))
  local attr = asrt.str(win.GetFileAttr(dir))
  asrt.num(attr:find("d"))
  asrt.table (win.GetFileInfo(dir))

  asrt.table (win.GetCPInfo(65001))
  asrt.table (win.GetLocalTime())
  asrt.table (win.GetSystemTime())
end

local function test_dialog_1()
  local tt = {"foo","bar","baz"}

  local Items =  {
    guid="7C5407A5-7F41-47AD-9F7E-CBD78E60438C";
    data = tt[1];

    {tp="dbox"; val="Text";    },
    {tp="text"; val="Text1";   },
    {tp="edit"; val="Text20";  },
    {tp="butt"; val="Text300"; },
  }
  Items.proc = function(hDlg, Msg, Par1, Par2)
    if Msg == F.DN_INITDIALOG then
      assert(Par1==3 and Par2==Items.data)

      asrt.eq(hDlg:send("DM_GETDLGDATA"), Par2)
      asrt.eq(hDlg:send("DM_SETDLGDATA", tt[2]), tt[1])
      asrt.eq(hDlg:send("DM_GETDLGDATA"), tt[2])

      local info = asrt.table(hDlg:send("DM_GETDIALOGINFO"))
      asrt.eq(info.Id, win.Uuid(Items.guid))
      asrt.eq(info.Owner, luamacroId)

      for k,item in ipairs(Items) do
        asrt.eq(hDlg:send("DM_GETTEXT", k), item.val)
        local newtext = "---" .. item.val
        local ref = newtext:len()
        if item.tp == "butt" then ref = ref+4 end
        asrt.eq(ref, hDlg:send("DM_SETTEXT", k, newtext))
      end

      hDlg:send("DM_CLOSE")
      Items.data = tt[3]
    end
  end

  sd.New(Items):Run()
  assert(Items.data == tt[3])
end

local function test_dialog_issue_28()
  local checkValue = nil
  local items = {
    width=30;
    {tp="dbox"; text="Issue #28"},
    {tp="butt"; btnnoclose=1; text="CRASH"; centergroup=1; },
  }
  items.proc=function(hDlg,msg,p1,p2)
    if msg==F.DN_BTNCLICK then
      hDlg:Close()
      far.Message("hello") -- USED TO CAUSE CRASH
      checkValue = true
      return 1
    end
  end
  local Dlg = sd.New(items)
  mf.acall(Dlg.Run, Dlg)
  Keys("Enter")
  asrt.istrue(Area.Dialog)
  Keys("Esc")
  asrt.istrue(Area.Shell)
  asrt.istrue(checkValue)
end

local function test_dialog_usercontrol()
  local w, h = 40, 12
  local uc = asrt.udata(far.CreateUserControl(w, h))

  -- __index (number)
  asrt.eq(#uc, w * h)
  asrt.table(uc[#uc])
  asrt.err(function() return uc[#uc + 1] end)

  -- __index (string)
  asrt.udata(uc.rawhandle)
  asrt.func(uc.write) -- test needed

  -- __newindex
  asrt.noerr(function() uc[#uc] = { Char="A"; Attributes=0; }; end)
  asrt.err(function() uc[#uc + 1] = {} end)
end

-- Commit cef18f95
-- The far:config dialog didn't respond to F4, Enter, Del, CtrlH...
-- The reason was sloppy detection of Far version in simpledialog.lua module.
local function test_dialog_cef18f95()
  -- open far:config dialog
  panel.SetCmdLine(nil, "far:config")
  Keys("Enter")
  asrt.istrue(Area.Dialog)
  asrt.eq(Dlg.Id, far.Guids.AdvancedConfigId)
  local wInfo = asrt.table(actl.GetWindowInfo())
  local hDlg  = asrt.udata(wInfo.Id)
  local index = asrt.num( hDlg:ListFindString(nil, { Pattern="*Cmdline.AutoComplete*"}) )
  asrt.eq(index, hDlg:ListSetCurPos(nil, {SelectPos=index}))

  -- collect initial state data
  local wasFalse, wasTrue, isFalse, isTrue
  local item  = asrt.table(hDlg:ListGetItem(nil, index))
  wasFalse = nil ~= item.Text:find("false")
  wasTrue  = nil ~= item.Text:find("true")
  asrt.neq(wasFalse, wasTrue)

  -- toggle the boolean
  Keys("F4")
  item  = asrt.table(hDlg:ListGetItem(nil, index))
  isFalse = nil ~= item.Text:find("false")
  isTrue  = nil ~= item.Text:find("true")
  asrt.eq(isFalse, wasTrue)
  asrt.eq(isTrue, wasFalse)

  -- toggle the boolean again to its original state
  Keys("Enter")
  item  = asrt.table(hDlg:ListGetItem(nil, index))
  isFalse = nil ~= item.Text:find("false")
  isTrue  = nil ~= item.Text:find("true")
  asrt.eq(isFalse, wasFalse)
  asrt.eq(isTrue, wasTrue)

  -- close far:config dialog
  Keys("Esc")
  asrt.istrue(Area.Shell)
end

-- commit daee1a50
-- DM_RESIZEDIALOG: prevent crashes
local function test_dialog_daee1a50()
  Keys("AltF7")
  asrt.eq(Dlg.Id, far.Guids.FindFileId)
  local hDlg = asrt.udata(actl.GetWindowInfo().Id)
  hDlg:ResizeDialog(nil, {Y=-1,X=-1})
  hDlg:ResizeDialog(nil, {Y=0, X=0})
  Keys("Esc")
end

function LF.test_dialog()
  test_dialog_1()
  test_dialog_issue_28()
  test_dialog_usercontrol()
  test_dialog_cef18f95()
  test_dialog_daee1a50()
end

function LF.test_far_Menu1()
  local items = { { text="line1" }, { text="line2" } }

  local bkeys1 = {
    {BreakKey="F1"}, {BreakKey="F2"},
    {BreakKey="1" }, {BreakKey="2" },
    {BreakKey="A" }, {BreakKey="B" },
  }

  local bkeys2 = "F1 F2 1 2 A B"

  for _, BK in ipairs { bkeys1,bkeys2 } do
    for _, item  in ipairs(bkeys1) do
      mf.acall(far.Menu, {}, items, BK)
      assert(Area.Menu)
      Keys(item.BreakKey)
      assert(Area.Shell)
    end
  end
end

function LF.test_far_Menu2()
  -- test far.Menu and Menu.GetValue
  local items = {"foo", "bar", "baz"}
  mf.acall(far.Menu, {}, items)
  asrt.istrue(Area.Menu)
  for j=1,#items do
    asrt.eq(Menu.GetValue(j), items[j])
  end
  for j=1,#items do
    asrt.eq(Menu.GetValue(0), items[j])
    Keys("Down")
  end
  asrt.eq(Menu.GetValue(-1), "")
  asrt.eq(Menu.GetValue(#items+1), "")
  Keys("Esc")
  asrt.istrue(Area.Shell)
end

local function test_far_MakeMenuItems()
  local t = asrt.table (far.MakeMenuItems("foo","bar","baz"))
  asrt.eq(#t, 3)
end

function LF.test_far_Menu()
  LF.test_far_Menu1()
  LF.test_far_Menu2()
--LF.test_far_Menu3()
  test_far_MakeMenuItems()
end

function LF.test_far_SaveScreen()
  local scr = asrt.udata(far.SaveScreen(0,0,20,10))
  far.FreeScreen(scr)
  asrt.func(far.RestoreScreen)
end

function LF.test_far_GetDirList()
  local dir = os.getenv("FARHOME") .. "\\Plugins\\luamacro"
  local list = asrt.table(far.GetDirList(dir))
  local item = asrt.table(list[1])
  asrt.str(item.FileName)
end

function LF.test_far_ShowHelp()
  mf.postmacro(function() asrt.eq(Area.Current,"Help"); Keys("Esc"); end)
  asrt.istrue(far.ShowHelp("", nil, "FHELP_FARHELP FHELP_USECONTENTS"))
--asrt.isfalse(far.ShowHelp())
end

function LF.test_bit64()
  for _,name in ipairs{"band","bnot","bor","bxor","lshift","rshift"} do
    asrt.eq   (_G[name], bit64[name])
    asrt.func (bit64[name])
  end

  local a,b,c = 0xFF,0xFE,0xFD
  asrt.eq(band(a,b,c,a,b,c), 0xFC)
  a,b,c = bit64.new(0xFF),bit64.new(0xFE),bit64.new(0xFD)
  asrt.eq(band(a,b,c,a,b,c), 0xFC)

  a,b = bit64.new("0xFFFF0000FFFF0000"),bit64.new("0x0000FFFF0000FFFF")
  asrt.eq(band(a,b), 0)
  asrt.eq(bor(a,b), -1)
  asrt.eq(a+b, -1)

  a,b,c = 1,2,4
  asrt.eq(bor(a,b,c,a,b,c), 7)

  for k=-3,3 do asrt.eq(bnot(k), -1-k) end
  asrt.eq(bnot(bit64.new(5)), -6)

  asrt.eq(bxor(0x01,0xF0,0xAA), 0x5B)
  asrt.eq(lshift(0xF731,4),  0xF7310)
  asrt.eq(rshift(0xF7310,4), 0xF731)

  local v = bit64.new(5)
  asrt.istrue(v+2==7  and 2+v==7)
  asrt.istrue(v-2==3  and 2-v==-3)
  asrt.istrue(v*2==10 and 2*v==10)
  asrt.istrue(v/2==2  and 2/v==0)
  asrt.istrue(v%2==1  and 2%v==2)
  asrt.istrue(v+v==10 and v-v==0 and v*v==25 and v/v==1 and v%v==0)

  local w = lshift(1,63)
  asrt.eq(w, bit64.new("0x8000".."0000".."0000".."0000"))
  asrt.eq(rshift(w,63), 1)
  asrt.eq(rshift(w,64), 0)
  asrt.eq(bit64.arshift(w,62), -2)
  asrt.eq(bit64.arshift(w,63), -1)
  asrt.eq(bit64.arshift(w,64), -1)
end

local function test_one_guid(val, func, keys, numEsc)
  numEsc = numEsc or 1
  val = far.Guids[val]
  asrt.str(val)
  asrt.eq(#val, 36)

  if func then func() end
  if keys then Keys(keys) end

  asrt.eq(Area.Dialog and Dlg.Id or Menu.Id, val)
  for _ = 1,numEsc do Keys("Esc") end
end

function LF.test_Guids()
  asrt.table(far.Guids)

  -- Work from TEMP directory to avoid read-only media, etc
  local TmpDir = assert(win.GetEnv"TEMP" or win.GetEnv"TMP", "no temporary directory found")
  asrt.istrue(panel.SetPanelDirectory(nil, 1, TmpDir))

  Keys("Esc"); print("far:config"); Keys("Enter")
  test_one_guid( "AdvancedConfigId")

  test_one_guid( "ApplyCommandId",           nil, "CtrlG")
  test_one_guid( "AskInsertMenuOrCommandId", nil, "F2 Ins", 2)
  test_one_guid( "ChangeDiskMenuId",         nil, "AltF1")
  test_one_guid( "ChangeDiskMenuId",         nil, "AltF2")
  test_one_guid( "CodePagesMenuId",          nil, "F9 Home 3*Right Enter End 3*Up Enter")

  local was_empty = APanel.Empty
  if was_empty then
    Keys("F7 1 Enter")
    asrt.isfalse(APanel.Empty)
  end
  test_one_guid( "CopyCurrentOnlyFileId",    nil, "End ShiftF5")
  test_one_guid( "CopyFilesId",              nil, "End F5")
  local keyname = Far.GetConfig("System.DeleteToRecycleBin") and "DeleteRecycleId" or "DeleteFileFolderId"
  test_one_guid( keyname,                    nil, "End F8")
  test_one_guid( "DeleteWipeId",             nil, "End AltDel")
  test_one_guid( "DescribeFileId",           nil, "End CtrlZ")
  test_one_guid( "FileAttrDlgId",            nil, "End CtrlA")
  test_one_guid( "HardSymLinkId",            nil, "End AltF6")
  test_one_guid( "MoveCurrentOnlyFileId",    nil, "End ShiftF6")
  test_one_guid( "MoveFilesId",              nil, "End F6")
  if was_empty then
    asrt.eq(APanel.Current, "1")
    Keys("F8")
    if Area.Dialog then Keys("Enter") end
    asrt.istrue(APanel.Empty)
  end

  test_one_guid( "EditUserMenuId",           nil, "F2 Ins Enter", 2)
  test_one_guid( "EditorReplaceId",          nil, "ShiftF4 Del Enter CtrlF7", 2)
  test_one_guid( "EditorSearchId",           nil, "ShiftF4 Del Enter F7", 2)
  test_one_guid( "EditorCanNotEditDirectoryId", nil, "ShiftF4 . . Enter", 1)
  test_one_guid( "FarAskQuitId",             nil, "F10")

  local h_old = Far.DisableHistory(0x07) -- disable all histories except dialog edit
  test_one_guid( "SelectFromEditHistoryId", nil, "ShiftF4 A Enter Esc ShiftF4 CtrlDown", 2)
  Far.DisableHistory(h_old) -- restore the old state

  local myMenu
  myMenu = function() mf.mainmenu("fileassociations") end
  test_one_guid( "FileAssocMenuId",          myMenu)
  test_one_guid( "FileAssocModifyId",        myMenu, "Ins", 2)

  myMenu = function() mf.mainmenu("foldershortcuts") end
  test_one_guid( "FolderShortcutsId",        myMenu)
--test_one_guid( "FolderShortcutsDlgId",     myMenu, "F4", 2)

  myMenu = function() mf.mainmenu("filehighlight") end
  test_one_guid( "HighlightMenuId",          myMenu)
  test_one_guid( "HighlightConfigId",        myMenu, "Ins", 2)

  myMenu = function() mf.mainmenu("filepanelmodes") end
  test_one_guid( "PanelViewModesId",         myMenu)
  test_one_guid( "PanelViewModesEditId",     myMenu, "Enter", 2)

  myMenu = function() mf.mainmenu("filemaskgroups") end
  test_one_guid( "MaskGroupsMenuId",         myMenu)
  test_one_guid( "EditMaskGroupId",          myMenu, "Ins", 2)

  test_one_guid( "FileOpenCreateId",         nil, "ShiftF4")
  test_one_guid( "FileSaveAsId",             nil, "ShiftF4 Del Enter ShiftF2", 2)
  test_one_guid( "FiltersConfigId",          nil, "CtrlI Ins", 2)
  test_one_guid( "FiltersMenuId",            nil, "CtrlI")
  test_one_guid( "FindFileId",               nil, "AltF7")
  test_one_guid( "HelpSearchId",             nil, "F1 F7", 2)
  test_one_guid( "HistoryCmdId",             nil, "AltF8")
  test_one_guid( "HistoryEditViewId",        nil, "AltF11")
  test_one_guid( "HistoryFolderId",          nil, "AltF12")
  test_one_guid( "MakeFolderId",             nil, "F7")
  test_one_guid( "PluginInformationId",      nil, "F11 F3", 2)
  test_one_guid( "PluginsConfigMenuId",      nil, "AltShiftF9")
  test_one_guid( "PluginsMenuId",            nil, "F11")
  test_one_guid( "ScreensSwitchId",          nil, "F12")
  test_one_guid( "SelectDialogId",           nil, "Add")
  test_one_guid( "SelectSortModeId",         nil, "CtrlF12")
  test_one_guid( "UnSelectDialogId",         nil, "Subtract")

  viewer.Viewer(far.PluginStartupInfo().ModuleName,"",nil,nil,nil,nil,"VF_NONMODAL VF_IMMEDIATERETURN")
  asrt.istrue(Area.Viewer)
  test_one_guid( "ViewerSearchId",           nil, "F7", 2)

  -- test_one_guid( "BadEditorCodePageId", nil, "")
  -- test_one_guid( "CannotRecycleFileId", nil, "")
  -- test_one_guid( "CannotRecycleFolderId", nil, "")
  -- test_one_guid( "ChangeDriveCannotReadDiskErrorId", nil, "")
  -- test_one_guid( "ChangeDriveModeId", nil, "")
  -- test_one_guid( "CopyOverwriteId", nil, "")
  -- test_one_guid( "CopyReadOnlyId", nil, "")
  -- test_one_guid( "DeleteAskDeleteROId", nil, "")
  -- test_one_guid( "DeleteAskWipeROId", nil, "")
  -- test_one_guid( "DeleteFolderId", nil, "")
  -- test_one_guid( "DeleteFolderRecycleId", nil, "")
  -- test_one_guid( "DeleteLinkId", nil, "")
  -- test_one_guid( "DeleteRecycleId", nil, "")
  -- test_one_guid( "DisconnectDriveId", nil, "")
  -- test_one_guid( "EditAskSaveExtId", nil, "")
  -- test_one_guid( "EditAskSaveId", nil, "")
  -- test_one_guid( "EditorAskOverwriteId", nil, "")
  -- test_one_guid( "EditorConfirmReplaceId", nil, "")
  -- test_one_guid( "EditorFileGetSizeErrorId", nil, "")
  -- test_one_guid( "EditorFileLongId", nil, "")
  -- test_one_guid( "EditorFindAllListId", nil, "")
  -- test_one_guid( "EditorOpenRSHId", nil, "")
  -- test_one_guid( "EditorReloadId", nil, "")
  -- test_one_guid( "EditorReloadModalId", nil, "")
  -- test_one_guid( "EditorSavedROId", nil, "")
  -- test_one_guid( "EditorSaveExitDeletedId", nil, "")
  -- test_one_guid( "EditorSaveF6DeletedId", nil, "")
  -- test_one_guid( "EditorSwitchUnicodeCPDisabledId", nil, "")
  -- test_one_guid( "FindFileResultId", nil, "")
  -- test_one_guid( "FolderShortcutsMoreId", nil, "")
  -- test_one_guid( "GetNameAndPasswordId", nil, "")
  -- test_one_guid( "RecycleFolderConfirmDeleteLinkId", nil, "")
  -- test_one_guid( "SelectAssocMenuId", nil, "")
  -- test_one_guid( "SUBSTDisconnectDriveId", nil, "")
  -- test_one_guid( "UserMenuUserInputId", nil, "")
  -- test_one_guid( "VHDDisconnectDriveId", nil, "")
  -- test_one_guid( "WipeFolderId", nil, "")
  -- test_one_guid( "WipeHardLinkId", nil, "")
end

function LF.test_all()
  LF.test_AdvControl()
  LF.test_bit64()
  LF.test_dialog()
  LF.test_far_GetDirList()
  LF.test_far_GetMsg()
--LF.test_far_GetPluginDirList()
  LF.test_far_Menu()
--LF.test_far_regex( --[[far.Log, true]] ) -- external test files
  LF.test_far_SaveScreen()
  LF.test_far_ShowHelp()
  LF.test_far_timer()
  LF.test_gmatch_coro()
  LF.test_Guids()
  LF.test_MacroControl()
  LF.test_PluginsControl()
  LF.test_RegexControl()
--LF.test_SplitCmdLine()
  LF.test_utf8()
  LF.test_win()
end

return LF
