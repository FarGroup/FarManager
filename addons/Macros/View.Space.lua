Macro {
  area="Viewer"; key="Space"; description="Use Space for listing in the viewer (like in Outlook Express or The Bat!)"; action = function()

  if Object.Eof then
    Keys('CtrlF10 Shift')
    if not APanel.Eof then Keys('Add Home') end
  else
    Keys('PgDn')
  end

  end;
}

