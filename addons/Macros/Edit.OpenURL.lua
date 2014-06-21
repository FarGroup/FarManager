local patt = regex.new( [=[
  \b http s?  :\/\/ [^\s`'"[\](){}<>]+ |
  \b ftp      :\/\/ [^\s`'"[\](){}<>]+ |
  \b www \d*  \.    [^\s`'"[\](){}<>]+ |
  \b mailto:        [^\s`'"[\](){}<>]+ |
  (?: \b [a-z]:)? (?: [\\\/]? [\w.\-]+ )+ [\\\/]?
]=], "ix")

Macro {
  description="Open URL under cursor";
  area="Editor"; key="CtrlEnter CtrlNumEnter";
  action=function()
    local s=editor.GetStringW()
    if not s then return end
    local pos = editor.GetInfo().CurPos
    if pos > s.StringLength+1 then return end
    if pos == s.StringLength+1 then pos = pos-1 end
    local text, start = s.StringText.."\0", 1
    while true do
      local b,e = patt:findW(text,start)
      --far.Show(b,e)
      if b == nil or b > pos then break end
      if e >= pos then
        local url = win.Utf16ToUtf8(win.subW(text,b,e))
        --far.Show(url)
        win.ShellExecute(nil, "open", url)
        break
      end
      start = e+1
    end
  end;
}
