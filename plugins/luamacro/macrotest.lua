-- Started: 2012-08-20.

local luamacroId="4ebbefc8-2084-4b7f-94c0-692ce136894d" -- LuaMacro plugin GUID

local function IsNumOrInt(v)
  return type(v)=="number" or bit64.type(v)
end

local function TestArea (area, msg)
  assert(Area[area]==true and Area.Current==area, msg or "assertion failed")
end

local function test_areas()
  Keys "AltIns"              TestArea "Other"      Keys "Esc"
  Keys "ShiftF4 CtrlY Enter" TestArea "Editor"     Keys "Esc"
  Keys "F7"                  TestArea "Dialog"     Keys "Esc"
  Keys "Alt?"                TestArea "Search"     Keys "Esc"
  Keys "AltF1"               TestArea "Disks"      Keys "Esc"
  Keys "AltF2"               TestArea "Disks"      Keys "Esc"
  Keys "F9"                  TestArea "MainMenu"   Keys "Esc"
  Keys "F12"                 TestArea "Menu"       Keys "Esc"
  Keys "F1"                  TestArea "Help"       Keys "Esc"
  Keys "CtrlL Tab"           TestArea "Info"       Keys "Tab CtrlL"
  Keys "CtrlQ Tab"           TestArea "QView"      Keys "Tab CtrlQ"
  Keys "CtrlT Tab"           TestArea "Tree"       Keys "Tab CtrlT"
  Keys "AltF10"              TestArea "FindFolder" Keys "Esc"
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
  local k0,k1 = akey(0),akey(1)
  assert(k0==0x0501007B and k1=="CtrlShiftF12" or
         k0==0x1401007B and k1=="RCtrlShiftF12")
  --todo: test 2nd parameter
end

local function test_bit64()
  assert(band(0xFF,0xFE,0xFD) == 0xFC)
  assert(bor(1,2,4) == 7)
  assert(bnot(5) == -6)
  assert(bxor(0x01,0xF0,0xAA) == 0x5B)
end

local function test_eval()
  temp=nil
  assert(eval("temp=5+7")==0)
  assert(temp==12)

  temp=nil
  assert(eval("temp=5+7",1)==0)
  assert(temp==nil)

  mf.postmacro(function() assert(Area.Dialog); Keys("Esc") end) -- test for error message box
  assert(eval("5+7")==11)
end

local function test_abs()
  assert(mf.abs(1.3)==1.3)
  assert(mf.abs(-1.3)==1.3)
  assert(mf.abs(0)==0)
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
  mf.clip(5,2) -- включить внутренний буфер обмена
  assert(mf.clip(5,-1)==2)
  assert(mf.clip(5,1)==2) -- включить буфер обмена OS
  assert(mf.clip(5,-1)==1)
  for clipnum=2,1,-1 do -- в конце оставляет включенным буфер обмена OS
    mf.clip(5,clipnum)
    local str = "foo"..clipnum
    assert(mf.clip(1,str) ~= 0)
    assert(mf.clip(0,str) == str)
  end
end

local function test_env()
  mf.env("Foo",1,"Bar")
  assert(mf.env("Foo")=="Bar")
  mf.env("Foo",1,"")
  assert(mf.env("Foo")=="")
end

local function test_fattr()
  local fname=assert(win.GetEnv"tmp" or win.GetEnv"temp").."\\tmp.tmp"
  assert(io.open(fname,"w")):close()
  local attr = mf.fattr(fname)
  assert(win.DeleteFile(fname))
  assert(type(attr)=="number")
end

local function test_fexist()
  local fname=assert(win.GetEnv"tmp" or win.GetEnv"temp").."\\tmp.tmp"
  assert(io.open(fname,"w")):close()
  assert(mf.fexist(fname))
  assert(win.DeleteFile(fname))
  assert(not mf.fexist(fname))
end

local function test_msgbox()
  mf.postmacro(function() Keys("Esc") end)
  assert(0 == msgbox("title","message"))
  mf.postmacro(function() Keys("Enter") end)
  assert(1 == msgbox("title","message"))
end

local function test_prompt()
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
  assert(mf.key(83951739)=="CtrlShiftF12")
  assert(mf.key("CtrlShiftF12")=="CtrlShiftF12")
  assert(mf.key("foobar")=="")
end

local function test_float()
  assert(mf.float("2.56e1")==25.6)
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
  mf.msave("macrotest", "testkey", "foo")
  assert(mf.mload("macrotest", "testkey")=="foo")
  mf.mdelete("macrotest", "*")
  assert(mf.mload("macrotest", "testkey")==nil)

  mf.msave("macrotest", "testkey", { a=5, {b=6}, c={d=7} })
  local t=mf.mload("macrotest", "testkey")
  assert(t.a==5 and t[1].b==6 and t.c.d==7)
  mf.mdelete("macrotest", "*")
  assert(mf.mload("macrotest", "testkey")==nil)
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

local function test_string()
  assert(mf.string(-5.7)=="-5.7")
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
  -- закомментировано, т.к. эти тесты будут работать не на любой конфигурации.
  --assert(mf.xlat("ghzybr")=="пряник")
  --assert(mf.xlat("сщьзгеук")=="computer")
end

local function test_beep()
  assert(type(mf.beep())=="boolean")
end

local function test_CmdLine()
  Keys"Esc f o o Space Б а р"
  assert(CmdLine.Value=="foo Бар")
  assert(not CmdLine.Selected)
  Keys"SelWord"
  assert(CmdLine.Selected)
  Keys"Esc"
  assert(CmdLine.Value=="")

  Keys"Esc"
  print("foo Бар")
  assert(CmdLine.Value=="foo Бар")
  Keys"Esc"

  Keys"Esc"
  printf("%s %d %s", "foo", 5+7, "Бар")
  assert(CmdLine.Value=="foo 12 Бар")
  Keys"Esc"
end

local function test_flock()
  assert(type(mf.flock(0,-1))=="number")
end

local function test_mf()
  test_abs()
  test_akey()
  test_asc()
  test_atoi()
  test_beep()
  test_bit64()
  test_chr()
  test_clip()
  test_date()
  test_env()
  test_eval()
  test_fattr()
  test_fexist()
  test_float()
  test_flock()
  test_fmatch()
  test_fsplit()
  test_iif()
  test_index()
  test_int()
  test_itoa()
  test_key()
  test_lcase()
  test_len()
  test_max()
  test_min()
  test_mod()
  test_msave()
  test_msgbox()
  test_prompt()
  test_replace()
  test_rindex()
  test_size2str()
  test_string()
  test_strpad()
  test_strwrap()
  test_substr()
  test_testfolder()
  test_trim()
  test_ucase()
  test_waitkey()
  test_xlat()
end

local function test_Far()
  local temp = Far.UpTime
  mf.sleep(50)
  temp = Far.UpTime - temp
  assert(temp > 40 and temp < 70)

  assert(type(Far.KbdLayout(0))=="number")
  assert(type(Far.KeyBar_Show(0))=="number")
  assert(type(Far.Window_Scroll)=="function")
end

local function test_XPanel(pan) -- (@pan: either APanel or PPanel)
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

local function test_Panel()
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

local function test_Plugin() -- Plugin.Call, Plugin.SyncCall: test arguments and returns
  local function test (func, N)
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
  local t = far.MacroExecute("return ...", nil,
    "foo",
    false,
    5,
    nil,
    bit64.new("0x8765876587658765"),
    {"bar"})
  assert(
    type(t) == "table"  and
    t.n  == 6           and
    t[1] == "foo"       and
    t[2] == false       and
    t[3] == 5           and
    t[4] == nil         and
    t[5] == bit64.new("0x8765876587658765") and
    type(t[6])=="table" and t[6][1]=="bar")
end

TestArea("Shell", "Run these tests from the Shell area.")
assert(not APanel.Plugin and not PPanel.Plugin, "Run these tests when neither of panels is a plugin panel.")

test_areas()
test_mf()
test_CmdLine()
test_Far()
test_Panel()
test_Plugin()
test_XPanel(APanel)
test_XPanel(PPanel)
test_far_MacroExecute()

far.Message("All tests OK", "LuaMacro")
