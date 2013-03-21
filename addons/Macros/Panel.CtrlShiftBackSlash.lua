Macro {
  area="Shell"; key="CtrlShiftBackSlash"; flags=""; description="Activate the same folder in the passive panel as in the active panel"; action = function()
    panel.SetPanelDirectory(nil,0,panel.GetPanelDirectory(nil,1))
  end;
}

