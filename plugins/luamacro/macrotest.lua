-- start: 2012-08-20

assert(Area.Shell and not Area.Help)
assert(Area.Current=="Shell")

Keys"F1"
assert(Area.Help and not Area.Shell)
assert(Area.Current=="Help")
Keys"Esc"

Keys"F7"
assert(Area.Dialog)
assert(Area.Current=="Dialog")
Keys"Esc"

assert(akey(0)==16842875, akey(0))
assert(akey(1)=="CtrlF12")
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
printf("%s %d %s", "foo", 5+7, "Бар")
assert(CmdLine.Value=="foo 12 Бар")

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

assert(type(mf.beep)=="function")

assert(mf.chr(48)=="0")
assert(mf.chr(1071)=="Я")

mf.clip(5,2) -- включить внутренний буфер обмена
assert(mf.clip(5,-1)==2)
assert(mf.clip(5,1)==2) -- включить буфер обмена OS
assert(mf.clip(5,-1)==1)
for clipnum=1,2 do
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

msgbox("LuaMacro", "All tests OK")
