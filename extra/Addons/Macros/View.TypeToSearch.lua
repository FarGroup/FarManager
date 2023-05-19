-- Based on https://forum.farmanager.com/viewtopic.php?p=173468#p173468
Macro {
  description="Bring up the Search dialog with the typed character or pasted text";
  area="Viewer";
  key="/\\S|[LR]CtrlV|ShiftIns/";
  action=function()
    Keys("F7 AKey")
  end
}
