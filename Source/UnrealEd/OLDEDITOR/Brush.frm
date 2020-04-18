VERSION 5.00
Begin VB.Form frmBrush 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Resize/Move Brush"
   ClientHeight    =   3240
   ClientLeft      =   2235
   ClientTop       =   3510
   ClientWidth     =   5250
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
   HelpContextID   =   107
   Icon            =   "Brush.frx":0000
   LinkTopic       =   "Form4"
   MDIChild        =   -1  'True
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3240
   ScaleWidth      =   5250
   ShowInTaskbar   =   0   'False
   Begin VB.Frame Frame3 
      Caption         =   "Move/Rotate"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   3015
      Left            =   120
      TabIndex        =   41
      Top             =   120
      Width           =   2415
      Begin VB.CommandButton YawZero 
         BackColor       =   &H00C0C0C0&
         Caption         =   "0"
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
         Left            =   2040
         TabIndex        =   28
         TabStop         =   0   'False
         Top             =   1920
         Width           =   255
      End
      Begin VB.CommandButton PitchZero 
         BackColor       =   &H00C0C0C0&
         Caption         =   "0"
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
         Left            =   2040
         TabIndex        =   31
         TabStop         =   0   'False
         Top             =   2280
         Width           =   255
      End
      Begin VB.CommandButton RollZero 
         BackColor       =   &H00C0C0C0&
         Caption         =   "0"
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
         Left            =   2040
         TabIndex        =   34
         TabStop         =   0   'False
         Top             =   2640
         Width           =   255
      End
      Begin VB.TextBox YawV 
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
         Left            =   600
         TabIndex        =   3
         Text            =   "0"
         Top             =   1920
         Width           =   735
      End
      Begin VB.TextBox PitchV 
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
         Left            =   600
         TabIndex        =   4
         Text            =   "0"
         Top             =   2280
         Width           =   735
      End
      Begin VB.TextBox RollV 
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
         Left            =   600
         TabIndex        =   5
         Text            =   "0"
         Top             =   2640
         Width           =   735
      End
      Begin VB.CommandButton YawMinus 
         BackColor       =   &H00C0C0C0&
         Caption         =   "<-"
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
         Left            =   1440
         TabIndex        =   26
         TabStop         =   0   'False
         Top             =   1920
         Width           =   255
      End
      Begin VB.CommandButton YawPlus 
         BackColor       =   &H00C0C0C0&
         Caption         =   "->"
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
         TabIndex        =   27
         TabStop         =   0   'False
         Top             =   1920
         Width           =   255
      End
      Begin VB.CommandButton PitchMinus 
         BackColor       =   &H00C0C0C0&
         Caption         =   "<-"
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
         Left            =   1440
         TabIndex        =   29
         TabStop         =   0   'False
         Top             =   2280
         Width           =   255
      End
      Begin VB.CommandButton PitchPlus 
         BackColor       =   &H00C0C0C0&
         Caption         =   "->"
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
         TabIndex        =   30
         TabStop         =   0   'False
         Top             =   2280
         Width           =   255
      End
      Begin VB.CommandButton RollMinus 
         BackColor       =   &H00C0C0C0&
         Caption         =   "<-"
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
         Left            =   1440
         TabIndex        =   32
         TabStop         =   0   'False
         Top             =   2640
         Width           =   255
      End
      Begin VB.CommandButton RollPlus 
         BackColor       =   &H00C0C0C0&
         Caption         =   "->"
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
         TabIndex        =   33
         TabStop         =   0   'False
         Top             =   2640
         Width           =   255
      End
      Begin VB.TextBox ZV 
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
         Left            =   600
         TabIndex        =   0
         Text            =   "0"
         Top             =   840
         Width           =   1335
      End
      Begin VB.CommandButton Move 
         BackColor       =   &H00C0C0C0&
         Caption         =   "&Move"
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
         Height          =   255
         Left            =   840
         TabIndex        =   24
         Top             =   510
         Width           =   735
      End
      Begin VB.CommandButton Reset 
         BackColor       =   &H00C0C0C0&
         Caption         =   "&Reset"
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
         TabIndex        =   23
         Top             =   510
         Width           =   615
      End
      Begin VB.CommandButton Unmove 
         BackColor       =   &H00C0C0C0&
         Caption         =   "&Back"
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
         TabIndex        =   25
         Top             =   510
         Width           =   615
      End
      Begin VB.TextBox YV 
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
         Left            =   600
         TabIndex        =   2
         Text            =   "0"
         Top             =   1560
         Width           =   1335
      End
      Begin VB.TextBox XV 
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
         Left            =   600
         TabIndex        =   1
         Text            =   "0"
         Top             =   1200
         Width           =   1335
      End
      Begin VB.OptionButton MoveAbs 
         Caption         =   "&Absolute"
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
         Left            =   1200
         TabIndex        =   22
         TabStop         =   0   'False
         Top             =   240
         Width           =   975
      End
      Begin VB.OptionButton MoveRel 
         Caption         =   "R&elative"
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
         TabIndex        =   21
         TabStop         =   0   'False
         Top             =   240
         Value           =   -1  'True
         Width           =   1095
      End
      Begin VB.Label Label8 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Roll"
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
         TabIndex        =   54
         Top             =   2640
         Width           =   495
      End
      Begin VB.Label Label9 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Pitch"
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
         TabIndex        =   53
         Top             =   2280
         Width           =   495
      End
      Begin VB.Label Label10 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Yaw"
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
         TabIndex        =   52
         Top             =   1920
         Width           =   495
      End
      Begin VB.Label Label11 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Z"
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
         TabIndex        =   40
         Top             =   840
         Width           =   495
      End
      Begin VB.Label Label12 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
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
         TabIndex        =   39
         Top             =   1560
         Width           =   495
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "X"
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
         TabIndex        =   42
         Top             =   1200
         Width           =   495
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Scaling && Sheering"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   3015
      Left            =   2640
      TabIndex        =   35
      Top             =   120
      Width           =   2535
      Begin VB.CommandButton Command1 
         Caption         =   "Reset"
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
         Left            =   840
         TabIndex        =   56
         Top             =   2760
         Width           =   855
      End
      Begin VB.CommandButton Help 
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
         Height          =   255
         Left            =   1920
         TabIndex        =   19
         Top             =   1920
         Width           =   615
      End
      Begin VB.CommandButton Close 
         Caption         =   "Close"
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
         Left            =   1800
         TabIndex        =   20
         Top             =   2760
         Width           =   735
      End
      Begin VB.CommandButton Apply 
         BackColor       =   &H00C0C0C0&
         Caption         =   "&Go"
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
         Left            =   1920
         TabIndex        =   55
         Top             =   360
         Width           =   495
      End
      Begin VB.OptionButton YZSheer 
         Caption         =   "YZ"
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
         Left            =   1575
         TabIndex        =   17
         TabStop         =   0   'False
         Top             =   2475
         Width           =   615
      End
      Begin VB.CommandButton Perm 
         Caption         =   "Perm"
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
         TabIndex        =   18
         Top             =   2760
         Width           =   735
      End
      Begin VB.CommandButton FlipX 
         Caption         =   "Flip"
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
         Left            =   1920
         TabIndex        =   51
         Top             =   1080
         Width           =   495
      End
      Begin VB.CommandButton FlipZ 
         Caption         =   "Flip"
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
         Left            =   1920
         TabIndex        =   46
         Top             =   720
         Width           =   495
      End
      Begin VB.CommandButton FlipY 
         Caption         =   "Flip"
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
         Left            =   1920
         TabIndex        =   50
         Top             =   1440
         Width           =   495
      End
      Begin VB.OptionButton YXSheer 
         Caption         =   "YX"
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
         Left            =   1575
         TabIndex        =   16
         TabStop         =   0   'False
         Top             =   2235
         Width           =   615
      End
      Begin VB.OptionButton XYSheer 
         Caption         =   "XY"
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
         Left            =   975
         TabIndex        =   14
         TabStop         =   0   'False
         Top             =   2235
         Width           =   615
      End
      Begin VB.TextBox LSheer 
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
         Left            =   120
         TabIndex        =   10
         Text            =   "0"
         Top             =   1920
         Width           =   615
      End
      Begin VB.OptionButton XZSheer 
         Caption         =   "XZ"
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
         Left            =   975
         TabIndex        =   15
         TabStop         =   0   'False
         Top             =   2475
         Width           =   615
      End
      Begin VB.CommandButton Command2 
         Caption         =   "0"
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
         Left            =   1440
         TabIndex        =   11
         TabStop         =   0   'False
         Top             =   1920
         Width           =   255
      End
      Begin VB.OptionButton ZYSheer 
         Caption         =   "ZY"
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
         Left            =   375
         TabIndex        =   13
         TabStop         =   0   'False
         Top             =   2475
         Width           =   615
      End
      Begin VB.OptionButton ZXSheer 
         Caption         =   "ZX"
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
         Left            =   375
         TabIndex        =   12
         TabStop         =   0   'False
         Top             =   2235
         Value           =   -1  'True
         Width           =   615
      End
      Begin VB.TextBox LZ 
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
         Left            =   120
         TabIndex        =   7
         Text            =   "100"
         Top             =   720
         Width           =   855
      End
      Begin VB.CommandButton z100 
         Caption         =   "100"
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
         TabIndex        =   47
         TabStop         =   0   'False
         Top             =   720
         Width           =   495
      End
      Begin VB.CommandButton x100 
         Caption         =   "100"
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
         TabIndex        =   45
         TabStop         =   0   'False
         Top             =   1080
         Width           =   495
      End
      Begin VB.CommandButton Prop100 
         Caption         =   "100"
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
         TabIndex        =   44
         TabStop         =   0   'False
         Top             =   360
         Width           =   495
      End
      Begin VB.CommandButton y100 
         Caption         =   "100"
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
         TabIndex        =   43
         TabStop         =   0   'False
         Top             =   1440
         Width           =   495
      End
      Begin VB.TextBox LY 
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
         Left            =   120
         TabIndex        =   9
         Text            =   "100"
         Top             =   1440
         Width           =   855
      End
      Begin VB.TextBox LX 
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
         Left            =   120
         TabIndex        =   8
         Text            =   "100"
         Top             =   1080
         Width           =   855
      End
      Begin VB.TextBox LProp 
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
         Left            =   120
         TabIndex        =   6
         Text            =   "100"
         Top             =   360
         Width           =   855
      End
      Begin VB.Label Label6 
         Alignment       =   2  'Center
         BackStyle       =   0  'Transparent
         Caption         =   "Sheer"
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
         Left            =   840
         TabIndex        =   49
         Top             =   1920
         Width           =   615
      End
      Begin VB.Label Label3 
         Caption         =   "Z"
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
         Left            =   1080
         TabIndex        =   48
         Top             =   720
         Width           =   255
      End
      Begin VB.Label Label2 
         Caption         =   "All"
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
         Left            =   1080
         TabIndex        =   36
         Top             =   360
         Width           =   255
      End
      Begin VB.Label Label4 
         Caption         =   "Y"
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
         Left            =   1080
         TabIndex        =   37
         Top             =   1440
         Width           =   255
      End
      Begin VB.Label Label5 
         Caption         =   "X"
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
         Left            =   1080
         TabIndex        =   38
         Top             =   1080
         Width           =   255
      End
   End
End
Attribute VB_Name = "frmBrush"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Const GWW_HWNDPARENT = (-8)
Dim ObjPropWord As Integer

Dim Resetting As Integer
Dim DoingIt As Integer

Dim Pitch As Long
Dim Yaw As Long
Dim Roll As Long

Private Sub Command1_Click()
    LProp.Text = "100"
    LX.Text = "100"
    LY.Text = "100"
    LZ.Text = "100"
    LSheer.Text = "0"
    Ed.Server.Exec "BRUSH SCALE RESET"
End Sub

Private Sub MoveAbs_Click()
    Unmove.Enabled = False
End Sub

Private Sub MoveRel_Click()
    Unmove.Enabled = True
End Sub

Private Sub Perm_Click()
    '
    ' This applies the current scale/sheer transformation
    ' to the brush permanently and resets scaling to
    ' defaults.
    '
    Ed.Server.Exec "BRUSH APPLYTRANSFORM"
    '
    Resetting = True ' Prevent multiple updates
    '
    LProp.Text = 100
    LX.Text = 100
    LY.Text = 100
    LZ.Text = 100
    LSheer.Text = 0
    '
    If MoveRel.Value Then ' Relative rotation
        Ed.Server.Exec "BRUSH ROTATETO PITCH=0 YAW=0 ROLL=0"
    Else ' Absolute rotation
        YawV.Text = 0
        PitchV.Text = 0
        RollV.Text = 0
    End If
    '
    Resetting = False
    '
    SendScale (0)
    '
End Sub

Private Sub Command2_Click()
    LSheer.Text = 0
End Sub

Private Sub FlipX_Click()
    LX.Text = Str(-Val(LX.Text))
    SendScale (0)
End Sub

Private Sub FlipY_Click()
    LY.Text = Str(-Val(LY.Text))
    SendScale (0)
End Sub

Private Sub FlipZ_Click()
    LZ.Text = Str(-Val(LZ.Text))
    SendScale (0)
End Sub

Private Sub Apply_Click()
    SendScale (0)
End Sub

Private Sub Close_Click()
    Hide
End Sub

Private Sub Help_Click()
    SendKeys "{F1}"
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "MoveRotateBrush", TOP_PANEL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub LProp_Click()
    SendScale (0)
End Sub

Private Sub LProp_LostFocus()
    SendScale (0)
End Sub

Private Sub LSheer_Click()
    SendScale (1)
End Sub

Private Sub LSheer_LostFocus()
    SendScale (1)
End Sub

Private Sub LX_Click()
    SendScale (0)
End Sub

Private Sub LX_LostFocus()
    SendScale (0)
End Sub

Private Sub LY_Click()
    SendScale (0)
End Sub

Private Sub LY_LostFocus()
    SendScale (0)
End Sub

Private Sub LZ_Click()
    SendScale (0)
End Sub

Private Sub LZ_LostFocus()
    SendScale (0)
End Sub

Private Sub Move_Click()
    SendMove (1)
    Call SendRotation(MoveRel.Value, 1)
End Sub

Private Sub PitchMinus_Click()
    If CalcRotation Then
        If MoveAbs.Value Then
            PitchV.Text = Trim(Str((Pitch - 32) And 255))
        ElseIf Val(PitchV.Text) = 0 Then
            Call SendRotationNum(True, -32, 0, 0)
        Else
            Call SendRotationNum(True, -Pitch, 0, 0)
        End If
    End If
End Sub

Private Sub PitchPlus_Click()
    If CalcRotation Then
        If MoveAbs.Value Then
            PitchV.Text = Trim(Str((Pitch + 32) And 255))
        ElseIf Val(PitchV.Text) = 0 Then
            Call SendRotationNum(True, 32, 0, 0)
        Else
            Call SendRotationNum(True, Pitch, 0, 0)
        End If
    End If
End Sub

Private Sub PitchZero_Click()
    PitchV.Text = "0"
    If (MoveRel.Value) Then
        Ed.Server.Exec "BRUSH ROTATETO PITCH=0"
    End If
End Sub


Private Sub Prop100_Click()
    LProp.Text = 100
    SendScale (0)
End Sub

Private Sub Reset_Click()
    Resetting = True ' To prevent sending stuff
    '
    XV.Text = 0
    YV.Text = 0
    ZV.Text = 0
    YawV.Text = 0
    PitchV.Text = 0
    RollV.Text = 0
    '
    Resetting = False
    Call SendRotation(MoveRel.Value, 1)
End Sub

Private Sub RollMinus_Click()
    If CalcRotation Then
        If MoveAbs.Value Then
            RollV.Text = Trim(Str((Roll - 32) And 255))
        ElseIf Roll = 0 Then
            Call SendRotationNum(True, 0, 0, -32)
        Else
            Call SendRotationNum(True, 0, 0, -Roll)
        End If
    End If
End Sub

Private Sub RollPlus_Click()
    If CalcRotation Then
        If MoveAbs.Value Then
            RollV.Text = Trim(Str((Roll + 32) And 255))
        ElseIf Val(RollV.Text) = 0 Then
            Call SendRotationNum(True, 0, 0, 32)
        Else
            Call SendRotationNum(True, 0, 0, Roll)
        End If
    End If
End Sub

Private Sub RollZero_Click()
    RollV.Text = 0
    Call SendRotation(False, 1)
    If (MoveRel.Value) Then
        Ed.Server.Exec "BRUSH ROTATETO ROLL=0"
    End If
End Sub


Private Sub SendMove(Direction As Single)
    Dim Cmd As String
    Dim XX As Single, YY As Single, ZZ As Single
    Dim Temp As Double
    '
    If MoveRel.Value Then
        Cmd = "BRUSH MOVEREL" ' Move relative
    Else
        Cmd = "BRUSH MOVETO" ' Move to absolute location
    End If
    '
    If Not Eval(XV, Temp) Then Exit Sub
    XX = Bound(Temp, -32767, 32767)
    '
    If Not Eval(YV, Temp) Then Exit Sub
    YY = Bound(Temp, -32767, 32767)
    '
    If Not Eval(ZV, Temp) Then Exit Sub
    ZZ = Bound(Temp, -32767, 32767)
    '
    Cmd = Cmd & " X=" + Trim(Str(XX * Direction))
    Cmd = Cmd & " Y=" + Trim(Str(YY * Direction))
    Cmd = Cmd & " Z=" + Trim(Str(ZZ * Direction))
    '
    Ed.Server.Exec Cmd
    '
End Sub

Function CalcRotation() As Boolean
    CalcRotation = False
    Dim Temp As Double
    '
    If Not Eval(YawV, Temp) Then Exit Function
    Yaw = Int(Temp)
    '
    If Not Eval(PitchV, Temp) Then Exit Function
    Pitch = Int(Temp)
    '
    If Not Eval(RollV, Temp) Then Exit Function
    Roll = Int(Temp)
    '
    CalcRotation = True
End Function

Private Sub SendRotation(ByVal MoveRel As Integer, Sign As Single)
    Dim Cmd As Integer
    '
    If Not Resetting Then
        If CalcRotation Then
            Call SendRotationNum(MoveRel, Pitch * Sign, Yaw * Sign, Roll * Sign)
        End If
    End If
    '
End Sub

Private Sub SendRotationNum(ByVal MoveRel As Long, Pitch As Long, Yaw As Long, Roll As Long)
    Dim Cmd As String
    '
    If MoveRel Then
        Cmd = "BRUSH ROTATEREL" ' Rotate relative
    Else
        Cmd = "BRUSH ROTATETO"  ' Rotate to absolute position
    End If
    '
    Cmd = Cmd & " PITCH=" + Trim(Str(Pitch And 255))
    Cmd = Cmd & " YAW=" + Trim(Str(Yaw And 255))
    Cmd = Cmd & " ROLL=" + Trim(Str(Roll And 255))
    '
    Ed.Server.Exec Cmd
End Sub

Private Sub SendScale(SendSheer As Integer)
    '
    Dim Sheer As Double
    Dim P As Double
    Dim Cmd As String
    '
    Cmd = "BRUSH SCALE"
    '
    ' Scaling
    '
    P = Val(LProp.Text)
    If (P = 0#) Then
        P = 1#
    End If
    P = P / 10000# ' Turn 100*100 into 1.0
    '
    If Val(LX.Text) <> 0 Then
        Cmd = Cmd & " X=" & Trim(Str(P * Val(LX.Text)))
    End If
    '
    If Val(LY.Text) <> 0 Then
        Cmd = Cmd & " Y=" & Trim(Str(Val(P * LY.Text)))
    End If
    '
    If Val(LZ.Text) <> 0 Then
        Cmd = Cmd & " Z=" & Trim(Str(Val(P * LZ.Text)))
    End If
    '
    ' Simple sheering
    '
    Sheer = Val(LSheer.Text) / 100
    If SendSheer Or Sheer <> 0 Then
        If XYSheer.Value Then
            Cmd = Cmd & " SHEERAXIS=XY"
        ElseIf XZSheer.Value Then
            Cmd = Cmd & " SHEERAXIS=XZ"
        ElseIf YXSheer.Value Then
            Cmd = Cmd & " SHEERAXIS=YX"
        ElseIf YZSheer.Value Then
            Cmd = Cmd & " SHEERAXIS=YZ"
        ElseIf ZXSheer.Value Then
            Cmd = Cmd & " SHEERAXIS=ZX"
        ElseIf ZYSheer.Value Then
            Cmd = Cmd & " SHEERAXIS=ZY"
        End If
        '
        Cmd = Cmd & " SHEER=" & Trim(Str(Sheer))
    End If
    '
    If Not Resetting Then
        Ed.Server.Exec Cmd
    End If
    '
End Sub

Private Sub Unmove_Click()
    SendMove (-1)
    Call SendRotation(MoveRel.Value, -1)
End Sub

Private Sub x100_Click()
    LX.Text = 100
    SendScale (0)
End Sub

Private Sub XYSheer_Click()
    SendScale (1)
End Sub

Private Sub XZSheer_Click()
    SendScale (1)
End Sub


Private Sub y100_Click()
    LY.Text = 100
    SendScale (0)
End Sub

Private Sub YawMinus_Click()
    If CalcRotation Then
        If MoveAbs.Value Then
            YawV.Text = Trim(Str((Yaw - 32) And 255))
        ElseIf Yaw = 0 Then
            Call SendRotationNum(True, 0, -32, 0)
        Else
            Call SendRotationNum(True, 0, -Yaw, 0)
        End If
    End If
End Sub

Private Sub YawPlus_Click()
    If CalcRotation Then
        If MoveAbs.Value Then
            YawV.Text = Trim(Str((Yaw + 32) And 255))
        ElseIf Val(YawV.Text) = 0 Then
            Call SendRotationNum(True, 0, 32, 0)
        Else
            Call SendRotationNum(True, 0, Yaw, 0)
        End If
    End If
End Sub

Private Sub YawZero_Click()
    YawV.Text = 0
    If (MoveRel.Value) Then
        Ed.Server.Exec "BRUSH ROTATETO YAW=0"
    End If
End Sub

Private Sub YXSheer_Click()
    SendScale (1)
End Sub

Private Sub YZSheer_Click()
    SendScale (1)
End Sub

Private Sub z100_Click()
    LZ.Text = 100
    SendScale (0)
End Sub

Private Sub ZXSheer_Click()
    SendScale (1)
End Sub

Private Sub ZYSheer_Click()
    SendScale (1)
End Sub

