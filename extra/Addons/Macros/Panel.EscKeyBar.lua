Macro {
  description="Use Esc to toggle panels on/off (with keybar)";
  area="Shell Tree QView Info";
  key="Esc";
  flags="EmptyCommandLine";
  action=function()
    Keys"CtrlO"
    if APanel.Visible~=PPanel.Visible then Keys "CtrlP" end -- unhide both panels
    Far.KeyBar_Show(APanel.Visible and 1 or 2)
  end;
}
