VERSION 5.00
Object = "{BDC217C8-ED16-11CD-956C-0000C04E4C0A}#1.1#0"; "TABCTL32.OCX"
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.2#0"; "COMCTL32.OCX"
Begin VB.Form frmRebuilder 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Rebuilder"
   ClientHeight    =   5145
   ClientLeft      =   8835
   ClientTop       =   3345
   ClientWidth     =   3855
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H00808080&
   HelpContextID   =   118
   Icon            =   "Rebuild.frx":0000
   LinkTopic       =   "Form4"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5145
   ScaleWidth      =   3855
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton Command2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
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
      Height          =   255
      Left            =   2820
      TabIndex        =   82
      Top             =   4860
      Width           =   975
   End
   Begin VB.CommandButton UpdStats 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Stats"
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
      TabIndex        =   1
      Top             =   4860
      Width           =   855
   End
   Begin VB.CommandButton Go 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Rebuild Geometry"
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
      Left            =   60
      TabIndex        =   0
      Top             =   4860
      Width           =   1815
   End
   Begin TabDlg.SSTab Tab1 
      Height          =   4815
      Left            =   60
      TabIndex        =   2
      Top             =   0
      Width           =   3735
      _ExtentX        =   6588
      _ExtentY        =   8493
      _Version        =   327681
      Style           =   1
      TabHeight       =   564
      ShowFocusRect   =   0   'False
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      TabCaption(0)   =   "Geometry  "
      TabPicture(0)   =   "Rebuild.frx":030A
      Tab(0).ControlEnabled=   -1  'True
      Tab(0).Control(0)=   "AutoBSP"
      Tab(0).Control(0).Enabled=   0   'False
      Tab(0).Control(1)=   "Frame8"
      Tab(0).Control(1).Enabled=   0   'False
      Tab(0).ControlCount=   2
      TabCaption(1)   =   "BSP  "
      TabPicture(1)   =   "Rebuild.frx":0326
      Tab(1).ControlEnabled=   0   'False
      Tab(1).Control(0)=   "Frame4"
      Tab(1).Control(0).Enabled=   0   'False
      Tab(1).Control(1)=   "Frame1"
      Tab(1).Control(1).Enabled=   0   'False
      Tab(1).Control(2)=   "Frame3"
      Tab(1).Control(2).Enabled=   0   'False
      Tab(1).Control(3)=   "AutoLights"
      Tab(1).Control(3).Enabled=   0   'False
      Tab(1).ControlCount=   4
      TabCaption(2)   =   "Lighting  "
      TabPicture(2)   =   "Rebuild.frx":0342
      Tab(2).ControlEnabled=   0   'False
      Tab(2).Control(0)=   "Command1"
      Tab(2).Control(0).Enabled=   0   'False
      Tab(2).Control(1)=   "Frame5"
      Tab(2).Control(1).Enabled=   0   'False
      Tab(2).Control(2)=   "Frame6"
      Tab(2).Control(2).Enabled=   0   'False
      Tab(2).ControlCount=   3
      Begin VB.CommandButton Command1 
         Caption         =   "Paths Define"
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
         Left            =   -73920
         TabIndex        =   83
         Top             =   4140
         Width           =   1395
      End
      Begin VB.CheckBox AutoLights 
         Caption         =   "&Auto Lighting"
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
         Left            =   -74880
         TabIndex        =   70
         Top             =   4440
         Value           =   1  'Checked
         Width           =   1335
      End
      Begin VB.Frame Frame3 
         Caption         =   "BSP Rebuild options"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   -1  'True
            Strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Left            =   -74880
         TabIndex        =   64
         Top             =   3420
         Width           =   3495
         Begin VB.CheckBox BuildZones 
            Caption         =   "Build Visibility Zones"
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
            Left            =   180
            TabIndex        =   71
            Top             =   600
            Value           =   1  'Checked
            Width           =   1875
         End
         Begin VB.CheckBox OptGeom 
            Caption         =   "&Optimize Geometry"
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
            Left            =   180
            TabIndex        =   65
            Top             =   360
            Value           =   1  'Checked
            Width           =   1815
         End
      End
      Begin VB.Frame Frame1 
         Caption         =   "Optimization"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   -1  'True
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1215
         Left            =   -74880
         TabIndex        =   60
         Top             =   2160
         Width           =   3495
         Begin VB.OptionButton Lame 
            Caption         =   "&Lame"
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
            TabIndex        =   63
            Top             =   360
            Width           =   855
         End
         Begin VB.OptionButton Good 
            Caption         =   "&Good"
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
            TabIndex        =   62
            Top             =   360
            Value           =   -1  'True
            Width           =   975
         End
         Begin VB.OptionButton Optimal 
            Caption         =   "&Optimal"
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
            Left            =   2160
            TabIndex        =   61
            Top             =   360
            Width           =   975
         End
         Begin ComctlLib.Slider Balance 
            Height          =   255
            Left            =   120
            TabIndex        =   69
            Top             =   600
            Width           =   3255
            _ExtentX        =   5741
            _ExtentY        =   450
            _Version        =   327682
            Max             =   100
            SelStart        =   15
            TickFrequency   =   5
            Value           =   15
         End
         Begin VB.Label BalanceValue 
            Alignment       =   2  'Center
            Caption         =   "15"
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
            TabIndex        =   68
            Top             =   900
            Width           =   495
         End
         Begin VB.Label Label2 
            Alignment       =   1  'Right Justify
            Caption         =   "Balance Tree"
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
            TabIndex        =   67
            Top             =   900
            Width           =   1215
         End
         Begin VB.Label Label1 
            Caption         =   "Minimize cuts"
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
            TabIndex        =   66
            Top             =   900
            Width           =   1215
         End
      End
      Begin VB.Frame Frame4 
         Caption         =   "BSP Stats"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   -1  'True
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1695
         Left            =   -74880
         TabIndex        =   34
         Top             =   420
         Width           =   3495
         Begin VB.Label Label3 
            Alignment       =   1  'Right Justify
            Caption         =   "Polys"
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
            TabIndex        =   59
            Top             =   345
            Width           =   615
         End
         Begin VB.Label Label4 
            Alignment       =   1  'Right Justify
            Caption         =   "Nodes"
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
            TabIndex        =   58
            Top             =   585
            Width           =   615
         End
         Begin VB.Label Label5 
            Alignment       =   1  'Right Justify
            Caption         =   "Ratio"
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
            TabIndex        =   57
            Top             =   840
            Width           =   615
         End
         Begin VB.Label Label6 
            Alignment       =   1  'Right Justify
            Caption         =   "Leaves"
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
            TabIndex        =   56
            Top             =   1320
            Width           =   615
         End
         Begin VB.Label Label7 
            Alignment       =   1  'Right Justify
            Caption         =   "Branches"
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
            TabIndex        =   55
            Top             =   360
            Width           =   975
         End
         Begin VB.Label Label8 
            Alignment       =   1  'Right Justify
            Caption         =   "Fronts"
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
            TabIndex        =   54
            Top             =   840
            Width           =   735
         End
         Begin VB.Label Label9 
            Alignment       =   1  'Right Justify
            Caption         =   "Backs"
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
            TabIndex        =   53
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label Label10 
            Alignment       =   1  'Right Justify
            Caption         =   "Coplanars"
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
            TabIndex        =   52
            Top             =   600
            Width           =   735
         End
         Begin VB.Label Label12 
            Alignment       =   1  'Right Justify
            Caption         =   "Avg Dep"
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
            TabIndex        =   51
            Top             =   1320
            Width           =   735
         End
         Begin VB.Label Label13 
            Alignment       =   1  'Right Justify
            Caption         =   "Max Dep"
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
            TabIndex        =   50
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label EdPolys 
            Caption         =   "?"
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
            Left            =   960
            TabIndex        =   49
            Top             =   360
            Width           =   495
         End
         Begin VB.Label Leaves 
            Caption         =   "?"
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
            Left            =   2400
            TabIndex        =   48
            Top             =   1320
            Width           =   495
         End
         Begin VB.Label Backs 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   47
            Top             =   1080
            Width           =   495
         End
         Begin VB.Label Fronts 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   46
            Top             =   840
            Width           =   495
         End
         Begin VB.Label Coplanars 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   45
            Top             =   600
            Width           =   495
         End
         Begin VB.Label Branches 
            Caption         =   "?"
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
            Left            =   2375
            TabIndex        =   44
            Top             =   360
            Width           =   495
         End
         Begin VB.Label AvgDepth 
            Caption         =   "?"
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
            Left            =   960
            TabIndex        =   43
            Top             =   1320
            Width           =   495
         End
         Begin VB.Label MaxDepth 
            Caption         =   "?"
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
            Left            =   960
            TabIndex        =   42
            Top             =   1080
            Width           =   495
         End
         Begin VB.Label Ratio 
            Caption         =   "?"
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
            Left            =   960
            TabIndex        =   41
            Top             =   840
            Width           =   495
         End
         Begin VB.Label Nodes 
            Caption         =   "?"
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
            Left            =   960
            TabIndex        =   40
            Top             =   600
            Width           =   495
         End
         Begin VB.Label pBranches 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   39
            Top             =   360
            Width           =   495
         End
         Begin VB.Label pLeaves 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   38
            Top             =   1320
            Width           =   495
         End
         Begin VB.Label pBacks 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   37
            Top             =   1080
            Width           =   495
         End
         Begin VB.Label pFronts 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   36
            Top             =   840
            Width           =   495
         End
         Begin VB.Label pCoplanars 
            Caption         =   "?"
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
            Left            =   2880
            TabIndex        =   35
            Top             =   600
            Width           =   495
         End
      End
      Begin VB.Frame Frame8 
         Caption         =   "Map Geometry Stats"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   -1  'True
            Strikethrough   =   0   'False
         EndProperty
         Height          =   3675
         Left            =   120
         TabIndex        =   21
         Top             =   480
         Width           =   3495
         Begin VB.Label Label28 
            Alignment       =   1  'Right Justify
            Caption         =   "Sides"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   195
            Left            =   240
            TabIndex        =   81
            Top             =   2520
            Width           =   975
         End
         Begin VB.Label MapSides 
            Caption         =   "?"
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
            TabIndex        =   80
            Top             =   2520
            Width           =   1215
         End
         Begin VB.Label MapPoints 
            Caption         =   "?"
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
            TabIndex        =   79
            Top             =   2040
            Width           =   1215
         End
         Begin VB.Label MapVectors 
            Caption         =   "?"
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
            TabIndex        =   78
            Top             =   2280
            Width           =   1215
         End
         Begin VB.Label MapBounds 
            Caption         =   "?"
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
            TabIndex        =   77
            Top             =   2760
            Width           =   1215
         End
         Begin VB.Label MapZones 
            Caption         =   "?"
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
            TabIndex        =   76
            Top             =   3000
            Width           =   1215
         End
         Begin VB.Label Label31 
            Alignment       =   1  'Right Justify
            Caption         =   "Vectors"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   195
            Left            =   240
            TabIndex        =   75
            Top             =   2280
            Width           =   975
         End
         Begin VB.Label Label30 
            Alignment       =   1  'Right Justify
            Caption         =   "Bounds"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   195
            Left            =   240
            TabIndex        =   74
            Top             =   2760
            Width           =   975
         End
         Begin VB.Label Label29 
            Alignment       =   1  'Right Justify
            Caption         =   "Zones"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   195
            Left            =   240
            TabIndex        =   73
            Top             =   3000
            Width           =   975
         End
         Begin VB.Label Label26 
            Alignment       =   1  'Right Justify
            Caption         =   "Points"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   195
            Left            =   240
            TabIndex        =   72
            Top             =   2040
            Width           =   975
         End
         Begin VB.Label MapTotalPolys 
            Caption         =   "?"
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
            TabIndex        =   33
            Top             =   1560
            Width           =   1215
         End
         Begin VB.Label MapAvgPolys 
            Caption         =   "?"
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
            TabIndex        =   32
            Top             =   1320
            Width           =   1215
         End
         Begin VB.Label MapSpecial 
            Caption         =   "?"
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
            TabIndex        =   31
            Top             =   1080
            Width           =   1215
         End
         Begin VB.Label MapSubtract 
            Caption         =   "?"
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
            TabIndex        =   30
            Top             =   840
            Width           =   1215
         End
         Begin VB.Label MapAdd 
            Caption         =   "?"
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
            TabIndex        =   29
            Top             =   600
            Width           =   1215
         End
         Begin VB.Label MapBrushes 
            Caption         =   "?"
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
            TabIndex        =   28
            Top             =   360
            Width           =   1215
         End
         Begin VB.Label Label18 
            Alignment       =   1  'Right Justify
            Caption         =   "Special"
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
            Left            =   480
            TabIndex        =   27
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label Label19 
            Alignment       =   1  'Right Justify
            Caption         =   "Average Polys"
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
            TabIndex        =   26
            Top             =   1320
            Width           =   1095
         End
         Begin VB.Label Label20 
            Alignment       =   1  'Right Justify
            Caption         =   "Subtract"
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
            TabIndex        =   25
            Top             =   840
            Width           =   975
         End
         Begin VB.Label Label21 
            Alignment       =   1  'Right Justify
            Caption         =   "Add"
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
            TabIndex        =   24
            Top             =   600
            Width           =   1095
         End
         Begin VB.Label Label22 
            Alignment       =   1  'Right Justify
            Caption         =   "Brushes"
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
            Left            =   480
            TabIndex        =   23
            Top             =   360
            Width           =   735
         End
         Begin VB.Label Label23 
            Alignment       =   1  'Right Justify
            Caption         =   "Total Polys"
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
            TabIndex        =   22
            Top             =   1560
            Width           =   1095
         End
      End
      Begin VB.CheckBox AutoBSP 
         Caption         =   "&Auto BSP"
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
         TabIndex        =   20
         Top             =   4440
         Value           =   1  'Checked
         Width           =   1575
      End
      Begin VB.Frame Frame5 
         Caption         =   "Lighting Stats"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   -1  'True
            Strikethrough   =   0   'False
         EndProperty
         Height          =   2175
         Left            =   -74880
         TabIndex        =   5
         Top             =   480
         Width           =   3375
         Begin VB.Label Label17 
            Alignment       =   1  'Right Justify
            Caption         =   "Avg Size"
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
            Left            =   660
            TabIndex        =   19
            Top             =   1080
            Width           =   735
         End
         Begin VB.Label Label16 
            Alignment       =   1  'Right Justify
            Caption         =   "Max Size"
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
            Left            =   660
            TabIndex        =   18
            Top             =   1320
            Width           =   735
         End
         Begin VB.Label Label15 
            Alignment       =   1  'Right Justify
            Caption         =   "Texel coverage"
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
            Left            =   180
            TabIndex        =   17
            Top             =   1560
            Width           =   1215
         End
         Begin VB.Label Label14 
            Alignment       =   1  'Right Justify
            Caption         =   "Mesh Points"
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
            Left            =   300
            TabIndex        =   16
            Top             =   840
            Width           =   1095
         End
         Begin VB.Label Label11 
            Alignment       =   1  'Right Justify
            Caption         =   "Meshes"
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
            Left            =   660
            TabIndex        =   15
            Top             =   600
            Width           =   735
         End
         Begin VB.Label LightMeshes 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "Meshes"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   14
            Top             =   600
            Width           =   975
         End
         Begin VB.Label LightMeshPts 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "MeshPts"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   13
            Top             =   840
            Width           =   975
         End
         Begin VB.Label LightAvgSize 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "AvgSize"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   12
            Top             =   1080
            Width           =   975
         End
         Begin VB.Label LightMaxSize 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "MaxSize"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   11
            Top             =   1320
            Width           =   975
         End
         Begin VB.Label LightCacheSize 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "CacheSize"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   10
            Top             =   1560
            Width           =   975
         End
         Begin VB.Label Label24 
            Alignment       =   1  'Right Justify
            Caption         =   "Lights"
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
            Left            =   420
            TabIndex        =   9
            Top             =   360
            Width           =   975
         End
         Begin VB.Label Label25 
            Alignment       =   1  'Right Justify
            Caption         =   "Square Meters"
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
            Left            =   300
            TabIndex        =   8
            Top             =   1800
            Width           =   1095
         End
         Begin VB.Label LightMeters 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "Meters"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   7
            Top             =   1800
            Width           =   975
         End
         Begin VB.Label LightCount 
            Caption         =   "?"
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
            Left            =   1500
            LinkItem        =   "Count"
            LinkTopic       =   "UNREALSV|LIGHT"
            TabIndex        =   6
            Top             =   360
            Width           =   975
         End
      End
      Begin VB.Frame Frame6 
         Caption         =   "Raytracing Options"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   -1  'True
            Strikethrough   =   0   'False
         EndProperty
         Height          =   1095
         Left            =   -74880
         TabIndex        =   3
         Top             =   2760
         Width           =   3375
         Begin VB.CheckBox LightSel 
            Caption         =   "Apply selected lights and lights in selected zone descriptors only."
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
            TabIndex        =   4
            Top             =   420
            Width           =   2775
         End
      End
   End
End
Attribute VB_Name = "frmRebuilder"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim UpdatingTab As Integer

Private Sub Backs_Change()
    pBacks.Caption = ShowPercentz(Caption)
End Sub

Private Sub Balance_Change()
    BalanceValue.Caption = Trim(Str(Balance.Value))
End Sub

Private Sub Branches_Change()
    pBranches.Caption = ShowPercentz(Caption)
End Sub

Private Sub Command1_Click()
    Ed.ServerExec "PATHS DEFINE"
End Sub

Private Sub Command2_Click()
    Hide
End Sub

'
' Lesson learned: You can't do things like with OLE: the FORM_Paint function could be called
' while we're in the middle of an existing OLE RPC call and then this would call Ed.ServerGetProp,
' which violates the rule of one active RPC call at a time. This really is only the tip of the
' iceberg of problems with OLE and mixed-language programming.
'
'Private Sub Form_Paint()
'    UpdStats_Click
'End Sub

Private Sub UpdStats_Click()
    Select Case Tab1.Tab
        Case 0: RefreshGeomStats
        Case 1: RefreshBSPStats
        Case 2: RefreshLightStats
    End Select
End Sub

Private Sub Coplanars_Change()
    pCoplanars.Caption = ShowPercentz(Caption)
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "Rebuilder", TOP_PANEL)
    RefreshGeomStats
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Fronts_Change()
    pFronts.Caption = ShowPercentz(Caption)
End Sub

Private Sub Go_Click()
    Select Case Tab1.Tab
        Case 0: Call GoGeometry
        Case 1: Call GoBSP
        Case 2: Call GoLights
    End Select
    Beep
End Sub

Private Sub GoBSP()
    Dim Cmd As String
    '
    ' Build BSP rebuld command:
    '
    Cmd = "BSP REBUILD"
    '
    If Lame.Value Then
        Cmd = Cmd & " LAME"
    ElseIf Good.Value Then
        Cmd = Cmd & " GOOD"
    ElseIf Optimal.Value Then
        Cmd = Cmd & " OPTIMAL"
    End If
    '
    Cmd = Cmd & " BALANCE=" & Trim(Str(Balance.Value))
    '
    If BuildZones.Value Then
        Cmd = Cmd & " ZONES"
    End If
    '
    If OptGeom.Value Then
        Cmd = Cmd & " OPTGEOM"
    End If
    '
    Ed.BeginSlowTask "Rebuilding BSP"
    Ed.ServerExec Cmd
    Ed.EndSlowTask
    '
    If (Tab1.Tab = 1) Then UpdateTab (1)
    '
    If AutoLights.Value Then
        GoLights
    End If
    '
End Sub

Private Sub GoGeometry()
    '
    Dim TempResult As Integer
    '
    Ed.BeginSlowTask "Rebuilding Map"
    Ed.ServerExec "MAP REBUILD"
    Ed.EndSlowTask
    '
    If (Tab1.Tab = 0) Then UpdateTab (0)
    '
    If AutoBSP Then
        GoBSP
    End If
    '
End Sub

Private Sub GoLights()
    Dim Cmd As String
    '
    Cmd = "LIGHT APPLY"
    '
    Cmd = Cmd & " SELECTED=" & OnOff(LightSel.Value)
    '
    Ed.BeginSlowTask "Illuminating world"
    Ed.ServerExec Cmd
    Ed.EndSlowTask
    '
    If (Tab1.Tab = 2) Then UpdateTab (2)
    '
End Sub

Private Sub RefreshBSPStats()
    EdPolys.Caption = Ed.ServerGetProp("BSP", "Polys")
    Nodes.Caption = Ed.ServerGetProp("BSP", "Nodes")
    MaxDepth.Caption = Ed.ServerGetProp("BSP", "MaxDepth")
    AvgDepth.Caption = Ed.ServerGetProp("BSP", "AvgDepth")
    Branches.Caption = Ed.ServerGetProp("BSP", "Branches")
    Coplanars.Caption = Ed.ServerGetProp("BSP", "Coplanars")
    Fronts.Caption = Ed.ServerGetProp("BSP", "Fronts")
    Backs.Caption = Ed.ServerGetProp("BSP", "Backs")
    Leaves.Caption = Ed.ServerGetProp("BSP", "Leaves")
    '
    ' Calculate percents:
    '
    pLeaves.Caption = ShowPercentz(Leaves.Caption)
    pBacks.Caption = ShowPercentz(Backs.Caption)
    pFronts.Caption = ShowPercentz(Fronts.Caption)
    pCoplanars.Caption = ShowPercentz(Coplanars.Caption)
    pBranches.Caption = ShowPercentz(Branches.Caption)
    '
    If Val(EdPolys.Caption) = 0 Then
        Ratio.Caption = "0"
    Else
        Ratio.Caption = Trim(Str(Int(100 * Val(Nodes.Caption) / Val(EdPolys.Caption)) / 100))
    End If
End Sub

Private Sub RefreshGeomStats()
    MapBrushes.Caption = Ed.ServerGetProp("Map", "Brushes")
    MapAdd.Caption = Ed.ServerGetProp("Map", "Add")
    MapSubtract.Caption = Ed.ServerGetProp("Map", "Subtract")
    MapSpecial.Caption = Ed.ServerGetProp("Map", "Special")
    MapAvgPolys.Caption = Ed.ServerGetProp("Map", "AvgPolys")
    MapTotalPolys.Caption = Ed.ServerGetProp("Map", "TotalPolys")
    MapPoints.Caption = Ed.ServerGetProp("Map", "Points")
    MapVectors.Caption = Ed.ServerGetProp("Map", "Vectors")
    MapSides.Caption = Ed.ServerGetProp("Map", "Sides")
    MapBounds.Caption = Ed.ServerGetProp("Map", "Bounds")
    MapZones.Caption = Ed.ServerGetProp("Map", "Zones")
End Sub

Private Sub RefreshLightStats()
    LightCount.Caption = Ed.ServerGetProp("Light", "Count")
    LightMeshes.Caption = Ed.ServerGetProp("Light", "Meshes")
    LightMeshPts.Caption = Ed.ServerGetProp("Light", "MeshPts")
    LightCacheSize.Caption = Ed.ServerGetProp("Light", "CacheSize")
    LightAvgSize.Caption = Ed.ServerGetProp("Light", "AvgSize")
    LightMaxSize.Caption = Ed.ServerGetProp("Light", "MaxSize")
    LightMeters.Caption = Ed.ServerGetProp("Light", "Meters")
End Sub

Private Function ShowPercentz(ByVal Caption As String) As String
    '
    If Val(Nodes.Caption) = 0 Then
        ShowPercentz = "(0%)"
    Else
        ShowPercentz = "(" & Trim(Str(Int(100 * Val(Caption) / Val(Nodes.Caption)))) & "%)"
    End If
    '
End Function


Private Sub UpdateGoButton(N As Integer)
    Select Case N
        Case 0:
            Go.Caption = "&Rebuild Geometry"
        Case 1:
            Go.Caption = "&Rebuild BSP"
        Case 2:
            Go.Caption = "&Apply Lights"
    End Select
End Sub

Private Sub UpdateTab(N As Integer)
    If UpdatingTab = 0 Then
        UpdatingTab = 1 ' Prevent recursion
        Select Case N
            Case 0: RefreshGeomStats
            Case 1: RefreshBSPStats
            Case 2: RefreshLightStats
        End Select
        '
        If Tab1.Tab <> N Then
            Tab1.Tab = N
        End If
        '
        UpdateGoButton (N)
        UpdatingTab = 0
    End If
End Sub

Private Sub Tab1_Click(PreviousTab As Integer)
    UpdateTab (Tab1.Tab)
End Sub

