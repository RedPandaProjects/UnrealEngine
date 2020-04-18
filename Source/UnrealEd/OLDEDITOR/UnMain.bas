Attribute VB_Name = "UNMAIN"
Option Explicit

Global Ed As UnrealEdApp

'
' Misc global constants
'
Global Const NoneString = "(none)"
Global Const AllString = "(All)"

Global GlobalError As String
Global GlobalResume As Integer
Global GlobalRestart As Integer
Global GlobalErr As Integer
Global GlobalAbortedModal As Integer

Global GToolClicking As Integer
Global GResult As Integer
Global GFindResult As Integer
Global GTemp As Integer
Global GImportExisting As Integer
Global GSettingMode As Integer
Global GResizingAll As Integer
Global GString As String
Global GPackage As String
Global GNumMiscForms As Integer
Global GPopupActorClass As String
Global GPolyPropsAction As Integer
Global GMiscForms(100) As Variant
Global ScriptEdTop As Integer
Global ScriptEdLeft As Integer

'
' Move to tools:
'
Global Const ToolGridX = 3
Global Const ToolGridY = 15

'
' Toolbar globals
'
Global PopupToolName As String
Global PopupToolControl As Control
Global PopupToolMoveable As Integer
Global PopupToolIndex As Integer
Global PopupNext As Integer
Global ToolbarCount As Integer

Global Const ToolModule = "" ' For bogus undefs.RegisterAllTools
Global Const ToolNames = ""

Sub InitApp()
    App.OleRequestPendingMsgText = _
        "The Unreal Server is busy working on your request.  Click OK to continue."
    App.OleRequestPendingMsgTitle = _
       "Server at work"
    App.OleServerBusyMsgText = _
       "The Unreal Server is busy working on a task.  Click OK to continue."
    App.OleServerBusyMsgTitle = _
       "Server at work"
    App.OleServerBusyRaiseError = False
    App.OleRequestPendingTimeout = 4 * 1000
    App.OleServerBusyTimeout = 4 * 1000
End Sub

Sub LeftClickTool(ByVal Tool As String, Source As Form)
    '
    Dim PropertiesName As String
    Dim ActivateName As String
    '
    ' Get name of 'properties' choice:
    '
    Call Ed.Tools.GetNames(Tool, PropertiesName, ActivateName)
    '
    frmPopups.ToolbarDo.Caption = ActivateName
    frmPopups.ToolbarProperties.Caption = PropertiesName
    frmPopups.ToolbarProperties.Visible = PropertiesName <> ""
    '
    PopupToolName = Tool
    '
    PopupNext = 0
    Source.PopupMenu frmPopups.Toolbar
    '
    If (PopupNext = 1) Then
        frmMain.PopupMenu frmMain.SelectDialog
    End If
    '
End Sub

'
' Bring up context-sensitive help for
' a tool or editing mode.
'
Sub ToolHelp(ContextID As Integer)
    '
    frmDialogs.ToolHelp.HelpContext = ContextID
    frmDialogs.ToolHelp.ShowHelp
    '
End Sub

Sub AddMiscForm(F As Form)
    Set GMiscForms(GNumMiscForms) = F
    GNumMiscForms = GNumMiscForms + 1
End Sub

Sub RemoveMiscForm(F As Variant)
    Dim i As Integer, j As Integer, Found As Integer
    Dim Temp As Form
    '
    For i = 0 To GNumMiscForms - 1
        If GMiscForms(i).Caption = F.Caption Then
            Set GMiscForms(i) = Nothing
            Found = 1
        Else
            Set Temp = GMiscForms(i)
            Set GMiscForms(i) = Nothing
            Set GMiscForms(j) = Temp
            j = j + 1
        End If
    Next i
    GNumMiscForms = j
    If Found = 0 Then MsgBox ("RemoveMiscForm: Form not found")
End Sub

Sub UnloadMiscForms()
    Dim i As Integer
    For i = GNumMiscForms - 1 To 0 Step -1
        Debug.Print "Unloading " & GMiscForms(i).Tag
        Unload GMiscForms(i)
    Next i
    If GNumMiscForms <> 0 Then MsgBox ("UnloadMiscForms: Failed RemoveMiscForm")
End Sub

Sub PreSaveAll()
    Dim i As Integer
    For i = 0 To GNumMiscForms - 1
        Call GMiscForms(i).PreSave
    Next i
End Sub

Sub PostLoad()
    Dim i As Integer
    For i = 0 To GNumMiscForms - 1
        Call GMiscForms(i).PostLoad
    Next i
End Sub

Sub UpdateDialog(c As Control)
    Dim S As String
    If c.filename <> "" Then
        S = c.filename
        While S <> "" And Right(S, 1) <> "\"
            S = Left(S, Len(S) - 1)
        Wend
        c.InitDir = S
    End If
    ChDir Ed.BaseDir
End Sub

