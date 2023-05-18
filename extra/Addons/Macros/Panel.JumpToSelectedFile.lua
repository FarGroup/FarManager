local cyclic = true -- set false to disable cyclic jump

local is_selected = 8
local function JumpToSelected (from, to)
  for pos=from,to,from<to and 1 or -1 do
    if Panel.Item(0,pos,is_selected) then return Panel.SetPosIdx(0,pos) end
  end
  if cyclic then
    mf.beep()
    JumpToSelected(APanel.ItemCount-to,to)
  end
end

Macro {
  description="Jump to the next selected file";
  area="Shell Search";
  key="CtrlShiftDown";
  flags="Selection";
  action=function()
    JumpToSelected(APanel.CurPos+1,APanel.ItemCount)
  end;
}

Macro {
  description="Jump to the previous selected file";
  area="Shell Search";
  key="CtrlShiftUp";
  flags="Selection";
  action=function()
    JumpToSelected(APanel.CurPos-1,1)
  end;
}

Macro {
  description="Jump to the first selected file";
  area="Shell Search";
  key="CtrlShiftHome CtrlShiftNum7";
  flags="Selection";
  action=function()
    JumpToSelected(1,APanel.ItemCount)
  end;
}

Macro {
  description="Jump to the last selected file";
  area="Shell Search";
  key="CtrlShiftEnd CtrlShiftNum1";
  flags="Selection";
  action=function()
    JumpToSelected(APanel.ItemCount,1)
  end;
}
