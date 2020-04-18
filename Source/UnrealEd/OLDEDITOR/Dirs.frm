VERSION 4.00
Begin VB.Form frmDirectories 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Directories"
   ClientHeight    =   4695
   ClientLeft      =   3450
   ClientTop       =   1290
   ClientWidth     =   6555
   BeginProperty Font 
      name            =   "MS Sans Serif"
      charset         =   0
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   Height          =   5160
   HelpContextID   =   108
   Left            =   3390
   LinkTopic       =   "Form6"
   ScaleHeight     =   4695
   ScaleWidth      =   6555
   ShowInTaskbar   =   0   'False
   Top             =   885
   Width           =   6675
   Begin VB.CommandButton Command2 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
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
      Left            =   4980
      TabIndex        =   11
      Top             =   4260
      Width           =   1455
   End
   Begin VB.CommandButton Command1 
      Caption         =   "&OK"
      Default         =   -1  'True
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
      Left            =   120
      TabIndex        =   10
      Top             =   4260
      Width           =   1455
   End
   Begin VB.Frame Frame1 
      Caption         =   "UnrealEd Subdirectories"
      BeginProperty Font 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   11.25
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Height          =   4035
      Left            =   120
      TabIndex        =   14
      Top             =   120
      Width           =   6315
      Begin VB.TextBox MacroDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   9
         Text            =   "Text1"
         Top             =   3600
         Width           =   2895
      End
      Begin VB.TextBox ToolDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   3
         Text            =   "Text1"
         Top             =   1440
         Width           =   2895
      End
      Begin VB.TextBox BrushDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   2
         Text            =   "Text1"
         Top             =   1080
         Width           =   2895
      End
      Begin VB.TextBox ModelDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   6
         Text            =   "Text1"
         Top             =   2520
         Width           =   2895
      End
      Begin VB.TextBox SoundDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   7
         Text            =   "Text1"
         Top             =   2880
         Width           =   2895
      End
      Begin VB.TextBox TextureDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   1
         Text            =   "Text1"
         Top             =   720
         Width           =   2895
      End
      Begin VB.TextBox ClassDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   4
         Text            =   "Text1"
         Top             =   1800
         Width           =   2895
      End
      Begin VB.TextBox ShapeDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   5
         Text            =   "Text1"
         Top             =   2160
         Width           =   2895
      End
      Begin VB.TextBox MusicDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   8
         Text            =   "Text1"
         Top             =   3240
         Width           =   2895
      End
      Begin VB.TextBox MapDir 
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
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   0
         Text            =   "Text1"
         Top             =   360
         Width           =   2895
      End
      Begin VB.Label Label29 
         Caption         =   "Example: macros"
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
         Left            =   4680
         TabIndex        =   32
         Top             =   3600
         Width           =   1395
      End
      Begin VB.Label Label28 
         Alignment       =   1  'Right Justify
         Caption         =   "Macros"
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
         Left            =   420
         TabIndex        =   31
         Top             =   3600
         Width           =   795
      End
      Begin VB.Label Label27 
         Alignment       =   1  'Right Justify
         Caption         =   "Add-on tools"
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
         TabIndex        =   19
         Top             =   1440
         Width           =   1095
      End
      Begin VB.Label Label26 
         Caption         =   "Example: tools"
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
         Left            =   4680
         TabIndex        =   21
         Top             =   1440
         Width           =   1455
      End
      Begin VB.Label Label6 
         Alignment       =   1  'Right Justify
         Caption         =   "Brushes"
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
         Left            =   360
         TabIndex        =   22
         Top             =   1080
         Width           =   855
      End
      Begin VB.Label Label10 
         Caption         =   "Example: brushes"
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
         Left            =   4680
         TabIndex        =   30
         Top             =   1080
         Width           =   1515
      End
      Begin VB.Label Label13 
         Alignment       =   1  'Right Justify
         Caption         =   "3D Models"
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
         Left            =   240
         TabIndex        =   29
         Top             =   2520
         Width           =   975
      End
      Begin VB.Label Label14 
         Caption         =   "Example: models"
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
         Left            =   4680
         TabIndex        =   28
         Top             =   2520
         Width           =   1395
      End
      Begin VB.Label Label25 
         Caption         =   "Example: shapes"
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
         Left            =   4680
         TabIndex        =   27
         Top             =   2160
         Width           =   1395
      End
      Begin VB.Label Label24 
         Alignment       =   1  'Right Justify
         Caption         =   "2D Shapes"
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
         TabIndex        =   26
         Top             =   2160
         Width           =   1095
      End
      Begin VB.Label Label19 
         Caption         =   "Example: music"
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
         Left            =   4680
         TabIndex        =   25
         Top             =   3240
         Width           =   1395
      End
      Begin VB.Label Label18 
         Caption         =   "Example: sounds"
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
         Left            =   4680
         TabIndex        =   24
         Top             =   2880
         Width           =   1335
      End
      Begin VB.Label Label17 
         Alignment       =   1  'Right Justify
         Caption         =   "Music"
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
         Left            =   540
         TabIndex        =   23
         Top             =   3240
         Width           =   675
      End
      Begin VB.Label Label12 
         Caption         =   "Example: textures"
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
         Left            =   4680
         TabIndex        =   12
         Top             =   720
         Width           =   1515
      End
      Begin VB.Label Label11 
         Caption         =   "Example: classes"
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
         Left            =   4680
         TabIndex        =   13
         Top             =   1800
         Width           =   1395
      End
      Begin VB.Label Label8 
         Caption         =   "Example: maps"
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
         Left            =   4680
         TabIndex        =   16
         Top             =   360
         Width           =   1395
      End
      Begin VB.Label Label7 
         Alignment       =   1  'Right Justify
         Caption         =   "Textures"
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
         Left            =   360
         TabIndex        =   20
         Top             =   720
         Width           =   855
      End
      Begin VB.Label Label5 
         Alignment       =   1  'Right Justify
         Caption         =   "Actor Classes"
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
         TabIndex        =   18
         Top             =   1800
         Width           =   1095
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Sounds"
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
         Left            =   480
         TabIndex        =   17
         Top             =   2880
         Width           =   735
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Maps"
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
         Left            =   420
         TabIndex        =   15
         Top             =   360
         Width           =   795
      End
   End
End
Attribute VB_Name = "frmDirectories"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()
    '
    ' Stick all settings into Profile structure
    '
    Ed.MapDir = MapDir.Text
    Ed.TextureDir = TextureDir.Text
    Ed.ClassDir = ClassDir.Text
    Ed.ShapeDir = ShapeDir.Text
    Ed.BrushDir = BrushDir.Text
    Ed.ModelDir = ModelDir.Text
    Ed.SoundDir = SoundDir.Text
    Ed.MusicDir = MusicDir.Text
    Ed.MacroDir = MacroDir.Text
    Ed.ToolDir = ToolDir.Text
    '
    Ed.SaveProfile
    Unload Me
End Sub

Private Sub Command2_Click()
   Unload Me
End Sub

Private Sub Form_Load()
    '
    Call Ed.SetOnTop(Me, "Directories", TOP_NORMAL)
    '
    ' Stick all settings into controls from Profile
    ' structure
    '
    MapDir.Text = Ed.MapDir
    TextureDir.Text = Ed.TextureDir
    ToolDir.Text = Ed.ToolDir
    ClassDir.Text = Ed.ClassDir
    ShapeDir.Text = Ed.ShapeDir
    BrushDir.Text = Ed.BrushDir
    ModelDir.Text = Ed.ModelDir
    SoundDir.Text = Ed.SoundDir
    MusicDir.Text = Ed.MusicDir
    MacroDir.Text = Ed.MacroDir
    '
End Sub

Private Sub UpdProfile(ByRef DestProfile As String, ByVal DirName As String)
    '
    ' Validate a path and stick it into the
    ' profile structure.
    '
    DestProfile = DirName
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub
