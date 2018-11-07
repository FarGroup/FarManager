Macro {
  area="Shell Tree QView Info"; key="Esc"; flags="EmptyCommandLine"; description="Use Esc to toggle panels on/off (with keybar)"; action = function()

Keys('CtrlO')
Far.KeyBar_Show((APanel.Visible or PPanel.Visible) and 1 or 2)

  end;
}

