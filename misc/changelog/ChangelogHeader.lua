Macro {
  area="Editor"; key="Ctrl`"; action = function()
    Keys("CtrlHome Down End CtrlLeft")
    build=mf.int(mf.substr(Editor.Value,Editor.RealPos-1))+1;
    Keys("CtrlHome")

    Tz=mf.date("%z");
    print("--------------------------------------------------------------------------------") Keys("Enter")
    print(mf.date("name %Y-%m0-%d %H:%M:%S")) print(mf.substr(Tz,0,3)..":"..mf.substr(Tz,3,2)) print(" - build ") print(build) print([[


1. <English>

· · · · · · · · · · · · · · · · · · · · · · · · ·

1. <Russian> (Optional. If omitted, also remove the separator above.)

]])

    Keys("Up Up Up Up Up Up Right Right Right ShiftEnd")
  end;
}
