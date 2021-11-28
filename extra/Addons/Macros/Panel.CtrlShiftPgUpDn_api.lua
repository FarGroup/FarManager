--same as Panel.CtrlShiftPgUpDn.lua, but implemented with plugin api (instead of macro api)
local ACTIVE = 1
local function Jump (direction)
  local source_path = panel.GetPanelDirectory(nil,ACTIVE).Name
  local source_pos = panel.GetPanelInfo(nil,ACTIVE)
  panel.SetPanelDirectory(nil,ACTIVE,"..")
  local p = panel.GetPanelInfo(nil,ACTIVE)
  local pos = p.CurrentItem + direction
  if pos<=p.ItemsNumber and pos>0 then
    local i = panel.GetPanelItem(nil,ACTIVE,pos)
    if i.FileAttributes:find"d" and (pos~=1 or i.FileName~="..") then
      panel.SetPanelDirectory(nil,ACTIVE,i.FileName)
      return
    end
  end
  panel.SetPanelDirectory(nil,ACTIVE,source_path)
  panel.RedrawPanel(nil,ACTIVE,source_pos)
end

local function notRoot() return not APanel.Root end

Macro {
  description="Jump to the next folder on the same level";
  area="Shell";
  key="CtrlShiftPgUp CtrlShiftNum9";
  condition=notRoot;
  action=function()
    Jump(-1)
  end;
}

Macro {
  description="Jump to the previous folder on the same level";
  area="Shell";
  key="CtrlShiftPgDn CtrlShiftNum3";
  condition=notRoot;
  action=function()
    Jump(1)
  end;
}
