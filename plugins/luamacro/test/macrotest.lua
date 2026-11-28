-- encoding: utf-8
-- Started: 2012-08-20.

--[[
-- The following macro can be used to run all the tests.

local MacroKey = "CtrlShiftF12"
Macro {
  description="Macro-engine test";
  area="Shell"; key=MacroKey;
  action = function()
    Far.DisableHistory(0x0F)
    local f = assert(loadfile(far.PluginStartupInfo().ModuleDir.."test\\macrotest.lua"))
    local mod = setfenv(f, getfenv())()
    mod.SetMacroKeys(MacroKey)
    mod.test_all()
    far.Message("All tests OK", "LuaMacro")
  end;
}
--]]

local MT = {} -- "macrotest", this module

local function LoadMod(name)
  return assert(loadfile(far.PluginStartupInfo().ModuleDir.."test\\"..name..".lua")) ()
end

local asrt    = LoadMod "assert"
MT.Mod_Dialog = LoadMod "test_dialog"
MT.Mod_Editor = LoadMod "test_editor"
MT.Mod_FSF    = LoadMod "test_fsf"
MT.Mod_Luafar = LoadMod "test_luafar"
MT.Mod_Panel  = LoadMod "test_panel"

local MacroKeys -- keys that invoke the whole macrotest from a macro
local F = far.Flags
local luamacroId="4ebbefc8-2084-4b7f-94c0-692ce136894d"
local hlfviewerId = "1AF0754D-5020-49CB-9474-1F82691C84C1"
local TKEY_BINARY = "__binary"

function MT.SetMacroKeys(strKeys)
  MacroKeys = asrt.str(strKeys, "bad parameter to SetMacroKeys")
end

local function CheckMacroKeys()
  asrt.str(MacroKeys, "MacroKeys value is not set")
end

local function pack (...)
  return { n=select("#",...), ... }
end

local TmpFileName = win.JoinPath(assert(win.GetEnv"TEMP" or win.GetEnv"TMP"), "tmp.tmp")

local function WriteTmpFile(...)
  local fp = assert(io.open(TmpFileName,"w"))
  fp:write(...)
  fp:close()
end

local function DeleteTmpFile()
  win.DeleteFile(TmpFileName)
end

local function TestArea (area, k_before, k_after)
  if k_before then Keys(k_before) end
  assert(Area[area]==true and Area.Current==area)
  if k_after then Keys(k_after) end
end

function MT.test_areas()
  for _=1,2 do -- ensure Area.Shell on both panels
    if Area.Info then Keys("CtrlL")
    elseif Area.QView then Keys("CtrlQ")
    end
    Keys("Tab")
  end

  TestArea ("Shell")
  TestArea ("Grabber",    "AltIns",     "Esc")
  TestArea ("Desktop",    "F12 0",      "F12 1")
  TestArea ("Editor",     "ShiftF4 CtrlY Enter", "Esc")
  TestArea ("Dialog",     "F7",         "Esc")
  TestArea ("Search",     "Alt?",       "Esc")
  TestArea ("Disks",      "AltF1",      "Esc")
  TestArea ("Disks",      "AltF2",      "Esc")
  TestArea ("MainMenu",   "F9",         "Esc")
  TestArea ("MainMenu",   "F9 Enter",   "Esc Esc")
  TestArea ("Menu",       "F12",        "Esc")
  TestArea ("Help",       "F1",         "Esc")
  TestArea ("Info",       "CtrlL Tab",  "Tab CtrlL")
  TestArea ("QView",      "CtrlQ Tab",  "Tab CtrlQ")
  if Far.GetConfig("Panel.Tree.TurnOffCompletely") ~= true then
    TestArea ("Tree",       "CtrlT Tab",  "Tab CtrlT")
    TestArea ("FindFolder", "AltF10",     "Esc")
  end
  TestArea ("UserMenu",   "F2",         "Esc")

  TestArea("Shell")
  asrt.isfalse (Area.Other)
  asrt.isfalse (Area.Viewer)
  asrt.isfalse (Area.Editor)
  asrt.isfalse (Area.Dialog)
  asrt.isfalse (Area.Search)
  asrt.isfalse (Area.Disks)
  asrt.isfalse (Area.MainMenu)
  asrt.isfalse (Area.Menu)
  asrt.isfalse (Area.Help)
  asrt.isfalse (Area.Info)
  asrt.isfalse (Area.QView)
  asrt.isfalse (Area.Tree)
  asrt.isfalse (Area.FindFolder)
  asrt.isfalse (Area.UserMenu)
  asrt.isfalse (Area.ShellAutoCompletion)
  asrt.isfalse (Area.DialogAutoCompletion)
  asrt.isfalse (Area.Grabber)
  asrt.isfalse (Area.Desktop)
end

-- V = akey(Mode[,Type])
local function test_mf_akey()
  CheckMacroKeys()
  asrt.eq(akey, mf.akey)
  local key,name = akey(0),akey(1)
  local ok = false
  for refkey in MacroKeys:gmatch("%S+") do
    if type(key)=="number" and name==refkey then ok=true; break; end
  end
  asrt.istrue(ok)
  -- (the 2nd parameter is tested in function test_mf_eval).
end

local function test_mf_eval()
  CheckMacroKeys()
  asrt.eq(eval, mf.eval)

  -- test arguments validity checking
  asrt.eq (eval(""), 0)
  asrt.eq (eval("", 0), 0)
  asrt.eq (eval(), -1)
  asrt.eq (eval(0), -1)
  asrt.eq (eval(true), -1)
  asrt.eq (eval("", -1), -1)
  asrt.eq (eval("", 5), -1)
  asrt.eq (eval("", true), -1)
  asrt.eq (eval("", 1, true), -1)
  asrt.eq (eval("",1,"javascript"), -1)

  -- test macro-not-found error
  asrt.eq (eval("", 2), -2)

  -- We will modify the global 'temp'. Let it be restored when the macro terminates.
  -- luacheck: globals temp
  mf.AddExitHandler(function(v) temp=v; end, temp)
  temp=3
  asrt.eq (eval("temp=5+7"), 0)
  asrt.eq (temp, 12)

  temp=3
  asrt.eq (eval("temp=5+7",0,"moonscript"), 0)
  asrt.eq (eval("temp=5+7",1,"lua"), 0)
  asrt.eq (eval("temp=5+7",3,"lua"), "")
  asrt.eq (eval("temp=5+7",1,"moonscript"), 0)
  asrt.eq (eval("temp=5+7",3,"moonscript"), "")
  asrt.eq (temp, 3)
  asrt.eq (eval("getfenv(1).temp=12",0,"moonscript"), 0)
  asrt.eq (temp, 12)

  asrt.eq (eval("5",0,"moonscript"), 0)
  asrt.eq (eval("5+7",1,"lua"), 11)
  asrt.eq (eval("5+7",1,"moonscript"), 0)
  asrt.eq (eval("5 7",1,"moonscript"), 11)

  -- test with Mode==2
  local refKey1, refKey2 = MacroKeys:match("(%S*)%s*(%S*)")
  local code = ([[
    local key = akey(1,0)
    assert(key=="%s" or key=="%s")
    assert(akey(1,1)=="CtrlA")
    foobar = (foobar or 0) + 1
    return foobar,false,5,nil,"foo"
  ]]):format(refKey1, refKey2)
  local Id = asrt.udata(far.MacroAdd(nil,nil,"CtrlA",code))
  for k=1,3 do
    local ret1,a,b,c,d,e = eval("CtrlA",2)
    asrt.istrue(ret1==0 and a==k and b==false and c==5 and d==nil and e=="foo")
  end
  asrt.istrue(far.MacroDelete(Id))
end

local function test_mf_abs()
  asrt.eq (mf.abs(1.3), 1.3)
  asrt.eq (mf.abs(-1.3), 1.3)
  asrt.eq (mf.abs(0), 0)
end

local function test_mf_acall()
  local a,b,c,d = mf.acall(function(p) return 3, nil, p, "foo" end, 77)
  asrt.istrue (a==3 and b==nil and c==77 and d=="foo")
  asrt.istrue (mf.acall(far.Show))
  TestArea("Menu",nil,"Esc")
end

local function test_mf_asc()
  asrt.eq (mf.asc("0"), 48)
  asrt.eq (mf.asc("Я"), 1071)
end

local function test_mf_atoi()
  local function check(str, base)
    asrt.eq(mf.atoi(str,base), tonumber(str,base))
  end

  for _,v in ipairs { "0", "-10", "0x11" } do check(v) end

  check("1011", 2)
  check("1234", 5)
  asrt.eq(mf.atoi(-1234, 5), -194)

  for _,v in ipairs { "123456789123456789", "-123456789123456789",
                      "0x1B69B4BACD05F15", "-0x1B69B4BACD05F15" } do
    asrt.eq(mf.atoi(v), bit64.new(v))
  end
end

local function test_mf_chr()
  asrt.eq (mf.chr(48), "0")
  asrt.eq (mf.chr(1071), "Я")
end

local function test_mf_clip()
  local oldval = far.PasteFromClipboard() -- store

  mf.clip(5,2) -- turn on the internal clipboard
  asrt.eq (mf.clip(5,-1), 2)
  asrt.eq (mf.clip(5,1),  2) -- turn on the OS clipboard
  asrt.eq (mf.clip(5,-1), 1)

  for clipnum=1,2 do
    mf.clip(5,clipnum)
    local str = "foo"..clipnum
    assert(mf.clip(1,str) > 0)
    assert(mf.clip(0) == str)
    assert(mf.clip(2,"bar") > 0)
    assert(mf.clip(0) == str.."bar")
  end

  mf.clip(5,1); mf.clip(1,"foo")
  mf.clip(5,2); mf.clip(1,"bar")
  asrt.eq (mf.clip(0), "bar")
  mf.clip(5,1); asrt.eq (mf.clip(0), "foo")
  mf.clip(5,2); asrt.eq (mf.clip(0), "bar")

  mf.clip(3);   asrt.eq (mf.clip(0), "foo")
  mf.clip(5,1); asrt.eq (mf.clip(0), "foo")

  mf.clip(5,2); mf.clip(1,"bar")
  mf.clip(5,1); asrt.eq (mf.clip(0), "foo")
  mf.clip(4);   asrt.eq (mf.clip(0), "bar")
  mf.clip(5,2); asrt.eq (mf.clip(0), "bar")

  mf.clip(5,1) -- leave the OS clipboard active in the end
  far.CopyToClipboard(oldval or "") -- restore
end

local function test_mf_env()
  mf.env("Foo",1,"Bar")
  asrt.eq (mf.env("Foo"), "Bar")
  mf.env("Foo",1,"")
  asrt.eq (mf.env("Foo"), "")
end

local function test_mf_fattr()
  DeleteTmpFile()
  asrt.eq (mf.fattr(TmpFileName), -1)
  WriteTmpFile("")
  local attr = mf.fattr(TmpFileName)
  DeleteTmpFile()
  assert(attr >= 0)
end

local function test_mf_fexist()
  WriteTmpFile("")
  asrt.istrue(mf.fexist(TmpFileName))
  DeleteTmpFile()
  asrt.isfalse(mf.fexist(TmpFileName))
end

local function test_mf_msgbox()
  asrt.eq (msgbox, mf.msgbox)
  mf.postmacro(Keys, "Esc")
  asrt.eq (0, msgbox("title","message"))
  mf.postmacro(Keys, "Enter")
  asrt.eq (1, msgbox("title","message"))
end

local function test_mf_prompt()
  asrt.eq (prompt, mf.prompt)
  mf.postmacro(Keys, "a b c Esc")
  asrt.isfalse (prompt())
  mf.postmacro(Keys, "a b c Enter")
  asrt.eq ("abc", prompt())
end

local function test_mf_date()
  asrt.str (mf.date())
  asrt.str (mf.date("%a"))
end

local function test_mf_fmatch()
  asrt.eq (mf.fmatch("Readme.txt", "*.txt"), 1)
  asrt.eq (mf.fmatch("Readme.txt", "Readme.*|*.txt"), 0)
  asrt.eq (mf.fmatch("c:\\Readme.txt", "/txt$/i"), 1)
  asrt.eq (mf.fmatch("c:\\Readme.txt", "/txt$"), -1)
end

local function test_mf_fsplit()
  local path="C:\\Program Files\\Far\\Far.exe"
  asrt.eq (mf.fsplit(path,0x01), "C:")
  asrt.eq (mf.fsplit(path,0x02), "\\Program Files\\Far\\")
  asrt.eq (mf.fsplit(path,0x04), "Far")
  asrt.eq (mf.fsplit(path,0x08), ".exe")

  asrt.eq (mf.fsplit(path,0x03), "C:\\Program Files\\Far\\")
  asrt.eq (mf.fsplit(path,0x0C), "Far.exe")
  asrt.eq (mf.fsplit(path,0x0F), path)
end

local function test_mf_iif()
  asrt.eq (mf.iif(true,  1, 2), 1)
  asrt.eq (mf.iif("a",   1, 2), 1)
  asrt.eq (mf.iif(100,   1, 2), 1)
  asrt.eq (mf.iif(false, 1, 2), 2)
  asrt.eq (mf.iif(nil,   1, 2), 2)
  asrt.eq (mf.iif(0,     1, 2), 2)
  asrt.eq (mf.iif("",    1, 2), 2)
end

local function test_mf_index()
  asrt.eq (mf.index("language","gua",0), 3)
  asrt.eq (mf.index("language","gua",1), 3)
  asrt.eq (mf.index("language","gUA",1), -1)
  asrt.eq (mf.index("language","gUA",0), 3)
end

local function test_mf_int()
  asrt.eq (mf.int("2.99"), 2)
  asrt.eq (mf.int("-2.99"), -2)
  asrt.eq (mf.int("0x10"), 0)
  asrt.eq (mf.int("123456789123456789"), bit64.new("123456789123456789"))
  asrt.eq (mf.int("-123456789123456789"), bit64.new("-123456789123456789"))
end

local function test_mf_itoa()
  asrt.eq (mf.itoa(100), "100")
  asrt.eq (mf.itoa(100,10), "100")
  asrt.eq (mf.itoa(bit64.new("123456789123456789")), "123456789123456789")
  asrt.eq (mf.itoa(bit64.new("-123456789123456789")), "-123456789123456789")
  asrt.eq (mf.itoa(100,2), "1100100")
  asrt.eq (mf.itoa(100,16), "64")
  asrt.eq (mf.itoa(100,36), "2s")
end

local function test_mf_key()
  asrt.eq (mf.key(0x01000000), "Ctrl")
  asrt.eq (mf.key(0x02000000), "Alt")
  asrt.eq (mf.key(0x04000000), "Shift")
  asrt.eq (mf.key(0x10000000), "RCtrl")
  asrt.eq (mf.key(0x20000000), "RAlt")

  asrt.eq (mf.key(0x0501007B), "CtrlShiftF12")
  asrt.eq (mf.key("CtrlShiftF12"), "CtrlShiftF12")

  asrt.eq (mf.key("foobar"), "")
end

-- Separate tests for mf.float and mf.string are locale-dependant, thus they are tested together.
local function test_mf_float_and_string()
  local t = { 0, -0, 2.56e1, -5.37, -2.2e100, 2.2e-100 }
  for _,num in ipairs(t) do
    asrt.eq (mf.float(mf.string(num)), num)
  end
end

local function test_mf_lcase()
  asrt.eq (mf.lcase("FOo БАр"), "foo бар")
end

local function test_mf_len()
  asrt.eq (mf.len(""), 0)
  asrt.eq (mf.len("FOo БАр"), 7)
end

local function test_mf_max()
  asrt.eq (mf.max(-2,-5), -2)
  asrt.eq (mf.max(2,5), 5)
end

local function test_mf_min()
  asrt.eq (mf.min(-2,-5), -5)
  asrt.eq (mf.min(2,5), 2)
end

local function test_mf_msave()
  local Key = "macrotest"

  -- test supported types, except tables
  local v1, v2, v3, v4, v5, v6 = nil, false, true, -5.67, "foo", bit64.new("0x1234567843218765")
  mf.msave(Key, "name1", v1)
  mf.msave(Key, "name2", v2)
  mf.msave(Key, "name3", v3)
  mf.msave(Key, "name4", v4)
  mf.msave(Key, "name5", v5)
  mf.msave(Key, "name6", v6)
  asrt.eq (mf.mload(Key, "name1"), v1)
  asrt.eq (mf.mload(Key, "name2"), v2)
  asrt.eq (mf.mload(Key, "name3"), v3)
  asrt.eq (mf.mload(Key, "name4"), v4)
  asrt.eq (mf.mload(Key, "name5"), v5)
  asrt.eq (mf.mload(Key, "name6"), v6)
  mf.mdelete(Key, "*")
  asrt.eq (mf.mload(Key, "name3"), nil)

  -- test tables
  mf.msave(Key, "name1", { a=5, {b="foo"}, c={d=false} })
  local t=mf.mload(Key, "name1")
  asrt.istrue(t.a==5 and t[1].b=="foo" and t.c.d==false)
  mf.mdelete(Key, "name1")
  asrt.isnil(mf.mload(Key, "name1"))

  -- test tables more
  local t1, t2, t3 = {5}, {6}, {}
  t1[2], t1[3], t1[4], t1[5] = t1, t2, t3, t3
  t2[2], t2[3] = t1, t2
  t1[t1], t1[t2] = 66, 77
  t2[t1], t2[t2] = 88, 99
  setmetatable(t3, { __index=t1 })
  mf.msave(Key, "name1", t1)

  local T1 = asrt.table(mf.mload(Key, "name1"))
  local T2 = asrt.table(T1[3])
  local T3 = T1[4]
  assert(type(T3)=="table" and T3==T1[5])
  assert(T1[1]==5 and T1[2]==T1 and T1[3]==T2)
  assert(T2[1]==6 and T2[2]==T1 and T2[3]==T2)
  assert(T1[T1]==66 and T1[T2]==77)
  assert(T2[T1]==88 and T2[T2]==99)
  assert(getmetatable(T3).__index==T1 and T3[1]==5 and rawget(T3,1)==nil)
  mf.mdelete(Key, "*")
  asrt.isnil(mf.mload(Key, "name1"))

  -- test locations (profiles)
  if win.GetEnv("FARPROFILE") ~= win.GetEnv("FARLOCALPROFILE") then
    mf.msave("key1", "name1", 100)
    mf.msave("key2", "name2", 200, "roaming")
    mf.msave("key1", "name1", 300, "local")
    for _ = 1,2 do
      asrt.eq(mf.mload("key1", "name1"), 100)
      asrt.eq(mf.mload("key1", "name1", "roaming"), 100)
      asrt.eq(mf.mload("key2", "name2"), 200)
      asrt.eq(mf.mload("key2", "name2", "roaming"), 200)
      asrt.eq(mf.mload("key1", "name1", "local"), 300)
    end
    mf.mdelete("key1", "name1")
    mf.mdelete("key2", "name2", "roaming")
    asrt.isnil(mf.mload("key1", "name1"))
    asrt.isnil(mf.mload("key2", "name2"))
    asrt.eq(mf.mload("key1", "name1", "local"), 300)
    mf.mdelete("key1", "name1", "local")
    asrt.isnil(mf.mload("key1", "name1", "local"))
  end
end

local function test_mf_mod()
  asrt.eq (mf.mod(11,4), 3)
  asrt.eq (math.fmod(11,4), 3)
  asrt.eq (11 % 4, 3)

  asrt.eq (mf.mod(-1,4), -1)
  asrt.eq (math.fmod(-1,4), -1)
  asrt.eq (-1 % 4, 3)
end

local function test_mf_replace()
  asrt.eq (mf.replace("Foo Бар", "o", "1"), "F11 Бар")
  asrt.eq (mf.replace("Foo Бар", "o", "1", 1), "F1o Бар")
  asrt.eq (mf.replace("Foo Бар", "O", "1", 1, 1), "Foo Бар")
  asrt.eq (mf.replace("Foo Бар", "O", "1", 1, 0), "F1o Бар")
end

local function test_mf_rindex()
  asrt.eq (mf.rindex("language","a",0), 5)
  asrt.eq (mf.rindex("language","a",1), 5)
  asrt.eq (mf.rindex("language","A",1), -1)
  asrt.eq (mf.rindex("language","A",0), 5)
end

local function test_mf_strpad()
  asrt.eq (mf.strpad("Foo",10,"*",  2), '***Foo****')
  asrt.eq (mf.strpad("",   10,"-*-",2), '-*--*--*--')
  asrt.eq (mf.strpad("",   10,"-*-"), '-*--*--*--')
  asrt.eq (mf.strpad("Foo",10), 'Foo       ')
  asrt.eq (mf.strpad("Foo",10,"-"), 'Foo-------')
  asrt.eq (mf.strpad("Foo",10," ",  1), '       Foo')
  asrt.eq (mf.strpad("Foo",10," ",  2), '   Foo    ')
  asrt.eq (mf.strpad("Foo",10,"1234567890",2), '123Foo1234')
end

local function test_mf_strwrap()
  asrt.eq (mf.strwrap("Пример строки, которая будет разбита на несколько строк по ширине в 7 символов.", 7,"\n"),
[[
Пример
строки,
которая
будет
разбита
на
несколь
ко
строк
по
ширине
в 7
символо
в.]])
end

local function test_mf_substr()
  asrt.eq (mf.substr("abcdef", 1), "bcdef")
  asrt.eq (mf.substr("abcdef", 1, 3), "bcd")
  asrt.eq (mf.substr("abcdef", 0, 4), "abcd")
  asrt.eq (mf.substr("abcdef", 0, 8), "abcdef")
  asrt.eq (mf.substr("abcdef", -1), "f")
  asrt.eq (mf.substr("abcdef", -2), "ef")
  asrt.eq (mf.substr("abcdef", -3, 1), "d")
  asrt.eq (mf.substr("abcdef", 0, -1), "abcde")
  asrt.eq (mf.substr("abcdef", 2, -1), "cde")
  asrt.eq (mf.substr("abcdef", 4, -4), "")
  asrt.eq (mf.substr("abcdef", -3, -1), "de")
end

local function test_mf_testfolder()
  assert(mf.testfolder(".") > 0)
  assert(mf.testfolder("C:\\") == 2)
  assert(mf.testfolder("@:\\") <= 0)
end

local function test_mf_trim()
  asrt.eq (mf.trim(" abc "), "abc")
  asrt.eq (mf.trim(" abc ",0), "abc")
  asrt.eq (mf.trim(" abc ",1), "abc ")
  asrt.eq (mf.trim(" abc ",2), " abc")
end

local function test_mf_ucase()
  asrt.eq (mf.ucase("FOo БАр"), "FOO БАР")
end

local function test_mf_waitkey()
  asrt.eq (mf.waitkey(50,0), "")
  asrt.eq (mf.waitkey(50,1), 0xFFFFFFFF)
end

local function test_mf_size2str()
  local F_COMMAS         = bit64.new("0x0800000000000000")
  local F_FLOATSIZE      = bit64.new("0x0080000000000000")
  local F_SHOWBYTESINDEX = bit64.new("0x0010000000000000")
  local F_ECONOMIC       = bit64.new("0x0040000000000000")
  local F_THOUSAND       = bit64.new("0x0400000000000000")
  local F_MINSIZEINDEX   = bit64.new("0x0020000000000000")

  asrt.eq(mf.size2str(123456, 0), "123456")
  asrt.eq(mf.size2str(123456, 0, 8), "  123456")
  asrt.eq(mf.size2str(123456, 0, -8), "123456  ")
  asrt.eq(mf.size2str(123456, F_COMMAS), "123,456")
  asrt.eq(mf.size2str(123456, F_SHOWBYTESINDEX), "123456 B")
  asrt.eq(mf.size2str(123456, F_FLOATSIZE), "121 K")
  asrt.eq(mf.size2str(123456, F_FLOATSIZE+F_ECONOMIC), "121K")
  asrt.eq(mf.size2str(123456, F_FLOATSIZE+F_THOUSAND), "123 k")
  asrt.eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x3), "4 T")
  asrt.eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x2), "4112 G")
  asrt.eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x1), "4210688 M")
  asrt.eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x0), "4311744512 K")
end

local function test_mf_xlat()
  asrt.str (mf.xlat("abc"))
  -- commented out, as these tests won't work with any Windows configuration:
  --assert(mf.xlat("ghzybr")=="пряник")
  --assert(mf.xlat("сщьзгеук")=="computer")
end

local function test_mf_beep()
  asrt.bool (mf.beep())
end

local function test_mf_flock()
  for k=0,2 do asrt.num (mf.flock(k,-1)) end
end

local function test_mf_GetMacroCopy()
  asrt.func (mf.GetMacroCopy)
end

local function test_mf_Keys()
  asrt.eq (Keys, mf.Keys)
  asrt.func (Keys)

  Keys("Esc F a r Space M a n a g e r Space Ф А Р")
  asrt.eq (panel.GetCmdLine(), "Far Manager ФАР")
  Keys("Esc")
  asrt.eq (panel.GetCmdLine(), "")
end

local function test_mf_exit()
  asrt.eq (exit, mf.exit)
  local N
  mf.postmacro(
    function()
      local function f() N=50; exit(); end
      f(); N=100
    end)
  mf.postmacro(Keys, "Esc")
  far.Message("dummy")
  asrt.eq (N, 50)
end

local function test_mf_mmode()
  asrt.eq (mmode, mf.mmode)
  asrt.eq (1, mmode(1,-1))
end

local function test_mf_print()
  asrt.eq (print, mf.print)
  asrt.func (print)
  -- test on command line
  local str = "abc ABC абв АБВ"
  Keys("Esc")
  print(str)
  asrt.eq (panel.GetCmdLine(), str)
  Keys("Esc")
  asrt.eq (panel.GetCmdLine(), "")
  -- test on dialog input field
  Keys("F7 CtrlY")
  print(str)
  asrt.eq (Dlg.GetValue(-1,0), str)
  Keys("Esc")
  -- test on editor
  str = "abc ABC\r\nабв АБВ"
  Keys("ShiftF4")
  print(TmpFileName)
  Keys("Enter CtrlHome Enter Up")
  print(str)
  Keys("CtrlHome"); assert(Editor.Value == "abc ABC")
  Keys("Down");     assert(Editor.Value == "абв АБВ")
  editor.Quit()
end

local function test_mf_postmacro()
  asrt.func (mf.postmacro)
end

local function test_mf_sleep()
  asrt.func (mf.sleep)
end

local function test_mf_usermenu()
  asrt.func (mf.usermenu)
end

function MT.test_mf()
  test_mf_abs()
  test_mf_acall()
  test_mf_akey()
  test_mf_asc()
  test_mf_atoi()
  test_mf_beep()
  test_mf_chr()
  test_mf_clip()
  test_mf_date()
  test_mf_env()
  test_mf_eval()
  test_mf_exit()
  test_mf_fattr()
  test_mf_fexist()
  test_mf_float_and_string()
  test_mf_flock()
  test_mf_fmatch()
  test_mf_fsplit()
  test_mf_GetMacroCopy()
  test_mf_iif()
  test_mf_index()
  test_mf_int()
  test_mf_itoa()
  test_mf_key()
  test_mf_Keys()
  test_mf_lcase()
  test_mf_len()
  test_mf_max()
  test_mf_min()
  test_mf_mmode()
  test_mf_mod()
  test_mf_msave()
  test_mf_msgbox()
  test_mf_postmacro()
  test_mf_print()
  test_mf_prompt()
  test_mf_replace()
  test_mf_rindex()
  test_mf_size2str()
  test_mf_sleep()
  test_mf_strpad()
  test_mf_strwrap()
  test_mf_substr()
  test_mf_testfolder()
  test_mf_trim()
  test_mf_ucase()
  test_mf_usermenu()
  test_mf_waitkey()
  test_mf_xlat()
end

function MT.test_CmdLine()
  Keys"Esc f o o Space Б а р"
  asrt.isfalse(CmdLine.Bof)
  asrt.istrue(CmdLine.Eof)
  asrt.isfalse(CmdLine.Empty)
  asrt.isfalse(CmdLine.Selected)
  asrt.eq (CmdLine.Value, "foo Бар")
  asrt.eq (CmdLine.ItemCount, 7)
  asrt.eq (CmdLine.CurPos, 8)

  Keys"SelWord"
  asrt.istrue(CmdLine.Selected)

  Keys"CtrlHome"
  asrt.istrue(CmdLine.Bof)
  asrt.isfalse(CmdLine.Eof)

  Keys"Esc"
  asrt.istrue(CmdLine.Bof)
  asrt.istrue(CmdLine.Eof)
  asrt.istrue(CmdLine.Empty)
  asrt.isfalse(CmdLine.Selected)
  asrt.eq (CmdLine.Value, "")
  asrt.eq (CmdLine.ItemCount, 0)
  asrt.eq (CmdLine.CurPos, 1)

  Keys"Esc"
  print("foo Бар")
  asrt.eq (CmdLine.Value, "foo Бар")

  Keys"Esc"
  print(("%s %d %s"):format("foo", 5+7, "Бар"))
  asrt.eq (CmdLine.Value, "foo 12 Бар")

  Keys"Esc"
end

local function test_Far_GetConfig()
  local t = {
    "Cmdline.AutoComplete", "boolean",
    "Cmdline.EditBlock", "boolean",
    "Cmdline.DelRemovesBlocks", "boolean",
    "Cmdline.PromptFormat", "string",
    "Cmdline.UsePromptFormat", "boolean",
    "CodePages.CPMenuMode", "boolean",
    "CodePages.NoAutoDetectCP", "string",
    "Confirmations.AllowReedit", "boolean",
    "Confirmations.Copy", "boolean",
    "Confirmations.Delete", "boolean",
    "Confirmations.DeleteFolder", "boolean",
    "Confirmations.DetachVHD", "boolean",
    "Confirmations.Drag", "boolean",
    "Confirmations.Esc", "boolean",
    "Confirmations.EscTwiceToInterrupt", "boolean",
    "Confirmations.Exit", "boolean",
    "Confirmations.HistoryClear", "boolean",
    "Confirmations.Move", "boolean",
    "Confirmations.RemoveConnection", "boolean",
    "Confirmations.RemoveHotPlug", "boolean",
    "Confirmations.RemoveSUBST", "boolean",
    "Confirmations.RO", "boolean",
    "Descriptions.AnsiByDefault", "boolean",
    "Descriptions.ListNames", "string",
    "Descriptions.ROUpdate", "boolean",
    "Descriptions.SaveInUTF", "boolean",
    "Descriptions.SetHidden", "boolean",
    "Descriptions.StartPos", "integer",
    "Descriptions.UpdateMode", "integer",
    "Dialog.AutoComplete", "boolean",
    "Dialog.CBoxMaxHeight", "integer",
    "Dialog.EditBlock", "boolean",
    "Dialog.EditHistory", "boolean",
    --"Dialog.EditLine", "integer",
    "Dialog.DelRemovesBlocks", "boolean",
    "Dialog.EULBsClear", "boolean",
    "Dialog.MouseButton", "integer",
    "Editor.AddUnicodeBOM", "boolean",
    "Editor.AllowEmptySpaceAfterEof", "boolean",
    "Editor.AutoDetectCodePage", "boolean",
    "Editor.AutoIndent", "boolean",
    "Editor.BSLikeDel", "boolean",
    "Editor.CharCodeBase", "integer",
    "Editor.DefaultCodePage", "integer",
    "Editor.DelRemovesBlocks", "boolean",
    "Editor.EditOpenedForWrite", "boolean",
    "Editor.EditorCursorBeyondEOL", "boolean",
    "Editor.ExpandTabs", "integer",
    "Editor.ExternalEditorName", "string",
    "Editor.FileSizeLimit", "integer",
    "Editor.KeepEditorEOL", "boolean",
    "Editor.PersistentBlocks", "boolean",
    "Editor.ReadOnlyLock", "integer",
    "Editor.SaveEditorPos", "boolean",
    "Editor.SaveEditorShortPos", "boolean",
    -- "Editor.SearchRegexp", "boolean", // Removed in 6135
    "Editor.SearchSelFound", "boolean",
    "Editor.SearchCursorAtEnd", "boolean",
    "Editor.ShowKeyBar", "boolean",
    "Editor.ShowScrollBar", "boolean",
    "Editor.ShowTitleBar", "boolean",
    "Editor.ShowWhiteSpace", "3-state",
    "Editor.TabSize", "integer",
    "Editor.UndoDataSize", "integer",
    "Editor.UseExternalEditor", "boolean",
    "Editor.WordDiv", "string",
    "Help.ActivateURL", "integer",
    -- "Help.HelpSearchRegexp", "boolean", // Removed in 6135
    "History.CommandHistory.Count", "integer",
    "History.CommandHistory.Lifetime", "integer",
    "History.DialogHistory.Count", "integer",
    "History.DialogHistory.Lifetime", "integer",
    "History.FolderHistory.Count", "integer",
    "History.FolderHistory.Lifetime", "integer",
    "History.ViewEditHistory.Count", "integer",
    "History.ViewEditHistory.Lifetime", "integer",
    "Interface.DelHighlightSelected", "boolean",
    "Interface.DelShowSelected", "integer",
    "Interface.DelShowTotal", "boolean",
    "Interface.AltF9", "boolean",
    "Interface.ClearType", "boolean",
    "Interface.CopyShowTotal", "boolean",
    "Interface.CtrlPgUp", "3-state",
    "Interface.CursorSize1", "integer",
    "Interface.CursorSize2", "integer",
    "Interface.CursorSize3", "integer",
    "Interface.CursorSize4", "integer",
    "Interface.EditorTitleFormat", "string",
    "Interface.FormatNumberSeparators", "string",
    "Interface.Mouse", "boolean",
    "Interface.SetIcon", "boolean",
    "Interface.SetAdminIcon", "boolean",
    "Interface.ShiftsKeyRules", "boolean",
    "Interface.ShowDotsInRoot", "boolean",
    "Interface.ShowMenuBar", "boolean",
    "Interface.RedrawTimeout", "integer",
    "Interface.TitleAddons", "string",
    "Interface.UseVk_oem_x", "boolean",
    "Interface.ViewerTitleFormat", "string",
    "Interface.Completion.Append", "boolean",
    "Interface.Completion.ModalList", "boolean",
    "Interface.Completion.ShowList", "boolean",
    "Interface.Completion.UseFilesystem", "3-state",
    "Interface.Completion.UseHistory", "3-state",
    "Interface.Completion.UsePath", "3-state",
    "Language.Main", "string",
    "Language.Help", "string",
    "Layout.FullscreenHelp", "boolean",
    "Layout.LeftHeightDecrement", "integer",
    "Layout.RightHeightDecrement", "integer",
    "Layout.WidthDecrement", "integer",
    "Macros.CONVFMT", "string",
    "Macros.DateFormat", "string",
    "Macros.KeyRecordCtrlDot", "string",
    "Macros.KeyRecordRCtrlDot", "string",
    "Macros.KeyRecordCtrlShiftDot", "string",
    "Macros.KeyRecordRCtrlShiftDot", "string",
    "Macros.ShowPlayIndicator", "boolean",
    "Panel.AutoUpdateLimit", "integer",
    "Panel.CtrlAltShiftRule", "integer",
    "Panel.CtrlFRule", "boolean",
    "Panel.Highlight", "boolean",
    "Panel.ReverseSort", "boolean",
    "Panel.RememberLogicalDrives", "boolean",
    "Panel.RightClickRule", "integer",
    "Panel.SelectFolders", "boolean",
    "Panel.ShellRightLeftArrowsRule", "boolean",
    "Panel.ShowHidden", "boolean",
    "Panel.ShortcutAlwaysChdir", "boolean",
    "Panel.SortFolderExt", "boolean",
    "Panel.RightClickSelect", "boolean",
    "Panel.Info.InfoComputerNameFormat", "integer",
    "Panel.Info.InfoUserNameFormat", "integer",
    "Panel.Info.ShowCDInfo", "boolean",
    "Panel.Info.ShowPowerStatus", "boolean",
    "Panel.Layout.ColoredGlobalColumnSeparator", "boolean",
    "Panel.Layout.ColumnTitles", "boolean",
    "Panel.Layout.DetailedJunction", "boolean",
    "Panel.Layout.DoubleGlobalColumnSeparator", "boolean",
    "Panel.Layout.FreeInfo", "boolean",
    "Panel.Layout.ScreensNumber", "boolean",
    "Panel.Layout.Scrollbar", "boolean",
    "Panel.Layout.ScrollbarMenu", "boolean",
    "Panel.Layout.ShowUnknownReparsePoint", "boolean",
    "Panel.Layout.SortMode", "boolean",
    "Panel.Layout.StatusLine", "boolean",
    "Panel.Layout.TotalInfo", "boolean",
    "Panel.Left.DirectoriesFirst", "boolean",
    "Panel.Left.SelectedFirst", "boolean",
    "Panel.Left.ShortNames", "boolean",
    "Panel.Left.SortGroups", "boolean",
    "Panel.Left.SortMode", "integer",
    "Panel.Left.ReverseSortOrder", "boolean",
    "Panel.Left.Type", "integer",
    "Panel.Left.ViewMode", "integer",
    "Panel.Left.Visible", "boolean",
    "Panel.Right.DirectoriesFirst", "boolean",
    "Panel.Right.SelectedFirst", "boolean",
    "Panel.Right.ShortNames", "boolean",
    "Panel.Right.SortGroups", "boolean",
    "Panel.Right.SortMode", "integer",
    "Panel.Right.ReverseSortOrder", "boolean",
    "Panel.Right.Type", "integer",
    "Panel.Right.ViewMode", "integer",
    "Panel.Right.Visible", "boolean",
    "Panel.Tree.AutoChangeFolder", "boolean",
    "Panel.Tree.MinTreeCount", "integer",
    "Panel.Tree.TreeFileAttr", "integer",
    "Panel.Tree.TurnOffCompletely", "boolean",
    "PluginConfirmations.EvenIfOnlyOnePlugin", "boolean",
    "PluginConfirmations.OpenFilePlugin", "3-state",
    "PluginConfirmations.Prefix", "boolean",
    "PluginConfirmations.SetFindList", "boolean",
    "PluginConfirmations.StandardAssociation", "boolean",
    "Policies.ShowHiddenDrives", "boolean",
    "Screen.Clock", "boolean",
    "Screen.DeltaX", "integer",
    "Screen.DeltaY", "integer",
    "Screen.KeyBar", "boolean",
    "Screen.ScreenSaver", "boolean",
    "Screen.ScreenSaverTime", "integer",
    "Screen.ViewerEditorClock", "boolean",
    "System.AllCtrlAltShiftRule", "integer",
    "System.AutoSaveSetup", "boolean",
    "System.AutoUpdateRemoteDrive", "boolean",
    "System.BoxSymbols", "string",
    "System.CASRule", "integer",
    "System.CloseCDGate", "boolean",
    "System.CmdHistoryRule", "boolean",
    "System.ConsoleDetachKey", "string",
    "System.CopyBufferSize", "integer",
    "System.CopyOpened", "boolean",
    "System.CopyTimeRule", "integer",
    "System.CopySecurityOptions", "integer",
    "System.DeleteToRecycleBin", "boolean",
    "System.DriveDisconnectMode", "boolean",
    "System.DriveMenuMode", "integer",
    "System.ElevationMode", "integer",
    "System.ExcludeCmdHistory", "integer",
    "System.FileSearchMode", "integer",
    "System.FindAlternateStreams", "boolean",
    "System.FindCodePage", "integer",
    "System.FindFolders", "boolean",
    "System.FindSymLinks", "boolean",
    "System.FlagPosixSemantics", "boolean",
    "System.FolderInfo", "string",
    "System.MsWheelDelta", "integer",
    "System.MsWheelDeltaEdit", "integer",
    "System.MsWheelDeltaHelp", "integer",
    "System.MsWheelDeltaView", "integer",
    "System.MsHWheelDelta", "integer",
    "System.MsHWheelDeltaEdit", "integer",
    "System.MsHWheelDeltaView", "integer",
    "System.MultiCopy", "boolean",
    "System.MultiMakeDir", "boolean",
    "System.OEMPluginsSupport", "boolean",
    "System.PluginMaxReadData", "integer",
    "System.QuotedName", "integer",
    "System.QuotedSymbols", "string",
    "System.SaveHistory", "boolean",
    "System.SaveFoldersHistory", "boolean",
    "System.SaveViewHistory", "boolean",
    "System.ScanJunction", "boolean",
    "System.ScanSymlinks", "boolean",
    "System.SearchInFirstSize", "string",
    "System.SearchOutFormat", "string",
    "System.SearchOutFormatWidth", "string",
    "System.SetAttrFolderRules", "boolean",
    "System.ShowCheckingFile", "boolean",
    "System.ShowStatusInfo", "string",
    "System.SilentLoadPlugin", "boolean",
    "System.SubstNameRule", "integer",
    "System.SubstPluginPrefix", "boolean",
    "System.UpdateEnvironment", "boolean",
    "System.UseFilterInSearch", "boolean",
    "System.UseRegisteredTypes", "boolean",
    "System.UseSystemCopy", "boolean",
    "System.WindowMode", "boolean",
    "System.WipeSymbol", "integer",
    "System.KnownIDs.EMenu", "string",
    "System.KnownIDs.Network", "string",
    "System.KnownIDs.Arclite", "string",
    "System.KnownIDs.Luamacro", "string",
    "System.KnownIDs.Netbox", "string",
    "System.Nowell.MoveRO", "boolean",
    "System.Exception.FarEventSvc", "string",
    "System.Exception.Used", "boolean",
    "System.Executor.~", "string",
    "System.Executor.ComspecArguments", "string",
    "System.Executor.ExcludeCmds", "string",
    "System.Executor.FullTitle", "boolean",
    "System.Executor.RestoreCP", "boolean",
    "System.Executor.UseAppPath", "boolean",
    "System.Executor.UseHomeDir", "boolean",
    "System.WordDiv", "string",
    "Viewer.AutoDetectCodePage", "boolean",
    "Viewer.DefaultCodePage", "integer",
    "Viewer.ExternalViewerName", "string",
    "Viewer.IsWrap", "boolean",
    "Viewer.MaxLineSize", "integer",
    "Viewer.PersistentBlocks", "boolean",
    "Viewer.SaveViewerCodepage", "boolean",
    "Viewer.SaveViewerPos", "boolean",
    "Viewer.SaveViewerShortPos", "boolean",
    "Viewer.SaveViewerWrapMode", "boolean",
    -- "Viewer.SearchEditFocus", "boolean", // Removed in 6099
    -- "Viewer.SearchRegexp", "boolean", // Removed in 6135
    "Viewer.SearchWrapStop", "3-state",
    "Viewer.ShowArrows", "boolean",
    "Viewer.ShowKeyBar", "boolean",
    "Viewer.ShowScrollbar", "boolean",
    "Viewer.TabSize", "integer",
    "Viewer.ShowTitleBar", "boolean",
    "Viewer.UseExternalViewer", "boolean",
    "Viewer.Visible0x00", "boolean",
    "Viewer.Wrap", "boolean",
    "Viewer.ZeroChar", "integer",
    "VMenu.LBtnClick", "integer",
    "VMenu.MBtnClick", "integer",
    "VMenu.RBtnClick", "integer",
    "XLat.Flags", "integer",
    "XLat.Layouts", "string",
    "XLat.Rules1", "string",
    "XLat.Rules2", "string",
    "XLat.Rules3", "string",
    "XLat.Table1", "string",
    "XLat.Table2", "string",
    "XLat.WordDivForXlat", "string",
  }
  for k=1,#t,2 do
    local val,tp = Far.GetConfig(t[k])
    assert(tp == t[k+1])
    if tp=="boolean" or tp=="string" then assert(type(val)==tp)
    elseif tp=="integer" then assert(type(val)=="number" or bit64.type(val))
    elseif tp=="3-state" then assert(type(val)=="boolean" or val=="other")
    else assert(false)
    end
  end
end

function MT.test_Far()
  asrt.bool (Far.FullScreen)
  asrt.num (Far.Height)
  asrt.bool (Far.IsUserAdmin)
  asrt.num (Far.PID)
  asrt.str (Far.Title)
  asrt.num (Far.Width)

  local temp = Far.UpTime
  mf.sleep(50)
  temp = Far.UpTime - temp
  assert(temp > 40 and temp < 80, temp)

  asrt.num (Far.GetConfig("Editor.defaultcodepage"))
  asrt.func (Far.Cfg_Get)
  asrt.func (Far.DisableHistory)
  asrt.num (Far.KbdLayout(0))
  asrt.num (Far.KeyBar_Show(0))
  asrt.func (Far.Window_Scroll)

  -- test_Far_GetConfig()
end

local function test_CheckAndGetHotKey()
  mf.acall(far.Menu, {Flags="FMENU_AUTOHIGHLIGHT"},
    {{text="abcd"},{text="abc&d"},{text="abcd"},{text="abcd"},{text="abcd"}})

  asrt.eq (Object.CheckHotkey("a"), 1)
  asrt.eq (Object.GetHotkey(1), "a")
  asrt.eq (Object.GetHotkey(), "a")
  asrt.eq (Object.GetHotkey(0), "a")

  asrt.eq (Object.CheckHotkey("b"), 3)
  asrt.eq (Object.GetHotkey(3), "b")

  asrt.eq (Object.CheckHotkey("c"), 4)
  asrt.eq (Object.GetHotkey(4), "c")

  asrt.eq (Object.CheckHotkey("d"), 2)
  asrt.eq (Object.GetHotkey(2), "d")

  asrt.eq (Object.CheckHotkey("e"), 0)

  asrt.eq (Object.CheckHotkey(""), 5)
  asrt.eq (Object.GetHotkey(5), "")
  asrt.eq (Object.GetHotkey(6), "")

  Keys("Esc")
end

--[[------------------------------------------------------------------------------------------------
0002554: При фильтрации видны элементы меню скрытые с помощью MIF_HIDDEN

Description:
Если активировать фильтр меню (RAlt), и начать фильтровать список, то подходящие под фильтр элементы
(с флагом MIF_HIDDEN) также будут видны.

Даже если ни один из скрытых элементов не подходит, то при отмене фильтра все они появятся.
--]]------------------------------------------------------------------------------------------------
local function test_mantis_2554()
  local Items = {
    {text="hidden", hidden=true}, {text="visible"},
  }

  local AssertInvariant = function()
    asrt.eq(0x20, Menu.ItemStatus(1)) -- Если флаг установлен, то пункт меню не выводится на экран.
    asrt.eq(0x01, Menu.ItemStatus(2)) -- Признак активности. Только один пункт может быть активным.
  end
  local AssertFilterIsOff = function() asrt.eq(0, Menu.Filter(0,-1)) end
  local AssertFilterIsOn  = function() asrt.eq(1, Menu.Filter(0,-1)) end

  -- open the menu
  asrt.istrue(Area.Shell)
  mf.acall(far.Menu,{},Items)
  asrt.istrue(Area.Menu)
  AssertFilterIsOff()
  AssertInvariant()

  -- turn the menu filter on
  Keys("CtrlAltF")
  AssertFilterIsOn()
  AssertInvariant()

  -- input something into the filter
  Keys("I")
  AssertInvariant()
  Keys("BS")
  AssertInvariant()
  AssertFilterIsOn()

  -- turn the menu filter off
  Keys("CtrlAltF")
  AssertFilterIsOff()
  AssertInvariant()

  -- close the menu
  asrt.istrue(Area.Menu)
  Keys("Esc")
  asrt.istrue(Area.Shell)
end

function MT.test_Menu()
  test_mantis_2554()

  Keys("F11")
  asrt.str(Menu.Value)
  asrt.eq(Menu.Id, far.Guids.PluginsMenuId)
  asrt.eq(Menu.Id, "937F0B1C-7690-4F85-8469-AA935517F202")
  asrt.num(Menu.HorizontalAlignment)
  Keys("Esc")

  asrt.func(Menu.Filter)
  asrt.func(Menu.FilterStr)
  asrt.func(Menu.GetItemExtendedData)
  asrt.func(Menu.GetValue)
  asrt.func(Menu.ItemStatus)
  asrt.func(Menu.Select)
  asrt.func(Menu.SetItemExtendedData)
  asrt.func(Menu.Show)
end

function MT.test_Object()
  asrt.bool (Object.Bof)
  asrt.num (Object.CurPos)
  asrt.bool (Object.Empty)
  asrt.bool (Object.Eof)
  asrt.num (Object.Height)
  asrt.num (Object.ItemCount)
  asrt.bool (Object.Selected)
  asrt.str (Object.Title)
  asrt.num (Object.Width)

  test_CheckAndGetHotKey()
end

function MT.test_Drv()
  Keys"AltF1"
  asrt.num (Drv.ShowMode)
  asrt.eq (Drv.ShowPos, 1)
  Keys"Esc AltF2"
  asrt.num (Drv.ShowMode)
  asrt.eq (Drv.ShowPos, 2)
  Keys"Esc"
end

function MT.test_Help()
  Keys"F1"
  asrt.str (Help.FileName)
  asrt.str (Help.SelTopic)
  asrt.str (Help.Topic)
  Keys"Esc"
end

function MT.test_Mouse()
  asrt.num (Mouse.X)
  asrt.num (Mouse.Y)
  asrt.num (Mouse.Button)
  asrt.num (Mouse.CtrlState)
  asrt.num (Mouse.EventFlags)
  asrt.num (Mouse.LastCtrlState)
end

function MT.test_Dlg()
  Keys"F7 a b c"
  asrt.istrue(Area.Dialog)
  asrt.eq(Dlg.Id, "FAD00DBE-3FFF-4095-9232-E1CC70C67737")
  asrt.eq(Dlg.Owner, "00000000-0000-0000-0000-000000000000")
  assert(Dlg.ItemCount > 6)
  asrt.eq(Dlg.ItemType, 4)
  asrt.eq(Dlg.CurPos, 3)
  asrt.eq(Dlg.PrevPos, 0)

  Keys"Tab"
  local pos = Dlg.CurPos
  assert(Dlg.CurPos > 3)
  asrt.eq(Dlg.PrevPos, 3)
  asrt.eq(pos, Dlg.SetFocus(3))
  asrt.eq(pos, Dlg.PrevPos)

  asrt.eq(Dlg.GetValue(0,0), Dlg.ItemCount)
  Keys"Esc"

  Keys"F10"
  asrt.istrue(Area.Dialog)
  asrt.str(Dlg.Id)
  asrt.eq(Dlg.Id, far.Guids.FarAskQuitId)
  Keys"Esc"
end

function MT.test_Plugin()
  -- Plugin.Menu
  asrt.isfalse(Plugin.Menu())
  asrt.istrue(Plugin.Menu(luamacroId, "EF6D67A2-59F7-4DF3-952E-F9049877B492")) -- call macrobrowser
  asrt.istrue(Area.Menu)
  asrt.eq(Menu.Id, "03DEFB28-8734-4EC0-8B25-C879846F0BE5")
  Keys("Esc")
  asrt.istrue(Area.Shell)

  -- Plugin.Config
  asrt.isfalse(Plugin.Config())
  if Plugin.Exist(hlfviewerId) then
    asrt.istrue(Plugin.Config(hlfviewerId))
    TestArea("Dialog", nil, "Esc")
    asrt.istrue(Area.Shell)
  end

  -- Plugin.Command
  asrt.isfalse(Plugin.Command())
  asrt.istrue(Plugin.Command(luamacroId))
  asrt.istrue(Plugin.Command(luamacroId, "luas:=5,6,7"))
  TestArea("Menu", nil, "Esc")
  asrt.istrue(Area.Shell)

  -- Plugin.Exist
  asrt.istrue(Plugin.Exist(luamacroId))
  asrt.isfalse(Plugin.Exist(luamacroId:gsub("^...","000")))

  -- Plugin.Call, Plugin.SyncCall
  local function test (func, N) -- test arguments and return
    local i1 = bit64.new("0x8765876587658765")
    local a1,a2,a3,a4,a5 = "foo", i1, -2.34, false, {[TKEY_BINARY]="foo\0bar"}
    local r1,r2,r3,r4,r5 = func(luamacroId, "argtest", a1,a2,a3,a4,a5)
    assert(r1==a1 and r2==a2 and r3==a3 and r4==a4
      and type(r5)=="table" and r5[TKEY_BINARY]==a5[TKEY_BINARY])

    local src = {}
    for k=1,N do src[k]=k end
    local trg = { func(luamacroId, "argtest", unpack(src)) }
    assert(#trg==N and trg[1]==1 and trg[N]==N)
  end
  test(Plugin.Call, 8000-8)
  test(Plugin.SyncCall, 8000-8)
end

-- Test in particular that Plugin.Call (a so-called "restricted" function) works properly
-- from inside a deeply nested coroutine.
function MT.test_coroutine()
  for k=1,2 do
    local Call = k==1 and Plugin.Call or Plugin.SyncCall
    local function f1()
      coroutine.yield(Call(luamacroId, "argtest", 1, false, "foo", nil))
    end
    local function f2() return coroutine.resume(coroutine.create(f1)) end
    local function f3() return coroutine.resume(coroutine.create(f2)) end
    local function f4() return coroutine.resume(coroutine.create(f3)) end
    local t = pack(f4())
    assert(t.n==7 and t[1]==true and t[2]==true and t[3]==true and
           t[4]==1 and t[5]==false and t[6]=="foo" and t[7]==nil)
  end
end

function MT.test_all()
  asrt.istrue(Area.Shell, "Run these tests from the Shell area.")
  asrt.isfalse(APanel.Plugin or PPanel.Plugin, "Run these tests when neither of panels is a plugin panel.")

  MT.test_areas()
  MT.test_mf()
  MT.test_CmdLine()
  MT.test_Help()
  MT.test_Dlg()
  MT.test_Drv()
  MT.test_Far()
  MT.test_Menu()
  MT.test_Mouse()
  MT.test_Object()
  MT.test_Plugin()
  MT.test_coroutine()

  MT.Mod_Dialog.test_all()
  MT.Mod_Editor.test_all()
  MT.Mod_FSF.test_all()
  MT.Mod_Luafar.test_all()
  MT.Mod_Panel.test_all()
end

return MT
