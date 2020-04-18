VERSION 5.00
Begin VB.Form frmPopups 
   Caption         =   "frmPopups"
   ClientHeight    =   3540
   ClientLeft      =   60
   ClientTop       =   0
   ClientWidth     =   7485
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   204
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   Icon            =   "Popups.frx":0000
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3540
   ScaleWidth      =   7485
   ShowInTaskbar   =   0   'False
   Begin VB.Menu Toolbar 
      Caption         =   "Toolbar"
      Begin VB.Menu ToolbarProperties 
         Caption         =   "&Properties..."
      End
      Begin VB.Menu ToolbarHelp 
         Caption         =   "&Help"
      End
      Begin VB.Menu MoveToolbar 
         Caption         =   "&Move toolbar"
      End
      Begin VB.Menu ZJIWZA 
         Caption         =   "-"
      End
      Begin VB.Menu ToolbarDo 
         Caption         =   "Do it"
      End
   End
   Begin VB.Menu Window 
      Caption         =   "Window"
      Begin VB.Menu AddClassHere 
         Caption         =   "&Add <class> here"
      End
      Begin VB.Menu AddLightHere 
         Caption         =   "&Add Light here"
      End
      Begin VB.Menu AddDivider 
         Caption         =   "-"
      End
      Begin VB.Menu Grid 
         Caption         =   "&Grid"
         Begin VB.Menu Grid1 
            Caption         =   "1 unit"
         End
         Begin VB.Menu Grid2 
            Caption         =   "2 units"
         End
         Begin VB.Menu Grid4 
            Caption         =   "4 units"
         End
         Begin VB.Menu Grid8 
            Caption         =   "8 units"
         End
         Begin VB.Menu Grid16 
            Caption         =   "16 units"
         End
         Begin VB.Menu Grid32 
            Caption         =   "32 units"
         End
         Begin VB.Menu Grid64 
            Caption         =   "64 units"
         End
         Begin VB.Menu Grid128 
            Caption         =   "128 units"
         End
         Begin VB.Menu Grid256 
            Caption         =   "256 units"
         End
         Begin VB.Menu GridDisabled 
            Caption         =   "Disabled"
         End
         Begin VB.Menu GridCustom 
            Caption         =   "Custom..."
         End
      End
      Begin VB.Menu Pivot 
         Caption         =   "&Pivot"
         Begin VB.Menu PivotSnapped 
            Caption         =   "Place pivot &snapped here"
         End
         Begin VB.Menu PivotHere 
            Caption         =   "Place pivot &here"
         End
      End
      Begin VB.Menu GridDivider 
         Caption         =   "-"
      End
      Begin VB.Menu ActProp 
         Caption         =   "&Actor Properties"
         Shortcut        =   {F4}
      End
      Begin VB.Menu TexProp 
         Caption         =   "&Surface Properties"
         Shortcut        =   {F5}
      End
      Begin VB.Menu Rebuilder 
         Caption         =   "&Rebuilder"
         Shortcut        =   {F8}
      End
   End
   Begin VB.Menu Help 
      Caption         =   "&Help"
      Begin VB.Menu HelpTopics 
         Caption         =   "&Help Topics"
         Shortcut        =   {F1}
      End
      Begin VB.Menu HelpCameras 
         Caption         =   "Help Using &Cameras"
         Shortcut        =   {F2}
      End
      Begin VB.Menu ZOAOE 
         Caption         =   "-"
      End
      Begin VB.Menu ReleaseNotes 
         Caption         =   "&Release Notes"
      End
      Begin VB.Menu ZOGMATISM 
         Caption         =   "-"
      End
      Begin VB.Menu About 
         Caption         =   "&About UnrealEd"
      End
   End
   Begin VB.Menu Panel 
      Caption         =   "Panel"
      Begin VB.Menu PanelBottom 
         Caption         =   "Panel on &Bottom"
      End
      Begin VB.Menu PanelTop 
         Caption         =   "Panel on &Top"
      End
      Begin VB.Menu ShowHidePanel 
         Caption         =   "&Hide Panel"
      End
   End
   Begin VB.Menu Browser 
      Caption         =   "&Browser"
      Begin VB.Menu BrowserRight 
         Caption         =   "Browser on &Right"
      End
      Begin VB.Menu BrowserLeft 
         Caption         =   "Browser on &Left"
      End
      Begin VB.Menu BrowserHide 
         Caption         =   "&Hide Browser"
      End
   End
   Begin VB.Menu TexBrowser 
      Caption         =   "TexBrowser"
      Begin VB.Menu TBProperties 
         Caption         =   "&Properties..."
      End
      Begin VB.Menu TBApply 
         Caption         =   "&Apply"
      End
      Begin VB.Menu ZTBXS 
         Caption         =   "-"
      End
      Begin VB.Menu TBExport 
         Caption         =   "Ex&port"
      End
      Begin VB.Menu TBDelete 
         Caption         =   "&Delete"
      End
   End
End
Attribute VB_Name = "frmPopups"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub About_Click()
    frmDialogs.About.ShowHelp
End Sub

Private Sub ActProp_Click()
    Ed.ServerExec "HOOK ACTORPROPERTIES" ''xyzzy
End Sub

Private Sub AddClassHere_Click()
    Ed.ServerExec "ACTOR ADD CLASS=" & Ed.CurrentClass
End Sub

Private Sub AddLightHere_Click()
    Ed.ServerExec "ACTOR ADD CLASS=LIGHT"
End Sub

Private Sub BrowserHide_Click()
    Ed.BrowserPos = 2
    frmMain.ResizeAll (True)
End Sub

Private Sub BrowserLeft_Click()
    Ed.BrowserPos = 1
    frmMain.ResizeAll (True)
End Sub

Private Sub BrowserRight_Click()
    Ed.BrowserPos = 0
    frmMain.ResizeAll (True)
End Sub

Private Sub ClassBro_Click()
    Ed.SetBrowserTopic ("Classes")
End Sub

Private Sub Grid1_Click()
    Call Ed.SetGridSize(1, 1, 1)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid128_Click()
    Call Ed.SetGridSize(128, 128, 128)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid16_Click()
    Call Ed.SetGridSize(16, 16, 16)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid2_Click()
    Call Ed.SetGridSize(2, 2, 2)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid256_Click()
    Call Ed.SetGridSize(256, 256, 256)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid32_Click()
    Call Ed.SetGridSize(32, 32, 32)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid4_Click()
    Call Ed.SetGridSize(4, 4, 4)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid64_Click()
    Call Ed.SetGridSize(64, 64, 64)
    Call Ed.SetGridMode(1)
End Sub

Private Sub Grid8_Click()
    Call Ed.SetGridSize(8, 8, 8)
    Call Ed.SetGridMode(1)
End Sub

Private Sub GridDisabled_Click()
    Call Ed.SetGridMode(0)
End Sub

Private Sub HelpCameras_Click()
    Call Ed.Tools.Handlers(Ed.ToolMode).DoHelp(Ed.ToolMode, Ed)
End Sub

Private Sub HelpTopics_Click()
    frmDialogs.HelpContents.HelpFile = App.Path + "\\help\\unrealed.hlp"
    frmDialogs.HelpContents.ShowHelp
End Sub

Private Sub MirrorX_Click()
    Ed.ServerExec "BRUSH MIRROR X"
End Sub

Private Sub MirrorY_Click()
    Ed.ServerExec "BRUSH MIRROR Y"
End Sub

Private Sub MirrorZ_Click()
    Ed.ServerExec "BRUSH MIRROR Z"
End Sub

Private Sub MoveToolbar_Click()
    Ed.ToolbarPos = 1 - Ed.ToolbarPos
    frmMain.ResizeAll (True)
End Sub

Private Sub PivotHere_Click()
    Ed.ServerExec "PIVOT HERE"
End Sub

Private Sub PivotSnapped_Click()
    Ed.ServerExec "PIVOT SNAPPED"
End Sub

Private Sub Rebuilder_Click()
    frmRebuilder.Show ' Rebuild dialog
    frmRebuilder.SetFocus
End Sub

Private Sub ReleaseNotes_Click()
    frmDialogs.RelNotes.ShowHelp
End Sub

Private Sub ResetBrushAll_Click()
    Ed.ServerExec "BRUSH RESET"
End Sub

Private Sub ResetBrushPosition_Click()
    Ed.ServerExec "BRUSH MOVETO X=0 Y=0 Z=0"
End Sub

Private Sub ResetBrushRotation_Click()
    Ed.ServerExec "BRUSH ROTATETO PITCH=0 YAW=0 ROLL=0"
End Sub

Private Sub ResetBrushScale_Click()
    Ed.ServerExec "BRUSH SCALE RESET"
End Sub

Private Sub TexBro_Click()
    Ed.SetBrowserTopic ("Textures")
End Sub

Private Sub TexProp_Click()
   frmSurfaceProps.Show
End Sub

Private Sub ToolbarDo_Click()
    Call Ed.Tools.Handlers(PopupToolName).DoClick(PopupToolName, Ed)
End Sub

Private Sub ToolbarHelp_Click()
    Call Ed.Tools.Handlers(PopupToolName).DoHelp(PopupToolName, Ed)
End Sub

Private Sub ToolbarProperties_Click()
    Call Ed.Tools.Handlers(PopupToolName).DoProperties(PopupToolName, Ed)
End Sub

Private Sub TWODEE_Click()
    frmTwoDee.Show
End Sub

'
' TexBrowser
'

Private Sub TBApply_Click()
    frmTexBrowser.BroApply_Click
End Sub

Private Sub TBDelete_Click()
    frmTexBrowser.BroDelete_Click
End Sub

Private Sub TBExport_Click()
    frmTexBrowser.BroExport_Click
End Sub

Private Sub TBProperties_Click()
    frmTexBrowser.BroEdit_Click
End Sub
