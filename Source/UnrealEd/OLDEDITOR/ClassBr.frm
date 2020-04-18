VERSION 4.00
Begin VB.Form frmClassBrowser 
   BorderStyle     =   0  'None
   Caption         =   "Class Browser"
   ClientHeight    =   6450
   ClientLeft      =   2910
   ClientTop       =   6030
   ClientWidth     =   2460
   Height          =   6915
   HelpContextID   =   329
   Left            =   2850
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6450
   ScaleWidth      =   2460
   ShowInTaskbar   =   0   'False
   Top             =   5625
   Width           =   2580
   Begin VB.PictureBox ClassHolder 
      BackColor       =   &H00C0C0C0&
      Height          =   4575
      Left            =   30
      ScaleHeight     =   4515
      ScaleWidth      =   2355
      TabIndex        =   0
      Top             =   60
      Width           =   2415
      Begin MSOutl.Outline Classes 
         Height          =   4035
         Left            =   0
         TabIndex        =   1
         Top             =   0
         Width           =   2355
         _Version        =   65536
         _ExtentX        =   4154
         _ExtentY        =   7117
         _StockProps     =   77
         ForeColor       =   -2147483640
         BackColor       =   12632256
         BorderStyle     =   0
         Style           =   2
         PicturePlus     =   "ClassBr.frx":0000
         PictureMinus    =   "ClassBr.frx":00FA
      End
   End
   Begin Threed.SSPanel ClassPanel 
      Height          =   1755
      Left            =   0
      TabIndex        =   2
      Top             =   4680
      Width           =   2475
      _Version        =   65536
      _ExtentX        =   4366
      _ExtentY        =   3096
      _StockProps     =   15
      Begin VB.CommandButton Close 
         Cancel          =   -1  'True
         Caption         =   "&Close"
         Height          =   255
         Left            =   1680
         TabIndex        =   12
         Tag             =   "Close this window"
         Top             =   1500
         Width           =   795
      End
      Begin VB.CommandButton Make 
         Caption         =   "&Make"
         Height          =   255
         Left            =   900
         TabIndex        =   3
         Top             =   1500
         Width           =   795
      End
      Begin VB.CommandButton ExportClass 
         Caption         =   "Ex&port..."
         Height          =   255
         Left            =   1080
         TabIndex        =   4
         Tag             =   "Export actor classes as text"
         Top             =   1140
         Width           =   1395
      End
      Begin VB.CommandButton ImportClass 
         Caption         =   "&Import..."
         Height          =   255
         Left            =   0
         TabIndex        =   5
         Tag             =   "Import actor classes as text"
         Top             =   1140
         Width           =   1095
      End
      Begin VB.CommandButton SaveClass 
         Caption         =   "&Save As..."
         Height          =   255
         Left            =   1080
         TabIndex        =   6
         Tag             =   "Save actor classes"
         Top             =   900
         Width           =   1395
      End
      Begin VB.CommandButton LoadClass 
         Caption         =   "&Load..."
         Height          =   255
         Left            =   0
         TabIndex        =   7
         Tag             =   "Load actor classes"
         Top             =   900
         Width           =   1095
      End
      Begin VB.CommandButton NewClass 
         Caption         =   "Create &New Class Below"
         Height          =   255
         Left            =   0
         TabIndex        =   8
         Tag             =   "Creates a new actor class"
         Top             =   540
         Width           =   2475
      End
      Begin VB.CommandButton EditDefActor 
         Caption         =   "&Default Props"
         Enabled         =   0   'False
         Height          =   255
         Left            =   1020
         TabIndex        =   9
         Tag             =   "Edit this class's default actor"
         Top             =   300
         Width           =   1455
      End
      Begin VB.CommandButton Refresh 
         Caption         =   "&Refresh"
         Height          =   255
         Left            =   0
         TabIndex        =   11
         Tag             =   "Refresh the class list"
         Top             =   1500
         Width           =   915
      End
      Begin VB.CommandButton EditScript 
         Caption         =   "&Edit Script"
         Height          =   255
         Left            =   0
         TabIndex        =   10
         Tag             =   "Edit this class's script"
         Top             =   300
         Width           =   1035
      End
      Begin VB.Label ClassDescr 
         Alignment       =   2  'Center
         BackStyle       =   0  'Transparent
         Caption         =   "This class is built-in."
         Height          =   285
         Left            =   0
         TabIndex        =   13
         Top             =   60
         Width           =   2415
      End
   End
End
Attribute VB_Name = "frmClassBrowser"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'
' Class browser
'
Option Explicit

'
' Public
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "ClassBrowser", TOP_BROWSER)
    ClassPanel.Top = Height - ClassPanel.Height
    ClassHolder.Height = Height - ClassPanel.Height
    Classes.Height = Height - ClassPanel.Height
    Refresh_Click
End Sub

Public Sub BrowserShow()
    Show
End Sub

Public Sub BrowserRefresh()
    '
End Sub

Public Sub BrowserHide()
    'Hide
    'Unload Me
End Sub

Public Function GetCurrent() As String
    Dim Result As String
    Result = Classes.List(Classes.ListIndex)
    If InStr(Result, Chr(9)) Then
        Result = Left(Result, InStr(Result, Chr(9)) - 1)
    End If
    If Left(Result, 1) = "*" Then
        Result = Mid(Result, 2)
    End If
    GetCurrent = Trim(Result)
End Function

'
' Private
'
Public Sub EditDefActor_Click()
    frmActorProperties.GetClassDefaultActor (GetCurrent())
End Sub

Sub LaunchScriptEd(ClassName As String, NewText As String, Cursor As Integer)
    Dim F As frmScriptEd
    Dim i As Integer
    '
    For i = 0 To GNumMiscForms - 1
        If GMiscForms(i).Caption = ClassName Then
            GMiscForms(i).SetFocus
            Exit Sub
        End If
    Next i
    '
    Set F = New frmScriptEd
    F.Caption = ClassName
    Call AddMiscForm(F)
    '
    If NewText = "" Then ' Get text from script
        F.Editbox.Text = Ed.Server.GetProp("TEXT", ClassName)
        F.Editbox.SelStart = Val(Ed.Server.GetProp("TEXTPOS", ClassName))
    Else ' Newly created script
        F.Editbox.Text = NewText
        F.Editbox.SelStart = Cursor
        Call Ed.Server.SetProp("TEXT", ClassName, NewText)
    End If
    F.ResultBox = "No compile results"
    F.Show
End Sub

Private Sub Classes_DblClick()
    EditScript_Click
End Sub

Public Sub Delete_Click()
    If ClassIsScript() Then
        If MsgBox("Deleting class " & GetCurrent() & _
            " will delete all actors belonging to the class, " & _
            " as well as all child classes.  Are you sure you " & _
            " want to delete it?", 4, _
            "Delete Class " & GetCurrent()) = 6 Then
            Ed.Server.Exec "CLASS DELETE NAME=" & GetCurrent()
            Refresh_Click
        End If
    End If
End Sub

Private Sub Classes_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        If ClassIsScript() Then
            frmPopups2.cbDelete.Visible = True
            frmPopups2.ClassEditScript.Visible = True
            frmPopups2.ClassEditScript.Caption = "&Edit " & GetCurrent() & " Script"
        Else
            frmPopups2.cbDelete.Visible = False
            frmPopups2.ClassEditScript.Visible = False
        End If
        frmPopups2.ClassEditActor.Caption = "Default " & GetCurrent() & " &Properties..."
        frmPopups2.ClassCreateNew.Caption = "Create new class below " & GetCurrent()
        PopupMenu frmPopups2.ClassBrowser
    End If
End Sub

Private Sub Close_Click()
    Hide
End Sub

Private Sub Command1_Click()
    Dim r As RECT
    Call GetClientRect(frmGeneric.hwnd, r)
    Call SetWindowPos(Me.hwnd, 0, 0, 0, r.Right - r.Left, r.Bottom - r.Top, 0)
End Sub

Public Sub ExportClass_Click()
    PrepareForSave
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub SelOpt_Click()
    If Height <> 6765 Then
        Height = 6765
    Else
        Height = 7935
    End If
End Sub

Public Sub EditScript_Click()
    If GetCurrent() <> "" And ClassIsScript() Then
        Call LaunchScriptEd(GetCurrent(), "", 0)
    End If
End Sub

Private Sub Classes_Click()
    Dim Temp As String
    '
    Temp = GetCurrent()
    If Temp <> "" Then
        frmMain.ActorCombo.List(0) = Temp
        frmMain.ActorCombo.ListIndex = 0
        Ed.Server.Exec "ACTOR SET ADDCLASS=" & Temp
    End If
    SetActorCaption
End Sub

Private Sub Classes_Expand(ListIndex As Integer)
    Dim Text As String
    Dim ClassName As String
    Text = Classes.List(ListIndex)
    ClassName = GrabString(Text)
    Call ExpandOutline(Classes, "Class", "QueryRes", "CLASS QUERY PARENT=" & ClassName, ListIndex, 0)
End Sub

Public Sub ImportClass_Click()
    Dim Fnames As String
    '
    ' Dialog for "Import Actor Class":
    '
    Ed.Server.Disable
    frmDialogs.ClassImport.filename = ""
    frmDialogs.ClassImport.InitDir = Ed.BaseDir + Ed.ActorDir
    frmDialogs.ClassImport.Action = 1 'Modal File-Open Box
    Ed.Server.Enable
    '
    If (frmDialogs.ClassImport.filename <> "") Then
        Screen.MousePointer = 11
        Fnames = Trim(frmDialogs.ClassImport.filename)
        While (Fnames <> "")
            Ed.Server.Exec "RES IMPORT FILE=" & Quotes(GrabFname(Fnames))
        Wend
        Refresh_Click
        Screen.MousePointer = 0
    End If
End Sub

Public Sub LoadClass_Click()
    Dim Fnames As String
    '
    ' Dialog for "Load Actor Class":
    '
    Ed.Server.Disable
    frmDialogs.ClassLoad.filename = ""
    frmDialogs.ClassLoad.InitDir = Ed.BaseDir + Ed.ActorDir
    frmDialogs.ClassLoad.Action = 1 'Modal File-Open Box
    Ed.Server.Enable
    '
    If (frmDialogs.ClassLoad.filename <> "") Then
        Screen.MousePointer = 11
        Fnames = Trim(frmDialogs.ClassLoad.filename)
        While (Fnames <> "")
            Ed.Server.Exec "RES LOAD FILE=" & Quotes(GrabFname(Fnames))
        Wend
        Refresh_Click
        Screen.MousePointer = 0
    End If
    Refresh_Click
End Sub

Public Sub NewClass_Click()
    Dim S As String
    If Classes.ListIndex >= 0 Then
        frmNewClass.ParentClassName = GetCurrent()
        frmNewClass.NewClassName = "My" & GetCurrent()
        Ed.Server.Disable
        frmNewClass.Show 1
        Ed.Server.Enable
        If GResult = 1 Then
            If Val(Ed.Server.GetProp("CLASS", "EXISTS NAME=" & Quotes(GString))) = 1 Then
                If MsgBox( _
                    "An actor class named " & GString & _
                    " Already exists.  Do you want to replace it?  Replacing an " & _
                    "existing class causes all of its child classes to be copied " & _
                    "over to the new class.", 4, _
                    "Class already exists") = 7 Then
                    Exit Sub
                End If
            End If
            Ed.Server.Exec "CLASS NEW NAME=" & Quotes(GString) & " PARENT=" & Quotes(GetCurrent())
            S = "Class " & GString & " Expands " & GetCurrent() & _
                Chr(13) & Chr(10) & Chr(13) & Chr(10)
            frmMain.ActorCombo.List(0) = GString
            Call ExpandOutline(frmClassBrowser.Classes, "Class", "QueryRes", "CLASS QUERY PARENT=" & GetCurrent(), frmClassBrowser.Classes.ListIndex, 1)
            Call SetCurrent
            Call LaunchScriptEd(GString, S, Len(S))
        End If
    End If
End Sub

Private Sub Refresh_Click()
    Dim i As Integer
    Dim N As Integer
    Call InitOutline
    '
    QuerySource = -1
    Classes.Clear
    Classes.AddItem "Root"
    Classes.ListIndex = 0
    '
    QuerySource = Classes.ListIndex
    Ed.Server.Exec "CLASS QUERY PARENT=ROOT"
    Call UpdateOutline(Classes, "Class", "QueryRes")
    '
    SetCurrent
End Sub

Sub SetCurrent()
    Dim N As Integer
    Dim i As Integer
    '
    N = Len(frmMain.ActorCombo.Text)
    For i = 0 To Classes.ListCount - 1
        If Mid(Classes.List(i), 1, N) = frmMain.ActorCombo.Text Then
            Classes.ListIndex = i
        ElseIf Mid(Classes.List(i), 2, N) = frmMain.ActorCombo.Text Then
            Classes.ListIndex = i
        End If
    Next i
    SetActorCaption
End Sub

Function ClassIsScript() As Boolean
    If Classes.ListIndex < 0 Then
        ClassIsScript = False
    ElseIf Left(Classes.List(Classes.ListIndex), 1) = "*" Then
        ClassIsScript = True
    Else
        ClassIsScript = False
    End If
End Function

Sub SetActorCaption()
    If ClassIsScript() Then
        EditScript.Enabled = True
        ClassDescr.Caption = "This class is scripted."
    Else
        EditScript.Enabled = False
        ClassDescr.Caption = "This class is built-in."
    End If
End Sub

Public Sub SaveClass_Click()
    Dim CurClass As String
    Dim Fname As String
    '
    CurClass = GetCurrent()
    If CurClass = "" Then Exit Sub
    '
    If ClassIsScript Then
        frmSaveClass.ClassThis.Value = True
        frmSaveClass.ClassThis.Enabled = True
    Else
        frmSaveClass.ClassBelow.Value = True
        frmSaveClass.ClassThis.Enabled = False
    End If
    frmSaveClass.ClassThis.Caption = "Just class " & CurClass
    frmSaveClass.ClassBelow.Caption = "Class " & CurClass & " and all scripted classes beneath it"
    frmSaveClass.ClassName = CurClass
    Ed.Server.Disable
    frmSaveClass.Show 1
    Ed.Server.Enable
    If GlobalAbortedModal Then Exit Sub
    '
    If InStr(CurClass, " ") > 0 Then
        Fname = Left(CurClass, InStr(CurClass, " ") - 1)
    Else
        Fname = CurClass
    End If
    frmDialogs.ClassSave.filename = Trim(Fname) & ".ucx"
    On Error GoTo BadFilename
TryAgain:
    Ed.Server.Disable
    frmDialogs.ClassSave.DialogTitle = "Save " & CurClass & " actor class"
    frmDialogs.ClassSave.InitDir = Ed.BaseDir + Ed.ActorDir
    frmDialogs.ClassSave.DefaultExt = "ucx"
    frmDialogs.ClassSave.Flags = 2 'Prompt if overwrite
    frmDialogs.ClassSave.Action = 2 'Modal Save-As Box
    Ed.Server.Enable
    On Error GoTo 0
    Fname = frmDialogs.ClassSave.filename
    If (Fname <> "") Then
        PrepareForSave
        If GResult = 1 Then
            Ed.Server.Exec "CLASS SAVE NAME=" & Quotes(CurClass) & " FILE=" & Quotes(Fname)
        Else
            Ed.Server.Exec "CLASS SAVEBELOW NAME=" & Quotes(CurClass) & " FILE=" & Quotes(Fname)
        End If
    End If
    Exit Sub
BadFilename: ' Bad filename handler:
    Fname = ""
    frmDialogs.ClassSave.filename = ""
    On Error GoTo 0
    GoTo TryAgain
End Sub

