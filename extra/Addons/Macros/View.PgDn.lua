Macro {
  area="Viewer"; key="PgDn"; description="PgDn at the end of file works as in Far Manager 1.70 beta 4 and earlier"; action = function()
if Object.Eof then Keys('End') else Keys('PgDn') end
  end;
}

