Macro {
  description="Use Del to remove files";
  area="Shell Tree Search";
  key="Del NumDel";
  flags="EnableOutput";
  condition=function()
    return Area.Search or CmdLine.Empty or CmdLine.Eof
  end;
  action=function()
    Keys("F8")
  end;
}
