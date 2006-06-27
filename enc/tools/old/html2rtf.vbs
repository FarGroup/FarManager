Directory = "D:\pluginsr.help\Project\"
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

set w=CreateObject("Word.Application")
'Set StdOut = WScript.StdOut

n = i - 1
For i = 0 To n
    w.Documents.Open(Directory & "rtf" & files(i)).SaveAs (Directory & "rtf" & filesr(i)), 6
    WScript.Echo files(i)

'    If Selection.Style = "Заголовок 1" Then
'      Selection.MoveDown Unit:=wdLine, Count:=2, Extend:=wdExtend
'      Selection.ParagraphFormat.KeepWithNext = wdToggle
'    Else
'        ddd = files(i)
'    End If
Next

w.Quit (0)
Set w = Nothing

For i = 0 To n
    Set MyFile = fs.GetFile(Directory & "rtf" & files(i))
    MyFile.Delete
Next
