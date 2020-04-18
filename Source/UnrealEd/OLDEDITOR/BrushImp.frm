VERSION 5.00
Begin VB.Form frmBrushImp 
   Caption         =   "Import a brush"
   ClientHeight    =   3690
   ClientLeft      =   4395
   ClientTop       =   2940
   ClientWidth     =   4425
   ControlBox      =   0   'False
   Icon            =   "BrushImp.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3690
   ScaleWidth      =   4425
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   3000
      TabIndex        =   9
      Top             =   3240
      Width           =   1335
   End
   Begin VB.CommandButton Ok 
      Caption         =   "&Ok"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   8
      Top             =   3240
      Width           =   1095
   End
   Begin VB.PictureBox Picture1 
      AutoSize        =   -1  'True
      BorderStyle     =   0  'None
      Height          =   480
      Left            =   120
      Picture         =   "BrushImp.frx":030A
      ScaleHeight     =   480
      ScaleWidth      =   480
      TabIndex        =   6
      Top             =   2040
      Width           =   480
   End
   Begin VB.Frame Frame2 
      Caption         =   "Cleanup"
      Height          =   855
      Left            =   120
      TabIndex        =   3
      Top             =   1080
      Width           =   4215
      Begin VB.OptionButton Option4 
         Caption         =   "Keep original polygons intact"
         Height          =   255
         Left            =   120
         TabIndex        =   5
         Top             =   480
         Width           =   3975
      End
      Begin VB.OptionButton MergeCoplanars 
         Caption         =   "Merge coplanar polygons"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   240
         Value           =   -1  'True
         Width           =   3975
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Solidity"
      Height          =   855
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   4215
      Begin VB.OptionButton Option2 
         Caption         =   "Nonsolid (contains gaps or holes)"
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   480
         Width           =   3855
      End
      Begin VB.OptionButton Solid 
         Caption         =   "Solid mesh (continuous mesh, no gaps or holes)."
         Height          =   255
         Left            =   120
         TabIndex        =   1
         Top             =   240
         Value           =   -1  'True
         Width           =   3855
      End
   End
   Begin VB.Label Label1 
      Caption         =   $"BrushImp.frx":074C
      Height          =   975
      Left            =   720
      TabIndex        =   7
      Top             =   2040
      Width           =   3615
   End
End
Attribute VB_Name = "frmBrushImp"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Cancel_Click()
    Me.Hide
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
End Sub

Private Sub Ok_Click()
    Ed.BeginSlowTask "Importing brush"
    Ed.ServerExec "BRUSH IMPORT FILE=" & _
        Quotes(frmDialogs.ImportBrush.filename) & _
        " MERGE=" & OnOff(MergeCoplanars.Value) & _
        " FLAGS=" & IIf(Solid.Value, 0, PF_NOTSOLID)
    Ed.EndSlowTask
    '
    Me.Hide
End Sub
