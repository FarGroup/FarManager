-- encoding: utf-8
-- Started: 2012-08-20.

--[[
-- The following macro can be used to run all the tests.

Macro {
  description="Macro-engine test";
  area="Shell"; key="CtrlShiftF12";
  action = function()
    local f = assert(loadfile(far.PluginStartupInfo().ModuleDir.."macrotest.lua"))
    setfenv(f, getfenv())().test_all()
    far.Message("All tests OK", "LuaMacro")
  end;
}
--]]

local MT = {} -- "macrotest", this module
local F = far.Flags
local luamacroId="4ebbefc8-2084-4b7f-94c0-692ce136894d" -- LuaMacro plugin GUID

local function pack (...)
  return { n=select("#",...), ... }
end

local function IsNumOrInt(v)
  return type(v)=="number" or bit64.type(v)
end

local TmpFileName = assert(win.GetEnv"tmp" or win.GetEnv"temp").."\\tmp.tmp"

local function WriteTmpFile(...)
  local fp = assert(io.open(TmpFileName,"w"))
  fp:write(...)
  fp:close()
end

local function DeleteTmpFile()
  assert(win.DeleteFile(TmpFileName))
end

local function TestArea (area, msg)
  assert(Area[area]==true and Area.Current==area, msg or "assertion failed")
end

function MT.test_areas()
  Keys "AltIns"              TestArea "Other"      Keys "Esc"
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
--Keys "CtrlT Tab"           TestArea "Tree"       Keys "Tab CtrlT"
--Keys "AltF10"              TestArea "FindFolder" Keys "Esc"
  Keys "F2"                  TestArea "UserMenu"   Keys "Esc"

  assert(Area.Current              =="Shell")
  assert(Area.Other                ==false)
  assert(Area.Shell                ==true)
  assert(Area.Viewer               ==false)
  assert(Area.Editor               ==false)
  assert(Area.Dialog               ==false)
  assert(Area.Search               ==false)
  assert(Area.Disks                ==false)
  assert(Area.MainMenu             ==false)
  assert(Area.Menu                 ==false)
  assert(Area.Help                 ==false)
  assert(Area.Info                 ==false)
  assert(Area.QView                ==false)
  assert(Area.Tree                 ==false)
  assert(Area.FindFolder           ==false)
  assert(Area.UserMenu             ==false)
  assert(Area.ShellAutoCompletion  ==false)
  assert(Area.DialogAutoCompletion ==false)
end

local function test_akey()
  assert(akey == mf.akey)
  local k0,k1 = akey(0),akey(1)
  assert(k0==0x0501007B and k1=="CtrlShiftF12" or
         k0==0x1401007B and k1=="RCtrlShiftF12")
  -- (the 2nd parameter is tested in function test_eval).
end

local function test_bit64()
  assert(band==bit64.band)
  assert(bnot==bit64.bnot)
  assert(bor==bit64.bor)
  assert(bxor==bit64.bxor)
  assert(lshift==bit64.lshift)
  assert(rshift==bit64.rshift)

  assert(band(0xFF,0xFE,0xFD) == 0xFC)
  assert(bor(1,2,4) == 7)
  assert(bnot(5) == -6)
  assert(bxor(0x01,0xF0,0xAA) == 0x5B)
  assert(lshift(0xF731,4) == 0xF7310)
  assert(rshift(0xF7310,4) == 0xF731)

  local v = bit64.new(5)
  assert(v+2==7  and 2+v==7)
  assert(v-2==3  and 2-v==-3)
  assert(v*2==10 and 2*v==10)
  assert(v/2==2  and 2/v==0)
  assert(v%2==1  and 2%v==2)
  assert(v+v==10 and v-v==0 and v*v==25 and v/v==1 and v%v==0)

  local w = lshift(1,63)
  assert(w == bit64.new("0x8000".."0000".."0000".."0000"))
  assert(rshift(w,63)==1)
  assert(rshift(w,64)==0)
  assert(bit64.arshift(w,62)==-2)
  assert(bit64.arshift(w,63)==-1)
  assert(bit64.arshift(w,64)==-1)
end

local function test_eval()
  assert(eval==mf.eval)

  temp=3
  assert(eval("temp=5+7")==0)
  assert(temp==12)

  temp=3
  assert(eval("temp=5+7",0,"moonscript")==0)
  assert(eval("temp=5+7",1,"lua")==0)
  assert(eval("temp=5+7",3,"lua")=="")
  assert(eval("temp=5+7",1,"moonscript")==0)
  assert(eval("temp=5+7",3,"moonscript")=="")
  assert(temp==3)
  assert(eval("getfenv(1).temp=12",0,"moonscript")==0)
  assert(temp==12)

  assert(eval("5",0,"moonscript")==0)
  assert(eval("5+7",1,"lua")==11)
  assert(eval("5+7",1,"moonscript")==0)
  assert(eval("5+7",1,"unknown")==-1)

  -- test with Mode==2
  local Id = assert(far.MacroAdd(nil,nil,"CtrlA",[[
    local key = akey(1,0)
    assert(key=="CtrlShiftF12" or key=="RCtrlShiftF12")
    assert(akey(1,1)=="CtrlA")
    foobar = (foobar or 0) + 1
    return foobar,false,5,nil,"foo"
  ]]))
  for k=1,3 do
    local ret1,a,b,c,d,e = eval("CtrlA",2)
    assert(ret1==0 and a==k and b==false and c==5 and d==nil and e=="foo")
  end
  assert(far.MacroDelete(Id))
end

local function test_abs()
  assert(mf.abs(1.3)==1.3)
  assert(mf.abs(-1.3)==1.3)
  assert(mf.abs(0)==0)
end

local function test_acall()
  local a,b,c,d = mf.acall(function(p) return 3, nil, p, "foo" end, 77)
  assert(a==3 and b==nil and c==77 and d=="foo")
  -- assert(true == mf.acall(far.Show))
  -- Keys"Esc"
end

local function test_asc()
  assert(mf.asc("0")==48)
  assert(mf.asc("Я")==1071)
end

local function test_atoi()
  assert(mf.atoi("0")==0)
  assert(mf.atoi("-10")==-10)
  assert(mf.atoi("0x11")==17)
  assert(mf.atoi("1011",2)==11)
  assert(mf.atoi("123456789123456789")==bit64.new("123456789123456789"))
  assert(mf.atoi("-123456789123456789")==bit64.new("-123456789123456789"))
  assert(mf.atoi("0x1B69B4BACD05F15")==bit64.new("0x1B69B4BACD05F15"))
  assert(mf.atoi("-0x1B69B4BACD05F15")==bit64.new("-0x1B69B4BACD05F15"))
end

local function test_chr()
  assert(mf.chr(48)=="0")
  assert(mf.chr(1071)=="Я")
end

local function test_clip()
  local oldval = far.PasteFromClipboard() -- store
  mf.clip(5,2) -- turn on the internal clipboard
  assert(mf.clip(5,-1)==2)
  assert(mf.clip(5,1)==2) -- turn on the OS clipboard
  assert(mf.clip(5,-1)==1)
  for clipnum=2,1,-1 do -- leaves the OS clipboard active in the end
    mf.clip(5,clipnum)
    local str = "foo"..clipnum
    assert(mf.clip(1,str) ~= 0)
    assert(mf.clip(0,str) == str)
  end
  far.CopyToClipboard(oldval) -- restore
end

local function test_env()
  mf.env("Foo",1,"Bar")
  assert(mf.env("Foo")=="Bar")
  mf.env("Foo",1,"")
  assert(mf.env("Foo")=="")
end

local function test_fattr()
  WriteTmpFile("")
  local attr = mf.fattr(TmpFileName)
  DeleteTmpFile()
  assert(type(attr)=="number")
end

local function test_fexist()
  WriteTmpFile("")
  assert(mf.fexist(TmpFileName))
  DeleteTmpFile()
  assert(not mf.fexist(TmpFileName))
end

local function test_msgbox()
  assert(msgbox == mf.msgbox)
  mf.postmacro(function() Keys("Esc") end)
  assert(0 == msgbox("title","message"))
  mf.postmacro(function() Keys("Enter") end)
  assert(1 == msgbox("title","message"))
end

local function test_prompt()
  assert(prompt == mf.prompt)
  mf.postmacro(function() Keys("a b c Esc") end)
  assert(not prompt())
  mf.postmacro(function() Keys("a b c Enter") end)
  assert("abc" == prompt())
end

local function test_date()
  assert(type(mf.date())=="string")
  assert(type(mf.date("%a"))=="string")
end

local function test_fmatch()
  assert(mf.fmatch("Readme.txt", "*.txt") == 1)
  assert(mf.fmatch("Readme.txt", "Readme.*|*.txt") == 0)
  assert(mf.fmatch("c:\\Readme.txt", "/txt$/i") == 1)
  assert(mf.fmatch("c:\\Readme.txt", "/txt$") == -1)
end

local function test_fsplit()
  local path="C:\\Program Files\\Far\\Far.exe"
  assert(mf.fsplit(path,1)=="C:\\")
  assert(mf.fsplit(path,2)=="\\Program Files\\Far\\")
  assert(mf.fsplit(path,4)=="Far")
  assert(mf.fsplit(path,8)==".exe")
end

local function test_iif()
  assert(mf.iif(true,  1, 2)==1)
  assert(mf.iif(false, 1, 2)==2)
  assert(mf.iif("a",   1, 2)==1)
  assert(mf.iif("",    1, 2)==2)
end

local function test_index()
  assert(mf.index("language","gua",0)==3)
  assert(mf.index("language","gua",1)==3)
  assert(mf.index("language","gUA",1)==-1)
  assert(mf.index("language","gUA",0)==3)
end

local function test_int()
  assert(mf.int("2.99")==2)
  assert(mf.int("-2.99")==-2)
  assert(mf.int("0x10")==0)
  assert(mf.int("123456789123456789")==bit64.new("123456789123456789"))
  assert(mf.int("-123456789123456789")==bit64.new("-123456789123456789"))
end

local function test_itoa()
  assert(mf.itoa(100)=="100")
  assert(mf.itoa(100,10)=="100")
  assert(mf.itoa(bit64.new("123456789123456789"))=="123456789123456789")
  assert(mf.itoa(bit64.new("-123456789123456789"))=="-123456789123456789")
  assert(mf.itoa(100,2)=="1100100")
  assert(mf.itoa(100,16)=="64")
  assert(mf.itoa(100,36)=="2s")
end

local function test_key()
  assert(mf.key(0x01000000)=="Ctrl")
  assert(mf.key(0x02000000)=="Alt")
  assert(mf.key(0x04000000)=="Shift")
  assert(mf.key(0x10000000)=="RCtrl")
  assert(mf.key(0x20000000)=="RAlt")

  assert(mf.key(0x0501007B)=="CtrlShiftF12")
  assert(mf.key("CtrlShiftF12")=="CtrlShiftF12")

  assert(mf.key("foobar")=="")
end

-- Separate tests for mf.float and mf.string are locale-dependant, thus they are tested together.
local function test_float_and_string()
  local t = { 0, -0, 2.56e1, -5.37, -2.2e100, 2.2e-100 }
  for _,num in ipairs(t) do
    assert(mf.float(mf.string(num))==num)
  end
end

local function test_lcase()
  assert(mf.lcase("FOo БАр")=="foo бар")
end

local function test_len()
  assert(mf.len("FOo БАр")==7)
end

local function test_max()
  assert(mf.max(-2,-5)==-2)
  assert(mf.max(2,5)==5)
end

local function test_min()
  assert(mf.min(-2,-5)==-5)
  assert(mf.min(2,5)==2)
end

local function test_msave()
  local Key = "macrotest"

  -- test supported types, except tables
  local v1, v2, v3, v4, v5, v6 = nil, false, true, -5.67, "foo", bit64.new("-123")
  mf.msave(Key, "name1", v1)
  mf.msave(Key, "name2", v2)
  mf.msave(Key, "name3", v3)
  mf.msave(Key, "name4", v4)
  mf.msave(Key, "name5", v5)
  mf.msave(Key, "name6", v6)
  assert(mf.mload(Key, "name1") == v1)
  assert(mf.mload(Key, "name2") == v2)
  assert(mf.mload(Key, "name3") == v3)
  assert(mf.mload(Key, "name4") == v4)
  assert(mf.mload(Key, "name5") == v5)
  assert(mf.mload(Key, "name6") == v6)
  mf.mdelete(Key, "*")
  assert(mf.mload(Key, "name3")==nil)

  -- test tables
  mf.msave(Key, "name1", { a=5, {b="foo"}, c={d=false} })
  local t=mf.mload(Key, "name1")
  assert(t.a==5 and t[1].b=="foo" and t.c.d==false)
  mf.mdelete(Key, "name1")
  assert(mf.mload(Key, "name1")==nil)

  -- test tables more
  local t1, t2, t3 = {5}, {6}, {}
  t1[2], t1[3], t1[4], t1[5] = t1, t2, t3, t3
  t2[2], t2[3] = t1, t2
  t1[t1], t1[t2] = 66, 77
  t2[t1], t2[t2] = 88, 99
  setmetatable(t3, { __index=t1 })
  mf.msave(Key, "name1", t1)

  local T1 = mf.mload(Key, "name1")
  assert(type(T1)=="table")
  local T2 = T1[3]
  assert(type(T2)=="table")
  local T3 = T1[4]
  assert(type(T3)=="table" and T3==T1[5])
  assert(T1[1]==5 and T1[2]==T1 and T1[3]==T2)
  assert(T2[1]==6 and T2[2]==T1 and T2[3]==T2)
  assert(T1[T1]==66 and T1[T2]==77)
  assert(T2[T1]==88 and T2[T2]==99)
  assert(getmetatable(T3).__index==T1 and T3[1]==5 and rawget(T3,1)==nil)
  mf.mdelete(Key, "*")
  assert(mf.mload(Key, "name1")==nil)

  -- test locations (profiles)
  if win.GetEnv("FARPROFILE") ~= win.GetEnv("FARLOCALPROFILE") then
    mf.msave("key1", "name1", 100)
    mf.msave("key2", "name2", 200, "roaming")
    mf.msave("key1", "name1", 300, "local")
    for k=1,2 do
      assert(mf.mload("key1", "name1") == 100)
      assert(mf.mload("key1", "name1", "roaming") == 100)
      assert(mf.mload("key2", "name2") == 200)
      assert(mf.mload("key2", "name2", "roaming") == 200)
      assert(mf.mload("key1", "name1", "local") == 300)
    end
    mf.mdelete("key1", "name1")
    mf.mdelete("key2", "name2", "roaming")
    assert(mf.mload("key1", "name1") == nil)
    assert(mf.mload("key2", "name2") == nil)
    assert(mf.mload("key1", "name1", "local") == 300)
    mf.mdelete("key1", "name1", "local")
    assert(mf.mload("key1", "name1", "local") == nil)
  end
end

local function test_mod()
  assert(mf.mod(11,4)==3)
end

local function test_replace()
  assert(mf.replace("Foo Бар", "o", "1")=="F11 Бар")
  assert(mf.replace("Foo Бар", "o", "1", 1)=="F1o Бар")
  assert(mf.replace("Foo Бар", "O", "1", 1, 1)=="Foo Бар")
  assert(mf.replace("Foo Бар", "O", "1", 1, 0)=="F1o Бар")
end

local function test_rindex()
  assert(mf.rindex("language","a",0)==5)
  assert(mf.rindex("language","a",1)==5)
  assert(mf.rindex("language","A",1)==-1)
  assert(mf.rindex("language","A",0)==5)
end

local function test_strpad()
  assert(mf.strpad("Foo",10,"*",  2) == '***Foo****')
  assert(mf.strpad("",   10,"-*-",2) == '-*--*--*--')
  assert(mf.strpad("",   10,"-*-")   == '-*--*--*--')
  assert(mf.strpad("Foo",10)         == 'Foo       ')
  assert(mf.strpad("Foo",10,"-")     == 'Foo-------')
  assert(mf.strpad("Foo",10," ",  1) == '       Foo')
  assert(mf.strpad("Foo",10," ",  2) == '   Foo    ')
  assert(mf.strpad("Foo",10,"1234567890",2) == '123Foo1234')
end

local function test_strwrap()
  assert(mf.strwrap("Пример строки, которая будет разбита на несколько строк по ширине в 7 символов.", 7)==
[[
Пример
строки,
которая
будет
разбита
на
несколько
строк
по
ширине
в 7
символов.]])

  assert(mf.strwrap("Пример строки, которая будет разбита на несколько строк по ширине в 7 символов.", 7,"\n",1)==
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

local function test_substr()
  assert(mf.substr("abcdef", 1) == "bcdef")
  assert(mf.substr("abcdef", 1, 3) == "bcd")
  assert(mf.substr("abcdef", 0, 4) == "abcd")
  assert(mf.substr("abcdef", 0, 8) == "abcdef")
  assert(mf.substr("abcdef", -1) == "f")
  assert(mf.substr("abcdef", -2) == "ef")
  assert(mf.substr("abcdef", -3, 1) == "d")
  assert(mf.substr("abcdef", 0, -1) == "abcde")
  assert(mf.substr("abcdef", 2, -1) == "cde")
  assert(mf.substr("abcdef", 4, -4) == "")
  assert(mf.substr("abcdef", -3, -1) == "de")
end

local function test_testfolder()
  assert(mf.testfolder(".") > 0)
  assert(mf.testfolder("C:\\") == 2)
  assert(mf.testfolder("@:\\") <= 0)
end

local function test_trim()
  assert(mf.trim(" abc ")=="abc")
  assert(mf.trim(" abc ",0)=="abc")
  assert(mf.trim(" abc ",1)=="abc ")
  assert(mf.trim(" abc ",2)==" abc")
end

local function test_ucase()
  assert(mf.ucase("FOo БАр")=="FOO БАР")
end

local function test_waitkey()
  assert(mf.waitkey(50,0)=="")
  assert(mf.waitkey(50,1)==0xFFFFFFFF)
end

local function test_size2str()
  assert(mf.size2str(123,0,5)=="  123")
  assert(mf.size2str(123,0,-5)=="123  ")
end

local function test_xlat()
  assert(type(mf.xlat("abc"))=="string")
  -- commented out, as these tests won't work with any Windows configuration:
  --assert(mf.xlat("ghzybr")=="пряник")
  --assert(mf.xlat("сщьзгеук")=="computer")
end

local function test_beep()
  assert(type(mf.beep())=="boolean")
end

local function test_flock()
  assert(type(mf.flock(0,-1))=="number")
end

local function test_GetMacroCopy()
  assert(type(mf.GetMacroCopy) == "function")
end

local function test_Keys()
  assert(Keys == mf.Keys)
  assert(type(Keys) == "function")

  Keys("Esc F a r Space M a n a g e r Space Ф А Р")
  assert(panel.GetCmdLine() == "Far Manager ФАР")
  Keys("Esc")
  assert(panel.GetCmdLine() == "")
end

local function test_exit()
  assert(exit == mf.exit)
  local N
  mf.postmacro(
    function()
      local function f() N=50; exit(); end
      f(); N=100
    end)
  mf.postmacro(function() Keys"Esc" end)
  far.Message("dummy")
  assert(N == 50)
end

local function test_mmode()
  assert(mmode == mf.mmode)
  assert(1 == mmode(1,-1))
end

local function test_print()
  assert(print == mf.print)
  assert(type(print) == "function")

  local str = "abc ABC абв АБВ"
  Keys("Esc")
  print(str)
  assert(panel.GetCmdLine() == str)
  Keys("Esc")
  assert(panel.GetCmdLine() == "")
end

local function test_postmacro()
  assert(type(mf.postmacro) == "function")
end

local function test_sleep()
  assert(type(mf.sleep) == "function")
end

local function test_usermenu()
  assert(type(mf.usermenu) == "function")
end

function MT.test_mf()
  test_abs()
  test_acall()
  test_akey()
  test_asc()
  test_atoi()
  test_beep()
  test_chr()
  test_clip()
  test_date()
  test_env()
  test_eval()
  test_exit()
  test_fattr()
  test_fexist()
  test_float_and_string()
  test_flock()
  test_fmatch()
  test_fsplit()
  test_GetMacroCopy()
  test_iif()
  test_index()
  test_int()
  test_itoa()
  test_key()
  test_Keys()
  test_lcase()
  test_len()
  test_max()
  test_min()
  test_mmode()
  test_mod()
  test_msave()
  test_msgbox()
  test_postmacro()
  test_print()
  test_prompt()
  test_replace()
  test_rindex()
  test_size2str()
  test_sleep()
  test_strpad()
  test_strwrap()
  test_substr()
  test_testfolder()
  test_trim()
  test_ucase()
  test_usermenu()
  test_waitkey()
  test_xlat()
end

function MT.test_CmdLine()
  Keys"Esc f o o Space Б а р"
  assert(CmdLine.Bof==false)
  assert(CmdLine.Eof==true)
  assert(CmdLine.Empty==false)
  assert(CmdLine.Selected==false)
  assert(CmdLine.Value=="foo Бар")
  assert(CmdLine.ItemCount==7)
  assert(CmdLine.CurPos==8)

  Keys"SelWord"
  assert(CmdLine.Selected)

  Keys"CtrlHome"
  assert(CmdLine.Bof==true)
  assert(CmdLine.Eof==false)

  Keys"Esc"
  assert(CmdLine.Bof==true)
  assert(CmdLine.Eof==true)
  assert(CmdLine.Empty==true)
  assert(CmdLine.Selected==false)
  assert(CmdLine.Value=="")
  assert(CmdLine.ItemCount==0)
  assert(CmdLine.CurPos==1)

  Keys"Esc"
  print("foo Бар")
  assert(CmdLine.Value=="foo Бар")

  Keys"Esc"
  print(("%s %d %s"):format("foo", 5+7, "Бар"))
  assert(CmdLine.Value=="foo 12 Бар")

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
    "Editor.SearchRegexp", "boolean",
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
    "Help.HelpSearchRegexp", "boolean",
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
    "Panel.Left.CaseSensitiveSort", "boolean",
    "Panel.Left.DirectoriesFirst", "boolean",
    "Panel.Left.NumericSort", "boolean",
    "Panel.Left.SelectedFirst", "boolean",
    "Panel.Left.ShortNames", "boolean",
    "Panel.Left.SortGroups", "boolean",
    "Panel.Left.SortMode", "integer",
    "Panel.Left.ReverseSortOrder", "boolean",
    "Panel.Left.Type", "integer",
    "Panel.Left.ViewMode", "integer",
    "Panel.Left.Visible", "boolean",
    "Panel.Right.CaseSensitiveSort", "boolean",
    "Panel.Right.DirectoriesFirst", "boolean",
    "Panel.Right.NumericSort", "boolean",
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
    "System.DelThreadPriority", "integer",
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
    "System.SmartFolderMonitor", "boolean",
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
    "System.Executor.BatchType", "string",
    "System.Executor.ExcludeCmds", "string",
    "System.Executor.FullTitle", "boolean",
    "System.Executor.RestoreCP", "boolean",
    "System.Executor.UseAppPath", "boolean",
    "System.Executor.UseHomeDir", "boolean",
    "System.Executor.NotQuotedShell", "string",
    "System.Executor.ComSpecParams", "string",
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
    "Viewer.SearchEditFocus", "boolean",
    "Viewer.SearchRegexp", "boolean",
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
  local temp = Far.UpTime
  mf.sleep(50)
  temp = Far.UpTime - temp
  assert(temp > 40 and temp < 70)

  assert(type(Far.KbdLayout(0))=="number")
  assert(type(Far.KeyBar_Show(0))=="number")
  assert(type(Far.Window_Scroll)=="function")

  test_Far_GetConfig()
end

local function test_CheckAndGetHotKey()
  mf.acall(far.Menu, {Flags="FMENU_AUTOHIGHLIGHT"},
    {{text="abcd"},{text="abc&d"},{text="abcd"},{text="abcd"},{text="abcd"}})

  assert(Object.CheckHotkey("a")==1)
  assert(Object.GetHotkey(1)=="a")
  assert(Object.GetHotkey()=="a")
  assert(Object.GetHotkey(0)=="a")

  assert(Object.CheckHotkey("b")==3)
  assert(Object.GetHotkey(3)=="b")

  assert(Object.CheckHotkey("c")==4)
  assert(Object.GetHotkey(4)=="c")

  assert(Object.CheckHotkey("d")==2)
  assert(Object.GetHotkey(2)=="d")

  assert(Object.CheckHotkey("e")==0)

  assert(Object.CheckHotkey("")==5)
  assert(Object.GetHotkey(5)=="")
  assert(Object.GetHotkey(6)=="")

  Keys("Esc")
end

function MT.test_Object()
  assert(type(Object.Bof)         == "boolean")
  assert(type(Object.CurPos)      == "number")
  assert(type(Object.Empty)       == "boolean")
  assert(type(Object.Eof)         == "boolean")
  assert(type(Object.Height)      == "number")
  assert(type(Object.ItemCount)   == "number")
  assert(type(Object.Selected)    == "boolean")
  assert(type(Object.Title)       == "string")
  assert(type(Object.Width)       == "number")

  test_CheckAndGetHotKey()
end

function MT.test_Drv()
  Keys"AltF1"
  assert(type(Drv.ShowMode) == "number")
  assert(Drv.ShowPos == 1)
  Keys"Esc AltF2"
  assert(type(Drv.ShowMode) == "number")
  assert(Drv.ShowPos == 2)
  Keys"Esc"
end

function MT.test_Help()
  Keys"F1"
  assert(type(Help.FileName)=="string")
  assert(type(Help.SelTopic)=="string")
  assert(type(Help.Topic)=="string")
  Keys"Esc"
end

function MT.test_Mouse()
  assert(type(Mouse.X) == "number")
  assert(type(Mouse.Y) == "number")
  assert(type(Mouse.Button) == "number")
  assert(type(Mouse.CtrlState) == "number")
  assert(type(Mouse.EventFlags) == "number")
  assert(type(Mouse.LastCtrlState) == "number")
end

function MT.test_XPanel(pan) -- (@pan: either APanel or PPanel)
  assert(type(pan.Bof)         == "boolean")
  assert(type(pan.ColumnCount) == "number")
  assert(type(pan.CurPos)      == "number")
  assert(type(pan.Current)     == "string")
  assert(type(pan.DriveType)   == "number")
  assert(type(pan.Empty)       == "boolean")
  assert(type(pan.Eof)         == "boolean")
  assert(type(pan.FilePanel)   == "boolean")
  assert(type(pan.Filter)      == "boolean")
  assert(type(pan.Folder)      == "boolean")
  assert(type(pan.Format)      == "string")
  assert(type(pan.Height)      == "number")
  assert(type(pan.HostFile)    == "string")
  assert(type(pan.ItemCount)   == "number")
  assert(type(pan.Left)        == "boolean")
  assert(type(pan.LFN)         == "boolean")
  assert(type(pan.OPIFlags)    == "number")
  assert(type(pan.Path)        == "string")
  assert(type(pan.Path0)       == "string")
  assert(type(pan.Plugin)      == "boolean")
  assert(type(pan.Prefix)      == "string")
  assert(type(pan.Root)        == "boolean")
  assert(type(pan.SelCount)    == "number")
  assert(type(pan.Selected)    == "boolean")
  assert(type(pan.Type)        == "number")
  assert(type(pan.UNCPath)     == "string")
  assert(type(pan.Visible)     == "boolean")
  assert(type(pan.Width)       == "number")

  if pan == APanel then
    Keys "End"  assert(pan.Eof==true)
    Keys "Home" assert(pan.Bof==true)
  end
end

local function test_Panel_Item()
  for pt=0,1 do
    assert(type(Panel.Item(pt,0,0))  =="string")
    assert(type(Panel.Item(pt,0,1))  =="string")
    assert(type(Panel.Item(pt,0,2))  =="number")
    assert(type(Panel.Item(pt,0,3))  =="string")
    assert(type(Panel.Item(pt,0,4))  =="string")
    assert(type(Panel.Item(pt,0,5))  =="string")
    assert(IsNumOrInt(Panel.Item(pt,0,6)))
    assert(IsNumOrInt(Panel.Item(pt,0,7)))
    assert(type(Panel.Item(pt,0,8))  =="boolean")
    assert(type(Panel.Item(pt,0,9))  =="number")
    assert(type(Panel.Item(pt,0,10)) =="boolean")
    assert(type(Panel.Item(pt,0,11)) =="string")
    assert(type(Panel.Item(pt,0,12)) =="string")
    assert(type(Panel.Item(pt,0,13)) =="number")
    assert(type(Panel.Item(pt,0,14)) =="number")
    assert(IsNumOrInt(Panel.Item(pt,0,15)))
    assert(IsNumOrInt(Panel.Item(pt,0,16)))
    assert(IsNumOrInt(Panel.Item(pt,0,17)))
    assert(type(Panel.Item(pt,0,18)) =="number")
    assert(IsNumOrInt(Panel.Item(pt,0,19)))
    assert(type(Panel.Item(pt,0,20)) =="string")
    assert(IsNumOrInt(Panel.Item(pt,0,21)))
    assert(type(Panel.Item(pt,0,22)) =="string")
    assert(type(Panel.Item(pt,0,23)) =="number")
  end
end

function MT.test_Panel()
  test_Panel_Item()

  assert(Panel.FAttr(0,":")==-1)
  assert(Panel.FAttr(1,":")==-1)

  assert(Panel.FExist(0,":")==0)
  assert(Panel.FExist(1,":")==0)

  assert(type(Panel.Select)    == "function")
  assert(type(Panel.SetPath)   == "function")
  assert(type(Panel.SetPos)    == "function")
  assert(type(Panel.SetPosIdx) == "function")
end

function MT.test_Dlg()
  Keys"F7 a b c"
  assert(Area.Dialog)
  assert(Dlg.Id == "FAD00DBE-3FFF-4095-9232-E1CC70C67737")
  assert(Dlg.Owner == "00000000-0000-0000-0000-000000000000")
  assert(Dlg.ItemCount > 6)
  assert(Dlg.ItemType == 4)
  assert(Dlg.CurPos == 3)
  assert(Dlg.PrevPos == 0)

  Keys"Tab"
  local pos = Dlg.CurPos
  assert(Dlg.CurPos > 3)
  assert(Dlg.PrevPos == 3)
  assert(pos == Dlg.SetFocus(3))
  assert(pos == Dlg.PrevPos)

  assert(Dlg.GetValue(0,0) == Dlg.ItemCount)
  Keys"Esc"
end

function MT.test_Plugin()
  assert(Plugin.Menu()==false)
  assert(Plugin.Config()==false)
  assert(Plugin.Command()==false)
  assert(Plugin.Command(luamacroId)==true)

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
    assert(type(t) == "table")
    assert(t.n  == 6)
    assert(t[1] == "foo")
    assert(t[2] == false)
    assert(t[3] == 5)
    assert(t[4] == nil)
    assert(t[5] == bit64.new("0x8765876587658765"))
    assert(type(t[6])=="table" and t[6][1]=="bar")
  end
  test("return ...", nil)
  test("return ...", "KMFLAGS_LUA")
  test("...", "KMFLAGS_MOONSCRIPT")
end

local function test_far_MacroAdd()
  local area, key, descr = "MACROAREA_SHELL", "CtrlA", "Test MacroAdd"

  local Id = far.MacroAdd(area, nil, key, [[A = { b=5 }]], descr)
  assert(type(Id)=="userdata" and far.MacroDelete(Id))

  Id = far.MacroAdd(-1, nil, key, [[A = { b=5 }]], descr)
  assert(not Id) -- bad area

  Id = far.MacroAdd(area, nil, key, [[A = { b:5 }]], descr)
  assert(not Id) -- bad code

  Id = far.MacroAdd(area, "KMFLAGS_MOONSCRIPT", key, [[A = { b:5 }]], descr)
  assert(type(Id)=="userdata" and far.MacroDelete(Id))

  Id = far.MacroAdd(area, "KMFLAGS_MOONSCRIPT", key, [[A = { b=5 }]], descr)
  assert(not Id) -- bad code

  Id = far.MacroAdd(area, nil, key, [[@c:\myscript 5+6,"foo"]], descr)
  assert(type(Id)=="userdata" and far.MacroDelete(Id))

  Id = far.MacroAdd(area, nil, key, [[@c:\myscript 5***6,"foo"]], descr)
  assert(type(Id)=="userdata" and far.MacroDelete(Id)) -- with @ there is no syntax check till the macro runs

  Id = far.MacroAdd(nil, nil, key, [[@c:\myscript]])
  assert(type(Id)=="userdata" and far.MacroDelete(Id)) -- check default area (MACROAREA_COMMON)

  local Id = far.MacroAdd(area,nil,key,[[Keys"F7" assert(Dlg.Id=="FAD00DBE-3FFF-4095-9232-E1CC70C67737") Keys"Esc"]],descr)
  assert(0==mf.eval("Shell/"..key, 2))
  assert(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) return false end)
  assert(-2 == mf.eval("Shell/"..key, 2))
  assert(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) error"deliberate error" end)
  assert(-2 == mf.eval("Shell/"..key, 2))
  assert(far.MacroDelete(Id))

  Id = far.MacroAdd(area,nil,key,[[a=5]],descr,function(id,flags) return id==Id end)
  assert(0 == mf.eval("Shell/"..key, 2))
  assert(far.MacroDelete(Id))

end

local function test_far_MacroCheck()
  assert(far.MacroCheck([[A = { b=5 }]]))
  assert(far.MacroCheck([[A = { b=5 }]], "KMFLAGS_LUA"))

  assert(not far.MacroCheck([[A = { b:5 }]], "KMFLAGS_SILENTCHECK"))

  assert(far.MacroCheck([[A = { b:5 }]], "KMFLAGS_MOONSCRIPT"))

  assert(not far.MacroCheck([[A = { b=5 }]], {KMFLAGS_MOONSCRIPT=1,KMFLAGS_SILENTCHECK=1} ))

  WriteTmpFile [[A = { b=5 }]] -- valid Lua, invalid MoonScript
  assert(far.MacroCheck("@"..TmpFileName, "KMFLAGS_LUA"))
  assert(far.MacroCheck("@"..TmpFileName.." 5+6,'foo'", "KMFLAGS_LUA")) -- valid file arguments
  assert(not far.MacroCheck("@"..TmpFileName.." 5***6,'foo'", "KMFLAGS_SILENTCHECK")) -- invalid file arguments
  assert(not far.MacroCheck("@"..TmpFileName, {KMFLAGS_MOONSCRIPT=1,KMFLAGS_SILENTCHECK=1}))

  WriteTmpFile [[A = { b:5 }]] -- invalid Lua, valid MoonScript
  assert(not far.MacroCheck("@"..TmpFileName, "KMFLAGS_SILENTCHECK"))
  assert(far.MacroCheck("@"..TmpFileName, "KMFLAGS_MOONSCRIPT"))
  DeleteTmpFile()

  assert(not far.MacroCheck([[@//////]], "KMFLAGS_SILENTCHECK"))
end

local function test_far_MacroGetArea()
  assert(far.MacroGetArea()==F.MACROAREA_SHELL)
end

local function test_far_MacroGetLastError()
  assert(far.MacroCheck("a=1"))
  assert(far.MacroGetLastError().ErrSrc=="")
  assert(not far.MacroCheck("a=", "KMFLAGS_SILENTCHECK"))
  assert(far.MacroGetLastError().ErrSrc:len() > 0)
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
  local rep = "%1%1"
  local R = regex.new(pat)

  local fr,to,cap
  local str, nfound, nrep

  assert(R:bracketscount()==2)

  fr,to,cap = regex.find("abc", pat)
  assert(fr==2 and to==3 and cap=="bc")
  fr,to,cap = regex.findW(L"abc", pat)
  assert(fr==2 and to==3 and cap==L"bc")

  fr,to,cap = R:find("abc")
  assert(fr==2 and to==3 and cap=="bc")
  fr,to,cap = R:findW(L"abc")
  assert(fr==2 and to==3 and cap==L"bc")

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

  str, nfound, nrep = regex.gsub(";a;", "a*", "ITEM")
  assert(str=="ITEM;ITEM;ITEM" and nfound==3 and nrep==3)
  str, nfound, nrep = regex.gsub(";a;", "a*?", "ITEM")
  assert(str=="ITEM;ITEMaITEM;ITEM" and nfound==4 and nrep==4)
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
      assert(p1 == 1)
    end
  end
  local Dlg = { {"DI_EDIT", 3,1,56,10, 0,0,0,0, "a"}, }
  mf.acall(far.Dialog, "",-1,-1,60,3,"Contents",Dlg, 0, DlgProc)
  assert(Area.Dialog)
  Keys("W 1 2 3 4 BS Esc")
  assert(check == 6)
  assert(Dlg[1][10] == "W123")
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
  assert(num == far.AdvControl("ACTL_GETWINDOWCOUNT"))

  -- Get information about 2 available windows
  t = assert(far.AdvControl("ACTL_GETWINDOWINFO", 1))
  assert(t.Type==F.WTYPE_DESKTOP and t.Id==0 and t.Pos==1 and t.Flags==0 and #t.TypeName>0 and
         t.Name=="")

  t = assert(far.AdvControl("ACTL_GETWINDOWINFO", 2))
  assert(t.Type==F.WTYPE_PANELS and t.Id==0 and t.Pos==2 and t.Flags==F.WIF_CURRENT and
         #t.TypeName>0 and #t.Name>0)
  assert(far.AdvControl("ACTL_GETWINDOWTYPE").Type == F.WTYPE_PANELS)

  -- Set "Desktop" as the current window
  assert(1 == far.AdvControl("ACTL_SETCURRENTWINDOW", 1))
  assert(1 == far.AdvControl("ACTL_COMMIT"))
  t = assert(far.AdvControl("ACTL_GETWINDOWINFO", 2)) -- "z-order": the window that was #1 is now #2
  assert(t.Type==0 and t.Id==0 and t.Pos==2 and t.Flags==F.WIF_CURRENT and #t.TypeName>0 and
         t.Name=="")
  assert(far.AdvControl("ACTL_GETWINDOWTYPE").Type == F.WTYPE_DESKTOP)
  t = assert(far.AdvControl("ACTL_GETWINDOWINFO", 1))
  assert(t.Type==F.WTYPE_PANELS and t.Id==0 and t.Pos==1 and t.Flags==0 and #t.TypeName>0 and
         #t.Name>0)

  -- Restore "Panels" as the current window
  assert(1 == far.AdvControl("ACTL_SETCURRENTWINDOW", 1))
  assert(1 == far.AdvControl("ACTL_COMMIT"))
  assert(far.AdvControl("ACTL_GETWINDOWTYPE").Type == F.WTYPE_PANELS)
end

local function test_AdvControl_Colors()
  local t = assert(far.AdvControl("ACTL_GETARRAYCOLOR"))
  assert(#t == 146)
  for n=1,#t do
    local color = assert(far.AdvControl("ACTL_GETCOLOR", n-1))
    assert(color.Flags and color.ForegroundColor and color.BackgroundColor)
    for k,v in pairs(color) do
      assert(t[n][k] == v)
    end
  end
  assert(not far.AdvControl("ACTL_GETCOLOR", #t))
  assert(not far.AdvControl("ACTL_GETCOLOR", -1))

  -- change the colors
  local arr, elem = {StartIndex=0; Flags=0}, {Flags=100; ForegroundColor=101; BackgroundColor=102}
  for n=1,#t do arr[n]=elem end
  assert(far.AdvControl("ACTL_SETARRAYCOLOR", nil, arr))
  for n=1,#t do
    local color = assert(far.AdvControl("ACTL_GETCOLOR", n-1))
    for k,v in pairs(elem) do
      assert(color[k] == v)
    end
  end

  -- restore the colors
  assert(far.AdvControl("ACTL_SETARRAYCOLOR", nil, t))
end

local function test_AdvControl_Synchro()
  local pass = 0
  local oldProcessSynchroEvent = export.ProcessSynchroEvent
  export.ProcessSynchroEvent =
    function(event,param)
      assert(pass==0 and param==123 or pass==1 and param==-456)
      pass = pass + 1
    end
  far.AdvControl("ACTL_SYNCHRO", 123)
  far.AdvControl("ACTL_SYNCHRO", -456)
  for k=1,2 do
    mf.acall(far.Show); Keys"Esc"
  end
  export.ProcessSynchroEvent = oldProcessSynchroEvent
  assert(pass == 2)
end

local function test_AdvControl_Misc()
  local t

  assert(type(far.AdvControl("ACTL_GETFARHWND"))=="userdata")

  assert(far.AdvControl("ACTL_GETFARMANAGERVERSION"):sub(1,1)=="3")
  assert(far.AdvControl("ACTL_GETFARMANAGERVERSION",true)==3)

  t = far.AdvControl("ACTL_GETFARRECT")
  assert(t.Left>=0 and t.Top>=0 and t.Right>t.Left and t.Bottom>t.Top)

  assert(0 == far.AdvControl("ACTL_SETCURSORPOS", nil, {X=-1,Y=0}))
  for k=0,2 do
    assert(1 == far.AdvControl("ACTL_SETCURSORPOS", nil, {X=k,Y=k+1}))
    t = assert(far.AdvControl("ACTL_GETCURSORPOS"))
    assert(t.X==k and t.Y==k+1)
  end

  assert(true == mf.acall(far.AdvControl, "ACTL_WAITKEY", nil, "F1"))
  Keys("F1")
  assert(true == mf.acall(far.AdvControl, "ACTL_WAITKEY"))
  Keys("F2")
end

local function test_AdvControl()
  test_AdvControl_Window()
  test_AdvControl_Colors()
  test_AdvControl_Synchro()
  test_AdvControl_Misc()
end

local function test_far_GetMsg()
  assert(type(far.GetMsg(0))=="string")
end

local function test_clipboard()
  local orig = far.PasteFromClipboard()
  local values = { nil, "foo", "", n=3 }
  for k=1,values.n do
    local v = values[k]
    far.CopyToClipboard(v)
    assert(far.PasteFromClipboard() == v)
  end
  far.CopyToClipboard(orig)
  assert(far.PasteFromClipboard() == orig)
end

local function test_far_FarClock()
  local temp = far.FarClock()
  mf.sleep(50)
  temp = far.FarClock() - temp
  assert(temp > 40000 and temp < 70000)
end

local function test_FarStandardFunctions()
  test_clipboard()
  test_far_FarClock()

  assert(far.ConvertPath([[c:\foo\bar\..\..\abc]], "CPM_FULL") == [[c:\abc]])

  assert(far.FormatFileSize(123456, 8)  == "  123456")
  assert(far.FormatFileSize(123456, -8) == "123456  ")

  assert(type(far.GetCurrentDirectory()) == "string")

  assert(far.GetPathRoot[[D:\foo\bar]] == [[D:\]])

  assert(far.LIsAlpha("A") == true)
  assert(far.LIsAlpha("Я") == true)
  assert(far.LIsAlpha("7") == false)
  assert(far.LIsAlpha(";") == false)

  assert(far.LIsAlphanum("A") == true)
  assert(far.LIsAlphanum("Я") == true)
  assert(far.LIsAlphanum("7") == true)
  assert(far.LIsAlphanum(";") == false)

  assert(far.LIsLower("A") == false)
  assert(far.LIsLower("a") == true)
  assert(far.LIsLower("Я") == false)
  assert(far.LIsLower("я") == true)
  assert(far.LIsLower("7") == false)
  assert(far.LIsLower(";") == false)

  assert(far.LIsUpper("A") == true)
  assert(far.LIsUpper("a") == false)
  assert(far.LIsUpper("Я") == true)
  assert(far.LIsUpper("я") == false)
  assert(far.LIsUpper("7") == false)
  assert(far.LIsUpper(";") == false)

  assert(far.LLowerBuf("abc-ABC-эюя-ЭЮЯ-7;") == "abc-abc-эюя-эюя-7;")
  assert(far.LUpperBuf("abc-ABC-эюя-ЭЮЯ-7;") == "ABC-ABC-ЭЮЯ-ЭЮЯ-7;")

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
  local fname = (win.GetEnv("TEMP") or ".").."\\far3-"..win.Uuid(win.Uuid()):sub(1,8)
  local fp = assert(io.open(fname, "w"))
  fp:close()
  local flags = {EF_NONMODAL=1, EF_IMMEDIATERETURN=1, EF_DISABLEHISTORY=1}
  assert(editor.Editor(fname,nil,nil,nil,nil,nil,flags) == F.EEC_MODIFIED)
  for k=1,3 do
    editor.InsertString()
    editor.SetString(nil, k, "foo")
  end
  assert(editor.SaveFile())
  assert(editor.Quit())
  local fp = assert(io.open(fname))
  local k = 0
  for line in fp:lines() do
    k = k + 1
    assert(line=="foo")
  end
  fp:close()
  win.DeleteFile(fname)
  assert(k == 3)
end

function MT.test_luafar()
  test_AdvControl()
  test_bit64()
  test_far_GetMsg()
  test_FarStandardFunctions()
  test_MacroControl()
  test_RegexControl()
  test_issue_3129()
end

-- Test in particular that Plugin.Call (a so-called "restricted" function) works properly
-- from inside a deeply nested coroutine.
local function test_coroutine()
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

local function test_cfind()
  assert(type(("").cfind) == "function")
  assert(("").cfind == unicode.utf8.cfind)
  local from, to, c1, c2 = ("абвгд"):cfind("(г)(д)", 4)
  assert(from==4 and to==5 and c1=="г" and c2=="д")
  assert(nil == ("абвгд"):cfind("(г)(д)", 5))
end

function MT.test_misc()
  test_coroutine()
  test_cfind()
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
  MT.test_Mouse()
  MT.test_Object()
  MT.test_Panel()
  MT.test_Plugin()
  MT.test_XPanel(APanel)
  MT.test_XPanel(PPanel)
  MT.test_mantis_1722()
  MT.test_luafar()
  MT.test_misc()
end

return MT
