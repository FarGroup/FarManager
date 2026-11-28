-- test_fsf.lua
local asrt = assert(loadfile(far.PluginStartupInfo().ModuleDir.."test\\assert.lua")) ()
local F = far.Flags

local function test_Clipboard()
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


local function test_path_funcs()
  local dir = asrt.table(panel.GetPanelDirectory(nil,1)).Name
  asrt.eq (far.GetCurrentDirectory(), dir)
  asrt.eq (far.ConvertPath("abcd"), win.JoinPath(dir, "abcd"))
  asrt.eq(far.ConvertPath([[c:\foo\bar\..\..\abc]], "CPM_FULL"), [[c:\abc]])
end


local function test_text_funcs()
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

  asrt.lt (far.LStricmp("abc", "def") , 0)
  asrt.gt (far.LStricmp("def", "abc") , 0)
  asrt.eq (far.LStricmp("abc", "abc") , 0)
  asrt.lt (far.LStricmp("ABC", "def") , 0)
  asrt.gt (far.LStricmp("DEF", "abc") , 0)
  asrt.eq (far.LStricmp("ABC", "abc") , 0)

  asrt.lt (far.LStrnicmp("abc", "def", 3), 0)
  asrt.gt (far.LStrnicmp("def", "abc", 3), 0)
  asrt.eq (far.LStrnicmp("abc", "abc", 3), 0)
  asrt.lt (far.LStrnicmp("ABC", "def", 3), 0)
  asrt.gt (far.LStrnicmp("DEF", "abc", 3), 0)
  asrt.eq (far.LStrnicmp("ABC", "abc", 3), 0)
  asrt.eq (far.LStrnicmp("111abc", "111def", 3), 0)
  asrt.lt (far.LStrnicmp("111abc", "111def", 4), 0)

  asrt.eq(far.GetPathRoot[[D:\foo\bar]], [[D:\]])
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
  for _ = 1,10 do
    win.Sleep(20)
    if temp ~= far.FarClock() % 10 then OK=true; break; end
  end
  asrt.istrue(OK)
end


local function test_all()
  test_Clipboard()
  test_path_funcs()
  test_ProcessName()
  test_text_funcs()
  test_far_FarClock()
end


return {
  test_all = test_all;
}
