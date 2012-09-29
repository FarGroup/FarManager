function export.GetGlobalInfo()
  return {
    Version       = { 0, 1, 0, 0 },
    MinFarVersion = { 3, 0, 0, 2827 },
    Guid          = win.Uuid("4ebbefc8-2084-4b7f-94c0-692ce136894d"),
    Title         = "LuaMacro",
    Description   = "Far macros in Lua",
    Author        = "Shmuel Zeigerman",
    -----------------------------------------------------------------
    --MinLuafarVersion = { 3, 0, 9 },
  }
end
