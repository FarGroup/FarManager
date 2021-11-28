local WIF_MODAL = far.Flags.WIF_MODAL
Macro {
  description="^Ctrl+Tab";
  area="Common";
  key="ShiftTab";
  condition=function()
    local wi = far.AdvControl("ACTL_GETWINDOWINFO")
    return wi and band(wi.Flags,WIF_MODAL)==0
  end;
  action=function()
    Keys("CtrlShiftTab")
  end;
}
