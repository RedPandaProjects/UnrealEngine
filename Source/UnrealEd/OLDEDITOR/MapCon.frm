VERSION 5.00
Begin VB.Form frmMapToolbar 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Map Toolbar"
   ClientHeight    =   3345
   ClientLeft      =   3030
   ClientTop       =   9780
   ClientWidth     =   5025
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
   HelpContextID   =   114
   Icon            =   "MapCon.frx":0000
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3345
   ScaleWidth      =   5025
   ShowInTaskbar   =   0   'False
   Begin VB.Frame Frame8 
      Caption         =   "Selected Brushes"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   3135
      Left            =   3240
      TabIndex        =   16
      Top             =   120
      Width           =   1695
      Begin VB.Frame Frame5 
         Caption         =   "Solidity"
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
         Left            =   120
         TabIndex        =   19
         Top             =   360
         Width           =   1455
         Begin VB.CommandButton SetStat 
            Caption         =   "Set"
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
            TabIndex        =   23
            Tag             =   "Set type of selected brushes"
            Top             =   240
            Width           =   495
         End
         Begin VB.OptionButton NonSolid 
            Caption         =   "Non-solid"
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
            Top             =   720
            Width           =   1095
         End
         Begin VB.OptionButton SemiSolid 
            Caption         =   "Semi-solid"
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
            Top             =   480
            Width           =   1095
         End
         Begin VB.OptionButton Solid 
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
            Left            =   120
            TabIndex        =   20
            Top             =   240
            Value           =   -1  'True
            Width           =   975
         End
      End
      Begin VB.TextBox GrpName 
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
         TabIndex        =   18
         Top             =   1920
         Width           =   1455
      End
      Begin VB.CommandButton SetGrpName 
         Caption         =   "Set Group Name"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   120
         TabIndex        =   17
         Tag             =   "Set group name of selected brushes"
         Top             =   1560
         Width           =   1455
      End
   End
   Begin VB.CommandButton Expand 
      Caption         =   ">"
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
      TabIndex        =   15
      Tag             =   "See more options"
      Top             =   3000
      Width           =   255
   End
   Begin VB.Frame Frame1 
      Caption         =   "Copy"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   0
      Top             =   1200
      Width           =   1335
      Begin VB.CommandButton MapCopyFromBrush 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "From Brush"
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
         TabIndex        =   2
         Tag             =   "Replace selected brushes"
         Top             =   600
         Width           =   1095
      End
      Begin VB.CommandButton MapCopyToBrush 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "To Brush"
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
         TabIndex        =   1
         Tag             =   "Copy the selected brush"
         Top             =   360
         Width           =   1095
      End
   End
   Begin VB.Frame Frame4 
      Caption         =   "Commands"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   11
      Top             =   120
      Width           =   1335
      Begin VB.CommandButton MapDupe 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Duplicate"
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
         TabIndex        =   13
         Tag             =   "Duplicates selected brushes"
         Top             =   600
         Width           =   1095
      End
      Begin VB.CommandButton MapDel 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Delete"
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
         Tag             =   "Deletes selected brushes"
         Top             =   360
         Width           =   1095
      End
   End
   Begin VB.Frame Frame3 
      Caption         =   "Select"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2775
      Left            =   1560
      TabIndex        =   6
      Top             =   120
      Width           =   1575
      Begin VB.CommandButton MapSelectNons 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Non-solids"
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
         TabIndex        =   25
         Tag             =   "Select all non-solid brushes"
         Top             =   2400
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectSemis 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Semi-solids"
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
         Tag             =   "Select all semi-solid brushes"
         Top             =   2160
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectSubtracts 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Subtracts"
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
         Tag             =   "Select all subtractive brushes"
         Top             =   1920
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectAdds 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Adds"
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
         TabIndex        =   8
         Tag             =   "Select all additive brushes"
         Top             =   1680
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectNone 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "None"
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
         Tag             =   "Unselect all brushes"
         Top             =   600
         Width           =   1335
      End
      Begin VB.CommandButton MapSelectAll 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
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
         Left            =   120
         TabIndex        =   7
         Tag             =   "Select all brushes"
         Top             =   360
         Width           =   1335
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Order"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   3
      Top             =   2280
      Width           =   1335
      Begin VB.CommandButton MapOrderToBack 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "To Last"
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
         TabIndex        =   4
         Tag             =   "Move selected brushes to last-added"
         Top             =   600
         Width           =   1095
      End
      Begin VB.CommandButton MapOrderToFront 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "To First"
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
         Tag             =   "Move selected brushes to first-added"
         Top             =   360
         Width           =   1095
      End
   End
   Begin VB.CommandButton Command1 
      Caption         =   "&Rebuild"
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
      TabIndex        =   26
      Tag             =   "Finish map-edit mode"
      Top             =   3000
      Width           =   735
   End
   Begin VB.CommandButton Done 
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
      Left            =   1560
      TabIndex        =   14
      Tag             =   "Finish map-edit mode"
      Top             =   3000
      Width           =   615
   End
End
Attribute VB_Name = "frmMapToolbar"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()
    frmRebuilder.Show
End Sub

Private Sub Help_Click()
    ToolHelp (114)
End Sub


Private Sub Expand_Click()
    If Width = 5130 Then
        Width = 3300
    Else
        Width = 5130
    End If
End Sub

Private Sub SetStat_Click()
    Dim Flags As Long
    '
    If SemiSolid.Value Then
        Flags = Flags + PF_SEMISOLID
    ElseIf NonSolid.Value Then
        Flags = Flags + PF_NOTSOLID
    Else
        Flags = 0
    End If
    '
    Ed.Server.Exec "MAP SETBRUSH CLEARFLAGS=" & _
        Trim(Str(PF_SEMISOLID + PF_NOTSOLID)) & _
        " SETFLAGS=" & Trim(Str(Flags))
End Sub

Private Sub Form_Load()
    Width = 3300
    Call Ed.SetOnTop(Me, "MapToolbar", TOP_PANEL)
End Sub

Private Sub MapCopyFromBrush_Click()
    Ed.Server.Exec "MAP BRUSH PUT"
End Sub

Private Sub MapCopyToBrush_Click()
    Ed.Server.Exec "MAP BRUSH GET"
End Sub

Private Sub MapDel_Click()
    Ed.Server.Exec "ACTOR DELETE"
End Sub

Private Sub MapDupe_Click()
    Ed.Server.Exec "ACTOR DUPLICATE"
End Sub

Private Sub MapOrderToBack_Click()
    Ed.Server.Exec "MAP SENDTO LAST"
End Sub

Private Sub MapOrderToFront_Click()
    Ed.Server.Exec "MAP SENDTO FIRST"
End Sub

Private Sub MapSelectAdds_Click()
    Ed.Server.Exec "MAP SELECT ADDS"
End Sub

Private Sub MapSelectAll_Click()
    Ed.Server.Exec "ACTOR SELECT ALL"
End Sub

Private Sub MapSelectFirst_Click()
    Ed.Server.Exec "MAP SELECT FIRST"
End Sub

Private Sub MapSelectLast_Click()
    Ed.Server.Exec "MAP SELECT LAST"
End Sub

Private Sub MapSelectNext_Click()
    Ed.Server.Exec "MAP SELECT NEXT"
End Sub

Private Sub MapSelectNone_Click()
    Ed.Server.Exec "ACTOR SELECT NONE"
End Sub

Private Sub MapSelectNons_Click()
    Ed.Server.Exec "MAP SELECT NONSOLIDS"
End Sub

Private Sub MapSelectPrevious_Click()
    Ed.Server.Exec "MAP SELECT PREVIOUS"
End Sub

Private Sub MapSelectSemis_Click()
    Ed.Server.Exec "MAP SELECT SEMISOLIDS"
End Sub

Private Sub MapSelectSubtracts_Click()
    Ed.Server.Exec "MAP SELECT SUBTRACTS"
End Sub

Private Sub SetGrpName_Click()
    Ed.Server.Exec "MAP SETBRUSH GROUP=" & GrpName.Text
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

        
