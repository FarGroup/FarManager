local StdM

Macro {
  area="Shell"; key="AltBS"; description="Use Alt-BS to undo folder changes"; action = function()

  Keys('AltF12')
  StdM = Object.CurPos == Object.ItemCount and not StdM
  if not StdM then Keys('Up') end
  Keys('ShiftEnter')

  end;
}

