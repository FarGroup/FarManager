Macro {
  area="Shell Editor Viewer Other"; key="/[LR]Alt[0-9]/"; description="Use Alt-digit to switch between screens"; action = function()
Keys('F12', akey(1):sub(-1))
  end;
}
