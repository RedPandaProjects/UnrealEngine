VERSION 5.00
Begin VB.Form frmPsSheet 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build a Sheet"
   ClientHeight    =   3915
   ClientLeft      =   7515
   ClientTop       =   2895
   ClientWidth     =   2280
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
   HelpContextID   =   330
   Icon            =   "PsSheet.frx":0000
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3915
   ScaleWidth      =   2280
   ShowInTaskbar   =   0   'False
   Begin VB.OptionButton SheetFloor 
      Caption         =   "Floor/Ceiling"
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
      TabIndex        =   6
      Top             =   360
      Value           =   -1  'True
      Width           =   1335
   End
   Begin VB.OptionButton SheetXWall 
      Caption         =   "X-Wall"
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
      TabIndex        =   7
      Top             =   600
      Width           =   1095
   End
   Begin VB.OptionButton SheetYWall 
      Caption         =   "Y-Wall"
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
      Top             =   840
      Width           =   1095
   End
   Begin VB.TextBox SheetU 
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
      Left            =   480
      TabIndex        =   0
      Text            =   "128"
      Top             =   1425
      Width           =   1095
   End
   Begin VB.TextBox SheetV 
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
      Left            =   480
      TabIndex        =   1
      Text            =   "128"
      Top             =   1665
      Width           =   1095
   End
   Begin VB.TextBox SheetGroup 
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
      Left            =   720
      TabIndex        =   2
      Text            =   "Sheet"
      Top             =   2040
      Width           =   855
   End
   Begin VB.CommandButton SheetBuild 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Build"
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
      TabIndex        =   3
      Top             =   3480
      Width           =   615
   End
   Begin VB.CommandButton Command3 
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Close"
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
      Left            =   1560
      TabIndex        =   5
      Top             =   3480
      Width           =   615
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Help"
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
      Left            =   840
      TabIndex        =   4
      Top             =   3480
      Width           =   615
   End
   Begin VB.Label Label1 
      Caption         =   "This brush builder should only be used with the 'Add Special' dialog, not the regular add and subtract functions."
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   15
      Top             =   2400
      Width           =   2055
   End
   Begin VB.Label Trigger 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Trigger"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   1560
      TabIndex        =   14
      Top             =   120
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.Label Label2 
      Caption         =   "Orientation:"
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
      TabIndex        =   13
      Top             =   120
      Width           =   975
   End
   Begin VB.Label Label3 
      Caption         =   "Size:"
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
      Top             =   1200
      Width           =   855
   End
   Begin VB.Label Label4 
      Caption         =   "U:"
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
      TabIndex        =   11
      Top             =   1455
      Width           =   255
   End
   Begin VB.Label Label5 
      Caption         =   "V:"
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
      TabIndex        =   10
      Top             =   1695
      Width           =   255
   End
   Begin VB.Label Label6 
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
      Left            =   120
      TabIndex        =   9
      Top             =   2040
      Width           =   615
   End
End
Attribute VB_Name = "frmPsSheet"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()
    ToolHelp (330)
End Sub

Private Sub Command3_Click()
    Me.Hide
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "SheetBrush", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub


Private Sub Trigger_Change()
    SheetBuild_Click
End Sub

Private Sub SheetBuild_Click()
    Dim U As Single
    Dim V As Single
    Dim Temp As Double
    '
    Call InitBrush("Sheet")
    '
    ' Validate parameters
    '
    If Not Eval(SheetU, Temp) Then Exit Sub
    U = Temp / 2
    '
    If Not Eval(SheetV, Temp) Then Exit Sub
    V = Temp / 2
    '
    If (U < 1) Or (V < 1) Then Exit Sub
    '
    Brush.NumPolys = 1
    If SheetFloor.Value Then
        Call MakeSymRectXY(1, 1, 1, U, V, 0, SheetGroup, "Sheet")
    ElseIf SheetXWall.Value Then
        Call MakeSymRectYZ(1, 1, 1, 0, U, V, SheetGroup, "Sheet")
    ElseIf SheetYWall.Value Then
        Call MakeSymRectXZ(1, 1, 1, U, 0, V, SheetGroup, "Sheet")
    End If
    Brush.Polys(1).Flags = PF_NOTSOLID
    '
    Call SendBrush(0)
    Call Ed.StatusText("Built a Sheet")
End Sub

'
' Focus change highlighting routines.
'
Private Sub SheetU_GotFocus()
    SelectAll SheetU
End Sub

Private Sub SheetV_GotFocus()
    SelectAll SheetV
End Sub

Private Sub SheetGroup_GotFocus()
    SelectAll SheetGroup
End Sub
