local patt = regex.new( [=[
  \b http s?  :\/\/ [^\s`'"[\](){}<>]+ |
  \b ftp      :\/\/ [^\s`'"[\](){}<>]+ |
  \b www     \.     [^\s`'"[\](){}<>]+ |
  \b mailto:        [^\s`'"[\](){}<>]+ |
  (?: \b [a-z]:)?
    (?:
      (?: [\\\/][\w.\-]+ )+ |
      [\w.\-]+
    )
]=], "ix")

Macro {
  description="Open URL under cursor";
  area="Editor"; key="CtrlEnter CtrlNumEnter";
  action=function()
    local s=editor.GetStringW()
    if not s then return end
    local pos = editor.GetInfo().CurPos
    if pos > s.StringLength then return end
    local text, start = s.StringText.."\0", 1
    while true do
      local b,e = patt:findW(text,start)
      if b == nil or b > pos then break end
      if e >= pos then
        win.ShellExecute(nil, "open", win.Utf16ToUtf8(win.subW(text,b,e)))
        break
      end
      start = e+1
    end
  end;
}
