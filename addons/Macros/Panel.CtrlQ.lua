Macro {
  area="Shell"; key="CtrlQ"; description="Toggle quick view panel"; action = function()

Keys('Tab')
if Area.QView then
  Keys('Tab Ctrl2 Tab CtrlClear')
else
  Keys('Tab Ctrl6 Tab')
  if APanel.Left then
    for ii=1,APanel.Width do Keys('CtrlRight') end
  else
    for ii=1,APanel.Width do Keys('CtrlLeft') end
  end
end
Keys('Tab CtrlQ')

  end;
}

