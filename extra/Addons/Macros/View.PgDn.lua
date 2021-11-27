Macro {
  description="PgDn: never scroll beyond EOF (like in FAR 1.70 beta 4 and earlier)";
  area="Viewer";
  key="PgDn Num3";
  action=function()
    Keys("AKey")
    if Object.Eof then
      Keys("End")
    end
  end;
}
