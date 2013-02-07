local guid="B66C436D-E657-419C-86D3-6834B9ACF3D2"

Macro {
  area="Editor"; key="CtrlM"; flags="DisableOutput"; description="Brackets matching - Find";
  condition=function() return Plugin.Exist(guid) end;
  action=function() Plugin.Menu(guid) Keys("Enter") end;
}

Macro {
  area="Editor"; key="CtrlShiftM"; flags="DisableOutput"; description="Brackets matching - Find & Select";
  condition=function() return Plugin.Exist(guid) end;
  action=function() Plugin.Menu(guid) Keys("Down Enter") end;
}
