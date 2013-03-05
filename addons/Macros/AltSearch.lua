Macro {
  area="Shell"; key="Alt"; flags=""; description="Use Alt for search by name (activate)"; action = function()

Keys('Alt<')

  end;
}

Macro {
  area="Shell"; key="RAlt"; flags=""; description="Use Right Alt for search by name (activate)"; action = function()

Keys('Alt<')

  end;
}

Macro {
  area="Search"; key="Alt"; flags=""; description="Use Alt for search by name (deactivate)"; action = function()
Keys('Esc')
  end;
}

Macro {
  area="Search"; key="RAlt"; flags=""; description="Use Right Alt for search by name (deactivate)"; action = function()
Keys('Esc')
  end;
}

