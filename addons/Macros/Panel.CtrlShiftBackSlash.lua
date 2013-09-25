Macro {
  area="Shell"; key="CtrlShiftBackSlash"; description="Activate the same folder in the passive panel as in the active panel"; action = function()
    panel.SetPanelDirectory(nil,0,panel.GetPanelDirectory(nil,1))
  end;
}

