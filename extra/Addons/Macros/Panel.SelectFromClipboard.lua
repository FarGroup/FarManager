local ACTIVE, SELECT, BYNAME = 0, 1, 2

Macro {
  description="Panel: Use Ctrl+S to select files from Clipboard";
  area="Shell";
  key="CtrlS";
  action=function()
    Panel.Select(ACTIVE,SELECT,BYNAME,far.PasteFromClipboard())
  end;
}

