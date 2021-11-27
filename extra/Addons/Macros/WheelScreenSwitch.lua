local WIF_MODAL = far.Flags.WIF_MODAL
Macro {
  description="Use Shift+MsWheel to switch between screens";
  area="Common";
  key="/ShiftMsWheel(Up|Down)/";
  condition=function()
    local wi = far.AdvControl("ACTL_GETWINDOWINFO")
    return wi and band(wi.Flags,WIF_MODAL)==0
  end;
  action=function()
    Keys("F12",mf.akey(1,1):match(".%l+$"),"Enter")
  end;
}
