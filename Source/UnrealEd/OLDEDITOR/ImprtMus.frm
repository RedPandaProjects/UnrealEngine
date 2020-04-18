VERSION 5.00
Begin VB.Form frmMusicImportDlg 
   Caption         =   "Imort Music"
   ClientHeight    =   1590
   ClientLeft      =   4305
   ClientTop       =   4365
   ClientWidth     =   4935
   LinkTopic       =   "Form2"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   1590
   ScaleWidth      =   4935
   Begin VB.CommandButton OkAll 
      Caption         =   "Ok to &All"
      Height          =   375
      Left            =   1260
      TabIndex        =   8
      Top             =   1200
      Width           =   1215
   End
   Begin VB.Frame Frame1 
      Caption         =   "Music name"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1155
      Left            =   60
      TabIndex        =   3
      Top             =   0
      Width           =   4755
      Begin VB.TextBox MusicName 
         BackColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   1500
         MaxLength       =   15
         TabIndex        =   4
         Text            =   "???"
         Top             =   660
         Width           =   2955
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "File:"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Top             =   360
         Width           =   1275
      End
      Begin VB.Label Fname 
         Caption         =   "Fname"
         Height          =   255
         Left            =   1500
         TabIndex        =   6
         Top             =   360
         Width           =   2955
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Song Name:"
         Height          =   255
         Left            =   60
         TabIndex        =   5
         Top             =   660
         Width           =   1335
      End
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Skip"
      Height          =   375
      Left            =   2580
      TabIndex        =   2
      Top             =   1200
      Width           =   1095
   End
   Begin VB.CommandButton OK 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Ok"
      Default         =   -1  'True
      Height          =   375
      Left            =   60
      TabIndex        =   1
      Top             =   1200
      Width           =   1095
   End
   Begin VB.CommandButton Cancel 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   3720
      TabIndex        =   0
      Top             =   1200
      Width           =   1095
   End
End
Attribute VB_Name = "frmMusicImportDlg"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim ThisFname
Dim AutoOk As Integer



Public Sub DoNext()
    Fname = GrabFname(GString)
    If frmMusicImportDlg.Fname = "" Then
        GlobalAbortedModal = 0
        AutoOk = 0
        Unload Me
    End If
    '
    frmMusicImportDlg.MusicName = GetFileNameOnly(Fname)
    '
    If AutoOk = 1 Then Ok_Click
End Sub

Private Sub Cancel_Click()
    GlobalAbortedModal = 1
    AutoOk = 0
    Hide
End Sub


Private Sub Form_Activate()
    DoNext
End Sub

Private Sub Form_Load()
    AutoOk = 0
    Call Ed.MakeFormFit(Me)
End Sub


Private Sub Ok_Click()
    Dim tmpFname As String
    Dim tmpName As String
    
    ' Get name of file that was chosen by user
    tmpFname = frmMusicImportDlg.Fname
    
    tmpName = Trim(frmMusicImportDlg.MusicName)
    
    GlobalAbortedModal = 0
    
    If Len(tmpName) = 0 Or Len(tmpName) > 15 Then
        MsgBox "Music name name must be given, and must be 1-15 characters", 16
    Else
        ' Tell the engine to import the specified resource
        Ed.ServerExec "MUSIC IMPORT FILE=" & Quotes(tmpFname) & _
            " NAME=" & Quotes(tmpName)
        DoNext
    End If
    '
'    GString = SoundFamily.Text
    '
End Sub


Private Sub OkAll_Click()
    AutoOk = 1
    Ok_Click
End Sub


