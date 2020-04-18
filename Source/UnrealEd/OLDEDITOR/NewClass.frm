VERSION 5.00
Begin VB.Form frmNewClass 
   Caption         =   "Create a new actor class"
   ClientHeight    =   2175
   ClientLeft      =   3765
   ClientTop       =   8385
   ClientWidth     =   4590
   Icon            =   "NewClass.frx":0000
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   2175
   ScaleWidth      =   4590
   ShowInTaskbar   =   0   'False
   Begin VB.Frame Frame1 
      Caption         =   "Actor Class"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1575
      Left            =   120
      TabIndex        =   4
      Top             =   120
      Width           =   4335
      Begin VB.TextBox NewPackageName 
         Height          =   285
         Left            =   2040
         TabIndex        =   1
         Top             =   1080
         Width           =   2175
      End
      Begin VB.TextBox NewClassName 
         Height          =   285
         Left            =   2040
         TabIndex        =   0
         Top             =   720
         Width           =   2175
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Package name:"
         Height          =   255
         Left            =   240
         TabIndex        =   8
         Top             =   1080
         Width           =   1695
      End
      Begin VB.Label ParentClassName 
         Caption         =   "ParentClassName"
         Height          =   255
         Left            =   2040
         TabIndex        =   7
         Top             =   360
         Width           =   1695
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "New actor class name:"
         Height          =   255
         Left            =   240
         TabIndex        =   6
         Top             =   720
         Width           =   1695
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "Parent class:"
         Height          =   255
         Left            =   360
         TabIndex        =   5
         Top             =   360
         Width           =   1575
      End
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   3600
      TabIndex        =   3
      Top             =   1800
      Width           =   855
   End
   Begin VB.CommandButton Ok 
      Caption         =   "C&reate this actor class"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   2
      Top             =   1800
      Width           =   1815
   End
End
Attribute VB_Name = "frmNewClass"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Cancel_Click()
    GResult = 0
    Unload Me
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub Ok_Click()
    GResult = 1
    GString = NewClassName.Text
    GPackage = NewPackageName.Text
    Unload Me
End Sub
