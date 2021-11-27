Macro {
  description="Change current drive";
  area="Shell Info QView Tree Search";
  key="CtrlLeft CtrlRight";
  flags="EmptyCommandLine";
  action=function()
    Keys("F9 Enter Up Enter")
    if not APanel.Plugin then         -- if plugin panel then switch to current drive
      local k = mf.akey(1,1):match("^Ctrl(.*)")
      repeat
        Keys(k)
      until Menu.Value:match("^%u: ") -- search drive letters only (skip plugins)
    end
    Keys("Enter")
  end;
}
