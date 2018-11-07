Macro {
  description="Panel: Use Space to select files";
  area="Shell"; key="Space";
  condition=function() return CmdLine.Empty and APanel.Visible end;
  action=function() Keys('Ins') end;
}
