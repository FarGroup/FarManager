Macro {
  description="Converts the word before cursor using XLat function";
  area="Dialog Editor Shell QView Tree Info";
  key="ShiftBS";
  condition=function()
    return Area.Dialog or Area.Editor or not CmdLine.Empty
  end;
  action=function()
    Keys("CtrlShiftLeft XLat CtrlRight")
  end;
}
