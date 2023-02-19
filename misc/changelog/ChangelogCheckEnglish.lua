local HeaderPattern = regex.new([[ ^\w.+\s20\d\d-\d\d-\d\d\s\d\d:\d\d:\d\d[+-]\d\d:00.*$ ]], "x")
local SeparatorPattern = regex.new([[^·(\s·)+$]], "x")
local CyrillicPattern = regex.new([[[А-Яа-я]+]], "x")

Macro {
  area="Editor"; key="CtrlShift`"; action = function()

    Keys("Home")

    local skipToHeader = true

    while Editor.CurLine < Editor.Lines do
      Keys("CtrlDown")
      local line = editor.GetStringW().StringText.."\0"

      if skipToHeader then
        if HeaderPattern:execW(line) then
          skipToHeader = false
        end
      else
        -- We've seen an entry header. The first section should be in English.
        -- Scanning for the language separator to start skipping the Russian section,
        -- or for Cyrillic characters which should not happen in the English section.
        -- Even though the current entry may be English-only, thus ending at the next
        -- entry's header, we do not look for a header. This is performance optimization.
        -- We avoid redundant evaluation of HeaderPattern on the lines of the English section.
        -- On the other hand, we will scan the next header for Cyrillics, but it's OK.
        if SeparatorPattern:execW(line) then
          skipToHeader = true
        else
          if CyrillicPattern:execW(line) then return end
        end
      end
    end
  end
}
