Macro {
  area="Shell"; key="CtrlS"; description="Panel: Use Ctrl-S to select files from Clipboard"; action = function()
Panel.Select(0,1,2,mf.clip(0))
  end;
}

