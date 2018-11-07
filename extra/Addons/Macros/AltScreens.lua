Macro {
  description="Use Alt-digit to switch between screens";
  area="Editor Viewer Shell QView Tree Info Desktop"; key="/[LR]Alt[0-9]/";
  action=function()
    local key = mf.akey(1,1):sub(-1)
    Keys("F12",Object.CheckHotkey(key)~=0 and key or "Esc")
  end;
}
