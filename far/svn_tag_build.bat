@echo --------------------------------------------------------------------
@echo Continue only if you are sure that you have set the correct
@echo build and commited the changes.
@echo This command will tag the trunk under tags/VERSION_bBUILD, if such
@echo tag already exists this command might cause problems.
@echo --------------------------------------------------------------------
@echo If you're not sure press СtrlC.
@echo --------------------------------------------------------------------
@echo --------------------------------------------------------------------
@echo Продолжайте тольке если вы уверены что вы выставили правильный
@echo номер билда и закомитили изменения.
@echo Эта команда пометит текущий trunk в tags/VERSION_bBUILD, если такая
@echo пометка уже существует, то это может привести к проблемам.
@echo --------------------------------------------------------------------
@echo Если вы не уверены, то нажмите CtrlC
@echo --------------------------------------------------------------------
@pause
@tools\m4 -P svn_tag_build.m4
