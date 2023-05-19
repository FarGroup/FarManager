local last
Macro {
  description="Opening previous file from view/edit history";
  area="Viewer Editor Shell QView Tree Info Search";
  key="Ctrl'";
  action=function()
    Keys("AltF11")
    if not Area.Menu then return end -- modal editor/viewer
    if not Object.Eof or last==Menu.Value then
      Keys("Up")
    end
    last = Menu.Value
    Keys("ShiftEnter")
  end;
}
