Macro {
  description="Close Fast find and send key to panel";
  area="Search";
  key="CtrlIns CtrlShiftIns CtrlAltIns CtrlNum0 CtrlShiftNum0 CtrlAltNum0"; --/Ctrl(Alt|Shift)?(Ins|Num0)/
  action=function()
    Keys("Esc AKey")
  end;
}
