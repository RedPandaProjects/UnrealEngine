VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "UnrealTools"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
'---------------------------------------
' Unreal Tools Class
'
' This implements all UnrealEd tool
' registry functions does generic
' tool-related work.
'---------------------------------------

Option Explicit

'
' Information about one tool
'
Private Type ToolRecord
    Tool As String ' Tool name
    Handler As Object ' Class it's in
    PropertiesName As String ' For left-click menu properties choice
    ActivateName As String ' For left-click menu activate choice
End Type

'
' Array of tools
'
Public MaxToolRecords As Integer
Public NumToolRecords As Integer
Private ToolRecords() As ToolRecord

'
' Common tools
'
Private ToolBrokenInstance As New ToolBroken
Private ToolBrushInstance As New ToolBrush
Private ToolCmdInstance As New ToolCmd
Private ToolModeInstance As New ToolMode
Private ToolToggleInstance As New ToolToggle

Private ToolAscImportInstance As Object

'
' Register a new tool.  This is called by
' ToolHandlerClass.Init().
'
Public Sub Register(Handler As Object, _
    Tool As String, ActivateName As String, _
    PropertiesName As String)
    '
    If NumToolRecords < MaxToolRecords Then
        '
        With ToolRecords(NumToolRecords)
            Set .Handler = Handler
            .Tool = UCase(Tool)
            .PropertiesName = PropertiesName
            .ActivateName = ActivateName
        End With
        '
        NumToolRecords = NumToolRecords + 1
        '
    End If
    '
End Sub

'
' Initialize all class info.
'
Private Sub Class_Initialize()
    '
    MaxToolRecords = 256
    NumToolRecords = 0
    '
    ReDim ToolRecords(MaxToolRecords)
    '
End Sub

'
' Initialize all tools
'
Public Function InitTools(Ed As Object)
    '
    Dim Test As Object
    '
    ' Call all tool init functions.  Must call
    ' "broken" tool first to assure it's element 0.
    '
    Call ToolBrokenInstance.InitTools(Ed)
    Call ToolBrushInstance.InitTools(Ed)
    Call ToolCmdInstance.InitTools(Ed)
    Call ToolModeInstance.InitTools(Ed)
    Call ToolToggleInstance.InitTools(Ed)
    '
    'Set ToolAscImportInstance = CreateObject("UnrealEdAscImport.UnrealEdAscImportTool")
    'Call ToolAscImportInstance.InitTools(Ed)
    '
End Function

'
' Find a tool handler by name
'
Public Function Handlers(ByVal Tool As String) As Object
    Dim i As Integer
    '
    ' Find the tool:
    '
    Tool = UCase(Tool)
    For i = 0 To NumToolRecords - 1
        If ToolRecords(i).Tool = Tool Then
            Set Handlers = ToolRecords(i).Handler
            GoTo Out
        End If
    
    Next i
    '
    ' Tool not found. Return "broken" tool.
    ' This prevents UnrealEd from crashing.
    '
    Set Handlers = ToolRecords(0).Handler
Out:
End Function

'
' Find a tool handler by name property-menu-choice
' name and activate-menu-choice name
'
Public Sub GetNames(ByVal Tool As String, PropertiesName As String, ActivateName As String)
    Dim i As Integer
    Tool = UCase(Tool)
    For i = 0 To NumToolRecords - 1
        If ToolRecords(i).Tool = Tool Then
            PropertiesName = ToolRecords(i).PropertiesName
            ActivateName = ToolRecords(i).ActivateName
            GoTo Out
        End If
        Next i
    PropertiesName = ""
    ActivateName = ""
Out:
End Sub

Public Sub Highlight(Tool As String, Highlight As Boolean)
    Dim i As Integer
    '
    GToolClicking = GToolClicking + 1 ' Prevent recursion war
    '
    ' Highlight toolbar tool:
    '
    For i = 0 To ToolbarCount - 1
        If frmMain.ToolIcons(i).Tag = Tool Then
            frmMain.ToolIcons(i).Value = Highlight
        End If
    Next i
    '
    GToolClicking = GToolClicking - 1
    '
End Sub

'
' Force new high and low pictures into a tool
'
Public Sub ForceNewPictures(ByVal Tool As String, ByRef Pic As Object, ByRef HiPic As Object)
    Dim i As Integer
    '
    GToolClicking = GToolClicking + 1 ' Prevent recursion war
    For i = 0 To ToolbarCount - 1
        If frmMain.ToolIcons(i).Tag = Tool Then
            frmMain.ToolIcons(i).PictureUp = Pic
            frmMain.ToolIcons(i).PictureDn = HiPic
        End If
    Next i
    '
    GToolClicking = GToolClicking - 1
End Sub

'
' Get all of the pictures for a tool.
' This is specific to the SSRibon control.
'
Public Sub GetPicture(ByVal Tool As String, PicDest As Object)
    Dim P As Object
    '
    If Tool = "" Then
        Set PicDest.PictureUp = LoadPicture("")
        Set PicDest.PictureDn = LoadPicture("")
    Else
        Set P = Ed.Tools.Handlers(Tool).Pic(Tool)
        If Not P Is Nothing Then
            Set PicDest.PictureUp = P
        Else
            Set PicDest.PictureUp = Clipboard.GetData(8)
        End If
        '
        Set P = Ed.Tools.Handlers(Tool).HiPic(Tool)
        If Not P Is Nothing Then
            Set PicDest.PictureDn = P
        Else
            Set PicDest.PictureDn = Clipboard.GetData(8)
        End If
    End If
    '
End Sub

Private Sub Class_Terminate()
    Dim i As Integer
    '
    ' Free references to all stuff
    '
    For i = 0 To NumToolRecords
        Set ToolRecords(i).Handler = Nothing
    Next i
End Sub

Public Sub Click(Tool As String)
    Call Handlers(Tool).DoClick(Tool, Ed)
End Sub

