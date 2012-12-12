Macro {
  area="Shell"; key="AltBS"; flags="DisableOutput"; description="Use Alt-BS to undo folder changes"; action = function()

  Keys('AltF12')
  _G.StdM_AltBS = Object.CurPos == Object.ItemCount and not _G.StdM_AltBS
  if not _G.StdM_AltBS then Keys('Up') end
  Keys('ShiftEnter')

  end;
}

