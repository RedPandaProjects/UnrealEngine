VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolToggle"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
'-----------------------------------------
' Tool class for all toggle tools (TOGGLE)
'-----------------------------------------

Option Explicit
Dim Gridding As Boolean

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Select Case (Tool)
    Case "GRID":
        Value = Ed.GridMode
        Status = "Grid is " + OnOff(Value)
    Case "ROTGRID":
        Value = Ed.RotGridMode
        Status = "Rotation grid is " + OnOff(Value)
    Case "SPEED":
        If Ed.SpeedMode = 0 Then
            Status = "Slow speed"
        ElseIf Ed.SpeedMode = 1 Or Ed.SpeedMode = 3 Then
            Status = "Medium speed"
        Else
            Status = "Fast speed"
        End If
        Value = False
    Case "SNAPVERTEX":
        Value = Ed.SnapVertex
        Status = "Vertex snapping is " + OnOff(Value)
    End Select
    '
    Call Ed.StatusText(Status)
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "GRID": ToolHelp (111)
    Case "ROTGRID": ToolHelp (112)
    Case "SNAPVERTEX": ToolHelp (405)
    Case "SPEED": ToolHelp (315)
    End Select
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    End Select
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    '
    Dim Highlight As Boolean
    Dim Temp As String
    '
    Select Case (Tool)
    Case "GRID":
        If Not Gridding Then
            Gridding = True
            Ed.GridMode = Not Ed.GridMode
            Ed.ServerExec "MODE GRID=" + OnOff(Ed.GridMode)
            Call Ed.SetGridSize(16, 16, 16)
            Ed.SetGridMode (1)
            Gridding = False
        End If
    Case "ROTGRID":
        Ed.RotGridMode = Not Ed.RotGridMode
        Ed.ServerExec "MODE ROTGRID=" + OnOff(Ed.RotGridMode)
    Case "SNAPVERTEX":
        Ed.SnapVertex = Not Ed.SnapVertex
        Ed.ServerExec "MODE SNAPVERTEX=" + OnOff(Ed.SnapVertex)
    Case "SPEED":
        Ed.SpeedMode = Ed.SpeedMode + 1
        If Ed.SpeedMode > 2 Then Ed.SpeedMode = 0
        '
        If Ed.SpeedMode = 0 Then
            Ed.ServerExec "MODE SPEED=1"
        ElseIf Ed.SpeedMode = 1 Or Ed.SpeedMode = 3 Then
            Ed.ServerExec "MODE SPEED=4"
        Else
            Ed.ServerExec "MODE SPEED=16"
        End If
        ' Update picture
        Call Ed.Tools.ForceNewPictures("SPEED", _
            frmMain.Pics.GraphicCell(29 + Ed.SpeedMode), _
            frmMain.HiPics.GraphicCell(29 + Ed.SpeedMode))
    End Select
    '
    ' Set new highlighting:
    '
    Call GetStatus(Tool, UnrealEdApp, Temp, Highlight)
    Call Ed.Tools.Highlight(Tool, Highlight)
    '
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "GRID": V = 32
    Case "ROTGRID": V = 35
    Case "SPEED": V = 30
    Case "SNAPVERTEX": V = 26
    End Select
    '
    Set HiPic = frmMain.HiPics.GraphicCell(V)
End Property

Property Get Pic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "GRID": V = 32
    Case "ROTGRID": V = 35
    Case "SPEED": V = 30
    Case "SNAPVERTEX": V = 26
    End Select
    '
    Set Pic = frmMain.Pics.GraphicCell(V)
End Property

Public Sub InitTools(UnrealEdApp As UnrealEdApp)
    Call UnrealEdApp.Tools.Register(Me, "GRID", "Toggle grid mode", "Grid Settings...")
    Call UnrealEdApp.Tools.Register(Me, "ROTGRID", "Toggle rotation grid", "Rotation Grid Settings...")
    Call UnrealEdApp.Tools.Register(Me, "SNAPVERTEX", "Toggle vertex snap", "Grid Settings...")
    Call UnrealEdApp.Tools.Register(Me, "SPEED", "Camera speed", "")
End Sub
