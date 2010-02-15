@echo --------------------------------------------------------------------
@echo Continue only if you are sure that you have set the correct
@echo build and commited the changes.
@echo This command will tag the trunk under tags/VERSION_bBUILD.
@echo --------------------------------------------------------------------
@echo If you're not sure press CtrlC.
@echo --------------------------------------------------------------------
@echo --------------------------------------------------------------------
@echo Продолжайте только если вы уверены, что вы выставили правильный
@echo номер билда и закоммитили изменения.
@echo Эта команда пометит текущий trunk в tags/VERSION_bBUILD.
@echo --------------------------------------------------------------------
@echo Если вы не уверены, то нажмите CtrlC
@echo --------------------------------------------------------------------
@pause
@echo.
@tools\m4 -P svn_tag_build.m4
