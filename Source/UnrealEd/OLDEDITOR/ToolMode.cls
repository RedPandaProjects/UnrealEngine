VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolMode"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
'---------------------------------------
' Tool class for all mode tools (MODE)
'---------------------------------------

Option Explicit

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Status = ""
    Value = False
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "CAMERAMOVE": ToolHelp (301)
    Case "CAMERAZOOM": ToolHelp (302)
    Case "BRUSHROTATE": ToolHelp (305)
    Case "BRUSHSHEER": ToolHelp (306)
    Case "BRUSHSCALE": ToolHelp (307)
    Case "BRUSHSNAP": ToolHelp (331)
    Case "BRUSHSTRETCH": ToolHelp (308)
    Case "TEXTUREPAN": ToolHelp (311)
    Case "TEXTURESET": ToolHelp (312)
    Case "TEXTUREROTATE": ToolHelp (313)
    Case "TEXTURESCALE": ToolHelp (314)
    End Select
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "TEXTUREPAN": frmSurfaceProps.Show
    Case "TEXTURESET": frmSurfaceProps.Show
    Case "TEXTUREROTATE": frmSurfaceProps.Show
    Case "TEXTURESCALE": frmSurfaceProps.Show
    End Select
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    '
    Dim i As Integer
    '
    ' Unhighlight previous mode:
    '
    Call Ed.Tools.Highlight(Ed.ToolMode, False)
    '
    Select Case (Tool)
    Case Else:
        Ed.ServerExec "MODE " + Tool
        Ed.ToolMode = "MODE:" + Tool
    End Select
    '
    ' Set and highlight new mode:
    '
    Ed.ToolMode = Tool
    Call Ed.Tools.Highlight(Ed.ToolMode, True)
    '
    Select Case (Tool)
        Case "CAMERAMOVE": i = 0
        Case "CAMERAZOOM": i = 1
        Case "BRUSHROTATE": i = 2
        Case "BRUSHSHEER": i = 4
        Case "BRUSHSCALE": i = 3
        Case "BRUSHSTRETCH": i = 5
        Case "BRUSHSNAP": i = 6
        Case "TEXTUREPAN": i = 10
        Case "TEXTURESET": i = 10
        Case "TEXTUREROTATE": i = 11
        Case "TEXTURESCALE": i = 10
    End Select
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "CAMERAMOVE": V = 0
    Case "CAMERAZOOM": V = 1
    Case "BRUSHROTATE": V = 4
    Case "BRUSHSHEER": V = 5
    Case "BRUSHSCALE": V = 6
    Case "BRUSHSTRETCH": V = 7
    Case "BRUSHSNAP": V = 27
    Case "TEXTUREPAN": V = 16
    Case "TEXTURESET": V = 17
    Case "TEXTUREROTATE": V = 18
    Case "TEXTURESCALE": V = 19
    End Select
    '
    Set HiPic = frmMain.HiPics.GraphicCell(V)
End Property

Property Get Pic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "CAMERAMOVE": V = 0
    Case "CAMERAZOOM": V = 1
    Case "BRUSHROTATE": V = 4
    Case "BRUSHSHEER": V = 5
    Case "BRUSHSCALE": V = 6
    Case "BRUSHSTRETCH": V = 7
    Case "BRUSHSNAP": V = 27
    Case "TEXTUREPAN": V = 16
    Case "TEXTURESET": V = 17
    Case "TEXTUREROTATE": V = 18
    Case "TEXTURESCALE": V = 19
    End Select
    '
    Set Pic = frmMain.Pics.GraphicCell(V)
End Property

Public Sub InitTools(UnrealEdApp As UnrealEdApp)
    Call UnrealEdApp.Tools.Register(Me, "CAMERAMOVE", "Move camera", "")
    Call UnrealEdApp.Tools.Register(Me, "CAMERAZOOM", "Zoom camera", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHROTATE", "Rotate brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSHEER", "Sheer brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSCALE", "Scale brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSTRETCH", "Stretch brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTUREPAN", "Pan textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTURESET", "Set textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTUREROTATE", "Rotate textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "TEXTURESCALE", "Scale textures", "Surface properties...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSHSNAP", "SnapScale Brush", "Brush properties...")
End Sub
