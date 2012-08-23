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

assert(mf.atoi("0")==0)
assert(mf.atoi("-10")==-10)
assert(mf.atoi("0x11")==17)
assert(mf.atoi("1011",2)==11)
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

assert(mf.int("2.99")==2)
assert(mf.int("-2.99")==-2)
assert(mf.int("0x10")==0)
assert(mf.int("123456789123456789")==bit64.new("123456789123456789"))
assert(mf.int("-123456789123456789")==bit64.new("-123456789123456789"))

assert(mf.itoa(100)=="100")
assert(mf.itoa(100,10)=="100")
assert(mf.itoa(bit64.new("123456789123456789"))=="123456789123456789")
assert(mf.itoa(bit64.new("-123456789123456789"))=="-123456789123456789")
assert(mf.itoa(100,2)=="1100100")
assert(mf.itoa(100,16)=="64")
assert(mf.itoa(100,36)=="2s")

assert(type(mf.kbdLayout(0))=="number")

assert(mf.key(83951739)=="CtrlShiftF12")
assert(mf.key("CtrlShiftF12")=="CtrlShiftF12")
assert(mf.key("foobar")=="")

assert(type(mf.KeyBar_Show(0))=="number")

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

assert(type(mf.Window_Scroll)=="function")

assert(type(mf.xlat)=="function")
-- закомментировано, т.к. эти тесты будут работать не на любой конфигурации.
--assert(mf.xlat("ghzybr")=="пряник")
--assert(mf.xlat("сщьзгеук")=="computer")

assert(type(APanel.Bof)         =="boolean")
assert(type(APanel.ColumnCount) =="number")
assert(type(APanel.CurPos)      =="number")
assert(type(APanel.Current)     =="string")
assert(type(APanel.DriveType)   =="number")
assert(type(APanel.Empty)       =="boolean")
assert(type(APanel.Eof)         =="boolean")
assert(type(APanel.FilePanel)   =="boolean")
assert(type(APanel.Filter)      =="boolean")
assert(type(APanel.Folder)      =="boolean")
assert(type(APanel.Format)      =="string")
assert(type(APanel.Height)      =="number")
assert(type(APanel.HostFile)    =="string")
assert(type(APanel.ItemCount)   =="number")
assert(type(APanel.Left)        =="boolean")
assert(type(APanel.LFN)         =="boolean")
assert(type(APanel.OPIFlags)    =="number")
assert(type(APanel.Path)        =="string")
assert(type(APanel.Path0)       =="string")
assert(type(APanel.Plugin)      =="boolean")
assert(type(APanel.Prefix)      =="string")
assert(type(APanel.Root)        =="boolean")
assert(type(APanel.SelCount)    =="number")
assert(type(APanel.Selected)    =="boolean")
assert(type(APanel.Type)        =="number")
assert(type(APanel.UNCPath)     =="string")
assert(type(APanel.Visible)     =="boolean")
assert(type(APanel.Width)       =="number")

assert(type(PPanel.Bof)         =="boolean")
assert(type(PPanel.ColumnCount) =="number")
assert(type(PPanel.CurPos)      =="number")
assert(type(PPanel.Current)     =="string")
assert(type(PPanel.DriveType)   =="number")
assert(type(PPanel.Empty)       =="boolean")
assert(type(PPanel.Eof)         =="boolean")
assert(type(PPanel.FilePanel)   =="boolean")
assert(type(PPanel.Filter)      =="boolean")
assert(type(PPanel.Folder)      =="boolean")
assert(type(PPanel.Format)      =="string")
assert(type(PPanel.Height)      =="number")
assert(type(PPanel.HostFile)    =="string")
assert(type(PPanel.ItemCount)   =="number")
assert(type(PPanel.Left)        =="boolean")
assert(type(PPanel.LFN)         =="boolean")
assert(type(PPanel.OPIFlags)    =="number")
assert(type(PPanel.Path)        =="string")
assert(type(PPanel.Path0)       =="string")
assert(type(PPanel.Plugin)      =="boolean")
assert(type(PPanel.Prefix)      =="string")
assert(type(PPanel.Root)        =="boolean")
assert(type(PPanel.SelCount)    =="number")
assert(type(PPanel.Selected)    =="boolean")
assert(type(PPanel.Type)        =="number")
assert(type(PPanel.UNCPath)     =="string")
assert(type(PPanel.Visible)     =="boolean")
assert(type(PPanel.Width)       =="number")

msgbox("LuaMacro", "All tests OK")
