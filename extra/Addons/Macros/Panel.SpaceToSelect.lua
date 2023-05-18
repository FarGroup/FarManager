local APANEL,INVERT,MODE_IDX,CUR_ITEM = 0,2,1,0
Macro {
  description="Panel: Use Space to select files";
  area="Shell";
  key="Space";
  condition=function()
    return CmdLine.Empty and APanel.Visible
  end;
  action=function()
    Panel.Select(APANEL,INVERT,MODE_IDX,CUR_ITEM)
  end;
}
