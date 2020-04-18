VERSION 4.00
Begin VB.Form frmATest 
   Caption         =   "UnrealEd Command Line"
   ClientHeight    =   1380
   ClientLeft      =   4560
   ClientTop       =   3795
   ClientWidth     =   5850
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
   Height          =   1845
   Left            =   4500
   LinkMode        =   1  'Source
   LinkTopic       =   "ATest"
   ScaleHeight     =   1380
   ScaleWidth      =   5850
   ShowInTaskbar   =   0   'False
   Top             =   3390
   Width           =   5970
   Begin VB.CommandButton Command2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
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
      Left            =   4680
      TabIndex        =   3
      Top             =   960
      Width           =   1095
   End
   Begin VB.TextBox Typed 
      Height          =   285
      Left            =   120
      TabIndex        =   0
      Top             =   600
      Width           =   5655
   End
   Begin VB.CommandButton Command1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Send Command"
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
      Top             =   960
      Width           =   1335
   End
   Begin VB.Label Label1 
      Caption         =   "UnrealEd Command-Line"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   12
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   60
      TabIndex        =   1
      Top             =   120
      Width           =   3615
   End
End
Attribute VB_Name = "frmATest"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()
    Ed.BeginSlowTask "Executing command"
    Ed.Server.Exec Typed.Text
    Ed.EndSlowTask
End Sub

Private Sub Command2_Click()
    Unload frmATest
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "TestDialog", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub


