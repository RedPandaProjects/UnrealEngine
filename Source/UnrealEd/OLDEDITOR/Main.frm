VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "threed32.ocx"
Object = "{27395F88-0C0C-101B-A3C9-08002B2F49FB}#1.1#0"; "picclp32.ocx"
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "comctl32.ocx"
Begin VB.MDIForm frmMain 
   AutoShowChildren=   0   'False
   BackColor       =   &H00000000&
   Caption         =   "UnrealEd"
   ClientHeight    =   8130
   ClientLeft      =   2085
   ClientTop       =   1845
   ClientWidth     =   15960
   Icon            =   "Main.frx":0000
   LinkTopic       =   "Form1"
   ScrollBars      =   0   'False
   Visible         =   0   'False
   WindowState     =   2  'Maximized
   Begin Threed.SSPanel SSPanel1 
      Align           =   1  'Align Top
      Height          =   255
      Left            =   0
      TabIndex        =   9
      Top             =   0
      Width           =   15960
      _Version        =   65536
      _ExtentX        =   28152
      _ExtentY        =   450
      _StockProps     =   15
      BackColor       =   12632256
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   204
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Begin VB.TextBox ProgressText 
         BackColor       =   &H8000000F&
         BorderStyle     =   0  'None
         Height          =   285
         Left            =   3000
         Locked          =   -1  'True
         TabIndex        =   11
         Top             =   30
         Visible         =   0   'False
         Width           =   8655
      End
      Begin ComctlLib.ProgressBar ProgressBar 
         Height          =   255
         Left            =   0
         TabIndex        =   10
         Top             =   0
         Visible         =   0   'False
         Width           =   2895
         _ExtentX        =   5106
         _ExtentY        =   450
         _Version        =   327682
         Appearance      =   1
      End
   End
   Begin Threed.SSPanel Toolbar 
      Align           =   3  'Align Left
      Height          =   7875
      Left            =   0
      TabIndex        =   1
      Top             =   255
      Visible         =   0   'False
      Width           =   2070
      _Version        =   65536
      _ExtentX        =   3651
      _ExtentY        =   13891
      _StockProps     =   15
      BackColor       =   -2147483633
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   204
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Begin VB.Timer Timer 
         Interval        =   60000
         Left            =   600
         Top             =   120
      End
      Begin VB.TextBox Callback 
         Height          =   345
         Left            =   120
         MultiLine       =   -1  'True
         TabIndex        =   8
         Text            =   "Main.frx":030A
         Top             =   120
         Visible         =   0   'False
         Width           =   345
      End
      Begin VB.PictureBox Holder 
         BackColor       =   &H00808080&
         Height          =   4695
         Left            =   0
         ScaleHeight     =   4635
         ScaleWidth      =   1755
         TabIndex        =   3
         Top             =   1080
         Width           =   1815
         Begin Threed.SSRibbon ToolIcons 
            Height          =   615
            Index           =   0
            Left            =   0
            TabIndex        =   4
            Top             =   0
            Visible         =   0   'False
            Width           =   615
            _Version        =   65536
            _ExtentX        =   1085
            _ExtentY        =   1085
            _StockProps     =   65
            BackColor       =   8421504
            PictureDnChange =   0
            RoundedCorners  =   0   'False
            BevelWidth      =   0
            Outline         =   0   'False
         End
         Begin VB.Label StatusText 
            Alignment       =   2  'Center
            BackStyle       =   0  'Transparent
            Caption         =   "Status"
            Height          =   495
            Left            =   0
            TabIndex        =   5
            Top             =   600
            Width           =   1695
         End
      End
      Begin VB.VScrollBar Scroller 
         Height          =   5775
         Left            =   1800
         TabIndex        =   2
         Top             =   0
         Visible         =   0   'False
         Width           =   245
      End
   End
   Begin Threed.SSPanel BrowserPanel 
      Align           =   4  'Align Right
      Height          =   7875
      Left            =   13530
      TabIndex        =   0
      Top             =   255
      Visible         =   0   'False
      Width           =   2430
      _Version        =   65536
      _ExtentX        =   4286
      _ExtentY        =   13891
      _StockProps     =   15
      BackColor       =   -2147483633
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   204
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      BorderWidth     =   0
      Begin VB.ComboBox BrowserTopicCombo 
         BackColor       =   &H00C0C0C0&
         Height          =   315
         Left            =   810
         Style           =   2  'Dropdown List
         TabIndex        =   6
         Tag             =   "Various resources you can browse"
         Top             =   45
         Width           =   1605
      End
      Begin VB.Label Label1 
         Caption         =   "Browse"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   9
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   -1  'True
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   75
         TabIndex        =   7
         Top             =   75
         Width           =   735
      End
   End
   Begin PicClip.PictureClip HiPics 
      Left            =   6600
      Top             =   480
      _ExtentX        =   7408
      _ExtentY        =   9260
      _Version        =   393216
      Rows            =   10
      Cols            =   8
      Picture         =   "Main.frx":0313
   End
   Begin PicClip.PictureClip Pics 
      Left            =   2280
      Top             =   480
      _ExtentX        =   7408
      _ExtentY        =   9260
      _Version        =   393216
      Rows            =   10
      Cols            =   8
      Picture         =   "Main.frx":18437
   End
   Begin VB.Menu File 
      Caption         =   "&File"
      Begin VB.Menu New 
         Caption         =   "&New level"
      End
      Begin VB.Menu Open 
         Caption         =   "&Open level"
         Shortcut        =   ^O
      End
      Begin VB.Menu Save 
         Caption         =   "&Save level"
         Shortcut        =   ^L
      End
      Begin VB.Menu SaveAs 
         Caption         =   "Save &As..."
         Shortcut        =   ^E
      End
      Begin VB.Menu ZWOOP 
         Caption         =   "-"
      End
      Begin VB.Menu PlayLevel 
         Caption         =   "&Play level"
         Shortcut        =   ^P
      End
      Begin VB.Menu X 
         Caption         =   "-"
      End
      Begin VB.Menu ImportLevel 
         Caption         =   "&Import Level"
      End
      Begin VB.Menu ExportLevel 
         Caption         =   "&Export level"
      End
      Begin VB.Menu ZSTOS 
         Caption         =   "-"
      End
      Begin VB.Menu Exit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu Edit 
      Caption         =   "&Edit"
      Begin VB.Menu EditCut 
         Caption         =   "Cu&t"
         Shortcut        =   ^X
      End
      Begin VB.Menu EditCopy 
         Caption         =   "&Copy"
         Shortcut        =   ^C
      End
      Begin VB.Menu EditPaste 
         Caption         =   "&Paste"
         Shortcut        =   ^V
      End
      Begin VB.Menu EditDivider1 
         Caption         =   "-"
      End
      Begin VB.Menu EditUndo 
         Caption         =   "&Undo"
         Shortcut        =   ^Z
      End
      Begin VB.Menu EditRedo 
         Caption         =   "&Redo"
         Shortcut        =   ^R
      End
      Begin VB.Menu ZBORE 
         Caption         =   "-"
      End
      Begin VB.Menu EditFind 
         Caption         =   "&Find"
         Shortcut        =   ^F
         Visible         =   0   'False
      End
      Begin VB.Menu EditFindNext 
         Caption         =   "Find &Next"
         Shortcut        =   {F3}
         Visible         =   0   'False
      End
      Begin VB.Menu EditDivider2 
         Caption         =   "-"
         Visible         =   0   'False
      End
      Begin VB.Menu Duplicate 
         Caption         =   "&Duplicate"
         Shortcut        =   ^W
      End
      Begin VB.Menu Delete 
         Caption         =   "Delete"
         Shortcut        =   {DEL}
      End
      Begin VB.Menu XXX 
         Caption         =   "-"
      End
      Begin VB.Menu SelectNone 
         Caption         =   "Select &None"
      End
      Begin VB.Menu SelectAll 
         Caption         =   "Select &All"
      End
      Begin VB.Menu SelectDialog 
         Caption         =   "Select Polys..."
         Begin VB.Menu SelMatchGroups 
            Caption         =   "Matching Groups (Shift-G)"
         End
         Begin VB.Menu SelMatchItems 
            Caption         =   "Matching Items (Shift-I)"
         End
         Begin VB.Menu SelMatchBrush 
            Caption         =   "Matching Brush (Shift-B)"
         End
         Begin VB.Menu SelMatchTex 
            Caption         =   "Matching Texture (Shift-T)"
         End
         Begin VB.Menu WZYRA 
            Caption         =   "-"
         End
         Begin VB.Menu SelAllAdj 
            Caption         =   "All Adjacents (Shift-J)"
         End
         Begin VB.Menu SelCoplAdj 
            Caption         =   "Adjacent Coplanars (Shift-C)"
         End
         Begin VB.Menu SelAdjWalls 
            Caption         =   "Adjacent Walls (Shift-W)"
         End
         Begin VB.Menu SelAdjFloors 
            Caption         =   "Adjacent Floors/Ceils (Shift-F)"
         End
         Begin VB.Menu SelAdjSlants 
            Caption         =   "Adjacent Slants (Shift-S)"
         End
         Begin VB.Menu ZIJWZ 
            Caption         =   "-"
         End
         Begin VB.Menu SelReverse 
            Caption         =   "Reverse (Shift-Q)"
         End
         Begin VB.Menu WIJQZA 
            Caption         =   "-"
         End
         Begin VB.Menu SelMemorize 
            Caption         =   "Memorize Set (Shift-M)"
         End
         Begin VB.Menu SelRecall 
            Caption         =   "Recall Memory (Shift-R)"
         End
         Begin VB.Menu SelIntersection 
            Caption         =   "Or with Memory (Shift-O)"
         End
         Begin VB.Menu SelUnion 
            Caption         =   "And with Memory (Shift-U)"
         End
         Begin VB.Menu SelXor 
            Caption         =   "Xor with Memory (Shift-X)"
         End
      End
   End
   Begin VB.Menu ScriptMenu 
      Caption         =   "&Script"
      Begin VB.Menu ScriptMakeChanged 
         Caption         =   "&Compile Changed Scripts"
         Shortcut        =   {F7}
      End
      Begin VB.Menu ScriptMakeAll 
         Caption         =   "Compile &All Scripts"
      End
   End
   Begin VB.Menu Brush 
      Caption         =   "&Brush"
      Begin VB.Menu BrushAdd 
         Caption         =   "&Add"
         Shortcut        =   ^A
      End
      Begin VB.Menu BrushSubtract 
         Caption         =   "&Subtract"
         Shortcut        =   ^S
      End
      Begin VB.Menu BrushIntersect 
         Caption         =   "&Intersect"
         Shortcut        =   ^N
      End
      Begin VB.Menu BrushDeintersect 
         Caption         =   "&Deintersect"
         Shortcut        =   ^D
      End
      Begin VB.Menu AddMovableBrush 
         Caption         =   "&Add Movable Brush"
      End
      Begin VB.Menu AddSpecial 
         Caption         =   "&Add Special..."
      End
      Begin VB.Menu ZRK 
         Caption         =   "-"
      End
      Begin VB.Menu ParametricSolids 
         Caption         =   "&Parametric solids"
         Begin VB.Menu ParSolRect 
            Caption         =   "&Rectangle"
         End
         Begin VB.Menu ParSolTube 
            Caption         =   "Cyllinder/Tube"
         End
         Begin VB.Menu ParSolCone 
            Caption         =   "Cone/Spire"
         End
         Begin VB.Menu ParSolLinearStair 
            Caption         =   "Linear Staircase"
         End
         Begin VB.Menu ParSolSpiralStair 
            Caption         =   "Spiral Staircase"
         End
         Begin VB.Menu CurvedStair 
            Caption         =   "Curved Staircase"
         End
         Begin VB.Menu ParSolSphereDome 
            Caption         =   "Sphere/Dome"
         End
      End
      Begin VB.Menu ZGYM 
         Caption         =   "-"
      End
      Begin VB.Menu BrushReset 
         Caption         =   "R&eset"
         Begin VB.Menu ResetRotation 
            Caption         =   "&Rotation"
         End
         Begin VB.Menu ResetScale 
            Caption         =   "&Scale"
         End
         Begin VB.Menu ResetPosition 
            Caption         =   "&Position"
         End
         Begin VB.Menu ZYCLUNT 
            Caption         =   "-"
         End
         Begin VB.Menu ResetAll 
            Caption         =   "&All"
         End
      End
      Begin VB.Menu ZZZ 
         Caption         =   "-"
      End
      Begin VB.Menu LoadBrush 
         Caption         =   "&Load"
         Shortcut        =   ^B
      End
      Begin VB.Menu SaveBrush 
         Caption         =   "&Save"
      End
      Begin VB.Menu SaveBrushAs 
         Caption         =   "Sa&ve As..."
      End
      Begin VB.Menu XCYZ 
         Caption         =   "-"
      End
      Begin VB.Menu BrushImport 
         Caption         =   "&Import..."
      End
      Begin VB.Menu BrushExport 
         Caption         =   "&Export..."
      End
      Begin VB.Menu BrushHull 
         Caption         =   "&Advanced.."
         Visible         =   0   'False
         Begin VB.Menu AddCutaway 
            Caption         =   "&Cutaway Zone"
         End
         Begin VB.Menu BrushSliceTex 
            Caption         =   "No-&Terrain Zone"
         End
         Begin VB.Menu AddNoCut 
            Caption         =   "&No-Cut Zone"
         End
      End
   End
   Begin VB.Menu Camera 
      Caption         =   "&Camera"
      Begin VB.Menu CamAllViews 
         Caption         =   "&All Views"
      End
      Begin VB.Menu CamTwoViews 
         Caption         =   "&Persp + Overhead"
      End
      Begin VB.Menu CamPersp 
         Caption         =   "P&ersp Only"
      End
      Begin VB.Menu CamOvh 
         Caption         =   "&Overhead Only"
      End
      Begin VB.Menu ZILBERT 
         Caption         =   "-"
      End
      Begin VB.Menu CamOpenFree 
         Caption         =   "&Open Free Camera"
      End
      Begin VB.Menu CamCloseAllFree 
         Caption         =   "&Close All Free Cameras"
      End
      Begin VB.Menu CameraResetAll 
         Caption         =   "&Reset All"
      End
   End
   Begin VB.Menu Options 
      Caption         =   "&Options"
      Begin VB.Menu ActorProperties 
         Caption         =   "&Actor properties..."
         Shortcut        =   {F4}
      End
      Begin VB.Menu SurfaceProperties 
         Caption         =   "&Surface properties..."
         Shortcut        =   {F5}
      End
      Begin VB.Menu Project 
         Caption         =   "&Level properties..."
         Shortcut        =   {F6}
      End
      Begin VB.Menu Rebuild 
         Caption         =   "&Rebuild..."
         Shortcut        =   {F8}
      End
      Begin VB.Menu Preferences 
         Caption         =   "&Preferences..."
      End
      Begin VB.Menu ValidateLevel 
         Caption         =   "&Validate Level"
      End
      Begin VB.Menu ViewLevelLinks 
         Caption         =   "&Show Links"
      End
   End
   Begin VB.Menu WIndow 
      Caption         =   "&Window"
      Begin VB.Menu MeshViewer 
         Caption         =   "&Mesh Viewer"
      End
      Begin VB.Menu WindowLog 
         Caption         =   "&Log"
      End
      Begin VB.Menu ScriptResults 
         Caption         =   "&Results"
      End
      Begin VB.Menu TwoDee 
         Caption         =   "&2D Shape Editor (Experimental)"
      End
      Begin VB.Menu FloorLofter 
         Caption         =   "Floor Lofter (Experimental)"
      End
      Begin VB.Menu ZFUS 
         Caption         =   "-"
      End
      Begin VB.Menu WinToolbar 
         Caption         =   "&Toolbar"
         Begin VB.Menu WinToolbarLeft 
            Caption         =   "&Left"
         End
         Begin VB.Menu WinToolbarRight 
            Caption         =   "&Right"
         End
      End
      Begin VB.Menu WinPanel 
         Caption         =   "&Panel"
         Visible         =   0   'False
         Begin VB.Menu WinPanelBottom 
            Caption         =   "&Bottom"
         End
         Begin VB.Menu WinPanelTop 
            Caption         =   "&Top"
         End
         Begin VB.Menu WinPanelHide 
            Caption         =   "&Hide"
         End
      End
      Begin VB.Menu WinBrowser 
         Caption         =   "&Browser"
         Begin VB.Menu WinBrowserRight 
            Caption         =   "&Right"
         End
         Begin VB.Menu WinBrowserLeft 
            Caption         =   "&Left"
         End
         Begin VB.Menu WinBrowserHide 
            Caption         =   "&Hide"
         End
      End
   End
   Begin VB.Menu Help 
      Caption         =   "&Help"
      Begin VB.Menu About 
         Caption         =   "&About UnrealEd"
      End
      Begin VB.Menu EpicWeb 
         Caption         =   "&Epic's Web Site"
      End
      Begin VB.Menu QIDJWE 
         Caption         =   "-"
      End
      Begin VB.Menu HelpIndex 
         Caption         =   "&Help Topics"
         Shortcut        =   {F1}
      End
      Begin VB.Menu HelpCam 
         Caption         =   "Help on &Cameras"
         Shortcut        =   {F2}
      End
      Begin VB.Menu RelNotes 
         Caption         =   "&Release Notes"
      End
      Begin VB.Menu ZIELZEB 
         Caption         =   "-"
      End
      Begin VB.Menu UnrealScriptHelp 
         Caption         =   "&UnrealScript help"
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim GInitialResized As Integer
Public hwndScript As Long
Public ScriptForm As frmScriptEd

Const BrowserWidth1 = 140
Const BrowserWidth2 = 152

Private Sub About_Click()
    frmDialogs.About.ShowHelp ' WinHelp
End Sub

Private Sub AddCutFirst_Click()
    Ed.BeginSlowTask "Adding brush to world"
    Ed.ServerExec "BRUSH ADD CUTFIRST"
    Ed.EndSlowTask
End Sub

Private Sub ActorProperties_Click()
    Ed.ServerExec "HOOK ACTORPROPERTIES" ''xyzzy
End Sub

Private Sub AddCutaway_Click()
    Ed.BeginSlowTask "Adding Cutaway Zone"
    Ed.ServerExec "BRUSH ADD CUTAWAY"
    Ed.EndSlowTask
End Sub

Private Sub AddMovableBrush_Click()
    'Ed.BeginSlowTask "Adding movable brush to world"
    Ed.ServerExec "BRUSH ADDMOVABLE"
    'Ed.EndSlowTask
End Sub

Private Sub AddNoCut_Click()
    Ed.BeginSlowTask "Adding No-Cut Zone"
    Ed.ServerExec "BRUSH ADD NOCUT"
    Ed.EndSlowTask
    '
    Call MsgBox("A no-cut zone will be added.  This will take effect the next time you rebuild geometry.", 64, "Adding No-Cut Zone")
End Sub

Private Sub AddSpecial_Click()
    frmAddSpecial.Show
End Sub

Private Sub Browser_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        PopupMenu frmPopups.Browser
    End If
End Sub

Private Sub BrowserHolder_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        PopupMenu frmPopups.Browser
    End If
End Sub

Private Sub BrowserTopicCombo_Click()
    Ed.SetBrowserTopic (BrowserTopicCombo.Text)
End Sub

Private Sub BrushAdd_Click()
    'Ed.BeginSlowTask "Adding brush to world"
    Ed.ServerExec "BRUSH ADD"
    'Ed.EndSlowTask
End Sub

Private Sub BrushDeintersect_Click()
    'Ed.BeginSlowTask "Deintersecting brush"
    Ed.ServerExec "BRUSH FROM DEINTERSECTION"
    'Ed.EndSlowTask
End Sub

Private Sub BrushExport_Click()
    '
    Dim ExportFname As String
    '
    ' Prompt for filename
    '
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.ExportBrush.ShowSave 'Modal Save-As Box
    ExportFname = frmDialogs.ExportBrush.FileName
    '
    Call UpdateDialog(frmDialogs.ExportBrush)
    If (ExportFname <> "") Then
        Ed.BeginSlowTask "Exporting brush"
        Ed.ServerExec "BRUSH EXPORT FILE=" & Quotes(ExportFname)
        Ed.EndSlowTask
    End If
Skip: Ed.ServerEnable
End Sub

Private Sub BrushImport_Click()
    '
    ' Dialog for "Brush Import":
    '
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.ImportBrush.FileName = ""
    frmDialogs.ImportBrush.DefaultExt = "t3d"
    frmDialogs.ImportBrush.ShowOpen 'Modal File-Open Box
    '
    Call UpdateDialog(frmDialogs.ImportBrush)
    If (frmDialogs.ImportBrush.FileName <> "") Then
        frmBrushImp.Show 1
    End If
Skip: Ed.ServerEnable
End Sub

Private Sub BrushIntersect_Click()
    'Ed.BeginSlowTask "Intersecting brush"
    Ed.ServerExec "BRUSH FROM INTERSECTION"
    'Ed.EndSlowTask
End Sub

Private Sub BrushReset_Click()
    Ed.ServerExec "BRUSH RESET"
End Sub

Private Sub BrushSliceTex_Click()
    Ed.BeginSlowTask "Adding No-Terrain Zone"
    Ed.ServerExec "BRUSH ADD NOTERRAIN"
    Ed.EndSlowTask
    '
    Call MsgBox("A No-Terrain zone will be added.  This will take effect the next time you rebuild geometry.  See the terrain help for complete information on using No-Terrain zones in UnrealEd.", 64, "Adding No-Terrain Zone")
End Sub

Private Sub BrushSubtract_Click()
    'Ed.BeginSlowTask "Subtracting brush from world"
    Ed.ServerExec "BRUSH SUBTRACT"
    'Ed.EndSlowTask
End Sub

Private Sub CameraAhead_Click()
    Ed.ServerExec "CAMERA LOOK AHEAD"
End Sub

Private Sub CameraDown_Click()
    Ed.ServerExec "CAMERA LOOK DOWN"
End Sub

Private Sub CameraEast_Click()
    Ed.ServerExec "CAMERA LOOK EAST"
End Sub

Private Sub CameraEntire_Click()
    Ed.ServerExec "CAMERA LOOK ENTIREMAP"
End Sub

Private Sub CameraNorth_Click()
    Ed.ServerExec "CAMERA LOOK NORTH"
End Sub

Private Sub CameraSouth_Click()
    Ed.ServerExec "CAMERA LOOK SOUTH"
End Sub

Private Sub CameraUp_Click()
    Ed.ServerExec "CAMERA LOOK UP"
End Sub

Private Sub CameraWest_Click()
    Ed.ServerExec "CAMERA LOOK WEST"
End Sub

Private Sub ClassBrows_Click()
    frmClassBrowser.Show
End Sub


Private Sub CamAllViews_Click()
    Ed.CameraVertRatio = 0.66
    Ed.CameraLeftRatio = 0.5
    Ed.CameraRightRatio = 0.5
    ResizeAll (True)
End Sub

Private Sub CamCloseAllFree_Click()
    Ed.ServerExec "CAMERA CLOSE FREE"
End Sub


Private Sub CameraResetAll_Click()
    Ed.ServerExec "CAMERA CLOSE ALL"
    Ed.CameraVertRatio = 0.66
    Ed.CameraLeftRatio = 0.5
    Ed.CameraRightRatio = 0.5
    ResizeAll (False)
End Sub

Private Sub CamOpenFree_Click()
    Ed.OpenFreeCamera
End Sub

Private Sub CamOvh_Click()
    Ed.CameraVertRatio = 1#
    Ed.CameraLeftRatio = 1#
    ResizeAll (True)
End Sub

Private Sub CamPersp_Click()
    Ed.CameraVertRatio = 1#
    Ed.CameraLeftRatio = 0#
    ResizeAll (True)
End Sub

Private Sub CamTwoViews_Click()
    Ed.CameraVertRatio = 1#
    Ed.CameraLeftRatio = 0.4
    ResizeAll (True)
End Sub

Private Sub Command1_Click()
    ToolHelp (123)
End Sub

Private Sub CurvedStair_Click()
    frmParSolCurvedStair.Show
End Sub

Private Sub Delete_Click()
    If MsgBox("Are you sure you want to delete?", vbOKCancel) = vbOK Then
        Ed.ServerExec "DELETE"
    End If
End Sub

Private Sub Directories_Click()
   'Ed.ServerDisable
   'frmDirectories.Show 1
   'Ed.ServerEnable
End Sub

Private Sub Duplicate_Click()
    Ed.ServerExec "DUPLICATE"
End Sub

Private Sub EditCopy_Click()
    If hwndScript <> 0 Then
        ScriptForm.EditCopy_Click
    Else
        Ed.ServerExec "EDIT COPY"
    End If
End Sub

Private Sub EditCut_Click()
    If hwndScript <> 0 Then
        ScriptForm.EditCut_Click
    Else
        Ed.ServerExec "EDIT CUT"
    End If
End Sub

Private Sub EditFind_Click()
    If hwndScript <> 0 Then
        ScriptForm.EditFind_Click
    End If
End Sub

Private Sub EditFindNext_Click()
    If hwndScript <> 0 Then
        ScriptForm.EditFindNext_Click
    End If
End Sub

Private Sub EditPaste_Click()
    If hwndScript <> 0 Then
        ScriptForm.EditPaste_Click
    Else
        Ed.ServerExec "EDIT PASTE"
    End If
End Sub

Private Sub EpicButton_Click()
    Ed.ServerExec "LAUNCH WEB"
End Sub

Private Sub EpicWeb_Click()
    Ed.ServerExec "LAUNCH WEB"
End Sub

Private Sub Exit_Click()
   Unload Me
End Sub

Private Sub ExportLevel_Click()
    '
    Dim ExportFname As String
    '
    ' Prompt for filename
    '
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.ExportMap.ShowSave
    ExportFname = frmDialogs.ExportMap.FileName
    '
    Call UpdateDialog(frmDialogs.ExportMap)
    If (ExportFname <> "") Then
        PreSaveAll
        Ed.BeginSlowTask "Exporting map"
        Ed.ServerExec "MAP EXPORT FILE=" & Quotes(ExportFname)
        Ed.EndSlowTask
    End If
    '
Skip: Ed.ServerEnable
End Sub

Private Sub FloorLofter_Click()
    frmFloorLofter.Show
End Sub

Private Sub HelpButton_Click()
    frmMain.PopupMenu frmPopups.Help
End Sub

Private Sub HelpCam_Click()
    '
    ' Bring up camera help specific to the
    ' current editor mode.
    '
    Call Ed.Tools.Handlers(Ed.ToolMode).DoHelp(Ed.ToolMode, Ed)
    '
End Sub

Private Sub HelpIndex_Click()
    frmDialogs.HelpContents.HelpFile = App.Path + "\\help\\unrealed.hlp"
    frmDialogs.HelpContents.ShowHelp ' Run WinHelp
End Sub

Private Sub Holder_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = &H70& Then ' Intercept F1
       KeyCode = 0
       Call Ed.Tools.Handlers(Ed.MRUTool).DoHelp(Ed.MRUTool, Ed)
    End If
End Sub

Private Sub ImportLevel_Click()
    '
    ' Dialog for "Map Import"
    '
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.ImportMap.FileName = ""
    frmDialogs.ImportMap.ShowOpen 'Modal File-Open Box
    '
    Call UpdateDialog(frmDialogs.ImportMap)
    If (frmDialogs.ImportMap.FileName <> "") Then
        '
        frmImportMap.Show 1
        '
        If GResult Then
            Ed.BeginSlowTask "Importing map"
            If GImportExisting Then ' Import new map
                Ed.ServerExec "MAP IMPORTADD FILE=" & Quotes(frmDialogs.ImportMap.FileName)
            Else ' Add to existing map
                Ed.ServerExec "MAP IMPORT FILE=" & Quotes(frmDialogs.ImportMap.FileName)
            End If
            Ed.EndSlowTask
            PostLoad
        End If
    End If
Skip: Ed.ServerEnable
End Sub

Private Sub LoadBrush_Click()
    '
    ' Dialog for "Load Brush":
    '
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.BrushOpen.FileName = ""
    frmDialogs.BrushOpen.ShowOpen 'Modal Brush-Open Box
    '
    Call UpdateDialog(frmDialogs.BrushOpen)
    If (frmDialogs.BrushOpen.FileName <> "") Then
        '
        ' Load the brush
        '
        Call UpdateDialog(frmDialogs.BrushOpen)
        Ed.BrushFname = frmDialogs.BrushOpen.FileName
        Ed.ServerExec "BRUSH LOAD FILE=" & Quotes(Ed.BrushFname)
    End If
Skip: Ed.ServerEnable
End Sub

Private Sub Map320x200_Click()
    Ed.ServerExec "CAMERA SIZE XR=320 YR=200"
End Sub

Private Sub Map400x300_Click()
    Ed.ServerExec "CAMERA SIZE XR=400 YR=300"
End Sub

Private Sub Map480x360_Click()
    Ed.ServerExec "CAMERA SIZE XR=480 YR=360"
End Sub

Private Sub Map560x420_Click()
    Ed.ServerExec "CAMERA SIZE XR=560 YR=420"
End Sub

Private Sub Map640x480_Click()
    Ed.ServerExec "CAMERA SIZE XR=640 YR=480"
End Sub

Private Sub MapFlat_Click()
    Ed.ServerExec "CAMERA SET MODE=FLAT"
End Sub

Private Sub MapFlatNorms_Click()
    Ed.ServerExec "CAMERA SET MODE=FLATNORMS"
End Sub

Private Sub MapIllum_Click()
    Ed.ServerExec "CAMERA SET MODE=ILLUM"
End Sub

Private Sub MapLight_Click()
    Ed.ServerExec "CAMERA SET MODE=SHADE"
End Sub

Private Sub MapPersp_Click()
    Ed.ServerExec "CAMERA SET MODE=MAP3D"
End Sub

Private Sub MapTextures_Click()
    Ed.ServerExec "CAMERA SET MODE=TEXTURES"
End Sub

Private Sub MapXY_Click()
    Ed.ServerExec "CAMERA SET MODE=MAPXY"
End Sub

Private Sub MapXZ_Click()
    Ed.ServerExec "CAMERA SET MODE=MAPXZ"
End Sub

Private Sub MapYZ_Click()
    Ed.ServerExec "CAMERA SET MODE=MAPYZ"
End Sub

Private Sub MDIForm_Load()
    '
    Dim i
    Dim S As String, T As String
    Dim Temp As String
    Dim Highlight As Boolean
    '
    ' Inhibit registry reading if desired.
    '
    If InStr(UCase(Command$), "RESET") Then
        MsgBox "Your UnrealEd settings have been reset."
        NoReadRegistry = True
    End If
    '
    ' Init App object properties.
    '
    Call InitApp
    '
    ' Create global UnrealEdApp object.
    '
    Set Ed = New UnrealEdApp
    '
    ' Show startup screen
    '
    Ed.Startup = 1
    App.Title = Ed.EditorAppName
    frmMain.Caption = Ed.EditorAppName
    frmMain.Show
    '
    ' Launch UnrealServer.  There is only
    ' one of these per instance.
    '
    Ed.GetProfile
    Call Ed.InitServer(frmMain.hwnd, frmMain.Callback.hwnd)
    'If Ed.Licensed = 0 Then frmLicense.Show 1 ' Show license info
    '
    ' Set help file dirs:
    '
    frmDialogs.ToolHelp.HelpFile = App.HelpFile
    frmDialogs.RelNotes.HelpFile = App.HelpFile
    frmDialogs.HelpContents.HelpFile = App.HelpFile
    frmDialogs.About.HelpFile = App.HelpFile
    '
    ' Initialize tools
    '
    Call Ed.Tools.InitTools(Ed)
    '
    Ed.Startup = 0
    GInitialResized = 1
    '
    Ed.ServerExec "APP HIDE"
    '
    ' Set server parameters to the defaults the
    ' editor expects:
    '
    Ed.ServerExec "MAP GRID X=16 Y=16 Z=16 BASE=ABSOLUTE SHOW2D=ON SHOW3D=OFF"
    Ed.ServerExec "MAP ROTGRID PITCH=4 YAW=4 ROLL=4"
    Ed.ServerExec "MODE CAMERAMOVE GRID=ON ROTGRID=ON SNAPTOPIVOT=ON SNAPDIST=10"
    '
    ResizeAll (False)
    '
    ' Init and show toolbar (must be drawn after camera in order
    ' for palette to come out right):
    '
    InitToolbar
    
    Ed.StatusText "UnrealEd is ready to go"
    '
    Call Ed.RegisterBrowserTopic(frmTexBrowser, "Textures")
    Call Ed.RegisterBrowserTopic(frmClassBrowser, "Classes")
    Call Ed.RegisterBrowserTopic(frmSoundFXBrowser, "SoundFX")
    Call Ed.RegisterBrowserTopic(frmMusicBrowser, "Music")
    Call Ed.SetBrowserTopic(Ed.InitialBrowserTopic)
    
    PreferencesChange
    
    ' Enable registry reading.
    NoReadRegistry = False

    ' Load command-line level, if any.
    If GetString(Command$, "FILE=", Temp) Then
        If InStr(Temp, ":") = 0 And (Left(Temp, 1) <> "\") Then
            Temp = App.Path + "\" + Temp
        End If
        Ed.BeginSlowTask "Loading " & Temp
        Ed.ServerExec "MAP LOAD FILE=" & Quotes(Temp)
        Ed.EndSlowTask
        '
        Ed.MapFname = Temp
        Caption = Ed.EditorAppName + " - " + Ed.MapFname
        Ed.LoadParamsFromLevel
        ResizeAll (True)
    End If

End Sub

Private Sub MDIForm_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If (Button And 2) <> 0 Then ' Right click
        PopupMenu frmPopups.Window
    End If
End Sub

Private Sub MDIForm_Resize()
    ResizeAll (True)
End Sub

Private Sub MDIForm_Unload(Cancel As Integer)
    Dim N As Integer
    
    ' Unload browser.
    Ed.UnloadBrowser
   
    ' Unload all forms
    Dim i As Long
    For i = Forms.Count - 1 To 0 Step -1
        Unload Forms(i)
    Next
   
    ' Save profile now that all forms have
    ' called their EndOnTop's.
    Ed.SaveProfile
   
    ' End the program, in case any stray objects
    ' are still hanging around in memory.
    Ed.ExitServer
   End
End Sub

Private Sub MeshViewer_Click()
    frmMeshViewer.Show
End Sub

Private Sub New_Click()
    Ed.ServerDisable
    If MsgBox("Are you sure you want to create a new map?", _
        vbOKCancel) = vbOK Then

        ' New map.
        Ed.MapFname = ""
        frmMain.Caption = Ed.EditorAppName
        Ed.ServerExec "MAP NEW"
        PostLoad

    End If
    Ed.ServerEnable
End Sub

Private Sub ObjectProperties_Click()
    Ed.ServerExec "HOOK ACTORPROPERTIES" ''xyzzy
End Sub

Private Sub Open_Click()
    
    ' Dialog for "Open Map".
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.MapOpen.FileName = ""
    frmDialogs.MapOpen.ShowOpen
    
    Call UpdateDialog(frmDialogs.MapOpen)
    If (frmDialogs.MapOpen.FileName <> "") Then
        
        ' Load the map, inhibiting redraw since we're
        ' about to resize everything anyway.
        Ed.MapFname = frmDialogs.MapOpen.FileName
        Caption = Ed.EditorAppName + " - " + Ed.MapFname
        
        Ed.BeginSlowTask "Loading map"
        Ed.ServerExec "MAP LOAD FILE=" & _
            Quotes(Ed.MapFname) & " REDRAW=OFF"
        Ed.EndSlowTask
        
        Ed.LoadParamsFromLevel
        ResizeAll (True)
        PostLoad
    End If
Skip: Ed.ServerEnable
End Sub

Private Sub PanelHolder_Mousedown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        PopupMenu frmPopups.Panel
    End If
End Sub

Private Sub ParSolCone_Click()
    frmParSolCone.Show
End Sub

Private Sub ParSolHeightMap_Click()
    frmParSolHeightMap.Show
End Sub

Private Sub ParSolLinearStair_Click()
    frmParSolLinearStair.Show
End Sub

Private Sub ParSolRect_Click()
    frmParSolCube.Show
End Sub

Private Sub ParSolSphereDome_Click()
    frmParSolSphere.Show
End Sub

Private Sub ParSolSpiralStair_Click()
    frmParSolSpiralStair.Show
End Sub

Private Sub ParSolTube_Click()
    frmParSolTube.Show
End Sub

Private Sub PolygonProperties_Click()
   frmSurfaceProps.Show
End Sub

Private Sub PlayButton_Click()
    PlayLevel_Click
End Sub

Private Sub Preferences_Click()
    Ed.ServerExec "HOOK PREFERENCES" ''xyzzy
End Sub

Private Sub Project_Click()
    Ed.ServerExec "HOOK LEVELPROPERTIES" ''xyzzy
End Sub

Private Sub Rebuild_Click()
    frmRebuilder.Show ' Rebuild dialog
    frmRebuilder.SetFocus
End Sub

Private Sub EditRedo_Click()
    If hwndScript <> 0 Then
        ScriptForm.EditRedo_Click
    Else
        Ed.ServerExec "TRANSACTION REDO"
    End If
End Sub

Private Sub RelNotes_Click()
    frmDialogs.RelNotes.ShowHelp ' Run WinHelp
End Sub

Private Sub ResetAll_Click()
    Ed.ServerExec "BRUSH RESET"
End Sub

Private Sub ResetPosition_Click()
    Ed.ServerExec "BRUSH MOVETO X=0 Y=0 Z=0"
End Sub

Private Sub ResetRotation_Click()
    Ed.ServerExec "BRUSH ROTATETO PITCH=0 YAW=0 ROLL=0"
End Sub

Private Sub ResetScale_Click()
    Ed.ServerExec "BRUSH SCALE X=1 Y=1 Z=1 SHEER=0"
End Sub

Private Sub Save_Click()
    On Error GoTo Skip
    If Ed.MapFname = "" Then
        '
        ' Prompt for filename
        '
        Ed.ServerDisable
        frmDialogs.MapSaveAs.ShowSave 'Modal Save-As Box
        Ed.MapFname = frmDialogs.MapSaveAs.FileName
        Ed.ServerEnable
        Call UpdateDialog(frmDialogs.MapSaveAs)
    End If
    '
    If Ed.MapFname <> "" Then
        '
        ' Save the map
        '
        PreSaveAll
        Caption = Ed.EditorAppName + " - " + Ed.MapFname
        Ed.BeginSlowTask ("Saving map")
        Ed.SaveParamsToLevel
        Ed.ServerExec "MAP SAVE FILE=" & Quotes(Ed.MapFname)
        Ed.EndSlowTask
    End If
Skip: Ed.ServerEnable
End Sub

Private Sub SaveAs_Click()
    '
    ' Just set the default filename to empty and
    ' call Save_Click to do the normal "save" procedure.
    '
    Ed.MapFname = ""
    Save_Click
End Sub

Private Sub SaveBrush_Click()
    On Error GoTo Skip
    If Ed.BrushFname = "" Then
        '
        ' Prompt for filename
        '
        Ed.ServerDisable
        frmDialogs.BrushSave.ShowSave
        Ed.BrushFname = frmDialogs.BrushSave.FileName
        Call UpdateDialog(frmDialogs.BrushSave)
    End If
    '
    If Ed.BrushFname <> "" Then
        '
        ' Save the brush
        '
        Ed.ServerExec "BRUSH SAVE FILE=" & Quotes(Ed.BrushFname)
    End If
Skip: Ed.ServerEnable
End Sub

Private Sub SaveBrushAs_Click()
    '
    ' Just set the default filename to empty and
    ' call Save_Click to do the normal "save" procedure.
    '
    Ed.BrushFname = ""
    SaveBrush_Click
End Sub

Private Sub ScriptEditDefaults_Click()
    ScriptForm.ScriptEditDefaults_Click
End Sub

Public Sub ScriptMakeAll_Click()
    Ed.BeginSlowTask "Compiling all scripts"
    PreSaveAll
    Ed.ServerExec "SCRIPT MAKE ALL"
    Ed.EndSlowTask
    frmClassBrowser.ProcessResults
End Sub

Public Sub ScriptMakeChanged_Click()
    Ed.BeginSlowTask "Compiling changed scripts"
    PreSaveAll
    Ed.ServerExec "SCRIPT MAKE"
    Ed.EndSlowTask
    frmClassBrowser.ProcessResults
End Sub

Private Sub ScriptResults_Click()
    frmResults.UpdateResults
End Sub

Private Sub SelAdjFloors_Click()
    Ed.ServerExec "POLY SELECT ADJACENT FLOORS"
End Sub

Private Sub SelAdjSlants_Click()
    Ed.ServerExec "POLY SELECT ADJACENT SLANTS"
End Sub

Private Sub SelAdjWalls_Click()
    Ed.ServerExec "POLY SELECT ADJACENT WALLS"
End Sub

Private Sub SelAllAdj_Click()
    Ed.ServerExec "POLY SELECT ADJACENT ALL"
End Sub

Private Sub SelCoplAdj_Click()
    Ed.ServerExec "POLY SELECT ADJACENT COPLANARS"
End Sub

Private Sub SelectAll_Click()
    Ed.ServerExec "SELECT ALL"
End Sub

Private Sub SelectNone_Click()
    Ed.ServerExec "SELECT NONE"
End Sub

Private Sub SelIntersection_Click()
    Ed.ServerExec "POLY SELECT MEMORY INTERSECTION"
End Sub

Private Sub SelMatchBrush_Click()
    Ed.ServerExec "POLY SELECT MATCHING BRUSH"
End Sub

Private Sub SelMatchGroups_Click()
    Ed.ServerExec "POLY SELECT MATCHING GROUPS"
End Sub

Private Sub SelMatchItems_Click()
    Ed.ServerExec "POLY SELECT MATCHING ITEMS"
End Sub

Private Sub SelMatchTex_Click()
    Ed.ServerExec "POLY SELECT MATCHING TEXTURE"
End Sub

Private Sub SelMemorize_Click()
    Ed.ServerExec "POLY SELECT MEMORY SET"
End Sub

Private Sub SelRecall_Click()
    Ed.ServerExec "POLY SELECT MEMORY RECALL"
End Sub

Private Sub SelReverse_Click()
    Ed.ServerExec "POLY SELECT REVERSE"
End Sub

Private Sub SelUnion_Click()
    Ed.ServerExec "POLY SELECT MEMORY UNION"
End Sub

Private Sub SelXor_Click()
    Ed.ServerExec "POLY SELECT MEMORY XOR"
End Sub

Private Sub ShowBackdrop_Click()
    Ed.ServerExec "CAMERA SET BACKDROP=TOGGLE"
End Sub

Private Sub ShowBrush_Click()
    Ed.ServerExec "CAMERA SET BRUSH=TOGGLE"
End Sub

Private Sub ShowGrid_Click()
    Ed.ServerExec "CAMERA SET GRID=TOGGLE"
End Sub

Private Sub ShowOcclusion_Click()
    Ed.ServerExec "CAMERA SET OCCLUSION=TOGGLE"
End Sub

Private Sub SSPanel5_Click()

End Sub
Private Sub TexBrows_Click()
    Ed.BrowserPos = 0
    ResizeAll (True)
End Sub

Private Sub TexPalette_Click()
    Ed.BrowserPos = 0
    ResizeAll (False)
End Sub

Private Sub SurfaceProperties_Click()
   frmSurfaceProps.Show
End Sub

Private Sub Timer_Timer()
    Ed.ServerExec "MaybeAutoSave"
End Sub

Private Sub TWODEE_Click()
    frmTwoDee.Show
End Sub

Private Sub EditUndo_Click()
    If hwndScript <> 0 Then
        ScriptForm.EditUndo_Click
    Else
        Ed.ServerExec "TRANSACTION UNDO"
    End If
End Sub

Private Sub UnrealScriptHelp_Click()
    '!!
End Sub

Private Sub ValidateLevel_Click()
    Call frmResults.UpdateStatus("Validating level:")
    Ed.ServerExec "LEVEL VALIDATE"
    frmResults.UpdateResults
    frmResults.Results_DblClick
End Sub

Public Sub ViewLevelLinks_Click()
    Call frmResults.UpdateStatus("Level links:")
    Ed.ServerExec "LEVEL LINKS"
    frmResults.UpdateResults
    frmResults.Results_DblClick
End Sub

Private Sub WinBrowserHide_Click()
    Ed.BrowserPos = 2
    ResizeAll (True)
End Sub

Private Sub WinBrowserLeft_Click()
    Ed.BrowserPos = 1
    ResizeAll (True)
End Sub

Private Sub WinBrowserRight_Click()
    Ed.BrowserPos = 0
    ResizeAll (True)
End Sub

Private Sub WindowLog_Click()
    Ed.ServerExec "SHOWLOG"
End Sub

Private Sub WinToolbarLeft_Click()
    Ed.ToolbarPos = 0 ' left
    ResizeAll (True)
End Sub

Private Sub WinToolbarRight_Click()
    Ed.ToolbarPos = 1 ' right
    ResizeAll (True)
End Sub

'---------------------------------'
' All code related to the toolbar '
'---------------------------------'

Private Sub MDIForm_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = &H70& Then ' Intercept F1
       KeyCode = 0
       Call Ed.Tools.Handlers(Ed.MRUTool).DoHelp(Ed.MRUTool, Ed)
    End If
End Sub

Private Sub ResizeToolbar()
    Dim W As Integer, H As Integer, MaxH As Integer
    '
    ToolbarCount = ToolGridX * ToolGridY
    '
    W = (ToolGridX * 35 + 6) * Screen.TwipsPerPixelX
    H = (ToolGridY * 35 + 32) * Screen.TwipsPerPixelY
    MaxH = Toolbar.Height - 32 * Screen.TwipsPerPixelY
    '
    Holder.Top = 1 * Screen.TwipsPerPixelY
    Holder.Height = H + 5 * Screen.TwipsPerPixelY
    Holder.Width = W - 3 * Screen.TwipsPerPixelX
    '
    If (H > MaxH) Then ' Must use scrollbar
        If Ed.ToolbarPos = 0 Then ' Left
            Holder.Left = 2 * Screen.TwipsPerPixelX
            Scroller.Left = W - 2 * Screen.TwipsPerPixelX
        Else ' Right
            Holder.Left = Scroller.Width + 2 * Screen.TwipsPerPixelX
            Scroller.Left = 1 * Screen.TwipsPerPixelX
        End If
        Toolbar.Width = W + 13 * Screen.TwipsPerPixelX
        Scroller.Height = Toolbar.Height
        Scroller.Min = 0
        Scroller.Max = H - MaxH ' - 14 * Screen.TwipsPerPixelY
        Scroller.Value = Scroller.Min
        Scroller.LargeChange = MaxH
        Scroller.SmallChange = MaxH / 8
        Scroller.Visible = True
    Else ' No scrollbar, everything fits nicely
        Holder.Left = 2 * Screen.TwipsPerPixelX
        Toolbar.Width = W
        Scroller.Visible = False
    End If
    '
    StatusText.Top = Holder.Height - 33 * Screen.TwipsPerPixelY
    StatusText.Left = 2 * Screen.TwipsPerPixelX
    StatusText.Width = Holder.Width - 10 * Screen.TwipsPerPixelX
End Sub

Private Sub InitToolbar()
    Dim i, j, N, V As Integer
    Dim Highlight As Boolean
    Dim Temp As String
    '
    ' Init defaults
    '
    Ed.GridMode = 1
    Ed.RotGridMode = 1
    Ed.SpeedMode = 1
    Ed.SnapVertex = 1
    '
    ' Build grid
    '
    For N = 0 To ToolbarCount - 1
        '
        i = Int(N / 3)
        j = N Mod 3
        '
        If N <> 0 Then
            Load ToolIcons(N)
        End If
        '
        ToolIcons(N).Left = (j * 35) * Screen.TwipsPerPixelX
        ToolIcons(N).Top = (i * 35) * Screen.TwipsPerPixelY
        ToolIcons(N).Width = 35 * Screen.TwipsPerPixelX
        ToolIcons(N).Height = 35 * Screen.TwipsPerPixelY
        ToolIcons(N).GroupNumber = N
        '
    Next N
    '
    ' Set all tool names
    '
    ToolIcons(0).Tag = "CAMERAMOVE"
    ToolIcons(1).Tag = "CAMERAZOOM"
    '
    ToolIcons(3).Tag = "BRUSHROTATE"
    ToolIcons(4).Tag = "BRUSHSHEER"
    '
    ToolIcons(6).Tag = "BRUSHSCALE"
    ToolIcons(7).Tag = "BRUSHSTRETCH"
    '
    ToolIcons(9).Tag = "BRUSHSNAP"
    ToolIcons(10).Tag = "POLY SELECT ALL" 'MWP
    '
    ToolIcons(12).Tag = "ACTOR SELECT ALL"
    ToolIcons(13).Tag = "ACTOR SELECT INSIDE" 'MWP
    '
    ToolIcons(15).Tag = "SELECT NONE"
    ToolIcons(16).Tag = "ACTOR SELECT INVERT" 'MWP
    '
    ToolIcons(18).Tag = "ACTOR REPLACE BRUSH" 'MWP
    ToolIcons(19).Tag = "ACTOR REPLACE" 'MWP
    '
    ToolIcons(21).Tag = "TRANSACTION UNDO" 'MWP
    ToolIcons(22).Tag = "TRANSACTION REDO" 'MWP
    '
    ToolIcons(24).Tag = "TEXTUREPAN"
    ToolIcons(25).Tag = "TEXTUREROTATE"
    '
    ToolIcons(27).Tag = "ACTOR HIDE UNSELECTED" 'MWP
    ToolIcons(28).Tag = "ACTOR UNHIDE ALL" 'MWP
    '
    ToolIcons(30).Tag = "ACTOR HIDE SELECTED" 'MWP
    ToolIcons(31).Tag = "ACTOR CLIP Z" 'MWP
    'ToolIcons(31).Tag = "ACTOR CLIP XY" 'MWP:XXX
    '
    'ToolIcons(33).Tag = "ACTOR CLIP XYZ" 'MWP:XXX
    ToolIcons(34).Tag = "SNAPVERTEX"
    '
    ToolIcons(36).Tag = "HELP"
    ToolIcons(37).Tag = "SPEED"
    '
    ToolIcons(39).Tag = "GRID"
    ToolIcons(40).Tag = "ROTGRID"
    '
    ' Brush tools:
    '
    ToolIcons(2).Tag = "BRUSH ADD"
    ToolIcons(5).Tag = "BRUSH SUBTRACT"
    ToolIcons(8).Tag = "BRUSH FROM INTERSECTION"
    ToolIcons(11).Tag = "BRUSH FROM DEINTERSECTION"
    ToolIcons(14).Tag = "BRUSH ADD SPECIAL"
    ToolIcons(17).Tag = "BRUSH ADDMOVER"
    ToolIcons(20).Tag = "CUBE"
    ToolIcons(23).Tag = "SPHERE"
    ToolIcons(26).Tag = "CYLINDER"
    ToolIcons(29).Tag = "CONE"
    ToolIcons(32).Tag = "STAIR"
    ToolIcons(35).Tag = "SPIRAL"
    ToolIcons(38).Tag = "CURVEDSTAIR"
    ToolIcons(41).Tag = "SHEET"
    '
    GToolClicking = 1
    For N = 0 To ToolbarCount - 1
        '
        ' Get picture
        '
        Call Ed.Tools.GetPicture(ToolIcons(N).Tag, ToolIcons(N))
        '
        ' Set highlighting
        '
        Call Ed.Tools.Handlers(ToolIcons(N).Tag).GetStatus(ToolIcons(N).Tag, Ed, Temp, Highlight)
        Call Ed.Tools.Highlight(ToolIcons(N).Tag, Highlight)
        '
        ' Make visible
        '
        ToolIcons(N).Visible = True
    Next N
    StatusText.ZOrder
    GToolClicking = 0
    '
    ' Set initial mode to first tool, and show picture:
    '
    Call Ed.Tools.Handlers("CAMERAMOVE").DoClick("CAMERAMOVE", Ed)
    '
End Sub

Private Sub Scroller_Scroll()
    Holder.Top = -Scroller.Value
End Sub

Private Sub Scroller_Change()
    Scroller_Scroll
End Sub

Private Sub ToolIcons_Click(index As Integer, Value As Integer)
    Dim i As Integer
    Dim Tool As String
    '
    If GToolClicking = 0 Then
        GToolClicking = 1
        Tool = ToolIcons(index).Tag
        Call Ed.Tools.Handlers(Tool).DoClick(Tool, Ed)
        Ed.MRUTool = Tool
        GToolClicking = 0
    End If
    '
End Sub

Private Sub ToolIcons_MouseDown(index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
    '
    Dim i As Integer
    Dim Icon As Integer
    Dim Temp As String
    Dim Highlight As Integer
    '
    If (Button And 2) <> 0 Then ' Left click
        Set PopupToolControl = ToolIcons(index)
        PopupToolMoveable = False
        PopupToolIndex = index
        Call LeftClickTool(ToolIcons(index).Tag, frmMain)
    End If
    '
End Sub

'
' Resize the entire screen: toolbar, panel,
' and browser.
'
Public Sub ResizeAll(Reopen As Boolean)
    Dim MustExit As Integer
    
    If GInitialResized = 0 Then Exit Sub ' Just starting up
    If WindowState = 1 Then Exit Sub ' Minimized
    
    If ScaleWidth < 480 * Screen.TwipsPerPixelX Then
        Width = Width - ScaleWidth + 480 * Screen.TwipsPerPixelX
        MustExit = True
    End If
    If ScaleHeight < 280 * Screen.TwipsPerPixelY Then
        Height = Height - ScaleHeight + 280 * Screen.TwipsPerPixelY
        MustExit = True
    End If
    If MustExit Then Exit Sub
    
    GResizingAll = 1
    
    ' Set visibility and positions.
    frmCameraHolder.Visible = False
        
    If Ed.BrowserPos = 2 Then ' Hide.
        BrowserPanel.Visible = False
    Else
        If BrowserPanel.Visible = False Then
            BrowserPanel.Align = 0
        End If
        BrowserPanel.Visible = True ' Must do before align
        '
        If Ed.BrowserPos = 0 Then ' Right.
            BrowserPanel.Align = 4
        Else ' Left.
            BrowserPanel.Align = 3
        End If
        '
        If BrowserTopicCombo.Text <> "" Then
            Ed.ReloadBrowser
        End If
    End If
    
    If Ed.ToolbarPos = 0 Then ' Left
        Toolbar.Align = 3
    Else ' Right
        Toolbar.Align = 4
    End If
    
    Toolbar.Visible = True
    ResizeToolbar
    
    ' Camera holder.
    frmCameraHolder.SetPos
    frmCameraHolder.OpenCameras (Reopen)
    frmCameraHolder.Show

    ' Force all forms to be clipped to the newly-sized
    ' window and forced in front of cameras.
    Ed.NoteResize

End Sub

'
' UnrealEdServer callback dispatcher
'
Private Sub Callback_KeyPress(KeyAscii As Integer)
    Dim N As Integer
    Dim S As String
    Dim IsMover As Boolean
    Dim IsBrush As Boolean
    Dim Cur As String
    Dim Class As String
    
    Select Case KeyAscii - 32
    Case EDC_BROWSE:
        Class = Ed.ServerGetProp("OBJ", "BROWSECLASS")
        Select Case UCase(Class)
            Case "PALETTE":
                Ed.SetBrowserTopic ("Textures")
            Case "TEXTURE":
                Ed.SetBrowserTopic ("Textures")
            Case "CLASS":
                Ed.SetBrowserTopic ("Classes")
            Case "MUSIC":
                Ed.SetBrowserTopic ("Music")
            Case "SOUND":
                Ed.SetBrowserTopic ("SoundFX")
            Case "MESH":
                frmMeshViewer.Show
        End Select
    Case EDC_USECURRENT:
        Class = Ed.ServerGetProp("OBJ", "BROWSECLASS")
        Select Case UCase(Class)
            Case "PALETTE":
                Cur = Ed.ServerGetProp("TEXTURE", "PALETTE TEXTURE=" & Ed.GetBrowserCurrentItem("Textures"))
            Case "TEXTURE":
                Cur = Ed.GetBrowserCurrentItem("Textures")
            Case "CLASS":
                Cur = Ed.CurrentClass
            Case "SOUND":
                Cur = Ed.GetBrowserCurrentItem("SoundFX")
            Case "MUSIC":
                Cur = Ed.GetBrowserCurrentItem("Music")
            Case "MESH":
                Cur = frmMeshViewer.GetCurrent()
        End Select
        If Cur <> "" Then
            Call Ed.ServerSetProp("OBJ", "NOTECURRENT", "CLASS=" & Class & " OBJECT=" & Cur)
        End If
    Case EDC_CURTEXCHANGE:
        frmTexBrowser.BrowserRefresh
    Case EDC_SELCHANGE:
        If GPolyPropsAction = 1 Then
            frmSurfaceProps.GetSelectedPolys
        End If
    Case EDC_RTCLICKTEXTURE:
        frmTexBrowser.BrowserRefresh
        Call frmTexBrowser.TextureList_MouseDown(2, 0, 0, 0)
    Case EDC_RTCLICKPOLY:
        frmPopups2.prProperties.Caption = "Surface &Properties (" & Ed.ServerGetProp("Polys", "NumSelected") & " selected)..."
        frmPopups2.prApplyTex.Caption = "Apply &Texture " & frmTexBrowser.GetCurrent()
        frmPopups2.AddClassHere.Visible = Ed.CurrentClass <> "Light"
        frmPopups2.AddClassHere.Caption = "&Add " & Ed.CurrentClass & " here"
        PopupMenu frmPopups2.PolyRtClick
    Case EDC_RTCLICKACTOR:
        N = Val(Ed.ServerGetProp("Actor", "NumSelected"))
        GPopupActorClass = Ed.ServerGetProp("Actor", "ClassSelected")
        
        IsBrush = Ed.ServerGetProp("Actor", "ISKINDOF CLASS=BRUSH") <> 0
        frmPopups2.Mirror.Visible = IsBrush
        frmPopups2.Order.Visible = IsBrush
        frmPopups2.CopyPolygons.Visible = IsBrush
        frmPopups2.Solidity.Visible = IsBrush
        frmPopups2.SelectBrushes.Visible = IsBrush

        IsMover = Ed.ServerGetProp("Actor", "ISKINDOF CLASS=MOVER") <> 0
        frmPopups2.arMoverKeyframe.Visible = IsMover
        
        If GPopupActorClass <> "" Then
            frmPopups2.arProps.Caption = GPopupActorClass & " &Properties (" & Trim(Str(N)) & " selected)..."
            frmPopups2.arSelectAllOfType.Caption = "Select all " & GPopupActorClass & " actors"
            frmPopups2.arSelectAllOfType.Visible = True
            frmPopups2.arScriptEdit.Visible = True
        Else
            frmPopups2.arProps.Caption = "Actor &Properties (" & Trim(Str(N)) & " selected)..."
            frmPopups2.arSelectAllOfType.Visible = False
            frmPopups2.arScriptEdit.Visible = False
        End If

        PopupMenu frmPopups2.ActorRtClick
    Case EDC_RTCLICKWINDOWCANADD:
        frmPopups.AddClassHere.Visible = True
        frmPopups.AddClassHere.Visible = Ed.CurrentClass <> "Light"
        frmPopups.AddClassHere.Caption = "&Add " & Ed.CurrentClass & " here"
        frmPopups.AddLightHere.Visible = True
        frmPopups.AddDivider.Visible = True
        frmPopups.Pivot.Visible = True
        PopupMenu frmPopups.Window
    Case EDC_RTCLICKWINDOW:
        frmPopups.AddClassHere.Visible = False
        frmPopups.AddLightHere.Visible = False
        frmPopups.AddDivider.Visible = False
        frmPopups.Pivot.Visible = False
        PopupMenu frmPopups.Window
    End Select
End Sub

Public Sub PreferencesChange()
End Sub

'
' Play level.
'

Private Sub PlayLevel_Click()
    PreSaveAll
    Ed.ServerExec "HOOK PLAYMAP"
End Sub

