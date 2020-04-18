VERSION 5.00
Begin VB.Form frmSoundImportDlg 
   Caption         =   "Import a sound"
   ClientHeight    =   2400
   ClientLeft      =   4305
   ClientTop       =   6870
   ClientWidth     =   4860
   LinkTopic       =   "Form2"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   2400
   ScaleWidth      =   4860
   Begin VB.CommandButton Cancel 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   3660
      TabIndex        =   6
      Top             =   1980
      Width           =   1155
   End
   Begin VB.CommandButton OK 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Ok"
      Default         =   -1  'True
      Height          =   375
      Left            =   60
      TabIndex        =   3
      Top             =   1980
      Width           =   1095
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Skip"
      Height          =   375
      Left            =   2460
      TabIndex        =   5
      Top             =   1980
      Width           =   1155
   End
   Begin VB.Frame Frame1 
      Caption         =   "Sound name"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1815
      Left            =   60
      TabIndex        =   7
      Top             =   60
      Width           =   4755
      Begin VB.TextBox SoundPackage 
         BackColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   960
         MaxLength       =   15
         TabIndex        =   2
         Text            =   "General"
         Top             =   1320
         Width           =   2415
      End
      Begin VB.TextBox SoundGroup 
         BackColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   960
         MaxLength       =   15
         TabIndex        =   1
         Text            =   "General"
         Top             =   960
         Width           =   2415
      End
      Begin VB.TextBox SoundName 
         BackColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   960
         MaxLength       =   15
         TabIndex        =   0
         Text            =   "???"
         Top             =   600
         Width           =   2415
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "Package:"
         Height          =   255
         Left            =   60
         TabIndex        =   12
         Top             =   1320
         Width           =   795
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Group:"
         Height          =   255
         Left            =   60
         TabIndex        =   11
         Top             =   960
         Width           =   795
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Name:"
         Height          =   255
         Left            =   60
         TabIndex        =   10
         Top             =   660
         Width           =   795
      End
      Begin VB.Label Fname 
         Caption         =   "Fname"
         Height          =   255
         Left            =   960
         TabIndex        =   9
         Top             =   360
         Width           =   3495
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "File:"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   360
         Width           =   735
      End
   End
   Begin VB.CommandButton OkAll 
      Caption         =   "Ok to &All"
      Height          =   375
      Left            =   1200
      TabIndex        =   4
      Top             =   1980
      Width           =   1215
   End
End
Attribute VB_Name = "frmSoundImportDlg"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim ThisFname
Dim AutoOk As Integer

Private Sub Cancel_Click()
    GlobalAbortedModal = 1
    AutoOk = 0
    Hide
End Sub

Private Sub Form_Activate()
    SoundGroup = frmSoundFXBrowser.GrpList.List(frmSoundFXBrowser.GrpList.ListIndex)
    SoundPackage = frmSoundFXBrowser.PkgList.List(frmSoundFXBrowser.PkgList.ListIndex)
    DoNext
End Sub

Private Sub Form_Load()
    AutoOk = 0
    Call Ed.MakeFormFit(Me)
End Sub

Public Sub DoNext()
    Fname = GrabFname(GString)
    If Fname = "" Then
        GlobalAbortedModal = 0
        AutoOk = 0
        Hide
    End If

    SoundName = GetFileNameOnly(Fname)
    If AutoOk = 1 Then Ok_Click
End Sub

Private Sub Ok_Click()
    Dim Name As String
    Dim Package As String
    
    Name = Trim(SoundName)
    Package = Trim(SoundPackage)
    
    GlobalAbortedModal = 0
    
    If Len(Name) = 0 Or Len(Name) > 31 Or Len(Package) = 0 Or Len(Package) > 31 Then
        MsgBox "Sound name, group, and package must be given, and must be 1-31 characters each", 16
    Else
        ' Tell the engine to import the specified sound
        Ed.BeginSlowTask "Importing sounds"
        Ed.ServerExec "AUDIO IMPORT" & _
            " FILE=" & Quotes(Fname) & _
            " NAME=" & Quotes(Name) & _
            " PACKAGE=" & Quotes(Package) & _
            " GROUP=" & Quotes(SoundGroup)
        Ed.EndSlowTask
        DoNext
    End If
End Sub

Private Sub OkAll_Click()
    AutoOk = 1
    Ok_Click
End Sub

