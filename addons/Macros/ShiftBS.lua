Macro {
  area="Dialog"; key="ShiftBS"; flags="DisableOutput"; description="Converts the word before cursor using XLat function"; action = function()
Keys('CtrlShiftLeft XLat CtrlRight')
  end;
}

Macro {
  area="Editor"; key="ShiftBS"; flags="DisableOutput"; description="Converts the word before cursor using XLat function"; action = function()
Keys('CtrlShiftLeft XLat CtrlRight')
  end;
}

Macro {
  area="Shell"; key="ShiftBS"; flags="DisableOutput|NotEmptyCommandLine"; description="Converts the word before cursor using XLat function"; action = function()
Keys('CtrlShiftLeft XLat CtrlRight')
  end;
}

