@echo off
  rem gnu.cmd {cp|mv|rm|mkdir} [-vfpr] param1 [param2 ...]
  rem ==================================================++
  for %%o in (v f p r) do set opt%%o=N
  for %%v in (mode last list terr) do set %%v=& rem
  rem
  set all=%* &rem
  for %%a in (%all:/=\%) do call :proc_param %%a
  rem
  set terr=bad or missed command: '%mode%'
  for %%c in (cp mv rm mkdir) do if /i "%mode%" == "%%c" (set terr=& set mode=%%c)
  if not "" == "%terr%" goto :param_errors
  set terr=missed '%mode%' parameter(s)  
goto :do_%mode%

rem =========================================
:proc_param
  set aa=%1
  if "-" == "%aa:~0,1%" goto :proc_opt
  if "" == "%mode%" set mode=%aa%& goto :EOF
  if not "" == "%last%" set list=%list% %last%
  set last=%aa%
  if "\" == "%aa:~-1%" set last=%aa:~0,-1%
goto :EOF
:proc_opt
  set aa=%aa:~1%
  if not "" == "%aa%" set opt%aa:~0,1%=Y& goto :proc_opt
goto :EOF

:show_op
  if not "Y" == "%optv%" goto :EOF
  set p1=%2& set p2=%3
  if exist %p1%\* set p1=%p1%\
  if not "" == "%p2%" if exist %p2%\* set p2=%p2%\
  echo %1 %p1% %p2%
goto :EOF

:cmd_errors
  set terr='%mode%' failed
:param_errors
  echo.& echo gnu.cmd ERROR: %terr%
exit 1

rem =========================================
:do_cp
:do_mv
  if "" == "%list%" goto :param_errors
  set op=copy& if "mv" == "%mode%" set op=move
  for %%a in (%list%) do call :show_op %op% %%a %last%& %op% /y %%a %last% 1>NUL|| goto :cmd_errors
goto :EOF

:do_mkdir
  if "" == "%last%" goto :param_errors
  for %%a in (%list% %last%) do if not exist %%a\* (call :show_op mkdir %%a& mkdir %%a|| goto :cmd_errors)
goto :EOF

:do_rm
  rem if "" == "%last%" goto :param_errors
  for %%a in (%list% %last%) do call :show_op delete %%a& call :rm_one %%a
goto :EOF
:rm_one
  if exist %1\* goto :rm_one_dir
  if exist %1 del /q %1 1>NUL|| goto :cmd_errors
goto :EOF
:rm_one_dir
  set op=rd /q& if "Y" == "%optr%" set op=rd /q /s
  %op% %1|| goto :cmd_errors
goto :EOF