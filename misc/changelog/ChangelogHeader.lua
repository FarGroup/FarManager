Macro {
  area="Editor"; key="Ctrl`"; action = function()
    Keys("CtrlHome Down End CtrlLeft")
    build=mf.int(mf.substr(Editor.Value,Editor.RealPos-1))+1;
    Keys("CtrlHome")

    print("--------------------------------------------------------------------------------") Keys("Enter")
    print(mf.date("name %d.%m0.%Y %H:%M:%S %z - build ")) print(build) print([[


1. <English>

· · · · · · · · · · · · · · · · · · · · · · · · ·

1. <Russian> (Optional. If omitted, also remove the separator above.)

]])

    Keys("Up Up Up Up Up Up Right Right Right ShiftEnd")
  end;
}
