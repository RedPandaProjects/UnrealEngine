VERSION 5.00
Begin VB.Form frmPopups2 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "frmMorePopups"
   ClientHeight    =   45
   ClientLeft      =   5460
   ClientTop       =   3390
   ClientWidth     =   3390
   Icon            =   "Popups2.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   45
   ScaleWidth      =   3390
   Begin VB.Menu ClassBrowser 
      Caption         =   "ClassBrowser"
      Begin VB.Menu ClassEditScript 
         Caption         =   "&Edit ... Script..."
      End
      Begin VB.Menu ClassEditActor 
         Caption         =   "Default &Properties..."
      End
      Begin VB.Menu ClassCreateNew 
         Caption         =   "&Create New Class Below..."
      End
      Begin VB.Menu cbDelete 
         Caption         =   "&Delete This Class"
      End
      Begin VB.Menu ZZOATS 
         Caption         =   "-"
      End
      Begin VB.Menu ClassLoad 
         Caption         =   "&Load Classes..."
      End
      Begin VB.Menu ClassSave 
         Caption         =   "&Save Classes..."
      End
   End
   Begin VB.Menu ActorRtClick 
      Caption         =   "ActorRtClick"
      Begin VB.Menu arProps 
         Caption         =   "Actor &Properties (3 selected)"
      End
      Begin VB.Menu arMoverKeyframe 
         Caption         =   "Mover &Keyframe"
         Begin VB.Menu arKey0 
            Caption         =   "Key &0 (Base)"
         End
         Begin VB.Menu arKey1 
            Caption         =   "Key &1"
         End
         Begin VB.Menu arKey2 
            Caption         =   "Key &2"
         End
         Begin VB.Menu arKey3 
            Caption         =   "Key &3"
         End
         Begin VB.Menu arKey4 
            Caption         =   "Key &4"
         End
         Begin VB.Menu arKey5 
            Caption         =   "Key &5"
         End
         Begin VB.Menu arKey6 
            Caption         =   "Key &6"
         End
         Begin VB.Menu arKey7 
            Caption         =   "Key &7"
         End
      End
      Begin VB.Menu ResetActor 
         Caption         =   "&Reset"
         Begin VB.Menu MoveToOrigin 
            Caption         =   "&Move to origin"
         End
         Begin VB.Menu ResetRotation 
            Caption         =   "Reset &rotation"
         End
         Begin VB.Menu ResetScaling 
            Caption         =   "Reset &scaling"
         End
         Begin VB.Menu ResetAllPositioning 
            Caption         =   "Reset &all positioning"
         End
      End
      Begin VB.Menu Mirror 
         Caption         =   "&Transform"
         Begin VB.Menu MirrorX 
            Caption         =   "Mirror about &X"
         End
         Begin VB.Menu MirrorY 
            Caption         =   "Mirror about &Y"
         End
         Begin VB.Menu MirrorZ 
            Caption         =   "Mirror about &Z (vertical)"
         End
         Begin VB.Menu Perm 
            Caption         =   "&Transform permanently"
         End
      End
      Begin VB.Menu Order 
         Caption         =   "&Order"
         Begin VB.Menu ToFirst 
            Caption         =   "To &first"
         End
         Begin VB.Menu ToLast 
            Caption         =   "To &last"
         End
      End
      Begin VB.Menu CopyPolygons 
         Caption         =   "&Copy polygons"
         Begin VB.Menu ToBrush 
            Caption         =   "&To brush"
         End
         Begin VB.Menu FromBrush 
            Caption         =   "&From brush"
         End
      End
      Begin VB.Menu Solidity 
         Caption         =   "&Solidity"
         Begin VB.Menu MakeSolid 
            Caption         =   "Make Solid"
         End
         Begin VB.Menu MakeSemisolid 
            Caption         =   "Make Semisolid"
         End
         Begin VB.Menu MakeNonsolid 
            Caption         =   "Make Nonsolid"
         End
      End
      Begin VB.Menu ZACTA 
         Caption         =   "-"
      End
      Begin VB.Menu arSelectAllOfType 
         Caption         =   "&Select all <type> actors"
      End
      Begin VB.Menu arSelAll 
         Caption         =   "Select &all"
      End
      Begin VB.Menu arSelectNone 
         Caption         =   "Select &none"
      End
      Begin VB.Menu SelectBrushes 
         Caption         =   "Select &brushes"
         Begin VB.Menu SelectAdds 
            Caption         =   "&Adds"
         End
         Begin VB.Menu SelectSubtracts 
            Caption         =   "&Subtracts"
         End
         Begin VB.Menu SelectSemisolids 
            Caption         =   "S&emisolids"
         End
         Begin VB.Menu SelectNonsolids 
            Caption         =   "&Nonsolids"
         End
      End
      Begin VB.Menu ZARA 
         Caption         =   "-"
      End
      Begin VB.Menu arDuplicate 
         Caption         =   "D&uplicate"
      End
      Begin VB.Menu arDelete 
         Caption         =   "&Delete"
      End
      Begin VB.Menu arScriptEdit 
         Caption         =   "&Edit script"
      End
   End
   Begin VB.Menu PolyRtClick 
      Caption         =   "PolyRtClick"
      Begin VB.Menu prProperties 
         Caption         =   "&Surface Properties (1 Selected)..."
      End
      Begin VB.Menu MXUJE 
         Caption         =   "-"
      End
      Begin VB.Menu AddClassHere 
         Caption         =   "&Add <class> here"
      End
      Begin VB.Menu AddLightHere 
         Caption         =   "Add &Light here"
      End
      Begin VB.Menu ZORROW 
         Caption         =   "-"
      End
      Begin VB.Menu prAlignSelected 
         Caption         =   "&Align Selected"
         Begin VB.Menu paFloorCeiling 
            Caption         =   "Align As &Floor/Ceiling"
         End
         Begin VB.Menu paOneTile 
            Caption         =   "Align &One Tile"
         End
         Begin VB.Menu ZALMUD 
            Caption         =   "-"
         End
         Begin VB.Menu paWallDirection 
            Caption         =   "Align Wall &Direction"
         End
         Begin VB.Menu paWallPanning 
            Caption         =   "Align Wall &Panning"
         End
         Begin VB.Menu ZOWKA 
            Caption         =   "-"
         End
         Begin VB.Menu paUnalign 
            Caption         =   "&Unalign back to default"
         End
      End
      Begin VB.Menu SurfReset 
         Caption         =   "&Reset"
      End
      Begin VB.Menu ZFG 
         Caption         =   "-"
      End
      Begin VB.Menu prSelect 
         Caption         =   "&Select"
         Begin VB.Menu psMatchingGroups 
            Caption         =   "Matching &Groups (Shift-G)"
         End
         Begin VB.Menu psMatchingItems 
            Caption         =   "Matching &Items (Shift-I)"
         End
         Begin VB.Menu psMatchingBrush 
            Caption         =   "Matching &Brush (Shift-B)"
         End
         Begin VB.Menu psMatchingTexture 
            Caption         =   "Matching &Texture (Shift-T)"
         End
         Begin VB.Menu ZPSAQA 
            Caption         =   "-"
         End
         Begin VB.Menu psAllAdjacents 
            Caption         =   "All Ad&jacents (Shift-J)"
         End
         Begin VB.Menu psAdjacentCoplanars 
            Caption         =   "Adjacent &Coplanars (Shift-C)"
         End
         Begin VB.Menu psAdjacentWalls 
            Caption         =   "Adjacent &Walls (Shift-W)"
         End
         Begin VB.Menu psAdjacentFloorCeils 
            Caption         =   "Adjacent &Floors/Ceils (Shift-F)"
         End
         Begin VB.Menu psAdjacentSlants 
            Caption         =   "Adjacent &Slants (Shift-S)"
         End
         Begin VB.Menu ZPSOUGH 
            Caption         =   "-"
         End
         Begin VB.Menu psReverse 
            Caption         =   "Reverse (Shift-Q)"
         End
         Begin VB.Menu PSQWER 
            Caption         =   "-"
         End
         Begin VB.Menu psMemorizeSet 
            Caption         =   "Memorize Set (Shift-M)"
         End
         Begin VB.Menu psRecallMemory 
            Caption         =   "&Recall Memory (Shift-R)"
         End
         Begin VB.Menu psOrMemory 
            Caption         =   "&Or with memory (Shift-O)"
         End
         Begin VB.Menu psAndMemory 
            Caption         =   "&And with memory (Shift-U)"
         End
         Begin VB.Menu psXorMemory 
            Caption         =   "&Xor with memory (Shift-X)"
         End
      End
      Begin VB.Menu poSelectAll 
         Caption         =   "Select &all"
      End
      Begin VB.Menu prSelectNone 
         Caption         =   "Select &none"
      End
      Begin VB.Menu ZOUST 
         Caption         =   "-"
      End
      Begin VB.Menu prApplyTex 
         Caption         =   "Apply &Texture"
      End
   End
End
Attribute VB_Name = "frmPopups2"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub AddClassHere_Click()
    Ed.ServerExec "ACTOR ADD CLASS=" & Ed.CurrentClass
    Ed.ServerExec "POLY SELECT NONE"
End Sub

Private Sub AddLightHere_Click()
    Ed.ServerExec "ACTOR ADD CLASS=LIGHT"
    Ed.ServerExec "POLY SELECT NONE"
End Sub

Private Sub arKey4_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=4"
End Sub

Private Sub arKey5_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=5"
End Sub

Private Sub arKey6_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=6"
End Sub

Private Sub arKey7_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=7"
End Sub

Private Sub arSelAll_Click()
    Ed.ServerExec "ACTOR SELECT ALL"
End Sub

Private Sub arSelectNone_Click()
    Ed.ServerExec "SELECT NONE"
End Sub

'
' ClassBrowser
'

Private Sub ClassEditScript_Click()
    frmClassBrowser.EditScript_Click
End Sub

Private Sub ClassCreateNew_Click()
    frmClassBrowser.NewClass_Click
End Sub

Private Sub ClassEditActor_Click()
    frmClassBrowser.EditDefActor_Click
End Sub

Private Sub ClassLoad_Click()
    frmClassBrowser.LoadClass_Click
End Sub

Private Sub ClassSave_Click()
    frmClassBrowser.SaveClass_Click
End Sub

Private Sub cbDelete_Click()
    frmClassBrowser.Delete_Click
End Sub

'
' ActorRtClick
'

Private Sub arProps_Click()
    Ed.ServerExec "HOOK ACTORPROPERTIES" ''xyzzy
End Sub

Private Sub arCeiling_Click()
    Ed.ServerExec "ACTOR MOVETO CEILING"
End Sub

Private Sub arDelete_Click()
    Ed.ServerExec "ACTOR DELETE"
End Sub

Private Sub arDuplicate_Click()
    Ed.ServerExec "ACTOR DUPLICATE"
End Sub

Private Sub arSelectAllOfType_Click()
    Ed.ServerExec "ACTOR SELECT OFCLASS CLASS=" & GPopupActorClass
End Sub

Private Sub arKey0_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=0"
End Sub

Private Sub arKey1_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=1"
End Sub

Private Sub arKey2_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=2"
End Sub

Private Sub arKey3_Click()
    Ed.ServerExec "ACTOR KEYFRAME NUM=3"
End Sub

Private Sub arScriptEdit_Click()
    Call frmClassBrowser.LaunchScriptEd(GPopupActorClass, "", 0, 0, "")
End Sub

Private Sub arScriptSubclass_Click()
    Call frmClassBrowser.MakeSubclass(GPopupActorClass, False)
    If GResult = 1 Then
        Ed.ServerExec "ACTOR SUBCLASS NEWCLASS=" & Quotes(GString)
    End If
End Sub

Private Sub FromBrush_Click()
    Ed.ServerExec "MAP BRUSH PUT"
End Sub

Private Sub MakeNonsolid_Click()
    Ed.ServerExec "MAP SETBRUSH" & _
        " CLEARFLAGS=" & Trim(Str(PF_SEMISOLID + PF_NOTSOLID)) & _
        " SETFLAGS=" & Trim(Str(PF_NOTSOLID))
End Sub

Private Sub MakeSemisolid_Click()
    Ed.ServerExec "MAP SETBRUSH" & _
        " CLEARFLAGS=" & Trim(Str(PF_SEMISOLID + PF_NOTSOLID)) & _
        " SETFLAGS=" & Trim(Str(PF_SEMISOLID))
End Sub

Private Sub MakeSolid_Click()
    Ed.ServerExec "MAP SETBRUSH" & _
        " CLEARFLAGS=" & Trim(Str(PF_SEMISOLID + PF_NOTSOLID)) & _
        " SETFLAGS=" & Trim(Str(0))
End Sub

Private Sub MirrorX_Click()
    Ed.ServerExec "ACTOR MIRROR X=-1"
End Sub

Private Sub MirrorY_Click()
    Ed.ServerExec "ACTOR MIRROR Y=-1"
End Sub

Private Sub MirrorZ_Click()
    Ed.ServerExec "ACTOR MIRROR Z=-1"
End Sub

Private Sub MoveToOrigin_Click()
    Ed.ServerExec "ACTOR RESET LOCATION"
End Sub

Private Sub Perm_Click()
    Ed.ServerExec "BRUSH APPLYTRANSFORM"
End Sub

Private Sub poSelectAll_Click()
    Ed.ServerExec "POLY SELECT ALL"
End Sub

'
' PolyRtClick
'

Private Sub prProperties_Click()
    frmSurfaceProps.Show
End Sub

Private Sub prSelect_Click()
    '
End Sub

Private Sub prSelectNone_Click()
    Ed.ServerExec "SELECT NONE"
End Sub

Private Sub prAlignSelected_Click()
    '
End Sub

Private Sub prApplyTex_Click()
    Ed.ServerExec "POLY SETTEXTURE"
End Sub

Private Sub prGrabTex_Click()
    '
End Sub

'
' PolyRtClick: Alignment
'

Private Sub paFloorCeiling_Click()
    Ed.ServerExec "POLY TEXALIGN FLOOR"
End Sub

Private Sub paOneTile_Click()
    Ed.ServerExec "POLY TEXALIGN ONETILE"
End Sub

Private Sub paUnalign_Click()
    Ed.ServerExec "POLY TEXALIGN DEFAULT"
End Sub

Private Sub paWallDirection_Click()
    Ed.ServerExec "POLY TEXALIGN WALLDIR"
End Sub

Private Sub paWallPanning_Click()
    Ed.ServerExec "POLY TEXALIGN WALLPAN"
End Sub


'
' PolyRtClick: Selection
'

Private Sub psAdjacentCoplanars_Click()
    Ed.ServerExec "POLY SELECT ADJACENT COPLANARS"
End Sub

Private Sub psAdjacentFloorCeils_Click()
    Ed.ServerExec "POLY SELECT ADJACENT FLOORS"
End Sub

Private Sub psAdjacentSlants_Click()
    Ed.ServerExec "POLY SELECT ADJACENT SLANTS"
End Sub

Private Sub psAdjacentWalls_Click()
    Ed.ServerExec "POLY SELECT ADJACENT WALLS"
End Sub

Private Sub psAllAdjacents_Click()
    Ed.ServerExec "POLY SELECT ADJACENT ALL"
End Sub

Private Sub psAndMemory_Click()
    Ed.ServerExec "POLY SELECT MEMORY UNION"
End Sub

Private Sub psMatchingBrush_Click()
    Ed.ServerExec "POLY SELECT MATCHING BRUSH"
End Sub

Private Sub psMatchingGroups_Click()
    Ed.ServerExec "POLY SELECT MATCHING GROUPS"
End Sub

Private Sub psMatchingItems_Click()
    Ed.ServerExec "POLY SELECT MATCHING ITEMS"
End Sub

Private Sub psMatchingTexture_Click()
    Ed.ServerExec "POLY SELECT MATCHING TEXTURE"
End Sub

Private Sub psMemorizeSet_Click()
    Ed.ServerExec "POLY SELECT MEMORY SET"
End Sub

Private Sub psOrMemory_Click()
    Ed.ServerExec "POLY SELECT MEMORY INTERSECTION"
End Sub

Private Sub psRecallMemory_Click()
    Ed.ServerExec "POLY SELECT MEMORY RECALL"
End Sub

Private Sub psReverse_Click()
    Ed.ServerExec "POLY SELECT REVERSE"
End Sub

Private Sub psXorMemory_Click()
    Ed.ServerExec "POLY SELECT MEMORY XOR"
End Sub

Private Sub ResetAllPositioning_Click()
    Ed.ServerExec "ACTOR RESET ALL"
End Sub

Private Sub ResetRotation_Click()
    Ed.ServerExec "ACTOR RESET ROTATION"
End Sub

Private Sub ResetScaling_Click()
    Ed.ServerExec "ACTOR RESET SCALE"
End Sub

Private Sub SelectAdds_Click()
    Ed.ServerExec "MAP SELECT ADDS"
End Sub

Private Sub SelectNonsolids_Click()
    Ed.ServerExec "MAP SELECT NONSOLIDS"
End Sub

Private Sub SelectSemisolids_Click()
    Ed.ServerExec "MAP SELECT SEMISOLIDS"
End Sub

Private Sub SelectSubtracts_Click()
    Ed.ServerExec "MAP SELECT SUBTRACTS"
End Sub

Private Sub SurfReset_Click()
    Ed.ServerExec "POLY TEXPAN RESET"
    Ed.ServerExec "POLY TEXSCALE"
    Ed.ServerExec "POLY TEXALIGN DEFAULT"
End Sub

Private Sub ToBrush_Click()
    Ed.ServerExec "MAP BRUSH GET"
End Sub

Private Sub ToFirst_Click()
    Ed.ServerExec "MAP SENDTO FIRST"
End Sub

Private Sub ToLast_Click()
    Ed.ServerExec "MAP SENDTO LAST"
End Sub
