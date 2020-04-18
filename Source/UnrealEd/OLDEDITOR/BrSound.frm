VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmSoundFXBrowser 
   BorderStyle     =   0  'None
   Caption         =   "Sound Browser"
   ClientHeight    =   6420
   ClientLeft      =   6945
   ClientTop       =   2535
   ClientWidth     =   2445
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6420
   ScaleWidth      =   2445
   ShowInTaskbar   =   0   'False
   Begin VB.ComboBox PkgList 
      BackColor       =   &H00C0C0C0&
      Height          =   315
      ItemData        =   "BrSound.frx":0000
      Left            =   60
      List            =   "BrSound.frx":0002
      MousePointer    =   1  'Arrow
      Sorted          =   -1  'True
      Style           =   2  'Dropdown List
      TabIndex        =   10
      Tag             =   "Current texture family"
      Top             =   0
      Width           =   2310
   End
   Begin VB.ComboBox GrpList 
      BackColor       =   &H00C0C0C0&
      Height          =   315
      ItemData        =   "BrSound.frx":0004
      Left            =   60
      List            =   "BrSound.frx":0006
      MousePointer    =   1  'Arrow
      Sorted          =   -1  'True
      Style           =   2  'Dropdown List
      TabIndex        =   9
      Tag             =   "Current texture family"
      Top             =   300
      Width           =   2310
   End
   Begin VB.PictureBox SoundsHolder 
      Height          =   4395
      Left            =   60
      ScaleHeight     =   4335
      ScaleWidth      =   2295
      TabIndex        =   7
      Top             =   660
      Width           =   2355
      Begin VB.ListBox SoundList 
         BackColor       =   &H8000000F&
         Height          =   4275
         IntegralHeight  =   0   'False
         Left            =   0
         Sorted          =   -1  'True
         TabIndex        =   8
         Top             =   0
         Width           =   2295
      End
   End
   Begin Threed.SSPanel SoundButtonPanel 
      Height          =   1035
      Left            =   0
      TabIndex        =   0
      Top             =   5100
      Width           =   2415
      _Version        =   65536
      _ExtentX        =   4260
      _ExtentY        =   1826
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
      Begin VB.CommandButton SoundExport 
         Caption         =   "&Export"
         Height          =   255
         Left            =   1200
         TabIndex        =   6
         Top             =   780
         Width           =   1155
      End
      Begin VB.CommandButton SoundSave 
         Caption         =   "&Save"
         Height          =   255
         Left            =   1200
         TabIndex        =   2
         Top             =   540
         Width           =   1155
      End
      Begin VB.CommandButton Delete 
         Caption         =   "&Delete"
         Height          =   255
         Left            =   1200
         TabIndex        =   11
         Top             =   300
         Width           =   1155
      End
      Begin VB.CommandButton SoundImport 
         Caption         =   "&Import"
         Height          =   255
         Left            =   60
         TabIndex        =   5
         Top             =   780
         Width           =   1155
      End
      Begin VB.CommandButton SoundLoad 
         Caption         =   "&Load"
         Height          =   255
         Left            =   60
         TabIndex        =   1
         Top             =   540
         Width           =   1155
      End
      Begin VB.CommandButton SoundStop 
         Caption         =   "&Stop"
         Height          =   255
         Left            =   60
         TabIndex        =   3
         Top             =   300
         Width           =   1155
      End
      Begin VB.CommandButton SoundTest 
         Caption         =   "&Play"
         Height          =   255
         Left            =   60
         TabIndex        =   4
         Top             =   60
         Width           =   2295
      End
   End
End
Attribute VB_Name = "frmSoundFXBrowser"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim Refreshing As Boolean

Private Sub Delete_Click()
    Dim Result As String
    Result = Ed.ServerGetProp("Obj", "Delete Class=Sound Object=" & GetCurrent())
    If Result = "" Then
        Call RefreshLists(PkgList.List(PkgList.ListIndex), GrpList.List(GrpList.ListIndex), "")
    Else
        Call MsgBox(Result, , "Can't delete sound")
    End If
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "SoundBrowser", TOP_BROWSER)
    SoundButtonPanel.Top = Height - SoundButtonPanel.Height
    SoundsHolder.Height = SoundButtonPanel.Top - SoundsHolder.Top
    SoundList.Height = SoundsHolder.ScaleHeight
    Call RefreshLists("", "", "")
End Sub

Public Function GetCurrent() As String
    If SoundList.ListIndex >= 0 Then
        GetCurrent = SoundList.List(SoundList.ListIndex)
    End If
End Function

Public Sub BrowserShow()
    Show
End Sub

Private Sub RefreshLists(Pkg As String, Grp As String, Snd As String)
    Dim S As String, i As Integer

    ' Prevent reentrancy.
    If Refreshing Then Exit Sub
    Refreshing = True

    ' Package list.
    PkgList.Clear
    S = Ed.ServerGetProp("OBJ", "PACKAGES CLASS=Sound")
    While S <> ""
        PkgList.AddItem GrabCommaString(S)
    Wend
    If PkgList.ListCount = 0 Then PkgList.AddItem (NoneString)
    PkgList.ListIndex = 0
    For i = 0 To PkgList.ListCount - 1
        If PkgList.List(i) = Pkg Then
            PkgList.ListIndex = i
        End If
    Next i

    ' Group list.
    GrpList.Clear
    GrpList.AddItem (AllString)
    S = Ed.ServerGetProp("OBJ", "GROUPS CLASS=Sound PACKAGE=" & PkgList.List(PkgList.ListIndex))
    While S <> ""
        GrpList.AddItem GrabCommaString(S)
    Wend
    If GrpList.ListCount > 1 Then
        GrpList.ListIndex = 1
    Else
        GrpList.ListIndex = 0
    End If
    For i = 0 To GrpList.ListCount - 1
        If GrpList.List(i) = Grp Then
            GrpList.ListIndex = i
        End If
    Next i

    ' Sound list.
    SoundList.Clear
    S = Ed.ServerGetProp("OBJ", "QUERY TYPE=Sound PACKAGE=" & PkgList.List(PkgList.ListIndex) & " GROUP=" & GrpList.List(GrpList.ListIndex))
    While S <> ""
        SoundList.AddItem GrabString(S)
    Wend
    If SoundList.ListCount = 0 Then SoundList.AddItem (NoneString)
    SoundList.ListIndex = 0
    For i = 0 To SoundList.ListCount - 1
        If SoundList.List(i) = Snd Then
            SoundList.ListIndex = i
        End If
    Next i

    Refreshing = False
End Sub

Public Sub BrowserHide()
    Unload Me
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub SoundClose_Click()
    Hide
End Sub

Private Sub GrpList_Click()
    Call RefreshLists(PkgList.List(PkgList.ListIndex), GrpList.List(GrpList.ListIndex), "")
End Sub

Private Sub PkgList_Click()
    Call RefreshLists(PkgList.List(PkgList.ListIndex), "", "")
End Sub

Private Sub SoundExport_Click()
    Dim Ext As String
    If GetCurrent() = "" Then Exit Sub

    Ext = Ed.ServerGetProp("Sound", "FileType PACKAGE=" & PkgList.List(PkgList.ListIndex) & " NAME=" & GetCurrent())

    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.SoundExportDlg.filename = Trim(GetCurrent()) & "." & Ext
    frmDialogs.SoundExportDlg.DialogTitle = "Export sound " & GetCurrent()
    frmDialogs.SoundExportDlg.ShowSave
    Call UpdateDialog(frmDialogs.SoundExportDlg)
    On Error GoTo 0

    If frmDialogs.SoundExportDlg.filename <> "" Then
        Ed.ServerExec "OBJ EXPORT TYPE=SOUND" & _
        " PACKAGE=" & Quotes(PkgList.List(PkgList.ListIndex)) & _
        " NAME=" & Quotes(GetCurrent()) & _
        " FILE=" & Quotes(frmDialogs.SoundExportDlg.filename)
    End If

Skip:
    Ed.ServerEnable
End Sub

Private Sub SoundList_DblClick()
    SoundTest_Click
End Sub

Private Sub SoundTest_Click()
    If GetCurrent() <> "" Then
        Ed.ServerExec "AUDIO PLAY NAME=" & GetCurrent()
    End If
End Sub

Private Sub SoundImport_Click()
    Dim S As String, T As String
    
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.SoundImportDlg.filename = ""
    frmDialogs.SoundImportDlg.ShowOpen
    Call UpdateDialog(frmDialogs.SoundImportDlg)
    On Error GoTo 0

    GString = Trim(frmDialogs.SoundImportDlg.filename)
    frmSoundImportDlg.Show 1
    Call RefreshLists(frmSoundImportDlg.SoundPackage, frmSoundImportDlg.SoundGroup, frmSoundImportDlg.SoundName)

Skip:
    Ed.ServerEnable
End Sub

Private Sub SoundLoad_Click()
    Dim S As String, T As String
    
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.SoundLoadDlg.filename = ""
    frmDialogs.SoundLoadDlg.ShowOpen
    Call UpdateDialog(frmDialogs.SoundLoadDlg)
    On Error GoTo 0

    If frmDialogs.SoundLoadDlg.filename <> "" Then
        S = frmDialogs.SoundLoadDlg.filename
        While S <> ""
            T = GetFileNameOnly(S)
            Ed.ServerExec "OBJ LOAD FILE=" & Quotes(GrabFname(S))
        Wend
        Call RefreshLists(T, "", "")
    End If

Skip:
    Ed.ServerEnable

End Sub

Private Sub SoundSave_Click()

    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.SoundSaveDlg.filename = PkgList.List(PkgList.ListIndex) & ".uax"
    frmDialogs.SoundSaveDlg.DialogTitle = "Save sound package " & PkgList.List(PkgList.ListIndex)
    frmDialogs.SoundSaveDlg.ShowSave
    Call UpdateDialog(frmDialogs.SoundSaveDlg)
    On Error GoTo 0

    If frmDialogs.SoundSaveDlg.filename <> "" Then
        Ed.ServerExec "OBJ SAVEPACKAGE PACKAGE=" & PkgList.List(PkgList.ListIndex) & " FILE=" & Quotes(frmDialogs.SoundSaveDlg.filename)
    End If

Skip:
    Ed.ServerEnable

End Sub

Private Sub SoundStop_Click()
    Ed.ServerExec "AUDIO PLAY NAME=None"
End Sub
