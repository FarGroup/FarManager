-- Open URL (http://url.spec.whatwg.org) or UNC under cursor.
-- In rare case if last URL symbol is lost, add "#": https://www.google.com/search?q=hello!#

-- Confirm launching the URL under cursor. Change it to false if you want no confirmations.
local Confirm = true

-- Change this to true to use selection (if present)
-- instead of a fragment matching the pattern as the target URL.
-- WARNING: setting this to true may be unsafe if you select and open a random code instead of an URL.
local UseSelection = false

local patt = regex.new( [=[
(?:
  (?<=  [`()[\]{}‘’‚‛‹›“”„‟«»"'<>]) (?: \b \i[\d\i+.-]*: | \\ | \b www)
    [^\s `()[\]{}‘’‚‛‹›“”„‟«»"'<> │┃┆┇┊┋╎╏║ ]+
    [^\s `()[\]{}‘’‚‛‹›“”„‟«»"'<> │┃┆┇┊┋╎╏║ |‖•·¦ !,-.:;?¶‐‑‒–—―…]
|
  (?: \b \i[\d\i+.-]*: | \\ | \b www) (?:
    [^\s│┃┆┇┊┋╎╏║]+  [`()[\]{}‘’‚‛‹›“”„‟«»"'<>] [^\s│┃┆┇┊┋╎╏║]+ [^\s│┃┆┇┊┋╎╏║ |‖•·¦ !,-.:;?¶‐‑‒–—―…] |
    [^\s│┃┆┇┊┋╎╏║]+ [^`()[\]{}‘’‚‛‹›“”„‟«»"'<>                    \s│┃┆┇┊┋╎╏║ |‖•·¦ !,-.:;?¶‐‑‒–—―…]
  )
)
]=], "ix")

local mouseClick,minY
Macro {
  description="Open URL or UNC under cursor";
  area="Editor"; key="CtrlEnter CtrlNumEnter CtrlMsLClick";
  condition=function(trigger)
    mouseClick = trigger:match("Ms")
    if not mouseClick then return true end
    local info = editor.GetInfo()
    local maxY = info.WindowSizeY - 1
    minY = 0
    if 0 ~= bit64.band(info.Options, far.Flags.EOPT_SHOWTITLEBAR) then
      minY, maxY = 1, maxY + 1
    end
    return Mouse.Y >= minY and Mouse.Y <= maxY
  end;
  action=function()
    if mouseClick then
      Editor.Pos(1, 3, Editor.Pos(0, 5)+Mouse.X)
      Editor.Pos(1, 1, Editor.Pos(0, 4)+Mouse.Y-minY)
    end
    local pos,url = editor.GetInfo().CurPos
    if UseSelection and Editor.Sel(0,4)==1 and Editor.Sel(0,0)==Editor.Sel(0,2) and pos>=Editor.Sel(0,1) and pos<=Editor.Sel(0,3) then
      url = Editor.SelValue
    else
      local s=editor.GetStringW()
      if not s then return end
      if pos > s.StringLength+1 then return end
      if pos == s.StringLength+1 then pos = pos-1 end
      local text, start = s.StringText.."\0", 1
      while true do
        local b,e = patt:findW(text,start)
        if b == nil or b > pos then break end
        if e >= pos then
          url = win.Utf16ToUtf8(win.subW(text,b,e))
          break
        end
        start = e + 1
      end
    end
    if url and (not Confirm or 1==far.Message(url, "Do you want to run this?", ";YesNo")) then
      win.ShellExecute(nil, "open", url)
    end
  end;
}

--[[

Terminal Characters

http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[:General_Category=Open_Punctuation:]
http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[%3AGeneral_Category%3DClose_Punctuation%3A]&g=
http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[:Terminal_Punctuation=Yes:]
http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[%3AGeneral_Category%3DFinal_Punctuation%3A]&g=
http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[:Quotation_Mark=Yes:]
http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[:Usage=punctuation:]
http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[:Block=Box_Drawing:]

Supported Tests

mailto:example@example.com   http://msdn.microsoft.com/en-us/library/aa752574(VS.85).aspx
(visit http://example.com/#hello)!   (this http://example.com/a/#test!)
http://example.org/💩   `http://example.com`   "test http://example.com"   http://example.com- link
www.example.com   «http://example.com»   <http://example.com>   http://example.com/a(b)c: link,
http://example.com/a:`b`  See http://example.com!   https://www.google.com/search?q=hello!# search.
<a href="http://example.com">Example</a>
"http://example.com! visit"   "here: http://example.com"   http://example.com/&s="abc"
https://groups.google.com/forum/#!searchin/alt.test.test/test2|sort:date
[Visit GitHub!](https://github.com)   url=www.example.com   www66.example.com
http://example.com/1┆http://example.com/2┆http://example.com/3
http://кц.рф   http://тест.укр   http://рнидс.срб   http://тест.қаз
http://عربي.امارات   http://وزارة-الأتصالات.مصر
\\?\C:\Windows\win.ini   \\i7\C$\Windows\win.ini   \\?\UNC\i7\C$\Windows\win.ini
C:\Windows\win.ini   C:/Windows/win.ini   \Windows\win.ini
file://C:/Windows/win.ini   file://C|/Windows\win.ini   file://i7/C$/Windows/win.ini
file:\\C:\Windows/win.ini   file:\C|\Windows\win.ini   file:\\i7\C$\Windows\win.ini
geo:1.44951,43.604363
magnet:?xt=urn:btih:757B25D9681D493167B8D3759DBFDDC983E80646&dn=ubuntu-14.04-server-amd64.iso&tr=http%3a%2f%2ftorrent.ubuntu.com%3a6969%2fannounce
dchub://example.com:411/John/uploads/applications/linux.iso
ed2k://|file|The_Two_Towers-The_Purist_Edit-Trailer.avi|14997504|965c013e991ee246d63d45ea71954c4d|/

--]]
