Macro {
  description="Use Esc to toggle panels on/off";
  area="Shell Info QView Tree";
  key="Esc";
  flags="EmptyCommandLine";
  action=function()
    Keys("CtrlO")
    if APanel.Visible~=PPanel.Visible then -- unhide both panels
      Keys("CtrlP")
    end
  end;
}
