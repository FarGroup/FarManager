local Numpad = {Multiply="*", Add="+", Subtract="-", Divide="/"}

Macro {
  description="Use numpad keys in command line";
  area="Shell Search";
  key="Multiply Add Subtract Divide";
  condition=function()
    return not CmdLine.Empty or Area.Search
  end;
  action=function()
    Keys(Numpad[mf.akey(1,1)])
  end;
}
