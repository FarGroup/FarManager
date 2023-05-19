Macro {
  description="Scroll screen (using keys like in editor)";
  area="Viewer";
  key="CtrlUp CtrlDown";
  action=function()
    Keys(mf.akey(1,1):match(".%l+$"))
  end;
}
