Macro {
  area="Common"; key="Esc"; flags="EmptyCommandLine"; description="Use Esc to toggle panels on/off (with keybar)"; action = function()

if Area.Shell or Area.Tree or Area.QView or Area.Info then  
  Keys('CtrlO')  
  Far.KeyBar_Show((APanel.Visible or PPanel.Visible) and 1 or 2) 
else  
  Keys('AKey')  
end

  end;
}

