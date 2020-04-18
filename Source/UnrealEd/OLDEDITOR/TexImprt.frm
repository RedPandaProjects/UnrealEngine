VERSION 5.00
Begin VB.Form frmTexImport 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Import Texture"
   ClientHeight    =   3375
   ClientLeft      =   2295
   ClientTop       =   3015
   ClientWidth     =   4920
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   HelpContextID   =   125
   Icon            =   "TexImprt.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3375
   ScaleWidth      =   4920
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton OkAll 
      Caption         =   "Ok to &All"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1320
      TabIndex        =   2
      Top             =   2940
      Width           =   1215
   End
   Begin VB.Frame Frame2 
      Caption         =   "Properties"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   60
      TabIndex        =   13
      Top             =   1860
      Width           =   4755
      Begin VB.CheckBox Blur 
         Caption         =   "Blur the texture"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   2520
         TabIndex        =   15
         Tag             =   "2"
         Top             =   600
         Width           =   1875
      End
      Begin VB.CheckBox BumpMap 
         Caption         =   "Import as bump map"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   2520
         TabIndex        =   14
         Tag             =   "2"
         Top             =   360
         Width           =   1875
      End
      Begin VB.CheckBox DoMips 
         Caption         =   "Generate Mipmaps"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   240
         TabIndex        =   6
         Top             =   600
         Value           =   1  'Checked
         Width           =   1875
      End
      Begin VB.CheckBox DoMasked 
         Caption         =   "Masked"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   240
         TabIndex        =   5
         Top             =   360
         Width           =   1875
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Texture"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1695
      Left            =   60
      TabIndex        =   8
      Top             =   120
      Width           =   4755
      Begin VB.TextBox TexGroup 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   285
         Left            =   1320
         MaxLength       =   15
         TabIndex        =   16
         Text            =   "TexGroup"
         Top             =   900
         Width           =   2295
      End
      Begin VB.TextBox TexName 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   285
         Left            =   1320
         MaxLength       =   15
         TabIndex        =   7
         Text            =   "TexName"
         Top             =   540
         Width           =   2295
      End
      Begin VB.TextBox TexSet 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   285
         Left            =   1320
         MaxLength       =   15
         TabIndex        =   0
         Text            =   "TexFamily"
         Top             =   1260
         Width           =   2295
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "Group:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   180
         TabIndex        =   17
         Top             =   900
         Width           =   975
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "File:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   120
         TabIndex        =   12
         Top             =   300
         Width           =   975
      End
      Begin VB.Label Fname 
         Caption         =   "Fname"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   1320
         TabIndex        =   11
         Top             =   300
         Width           =   3315
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Name:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   60
         TabIndex        =   10
         Top             =   600
         Width           =   1095
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Package:"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   180
         TabIndex        =   9
         Top             =   1260
         Width           =   975
      End
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Skip"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   2640
      TabIndex        =   3
      Top             =   2940
      Width           =   1095
   End
   Begin VB.CommandButton OK 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Ok"
      Default         =   -1  'True
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   2940
      Width           =   1095
   End
   Begin VB.CommandButton Cancel 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   3780
      TabIndex        =   4
      Top             =   2940
      Width           =   1095
   End
End
Attribute VB_Name = "frmTexImport"
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

Public Sub DoNext()
    Fname = GrabFname(GString)
    If Fname = "" Then
        GlobalAbortedModal = 0
        AutoOk = 0
        Hide
    End If
    
    TexName = GetFileNameOnly(Fname)
    If TexName = AllString Then
        TexName = "Untitled"
    End If
    
    If AutoOk = 1 Then Ok_Click
End Sub

Private Sub Form_Activate()
    TexSet.SelStart = 0
    TexSet.SelLength = Len(TexSet.Text)
    TexSet.SetFocus
    
    DoNext
End Sub

Private Sub Form_Load()
    AutoOk = 0
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub Ok_Click()
    Dim TexFname As String
    Dim TexName As String
    Dim TexSet As String
    Dim TexFlags As Long
    
    TexFname = Fname
    TexName = Trim(frmTexImport.TexName)
    TexSet = Trim(frmTexImport.TexSet)
    
    GlobalAbortedModal = 0

    If Len(TexName) = 0 Or Len(TexName) > 15 Or Len(TexSet) = 0 Or Len(TexSet) > 15 Then
        MsgBox "Texture name and Set name must be given, and must be 1-31 characters each", 16
    Else
        TexFlags = 0
        If BumpMap.Value Then TexFlags = TexFlags + TF_BumpMap
        If Blur.Value Then TexFlags = TexFlags + TF_Blur
        Ed.BeginSlowTask "Importing Texture"
        Ed.ServerExec "TEXTURE IMPORT FILE=" & Quotes(TexFname) & _
            " NAME=" & Quotes(TexName) & _
            " PACKAGE=" & Quotes(TexSet) & _
            " GROUP=" & Quotes(TexGroup) & _
            " MIPS=" & OnOff(Int(DoMips.Value)) & _
            " FLAGS=" & Trim(Str(IIf(DoMasked.Value, PF_MASKED, 0))) & _
            " TEXFLAGS=" & Trim(Str(TexFlags))
        Ed.EndSlowTask
        DoNext
    End If

End Sub

Private Sub OkAll_Click()
    AutoOk = 1
    Ok_Click
End Sub

