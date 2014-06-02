Macro {
  area="Dialog Editor Shell"; key="ShiftBS"; description="Converts the word before cursor using XLat function";
  condition = function() return not (Area.Shell and CmdLine.Empty) end;
  action = function() Keys('CtrlShiftLeft XLat CtrlRight') end;
}
