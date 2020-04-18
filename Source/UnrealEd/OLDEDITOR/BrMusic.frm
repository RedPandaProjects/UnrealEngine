VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmMusicBrowser 
   BorderStyle     =   0  'None
   Caption         =   "Music Browser"
   ClientHeight    =   5865
   ClientLeft      =   1305
   ClientTop       =   2820
   ClientWidth     =   2490
   LinkTopic       =   "Form2"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5865
   ScaleWidth      =   2490
   ShowInTaskbar   =   0   'False
   Begin VB.PictureBox MusicHolder 
      Height          =   4875
      Left            =   60
      ScaleHeight     =   4815
      ScaleWidth      =   2295
      TabIndex        =   7
      Top             =   60
      Width           =   2355
      Begin VB.ListBox SongList 
         BackColor       =   &H8000000F&
         Height          =   4815
         IntegralHeight  =   0   'False
         Left            =   0
         Sorted          =   -1  'True
         TabIndex        =   8
         Top             =   0
         Width           =   2295
      End
   End
   Begin Threed.SSPanel MusicButtonPanel 
      Height          =   855
      Left            =   0
      TabIndex        =   0
      Top             =   4980
      Width           =   2415
      _Version        =   65536
      _ExtentX        =   4260
      _ExtentY        =   1508
      _StockProps     =   15
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
      Begin VB.CommandButton MusicExport 
         Caption         =   "&Export"
         Height          =   255
         Left            =   1200
         TabIndex        =   6
         Top             =   540
         Width           =   1155
      End
      Begin VB.CommandButton MusicImport 
         Caption         =   "&Import"
         Height          =   255
         Left            =   60
         TabIndex        =   5
         Top             =   540
         Width           =   1155
      End
      Begin VB.CommandButton MusicSave 
         Caption         =   "&Save"
         Height          =   255
         Left            =   1200
         TabIndex        =   4
         Top             =   300
         Width           =   1155
      End
      Begin VB.CommandButton MusicLoad 
         Caption         =   "&Load"
         Height          =   255
         Left            =   60
         TabIndex        =   3
         Top             =   300
         Width           =   1155
      End
      Begin VB.CommandButton MusicStop 
         Caption         =   "&Stop"
         Height          =   255
         Left            =   1200
         TabIndex        =   2
         Top             =   60
         Width           =   1155
      End
      Begin VB.CommandButton MusicTest 
         Caption         =   "&Play"
         Height          =   255
         Left            =   60
         TabIndex        =   1
         Top             =   60
         Width           =   1155
      End
   End
End
Attribute VB_Name = "frmMusicBrowser"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "MusicBrowser", TOP_BROWSER)
    MusicButtonPanel.Top = Height - MusicButtonPanel.Height
    MusicHolder.Height = Height - MusicButtonPanel.Height
    SongList.Height = MusicHolder.ScaleHeight
    BrowserRefresh ""
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub MusicExport_Click()
    Dim Ext As String
    If GetCurrent() = "" Then Exit Sub

    Ext = Ed.ServerGetProp("Music", "FileType PACKAGE=" & GetCurrent() & " NAME=" & GetCurrent())

    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.MusicExportDlg.filename = Trim(GetCurrent()) & "." & Ext
    frmDialogs.MusicExportDlg.DialogTitle = "Export music " & GetCurrent()
    frmDialogs.MusicExportDlg.ShowSave
    Call UpdateDialog(frmDialogs.MusicExportDlg)
    On Error GoTo 0

    If frmDialogs.MusicExportDlg.filename <> "" Then
        Ed.ServerExec "OBJ EXPORT TYPE=MUSIC" & _
        " PACKAGE=" & Quotes(GetCurrent()) & _
        " NAME=" & Quotes(GetCurrent()) & _
        " FILE=" & Quotes(frmDialogs.MusicExportDlg.filename)
    End If

Skip:
    Ed.ServerEnable

End Sub

Private Sub MusicImport_Click()
    Dim S As String, T As String
    
    Ed.ServerDisable
    On Error GoTo Skip
    
    frmDialogs.MusicImportDlg.filename = ""
    frmDialogs.MusicImportDlg.ShowOpen
    Call UpdateDialog(frmDialogs.MusicImportDlg)

    S = Trim(frmDialogs.MusicImportDlg.filename)
    While S <> ""
        T = GrabFname(S)
        Ed.ServerExec "OBJ IMPORT STANDALONE TYPE=MUSIC FILE=" & Quotes(T) & " NAME=" & GetFileNameOnly(T) & " PACKAGE=" & Quotes(GetFileNameOnly(T))
    Wend

    If GlobalAbortedModal = 0 Then BrowserRefresh GetFileNameOnly(T)

Skip:
    Ed.ServerEnable
End Sub

Private Sub MusicLoad_Click()
    Dim S As String, T As String
    
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.MusicLoadDlg.filename = ""
    frmDialogs.MusicLoadDlg.ShowOpen
    Call UpdateDialog(frmDialogs.MusicLoadDlg)
    On Error GoTo 0
    
    If frmDialogs.MusicLoadDlg.filename <> "" Then
        S = frmDialogs.MusicLoadDlg.filename
        While S <> ""
            T = GrabFname(S)
            Ed.ServerExec "OBJ LOAD FILE=" & Quotes(T)
        Wend
        BrowserRefresh GetFileNameOnly(T)
    End If

Skip:
    Ed.ServerEnable

End Sub

Private Sub MusicSave_Click()
    If GetCurrent() = "" Then Exit Sub

    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.MusicSaveDlg.filename = Trim(GetCurrent()) & ".umx"
    frmDialogs.MusicSaveDlg.DialogTitle = "Save music " & GetCurrent()
    frmDialogs.MusicSaveDlg.ShowSave
    Call UpdateDialog(frmDialogs.MusicSaveDlg)
    On Error GoTo 0

    If frmDialogs.MusicSaveDlg.filename <> "" Then
        Ed.ServerExec "OBJ SAVEPACKAGE PACKAGE=" & Quotes(GetCurrent()) & " FILE=" & Quotes(frmDialogs.MusicSaveDlg.filename)
    End If

Skip:
    Ed.ServerEnable
End Sub

Public Function GetCurrent() As String
    If SongList.ListIndex >= 0 Then
        GetCurrent = SongList.List(SongList.ListIndex)
    End If
End Function

Public Sub BrowserHide()
    Unload Me
End Sub

Public Sub BrowserRefresh(N As String)
    Dim S As String, T As String, i As Integer
    
    SongList.Clear
    S = Ed.ServerGetProp("OBJ", "QUERY TYPE=MUSIC")
    While S <> ""
        T = GrabString(S)
        SongList.AddItem T
    Wend
    If SongList.ListCount > 0 Then
        SongList.ListIndex = 0
    End If
    For i = 0 To SongList.ListCount - 1
        If UCase(SongList.List(i)) = UCase(N) Then
            SongList.ListIndex = i
        End If
    Next
End Sub

Public Sub BrowserShow()
    Show
End Sub

Private Sub MusicStop_Click()
    Ed.ServerExec "MUSIC PLAY NAME=None"
End Sub

Private Sub MusicTest_Click()
    If GetCurrent() <> "" Then
        Ed.ServerExec "MUSIC PLAY NAME=" & GetCurrent()
    End If
End Sub

Private Sub SongList_DblClick()
    MusicTest_Click
End Sub
