-- test_dialog.lua
local asrt = assert(loadfile(far.PluginStartupInfo().ModuleDir.."test\\assert.lua")) ()

local F = far.Flags

--[[------------------------------------------------------------------------------------------------
0001722: DN_EDITCHANGE приходит лишний раз и с ложной информацией

Description:
  [ Far 2.0.1807, Far 3.0.1897 ]
  Допустим диалог состоит из единственного элемента DI_EDIT, больше элементов нет. При появлении
  диалога сразу нажмём на клавишу, допустим, W. Приходят два события DN_EDITCHANGE вместо одного,
  причём в первом из них PtrData указывает на пустую строку.

  Последующие нажатия на клавиши, вызывающие изменения текста, отрабатываются правильно, лишние
  ложные события не приходят.
--]]------------------------------------------------------------------------------------------------
local function test_mantis_1722()
  local check = 0
  local function DlgProc (hDlg, msg, p1, p2)
    if msg == F.DN_EDITCHANGE then
      check = check + 1
      asrt.eq(p1, 1)
    end
  end
  local Dlg = { {"DI_EDIT", 3,1,56,10, 0,0,0,0, "a"}, }
  mf.acall(far.Dialog, nil,-1,-1,60,3,"Contents",Dlg, 0, DlgProc)
  asrt.istrue(Area.Dialog)
  Keys("W 1 2 3 4 BS Esc")
  asrt.eq(check, 6)
  asrt.eq(Dlg[1][10], "W123")
end

-- Сказано: "Флаг DIF_EDITEXPAND "расширяет" переменные среды после завершения выполнения диалога"
-- На самом деле переменные разворачиваются ещё при открытом диалоге,
-- и в момент DN_CLOSE обработчик получает уже развёрнутые строки.
local function test_mantis_3070()
  local count = 0
  local var = "%FARHOME%"
  local items = {
    { F.DI_EDIT,1,1,18,1,0,"test",nil,F.DIF_EDITEXPAND,var }
  }

  local function proc(hDlg,Msg,Param1,Param2)
    if Msg == F.DN_CLOSE then
      count = count + 1
      local txt = asrt.str(hDlg:GetText(1))
      asrt.eq(txt, var)
    end
  end

  asrt.istrue(Area.Shell)
  mf.acall(far.Dialog,"",-1,-1,20,3,nil,items,nil,proc)
  asrt.istrue(Area.Dialog)
  Keys("Enter")
  asrt.istrue(Area.Shell)

  asrt.eq(count, 1)
  local home = asrt.str(win.GetEnv("FARHOME"))
  asrt.neq(home, var)
  asrt.eq(home, items[1][10])
end

local function test_all()
  test_mantis_1722()
  test_mantis_3070()
end

return {
  test_all = test_all;
}
