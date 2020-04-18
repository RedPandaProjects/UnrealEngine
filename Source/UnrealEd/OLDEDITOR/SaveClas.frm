VERSION 5.00
Begin VB.Form frmSaveClass 
   Caption         =   "Save Actor Classes"
   ClientHeight    =   990
   ClientLeft      =   2820
   ClientTop       =   10140
   ClientWidth     =   5220
   Icon            =   "SaveClas.frx":0000
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   990
   ScaleWidth      =   5220
   ShowInTaskbar   =   0   'False
   Begin VB.Frame Frame2 
      Caption         =   "Package to save"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   915
      Left            =   120
      TabIndex        =   3
      Top             =   60
      Width           =   3975
      Begin VB.ComboBox Combo1 
         Height          =   315
         Left            =   180
         Style           =   2  'Dropdown List
         TabIndex        =   4
         Top             =   420
         Width           =   3675
      End
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   4200
      TabIndex        =   1
      Top             =   600
      Width           =   975
   End
   Begin VB.CommandButton Save 
      Caption         =   "&Save"
      Default         =   -1  'True
      Height          =   375
      Left            =   4200
      TabIndex        =   0
      Top             =   180
      Width           =   975
   End
   Begin VB.Label ClassName 
      Caption         =   "ClassName (invisible)"
      Height          =   735
      Left            =   4200
      TabIndex        =   2
      Top             =   1020
      Visible         =   0   'False
      Width           =   975
   End
End
Attribute VB_Name = "frmSaveClass"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'
' Modal save actor class form called from frmClassBrowser
'
Option Explicit

Private Sub Cancel_Click()
    GlobalAbortedModal = 1
    Unload Me
End Sub

Private Sub Form_Load()
    Dim S As String, T As String
    Call Ed.MakeFormFit(Me)
    S = Ed.ServerGetProp("OBJ", "PACKAGES CLASS=Class")
    While S <> ""
        T = GrabCommaString(S)
        Combo1.AddItem T
    Wend
    Combo1.ListIndex = 0
End Sub

Private Sub Save_Click()
    GString = Combo1.Text
    GResult = 1
    GlobalAbortedModal = 0
    Unload Me
End Sub
