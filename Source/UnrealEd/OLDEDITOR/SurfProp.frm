VERSION 5.00
Object = "{BDC217C8-ED16-11CD-956C-0000C04E4C0A}#1.1#0"; "TABCTL32.OCX"
Begin VB.Form frmSurfaceProps 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Surface Properties"
   ClientHeight    =   2595
   ClientLeft      =   3840
   ClientTop       =   8865
   ClientWidth     =   6360
   BeginProperty Font 
      Name            =   "Arial"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   HelpContextID   =   124
   Icon            =   "SurfProp.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   2595
   ScaleWidth      =   6360
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton Help 
      Caption         =   "&Help"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   5460
      TabIndex        =   23
      Top             =   600
      Width           =   855
   End
   Begin VB.CommandButton ResetAll 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Reset All"
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
      Left            =   5460
      TabIndex        =   22
      Top             =   120
      Width           =   855
   End
   Begin VB.CommandButton Close 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Cancel          =   -1  'True
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
      Left            =   5460
      TabIndex        =   24
      Top             =   2160
      Width           =   855
   End
   Begin TabDlg.SSTab PropsTab 
      Height          =   2415
      Left            =   60
      TabIndex        =   4
      Top             =   120
      Width           =   5295
      _ExtentX        =   9340
      _ExtentY        =   4260
      _Version        =   327681
      Style           =   1
      Tabs            =   6
      TabsPerRow      =   6
      TabHeight       =   494
      ShowFocusRect   =   0   'False
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      TabCaption(0)   =   "Effects"
      TabPicture(0)   =   "SurfProp.frx":030A
      Tab(0).ControlEnabled=   -1  'True
      Tab(0).Control(0)=   "PolyFlagsHolder"
      Tab(0).Control(0).Enabled=   0   'False
      Tab(0).ControlCount=   1
      TabCaption(1)   =   "Rotate "
      TabPicture(1)   =   "SurfProp.frx":0326
      Tab(1).ControlEnabled=   0   'False
      Tab(1).Control(0)=   "Frame6"
      Tab(1).ControlCount=   1
      TabCaption(2)   =   "Pan "
      TabPicture(2)   =   "SurfProp.frx":0342
      Tab(2).ControlEnabled=   0   'False
      Tab(2).Control(0)=   "PanReset"
      Tab(2).Control(1)=   "Frame3"
      Tab(2).Control(2)=   "Frame4"
      Tab(2).ControlCount=   3
      TabCaption(3)   =   "Align "
      TabPicture(3)   =   "SurfProp.frx":035E
      Tab(3).ControlEnabled=   0   'False
      Tab(3).Control(0)=   "Label3"
      Tab(3).Control(1)=   "Label8"
      Tab(3).Control(2)=   "Label7"
      Tab(3).Control(3)=   "Label6"
      Tab(3).Control(4)=   "Label5"
      Tab(3).Control(5)=   "Label9"
      Tab(3).Control(6)=   "ColumnTexels"
      Tab(3).Control(7)=   "AlignWall"
      Tab(3).Control(8)=   "Unalign"
      Tab(3).Control(9)=   "WallColumn"
      Tab(3).Control(10)=   "AlignGrade"
      Tab(3).Control(11)=   "OneTile"
      Tab(3).Control(12)=   "WallPan"
      Tab(3).ControlCount=   13
      TabCaption(4)   =   "Scale"
      TabPicture(4)   =   "SurfProp.frx":037A
      Tab(4).ControlEnabled=   0   'False
      Tab(4).Control(0)=   "Frame5"
      Tab(4).Control(1)=   "Frame2"
      Tab(4).ControlCount=   2
      TabCaption(5)   =   "Editor"
      TabPicture(5)   =   "SurfProp.frx":0396
      Tab(5).ControlEnabled=   0   'False
      Tab(5).Control(0)=   "Frame7"
      Tab(5).Control(1)=   "Frame1"
      Tab(5).ControlCount=   2
      Begin VB.Frame Frame1 
         Caption         =   "Surface Stats"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   795
         Left            =   -74880
         TabIndex        =   53
         Top             =   1500
         Width           =   4995
         Begin VB.Label Label10 
            Alignment       =   1  'Right Justify
            Caption         =   "Mesh size:"
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
            Index           =   2
            Left            =   120
            TabIndex        =   61
            Top             =   480
            Width           =   915
         End
         Begin VB.Label MeshSize 
            Caption         =   "###x###"
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
            Left            =   1140
            TabIndex        =   60
            Top             =   480
            Width           =   735
         End
         Begin VB.Label Label10 
            Alignment       =   1  'Right Justify
            Caption         =   "Meshels:"
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
            Index           =   1
            Left            =   3180
            TabIndex        =   59
            Top             =   240
            Width           =   1035
         End
         Begin VB.Label SurfCache 
            Caption         =   "###"
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
            Left            =   4320
            TabIndex        =   58
            Top             =   240
            Width           =   615
         End
         Begin VB.Label Label12 
            Alignment       =   1  'Right Justify
            Caption         =   "Dynamic lights: "
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
            Left            =   1560
            TabIndex        =   57
            Top             =   240
            Width           =   1215
         End
         Begin VB.Label DynamicLights 
            Caption         =   "###"
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
            Left            =   2820
            TabIndex        =   56
            Top             =   240
            Width           =   435
         End
         Begin VB.Label StaticLights 
            Caption         =   "###"
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
            Left            =   1140
            TabIndex        =   55
            Top             =   240
            Width           =   435
         End
         Begin VB.Label Label10 
            Alignment       =   1  'Right Justify
            Caption         =   "Static lights: "
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
            Index           =   0
            Left            =   60
            TabIndex        =   54
            Top             =   240
            Width           =   1035
         End
      End
      Begin VB.PictureBox PolyFlagsHolder 
         BorderStyle     =   0  'None
         Height          =   1995
         Left            =   60
         ScaleHeight     =   1995
         ScaleWidth      =   5175
         TabIndex        =   48
         Top             =   360
         Width           =   5175
      End
      Begin VB.Frame Frame2 
         Caption         =   "Simple Scaling"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1215
         Left            =   -74760
         TabIndex        =   45
         Top             =   600
         Width           =   1335
         Begin VB.ComboBox ScaleList 
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
            Height          =   315
            Left            =   120
            TabIndex        =   47
            Text            =   "1.0"
            Top             =   360
            Width           =   1095
         End
         Begin VB.CommandButton Command8 
            Caption         =   "&Apply"
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
            TabIndex        =   46
            Top             =   720
            Width           =   1095
         End
      End
      Begin VB.Frame Frame5 
         Caption         =   "Custom Scaling"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1215
         Left            =   -73200
         TabIndex        =   38
         Top             =   600
         Width           =   3255
         Begin VB.TextBox U 
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   315
            Left            =   1080
            TabIndex        =   42
            Text            =   "1.0"
            Top             =   360
            Width           =   1095
         End
         Begin VB.TextBox V 
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   315
            Left            =   1080
            TabIndex        =   41
            Text            =   "1.0"
            Top             =   720
            Width           =   1095
         End
         Begin VB.CommandButton Command5 
            Caption         =   "&Apply"
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
            Left            =   2280
            TabIndex        =   40
            Top             =   360
            Width           =   735
         End
         Begin VB.CommandButton Command7 
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
            Left            =   2280
            TabIndex        =   39
            Top             =   720
            Width           =   735
         End
         Begin VB.Label Label1 
            Alignment       =   1  'Right Justify
            Caption         =   "Texture U:"
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
            TabIndex        =   44
            Top             =   360
            Width           =   855
         End
         Begin VB.Label Label4 
            Alignment       =   1  'Right Justify
            Caption         =   "Texture V:"
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
            TabIndex        =   43
            Top             =   720
            Width           =   855
         End
      End
      Begin VB.CommandButton WallPan 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Wall Pan"
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
         Left            =   -74520
         TabIndex        =   36
         Top             =   1200
         Width           =   1215
      End
      Begin VB.CommandButton OneTile 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "One Tile"
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
         Left            =   -74520
         TabIndex        =   25
         Top             =   600
         Width           =   1215
      End
      Begin VB.CommandButton AlignGrade 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
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
         Left            =   -74520
         TabIndex        =   27
         Top             =   375
         Width           =   1215
      End
      Begin VB.CommandButton WallColumn 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Wall Column"
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
         Left            =   -74520
         TabIndex        =   33
         Top             =   1440
         Width           =   1215
      End
      Begin VB.CommandButton Unalign 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Unalign"
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
         Left            =   -74520
         TabIndex        =   28
         Top             =   1800
         Width           =   1215
      End
      Begin VB.CommandButton AlignWall 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Wall Direction"
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
         Left            =   -74520
         TabIndex        =   26
         Top             =   960
         Width           =   1215
      End
      Begin VB.TextBox ColumnTexels 
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
         Left            =   -71235
         TabIndex        =   35
         Text            =   "256"
         Top             =   1410
         Width           =   930
      End
      Begin VB.Frame Frame7 
         Caption         =   "Cutaway Zones"
         Enabled         =   0   'False
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
         Left            =   -74880
         TabIndex        =   17
         Top             =   420
         Width           =   2535
         Begin VB.CommandButton CutHideAll 
            Caption         =   "Hide All"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   19
            Top             =   600
            Width           =   855
         End
         Begin VB.CommandButton CutHideSel 
            Caption         =   "Hide Selected"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   1080
            TabIndex        =   21
            Top             =   600
            Width           =   1335
         End
         Begin VB.CommandButton CutShowAll 
            Caption         =   "Show All"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   18
            Top             =   360
            Width           =   855
         End
         Begin VB.CommandButton CutShowSel 
            Caption         =   "Show Selected"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   1080
            TabIndex        =   20
            Top             =   360
            Width           =   1335
         End
      End
      Begin VB.Frame Frame6 
         Caption         =   "Simple Rotation"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1095
         Left            =   -74400
         TabIndex        =   16
         Top             =   600
         Width           =   4095
         Begin VB.CommandButton RotMD 
            Caption         =   "Big Diagonal"
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
            Left            =   2280
            TabIndex        =   52
            Top             =   720
            Width           =   1575
         End
         Begin VB.CommandButton RotD 
            Caption         =   "Small Diagonal"
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
            Left            =   2280
            TabIndex        =   51
            Top             =   360
            Width           =   1575
         End
         Begin VB.CommandButton Rot45 
            Caption         =   "+45"
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
            TabIndex        =   50
            Top             =   720
            Width           =   615
         End
         Begin VB.CommandButton RotM45 
            Caption         =   "-45"
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
            Top             =   360
            Width           =   615
         End
         Begin VB.CommandButton FlipV 
            Caption         =   "FlipV"
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
            Left            =   1560
            TabIndex        =   8
            Top             =   720
            Width           =   615
         End
         Begin VB.CommandButton FlipU 
            Caption         =   "Flip U"
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
            Left            =   1560
            TabIndex        =   7
            Top             =   360
            Width           =   615
         End
         Begin VB.CommandButton RotM90 
            Caption         =   "-90"
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
            TabIndex        =   5
            Top             =   360
            Width           =   615
         End
         Begin VB.CommandButton Rot90 
            Caption         =   "+90"
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
            TabIndex        =   6
            Top             =   720
            Width           =   615
         End
      End
      Begin VB.Frame Frame4 
         Caption         =   "Pan Amount"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1335
         Left            =   -72960
         TabIndex        =   15
         Top             =   480
         Width           =   1335
         Begin VB.OptionButton Pan1 
            Caption         =   "1"
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
            Top             =   240
            Value           =   -1  'True
            Width           =   495
         End
         Begin VB.OptionButton Pan4 
            Caption         =   "4"
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
            TabIndex        =   10
            Top             =   480
            Width           =   495
         End
         Begin VB.OptionButton Pan16 
            Caption         =   "16"
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
            TabIndex        =   11
            Top             =   720
            Width           =   495
         End
         Begin VB.OptionButton Pan64 
            Caption         =   "64"
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
            Top             =   960
            Width           =   495
         End
      End
      Begin VB.Frame Frame3 
         Caption         =   "Pan Buttons"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1335
         Left            =   -74400
         TabIndex        =   14
         Top             =   480
         Width           =   1215
         Begin VB.CommandButton PanVM 
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   480
            TabIndex        =   3
            Top             =   360
            Width           =   255
         End
         Begin VB.CommandButton PanUM 
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   240
            TabIndex        =   2
            Top             =   600
            Width           =   255
         End
         Begin VB.CommandButton PanU 
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   720
            TabIndex        =   1
            Top             =   600
            Width           =   255
         End
         Begin VB.CommandButton PanV 
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
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
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
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
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   -73200
         TabIndex        =   37
         Top             =   1200
         Width           =   3015
      End
      Begin VB.Label Label5 
         Caption         =   "Align wall columns. Texels:"
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
         Left            =   -73200
         TabIndex        =   34
         Top             =   1440
         Width           =   2055
      End
      Begin VB.Label Label6 
         Caption         =   "Reset to default unaligned state"
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
         Left            =   -73200
         TabIndex        =   32
         Top             =   1800
         Width           =   3015
      End
      Begin VB.Label Label7 
         Caption         =   "Align wall texture directions"
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
         Left            =   -73200
         TabIndex        =   31
         Top             =   960
         Width           =   3015
      End
      Begin VB.Label Label8 
         Caption         =   "Align floor, ceiling, and slope textures"
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
         Left            =   -73200
         TabIndex        =   30
         Top             =   375
         Width           =   3015
      End
      Begin VB.Label Label3 
         Caption         =   "Stretch texture to tile it exactly once"
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
         Left            =   -73200
         TabIndex        =   29
         Top             =   600
         Width           =   3015
      End
   End
End
Attribute VB_Name = "frmSurfaceProps"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
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
    Dim N As Integer, L As Integer
    
    OnFlags = Val(Ed.ServerGetProp("Polys", "SelectedSetFlags"))
    OffFlags = Val(Ed.ServerGetProp("Polys", "SelectedClearFlags"))
    N = Val(Ed.ServerGetProp("Polys", "NumSelected"))
    
    Call PolyFlagsForm.SetFlagBits(OnFlags, OffFlags)
    Caption = "Surface properties (" & Trim(Str(N)) & " selected)"
    
    StaticLights.Caption = Ed.ServerGetProp("Polys", "StaticLights")
    DynamicLights.Caption = Ed.ServerGetProp("Polys", "DynamicLights")
    SurfCache.Caption = Ed.ServerGetProp("Polys", "Meshels")
    MeshSize.Caption = Ed.ServerGetProp("Polys", "MeshSize")

    N = Val(Ed.ServerGetProp("Polys", "NumSelected"))
    If N = 0 Then
        PropsTab.Enabled = False
    Else
        PropsTab.Visible = False
        PropsTab.Enabled = True
        PropsTab.Visible = True
    End If
End Sub

Public Sub PolyFlagsUpdate(NewOnFlags As Long, NewOffFlags As Long)
    Ed.ServerExec "POLY SET SETFLAGS=" & Trim(Str(NewOnFlags)) & " CLEARFLAGS=" & Trim(Str(NewOffFlags))
End Sub

'
' Private
'

Private Sub AlignFloor_Click()
    Ed.ServerExec "POLY TEXALIGN FLOOR"
End Sub

Private Sub AlignGrade_Click()
    Ed.ServerExec "POLY TEXALIGN FLOOR"
End Sub

Private Sub AlignWall_Click()
    Ed.ServerExec "POLY TEXALIGN WALLDIR"
End Sub

Private Sub Close_Click()
    Hide
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Unload PolyFlagsForm
    Set PolyFlagsForm = Nothing
    '
    Call Ed.EndOnTop(Me)
    GPolyPropsAction = 0
End Sub

Private Sub ResetAll_Click()
    Ed.ServerExec "POLY TEXPAN RESET"
    Ed.ServerExec "POLY TEXSCALE"
    Ed.ServerExec "POLY TEXALIGN DEFAULT"
End Sub

Private Sub Help_Click()
    SendKeys "{F1}"
End Sub

Private Sub Rot45_Click()
    Ed.ServerExec "POLY TEXMULT UU=" & Sqr2 & " VV=" & Sqr2 & _
    " UV=" & Sqr2 & " VU=-" & Sqr2
End Sub

Private Sub RotD_Click()
    Ed.ServerExec "POLY TEXMULT UU=1 VV=1 UV=-1 VU=1"
End Sub

Private Sub RotM45_Click()
    Ed.ServerExec "POLY TEXMULT UU=" & Sqr2 & " VV=" & Sqr2 & _
    " UV=-" & Sqr2 & " VU=" & Sqr2
End Sub

Private Sub RotMD_Click()
    Ed.ServerExec "POLY TEXMULT UU=0.5 VV=0.5 UV=-0.5 VU=0.5"
End Sub

Private Sub WallColumn_Click()
    Dim Temp As Double
    If Not Eval(ColumnTexels, Temp) Then Exit Sub
    Ed.ServerExec "POLY TEXALIGN WALLCOLUMN TEXELS=" & Trim(Str(Int(Temp)))
End Sub

Private Sub WallPan_Click()
    Ed.ServerExec "POLY TEXALIGN WALLPAN"
End Sub

Private Sub Command5_Click()
    Dim UU As Double
    Dim VV As Double
    '
    If Eval(U.Text, UU) Then
        If Eval(V.Text, VV) Then
            If (UU <> 0) And (VV <> 0) Then
                Ed.ServerExec "POLY TEXSCALE UU=" & Trim(Str(1# / UU)) & " VV=" & Trim(Str(1# / V))
            End If
        End If
    End If
End Sub

Private Sub Command7_Click()
    U.Text = "1.0"
    V.Text = "1.0"
    Ed.ServerExec "POLY TEXSCALE UU=1 VV=1"
End Sub

Private Sub Command8_Click()
    ScaleList_Click
End Sub

Private Sub CutHideAll_Click()
    Ed.ServerExec "CUTAWAY HIDE ALL"
End Sub

Private Sub CutHideSel_Click()
   Ed.ServerExec "CUTAWAY HIDE SELECTED"
End Sub

Private Sub CutShowAll_Click()
    Ed.ServerExec "CUTAWAY SHOW ALL"
End Sub

Private Sub CutShowSel_Click()
   Ed.ServerExec "CUTAWAY SHOW SELECTED"
End Sub

Private Sub FlipU_Click()
    Ed.ServerExec "POLY TEXMULT UU=-1 VV=1"
End Sub

Private Sub FlipV_Click()
    Ed.ServerExec "POLY TEXMULT UU=1 VV=-1"
End Sub

Private Sub Form_Load()
    Loading = 1
    GPolyPropsAction = 1
    Call Ed.SetOnTop(Me, "SurfaceProperties", TOP_PANEL)
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
    Ed.ServerExec "POLY TEXALIGN ONETILE"
End Sub

Private Sub PanReset_Click()
    Ed.ServerExec "POLY TEXPAN RESET"
End Sub

Private Function PanStr() As String
    If (Pan1.Value) Then PanStr = "1"
    If (Pan4.Value) Then PanStr = "4"
    If (Pan16.Value) Then PanStr = "16"
    If (Pan64.Value) Then PanStr = "64"
End Function

Private Sub PanU_Click()
    Ed.ServerExec "POLY TEXPAN U=" & PanStr()
End Sub

Private Sub PanUM_Click()
    Ed.ServerExec "POLY TEXPAN U=-" & PanStr()
End Sub

Private Sub PanV_Click()
    Ed.ServerExec "POLY TEXPAN V=" & PanStr()
End Sub

Private Sub PanVM_Click()
    Ed.ServerExec "POLY TEXPAN V=-" & PanStr()
End Sub

Private Sub Rot90_Click()
    Ed.ServerExec "POLY TEXMULT UU=0 VV=0 UV=1 VU=-1"
End Sub

Private Sub RotM90_Click()
    Ed.ServerExec "POLY TEXMULT UU=0 VV=0 UV=-1 VU=1"
End Sub

Private Sub ScaleList_Click()
    Dim V As Double
    If Not Loading Then
        If Eval(ScaleList.Text, V) Then
            If V <> 0 Then
                V = 1# / V
                Ed.ServerExec "POLY TEXSCALE UU=" & Trim(Str(V)) & " VV=" & Trim(Str(V))
            End If
        End If
    End If
End Sub

Private Sub Unalign_Click()
    Ed.ServerExec "POLY TEXALIGN DEFAULT"
End Sub
