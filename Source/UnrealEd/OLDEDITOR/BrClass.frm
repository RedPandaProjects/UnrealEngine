VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Object = "{BE4F3AC8-AEC9-101A-947B-00DD010F7B46}#1.0#0"; "MSOUTL32.OCX"
Begin VB.Form frmClassBrowser 
   BorderStyle     =   0  'None
   Caption         =   "Class Browser"
   ClientHeight    =   6450
   ClientLeft      =   4770
   ClientTop       =   2130
   ClientWidth     =   2475
   HelpContextID   =   329
   Icon            =   "BrClass.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6450
   ScaleWidth      =   2475
   ShowInTaskbar   =   0   'False
   Begin VB.CheckBox ActorsOnly 
      Caption         =   "Only show Actor classes"
      Height          =   255
      Left            =   180
      TabIndex        =   8
      Top             =   60
      Value           =   1  'Checked
      Width           =   2295
   End
   Begin VB.PictureBox ClassHolder 
      BackColor       =   &H00C0C0C0&
      Height          =   5010
      Left            =   0
      ScaleHeight     =   4950
      ScaleWidth      =   2370
      TabIndex        =   0
      Top             =   360
      Width           =   2430
      Begin MSOutl.Outline Classes 
         Height          =   3135
         Left            =   0
         TabIndex        =   1
         Top             =   0
         Width           =   2415
         _Version        =   65536
         _ExtentX        =   4260
         _ExtentY        =   5530
         _StockProps     =   77
         ForeColor       =   -2147483640
         BackColor       =   12632256
         BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         BorderStyle     =   0
         Style           =   2
         PicturePlus     =   "BrClass.frx":030A
         PictureMinus    =   "BrClass.frx":0404
      End
   End
   Begin Threed.SSPanel ClassPanel 
      Height          =   1155
      Left            =   0
      TabIndex        =   2
      Top             =   5400
      Width           =   2475
      _Version        =   65536
      _ExtentX        =   4366
      _ExtentY        =   2037
      _StockProps     =   15
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Begin VB.CommandButton Delete 
         Caption         =   "Delete"
         Height          =   255
         Left            =   1500
         TabIndex        =   10
         Top             =   780
         Width           =   900
      End
      Begin VB.CommandButton SaveClass 
         Caption         =   "&Save"
         Height          =   255
         Left            =   780
         TabIndex        =   3
         Tag             =   "Save actor classes"
         Top             =   780
         Width           =   735
      End
      Begin VB.CommandButton LoadClass 
         Caption         =   "&Load"
         Height          =   255
         Left            =   0
         TabIndex        =   4
         Tag             =   "Load actor classes"
         Top             =   780
         Width           =   795
      End
      Begin VB.CommandButton ExportAll 
         Caption         =   "Export All"
         Height          =   255
         Left            =   1500
         TabIndex        =   12
         Tag             =   "Save actor classes"
         Top             =   540
         Width           =   900
      End
      Begin VB.CommandButton Export 
         Caption         =   "Export"
         Height          =   255
         Left            =   780
         TabIndex        =   9
         Tag             =   "Save actor classes"
         Top             =   540
         Width           =   720
      End
      Begin VB.CommandButton NewClass 
         Caption         =   "&New..."
         Height          =   255
         Left            =   0
         TabIndex        =   5
         Tag             =   "Creates a new actor class"
         Top             =   540
         Width           =   795
      End
      Begin VB.CommandButton EditDefActor 
         Caption         =   "&Defaults"
         Height          =   255
         Left            =   1200
         TabIndex        =   6
         Tag             =   "Edit this class's default actor"
         Top             =   300
         Width           =   1200
      End
      Begin VB.CommandButton EditScript 
         Caption         =   "&Edit Code"
         Height          =   255
         Left            =   0
         TabIndex        =   7
         Tag             =   "Edit this class's script"
         Top             =   300
         Width           =   1215
      End
      Begin VB.Label ClassDescr 
         Alignment       =   2  'Center
         Caption         =   "This class is scripted"
         Height          =   255
         Left            =   60
         TabIndex        =   11
         Top             =   60
         Width           =   2355
      End
   End
End
Attribute VB_Name = "frmClassBrowser"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Class browser: This is a form that implements
' the browser interface.
'
Option Explicit
Dim PrevTopic As String

Private Sub ActorsOnly_Click()
    Refresh_Click
    Classes.Expand(0) = True
End Sub

Private Sub Export_Click()
    Dim All As Boolean
    If MsgBox("This option will export all modified classes to text .uc files which can later be rebuilt. Do you want to do this?", vbYesNo, "Export classes to *.uc files") = vbYes Then
        PreSaveAll
        Ed.ServerExec "CLASS SPEW"
    End If
End Sub

Private Sub ExportAll_Click()
    Dim All As Boolean
    If MsgBox("This option will export all classes to text .uc files which can later be rebuilt. Do you want to do this?", vbYesNo, "Export classes to *.uc files") = vbYes Then
        PreSaveAll
        Ed.ServerExec "CLASS SPEW ALL"
    End If
End Sub

'
' Public (Browser Interface)
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "ClassBrowser", TOP_BROWSER)
    ClassPanel.Top = Height - ClassPanel.Height
    ClassHolder.Height = Height - ClassPanel.Height - ClassHolder.Top
    Classes.Height = ClassHolder.Height
    Refresh_Click
    Classes.Expand(0) = True
    'If Ed.GodMode = 0 Then
    '    ActorsOnly.Enabled = False
    'End If
End Sub

Public Sub BrowserShow()
    Show
End Sub

Public Sub BrowserRefresh()
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

Public Function GetCurrentPackage() As String
    Dim c As String
    c = GetCurrent()
    GetCurrentPackage = Ed.ServerGetProp("Class", "Package Class=" & c)
End Function

'
' Public (other)
'

Public Sub EditDefActor_Click()
    Ed.ServerExec "HOOK CLASSPROPERTIES CLASS=" & GetCurrent() ''xyzzy
End Sub

Public Sub LaunchScriptEd( _
    ClassName As String, _
    NewText As String, _
    Cursor As Long, _
    ErrorLine As Long, _
    ErrorText As String)

    ' Locals.
    Dim F As frmScriptEd
    Dim Package As String, Caption As String
    Dim Editable As Boolean
    Dim S As String
    Dim i As Long, j As Long

    ' Find package name.
    Package = Ed.ServerGetProp("CLASS", "PACKAGE CLASS=" & ClassName)
    Caption = Package & "." & ClassName
    
    ' Make sure the script exists.
    If Ed.ServerGetProp("SCRIPTPOS", ClassName) <> "" Then
        For i = 0 To GNumMiscForms - 1
            If GMiscForms(i).Caption = Caption Then
                Set F = GMiscForms(i)
                GoTo HaveIt
            End If
        Next i

        Set F = New frmScriptEd
        Call AddMiscForm(F)
        F.Caption = Caption

        If NewText <> "" Then
            ' Creating a new script.
            Call Ed.ServerSetProp("SCRIPT", ClassName, NewText)
            Call Ed.ServerSetProp("SCRIPTPOS", ClassName, Str(Cursor))
        End If

        ' Bring up script.
        F.LoadAll
        F.ResetUndo

HaveIt:
        ' Make read-only if it's not editable.
        'Editable = Ed.GodMode Or (UCase(Package) <> "UNENGINE" And UCase(Package) <> "UNEDITOR" And UCase(Package) <> "UNREALI")
        Editable = True ''!!
        F.EditBox.Locked = Not Editable

        ' Go to error.
        If ErrorLine <> 0 Then
            i = 1
            j = 1
            S = F.EditBox.Text
            While i < ErrorLine
                j = 1 + InStr(j, S, Chr(10))
                If j = 1 Then
                    ' Not found.
                    ErrorLine = 0
                    GoTo ShowIt
                End If
                i = i + 1
            Wend
            i = InStr(j + 1, S, Chr(13))
            If i <> 0 Then i = i - j
            F.ErrorBox.Text = ErrorText
            Beep
        Else
            F.ErrorBox.Text = ""
        End If
ShowIt:
        F.Show
        F.SetFocus
        F.DoResize
        If ErrorLine <> 0 Then Call F.GotoText(j - 1, i)
    End If
End Sub

'
' Private
'

Private Sub Classes_DblClick()
    EditScript_Click
End Sub

Public Sub Delete_Click()
    Dim Result As String
    Dim Cur As String
    Cur = GetCurrent()
    Ed.ServerExec "SETCURRENTCLASS Class=Light"
    Result = Ed.ServerGetProp("Obj", "Delete Class=Class Object=" & Cur)
    If Result = "" Then
        Refresh_Click
    Else
        Call MsgBox(Result, , "Can't delete class")
    End If
End Sub

Private Sub Classes_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        frmPopups2.ClassEditScript.Caption = "&Edit " & GetCurrent() & " Script"
        If ClassIsEditable() Then
            frmPopups2.cbDelete.Visible = True
        Else
            frmPopups2.cbDelete.Visible = False
        End If
        frmPopups2.ClassEditActor.Caption = "Default " & GetCurrent() & " &Properties..."
        frmPopups2.ClassCreateNew.Caption = "Create new class below " & GetCurrent()
        PopupMenu frmPopups2.ClassBrowser
    End If
End Sub

Private Sub SelOpt_Click()
    If Height <> 6765 Then
        Height = 6765
    Else
        Height = 7935
    End If
End Sub

Public Sub EditScript_Click()
    If GetCurrent() <> "" Then
        Call LaunchScriptEd(GetCurrent(), "", 0, 0, "")
    End If
End Sub

Private Sub Classes_Click()
    Dim Temp As String
    '
    Temp = GetCurrent()
    If Temp <> "" Then
        Ed.CurrentClass = Temp
        Ed.ServerExec "SETCURRENTCLASS CLASS=" & Ed.CurrentClass
    End If
    SetActorCaption
End Sub

Private Sub Classes_Expand(ListIndex As Integer)
    Dim Text As String
    Dim ClassName As String
    Text = Classes.List(ListIndex)
    ClassName = GrabString(Text)
    If Left(ClassName, 1) = "*" Then ClassName = Mid(ClassName, 2)
    Call ExpandOutline(Classes, Ed.ServerGetProp( _
        "Class", "Query PARENT=" & ClassName), ListIndex, False)
End Sub

Public Sub LoadClass_Click()
    Dim Fnames As String, Fname As String, Pkg As String, PkgDef As String, NameDef As String
    Dim i As Integer

    ' Dialog for "Load Class".
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.ClassLoad.filename = ""
    frmDialogs.ClassSave.InitDir = Ed.BaseDir
    frmDialogs.ClassLoad.ShowOpen

    Call UpdateDialog(frmDialogs.ClassLoad)
    If (frmDialogs.ClassLoad.filename <> "") Then
        Ed.BeginSlowTask "Loading and compiling classes"
        Screen.MousePointer = 11
        Fnames = Trim(frmDialogs.ClassLoad.filename)
        While (Fnames <> "")
            Fname = GrabFname(Fnames)
            PkgDef = ""
            i = InStr(Fname, "\Classes")
            If i > 0 Then
                Pkg = Left(Fname, i - 1)
                While InStr(Pkg, "\") > 0
                    Pkg = Mid(Pkg, InStr(Pkg, "\") + 1)
                Wend
                If Pkg <> "" Then PkgDef = " PACKAGE=" & Pkg

                NameDef = Mid(Fname, i + 9)
                If InStr(NameDef, ".") > 0 Then NameDef = Left(NameDef, InStr(NameDef, ".") - 1)
                If NameDef <> "" Then NameDef = " NAME=" & NameDef
            End If
            Ed.ServerExec "CLASS LOAD FILE=" & Quotes(Fname) & PkgDef & NameDef
        Wend
        frmMain.ScriptMakeChanged_Click
        Refresh_Click
        Classes.Expand(0) = True
        PostLoad
        Screen.MousePointer = 0
        Ed.EndSlowTask
    End If
Skip:
    Ed.ServerEnable
End Sub

Public Sub MakeSubclass(ParentClass As String, Browser As Boolean)
    Dim S As String
    
    ' Show create-new-class dialog
    frmNewClass.ParentClassName = ParentClass
    frmNewClass.NewClassName = "My" & ParentClass
    frmNewClass.NewPackageName = "MyLevel" ' !!fix
    Ed.ServerDisable
    frmNewClass.Show 1
    Ed.ServerEnable
    
    If GResult = 1 Then
        If Val(Ed.ServerGetProp("CLASS", "EXISTS NAME=" & Quotes(GString))) = 1 Then
            
            ' This class already exists.
            If MsgBox( _
                "An actor class named " & GString & _
                " already exists.  Do you want to replace it?  Replacing an " & _
                "existing class causes all of its child classes to be copied " & _
                "over to the new class.", 4, _
                "Class already exists") = 7 Then
                GResult = 0
                Exit Sub
            End If
        End If

        ' Create the new class.
        Ed.ServerExec "CLASS NEW NAME=" & GString & " PACKAGE=" & GPackage & " PARENT=" & ParentClass

        ' Expand the outline.
        If Browser Then
            Call ExpandOutline(Classes, Ed.ServerGetProp( _
                "Class", "Query Parent=" & GetCurrent()), frmClassBrowser.Classes.ListIndex, True)
        End If

        ' Make this the current class.
        Ed.CurrentClass = GString
        Ed.ServerExec "SETCURRENTCLASS CLASS=" & Ed.CurrentClass
        Call SetCurrent

        ' Set starting script text.
        S = _
            "//=============================================================================" & Chr(13) & Chr(10) & _
            "// " & GString & "." & Chr(13) & Chr(10) & _
            "//=============================================================================" & Chr(13) & Chr(10) & _
            "class " & GString & " expands " & ParentClass & ";" & _
            Chr(13) & Chr(10) & Chr(13) & Chr(10)
        
        ' Launch the script editor.
        Call LaunchScriptEd(GString, S, Len(S), 0, "")
    End If
End Sub

Public Sub NewClass_Click()
    If Classes.ListIndex >= 0 Then
        Call MakeSubclass(GetCurrent(), True)
    End If
End Sub

Private Sub Refresh_Click()
    Dim i As Integer
    Dim N As Integer
    Dim BaseClass As String
    '
    Call InitOutline
    '
    If ActorsOnly.Value = 1 Then
        BaseClass = "Actor"
    Else
        BaseClass = "Object"
    End If
    '
    QuerySource = -1
    Classes.Clear
    Classes.AddItem "*" & BaseClass
    Classes.ListIndex = 0
    '
    QuerySource = Classes.ListIndex
    Call UpdateOutline(Classes, Ed.ServerGetProp("Class", "Query Parent=" & BaseClass))
    SetCurrent
End Sub

Sub SetCurrent()
    Dim N As Integer
    Dim i As Integer
    '
    N = Len(Ed.CurrentClass)
    For i = 0 To Classes.ListCount - 1
        If Mid(Classes.List(i), 1, N) = Ed.CurrentClass Then
            Classes.ListIndex = i
        ElseIf Mid(Classes.List(i), 2, N) = Ed.CurrentClass Then
            Classes.ListIndex = i
        End If
    Next i
    SetActorCaption
End Sub

Function ClassIsEditable() As Boolean
    If Classes.ListIndex < 0 Then
        ClassIsEditable = False
    ElseIf Left(Classes.List(Classes.ListIndex), 1) <> "*" Then
        ClassIsEditable = True
    Else 'If Ed.GodMode Then
        ClassIsEditable = True
    'Else
    '    ClassIsEditable = False
    End If
End Function

Sub SetActorCaption()
    Dim c As String
    
    c = Ed.ServerGetProp("CLASS", "PACKAGE CLASS=" & GetCurrent()) & "." & GetCurrent()
    If ClassIsEditable() Then
        EditScript.Enabled = True
        ClassDescr.Caption = c & " (scripted)"
    Else
        EditScript.Enabled = False
        ClassDescr.Caption = c & " (built-in)"
    End If
End Sub

Public Sub SaveClass_Click()
    Dim CurClass As String
    Dim Fname As String
    '
    CurClass = GetCurrent()
    If CurClass <> "" Then
        frmSaveClass.Caption = "Save class package to *.u file"
        frmSaveClass.ClassName = CurClass
        Ed.ServerDisable
        frmSaveClass.Show 1
        Ed.ServerEnable
        If GlobalAbortedModal Then Exit Sub
        '
        PreSaveAll
        Ed.ServerExec "OBJ SAVEPACKAGE" & _
            " PACKAGE=" & GString & _
            " FILE=" & Quotes(GString & ".u")
    End If
End Sub

' Process results after compiling a script.
Public Sub ProcessResults()
    Dim S As String, T As String, i As Long, Line As Long, Msg As String
    S = Ed.ServerGetProp("Text", "Results")
    If Left(S, 9) = "Error in " And InStr(S, ":") > 0 Then
        ' Script compiler error.
        Msg = Mid(S, InStr(S, ":") + 1)
        If InStr(Msg, Chr(13)) Then Msg = Left(Msg, InStr(Msg, Chr(13)) - 1)
        
        S = Mid(S, 10)
        i = InStr(S, ", Line ")
        If i <> 0 Then
            Line = Val(Mid(S, i + 7)) ' Line number
            S = Left(S, i - 1) ' Class name
            Call LaunchScriptEd(S, "", 0, Line, Msg)
        End If
    Else
        Ed.StatusText S
        If Not frmMain.ScriptForm Is Nothing Then
            frmMain.ScriptForm.ErrorBox.Text = ""
            frmMain.ScriptForm.LoadAll
        End If
    End If
End Sub
