VERSION 5.00
Begin VB.Form frmParSolHeightMap 
   Appearance      =   0  'Flat
   BackColor       =   &H80000005&
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build Landscape"
   ClientHeight    =   5535
   ClientLeft      =   1365
   ClientTop       =   4035
   ClientWidth     =   11565
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
   ForeColor       =   &H80000008&
   Icon            =   "PsHMap.frx":0000
   LinkTopic       =   "Form6"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5535
   ScaleWidth      =   11565
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton Command10 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Import..."
      Height          =   375
      Left            =   3600
      TabIndex        =   20
      Top             =   4680
      Width           =   975
   End
   Begin VB.CheckBox Check3 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Invert terrain"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   720
      TabIndex        =   21
      Top             =   3840
      Width           =   1695
   End
   Begin VB.HScrollBar HScroll3 
      Height          =   255
      Left            =   960
      TabIndex        =   22
      Top             =   4800
      Width           =   1455
   End
   Begin VB.CheckBox Check4 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Use picture's dimensions"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   720
      TabIndex        =   23
      Top             =   4080
      Value           =   1  'Checked
      Width           =   2535
   End
   Begin VB.HScrollBar HScroll1 
      Height          =   255
      Left            =   7560
      TabIndex        =   53
      Top             =   3120
      Width           =   1335
   End
   Begin VB.HScrollBar HScroll2 
      Height          =   255
      Left            =   7560
      TabIndex        =   52
      Top             =   3840
      Width           =   1335
   End
   Begin VB.CommandButton Command9 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Apply Terrain"
      Height          =   375
      Left            =   9960
      TabIndex        =   51
      Top             =   4800
      Width           =   1335
   End
   Begin VB.CheckBox Check1 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Add to existing terrain"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   7320
      TabIndex        =   50
      Top             =   4680
      Width           =   2175
   End
   Begin VB.CheckBox Check2 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Affect only editing range"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   7320
      TabIndex        =   49
      Top             =   4920
      Width           =   2415
   End
   Begin VB.CommandButton Command11 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "<-"
      Height          =   255
      Left            =   9840
      TabIndex        =   48
      Top             =   3120
      Width           =   255
   End
   Begin VB.CommandButton Command12 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "->"
      Height          =   255
      Left            =   10800
      TabIndex        =   47
      Top             =   3120
      Width           =   255
   End
   Begin VB.TextBox Text7 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Height          =   285
      Left            =   10200
      TabIndex        =   46
      Text            =   "4"
      Top             =   3120
      Width           =   495
   End
   Begin VB.CommandButton Command1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Smooth"
      Height          =   255
      Left            =   7440
      TabIndex        =   15
      Top             =   2160
      Width           =   855
   End
   Begin VB.CommandButton Command3 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Sharpen"
      Height          =   255
      Left            =   8400
      TabIndex        =   16
      Top             =   2160
      Width           =   855
   End
   Begin VB.OptionButton Option1 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Entire Brush"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   9600
      TabIndex        =   45
      Top             =   720
      Width           =   1335
   End
   Begin VB.OptionButton Option2 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Editing range only"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   9000
      TabIndex        =   44
      Top             =   1080
      Width           =   1935
   End
   Begin VB.TextBox Text6 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Height          =   285
      Left            =   8520
      TabIndex        =   43
      Text            =   "4"
      Top             =   600
      Width           =   495
   End
   Begin VB.CommandButton Command6 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "->"
      Height          =   255
      Left            =   9000
      TabIndex        =   42
      Top             =   600
      Width           =   255
   End
   Begin VB.CommandButton Command7 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "<-"
      Height          =   255
      Left            =   8280
      TabIndex        =   41
      Top             =   600
      Width           =   255
   End
   Begin VB.CommandButton Command8 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Noise"
      Height          =   255
      Left            =   9360
      TabIndex        =   40
      Top             =   2160
      Width           =   855
   End
   Begin VB.OptionButton Option3 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Linear"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   5280
      TabIndex        =   37
      Top             =   1560
      Width           =   1455
   End
   Begin VB.OptionButton Option4 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Quadratic"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   5280
      TabIndex        =   36
      Top             =   1320
      Value           =   -1  'True
      Width           =   1455
   End
   Begin VB.TextBox Text5 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Height          =   285
      Left            =   5760
      TabIndex        =   35
      Text            =   "4"
      Top             =   480
      Width           =   495
   End
   Begin VB.CommandButton Command4 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "->"
      Height          =   255
      Left            =   6360
      TabIndex        =   34
      Top             =   480
      Width           =   255
   End
   Begin VB.CommandButton Command5 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "<-"
      Height          =   255
      Left            =   5400
      TabIndex        =   33
      Top             =   480
      Width           =   255
   End
   Begin VB.OptionButton Option5 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Rough (no falloff)"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   5280
      TabIndex        =   32
      Top             =   1800
      Width           =   2055
   End
   Begin VB.OptionButton Option6 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Blumpy"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   5280
      TabIndex        =   31
      Top             =   2040
      Width           =   975
   End
   Begin VB.CheckBox Check6 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Special"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   3360
      TabIndex        =   14
      Top             =   1200
      Width           =   975
   End
   Begin VB.CheckBox Check5 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "Brush"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   3360
      TabIndex        =   28
      Top             =   960
      Width           =   735
   End
   Begin VB.TextBox Text4 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      ForeColor       =   &H00FFFF00&
      Height          =   285
      Left            =   1320
      TabIndex        =   12
      Text            =   "500"
      Top             =   480
      Width           =   735
   End
   Begin VB.TextBox Text3 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      ForeColor       =   &H00FFFF00&
      Height          =   285
      Left            =   1320
      TabIndex        =   3
      Text            =   "Scape"
      Top             =   840
      Width           =   975
   End
   Begin VB.TextBox Text2 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      ForeColor       =   &H00FFFF00&
      Height          =   285
      Left            =   1320
      TabIndex        =   0
      Text            =   "50"
      Top             =   120
      Width           =   735
   End
   Begin VB.TextBox Text1 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      ForeColor       =   &H00FFFF00&
      Height          =   285
      Left            =   3360
      TabIndex        =   1
      Text            =   "20"
      Top             =   120
      Width           =   735
   End
   Begin VB.TextBox Height 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      ForeColor       =   &H00FFFF00&
      Height          =   285
      Left            =   3360
      TabIndex        =   2
      Text            =   "20"
      Top             =   480
      Width           =   735
   End
   Begin VB.CommandButton Build 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Build"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   4
      Top             =   1800
      Width           =   975
   End
   Begin VB.CommandButton Command2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Close"
      Height          =   375
      Left            =   3480
      TabIndex        =   6
      Top             =   1800
      Width           =   975
   End
   Begin VB.Label Label16 
      Appearance      =   0  'Flat
      BackColor       =   &H00808080&
      Caption         =   "You can import a terrain map from a PCX file.  The file must have 256 shades of gray and a linear palette."
      ForeColor       =   &H80000008&
      Height          =   615
      Left            =   720
      TabIndex        =   24
      Top             =   2760
      Width           =   3615
   End
   Begin VB.Label Label17 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Import Options"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   600
      TabIndex        =   25
      Top             =   3480
      Width           =   1695
   End
   Begin VB.Label Label18 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Import Scale"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   600
      TabIndex        =   26
      Top             =   4440
      Width           =   1575
   End
   Begin VB.Label Label12 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Fractal base level"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   7320
      TabIndex        =   29
      Top             =   3480
      Width           =   2535
   End
   Begin VB.Label Label13 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Terrain: Mostly water"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   9000
      TabIndex        =   30
      Top             =   3840
      Width           =   2175
   End
   Begin VB.Label Label14 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Magnitude"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   7320
      TabIndex        =   57
      Top             =   2760
      Width           =   2535
   End
   Begin VB.Label Label15 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Options"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   7320
      TabIndex        =   56
      Top             =   4320
      Width           =   1455
   End
   Begin VB.Label Label20 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "50%"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   9000
      TabIndex        =   55
      Top             =   3120
      Width           =   855
   End
   Begin VB.Label Label21 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Scale"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   9840
      TabIndex        =   54
      Top             =   2760
      Width           =   735
   End
   Begin VB.Label Label9 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Apply Filter"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   7800
      TabIndex        =   17
      Top             =   1800
      Width           =   2535
   End
   Begin VB.Label Label10 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Filter Magnitude"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   7920
      TabIndex        =   18
      Top             =   120
      Width           =   2535
   End
   Begin VB.Label Label11 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Area to filter"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   7680
      TabIndex        =   19
      Top             =   1440
      Width           =   2535
   End
   Begin VB.Label Label6 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Radius of editing range"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   4680
      TabIndex        =   39
      Top             =   120
      Width           =   3015
   End
   Begin VB.Label Label8 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Editing range falloff"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   375
      Left            =   4680
      TabIndex        =   38
      Top             =   960
      Width           =   2415
   End
   Begin VB.Label Label19 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Type"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   2760
      TabIndex        =   27
      Top             =   960
      Width           =   495
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "X Cells"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   2640
      TabIndex        =   13
      Top             =   120
      Width           =   615
   End
   Begin VB.Label Trigger 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Trigger"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   1800
      TabIndex        =   11
      Top             =   1920
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.Label Label4 
      Alignment       =   2  'Center
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Item Name is Cell"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   0
      TabIndex        =   10
      Top             =   1200
      Width           =   1815
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Group Name"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   120
      TabIndex        =   9
      Top             =   840
      Width           =   1095
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Heigh Span"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   120
      TabIndex        =   8
      Top             =   480
      Width           =   1095
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Y Cells"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   2640
      TabIndex        =   7
      Top             =   480
      Width           =   615
   End
   Begin VB.Label Label7 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BackStyle       =   0  'Transparent
      Caption         =   "Side Length"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   120
      Width           =   1095
   End
End
Attribute VB_Name = "frmParSolHeightMap"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Build_Click()
    '
    '
    '
End Sub

Private Sub Command2_Click()
    Hide
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, frmMain.hwnd, TOP_NORMAL)
End Sub

Private Sub Trigger_Change()
    Build_Click
End Sub

