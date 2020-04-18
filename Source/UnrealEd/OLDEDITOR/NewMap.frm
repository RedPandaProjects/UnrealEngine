VERSION 4.00
Begin VB.Form frmNewMap 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Create New Level"
   ClientHeight    =   1830
   ClientLeft      =   6210
   ClientTop       =   2715
   ClientWidth     =   5070
   BeginProperty Font 
      name            =   "MS Sans Serif"
      charset         =   0
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   Height          =   2190
   HelpContextID   =   115
   Icon            =   "NewMap.frx":0000
   Left            =   6150
   LinkTopic       =   "Form2"
   ScaleHeight     =   1830
   ScaleWidth      =   5070
   ShowInTaskbar   =   0   'False
   Top             =   2415
   Width           =   5190
   Begin VB.TextBox Title 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1680
      LinkItem        =   "0"
      LinkTopic       =   "UNREALSV|LEV"
      TabIndex        =   0
      Top             =   480
      Width           =   3255
   End
   Begin VB.TextBox Creator 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1680
      LinkItem        =   "1"
      LinkTopic       =   "UNREALSV|LEV"
      TabIndex        =   1
      Top             =   840
      Width           =   3255
   End
   Begin VB.CommandButton Cancel 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   3840
      TabIndex        =   4
      Top             =   1320
      Width           =   1095
   End
   Begin VB.CommandButton OK 
      BackColor       =   &H00C0C0C0&
      Caption         =   "OK"
      Default         =   -1  'True
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   120
      TabIndex        =   2
      Top             =   1320
      Width           =   1095
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
      BackStyle       =   0  'Transparent
      Caption         =   "Level Title:"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   600
      TabIndex        =   3
      Top             =   480
      Width           =   975
   End
   Begin VB.Label Label4 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
      BackStyle       =   0  'Transparent
      Caption         =   "Original Creator:"
      BeginProperty Font 
         name            =   "MS Sans Serif"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   840
      Width           =   1455
   End
End
Attribute VB_Name = "frmNewMap"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Cancel_Click()
    Unload Me
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub Ok_Click()
   '
   ' New map:
   '
   Ed.MapFname = ""
   frmMain.Caption = Ed.EditorAppName
   '
   Ed.Server.Exec "MAP NEW"
   '
   Call Ed.Server.SetProp("Actor", "LevelProperties", "LevelTitle=" & Title.Text)
   Call Ed.Server.SetProp("Actor", "LevelProperties", "LevelAuthor=" & Creator.Text)
   '
   PostLoad
   '
   Unload Me
   '
End Sub

