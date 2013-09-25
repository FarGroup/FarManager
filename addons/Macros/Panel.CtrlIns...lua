Macro {
  area="Shell"; key="CtrlIns"; flags="EmptyCommandLine NoPluginPanels"; description="If CtrlIns was pressed while on .. and command line is empty then copy to clipboard the path of the current folder"; action = function()

if APanel.Bof and not APanel.Selected and not APanel.Root then
  Keys('F7 CtrlShift[ CtrlIns Esc')
else
  Keys('CtrlIns')
end

  end;
}

