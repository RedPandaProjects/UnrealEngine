VERSION 4.00
Begin VB.Form frmPolyProps 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Polygon Properties"
   ClientHeight    =   2595
   ClientLeft      =   2490
   ClientTop       =   4770
   ClientWidth     =   6360
   BeginProperty Font 
      name            =   "Arial"
      charset         =   0
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   Height          =   2955
   HelpContextID   =   124
   Left            =   2430
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   2595
   ScaleWidth      =   6360
   ShowInTaskbar   =   0   'False
   Top             =   4470
   Width           =   6480
   Begin VB.CommandButton Help 
      Caption         =   "&Help"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   400
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   5460
      TabIndex        =   27
      Top             =   600
      Width           =   855
   End
   Begin VB.CommandButton ResetAll 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Reset All"
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
      Left            =   5460
      TabIndex        =   26
      Top             =   120
      Width           =   855
   End
   Begin VB.CommandButton Close 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Cancel          =   -1  'True
      Caption         =   "&Close"
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
      Left            =   5460
      TabIndex        =   28
      Top             =   2160
      Width           =   855
   End
   Begin TabDlg.SSTab PropsTab 
      Height          =   2415
      Left            =   120
      TabIndex        =   4
      Top             =   120
      Width           =   5295
      _Version        =   65536
      _ExtentX        =   9340
      _ExtentY        =   4260
      _StockProps     =   15
      Caption         =   "Rotate "
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      TabsPerRow      =   6
      Tab             =   1
      TabOrientation  =   0
      Tabs            =   6
      Style           =   1
      TabMaxWidth     =   0
      TabHeight       =   494
      ShowFocusRect   =   0   'False
      TabCaption(0)   =   "Effects"
      Tab(0).ControlCount=   1
      Tab(0).ControlEnabled=   0   'False
      Tab(0).Control(0)=   "PolyFlagsHolder"
      TabCaption(1)   =   "Rotate "
      Tab(1).ControlCount=   1
      Tab(1).ControlEnabled=   -1  'True
      Tab(1).Control(0)=   "Frame6"
      TabCaption(2)   =   "Pan "
      Tab(2).ControlCount=   3
      Tab(2).ControlEnabled=   0   'False
      Tab(2).Control(0)=   "Frame4"
      Tab(2).Control(1)=   "Frame3"
      Tab(2).Control(2)=   "PanReset"
      TabCaption(3)   =   "Align "
      Tab(3).ControlCount=   13
      Tab(3).ControlEnabled=   0   'False
      Tab(3).Control(0)=   "WallPan"
      Tab(3).Control(1)=   "OneTile"
      Tab(3).Control(2)=   "AlignGrade"
      Tab(3).Control(3)=   "WallColumn"
      Tab(3).Control(4)=   "Unalign"
      Tab(3).Control(5)=   "AlignWall"
      Tab(3).Control(6)=   "ColumnTexels"
      Tab(3).Control(7)=   "Label9"
      Tab(3).Control(8)=   "Label5"
      Tab(3).Control(9)=   "Label6"
      Tab(3).Control(10)=   "Label7"
      Tab(3).Control(11)=   "Label8"
      Tab(3).Control(12)=   "Label3"
      TabCaption(4)   =   "Scale"
      Tab(4).ControlCount=   2
      Tab(4).ControlEnabled=   0   'False
      Tab(4).Control(0)=   "Frame5"
      Tab(4).Control(1)=   "Frame2"
      TabCaption(5)   =   "Editor"
      Tab(5).ControlCount=   3
      Tab(5).ControlEnabled=   0   'False
      Tab(5).Control(0)=   "Frame8"
      Tab(5).Control(1)=   "Frame7"
      Tab(5).Control(2)=   "Label2"
      Begin VB.PictureBox PolyFlagsHolder 
         BorderStyle     =   0  'None
         Height          =   1995
         Left            =   -74940
         ScaleHeight     =   1995
         ScaleWidth      =   5175
         TabIndex        =   52
         Top             =   360
         Width           =   5175
      End
      Begin VB.Frame Frame2 
         Caption         =   "Simple Scaling"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   1215
         Left            =   -74760
         TabIndex        =   49
         Top             =   600
         Width           =   1335
         Begin VB.ComboBox ScaleList 
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
            Height          =   315
            Left            =   120
            TabIndex        =   51
            Text            =   "1.0"
            Top             =   360
            Width           =   1095
         End
         Begin VB.CommandButton Command8 
            Caption         =   "&Apply"
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
            TabIndex        =   50
            Top             =   720
            Width           =   1095
         End
      End
      Begin VB.Frame Frame5 
         Caption         =   "Custom Scaling"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   1215
         Left            =   -73200
         TabIndex        =   42
         Top             =   600
         Width           =   3255
         Begin VB.TextBox U 
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   315
            Left            =   1080
            TabIndex        =   46
            Text            =   "1.0"
            Top             =   360
            Width           =   1095
         End
         Begin VB.TextBox V 
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   315
            Left            =   1080
            TabIndex        =   45
            Text            =   "1.0"
            Top             =   720
            Width           =   1095
         End
         Begin VB.CommandButton Command5 
            Caption         =   "&Apply"
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
            Left            =   2280
            TabIndex        =   44
            Top             =   360
            Width           =   735
         End
         Begin VB.CommandButton Command7 
            Caption         =   "&Reset"
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
            Left            =   2280
            TabIndex        =   43
            Top             =   720
            Width           =   735
         End
         Begin VB.Label Label1 
            Alignment       =   1  'Right Justify
            Caption         =   "Texture U:"
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
            TabIndex        =   48
            Top             =   360
            Width           =   855
         End
         Begin VB.Label Label4 
            Alignment       =   1  'Right Justify
            Caption         =   "Texture V:"
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
            TabIndex        =   47
            Top             =   720
            Width           =   855
         End
      End
      Begin VB.CommandButton WallPan 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Wall Pan"
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
         Left            =   -74520
         TabIndex        =   40
         Top             =   1200
         Width           =   1215
      End
      Begin VB.CommandButton OneTile 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "One Tile"
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
         Left            =   -74520
         TabIndex        =   29
         Top             =   600
         Width           =   1215
      End
      Begin VB.CommandButton AlignGrade 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Floor/Ceiling"
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
         Left            =   -74520
         TabIndex        =   31
         Top             =   375
         Width           =   1215
      End
      Begin VB.CommandButton WallColumn 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Wall Column"
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
         Left            =   -74520
         TabIndex        =   37
         Top             =   1440
         Width           =   1215
      End
      Begin VB.CommandButton Unalign 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Unalign"
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
         Left            =   -74520
         TabIndex        =   32
         Top             =   1800
         Width           =   1215
      End
      Begin VB.CommandButton AlignWall 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Wall Direction"
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
         Left            =   -74520
         TabIndex        =   30
         Top             =   960
         Width           =   1215
      End
      Begin VB.TextBox ColumnTexels 
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   285
         Left            =   -71235
         TabIndex        =   39
         Text            =   "256"
         Top             =   1410
         Width           =   930
      End
      Begin VB.Frame Frame8 
         Caption         =   "Selected Polys"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Left            =   -71640
         TabIndex        =   19
         Top             =   480
         Width           =   1695
         Begin VB.CommandButton SetGrpName 
            Caption         =   "Set Item Name"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   300
            Left            =   120
            TabIndex        =   24
            Top             =   240
            Width           =   1455
         End
         Begin VB.TextBox ItemName 
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   315
            Left            =   120
            TabIndex        =   25
            Top             =   600
            Width           =   1455
         End
      End
      Begin VB.Frame Frame7 
         Caption         =   "Cutaway Zones"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Left            =   -74520
         TabIndex        =   18
         Top             =   480
         Width           =   2535
         Begin VB.CommandButton CutHideAll 
            Caption         =   "Hide All"
            BeginProperty Font 
               name            =   "Arial"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   21
            Top             =   600
            Width           =   855
         End
         Begin VB.CommandButton CutHideSel 
            Caption         =   "Hide Selected"
            BeginProperty Font 
               name            =   "Arial"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   1080
            TabIndex        =   23
            Top             =   600
            Width           =   1335
         End
         Begin VB.CommandButton CutShowAll 
            Caption         =   "Show All"
            BeginProperty Font 
               name            =   "Arial"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   20
            Top             =   360
            Width           =   855
         End
         Begin VB.CommandButton CutShowSel 
            Caption         =   "Show Selected"
            BeginProperty Font 
               name            =   "Arial"
               charset         =   0
               weight          =   400
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   1080
            TabIndex        =   22
            Top             =   360
            Width           =   1335
         End
      End
      Begin VB.Frame Frame6 
         Caption         =   "Simple Rotation"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   1095
         Left            =   600
         TabIndex        =   16
         Top             =   600
         Width           =   4095
         Begin VB.CommandButton RotMD 
            Caption         =   "Big Diagonal"
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
            Left            =   2280
            TabIndex        =   56
            Top             =   720
            Width           =   1575
         End
         Begin VB.CommandButton RotD 
            Caption         =   "Small Diagonal"
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
            Left            =   2280
            TabIndex        =   55
            Top             =   360
            Width           =   1575
         End
         Begin VB.CommandButton Rot45 
            Caption         =   "+45"
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
            Left            =   840
            TabIndex        =   54
            Top             =   720
            Width           =   615
         End
         Begin VB.CommandButton RotM45 
            Caption         =   "-45"
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
            Left            =   840
            TabIndex        =   53
            Top             =   360
            Width           =   615
         End
         Begin VB.CommandButton FlipV 
            Caption         =   "FlipV"
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
            Left            =   1560
            TabIndex        =   8
            Top             =   720
            Width           =   615
         End
         Begin VB.CommandButton FlipU 
            Caption         =   "Flip U"
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
            Left            =   1560
            TabIndex        =   7
            Top             =   360
            Width           =   615
         End
         Begin VB.CommandButton RotM90 
            Caption         =   "-90"
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
            Top             =   360
            Width           =   615
         End
         Begin VB.CommandButton Rot90 
            Caption         =   "+90"
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
            TabIndex        =   6
            Top             =   720
            Width           =   615
         End
      End
      Begin VB.Frame Frame4 
         Caption         =   "Pan Amount"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   1335
         Left            =   -72960
         TabIndex        =   15
         Top             =   480
         Width           =   1335
         Begin VB.OptionButton Pan1 
            Caption         =   "1"
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
            TabIndex        =   9
            Top             =   240
            Value           =   -1  'True
            Width           =   495
         End
         Begin VB.OptionButton Pan4 
            Caption         =   "4"
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
            TabIndex        =   10
            Top             =   480
            Width           =   495
         End
         Begin VB.OptionButton Pan16 
            Caption         =   "16"
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
            TabIndex        =   11
            Top             =   720
            Width           =   495
         End
         Begin VB.OptionButton Pan64 
            Caption         =   "64"
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
            TabIndex        =   12
            Top             =   960
            Width           =   495
         End
      End
      Begin VB.Frame Frame3 
         Caption         =   "Pan Buttons"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   1335
         Left            =   -74400
         TabIndex        =   14
         Top             =   480
         Width           =   1215
         Begin VB.CommandButton PanVM 
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   480
            TabIndex        =   3
            Top             =   360
            Width           =   255
         End
         Begin VB.CommandButton PanUM 
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   240
            TabIndex        =   2
            Top             =   600
            Width           =   255
         End
         Begin VB.CommandButton PanU 
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   720
            TabIndex        =   1
            Top             =   600
            Width           =   255
         End
         Begin VB.CommandButton PanV 
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   0
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   480
            TabIndex        =   0
            Top             =   840
            Width           =   255
         End
      End
      Begin VB.CommandButton PanReset 
         Caption         =   "Reset"
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
         Left            =   -71400
         TabIndex        =   13
         Top             =   1560
         Width           =   1095
      End
      Begin VB.Label Label9 
         Caption         =   "Align wall texture vertical panning"
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
         Left            =   -73200
         TabIndex        =   41
         Top             =   1200
         Width           =   3015
      End
      Begin VB.Label Label5 
         Caption         =   "Align wall columns. Texels:"
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
         Left            =   -73200
         TabIndex        =   38
         Top             =   1440
         Width           =   2055
      End
      Begin VB.Label Label6 
         Caption         =   "Reset to default unaligned state"
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
         Left            =   -73200
         TabIndex        =   36
         Top             =   1800
         Width           =   3015
      End
      Begin VB.Label Label7 
         Caption         =   "Align wall texture directions"
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
         Left            =   -73200
         TabIndex        =   35
         Top             =   960
         Width           =   3015
      End
      Begin VB.Label Label8 
         Caption         =   "Align floor, ceiling, and slope textures"
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
         Left            =   -73200
         TabIndex        =   34
         Top             =   375
         Width           =   3015
      End
      Begin VB.Label Label3 
         Caption         =   "Stretch texture to tile it exactly once"
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
         Left            =   -73200
         TabIndex        =   33
         Top             =   600
         Width           =   3015
      End
      Begin VB.Label Label2 
         Alignment       =   2  'Center
         Caption         =   "These options that apply only in UnrealEd, not during gameplay."
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
         Left            =   -74760
         TabIndex        =   17
         Top             =   1560
         Width           =   4935
      End
   End
End
Attribute VB_Name = "frmPolyProps"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim InPan As Integer
Dim Loading As Integer
Dim PolyFlagsForm As Form
Dim Sqr2 As String

'
' Public
'
Public Sub GetSelectedPolys()
    Dim OnFlags As Long, OffFlags As Long
    '
    Dim N As Integer
    N = Val(Ed.Server.GetProp("Polys", "NumSelected"))
    If N = 0 Then
        PropsTab.Enabled = False
    Else
        PropsTab.Enabled = True
    End If
    '
    OnFlags = Val(Ed.Server.GetProp("Polys", "SelectedSetFlags"))
    OffFlags = Val(Ed.Server.GetProp("Polys", "SelectedClearFlags"))
    '
    Call PolyFlagsForm.SetFlagBits(OnFlags, OffFlags)
    Caption = "Polygon Properties (" & Str(N) & " selected)"
End Sub

Public Sub PolyFlagsUpdate(NewOnFlags As Long, NewOffFlags As Long)
    Ed.Server.Exec "POLY SET SETFLAGS=" & Trim(Str(NewOnFlags)) & " CLEARFLAGS=" & Trim(Str(NewOffFlags))
End Sub

'
' Private
'

Private Sub AlignFloor_Click()
    Ed.Server.Exec "POLY TEXALIGN FLOOR"
End Sub

Private Sub AlignGrade_Click()
    Ed.Server.Exec "POLY TEXALIGN FLOOR"
End Sub

Private Sub AlignWall_Click()
    Ed.Server.Exec "POLY TEXALIGN WALLDIR"
End Sub

Private Sub Close_Click()
    Hide
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Unload PolyFlagsForm
    Set PolyFlagsForm = Nothing
    '
    Call Ed.EndOnTop(Me)
    GPolyPropsAction = 1
End Sub

Private Sub ResetAll_Click()
    Ed.Server.Exec "POLY TEXPAN RESET"
    Ed.Server.Exec "POLY TEXSCALE"
    Ed.Server.Exec "POLY TEXALIGN DEFAULT"
End Sub

Private Sub Help_Click()
    SendKeys "{F1}"
End Sub

Private Sub Rot45_Click()
    Ed.Server.Exec "POLY TEXMULT UU=" & Sqr2 & " VV=" & Sqr2 & _
    " UV=" & Sqr2 & " VU=-" & Sqr2
End Sub

Private Sub RotD_Click()
    Ed.Server.Exec "POLY TEXMULT UU=1 VV=1 UV=-1 VU=1"
End Sub

Private Sub RotM45_Click()
    Ed.Server.Exec "POLY TEXMULT UU=" & Sqr2 & " VV=" & Sqr2 & _
    " UV=-" & Sqr2 & " VU=" & Sqr2
End Sub

Private Sub RotMD_Click()
    Ed.Server.Exec "POLY TEXMULT UU=0.5 VV=0.5 UV=-0.5 VU=0.5"
End Sub

Private Sub WallColumn_Click()
    Dim Temp As Double
    If Not Eval(ColumnTexels, Temp) Then Exit Sub
    Ed.Server.Exec "POLY TEXALIGN WALLCOLUMN TEXELS=" & Trim(Str(Int(Temp)))
End Sub

Private Sub WallPan_Click()
    Ed.Server.Exec "POLY TEXALIGN WALLPAN"
End Sub

Private Sub Command5_Click()
    Dim UU As Double
    Dim VV As Double
    '
    UU = Val(U.Text)
    VV = Val(V.Text)
    '
    If (UU <> 0) And (VV <> 0) Then
        Ed.Server.Exec "POLY TEXSCALE UU=" & Trim(Str(1# / UU)) & " VV=" & Trim(Str(1# / V))
    End If
End Sub

Private Sub Command7_Click()
    U.Text = "1.0"
    V.Text = "1.0"
    Ed.Server.Exec "POLY TEXSCALE UU=1 VV=1"
End Sub

Private Sub Command8_Click()
    ScaleList_Click
End Sub

Private Sub CutHideAll_Click()
    Ed.Server.Exec "CUTAWAY HIDE ALL"
End Sub

Private Sub CutHideSel_Click()
   Ed.Server.Exec "CUTAWAY HIDE SELECTED"
End Sub

Private Sub CutShowAll_Click()
    Ed.Server.Exec "CUTAWAY SHOW ALL"
End Sub

Private Sub CutShowSel_Click()
   Ed.Server.Exec "CUTAWAY SHOW SELECTED"
End Sub

Private Sub FlipU_Click()
    Ed.Server.Exec "POLY TEXMULT UU=-1 VV=1"
End Sub

Private Sub FlipV_Click()
    Ed.Server.Exec "POLY TEXMULT UU=1 VV=-1"
End Sub

Private Sub Form_Load()
    Loading = 1
    GPolyPropsAction = 1
    Call Ed.SetOnTop(Me, "PolygonProperties", TOP_PANEL)
    Sqr2 = Trim(Str(1# / Sqr(2)))
    '
    ' Add values to scale list
    '
    ScaleList.AddItem "0.0625"
    ScaleList.AddItem "0.125"
    ScaleList.AddItem "0.25"
    ScaleList.AddItem "0.5"
    ScaleList.AddItem "1.0"
    ScaleList.AddItem "2.0"
    ScaleList.AddItem "4.0"
    ScaleList.AddItem "8.0"
    ScaleList.AddItem "16.0"
    '
    Set PolyFlagsForm = New frmPolyFlags
    Call PolyFlagsForm.SetFormParent(Me, PolyFlagsHolder)
    PolyFlagsForm.Show
    '
    Loading = 0
    '
    Call GetSelectedPolys
End Sub

Private Sub OneTile_Click()
    Ed.Server.Exec "POLY TEXALIGN ONETILE"
End Sub

Private Sub PanReset_Click()
    Ed.Server.Exec "POLY TEXPAN RESET"
End Sub

Private Function PanStr() As String
    If (Pan1.Value) Then PanStr = "1"
    If (Pan4.Value) Then PanStr = "4"
    If (Pan16.Value) Then PanStr = "16"
    If (Pan64.Value) Then PanStr = "64"
End Function

Private Sub PanU_Click()
    Ed.Server.Exec "POLY TEXPAN U=" & PanStr()
End Sub

Private Sub PanUM_Click()
    Ed.Server.Exec "POLY TEXPAN U=-" & PanStr()
End Sub

Private Sub PanV_Click()
    Ed.Server.Exec "POLY TEXPAN V=" & PanStr()
End Sub

Private Sub PanVM_Click()
    Ed.Server.Exec "POLY TEXPAN V=-" & PanStr()
End Sub

Private Sub Rot90_Click()
    Ed.Server.Exec "POLY TEXMULT UU=0 VV=0 UV=1 VU=-1"
End Sub

Private Sub RotM90_Click()
    Ed.Server.Exec "POLY TEXMULT UU=0 VV=0 UV=-1 VU=1"
End Sub

Private Sub ScaleList_Click()
    Dim V As Double
    If Not Loading Then
        If Eval(ScaleList.Text, V) Then
            If V <> 0 Then
                V = 1# / V
                Ed.Server.Exec "POLY TEXSCALE UU=" & Trim(Str(V)) & " VV=" & Trim(Str(V))
            End If
        End If
    End If
End Sub

Private Sub SetGrpName_Click()
    Ed.Server.Exec "POLY SET ITEM=" & ItemName.Text
End Sub

Private Sub Unalign_Click()
    Ed.Server.Exec "POLY TEXALIGN DEFAULT"
End Sub
