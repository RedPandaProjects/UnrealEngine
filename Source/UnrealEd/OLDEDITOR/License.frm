VERSION 5.00
Begin VB.Form frmLicense 
   Caption         =   "Licensing Notice"
   ClientHeight    =   4800
   ClientLeft      =   1305
   ClientTop       =   1635
   ClientWidth     =   6930
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
   HelpContextID   =   105
   Icon            =   "License.frx":0000
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   4800
   ScaleWidth      =   6930
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton RelNotes 
      Caption         =   "Release Notes"
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
      Left            =   3960
      TabIndex        =   10
      Top             =   4320
      Width           =   1335
   End
   Begin VB.CommandButton Agree 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "I &Agree"
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
      Left            =   5400
      TabIndex        =   2
      Top             =   4320
      Width           =   1095
   End
   Begin VB.CommandButton Cancel 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Cancel"
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
      Left            =   2760
      TabIndex        =   1
      Top             =   4320
      Width           =   1095
   End
   Begin VB.Image Image1 
      Height          =   1140
      Left            =   240
      Picture         =   "License.frx":030A
      Top             =   240
      Width           =   1530
   End
   Begin VB.Label Label8 
      Caption         =   "Epic MegaGames, Inc."
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1920
      TabIndex        =   9
      Top             =   720
      Width           =   3015
   End
   Begin VB.Label Label7 
      Caption         =   "Thanks."
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
      Left            =   240
      TabIndex        =   8
      Top             =   4440
      Width           =   735
   End
   Begin VB.Label Label6 
      Caption         =   "3. Upon written request from Epic, destroy all copies of the software and return all materials associated with it."
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      TabIndex        =   7
      Top             =   3840
      Width           =   6375
   End
   Begin VB.Label Label5 
      Caption         =   $"License.frx":262C
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   240
      TabIndex        =   6
      Top             =   3120
      Width           =   6255
   End
   Begin VB.Label Label4 
      Caption         =   "1. Use it solely in conjunction with development of products or product content for publication by Epic MegaGames, Inc."
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      TabIndex        =   5
      Top             =   2520
      Width           =   6375
   End
   Begin VB.Label Label3 
      Caption         =   "To use this software, you must agree to:"
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
      Left            =   240
      TabIndex        =   4
      Top             =   2160
      Width           =   3495
   End
   Begin VB.Label Label2 
      Caption         =   $"License.frx":26E6
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      TabIndex        =   3
      Top             =   1680
      Width           =   6495
   End
   Begin VB.Label Label1 
      Caption         =   "Confidential Development Software"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   1920
      TabIndex        =   0
      Top             =   1080
      Width           =   4695
   End
End
Attribute VB_Name = "frmLicense"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Cancel_Click()
    End
End Sub

Private Sub Agree_Click()
    Unload Me
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub RelNotes_Click()
    frmDialogs.RelNotes.ShowHelp
End Sub
