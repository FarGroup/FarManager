-- Started: 2012-08-20.

assert(not (APanel.Plugin or PPanel.Plugin),
  "Run these tests when neither of panels is a plugin panel.")

assert(Area.Shell and not Area.Help, "Run these tests from the Shell area.")
assert(Area.Current=="Shell")

Keys"F1"
assert(Area.Help and not Area.Shell)
assert(Area.Current=="Help")
Keys"Esc"

Keys"F7"
assert(Area.Dialog)
assert(Area.Current=="Dialog")
Keys"Esc"

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

assert(akey(0)==83951739)
assert(akey(1)=="CtrlShiftF12")
--todo: test 2nd parameter

assert(band(0xFF,0xFE,0xFD) == 0xFC)
assert(bor(1,2,4) == 7)
assert(bnot(5) == -6)
assert(bxor(0x01,0xF0,0xAA) == 0x5B)

temp=nil
assert(eval("temp=5+7")==0)
assert(temp==12)

temp=nil
assert(eval("temp=5+7",1)==0)
assert(temp==nil)

--assert(eval("5+7")==11) -- todo: suppress error message

Keys"Esc f o o Space Б а р"
assert(CmdLine.Value=="foo Бар")
assert(not CmdLine.Selected)
Keys"SelWord"
assert(CmdLine.Selected)
Keys"Esc"
assert(CmdLine.Value=="")

assert(type(msgbox)=="function")

Keys"Esc"
print("foo Бар")
assert(CmdLine.Value=="foo Бар")
Keys"Esc"

Keys"Esc"
printf("%s %d %s", "foo", 5+7, "Бар")
assert(CmdLine.Value=="foo 12 Бар")
Keys"Esc"

assert(type(prompt)=="function")

assert(mf.abs(1.3)==1.3)
assert(mf.abs(-1.3)==1.3)
assert(mf.abs(0)==0)

assert(mf.asc("0")==48)
assert(mf.asc("Я")==1071)

assert(mf.atoi("0")==bit64.new(0))
assert(mf.atoi("-10")==bit64.new(-10))
assert(mf.atoi("0x11")==bit64.new(17))
assert(mf.atoi("1011",2)==bit64.new(11))
assert(mf.atoi("123456789123456789")==bit64.new("123456789123456789"))
assert(mf.atoi("-123456789123456789")==bit64.new("-123456789123456789"))
assert(mf.atoi("0x1B69B4BACD05F15")==bit64.new("0x1B69B4BACD05F15"))
assert(mf.atoi("-0x1B69B4BACD05F15")==bit64.new("-0x1B69B4BACD05F15"))

assert(type(mf.beep)=="function")

assert(mf.chr(48)=="0")
assert(mf.chr(1071)=="Я")

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

assert(type(mf.date())=="string")
assert(type(mf.date("%a"))=="string")

mf.env("Foo",1,"Bar")
assert(mf.env("Foo")=="Bar")
mf.env("Foo",1,"")
assert(mf.env("Foo")=="")

local fname=assert(win.GetEnv"tmp" or win.GetEnv"temp").."\\tmp.tmp"
assert(io.open(fname,"w")):close()
local attr = mf.fattr(fname)
assert(win.DeleteFile(fname))
assert(type(attr)=="number")

local fname=assert(win.GetEnv"tmp" or win.GetEnv"temp").."\\tmp.tmp"
assert(io.open(fname,"w")):close()
assert(mf.fexist(fname))
assert(win.DeleteFile(fname))
assert(not mf.fexist(fname))

assert(mf.float("2.56e1")==25.6)

assert(type(mf.flock)=="function")

assert(mf.fmatch("Readme.txt", "*.txt") == 1)
assert(mf.fmatch("Readme.txt", "Readme.*|*.txt") == 0)
assert(mf.fmatch("c:\Readme.txt", "/txt$/i") == 1)
assert(mf.fmatch("c:\Readme.txt", "/txt$") == -1)

local path="C:\\Program Files\\Far\\Far.exe"
assert(mf.fsplit(path,1)=="C:\\")
assert(mf.fsplit(path,2)=="\\Program Files\\Far\\")
assert(mf.fsplit(path,4)=="Far")
assert(mf.fsplit(path,8)==".exe")

assert(mf.iif(true,  1, 2)==1)
assert(mf.iif(false, 1, 2)==2)
assert(mf.iif("a",   1, 2)==1)
assert(mf.iif("",    1, 2)==2)

assert(mf.index("language","gua",0)==3)
assert(mf.index("language","gua",1)==3)
assert(mf.index("language","gUA",1)==-1)
assert(mf.index("language","gUA",0)==3)

assert(mf.int("2.99")==bit64.new(2))
assert(mf.int("-2.99")==bit64.new(-2))
assert(mf.int("0x10")==bit64.new(0))
assert(mf.int("123456789123456789")==bit64.new("123456789123456789"))
assert(mf.int("-123456789123456789")==bit64.new("-123456789123456789"))

assert(mf.itoa(100)=="100")
assert(mf.itoa(100,10)=="100")
assert(mf.itoa(bit64.new("123456789123456789"))=="123456789123456789")
assert(mf.itoa(bit64.new("-123456789123456789"))=="-123456789123456789")
assert(mf.itoa(100,2)=="1100100")
assert(mf.itoa(100,16)=="64")
assert(mf.itoa(100,36)=="2s")

assert(mf.key(83951739)=="CtrlShiftF12")
assert(mf.key("CtrlShiftF12")=="CtrlShiftF12")
assert(mf.key("foobar")=="")

assert(mf.lcase("FOo БАр")=="foo бар")

assert(mf.len("FOo БАр")==7)

assert(mf.max(-2,-5)==-2)
assert(mf.max(2,5)==5)

assert(mf.min(-2,-5)==-5)
assert(mf.min(2,5)==2)

mf.msave("macrotest", "testkey", "foo")
assert(mf.mload("macrotest", "testkey")=="foo")
mf.mdelete("macrotest", "*")
assert(mf.mload("macrotest", "testkey")==nil)

assert(mf.mod(11,4)==3)

assert(mf.replace("Foo Бар", "o", "1")=="F11 Бар")
assert(mf.replace("Foo Бар", "o", "1", 1)=="F1o Бар")
assert(mf.replace("Foo Бар", "O", "1", 1, 1)=="Foo Бар")
assert(mf.replace("Foo Бар", "O", "1", 1, 0)=="F1o Бар")

assert(mf.rindex("language","a",0)==5)
assert(mf.rindex("language","a",1)==5)
assert(mf.rindex("language","A",1)==-1)
assert(mf.rindex("language","A",0)==5)

do
  -- flags
  local sfx=bit64.new"0x0010000000000000"
  local dlm=bit64.new"0x0800000000000000"
  local exp=bit64.new"0x0080000000000000"
  local eco=bit64.new"0x0040000000000000"
  local dec=bit64.new"0x0400000000000000"
  local min=bit64.new"0x0020000000000000"

  assert(mf.size2str(123,0,5)=="  123")
  assert(mf.size2str(123,0,-5)=="123  ")
end

local temp=os.clock()
mf.sleep(50)
assert(os.clock()-temp > 0.04)

assert(mf.string(-5.7)=="-5.7")

assert(mf.strpad("Foo",10,"*",  2) == '***Foo****')
assert(mf.strpad("",   10,"-*-",2) == '-*--*--*--')
assert(mf.strpad("",   10,"-*-")   == '-*--*--*--')
assert(mf.strpad("Foo",10)         == 'Foo       ')
assert(mf.strpad("Foo",10,"-")     == 'Foo-------')
assert(mf.strpad("Foo",10," ",  1) == '       Foo')
assert(mf.strpad("Foo",10," ",  2) == '   Foo    ')
assert(mf.strpad("Foo",10,"1234567890",2) == '123Foo1234')

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

assert(mf.testfolder(".") > 0)
assert(mf.testfolder("C:\\") == 2)
assert(mf.testfolder("@:\\") <= 0)

assert(mf.trim(" abc ")=="abc")
assert(mf.trim(" abc ",0)=="abc")
assert(mf.trim(" abc ",1)=="abc ")
assert(mf.trim(" abc ",2)==" abc")

assert(mf.ucase("FOo БАр")=="FOO БАР")

assert(mf.waitkey(50,0)=="")
assert(mf.waitkey(50,1)==0xFFFFFFFF)

assert(type(mf.xlat)=="function")
-- закомментировано, т.к. эти тесты будут работать не на любой конфигурации.
--assert(mf.xlat("ghzybr")=="пряник")
--assert(mf.xlat("сщьзгеук")=="computer")

for _,pan in ipairs{APanel,PPanel} do
  assert(type(pan.Bof)         =="boolean")
  assert(type(pan.ColumnCount) =="number")
  assert(type(pan.CurPos)      =="number")
  assert(type(pan.Current)     =="string")
  assert(type(pan.DriveType)   =="number")
  assert(type(pan.Empty)       =="boolean")
  assert(type(pan.Eof)         =="boolean")
  assert(type(pan.FilePanel)   =="boolean")
  assert(type(pan.Filter)      =="boolean")
  assert(type(pan.Folder)      =="boolean")
  assert(type(pan.Format)      =="string")
  assert(type(pan.Height)      =="number")
  assert(type(pan.HostFile)    =="string")
  assert(type(pan.ItemCount)   =="number")
  assert(type(pan.Left)        =="boolean")
  assert(type(pan.LFN)         =="boolean")
  assert(type(pan.OPIFlags)    =="number")
  assert(type(pan.Path)        =="string")
  assert(type(pan.Path0)       =="string")
  assert(type(pan.Plugin)      =="boolean")
  assert(type(pan.Prefix)      =="string")
  assert(type(pan.Root)        =="boolean")
  assert(type(pan.SelCount)    =="number")
  assert(type(pan.Selected)    =="boolean")
  assert(type(pan.Type)        =="number")
  assert(type(pan.UNCPath)     =="string")
  assert(type(pan.Visible)     =="boolean")
  assert(type(pan.Width)       =="number")
end

assert(Panel.FAttr(0,":")==-1)
assert(Panel.FAttr(1,":")==-1)

assert(Panel.FExist(0,":")==0)
assert(Panel.FExist(1,":")==0)

for pt=0,1 do
  assert(type(Panel.Item(pt,0,0))  =="string")
  assert(type(Panel.Item(pt,0,1))  =="string")
  assert(type(Panel.Item(pt,0,2))  =="number")
  assert(type(Panel.Item(pt,0,3))  =="string")
  assert(type(Panel.Item(pt,0,4))  =="string")
  assert(type(Panel.Item(pt,0,5))  =="string")
  assert(bit64.type(Panel.Item(pt,0,6)))
  assert(bit64.type(Panel.Item(pt,0,7)))
  assert(type(Panel.Item(pt,0,8))  =="number")
  assert(type(Panel.Item(pt,0,9))  =="number")
  assert(type(Panel.Item(pt,0,10)) =="number")
  assert(type(Panel.Item(pt,0,11)) =="string")
  assert(type(Panel.Item(pt,0,12)) =="string")
  assert(type(Panel.Item(pt,0,13)) =="number")
  assert(type(Panel.Item(pt,0,14)) =="number")
  assert(bit64.type(Panel.Item(pt,0,15)))
  assert(bit64.type(Panel.Item(pt,0,16)))
  assert(bit64.type(Panel.Item(pt,0,17)))
  assert(type(Panel.Item(pt,0,18)) =="number")
  assert(bit64.type(Panel.Item(pt,0,19)))
  assert(type(Panel.Item(pt,0,20)) =="string")
  assert(bit64.type(Panel.Item(pt,0,21)))
  assert(type(Panel.Item(pt,0,22)) =="string")
  assert(type(Panel.Item(pt,0,23)) =="number")
end

assert(type(Panel.Select)=="function")

assert(type(Panel.SetPath)=="function")

assert(type(Panel.SetPos)=="function")

assert(type(Panel.SetPosIdx)=="function")

assert(type(Far.KbdLayout(0))=="number")

assert(type(Far.KeyBar_Show(0))=="number")

assert(type(Far.Window_Scroll)=="function")

msgbox("LuaMacro", "All tests OK")
