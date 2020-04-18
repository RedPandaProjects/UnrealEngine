VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolBroken"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
'-----------------------------------------
' Tool class for the bogus tool
'
' This tool doesn't do anything; it just
' takes the place of any damaged tools
' that won't work for some reason.
'-----------------------------------------

Option Explicit

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Value = False
    Status = "Out Of Order"
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    ToolHelp (400) ' Broken tool help
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    '
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Call MsgBox("This tool is broken", 0, "Tool problem")
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    ' Return broken disk picture
    Set HiPic = frmMain.Pics.GraphicCell(63)
End Property

Property Get Pic(ByVal Tool As String) As Object
    ' Return broken disk picture
    Set Pic = frmMain.Pics.GraphicCell(63)
End Property

Public Sub InitTools(UnrealEdApp As UnrealEdApp)
    Call UnrealEdApp.Tools.Register(Me, "BROKEN", "Out of order", "")
End Sub
