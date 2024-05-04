-- encoding: utf-8
-- Started: 2012-08-20.

--[[
-- The following macro can be used to run all the tests.

Macro {
  description="Macro-engine test";
  area="Shell"; key="CtrlShiftF12";
  action = function()
    Far.DisableHistory(0x0F)
    local f = assert(loadfile(far.PluginStartupInfo().ModuleDir.."macrotest.lua"))
    setfenv(f, getfenv())().test_all()
    far.Message("All tests OK", "LuaMacro")
  end;
}
--]]

local function assert_eq(a,b)      assert(a == b)               return true; end
local function assert_neq(a,b)     assert(a ~= b)               return true; end
local function assert_num(v)       assert(type(v)=="number")    return v; end
local function assert_str(v)       assert(type(v)=="string")    return v; end
local function assert_table(v)     assert(type(v)=="table")     return v; end
local function assert_bool(v)      assert(type(v)=="boolean")   return v; end
local function assert_func(v)      assert(type(v)=="function")  return v; end
local function assert_udata(v)     assert(type(v)=="userdata")  return v; end
local function assert_nil(v)       assert(v==nil)               return v; end
local function assert_false(v)     assert(v==false)             return v; end
local function assert_true(v)      assert(v==true)              return v; end
local function assert_falsy(v)     assert(not v == true)        return v; end
local function assert_truthy(v)    assert(not v == false)       return v; end

local function assert_range(val, low, high)
  if low then assert(val >= low) end
  if high then assert(val <= high) end
  return true
end

local MT = {} -- "macrotest", this module
local F = far.Flags
local band, bor, bnot = bit64.band, bit64.bor, bit64.bnot
local luamacroId="4ebbefc8-2084-4b7f-94c0-692ce136894d" -- LuaMacro plugin GUID

local function pack (...)
  return { n=select("#",...), ... }
end

local function IsNumOrInt(v)
  return type(v)=="number" or bit64.type(v)
end

local TmpFileName = win.JoinPath(assert(win.GetEnv"tmp" or win.GetEnv"temp"), "tmp.tmp")

local function WriteTmpFile(...)
  local fp = assert(io.open(TmpFileName,"w"))
  fp:write(...)
  fp:close()
end

local function DeleteTmpFile()
  win.DeleteFile(TmpFileName)
end

local function TestArea (area, msg)
  assert(Area[area]==true and Area.Current==area, msg or "assertion failed!")
end

function MT.test_areas()
  Keys "AltIns"              TestArea "Grabber"    Keys "Esc"
  Keys "F12 0"               TestArea "Desktop"    Keys "F12 1"
  Keys "ShiftF4 CtrlY Enter" TestArea "Editor"     Keys "Esc"
  Keys "F7"                  TestArea "Dialog"     Keys "Esc"
  Keys "Alt?"                TestArea "Search"     Keys "Esc"
  Keys "AltF1"               TestArea "Disks"      Keys "Esc"
  Keys "AltF2"               TestArea "Disks"      Keys "Esc"
  Keys "F9"                  TestArea "MainMenu"   Keys "Esc"
  Keys "F9 Enter"            TestArea "MainMenu"   Keys "Esc Esc"
  Keys "F12"                 TestArea "Menu"       Keys "Esc"
  Keys "F1"                  TestArea "Help"       Keys "Esc"
  Keys "CtrlL Tab"           TestArea "Info"       Keys "Tab CtrlL"
  Keys "CtrlQ Tab"           TestArea "QView"      Keys "Tab CtrlQ"
  if Far.GetConfig("Panel.Tree.TurnOffCompletely") ~= true then
    Keys "CtrlT Tab"         TestArea "Tree"       Keys "Tab CtrlT"
    Keys "AltF10"            TestArea "FindFolder" Keys "Esc"
  end
  Keys "F2"                  TestArea "UserMenu"   Keys "Esc"

  assert_eq    (Area.Current, "Shell")
  assert_false (Area.Other)
  assert_true  (Area.Shell)
  assert_false (Area.Viewer)
  assert_false (Area.Editor)
  assert_false (Area.Dialog)
  assert_false (Area.Search)
  assert_false (Area.Disks)
  assert_false (Area.MainMenu)
  assert_false (Area.Menu)
  assert_false (Area.Help)
  assert_false (Area.Info)
  assert_false (Area.QView)
  assert_false (Area.Tree)
  assert_false (Area.FindFolder)
  assert_false (Area.UserMenu)
  assert_false (Area.ShellAutoCompletion)
  assert_false (Area.DialogAutoCompletion)
  assert_false (Area.Grabber)
  assert_false (Area.Desktop)
end

local function test_mf_akey()
  assert_eq(akey, mf.akey)
  local k0,k1 = akey(0),akey(1)
  assert(k0==0x0501007B and k1=="CtrlShiftF12" or
         k0==0x1401007B and k1=="RCtrlShiftF12")
  -- (the 2nd parameter is tested in function test_mf_eval).
end

local function test_bit64()
  for _,name in ipairs{"band","bnot","bor","bxor","lshift","rshift"} do
    assert_eq   (_G[name], bit64[name])
    assert_func (bit64[name])
  end

  local a,b,c = 0xFF,0xFE,0xFD
  assert_eq(band(a,b,c,a,b,c), 0xFC)
  a,b,c = bit64.new(0xFF),bit64.new(0xFE),bit64.new(0xFD)
  assert_eq(band(a,b,c,a,b,c), 0xFC)

  a,b = bit64.new("0xFFFF0000FFFF0000"),bit64.new("0x0000FFFF0000FFFF")
  assert_eq(band(a,b), 0)
  assert_eq(bor(a,b), -1)
  assert_eq(a+b, -1)

  a,b,c = 1,2,4
  assert_eq(bor(a,b,c,a,b,c), 7)

  for k=-3,3 do assert_eq(bnot(k), -1-k) end
  assert_eq(bnot(bit64.new(5)), -6)

  assert_eq(bxor(0x01,0xF0,0xAA), 0x5B)
  assert_eq(lshift(0xF731,4),  0xF7310)
  assert_eq(rshift(0xF7310,4), 0xF731)

  local v = bit64.new(5)
  assert_true(v+2==7  and 2+v==7)
  assert_true(v-2==3  and 2-v==-3)
  assert_true(v*2==10 and 2*v==10)
  assert_true(v/2==2  and 2/v==0)
  assert_true(v%2==1  and 2%v==2)
  assert_true(v+v==10 and v-v==0 and v*v==25 and v/v==1 and v%v==0)

  local w = lshift(1,63)
  assert_eq(w, bit64.new("0x8000".."0000".."0000".."0000"))
  assert_eq(rshift(w,63), 1)
  assert_eq(rshift(w,64), 0)
  assert_eq(bit64.arshift(w,62), -2)
  assert_eq(bit64.arshift(w,63), -1)
  assert_eq(bit64.arshift(w,64), -1)
end

local function test_mf_eval()
  assert_eq(eval, mf.eval)

  -- test arguments validity checking
  assert_eq (eval(""), 0)
  assert_eq (eval("", 0), 0)
  assert_eq (eval(), -1)
  assert_eq (eval(0), -1)
  assert_eq (eval(true), -1)
  assert_eq (eval("", -1), -1)
  assert_eq (eval("", 5), -1)
  assert_eq (eval("", true), -1)
  assert_eq (eval("", 1, true), -1)
  assert_eq (eval("",1,"javascript"), -1)

  -- test macro-not-found error
  assert_eq (eval("", 2), -2)

  temp=3
  assert_eq (eval("temp=5+7"), 0)
  assert_eq (temp, 12)

  temp=3
  assert_eq (eval("temp=5+7",0,"moonscript"), 0)
  assert_eq (eval("temp=5+7",1,"lua"), 0)
  assert_eq (eval("temp=5+7",3,"lua"), "")
  assert_eq (eval("temp=5+7",1,"moonscript"), 0)
  assert_eq (eval("temp=5+7",3,"moonscript"), "")
  assert_eq (temp, 3)
  assert_eq (eval("getfenv(1).temp=12",0,"moonscript"), 0)
  assert_eq (temp, 12)

  assert_eq (eval("5",0,"moonscript"), 0)
  assert_eq (eval("5+7",1,"lua"), 11)
  assert_eq (eval("5+7",1,"moonscript"), 0)
  assert_eq (eval("5 7",1,"moonscript"), 11)

  -- test with Mode==2
  local Id = assert_udata(far.MacroAdd(nil,nil,"CtrlA",[[
    local key = akey(1,0)
    assert(key=="CtrlShiftF12" or key=="RCtrlShiftF12")
    assert(akey(1,1)=="CtrlA")
    foobar = (foobar or 0) + 1
    return foobar,false,5,nil,"foo"
  ]]))
  for k=1,3 do
    local ret1,a,b,c,d,e = eval("CtrlA",2)
    assert_true(ret1==0 and a==k and b==false and c==5 and d==nil and e=="foo")
  end
  assert_true(far.MacroDelete(Id))
end

local function test_mf_abs()
  assert_eq (mf.abs(1.3), 1.3)
  assert_eq (mf.abs(-1.3), 1.3)
  assert_eq (mf.abs(0), 0)
end

local function test_mf_acall()
  local a,b,c,d = mf.acall(function(p) return 3, nil, p, "foo" end, 77)
  assert_true (a==3 and b==nil and c==77 and d=="foo")
  assert_true (mf.acall(far.Show))
  Keys"Esc"
end

local function test_mf_asc()
  assert_eq (mf.asc("0"), 48)
  assert_eq (mf.asc("Я"), 1071)
end

local function test_mf_atoi()
  assert_eq (mf.atoi("0"), 0)
  assert_eq (mf.atoi("-10"), -10)
  assert_eq (mf.atoi("0x11"), 17)
  assert_eq (mf.atoi("1011",2), 11)
  assert_eq (mf.atoi("123456789123456789"),  bit64.new("123456789123456789"))
  assert_eq (mf.atoi("-123456789123456789"), bit64.new("-123456789123456789"))
  assert_eq (mf.atoi("0x1B69B4BACD05F15"),   bit64.new("0x1B69B4BACD05F15"))
  assert_eq (mf.atoi("-0x1B69B4BACD05F15"),  bit64.new("-0x1B69B4BACD05F15"))
end

local function test_mf_chr()
  assert_eq (mf.chr(48), "0")
  assert_eq (mf.chr(1071), "Я")
end

local function test_mf_clip()
  local oldval = far.PasteFromClipboard() -- store

  mf.clip(5,2) -- turn on the internal clipboard
  assert_eq (mf.clip(5,-1), 2)
  assert_eq (mf.clip(5,1),  2) -- turn on the OS clipboard
  assert_eq (mf.clip(5,-1), 1)

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
  assert_eq (mf.clip(0), "bar")
  mf.clip(5,1); assert_eq (mf.clip(0), "foo")
  mf.clip(5,2); assert_eq (mf.clip(0), "bar")

  mf.clip(3);   assert_eq (mf.clip(0), "foo")
  mf.clip(5,1); assert_eq (mf.clip(0), "foo")

  mf.clip(5,2); mf.clip(1,"bar")
  mf.clip(5,1); assert_eq (mf.clip(0), "foo")
  mf.clip(4);   assert_eq (mf.clip(0), "bar")
  mf.clip(5,2); assert_eq (mf.clip(0), "bar")

  mf.clip(5,1) -- leave the OS clipboard active in the end
  far.CopyToClipboard(oldval or "") -- restore
end

local function test_mf_env()
  mf.env("Foo",1,"Bar")
  assert_eq (mf.env("Foo"), "Bar")
  mf.env("Foo",1,"")
  assert_eq (mf.env("Foo"), "")
end

local function test_mf_fattr()
  DeleteTmpFile()
  assert_eq (mf.fattr(TmpFileName), -1)
  WriteTmpFile("")
  local attr = mf.fattr(TmpFileName)
  DeleteTmpFile()
  assert(attr >= 0)
end

local function test_mf_fexist()
  WriteTmpFile("")
  assert_true(mf.fexist(TmpFileName))
  DeleteTmpFile()
  assert_false(mf.fexist(TmpFileName))
end

local function test_mf_msgbox()
  assert_eq (msgbox, mf.msgbox)
  mf.postmacro(function() Keys("Esc") end)
  assert_eq (0, msgbox("title","message"))
  mf.postmacro(function() Keys("Enter") end)
  assert_eq (1, msgbox("title","message"))
end

local function test_mf_prompt()
  assert_eq (prompt, mf.prompt)
  mf.postmacro(function() Keys("a b c Esc") end)
  assert_false (prompt())
  mf.postmacro(function() Keys("a b c Enter") end)
  assert_eq ("abc", prompt())
end

local function test_mf_date()
  assert_str (mf.date())
  assert_str (mf.date("%a"))
end

local function test_mf_fmatch()
  assert_eq (mf.fmatch("Readme.txt", "*.txt"), 1)
  assert_eq (mf.fmatch("Readme.txt", "Readme.*|*.txt"), 0)
  assert_eq (mf.fmatch("c:\\Readme.txt", "/txt$/i"), 1)
  assert_eq (mf.fmatch("c:\\Readme.txt", "/txt$"), -1)
end

local function test_mf_fsplit()
  local path="C:\\Program Files\\Far\\Far.exe"
  assert_eq (mf.fsplit(path,0x01), "C:")
  assert_eq (mf.fsplit(path,0x02), "\\Program Files\\Far\\")
  assert_eq (mf.fsplit(path,0x04), "Far")
  assert_eq (mf.fsplit(path,0x08), ".exe")

  assert_eq (mf.fsplit(path,0x03), "C:\\Program Files\\Far\\")
  assert_eq (mf.fsplit(path,0x0C), "Far.exe")
  assert_eq (mf.fsplit(path,0x0F), path)
end

local function test_mf_iif()
  assert_eq (mf.iif(true,  1, 2), 1)
  assert_eq (mf.iif("a",   1, 2), 1)
  assert_eq (mf.iif(100,   1, 2), 1)
  assert_eq (mf.iif(false, 1, 2), 2)
  assert_eq (mf.iif(nil,   1, 2), 2)
  assert_eq (mf.iif(0,     1, 2), 2)
  assert_eq (mf.iif("",    1, 2), 2)
end

local function test_mf_index()
  assert_eq (mf.index("language","gua",0), 3)
  assert_eq (mf.index("language","gua",1), 3)
  assert_eq (mf.index("language","gUA",1), -1)
  assert_eq (mf.index("language","gUA",0), 3)
end

local function test_mf_int()
  assert_eq (mf.int("2.99"), 2)
  assert_eq (mf.int("-2.99"), -2)
  assert_eq (mf.int("0x10"), 0)
  assert_eq (mf.int("123456789123456789"), bit64.new("123456789123456789"))
  assert_eq (mf.int("-123456789123456789"), bit64.new("-123456789123456789"))
end

local function test_mf_itoa()
  assert_eq (mf.itoa(100), "100")
  assert_eq (mf.itoa(100,10), "100")
  assert_eq (mf.itoa(bit64.new("123456789123456789")), "123456789123456789")
  assert_eq (mf.itoa(bit64.new("-123456789123456789")), "-123456789123456789")
  assert_eq (mf.itoa(100,2), "1100100")
  assert_eq (mf.itoa(100,16), "64")
  assert_eq (mf.itoa(100,36), "2s")
end

local function test_mf_key()
  assert_eq (mf.key(0x01000000), "Ctrl")
  assert_eq (mf.key(0x02000000), "Alt")
  assert_eq (mf.key(0x04000000), "Shift")
  assert_eq (mf.key(0x10000000), "RCtrl")
  assert_eq (mf.key(0x20000000), "RAlt")

  assert_eq (mf.key(0x0501007B), "CtrlShiftF12")
  assert_eq (mf.key("CtrlShiftF12"), "CtrlShiftF12")

  assert_eq (mf.key("foobar"), "")
end

-- Separate tests for mf.float and mf.string are locale-dependant, thus they are tested together.
local function test_mf_float_and_string()
  local t = { 0, -0, 2.56e1, -5.37, -2.2e100, 2.2e-100 }
  for _,num in ipairs(t) do
    assert_eq (mf.float(mf.string(num)), num)
  end
end

local function test_mf_lcase()
  assert_eq (mf.lcase("FOo БАр"), "foo бар")
end

local function test_mf_len()
  assert_eq (mf.len(""), 0)
  assert_eq (mf.len("FOo БАр"), 7)
end

local function test_mf_max()
  assert_eq (mf.max(-2,-5), -2)
  assert_eq (mf.max(2,5), 5)
end

local function test_mf_min()
  assert_eq (mf.min(-2,-5), -5)
  assert_eq (mf.min(2,5), 2)
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
  assert_eq (mf.mload(Key, "name1"), v1)
  assert_eq (mf.mload(Key, "name2"), v2)
  assert_eq (mf.mload(Key, "name3"), v3)
  assert_eq (mf.mload(Key, "name4"), v4)
  assert_eq (mf.mload(Key, "name5"), v5)
  assert_eq (mf.mload(Key, "name6"), v6)
  mf.mdelete(Key, "*")
  assert_eq (mf.mload(Key, "name3"), nil)

  -- test tables
  mf.msave(Key, "name1", { a=5, {b="foo"}, c={d=false} })
  local t=mf.mload(Key, "name1")
  assert_true(t.a==5 and t[1].b=="foo" and t.c.d==false)
  mf.mdelete(Key, "name1")
  assert_nil(mf.mload(Key, "name1"))

  -- test tables more
  local t1, t2, t3 = {5}, {6}, {}
  t1[2], t1[3], t1[4], t1[5] = t1, t2, t3, t3
  t2[2], t2[3] = t1, t2
  t1[t1], t1[t2] = 66, 77
  t2[t1], t2[t2] = 88, 99
  setmetatable(t3, { __index=t1 })
  mf.msave(Key, "name1", t1)

  local T1 = assert_table(mf.mload(Key, "name1"))
  local T2 = assert_table(T1[3])
  local T3 = T1[4]
  assert(type(T3)=="table" and T3==T1[5])
  assert(T1[1]==5 and T1[2]==T1 and T1[3]==T2)
  assert(T2[1]==6 and T2[2]==T1 and T2[3]==T2)
  assert(T1[T1]==66 and T1[T2]==77)
  assert(T2[T1]==88 and T2[T2]==99)
  assert(getmetatable(T3).__index==T1 and T3[1]==5 and rawget(T3,1)==nil)
  mf.mdelete(Key, "*")
  assert_nil(mf.mload(Key, "name1"))

  -- test locations (profiles)
  if win.GetEnv("FARPROFILE") ~= win.GetEnv("FARLOCALPROFILE") then
    mf.msave("key1", "name1", 100)
    mf.msave("key2", "name2", 200, "roaming")
    mf.msave("key1", "name1", 300, "local")
    for k=1,2 do
      assert_eq(mf.mload("key1", "name1"), 100)
      assert_eq(mf.mload("key1", "name1", "roaming"), 100)
      assert_eq(mf.mload("key2", "name2"), 200)
      assert_eq(mf.mload("key2", "name2", "roaming"), 200)
      assert_eq(mf.mload("key1", "name1", "local"), 300)
    end
    mf.mdelete("key1", "name1")
    mf.mdelete("key2", "name2", "roaming")
    assert_nil(mf.mload("key1", "name1"))
    assert_nil(mf.mload("key2", "name2"))
    assert_eq(mf.mload("key1", "name1", "local"), 300)
    mf.mdelete("key1", "name1", "local")
    assert_nil(mf.mload("key1", "name1", "local"))
  end
end

local function test_mf_mod()
  assert_eq (mf.mod(11,4), 3)
  assert_eq (math.fmod(11,4), 3)
  assert_eq (11 % 4, 3)

  assert_eq (mf.mod(-1,4), -1)
  assert_eq (math.fmod(-1,4), -1)
  assert_eq (-1 % 4, 3)
end

local function test_mf_replace()
  assert_eq (mf.replace("Foo Бар", "o", "1"), "F11 Бар")
  assert_eq (mf.replace("Foo Бар", "o", "1", 1), "F1o Бар")
  assert_eq (mf.replace("Foo Бар", "O", "1", 1, 1), "Foo Бар")
  assert_eq (mf.replace("Foo Бар", "O", "1", 1, 0), "F1o Бар")
end

local function test_mf_rindex()
  assert_eq (mf.rindex("language","a",0), 5)
  assert_eq (mf.rindex("language","a",1), 5)
  assert_eq (mf.rindex("language","A",1), -1)
  assert_eq (mf.rindex("language","A",0), 5)
end

local function test_mf_strpad()
  assert_eq (mf.strpad("Foo",10,"*",  2), '***Foo****')
  assert_eq (mf.strpad("",   10,"-*-",2), '-*--*--*--')
  assert_eq (mf.strpad("",   10,"-*-"), '-*--*--*--')
  assert_eq (mf.strpad("Foo",10), 'Foo       ')
  assert_eq (mf.strpad("Foo",10,"-"), 'Foo-------')
  assert_eq (mf.strpad("Foo",10," ",  1), '       Foo')
  assert_eq (mf.strpad("Foo",10," ",  2), '   Foo    ')
  assert_eq (mf.strpad("Foo",10,"1234567890",2), '123Foo1234')
end

local function test_mf_strwrap()
  assert_eq (mf.strwrap("Пример строки, которая будет разбита на несколько строк по ширине в 7 символов.", 7,"\n"),
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
  assert_eq (mf.substr("abcdef", 1), "bcdef")
  assert_eq (mf.substr("abcdef", 1, 3), "bcd")
  assert_eq (mf.substr("abcdef", 0, 4), "abcd")
  assert_eq (mf.substr("abcdef", 0, 8), "abcdef")
  assert_eq (mf.substr("abcdef", -1), "f")
  assert_eq (mf.substr("abcdef", -2), "ef")
  assert_eq (mf.substr("abcdef", -3, 1), "d")
  assert_eq (mf.substr("abcdef", 0, -1), "abcde")
  assert_eq (mf.substr("abcdef", 2, -1), "cde")
  assert_eq (mf.substr("abcdef", 4, -4), "")
  assert_eq (mf.substr("abcdef", -3, -1), "de")
end

local function test_mf_testfolder()
  assert(mf.testfolder(".") > 0)
  assert(mf.testfolder("C:\\") == 2)
  assert(mf.testfolder("@:\\") <= 0)
end

local function test_mf_trim()
  assert_eq (mf.trim(" abc "), "abc")
  assert_eq (mf.trim(" abc ",0), "abc")
  assert_eq (mf.trim(" abc ",1), "abc ")
  assert_eq (mf.trim(" abc ",2), " abc")
end

local function test_mf_ucase()
  assert_eq (mf.ucase("FOo БАр"), "FOO БАР")
end

local function test_mf_waitkey()
  assert_eq (mf.waitkey(50,0), "")
  assert_eq (mf.waitkey(50,1), 0xFFFFFFFF)
end

local function test_mf_size2str()
  local F_COMMAS         = bit64.new("0x0800000000000000")
  local F_FLOATSIZE      = bit64.new("0x0080000000000000")
  local F_SHOWBYTESINDEX = bit64.new("0x0010000000000000")
  local F_ECONOMIC       = bit64.new("0x0040000000000000")
  local F_THOUSAND       = bit64.new("0x0400000000000000")
  local F_MINSIZEINDEX   = bit64.new("0x0020000000000000")

  assert_eq(mf.size2str(123456, 0), "123456")
  assert_eq(mf.size2str(123456, 0, 8), "  123456")
  assert_eq(mf.size2str(123456, 0, -8), "123456  ")
  assert_eq(mf.size2str(123456, F_COMMAS), "123,456")
  assert_eq(mf.size2str(123456, F_SHOWBYTESINDEX), "123456 B")
  assert_eq(mf.size2str(123456, F_FLOATSIZE), "121 K")
  assert_eq(mf.size2str(123456, F_FLOATSIZE+F_ECONOMIC), "121K")
  assert_eq(mf.size2str(123456, F_FLOATSIZE+F_THOUSAND), "123 k")
  assert_eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x3), "4 T")
  assert_eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x2), "4112 G")
  assert_eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x1), "4210688 M")
  assert_eq(mf.size2str(2^42+2^34, F_SHOWBYTESINDEX+F_MINSIZEINDEX+0x0), "4311744512 K")
end

local function test_mf_xlat()
  assert_str (mf.xlat("abc"))
  -- commented out, as these tests won't work with any Windows configuration:
  --assert(mf.xlat("ghzybr")=="пряник")
  --assert(mf.xlat("сщьзгеук")=="computer")
end

local function test_mf_beep()
  assert_bool (mf.beep())
end

local function test_mf_flock()
  for k=0,2 do assert_num (mf.flock(k,-1)) end
end

local function test_mf_GetMacroCopy()
  assert_func (mf.GetMacroCopy)
end

local function test_mf_Keys()
  assert_eq (Keys, mf.Keys)
  assert_func (Keys)

  Keys("Esc F a r Space M a n a g e r Space Ф А Р")
  assert_eq (panel.GetCmdLine(), "Far Manager ФАР")
  Keys("Esc")
  assert_eq (panel.GetCmdLine(), "")
end

local function test_mf_exit()
  assert_eq (exit, mf.exit)
  local N
  mf.postmacro(
    function()
      local function f() N=50; exit(); end
      f(); N=100
    end)
  mf.postmacro(function() Keys"Esc" end)
  far.Message("dummy")
  assert_eq (N, 50)
end

local function test_mf_mmode()
  assert_eq (mmode, mf.mmode)
  assert_eq (1, mmode(1,-1))
end

local function test_mf_print()
  assert_eq (print, mf.print)
  assert_func (print)
  -- test on command line
  local str = "abc ABC абв АБВ"
  Keys("Esc")
  print(str)
  assert_eq (panel.GetCmdLine(), str)
  Keys("Esc")
  assert_eq (panel.GetCmdLine(), "")
  -- test on dialog input field
  Keys("F7 CtrlY")
  print(str)
  assert_eq (Dlg.GetValue(-1,0), str)
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
  assert_func (mf.postmacro)
end

local function test_mf_sleep()
  assert_func (mf.sleep)
end

local function test_mf_usermenu()
  assert_func (mf.usermenu)
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
  assert_false(CmdLine.Bof)
  assert_true(CmdLine.Eof)
  assert_false(CmdLine.Empty)
  assert_false(CmdLine.Selected)
  assert_eq (CmdLine.Value, "foo Бар")
  assert_eq (CmdLine.ItemCount, 7)
  assert_eq (CmdLine.CurPos, 8)

  Keys"SelWord"
  assert_true(CmdLine.Selected)

  Keys"CtrlHome"
  assert_true(CmdLine.Bof)
  assert_false(CmdLine.Eof)

  Keys"Esc"
  assert_true(CmdLine.Bof)
  assert_true(CmdLine.Eof)
  assert_true(CmdLine.Empty)
  assert_false(CmdLine.Selected)
  assert_eq (CmdLine.Value, "")
  assert_eq (CmdLine.ItemCount, 0)
  assert_eq (CmdLine.CurPos, 1)

  Keys"Esc"
  print("foo Бар")
  assert_eq (CmdLine.Value, "foo Бар")

  Keys"Esc"
  print(("%s %d %s"):format("foo", 5+7, "Бар"))
  assert_eq (CmdLine.Value, "foo 12 Бар")

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
  assert_bool (Far.FullScreen)
  assert_num (Far.Height)
  assert_bool (Far.IsUserAdmin)
  assert_num (Far.PID)
  assert_str (Far.Title)
  assert_num (Far.Width)

  local temp = Far.UpTime
  mf.sleep(50)
  temp = Far.UpTime - temp
  assert(temp > 40 and temp < 80, temp)

  assert_num (Far.GetConfig("Editor.defaultcodepage"))
  assert_func (Far.Cfg_Get)
  assert_func (Far.DisableHistory)
  assert_num (Far.KbdLayout(0))
  assert_num (Far.KeyBar_Show(0))
  assert_func (Far.Window_Scroll)

  -- test_Far_GetConfig()
end

local function test_CheckAndGetHotKey()
  mf.acall(far.Menu, {Flags="FMENU_AUTOHIGHLIGHT"},
    {{text="abcd"},{text="abc&d"},{text="abcd"},{text="abcd"},{text="abcd"}})

  assert_eq (Object.CheckHotkey("a"), 1)
  assert_eq (Object.GetHotkey(1), "a")
  assert_eq (Object.GetHotkey(), "a")
  assert_eq (Object.GetHotkey(0), "a")

  assert_eq (Object.CheckHotkey("b"), 3)
  assert_eq (Object.GetHotkey(3), "b")

  assert_eq (Object.CheckHotkey("c"), 4)
  assert_eq (Object.GetHotkey(4), "c")

  assert_eq (Object.CheckHotkey("d"), 2)
  assert_eq (Object.GetHotkey(2), "d")

  assert_eq (Object.CheckHotkey("e"), 0)

  assert_eq (Object.CheckHotkey(""), 5)
  assert_eq (Object.GetHotkey(5), "")
  assert_eq (Object.GetHotkey(6), "")

  Keys("Esc")
end

function MT.test_Menu()
  Keys("F11")
  assert_str(Menu.Value)
  assert_eq(Menu.Id, far.Guids.PluginsMenuId)
  assert_eq(Menu.Id, "937F0B1C-7690-4F85-8469-AA935517F202")
  Keys("Esc")

  assert_func(Menu.Filter)
  assert_func(Menu.FilterStr)
  assert_func(Menu.GetValue)
  assert_func(Menu.ItemStatus)
  assert_func(Menu.Select)
  assert_func(Menu.Show)
end

function MT.test_Object()
  assert_bool (Object.Bof)
  assert_num (Object.CurPos)
  assert_bool (Object.Empty)
  assert_bool (Object.Eof)
  assert_num (Object.Height)
  assert_num (Object.ItemCount)
  assert_bool (Object.Selected)
  assert_str (Object.Title)
  assert_num (Object.Width)

  test_CheckAndGetHotKey()
end

function MT.test_Drv()
  Keys"AltF1"
  assert_num (Drv.ShowMode)
  assert_eq (Drv.ShowPos, 1)
  Keys"Esc AltF2"
  assert_num (Drv.ShowMode)
  assert_eq (Drv.ShowPos, 2)
  Keys"Esc"
end

function MT.test_Help()
  Keys"F1"
  assert_str (Help.FileName)
  assert_str (Help.SelTopic)
  assert_str (Help.Topic)
  Keys"Esc"
end

function MT.test_Mouse()
  assert_num (Mouse.X)
  assert_num (Mouse.Y)
  assert_num (Mouse.Button)
  assert_num (Mouse.CtrlState)
  assert_num (Mouse.EventFlags)
  assert_num (Mouse.LastCtrlState)
end

function MT.test_XPanel(pan) -- (@pan: either APanel or PPanel)
  assert_bool (pan.Bof)
  assert_num  (pan.ColumnCount)
  assert_num  (pan.CurPos)
  assert_str  (pan.Current)
  assert_num  (pan.DriveType)
  assert_bool (pan.Empty)
  assert_bool (pan.Eof)
  assert_bool (pan.FilePanel)
  assert_bool (pan.Filter)
  assert_bool (pan.Folder)
  assert_str  (pan.Format)
  assert_num  (pan.Height)
  assert_str  (pan.HostFile)
  assert_num  (pan.ItemCount)
  assert_bool (pan.Left)
  assert_bool (pan.LFN)
  assert_num  (pan.OPIFlags)
  assert_str  (pan.Path)
  assert_str  (pan.Path0)
  assert_bool (pan.Plugin)
  assert_str  (pan.Prefix)
  assert_bool (pan.Root)
  assert_num  (pan.SelCount)
  assert_bool (pan.Selected)
  assert_num  (pan.Type)
  assert_str  (pan.UNCPath)
  assert_bool (pan.Visible)
  assert_num  (pan.Width)

  if pan == APanel then
    Keys "End"  assert_true(pan.Eof)
    Keys "Home" assert_true(pan.Bof)
  end
end

local function test_Panel_Item()
  for pt=0,1 do
    assert_str (Panel.Item(pt,0,0))
    assert_str (Panel.Item(pt,0,1))
    assert_num (Panel.Item(pt,0,2))
    assert_str (Panel.Item(pt,0,3))
    assert_str (Panel.Item(pt,0,4))
    assert_str (Panel.Item(pt,0,5))
    assert(IsNumOrInt(Panel.Item(pt,0,6)))
    assert(IsNumOrInt(Panel.Item(pt,0,7)))
    assert_bool (Panel.Item(pt,0,8))
    assert_num (Panel.Item(pt,0,9))
    assert_num (Panel.Item(pt,0,10))
    assert_str (Panel.Item(pt,0,11))
    assert_str (Panel.Item(pt,0,12))
    assert_num (Panel.Item(pt,0,13))
    assert_num (Panel.Item(pt,0,14))
    assert(IsNumOrInt(Panel.Item(pt,0,15)))
    assert(IsNumOrInt(Panel.Item(pt,0,16)))
    assert(IsNumOrInt(Panel.Item(pt,0,17)))
    assert_num (Panel.Item(pt,0,18))
    assert(IsNumOrInt(Panel.Item(pt,0,19)))
    assert_str (Panel.Item(pt,0,20))
    assert(IsNumOrInt(Panel.Item(pt,0,21)))
    assert(not pcall(Panel.Item,pt,0,22))
    assert_num (Panel.Item(pt,0,23))
  end
end

local function test_Panel_SetPath()
  -- store
  local adir_old = panel.GetPanelDirectory(nil,1).Name
  local pdir_old = panel.GetPanelDirectory(nil,0).Name
  --test
  local pdir = "c:\\windows"
  local adir = "c:\\windows\\system32"
  local afile = "cmd.exe"
  assert_true(Panel.SetPath(1, pdir))
  assert_true(Panel.SetPath(0, adir, afile))
  assert_eq (pdir, panel.GetPanelDirectory(nil,0).Name:lower())
  assert_eq (adir, panel.GetPanelDirectory(nil,1).Name:lower())
  assert_eq (panel.GetCurrentPanelItem(nil,1).FileName:lower(), afile)
  -- restore
  assert_true(Panel.SetPath(1, pdir_old))
  assert_true(Panel.SetPath(0, adir_old))
end

-- N=Panel.Select(panelType,Action[,Mode[,Items]])
local function Test_Panel_Select()
  local adir_old = panel.GetPanelDirectory(nil,1) -- store active panel directory

  local PS = assert_func(Panel.Select)
  local RM,ADD,INV,RST = 0,1,2,3 -- Action
  local MODE

  local dir = assert_str(win.GetEnv("FARHOME"))
  assert_true(panel.SetPanelDirectory(nil,1,dir))
  local pi = assert_table(panel.GetPanelInfo(1))
  local ItemsCount = assert_num(pi.ItemsNumber)-1 -- don't count ".."
  assert(ItemsCount>=10, "not enough files to test")

  --------------------------------------------------------------
  MODE = 0
  assert_eq(ItemsCount,PS(0,ADD,MODE)) -- select all
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(ItemsCount, pi.SelectedItemsNumber)

  assert_eq(ItemsCount,PS(0,RM,MODE)) -- clear all
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  assert_eq(ItemsCount,PS(0,INV,MODE)) -- invert
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(ItemsCount, pi.SelectedItemsNumber)

  assert_eq(0,PS(0,INV,MODE)) -- invert again (return value is the selection count, contrary to docs)
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 1
  assert_eq(1,PS(0,ADD,MODE,5))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(1, pi.SelectedItemsNumber)

  assert_eq(1,PS(0,RM,MODE,5))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  assert_eq(1,PS(0,INV,MODE,5))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(1, pi.SelectedItemsNumber)

  assert_eq(1,PS(0,INV,MODE,5))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 2
  local list = win.JoinPath(dir,"FarEng.hlf").."\nFarEng.lng" -- the 1-st file with path, the 2-nd without
  assert_eq(2,PS(0,ADD,MODE,list))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(2, pi.SelectedItemsNumber)

  assert_eq(2,PS(0,RM,MODE,list))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  assert_eq(2,PS(0,INV,MODE,list))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(2, pi.SelectedItemsNumber)

  assert_eq(2,PS(0,INV,MODE,list))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 3
  local mask = "*.hlf;*.lng"
  local count = 0
  for i=1,pi.ItemsNumber do
    local item = assert_table(panel.GetPanelItem(nil,1,i))
    if far.CmpNameList(mask, item.FileName) then count=count+1 end
  end
  assert(count>1, "not enough files to test")

  assert_eq(count,PS(0,ADD,MODE,mask))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(count, pi.SelectedItemsNumber)

  assert_eq(count,PS(0,RM,MODE,mask))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  assert_eq(count,PS(0,INV,MODE,mask))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(count, pi.SelectedItemsNumber)

  assert_eq(count,PS(0,INV,MODE,mask))
  pi = assert_table(panel.GetPanelInfo(1))
  assert_eq(0, pi.SelectedItemsNumber)

  panel.SetPanelDirectory(nil,1,adir_old) -- restore active panel directory
end

function MT.test_Panel()
  test_Panel_Item()

  assert_eq (Panel.FAttr(0,":"), -1)
  assert_eq (Panel.FAttr(1,":"), -1)

  assert_eq (Panel.FExist(0,":"), 0)
  assert_eq (Panel.FExist(1,":"), 0)

  Test_Panel_Select()
  test_Panel_SetPath()
  assert_func (Panel.SetPos)
  assert_func (Panel.SetPosIdx)
end

function MT.test_Dlg()
  Keys"F7 a b c"
  assert_true(Area.Dialog)
  assert_eq(Dlg.Id, "FAD00DBE-3FFF-4095-9232-E1CC70C67737")
  assert_eq(Dlg.Owner, "00000000-0000-0000-0000-000000000000")
  assert(Dlg.ItemCount > 6)
  assert_eq(Dlg.ItemType, 4)
  assert_eq(Dlg.CurPos, 3)
  assert_eq(Dlg.PrevPos, 0)

  Keys"Tab"
  local pos = Dlg.CurPos
  assert(Dlg.CurPos > 3)
  assert_eq(Dlg.PrevPos, 3)
  assert_eq(pos, Dlg.SetFocus(3))
  assert_eq(pos, Dlg.PrevPos)

  assert_eq(Dlg.GetValue(0,0), Dlg.ItemCount)
  Keys"Esc"
end

function MT.test_Plugin()
  assert_false(Plugin.Menu())
  assert_false(Plugin.Config())
  assert_false(Plugin.Command())
  assert_true(Plugin.Command(luamacroId))

  assert_true(Plugin.Exist(luamacroId))
  assert_false(Plugin.Exist(luamacroId:gsub("^...","000")))

  local function test (func, N) -- Plugin.Call, Plugin.SyncCall: test arguments and returns
    local i1 = bit64.new("0x8765876587658765")
    local r1,r2,r3,r4,r5 = func(luamacroId, "argtest", "foo", i1, -2.34, false, {"foo\0bar"})
    assert(r1=="foo" and r2==i1 and r3==-2.34 and r4==false and type(r5)=="table" and r5[1]=="foo\0bar")

    local src = {}
    for k=1,N do src[k]=k end
    local trg = { func(luamacroId, "argtest", unpack(src)) }
    assert(#trg==N and trg[1]==1 and trg[N]==N)
  end
  test(Plugin.Call, 8000-8)
  test(Plugin.SyncCall, 8000-8)
end

local function test_far_MacroExecute()
  local function test(code, flags)
    local t = far.MacroExecute(code, flags,
      "foo",
      false,
      5,
      nil,
      bit64.new("0x8765876587658765"),
      {"bar"})
    assert_table (t)
    assert_eq    (t.n,  6)
    assert_eq    (t[1], "foo")
    assert_false (t[2])
    assert_eq    (t[3], 5)
    assert_nil   (t[4])
    assert_eq    (t[5], bit64.new("0x8765876587658765"))
    assert_eq    (assert_table(t[6])[1], "bar")
  end
  test("return ...", nil)
  test("return ...", "KMFLAGS_LUA")
  test("...", "KMFLAGS_MOONSCRIPT")
end

local function test_far_MacroAdd()
  local area, key, descr = "MACROAREA_SHELL", "CtrlA", "Test MacroAdd"

  local Id = far.MacroAdd(area, nil, key, [[A = { b=5 }]], descr)
  assert_true(far.MacroDelete(assert_udata(Id)))

  Id = far.MacroAdd(-1, nil, key, [[A = { b=5 }]], descr)
  assert_nil(Id) -- bad area

  Id = far.MacroAdd(area, nil, key, [[A = { b:5 }]], descr)
  assert_nil(Id) -- bad code

  Id = far.MacroAdd(area, "KMFLAGS_MOONSCRIPT", key, [[A = { b:5 }]], descr)
  assert_true(far.MacroDelete(assert_udata(Id)))

  Id = far.MacroAdd(area, "KMFLAGS_MOONSCRIPT", key, [[A = { b=5 }]], descr)
  assert_nil(Id) -- bad code

  Id = far.MacroAdd(area, nil, key, [[@c:\myscript 5+6,"foo"]], descr)
  assert_true(far.MacroDelete(assert_udata(Id)))

  Id = far.MacroAdd(area, nil, key, [[@c:\myscript 5***6,"foo"]], descr)
  assert_true(far.MacroDelete(assert_udata(Id))) -- with @ there is no syntax check till the macro runs

  Id = far.MacroAdd(nil, nil, key, [[@c:\myscript]])
  assert_true(far.MacroDelete(assert_udata(Id))) -- check default area (MACROAREA_COMMON)

  Id = far.MacroAdd(area,nil,key,[[Keys"F7" assert(Dlg.Id=="FAD00DBE-3FFF-4095-9232-E1CC70C67737") Keys"Esc"]],descr)
  assert_eq(0, mf.eval("Shell/"..key, 2))
  assert_true(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) return false end)
  assert_eq(-2, mf.eval("Shell/"..key, 2))
  assert_true(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) error"deliberate error" end)
  assert_eq(-2, mf.eval("Shell/"..key, 2))
  assert_true(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) return id==Id end)
  assert_eq(0, mf.eval("Shell/"..key, 2))
  assert_true(far.MacroDelete(Id))

end

local function test_far_MacroCheck()
  assert_true(far.MacroCheck([[A = { b=5 }]]))
  assert_true(far.MacroCheck([[A = { b=5 }]], "KMFLAGS_LUA"))

  assert_false(far.MacroCheck([[A = { b:5 }]], "KMFLAGS_SILENTCHECK"))

  assert_true(far.MacroCheck([[A = { b:5 }]], "KMFLAGS_MOONSCRIPT"))

  assert_false(far.MacroCheck([[A = { b=5 }]], {KMFLAGS_MOONSCRIPT=1,KMFLAGS_SILENTCHECK=1} ))

  WriteTmpFile [[A = { b=5 }]] -- valid Lua, invalid MoonScript
  assert_true (far.MacroCheck("@"..TmpFileName, "KMFLAGS_LUA"))
  assert_true (far.MacroCheck("@"..TmpFileName.." 5+6,'foo'", "KMFLAGS_LUA")) -- valid file arguments
  assert_false(far.MacroCheck("@"..TmpFileName.." 5***6,'foo'", "KMFLAGS_SILENTCHECK")) -- invalid file arguments
  assert_false(far.MacroCheck("@"..TmpFileName, {KMFLAGS_MOONSCRIPT=1,KMFLAGS_SILENTCHECK=1}))

  WriteTmpFile [[A = { b:5 }]] -- invalid Lua, valid MoonScript
  assert_false (far.MacroCheck("@"..TmpFileName, "KMFLAGS_SILENTCHECK"))
  assert_true  (far.MacroCheck("@"..TmpFileName, "KMFLAGS_MOONSCRIPT"))
  DeleteTmpFile()

  assert_false (far.MacroCheck([[@//////]], "KMFLAGS_SILENTCHECK"))
end

local function test_far_MacroGetArea()
  assert_eq(far.MacroGetArea(), F.MACROAREA_SHELL)
end

local function test_far_MacroGetLastError()
  assert_true  (far.MacroCheck("a=1"))
  assert_eq    (far.MacroGetLastError().ErrSrc, "")
  assert_false (far.MacroCheck("a=", "KMFLAGS_SILENTCHECK"))
  assert       (far.MacroGetLastError().ErrSrc:len() > 0)
end

local function test_far_MacroGetState()
  local st = far.MacroGetState()
  assert(st==F.MACROSTATE_EXECUTING or st==F.MACROSTATE_EXECUTING_COMMON)
end

local function test_MacroControl()
  test_far_MacroAdd()
  test_far_MacroCheck()
  test_far_MacroExecute()
  test_far_MacroGetArea()
  test_far_MacroGetLastError()
  test_far_MacroGetState()
end

local function test_RegexControl()
  local L = win.Utf8ToUtf16
  local pat = "([bc]+)"
  local pat2 = "([bc]+)|(zz)"
  local rep = "%1%1"
  local R = regex.new(pat)
  local R2 = regex.new(pat2)

  local fr,to,cap
  local str, nfound, nrep

  assert_eq(R:bracketscount(), 2)

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
  local fr,to,c1,c2,c3
  fr,to,c1 = regex.find("{}", "\\{(.)?\\}")
  assert(fr==1 and to==2 and c1==false)
  fr,to,c1,c2,c3 = regex.find("bbb", "(b)?b(b)?(b)?b")
  assert(fr==1 and to==3 and c1=="b" and c2==false and c3==false)

  -- Mantis 1388 (https://bugs.farmanager.com/view.php?id=1388)
  c1,c2 = regex.match("123", "(\\d+)A|(\\d+)")
  assert(c1==false and c2=="123")

  -- Issue #609 (https://github.com/FarGroup/FarManager/issues/609)
  c1 = regex.match("88", "(8)+")
  assert_eq(c1, "8")
end

--[[------------------------------------------------------------------------------------------------
0001722: DN_EDITCHANGE приходит лишний раз и с ложной информацией

Description:
  [ Far 2.0.1807, Far 3.0.1897 ]
  Допустим диалог состоит из единственного элемента DI_EDIT, больше элементов нет. При появлении
  диалога сразу нажмём на клавишу, допустим, W. Приходят два события DN_EDITCHANGE вместо одного,
  причём в первом из них PtrData указывает на пустую строку.

  Последующие нажатия на клавиши, вызывающие изменения текста, отрабатываются правильно, лишние
  ложные события не приходят.
--]]------------------------------------------------------------------------------------------------
function MT.test_mantis_1722()
  local check = 0
  local function DlgProc (hDlg, msg, p1, p2)
    if msg == F.DN_EDITCHANGE then
      check = check + 1
      assert_eq(p1, 1)
    end
  end
  local Dlg = { {"DI_EDIT", 3,1,56,10, 0,0,0,0, "a"}, }
  mf.acall(far.Dialog, nil,-1,-1,60,3,"Contents",Dlg, 0, DlgProc)
  assert_true(Area.Dialog)
  Keys("W 1 2 3 4 BS Esc")
  assert_eq(check, 6)
  assert_eq(Dlg[1][10], "W123")
end

local function test_utf8_len()
  assert_eq ((""):len(), 0)
  assert_eq (("FOo БАр"):len(), 7)
end

local function test_utf8_sub()
  local text = "abcdабвг"
  local len = assert(text:len()==8) and 8

  for _,start in ipairs{-len*3, 0, 1} do
    assert_eq (text:sub(start, -len*4), "")
    assert_eq (text:sub(start, -len*3), "")
    assert_eq (text:sub(start, -len*2), "")
    assert_eq (text:sub(start, -len-1 + 0), "")
    assert_eq (text:sub(start,          0), "")
    assert_eq (text:sub(start, -len-1 + 1), "a")
    assert_eq (text:sub(start,          1), "a")
    assert_eq (text:sub(start, -len-1 + 6), "abcdаб")
    assert_eq (text:sub(start,          6), "abcdаб")
    assert_eq (text:sub(start, len*1), text)
    assert_eq (text:sub(start, len*2), text)
  end

  for _,start in ipairs{3, -6} do
    assert_eq (text:sub(start, -len*2), "")
    assert_eq (text:sub(start,      0), "")
    assert_eq (text:sub(start,      1), "")
    assert_eq (text:sub(start, start-1), "")
    assert_eq (text:sub(start,      -6), "c")
    assert_eq (text:sub(start, start+0), "c")
    assert_eq (text:sub(start,      -5), "cd")
    assert_eq (text:sub(start, start+3), "cdаб")
    assert_eq (text:sub(start,      -3), "cdаб")
    assert_eq (text:sub(start, len), "cdабвг")
    assert_eq (text:sub(start, 2*len), "cdабвг")
  end

  for _,start in ipairs{len+1, 2*len} do
    for _,fin in ipairs{-2*len, -1*len, -1, 0, 1, len-1, len, 2*len} do
      assert_eq(text:sub(start,fin), "")
    end
  end

  for _,start in ipairs{-2*len,-len-1,-len,-len+1,-1,0,1,len-1,len,len+1} do
    assert_eq(text:sub(start), text:sub(start,len))
  end

  assert_false(pcall(text.sub, text))
  assert_false(pcall(text.sub, text, {}))
  assert_false(pcall(text.sub, text, nil))
end

local function test_utf8_lower_upper()
  assert_eq ((""):lower(), "")
  assert_eq (("abc"):lower(), "abc")
  assert_eq (("ABC"):lower(), "abc")

  assert_eq ((""):upper(), "")
  assert_eq (("abc"):upper(), "ABC")
  assert_eq (("ABC"):upper(), "ABC")

  local russian_abc = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя"
  local part1, part2 = russian_abc:sub(1,33), russian_abc:sub(34)
  assert_eq (part1:lower(), part2)
  assert_eq (part2:lower(), part2)
  assert_eq (part1:upper(), part1)
  assert_eq (part2:upper(), part1)

  local noletters = "1234567890~@#$%^&*()_+-=[]{}|/\\';.,"
  assert_eq (noletters:lower(), noletters)
  assert_eq (noletters:upper(), noletters)
end

---------------------------------------------------------------------------------------------------
-- ACTL_GETWINDOWCOUNT, ACTL_GETWINDOWTYPE, ACTL_GETWINDOWINFO, ACTL_SETCURRENTWINDOW, ACTL_COMMIT
---------------------------------------------------------------------------------------------------
local function test_AdvControl_Window()
  local num, t

  num = far.AdvControl("ACTL_GETWINDOWCOUNT")
  assert(num == 2)
  mf.acall(far.Show); mf.acall(far.Show)
  assert(far.AdvControl("ACTL_GETWINDOWTYPE").Type == F.WTYPE_VMENU)
  assert(num+2 == far.AdvControl("ACTL_GETWINDOWCOUNT"))
  Keys("Esc Esc")
  assert_eq(num, far.AdvControl("ACTL_GETWINDOWCOUNT"))

  -- Get information about 2 available windows
  t = assert_table(far.AdvControl("ACTL_GETWINDOWINFO", 1))
  assert(t.Type==F.WTYPE_DESKTOP and t.Id==0 and t.Pos==1 and t.Flags==0 and #t.TypeName>0 and
         t.Name=="")

  t = assert_table(far.AdvControl("ACTL_GETWINDOWINFO", 2))
  assert(t.Type==F.WTYPE_PANELS and t.Id==0 and t.Pos==2 and t.Flags==F.WIF_CURRENT and
         #t.TypeName>0 and #t.Name>0)
  assert_eq(far.AdvControl("ACTL_GETWINDOWTYPE").Type, F.WTYPE_PANELS)

  -- Set "Desktop" as the current window
  assert_eq (1, far.AdvControl("ACTL_SETCURRENTWINDOW", 1))
  assert_eq (1, far.AdvControl("ACTL_COMMIT"))
  t = assert_table(far.AdvControl("ACTL_GETWINDOWINFO", 2)) -- "z-order": the window that was #1 is now #2
  assert(t.Type==0 and t.Id==0 and t.Pos==2 and t.Flags==F.WIF_CURRENT and #t.TypeName>0 and
         t.Name=="")
  assert_eq (far.AdvControl("ACTL_GETWINDOWTYPE").Type, F.WTYPE_DESKTOP)
  t = assert_table(far.AdvControl("ACTL_GETWINDOWINFO", 1))
  assert(t.Type==F.WTYPE_PANELS and t.Id==0 and t.Pos==1 and t.Flags==0 and #t.TypeName>0 and
         #t.Name>0)

  -- Restore "Panels" as the current window
  assert_eq (1, far.AdvControl("ACTL_SETCURRENTWINDOW", 1))
  assert_eq (1, far.AdvControl("ACTL_COMMIT"))
  assert_eq (far.AdvControl("ACTL_GETWINDOWTYPE").Type, F.WTYPE_PANELS)
end

local function test_AdvControl_Colors()
  local allcolors = assert_table(far.AdvControl("ACTL_GETARRAYCOLOR"))
  assert(#allcolors == 146)
  for n=1,#allcolors do
    local color = assert(far.AdvControl("ACTL_GETCOLOR", n-1))
    assert(color.Flags and color.ForegroundColor and color.BackgroundColor)
    for k,v in pairs(color) do
      assert_eq(allcolors[n][k], v)
    end
  end
  assert_nil(far.AdvControl("ACTL_GETCOLOR", #allcolors))
  assert_nil(far.AdvControl("ACTL_GETCOLOR", -1))

  -- change the colors
  local arr, elem = {StartIndex=0; Flags=0}, {Flags=100; ForegroundColor=101; BackgroundColor=102}
  for n=1,#allcolors do arr[n]=elem end
  assert_eq(far.AdvControl("ACTL_SETARRAYCOLOR", nil, arr), 1)
  for n=1,#allcolors do
    local color = assert_table(far.AdvControl("ACTL_GETCOLOR", n-1))
    for k,v in pairs(elem) do
      assert_eq(color[k], v)
    end
  end

  -- restore the colors
  assert(far.AdvControl("ACTL_SETARRAYCOLOR", nil, allcolors))
end

local function test_AdvControl_Synchro()
  local pass = 0
  local oldProcessSynchroEvent = export.ProcessSynchroEvent
  export.ProcessSynchroEvent =
    function(event,param)
      assert_true(pass==0 and param==123 or pass==1 and param==-456)
      pass = pass + 1
    end
  far.AdvControl("ACTL_SYNCHRO", 123)
  far.AdvControl("ACTL_SYNCHRO", -456)
  for k=1,2 do
    mf.acall(far.Show); Keys"Esc"
  end
  export.ProcessSynchroEvent = oldProcessSynchroEvent
  assert_eq(pass, 2)
end

local function test_AdvControl_Misc()
  local t

  assert_udata(far.AdvControl("ACTL_GETFARHWND"))

  assert_eq(far.AdvControl("ACTL_GETFARMANAGERVERSION"):sub(1,1), "3")
  assert_eq(far.AdvControl("ACTL_GETFARMANAGERVERSION",true), 3)

  t = far.AdvControl("ACTL_GETFARRECT")
  assert(t.Left>=0 and t.Top>=0 and t.Right>t.Left and t.Bottom>t.Top)

  assert_eq(0, far.AdvControl("ACTL_SETCURSORPOS", nil, {X=-1,Y=0}))
  for k=0,2 do
    assert_eq(1, far.AdvControl("ACTL_SETCURSORPOS", nil, {X=k,Y=k+1}))
    t = assert_table(far.AdvControl("ACTL_GETCURSORPOS"))
    assert(t.X==k and t.Y==k+1)
  end

  assert_true(mf.acall(far.AdvControl, "ACTL_WAITKEY", nil, "F1"))
  Keys("F1")
  assert_true(mf.acall(far.AdvControl, "ACTL_WAITKEY"))
  Keys("F2")
end

local function test_AdvControl()
  test_AdvControl_Window()
  test_AdvControl_Colors()
  test_AdvControl_Synchro()
  test_AdvControl_Misc()
end

local function test_far_GetMsg()
  assert_str (far.GetMsg(0))
end

local function test_clipboard()
  local orig = far.PasteFromClipboard()
  local values = { nil, "foo", "", n=3 }
  for k=1,values.n do
    local v = values[k]
    far.CopyToClipboard(v)
    assert_eq(far.PasteFromClipboard(), v)
  end
  if orig then far.CopyToClipboard(orig) end
  assert_eq(far.PasteFromClipboard(), orig)
end

local function test_far_FarClock()
  -- check time difference
  local temp = far.FarClock()
  win.Sleep(500)
  temp = (far.FarClock() - temp) / 1000
  assert(temp > 480 and temp < 550, temp)
  -- check granularity
  local OK = false
  temp = far.FarClock() % 10
  for k=1,10 do
    win.Sleep(20)
    if temp ~= far.FarClock() % 10 then OK=true; break; end
  end
  assert_true(OK)
end

local function test_ProcessName()
  assert_true  (far.CheckMask("f*.ex?"))
  assert_true  (far.CheckMask("/(abc)?def/"))
  assert_false (far.CheckMask("/[[[/"))

  assert_eq    (far.GenerateName("a??b.*", "cdef.txt"),     "adeb.txt")
  assert_eq    (far.GenerateName("a??b.*", "cdef.txt", 50), "adeb.txt")
  assert_eq    (far.GenerateName("a??b.*", "cdef.txt", 2),  "adbef.txt")

  assert_true  (far.CmpName("f*.ex?",      "ftp.exe"        ))
  assert_true  (far.CmpName("f*.ex?",      "fc.exe"         ))
  assert_true  (far.CmpName("f*.ex?",      "f.ext"          ))

  assert_true  (far.CmpName("f*.ex?",      "FTP.exe"        ))

  assert_false (far.CmpName("f*.ex?",      "a/f.ext"        ))
  assert_false (far.CmpName("f*.ex?",      "a/f.ext", 0     ))
  assert_true  (far.CmpName("f*.ex?",      "a/f.ext", "PN_SKIPPATH" ))

  assert_true  (far.CmpName("*co*",        "color.ini"      ))
  assert_true  (far.CmpName("*co*",        "edit.com"       ))
  assert_true  (far.CmpName("[c-ft]*.txt", "config.txt"     ))
  assert_true  (far.CmpName("[c-ft]*.txt", "demo.txt"       ))
  assert_true  (far.CmpName("[c-ft]*.txt", "faq.txt"        ))
  assert_true  (far.CmpName("[c-ft]*.txt", "tips.txt"       ))
  assert_true  (far.CmpName("*",           "foo.bar"        ))
  assert_true  (far.CmpName("*.cpp",       "foo.cpp"        ))
  assert_false (far.CmpName("*.cpp",       "foo.abc"        ))
  assert_false (far.CmpName("*|*.cpp",     "foo.abc"        )) -- exclude mask not supported
  assert_false (far.CmpName("*,*",         "foo.bar"        )) -- mask list not supported

  assert_true (far.CmpNameList("*",          "foo.bar"    ))
  assert_true (far.CmpNameList("*.cpp",      "foo.cpp"    ))
  assert_false(far.CmpNameList("*.cpp",      "foo.abc"    ))

  assert_true (far.CmpNameList("*|*.cpp",    "foo.abc"    )) -- exclude mask IS supported
  assert_true (far.CmpNameList("|*.cpp",     "foo.abc"    ))
  assert_true (far.CmpNameList("*|",         "foo.abc"    ))
  assert_true (far.CmpNameList("*|bar|*",    "foo.abc"    ))
  assert_false(far.CmpNameList("*|*.abc",    "foo.abc"    ))
  assert_false(far.CmpNameList("|",          "foo.abc"    ))

  assert_true (far.CmpNameList("*.aa,*.bar", "foo.bar"    ))
  assert_true (far.CmpNameList("*.aa,*.bar", "c:/foo.bar" ))
  assert_true (far.CmpNameList("/.+/",       "c:/foo.bar" ))
  assert_true (far.CmpNameList("/bar$/",     "c:/foo.bar" ))
  assert_false(far.CmpNameList("/dar$/",     "c:/foo.bar" ))
  assert_true (far.CmpNameList("/abcd/;*",    "/abcd/foo.bar", "PN_SKIPPATH"))
  assert_true (far.CmpNameList("/Makefile(.+)?/", "Makefile"))
  assert_true (far.CmpNameList("/makefile([._\\-].+)?$/i", "Makefile", "PN_SKIPPATH"))

  assert_false (far.CmpNameList("f*.ex?",    "a/f.ext", 0     ))
  assert_true  (far.CmpNameList("f*.ex?",    "a/f.ext", "PN_SKIPPATH" ))

  assert_true  (far.CmpNameList("/ BAR ; /xi  ;*.md", "bar;foo"))
  assert_false (far.CmpNameList("/ BAR ; /xi  ;*.md", "bar,foo"))
  assert_true  (far.CmpNameList("/ BAR ; /xi  ;*.md", "README.md"))
  assert_false (far.CmpNameList("/ BAR ; /xi  ;*.md", "README.me"))
end

local function test_FarStandardFunctions()
  test_clipboard()
  test_far_FarClock()

  test_ProcessName()

  assert_eq(far.ConvertPath([[c:\foo\bar\..\..\abc]], "CPM_FULL"), [[c:\abc]])

  assert_eq(far.FormatFileSize(123456, 0), "123456")
  assert_eq(far.FormatFileSize(123456, 8), "  123456")
  assert_eq(far.FormatFileSize(123456, -8), "123456  ")
  assert_eq(far.FormatFileSize(123456, 0, F.FFFS_COMMAS), "123,456")
  assert_eq(far.FormatFileSize(123456, 0, F.FFFS_SHOWBYTESINDEX), "123456 B")
  assert_eq(far.FormatFileSize(123456, 0, F.FFFS_FLOATSIZE), "121 K")
  assert_eq(far.FormatFileSize(123456, 0, F.FFFS_FLOATSIZE+F.FFFS_ECONOMIC), "121K")
  assert_eq(far.FormatFileSize(123456, 0, F.FFFS_FLOATSIZE+F.FFFS_THOUSAND), "123 k")
  assert_eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x3), "4 T")
  assert_eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x2), "4112 G")
  assert_eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x1), "4210688 M")
  assert_eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x0), "4311744512 K")

  assert_str (far.GetCurrentDirectory())

  assert_eq(far.GetPathRoot[[D:\foo\bar]], [[D:\]])

  assert_true  (far.LIsAlpha("A"))
  assert_true  (far.LIsAlpha("Я"))
  assert_false (far.LIsAlpha("7"))
  assert_false (far.LIsAlpha(";"))

  assert_true  (far.LIsAlphanum("A"))
  assert_true  (far.LIsAlphanum("Я"))
  assert_true  (far.LIsAlphanum("7"))
  assert_false (far.LIsAlphanum(";"))

  assert_false (far.LIsLower("A"))
  assert_true  (far.LIsLower("a"))
  assert_false (far.LIsLower("Я"))
  assert_true  (far.LIsLower("я"))
  assert_false (far.LIsLower("7"))
  assert_false (far.LIsLower(";"))

  assert_true  (far.LIsUpper("A"))
  assert_false (far.LIsUpper("a"))
  assert_true  (far.LIsUpper("Я"))
  assert_false (far.LIsUpper("я"))
  assert_false (far.LIsUpper("7"))
  assert_false (far.LIsUpper(";"))

  assert_eq (far.LLowerBuf("abc-ABC-эюя-ЭЮЯ-7;"), "abc-abc-эюя-эюя-7;")
  assert_eq (far.LUpperBuf("abc-ABC-эюя-ЭЮЯ-7;"), "ABC-ABC-ЭЮЯ-ЭЮЯ-7;")

  assert(far.LStricmp("abc","def") < 0)
  assert(far.LStricmp("def","abc") > 0)
  assert(far.LStricmp("abc","abc") == 0)
  assert(far.LStricmp("ABC","def") < 0)
  assert(far.LStricmp("DEF","abc") > 0)
  assert(far.LStricmp("ABC","abc") == 0)

  assert(far.LStrnicmp("abc","def",3) < 0)
  assert(far.LStrnicmp("def","abc",3) > 0)
  assert(far.LStrnicmp("abc","abc",3) == 0)
  assert(far.LStrnicmp("ABC","def",3) < 0)
  assert(far.LStrnicmp("DEF","abc",3) > 0)
  assert(far.LStrnicmp("ABC","abc",3) == 0)
  assert(far.LStrnicmp("111abc","111def",3) == 0)
  assert(far.LStrnicmp("111abc","111def",4) < 0)
end

-- "Several lines are merged into one".
local function test_issue_3129()
  local fname = win.JoinPath(win.GetEnv("TEMP") or ".", "far3-"..win.Uuid("L"):sub(1,8))
  local fp = assert(io.open(fname, "w"))
  fp:close()
  local flags = {EF_NONMODAL=1, EF_IMMEDIATERETURN=1, EF_DISABLEHISTORY=1}
  assert(editor.Editor(fname,nil,nil,nil,nil,nil,flags) == F.EEC_MODIFIED)
  for k=1,3 do
    editor.InsertString()
    editor.SetString(nil, k, "foo")
  end
  assert_true (editor.SaveFile())
  assert_true (editor.Quit())
  fp = assert(io.open(fname))
  local k = 0
  for line in fp:lines() do
    k = k + 1
    assert_eq (line, "foo")
  end
  fp:close()
  win.DeleteFile(fname)
  assert_eq (k, 3)
end

local function test_gmatch_coro()
  local function yieldFirst(it)
    return coroutine.wrap(function()
      coroutine.yield(it())
    end)
  end

  local it = ("1 2 3"):gmatch("(%d+)")
  local head = yieldFirst(it)
  assert_eq (head(), "1")
end

local function test_PluginsControl()
  local mod = assert(far.PluginStartupInfo().ModuleName)
  local hnd1 = far.FindPlugin("PFM_MODULENAME", mod)
  assert_udata(hnd1)
  local hnd2 = far.FindPlugin("PFM_GUID", far.PluginStartupInfo().PluginGuid)
  assert_udata(hnd2)

  local info = far.GetPluginInformation(hnd1)
  assert_table(info)
  assert_table(info.GInfo)
  assert_table(info.PInfo)
  assert_eq(mod, info.ModuleName)
  assert_num(info.Flags)
  assert(0 ~= band(info.Flags, F.FPF_LOADED))
  assert(0 == band(info.Flags, F.FPF_ANSI))

  local pluglist = far.GetPlugins()
  assert_table(pluglist)
  assert(#pluglist >= 1)
  for _,plug in ipairs(pluglist) do
    assert_udata(plug)
  end

  assert_func(far.LoadPlugin)
  assert_func(far.ForcedLoadPlugin)
  assert_func(far.UnloadPlugin)
end

local function test_far_timer()
  local N = 0
  local timer = far.Timer(50, function(hnd)
      N = N+1
      if N==3 then hnd:Close() end
    end)
  while not timer.Closed do Keys("foobar") end
  assert_eq (N, 3)
end

local function test_win_CompareString()
  assert(win.CompareString("a","b") < 0)
  assert(win.CompareString("b","a") > 0)
  assert(win.CompareString("b","b") == 0)
end

local function test_win()
  test_win_CompareString()

  assert_str(win.GetCurrentDir())
  assert_table(win.EnumSystemCodePages())
  assert_num(win.GetACP())
  assert_num(win.GetOEMCP())

  local dir = assert_str(win.GetEnv("FARHOME"))
  local attr = assert_str(win.GetFileAttr(dir))
  assert_num(attr:find("d"))
end

local function test_utf8()
  test_utf8_len()
  test_utf8_sub()
  test_utf8_lower_upper()
end

local function test_one_guid(val, func, keys, numEsc)
  numEsc = numEsc or 1
  val = far.Guids[val]
  assert_str(val)
  assert_eq(#val, 36)

  if func then func() end
  if keys then Keys(keys) end

  assert_eq(Area.Dialog and Dlg.Id or Menu.Id, val)
  for _ = 1,numEsc do Keys("Esc") end
end

local function test_Guids()
  assert_table(far.Guids)

  Keys("Esc"); print("far:config"); Keys("Enter")
  test_one_guid( "AdvancedConfigId")

  test_one_guid( "ApplyCommandId",           nil, "CtrlG")
  test_one_guid( "AskInsertMenuOrCommandId", nil, "F2 Ins", 2)
  test_one_guid( "ChangeDiskMenuId",         nil, "AltF1")
  test_one_guid( "ChangeDiskMenuId",         nil, "AltF2")
  test_one_guid( "CodePagesMenuId",          nil, "F9 Home 3*Right Enter End 3*Up Enter")
  test_one_guid( "CopyCurrentOnlyFileId",    nil, "End ShiftF5")
  test_one_guid( "CopyFilesId",              nil, "End F5")
--test_one_guid( "DeleteFileFolderId",       nil, "End F8")
  test_one_guid( "DeleteWipeId",             nil, "End AltDel")
  test_one_guid( "DescribeFileId",           nil, "End CtrlZ")
  test_one_guid( "EditUserMenuId",           nil, "F2 Ins Enter", 2)
  test_one_guid( "EditorReplaceId",          nil, "End F4 CtrlF7", 2)
  test_one_guid( "EditorSearchId",           nil, "End F4 F7", 2)
  test_one_guid( "FarAskQuitId",             nil, "F10")

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

  test_one_guid( "FileAttrDlgId",            nil, "End CtrlA")
  test_one_guid( "FileOpenCreateId",         nil, "ShiftF4")
  test_one_guid( "FileSaveAsId",             nil, "End F4 ShiftF2", 2)
  test_one_guid( "FiltersConfigId",          nil, "CtrlI Ins", 2)
  test_one_guid( "FiltersMenuId",            nil, "CtrlI")
  test_one_guid( "FindFileId",               nil, "AltF7")
  test_one_guid( "HardSymLinkId",            nil, "End AltF6")
  test_one_guid( "HelpSearchId",             nil, "F1 F7", 2)
  test_one_guid( "HistoryCmdId",             nil, "AltF8")
  test_one_guid( "HistoryEditViewId",        nil, "AltF11")
  test_one_guid( "HistoryFolderId",          nil, "AltF12")
  test_one_guid( "MakeFolderId",             nil, "F7")
  test_one_guid( "MoveCurrentOnlyFileId",    nil, "End ShiftF6")
  test_one_guid( "MoveFilesId",              nil, "End F6")
  test_one_guid( "PluginInformationId",      nil, "F11 F3", 2)
  test_one_guid( "PluginsConfigMenuId",      nil, "AltShiftF9")
  test_one_guid( "PluginsMenuId",            nil, "F11")
  test_one_guid( "ScreensSwitchId",          nil, "F12")
  test_one_guid( "SelectDialogId",           nil, "Add")
  test_one_guid( "SelectSortModeId",         nil, "CtrlF12")
  test_one_guid( "UnSelectDialogId",         nil, "Subtract")
  test_one_guid( "ViewerSearchId",           nil, "End F3 F7", 2)

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
  -- test_one_guid( "EditorCanNotEditDirectoryId", nil, "")
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
  -- test_one_guid( "EjectHotPlugMediaErrorId", nil, "")
  -- test_one_guid( "FindFileResultId", nil, "")
  -- test_one_guid( "FolderShortcutsMoreId", nil, "")
  -- test_one_guid( "GetNameAndPasswordId", nil, "")
  -- test_one_guid( "RecycleFolderConfirmDeleteLinkId", nil, "")
  -- test_one_guid( "RemoteDisconnectDriveError1Id", nil, "")
  -- test_one_guid( "RemoteDisconnectDriveError2Id", nil, "")
  -- test_one_guid( "SelectAssocMenuId", nil, "")
  -- test_one_guid( "SelectFromEditHistoryId", nil, "")
  -- test_one_guid( "SUBSTDisconnectDriveError1Id", nil, "")
  -- test_one_guid( "SUBSTDisconnectDriveError2Id", nil, "")
  -- test_one_guid( "SUBSTDisconnectDriveId", nil, "")
  -- test_one_guid( "UserMenuUserInputId", nil, "")
  -- test_one_guid( "VHDDisconnectDriveErrorId", nil, "")
  -- test_one_guid( "VHDDisconnectDriveId", nil, "")
  -- test_one_guid( "WipeFolderId", nil, "")
  -- test_one_guid( "WipeHardLinkId", nil, "")
end

function MT.test_luafar()
  test_bit64()
  test_utf8()
  test_win()
  test_AdvControl()
  test_MacroControl()
  test_PluginsControl()
  test_RegexControl()
  test_FarStandardFunctions()
  test_far_GetMsg()
  test_far_timer()
  test_gmatch_coro()
  test_issue_3129()
  test_Guids()
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
  TestArea("Shell", "Run these tests from the Shell area.")
  assert(not APanel.Plugin and not PPanel.Plugin, "Run these tests when neither of panels is a plugin panel.")

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
  MT.test_Panel()
  MT.test_Plugin()
  MT.test_XPanel(APanel)
  MT.test_XPanel(PPanel)
  MT.test_mantis_1722()
  MT.test_luafar()
  MT.test_coroutine()
end

return MT
