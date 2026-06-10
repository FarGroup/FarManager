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
    local f = assert(loadfile(far.PluginStartupInfo().ModuleDir.."macrotest.lua"))
    local mod = setfenv(f, getfenv())()
    mod.SetMacroKeys(MacroKey)
    mod.test_all()
    far.Message("All tests OK", "LuaMacro")
  end;
}
--]]

local MT = {} -- "macrotest", this module

local AF = "my assertion failed"
local asrt = {}

function asrt.eq(a,b,m)      assert(a == b, m or AF)               return true; end
function asrt.neq(a,b,m)     assert(a ~= b, m or AF)               return true; end
function asrt.lt(a,b,m)      assert(a < b,  m or AF)               return true; end
function asrt.gt(a,b,m)      assert(a > b,  m or AF)               return true; end
function asrt.lte(a,b,m)     assert(a <= b, m or AF)               return true; end
function asrt.gte(a,b,m)     assert(a >= b, m or AF)               return true; end
function asrt.num(v,m)       assert(type(v)=="number", m or AF)    return v; end
function asrt.str(v,m)       assert(type(v)=="string", m or AF)    return v; end
function asrt.table(v,m)     assert(type(v)=="table", m or AF)     return v; end
function asrt.bool(v,m)      assert(type(v)=="boolean", m or AF)   return true; end
function asrt.func(v,m)      assert(type(v)=="function", m or AF)  return v; end
function asrt.udata(v,m)     assert(type(v)=="userdata", m or AF)  return v; end
function asrt.isnil(v,m)     assert(v==nil, m or AF)               return true; end
function asrt.isfalse(v,m)   assert(v==false, m or AF)             return true; end
function asrt.istrue(v,m)    assert(v==true, m or AF)              return true; end
function asrt.err(...)       assert(pcall(...)==false, AF)         return true; end
function asrt.noerr(...)     assert(pcall(...)==true, AF)          return true; end

function asrt.range(v, low, high)
  if low then assert(v >= low, v) end
  if high then assert(v <= high, v) end
  return v
end

function asrt.numint(v,m)
  assert(type(v)=="number" or bit64.type(v), m or AF)
  return v
end

local MacroKeys -- keys that invoke the whole macrotest from a macro
local F = far.Flags
local band, bor, bnot = bit64.band, bit64.bor, bit64.bnot
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

local TmpFileName = win.JoinPath(assert(win.GetEnv"tmp" or win.GetEnv"temp"), "tmp.tmp")

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

local function test_bit64()
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
    for k=1,2 do
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

function MT.test_Menu()
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

local function test_XPanel(pan) -- (@pan: either APanel or PPanel)
  asrt.bool (pan.Bof)
  asrt.num  (pan.ColumnCount)
  asrt.num  (pan.CurPos)
  asrt.str  (pan.Current)
  asrt.num  (pan.DriveType)
  asrt.bool (pan.Empty)
  asrt.bool (pan.Eof)
  asrt.bool (pan.FilePanel)
  asrt.bool (pan.Filter)
  asrt.bool (pan.Folder)
  asrt.str  (pan.Format)
  asrt.num  (pan.Height)
  asrt.str  (pan.HostFile)
  asrt.num  (pan.ItemCount)
  asrt.bool (pan.Left)
  asrt.bool (pan.LFN)
  asrt.num  (pan.OPIFlags)
  asrt.str  (pan.Path)
  asrt.str  (pan.Path0)
  asrt.bool (pan.Plugin)
  asrt.str  (pan.Prefix)
  asrt.bool (pan.Root)
  asrt.num  (pan.SelCount)
  asrt.bool (pan.Selected)
  asrt.num  (pan.Type)
  asrt.str  (pan.UNCPath)
  asrt.bool (pan.Visible)
  asrt.num  (pan.Width)

  if pan == APanel then
    Keys "End"  asrt.istrue(pan.Eof)
    Keys "Home" asrt.istrue(pan.Bof)
  end
end

MT.test_APanel = function() test_XPanel(APanel) end
MT.test_PPanel = function() test_XPanel(PPanel) end

local function test_Panel_Item()
  local index = 0 -- 0 is the current element, otherwise element index
  for pan=0,1 do
    asrt.str    (Panel.Item(pan,index,0))  -- file name
    asrt.str    (Panel.Item(pan,index,1))  -- short file name
    asrt.num    (Panel.Item(pan,index,2))  -- file attributes
    asrt.str    (Panel.Item(pan,index,3))  -- creation time
    asrt.str    (Panel.Item(pan,index,4))  -- last access time
    asrt.str    (Panel.Item(pan,index,5))  -- modification time
    asrt.numint (Panel.Item(pan,index,6))  -- size
    asrt.numint (Panel.Item(pan,index,7))  -- packed size
    asrt.bool   (Panel.Item(pan,index,8))  -- selected
    asrt.num    (Panel.Item(pan,index,9))  -- number of links
    asrt.num    (Panel.Item(pan,index,10)) -- sort group
    asrt.str    (Panel.Item(pan,index,11)) -- diz text
    asrt.str    (Panel.Item(pan,index,12)) -- owner
    asrt.num    (Panel.Item(pan,index,13)) -- crc32
    asrt.num    (Panel.Item(pan,index,14)) -- position when read from the file system
    asrt.numint (Panel.Item(pan,index,15)) -- creation time
    asrt.numint (Panel.Item(pan,index,16)) -- last access time
    asrt.numint (Panel.Item(pan,index,17)) -- modification time
    asrt.num    (Panel.Item(pan,index,18)) -- number of streams
    asrt.numint (Panel.Item(pan,index,19)) -- size of streams
    asrt.str    (Panel.Item(pan,index,20)) -- change time
    asrt.numint (Panel.Item(pan,index,21)) -- change time
    asrt.isfalse(pcall(Panel.Item,pan,index,22))
    asrt.num    (Panel.Item(pan,index,23))
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
  asrt.istrue(Panel.SetPath(1, pdir))
  asrt.istrue(Panel.SetPath(0, adir, afile))
  asrt.eq (pdir, panel.GetPanelDirectory(nil,0).Name:lower())
  asrt.eq (adir, panel.GetPanelDirectory(nil,1).Name:lower())
  asrt.eq (panel.GetCurrentPanelItem(nil,1).FileName:lower(), afile)
  -- restore
  asrt.istrue(Panel.SetPath(1, pdir_old))
  asrt.istrue(Panel.SetPath(0, adir_old))
end

-- N=Panel.Select(panelType,Action[,Mode[,Items]])
local function Test_Panel_Select()
  local adir_old = panel.GetPanelDirectory(nil,1) -- store active panel directory

  local PS = asrt.func(Panel.Select)
  local RM,ADD,INV,RST = 0,1,2,3 -- Action
  local MODE

  local dir = asrt.str(win.GetEnv("FARHOME"))
  asrt.istrue(panel.SetPanelDirectory(nil,1,dir))
  local pi = asrt.table(panel.GetPanelInfo(1))
  local ItemsCount = asrt.num(pi.ItemsNumber)-1 -- don't count ".."
  assert(ItemsCount>=10, "not enough files to test")

  --------------------------------------------------------------
  MODE = 0
  asrt.eq(ItemsCount,PS(0,ADD,MODE)) -- select all
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(ItemsCount, pi.SelectedItemsNumber)

  asrt.eq(ItemsCount,PS(0,RM,MODE)) -- clear all
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(ItemsCount,PS(0,INV,MODE)) -- invert
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(ItemsCount, pi.SelectedItemsNumber)

  asrt.eq(0,PS(0,INV,MODE)) -- invert again (return value is the selection count, contrary to docs)
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 1
  asrt.eq(1,PS(0,ADD,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(1, pi.SelectedItemsNumber)

  asrt.eq(1,PS(0,RM,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(1,PS(0,INV,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(1, pi.SelectedItemsNumber)

  asrt.eq(1,PS(0,INV,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 2
  local list = win.JoinPath(dir,"FarEng.hlf").."\nFarEng.lng" -- the 1-st file with path, the 2-nd without
  asrt.eq(2,PS(0,ADD,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(2, pi.SelectedItemsNumber)

  asrt.eq(2,PS(0,RM,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(2,PS(0,INV,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(2, pi.SelectedItemsNumber)

  asrt.eq(2,PS(0,INV,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 3
  local mask = "*.hlf;*.lng"
  local count = 0
  for i=1,pi.ItemsNumber do
    local item = asrt.table(panel.GetPanelItem(nil,1,i))
    if far.CmpNameList(mask, item.FileName) then count=count+1 end
  end
  assert(count>1, "not enough files to test")

  asrt.eq(count,PS(0,ADD,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(count, pi.SelectedItemsNumber)

  asrt.eq(count,PS(0,RM,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(count,PS(0,INV,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(count, pi.SelectedItemsNumber)

  asrt.eq(count,PS(0,INV,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  panel.SetPanelDirectory(nil,1,adir_old) -- restore active panel directory
end

function MT.test_Panel()
  test_Panel_Item()

  asrt.eq (Panel.FAttr(0,":"), -1)
  asrt.eq (Panel.FAttr(1,":"), -1)

  asrt.eq (Panel.FExist(0,":"), 0)
  asrt.eq (Panel.FExist(1,":"), 0)

  Test_Panel_Select()
  test_Panel_SetPath()
  asrt.func (Panel.SetPos)
  asrt.func (Panel.SetPosIdx)
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
  asrt.eq(c1, "8")
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
      asrt.eq(p1, 1)
    end
  end
  local Dlg = { {"DI_EDIT", 3,1,56,10, 0,0,0,0, "a"}, }
  mf.acall(far.Dialog, nil,-1,-1,60,3,"Contents",Dlg, 0, DlgProc)
  asrt.istrue(Area.Dialog)
  Keys("W 1 2 3 4 BS Esc")
  asrt.eq(check, 6)
  asrt.eq(Dlg[1][10], "W123")
end

local function test_utf8_len()
  asrt.eq ((""):len(), 0)
  asrt.eq (("FOo БАр"):len(), 7)
end

local function test_utf8_sub()
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

local function test_utf8_lower_upper()
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

local function test_AdvControl_Colors()
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

local function test_AdvControl_Synchro()
  local pass = 0
  local oldProcessSynchroEvent = export.ProcessSynchroEvent
  export.ProcessSynchroEvent =
    function(event,param)
      asrt.istrue(pass==0 and param==123 or pass==1 and param==-456)
      pass = pass + 1
    end
  far.AdvControl("ACTL_SYNCHRO", 123)
  far.AdvControl("ACTL_SYNCHRO", -456)
  for k=1,2 do
    mf.acall(far.Show); Keys"Esc"
  end
  export.ProcessSynchroEvent = oldProcessSynchroEvent
  asrt.eq(pass, 2)
end

local function test_AdvControl_Misc()
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

local function test_AdvControl()
  test_AdvControl_Window()
  test_AdvControl_Colors()
  test_AdvControl_Synchro()
  test_AdvControl_Misc()
end

local function test_far_GetMsg()
  asrt.str (far.GetMsg(0))
end

local function test_clipboard()
  local orig = far.PasteFromClipboard()
  local values = { nil, "foo", "", n=3 }
  for k=1,values.n do
    local v = values[k]
    far.CopyToClipboard(v)
    asrt.eq(far.PasteFromClipboard(), v)
  end
  if orig then far.CopyToClipboard(orig) end
  asrt.eq(far.PasteFromClipboard(), orig)
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
  asrt.istrue(OK)
end

local function test_ProcessName()
  asrt.istrue  (far.CheckMask("f*.ex?"))
  asrt.istrue  (far.CheckMask("/(abc)?def/"))
  asrt.isfalse (far.CheckMask("/[[[/"))

  asrt.eq    (far.GenerateName("a??b.*", "cdef.txt"),     "adeb.txt")
  asrt.eq    (far.GenerateName("a??b.*", "cdef.txt", 50), "adeb.txt")
  asrt.eq    (far.GenerateName("a??b.*", "cdef.txt", 2),  "adbef.txt")

  asrt.istrue  (far.CmpName("f*.ex?",      "ftp.exe"        ))
  asrt.istrue  (far.CmpName("f*.ex?",      "fc.exe"         ))
  asrt.istrue  (far.CmpName("f*.ex?",      "f.ext"          ))

  asrt.istrue  (far.CmpName("f*.ex?",      "FTP.exe"        ))

  asrt.isfalse (far.CmpName("f*.ex?",      "a/f.ext"        ))
  asrt.isfalse (far.CmpName("f*.ex?",      "a/f.ext", 0     ))
  asrt.istrue  (far.CmpName("f*.ex?",      "a/f.ext", "PN_SKIPPATH" ))

  asrt.istrue  (far.CmpName("*co*",        "color.ini"      ))
  asrt.istrue  (far.CmpName("*co*",        "edit.com"       ))
  asrt.istrue  (far.CmpName("[c-ft]*.txt", "config.txt"     ))
  asrt.istrue  (far.CmpName("[c-ft]*.txt", "demo.txt"       ))
  asrt.istrue  (far.CmpName("[c-ft]*.txt", "faq.txt"        ))
  asrt.istrue  (far.CmpName("[c-ft]*.txt", "tips.txt"       ))
  asrt.istrue  (far.CmpName("*",           "foo.bar"        ))
  asrt.istrue  (far.CmpName("*.cpp",       "foo.cpp"        ))
  asrt.isfalse (far.CmpName("*.cpp",       "foo.abc"        ))
  asrt.isfalse (far.CmpName("*|*.cpp",     "foo.abc"        )) -- exclude mask not supported
  asrt.isfalse (far.CmpName("*,*",         "foo.bar"        )) -- mask list not supported

  asrt.istrue (far.CmpNameList("*",          "foo.bar"    ))
  asrt.istrue (far.CmpNameList("*.cpp",      "foo.cpp"    ))
  asrt.isfalse(far.CmpNameList("*.cpp",      "foo.abc"    ))

  asrt.istrue (far.CmpNameList("*|*.cpp",    "foo.abc"    )) -- exclude mask IS supported
  asrt.istrue (far.CmpNameList("|*.cpp",     "foo.abc"    ))
  asrt.istrue (far.CmpNameList("*|",         "foo.abc"    ))
  asrt.istrue (far.CmpNameList("*|bar|*",    "foo.abc"    ))
  asrt.isfalse(far.CmpNameList("*|*.abc",    "foo.abc"    ))
  asrt.isfalse(far.CmpNameList("|",          "foo.abc"    ))

  asrt.istrue (far.CmpNameList("*.aa,*.bar", "foo.bar"    ))
  asrt.istrue (far.CmpNameList("*.aa,*.bar", "c:/foo.bar" ))
  asrt.istrue (far.CmpNameList("/.+/",       "c:/foo.bar" ))
  asrt.istrue (far.CmpNameList("/bar$/",     "c:/foo.bar" ))
  asrt.isfalse(far.CmpNameList("/dar$/",     "c:/foo.bar" ))
  asrt.istrue (far.CmpNameList("/abcd/;*",    "/abcd/foo.bar", "PN_SKIPPATH"))
  asrt.istrue (far.CmpNameList("/Makefile(.+)?/", "Makefile"))
  asrt.istrue (far.CmpNameList("/makefile([._\\-].+)?$/i", "Makefile", "PN_SKIPPATH"))

  asrt.isfalse (far.CmpNameList("f*.ex?",    "a/f.ext", 0     ))
  asrt.istrue  (far.CmpNameList("f*.ex?",    "a/f.ext", "PN_SKIPPATH" ))

  asrt.istrue  (far.CmpNameList("/ BAR ; /xi  ;*.md", "bar;foo"))
  asrt.isfalse (far.CmpNameList("/ BAR ; /xi  ;*.md", "bar,foo"))
  asrt.istrue  (far.CmpNameList("/ BAR ; /xi  ;*.md", "README.md"))
  asrt.isfalse (far.CmpNameList("/ BAR ; /xi  ;*.md", "README.me"))
end

local function test_FarStandardFunctions()
  test_clipboard()
  test_far_FarClock()

  test_ProcessName()

  asrt.eq(far.ConvertPath([[c:\foo\bar\..\..\abc]], "CPM_FULL"), [[c:\abc]])

  asrt.eq(far.FormatFileSize(123456, 0), "123456")
  asrt.eq(far.FormatFileSize(123456, 8), "  123456")
  asrt.eq(far.FormatFileSize(123456, -8), "123456  ")
  asrt.eq(far.FormatFileSize(123456, 0, F.FFFS_COMMAS), "123,456")
  asrt.eq(far.FormatFileSize(123456, 0, F.FFFS_SHOWBYTESINDEX), "123456 B")
  asrt.eq(far.FormatFileSize(123456, 0, F.FFFS_FLOATSIZE), "121 K")
  asrt.eq(far.FormatFileSize(123456, 0, F.FFFS_FLOATSIZE+F.FFFS_ECONOMIC), "121K")
  asrt.eq(far.FormatFileSize(123456, 0, F.FFFS_FLOATSIZE+F.FFFS_THOUSAND), "123 k")
  asrt.eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x3), "4 T")
  asrt.eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x2), "4112 G")
  asrt.eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x1), "4210688 M")
  asrt.eq(far.FormatFileSize(2^42+2^34, 0, F.FFFS_SHOWBYTESINDEX+F.FFFS_MINSIZEINDEX, 0x0), "4311744512 K")

  asrt.str (far.GetCurrentDirectory())

  asrt.eq(far.GetPathRoot[[D:\foo\bar]], [[D:\]])

  asrt.istrue  (far.LIsAlpha("A"))
  asrt.istrue  (far.LIsAlpha("Я"))
  asrt.isfalse (far.LIsAlpha("7"))
  asrt.isfalse (far.LIsAlpha(";"))

  asrt.istrue  (far.LIsAlphanum("A"))
  asrt.istrue  (far.LIsAlphanum("Я"))
  asrt.istrue  (far.LIsAlphanum("7"))
  asrt.isfalse (far.LIsAlphanum(";"))

  asrt.isfalse (far.LIsLower("A"))
  asrt.istrue  (far.LIsLower("a"))
  asrt.isfalse (far.LIsLower("Я"))
  asrt.istrue  (far.LIsLower("я"))
  asrt.isfalse (far.LIsLower("7"))
  asrt.isfalse (far.LIsLower(";"))

  asrt.istrue  (far.LIsUpper("A"))
  asrt.isfalse (far.LIsUpper("a"))
  asrt.istrue  (far.LIsUpper("Я"))
  asrt.isfalse (far.LIsUpper("я"))
  asrt.isfalse (far.LIsUpper("7"))
  asrt.isfalse (far.LIsUpper(";"))

  asrt.eq (far.LLowerBuf("abc-ABC-эюя-ЭЮЯ-7;"), "abc-abc-эюя-эюя-7;")
  asrt.eq (far.LUpperBuf("abc-ABC-эюя-ЭЮЯ-7;"), "ABC-ABC-ЭЮЯ-ЭЮЯ-7;")

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
  asrt.istrue (editor.SaveFile())
  asrt.istrue (editor.Quit())
  fp = assert(io.open(fname))
  local k = 0
  for line in fp:lines() do
    k = k + 1
    asrt.eq (line, "foo")
  end
  fp:close()
  win.DeleteFile(fname)
  asrt.eq (k, 3)
end

local function test_gmatch_coro()
  local function yieldFirst(it)
    return coroutine.wrap(function()
      coroutine.yield(it())
    end)
  end

  local it = ("1 2 3"):gmatch("(%d+)")
  local head = yieldFirst(it)
  asrt.eq (head(), "1")
end

local function test_PluginsControl()
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

local function test_far_timer()
  local N = 0
  local timer = far.Timer(50, function(hnd)
      N = N+1
      if N==3 then hnd:Close() end
    end)
  while not timer.Closed do Keys("foobar") end
  asrt.eq (N, 3)
end

local function test_win_CompareString()
  assert(win.CompareString("a","b") < 0)
  assert(win.CompareString("b","a") > 0)
  assert(win.CompareString("b","b") == 0)
end

local function test_win()
  test_win_CompareString()

  asrt.str(win.GetCurrentDir())
  asrt.table(win.EnumSystemCodePages())
  asrt.num(win.GetACP())
  asrt.num(win.GetOEMCP())

  local dir = asrt.str(win.GetEnv("FARHOME"))
  local attr = asrt.str(win.GetFileAttr(dir))
  asrt.num(attr:find("d"))
end

local function test_utf8()
  test_utf8_len()
  test_utf8_sub()
  test_utf8_lower_upper()
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

local function test_Guids()
  asrt.table(far.Guids)

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
  test_one_guid( "SelectFromEditHistoryId",  nil, "ShiftF4 A Enter Esc ShiftF4 CtrlDown", 2)
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

local function test_Editor_Sel_Cmdline(args)
  for _, str in ipairs(args) do
    asrt.istrue(Area.Shell)
    panel.SetCmdLine(nil, str)

    local act = 0
    for opt = 0,4 do
      asrt.eq(Editor.Sel(act,opt), 0) -- no block exists, zeros returned
    end

    local line, pos, w, h = 1, 13, 5, 1

    panel.SetCmdLineSelection(nil, pos, pos+w)
    asrt.eq(Editor.Sel(act,0), line)
    asrt.eq(Editor.Sel(act,1), pos)
    asrt.eq(Editor.Sel(act,2), line)
    asrt.eq(Editor.Sel(act,3), pos + w)
    asrt.eq(Editor.Sel(act,4), 1)
    --------------------------------------------------------------------------------------------------
    act = 1
    panel.SetCmdLineSelection(nil, pos, pos+w)

    asrt.eq(1, Editor.Sel(act, 0)) -- set cursor at block start
    asrt.eq(panel.GetCmdLinePos(), pos)

    asrt.eq(1, Editor.Sel(act, 1)) -- set cursor next to block end
    asrt.eq(panel.GetCmdLinePos(), pos + w + 1)
    --------------------------------------------------------------------------------------------------
    for act = 2,3 do
      panel.SetCmdLinePos(nil,pos)             -- set block start
      asrt.eq(1, Editor.Sel(act,0))          -- +++
      panel.SetCmdLinePos(nil,pos + w)         -- set block end (it also selects the block)
      asrt.eq(1, Editor.Sel(act,1))          -- +++

      asrt.eq(Editor.Sel(0,0), 1)
      asrt.eq(Editor.Sel(0,1), pos)
      asrt.eq(Editor.Sel(0,2), 1)
      asrt.eq(Editor.Sel(0,3), pos + w - 1)
      asrt.eq(Editor.Sel(0,4), 1)
    end
    --------------------------------------------------------------------------------------------------
    asrt.eq(1, Editor.Sel(4)) -- reset the block
    asrt.eq(Editor.Sel(0,4), 0)

    panel.SetCmdLine(nil, "")
  end
end

local function test_Editor_Sel_Dialog(args)
  Keys("F7 Del")
  asrt.istrue(Area.Dialog)
  local inf = actl.GetWindowInfo()
  local hDlg = asrt.udata(inf.Id)
  local EditPos = asrt.num(hDlg:GetFocus())

  for _, str in ipairs(args) do
    hDlg:SetText(EditPos, str)

    local act = 0
    for opt = 0,4 do
      asrt.eq(Editor.Sel(act,opt), 0) -- no block exists, zeros returned
    end

    local tSel = { BlockType = F.BTYPE_STREAM; BlockStartLine = 1; BlockStartPos = 13,
                   BlockWidth = 5; BlockHeight = 1; }

    hDlg:SetSelection(EditPos, tSel)
    asrt.eq(Editor.Sel(act,0), tSel.BlockStartLine)
    asrt.eq(Editor.Sel(act,1), tSel.BlockStartPos)
    asrt.eq(Editor.Sel(act,2), tSel.BlockStartLine)
    asrt.eq(Editor.Sel(act,3), tSel.BlockStartPos + tSel.BlockWidth - 1) -- [-1: DIFFERENT FROM EDITOR]
    asrt.eq(Editor.Sel(act,4), tSel.BlockType)
    --------------------------------------------------------------------------------------------------
    act = 1
    hDlg:SetSelection(EditPos, tSel)

    asrt.eq(1, Editor.Sel(act, 0)) -- set cursor at block start
    local pos = asrt.table(hDlg:GetCursorPos(EditPos))
    asrt.eq(pos.Y+1, tSel.BlockStartLine)
    asrt.eq(pos.X+1, tSel.BlockStartPos)

    asrt.eq(1, Editor.Sel(act, 1)) -- set cursor next to block end
    pos = asrt.table(hDlg:GetCursorPos(EditPos))
    asrt.eq(pos.Y+1, tSel.BlockStartLine)
    asrt.eq(pos.X+1, tSel.BlockStartPos + tSel.BlockWidth)
    --------------------------------------------------------------------------------------------------
    for act = 2,3 do
      hDlg:SetCursorPos(EditPos, {X=10; Y=0})  -- set block start
      asrt.eq(1, Editor.Sel(act,0))          -- +++
      hDlg:SetCursorPos(EditPos, {X=20; Y=0})  -- set block end (it also selects the block)
      asrt.eq(1, Editor.Sel(act,1))          -- +++

      asrt.eq(Editor.Sel(0,0), 1)
      asrt.eq(Editor.Sel(0,1), 10+1)
      asrt.eq(Editor.Sel(0,2), 1)
      asrt.eq(Editor.Sel(0,3), 20)
      asrt.eq(Editor.Sel(0,4), 1)
    end
    --------------------------------------------------------------------------------------------------
    asrt.eq(1, Editor.Sel(4)) -- reset the block
    asrt.eq(Editor.Sel(0,4), 0)
  end

  Keys("Esc")
end

local function test_Editor_Sel_Editor(args)
  local R2T, T2R = editor.RealToTab, editor.TabToReal
  local T2R2T = function(id, y, x)
    return R2T(id, y, T2R(id, y, x)) -- needed when the column position is "inside" a tab
  end

  Keys("ShiftF4 Del Enter")
  asrt.istrue(Area.Editor)

  for _, str in ipairs(args) do
    Keys("CtrlA Del")

    for k=1,8 do
      editor.InsertString()
      editor.SetString(nil, k, str)
    end

    local act = 0
    for opt = 0,4 do
      asrt.eq(Editor.Sel(act,opt), 0) -- no block exists, zeros returned
    end

    local line, pos, w, h = 3, 13, 5, 4

    for ii=1,2 do
      local typ = ii==1 and "BTYPE_STREAM" or "BTYPE_COLUMN"
      local ref_x1 = (ii==1 and R2T or T2R2T)(nil, line, pos)
      local ref_x2 = (ii==1 and R2T or T2R2T)(nil, line-1+h, pos+w)

      asrt.istrue(editor.Select(nil, typ, line, pos, w, h)) -- select the block
      asrt.eq(Editor.Sel(act,0), line)      -- get block start line
      asrt.eq(Editor.Sel(act,1), ref_x1)    -- get block start pos
      asrt.eq(Editor.Sel(act,2), line-1+h)  -- get block end line
      asrt.eq(Editor.Sel(act,3), ref_x2)    -- get block end pos
      asrt.eq(Editor.Sel(act,4), ii)        -- get block type
    end
    --------------------------------------------------------------------------------------------------
    act = 1
    for ii=1,2 do
      local typ = ii==1 and "BTYPE_STREAM" or "BTYPE_COLUMN"
      local ref_x1 = ii==1 and pos or T2R(nil, line, pos) or pos
      local ref_x2 = ii==1 and pos+w or T2R(nil, line-1+h, pos+w)

      local inf
      asrt.istrue(editor.Select(nil, typ, line, pos, w, h)) -- select the block

      asrt.eq(1, Editor.Sel(act, 0)) -- set cursor at block start
      inf = editor.GetInfo()
      asrt.eq(inf.CurLine, line)
      asrt.eq(inf.CurPos, ref_x1)

      asrt.eq(1, Editor.Sel(act, 1)) -- set cursor next to block end
      inf = editor.GetInfo()
      asrt.eq(inf.CurLine, line-1+h)
      asrt.eq(inf.CurPos, ref_x2)
    end
    --------------------------------------------------------------------------------------------------
    local y1,x1,y2,x2 = 2,10,6,20
    for act = 2,3 do
      editor.SetPosition(nil,y1,x1)    -- set block start
      asrt.eq(1, Editor.Sel(act,0))  -- +++
      editor.SetPosition(nil,y2,x2)    -- set block end (it also selects the block)
      asrt.eq(1, Editor.Sel(act,1))  -- +++

      local ref_x1 = (act==2 and R2T or T2R2T)(nil,y1,x1)
      local ref_x2 = (act==2 and R2T or T2R2T)(nil,y2,x2)

      asrt.eq(Editor.Sel(0,0), y1)                 -- get block start line
      asrt.eq(Editor.Sel(0,1), ref_x1)             -- get block start pos
      asrt.eq(Editor.Sel(0,2), y2)                 -- get block end line
      asrt.eq(Editor.Sel(0,3), ref_x2)             -- get block end pos
      asrt.eq(Editor.Sel(0,4), act==2 and 1 or 2)  -- get block type
    end
    --------------------------------------------------------------------------------------------------
    asrt.eq(1, Editor.Sel(4)) -- reset the block
    asrt.eq(Editor.Sel(0,4), 0)
  end

  asrt.istrue(editor.Quit())
end

local function test_Editor_Misc()
  local fname = far.MkTemp()
  local flags = {EF_NONMODAL=1, EF_IMMEDIATERETURN=1, EF_DISABLEHISTORY=1, EF_DELETEONCLOSE=1}
  asrt.eq(editor.Editor(fname,nil,nil,nil,nil,nil,flags), F.EEC_MODIFIED)
  asrt.istrue(Area.Editor)

  local EI = asrt.table(editor.GetInfo())

  local str = ("123456789-"):rep(4)
  local str2, num

  -- test Editor.Value
  editor.SetString(nil,1,str)
  asrt.eq(Editor.Value, str)

  -- test insertion with overtype=OFF
  editor.SetString(nil,1,str)
  editor.SetPosition(nil,1,1)
  editor.InsertText(nil, "AB")
  asrt.eq(Editor.Value, "AB"..str)

  -- test insertion with overtype=ON
  editor.SetString(nil,1,str)
  Keys("Ins")
  editor.SetPosition(nil,1,1)
  editor.InsertText(nil, "CD")
  asrt.eq(Editor.Value, "CD"..str:sub(3))
  Keys("Ins")

  -- test insertion beyond EOL (overtype=ON then OFF)
  num = 20
  asrt.istrue(editor.SetParam(nil, "ESPT_CURSORBEYONDEOL", true))
  str2 = str .. (" "):rep(num) .. "AB"
  for _=1,2 do
    Keys("Ins")
    editor.SetString(nil,1,str)
    editor.SetPosition(nil, 1, #str + 1 + num)
    editor.InsertText(nil, "AB")
    asrt.eq(Editor.Value, str2)
  end
  asrt.istrue(editor.SetParam(nil, "ESPT_CURSORBEYONDEOL", band(EI.Options, F.EOPT_CURSORBEYONDEOL) ~= 0))

  editor.Quit()
end

function MT.test_Editor()
  local args = {
    ("123456789-"):rep(4),     -- plain ASCII
    ("12\t4\t6789-"):rep(4),   -- includes tabs
    ("12🔥4🔥6789-"):rep(4),   -- includes 🔥 (a multi-byte double-width character)
    ("1Ю23456789-"):rep(4),    -- insert 'Ю' (a multi-byte character) into position 2
    ("1Ю2\t4\t6789-"):rep(4),  -- ditto
    ("1Ю2🔥4🔥6789-"):rep(4),  -- ditto
  }
  test_Editor_Sel_Cmdline(args)
  test_Editor_Sel_Dialog(args)
  test_Editor_Sel_Editor(args)
  test_Editor_Misc()
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
  MT.test_Editor()
  MT.test_Far()
  MT.test_Menu()
  MT.test_Mouse()
  MT.test_Object()
  MT.test_Panel()
  MT.test_Plugin()
  MT.test_APanel()
  MT.test_PPanel()
  MT.test_mantis_1722()
  MT.test_luafar()
  MT.test_coroutine()
end

return MT
