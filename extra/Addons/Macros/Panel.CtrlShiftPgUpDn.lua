local ACTIVE = 0
local GET_ATTR = 2
local GET_NAME = 0
local FILE_ATTRIBUTE_DIRECTORY = 0x00000010
local function Jump (direction)
  local source_path,source_pos = APanel.Path,APanel.CurPos
  Panel.SetPath(ACTIVE,"..")
  local pos = APanel.CurPos + direction
  if pos<=APanel.ItemCount and pos>0 then
    local is_folder = 0~=band(Panel.Item(ACTIVE,pos,GET_ATTR),FILE_ATTRIBUTE_DIRECTORY)
    local name = Panel.Item(ACTIVE,pos,GET_NAME)
    if is_folder and (pos~=1 or name~="..") then
      Panel.SetPath(ACTIVE,name)
      return
    end
  end
  Panel.SetPath(ACTIVE,source_path)
  Panel.SetPosIdx(ACTIVE,source_pos)
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
