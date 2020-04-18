VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmParSolCube 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build a Cube"
   ClientHeight    =   4395
   ClientLeft      =   5145
   ClientTop       =   4095
   ClientWidth     =   2490
   ControlBox      =   0   'False
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H00C0C0C0&
   HelpContextID   =   153
   Icon            =   "PsRect.frx":0000
   LinkTopic       =   "Form5"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   4395
   ScaleWidth      =   2490
   ShowInTaskbar   =   0   'False
   Begin Threed.SSPanel SSPanel1 
      Height          =   1455
      Left            =   120
      TabIndex        =   17
      Top             =   120
      Width           =   1455
      _Version        =   65536
      _ExtentX        =   2566
      _ExtentY        =   2566
      _StockProps     =   15
      ForeColor       =   -2147483640
      BackColor       =   12632256
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      BevelInner      =   1
      Begin VB.Line Line1 
         BorderColor     =   &H0000FFFF&
         X1              =   120
         X2              =   1080
         Y1              =   120
         Y2              =   120
      End
      Begin VB.Line Line2 
         BorderColor     =   &H0000FFFF&
         X1              =   1080
         X2              =   1080
         Y1              =   120
         Y2              =   1080
      End
      Begin VB.Line Line3 
         BorderColor     =   &H0000FFFF&
         X1              =   120
         X2              =   120
         Y1              =   120
         Y2              =   1080
      End
      Begin VB.Line Line4 
         BorderColor     =   &H0000FFFF&
         X1              =   120
         X2              =   1080
         Y1              =   1080
         Y2              =   1080
      End
      Begin VB.Line Line5 
         BorderColor     =   &H0000FFFF&
         X1              =   1320
         X2              =   1320
         Y1              =   360
         Y2              =   1320
      End
      Begin VB.Line Line6 
         BorderColor     =   &H0000FFFF&
         X1              =   360
         X2              =   1320
         Y1              =   1320
         Y2              =   1320
      End
      Begin VB.Line Line7 
         BorderColor     =   &H0000FFFF&
         X1              =   1080
         X2              =   1320
         Y1              =   120
         Y2              =   360
      End
      Begin VB.Line Line8 
         BorderColor     =   &H0000FFFF&
         X1              =   120
         X2              =   360
         Y1              =   1080
         Y2              =   1320
      End
      Begin VB.Line Line9 
         BorderColor     =   &H0000FFFF&
         X1              =   1080
         X2              =   1320
         Y1              =   1080
         Y2              =   1320
      End
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
      Left            =   960
      TabIndex        =   6
      Top             =   3960
      Width           =   615
   End
   Begin VB.OptionButton IsSolid 
      Caption         =   "Solid"
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
      Left            =   1680
      TabIndex        =   8
      Top             =   1080
      Value           =   -1  'True
      Width           =   735
   End
   Begin VB.OptionButton IsHollow 
      Caption         =   "Hollow"
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
      Left            =   1680
      TabIndex        =   9
      Top             =   1320
      Width           =   855
   End
   Begin VB.TextBox Group 
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
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   4
      Text            =   "Rect"
      Top             =   3120
      Width           =   1095
   End
   Begin VB.TextBox RWidth 
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
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   1
      Text            =   "256"
      Top             =   2040
      Width           =   1095
   End
   Begin VB.TextBox RBreadth 
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
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   2
      Text            =   "256"
      Top             =   2400
      Width           =   1095
   End
   Begin VB.TextBox RThickness 
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
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   3
      Text            =   "16"
      Top             =   2760
      Width           =   1095
   End
   Begin VB.TextBox RHeight 
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
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1320
      TabIndex        =   0
      Text            =   "256"
      Top             =   1680
      Width           =   1095
   End
   Begin VB.CommandButton Build 
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
      TabIndex        =   5
      Top             =   3960
      Width           =   735
   End
   Begin VB.CommandButton Command2 
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
      Left            =   1680
      TabIndex        =   7
      Top             =   3960
      Width           =   735
   End
   Begin VB.Label Trigger 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Trigger"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   1680
      TabIndex        =   16
      Top             =   720
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.Label Label10 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Group Name"
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
      TabIndex        =   15
      Top             =   3120
      Width           =   975
   End
   Begin VB.Label Label8 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Wall Thickness"
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
      Left            =   0
      TabIndex        =   14
      Top             =   2760
      Width           =   1215
   End
   Begin VB.Label Label7 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Height"
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
      Left            =   600
      TabIndex        =   13
      Top             =   1680
      Width           =   615
   End
   Begin VB.Label Label6 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Width"
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
      Left            =   720
      TabIndex        =   12
      Top             =   2040
      Width           =   495
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Breadth"
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
      Left            =   600
      TabIndex        =   11
      Top             =   2400
      Width           =   615
   End
   Begin VB.Label Label11 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Item Names are: Floor,Ceiling,Wall,Outside"
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
      Left            =   240
      TabIndex        =   10
      Top             =   3480
      Width           =   2175
   End
End
Attribute VB_Name = "frmParSolCube"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Build_Click()
    Dim HHeight As Single ' Half-height, etc
    Dim HWidth As Single
    Dim HBreadth As Single
    Dim WThickness As Single
    Dim Temp As Double
    Dim Group As String
    Dim Hollow As Integer
    Dim N As Integer
    '
    Call InitBrush("Cube")
    '
    ' Validate parameters
    '
    If Not Eval(RHeight, Temp) Then Exit Sub
    HHeight = Temp / 2
    '
    If Not Eval(RWidth, Temp) Then Exit Sub
    HWidth = Temp / 2
    '
    If Not Eval(RBreadth, Temp) Then Exit Sub
    HBreadth = Temp / 2
    '
    If Not Eval(RThickness, Temp) Then Exit Sub
    WThickness = Temp
    '
    Group = UCase$(Group)
    Hollow = Int(IsHollow.Value)
    '
    If HHeight <= 0 Or HWidth <= 0 Or HBreadth <= 0 Then
        MsgBox ("You must give all numbers, and they all must be positive and nonzero")
        Exit Sub
    End If
    '
    If Hollow Then
        If WThickness <= 0 Then
            MsgBox ("Thickness must be positive!")
            Exit Sub
        End If
        If WThickness >= HHeight Or WThickness >= HWidth Or WThickness >= HBreadth Then
            MsgBox ("Wall is too thick for its size!")
            Exit Sub
        End If
    End If
    '
    ' Build outside
    '
    Brush.NumPolys = 6
    Call MakeSymRectXY(1, 1, 1, HBreadth, HWidth, HHeight, Group, "OUTSIDE")
    Call MakeSymRectXY(2, 4, -1, HBreadth, HWidth, -HHeight, Group, "OUTSIDE")
    Call MakeSymRectXZ(3, 1, 1, HBreadth, HWidth, HHeight, Group, "OUTSIDE")
    Call MakeSymRectXZ(4, 4, -1, HBreadth, -HWidth, HHeight, Group, "OUTSIDE")
    Call MakeSymRectYZ(5, 4, -1, HBreadth, HWidth, HHeight, Group, "OUTSIDE")
    Call MakeSymRectYZ(6, 1, 1, -HBreadth, HWidth, HHeight, Group, "OUTSIDE")
    '
    ' Build inside:
    '
    If Hollow Then
        Brush.NumPolys = 12
        HHeight = HHeight - WThickness
        HWidth = HWidth - WThickness
        HBreadth = HBreadth - WThickness
        Call MakeSymRectXY(7, 4, -1, HBreadth, HWidth, HHeight, Group, "CEILING")
        Call MakeSymRectXY(8, 1, 1, HBreadth, HWidth, -HHeight, Group, "FLOOR")
        Call MakeSymRectXZ(9, 4, -1, HBreadth, HWidth, HHeight, Group, "WALL")
        Call MakeSymRectXZ(10, 1, 1, HBreadth, -HWidth, HHeight, Group, "WALL")
        Call MakeSymRectYZ(11, 1, 1, HBreadth, HWidth, HHeight, Group, "WALL")
        Call MakeSymRectYZ(12, 4, -1, -HBreadth, HWidth, HHeight, Group, "WALL")
    End If
    '
    Call SendBrush(0)
    Call Ed.StatusText("Built a Cube")
End Sub

Private Sub Command1_Click()
    ToolHelp (153)
End Sub

Private Sub Command2_Click()
    Hide
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "BuildCube", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Trigger_Change()
    Build_Click
End Sub

'
' Focus change highlighting routines.
'
Private Sub RHeight_GotFocus()
    SelectAll RHeight
End Sub

Private Sub RWidth_GotFocus()
    SelectAll RWidth
End Sub

Private Sub RBreadth_GotFocus()
    SelectAll RBreadth
End Sub

Private Sub RThickness_GotFocus()
    SelectAll RThickness
End Sub

Private Sub Group_GotFocus()
    SelectAll Group
End Sub


