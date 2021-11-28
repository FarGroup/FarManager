local ACTIVE, PASSIVE = 1, 0
local pTypes = {"CtrlT", "CtrlQ", "CtrlL"} -- Tree, QView, Info

Macro {
  description="Activate the same folder in the passive panel as in the active panel";
  area="Shell";
  key="CtrlShiftBackSlash";
  action=function()
    if not PPanel.Visible then Keys("CtrlP") end
    Keys(pTypes[PPanel.Type])
    local ActiveDir = panel.GetPanelDirectory(nil, ACTIVE)
    if panel.SetPanelDirectory(nil, PASSIVE, ActiveDir) then
      Panel.SetPos(1, APanel.Current)
    end
  end;
}
