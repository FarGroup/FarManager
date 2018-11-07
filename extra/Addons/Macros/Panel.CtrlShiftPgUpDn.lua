Macro {
  area="Shell"; key="CtrlShiftPgUp"; description="Jump to the previous folder on the same level"; action = function()

  if not APanel.Root then Keys('CtrlPgUp') end
  if not APanel.Bof then
    Keys('Up')
    if APanel.Folder then Keys('CtrlPgDn') end
  end

  end;
}

Macro {
  area="Shell"; key="CtrlShiftPgDn"; description="Jump to the next folder on the same level"; action = function()

  if not APanel.Root then Keys('CtrlPgUp') end
  if not APanel.Eof then
    Keys('Down')
    if APanel.Folder then Keys('CtrlPgDn') end
  end

  end;
}

