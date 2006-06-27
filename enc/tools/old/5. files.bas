Sub PlugRinG()
'
' PlugRinG Макрос
' Макрос записан 23.05.2000 Cail Lomecb
'   Скорректирован By SVS 31.08.2000
'    задолбался все время обновлять...
' VB MD!

'Directory = "d:\helps\pluginsr.help\"
Directory = "D:\FAR\pluginsr.help\yole\enc\"
FileIndex = Directory + "_tools\basfiles.lst"

Dim files(1500)
Dim filesr(1500)

Dim fs, f
Set fs = CreateObject("Scripting.FileSystemObject")
Set f = fs.OpenTextFile(FileIndex, 1, 0, 0)
i = 0
Do While f.AtEndOfLine <> True
    files(i) = f.ReadLine
    filesr(i) = f.ReadLine
    i = i + 1
Loop
f.Close

n = i - 1
For i = 0 To n
    ChangeFileOpenDirectory Directory + "meta"
    Documents.Open filename:=files(i), ConfirmConversions:=False, _
        ReadOnly:=False, AddToRecentFiles:=False, PasswordDocument:="", _
        PasswordTemplate:="", Revert:=False, WritePasswordDocument:="", _
        WritePasswordTemplate:="", Format:=wdOpenFormatAuto

    If Selection.Style = "Заголовок 1" Then
      Selection.MoveDown Unit:=wdLine, count:=2, Extend:=wdExtend
      Selection.ParagraphFormat.KeepWithNext = wdToggle
    End If

    ChangeFileOpenDirectory Directory + "rtf"
    ActiveDocument.SaveAs filename:=filesr(i), FileFormat:= _
        wdFormatRTF, LockComments:=False, Password:="", AddToRecentFiles:=True, _
        WritePassword:="", ReadOnlyRecommended:=False, EmbedTrueTypeFonts:=False, _
         SaveNativePictureFormat:=False, SaveFormsData:=False, SaveAsAOCELetter:= _
        False
    ActiveDocument.Close
Next i
End Sub
