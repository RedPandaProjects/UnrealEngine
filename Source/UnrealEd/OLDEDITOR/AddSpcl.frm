VERSION 5.00
Begin VB.Form frmAddSpecial 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Add Special Brush"
   ClientHeight    =   3285
   ClientLeft      =   4110
   ClientTop       =   4950
   ClientWidth     =   3210
   HelpContextID   =   316
   Icon            =   "AddSpcl.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3285
   ScaleWidth      =   3210
   ShowInTaskbar   =   0   'False
   Begin VB.ComboBox Settings 
      Height          =   315
      Left            =   1200
      Style           =   2  'Dropdown List
      TabIndex        =   16
      Tag             =   "Predefined settings"
      Top             =   120
      Width           =   1935
   End
   Begin VB.Frame Frame5 
      Caption         =   "Effects"
      Height          =   1335
      Left            =   120
      TabIndex        =   13
      Top             =   1440
      Width           =   1455
      Begin VB.CheckBox Invisible 
         Caption         =   "Invisible"
         Height          =   255
         Left            =   120
         TabIndex        =   18
         Top             =   840
         Width           =   1215
      End
      Begin VB.CheckBox ZonePortal 
         Caption         =   "Zone Portal"
         Height          =   255
         Left            =   120
         TabIndex        =   17
         Top             =   600
         Width           =   1215
      End
      Begin VB.CheckBox Transparent 
         Caption         =   "Transparent"
         Height          =   195
         Left            =   120
         TabIndex        =   14
         Tag             =   "Textures are transparent"
         Top             =   360
         Width           =   1215
      End
   End
   Begin VB.CommandButton Close 
      Caption         =   "&Close"
      Height          =   375
      Left            =   2280
      TabIndex        =   11
      Tag             =   "Close this window"
      Top             =   2880
      Width           =   855
   End
   Begin VB.CommandButton Help 
      Caption         =   "&Help"
      Height          =   375
      Left            =   1440
      TabIndex        =   10
      Tag             =   "Help"
      Top             =   2880
      Width           =   735
   End
   Begin VB.CommandButton Add 
      Caption         =   "&Add Special"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   9
      Tag             =   "Add this brush specially"
      Top             =   2880
      Width           =   1215
   End
   Begin VB.Frame Frame3 
      Caption         =   "Solidity"
      Height          =   1395
      Left            =   1680
      TabIndex        =   2
      Top             =   1380
      Width           =   1455
      Begin VB.OptionButton NonSolid 
         Caption         =   "Non-solid"
         Height          =   255
         Left            =   120
         TabIndex        =   12
         Tag             =   "Brush has no effect on player movement"
         Top             =   720
         Width           =   1215
      End
      Begin VB.OptionButton SemiSolid 
         Caption         =   "Semi-solid"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Tag             =   "Brush is semi-solid"
         Top             =   480
         Width           =   1215
      End
      Begin VB.OptionButton Solid 
         Caption         =   "Solid"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Tag             =   "Brush is solid"
         Top             =   240
         Value           =   -1  'True
         Width           =   1215
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Visibility"
      Height          =   855
      Left            =   1680
      TabIndex        =   1
      Top             =   480
      Width           =   1455
      Begin VB.OptionButton TwoSided 
         Caption         =   "2-Sided"
         Height          =   255
         Left            =   120
         TabIndex        =   6
         Tag             =   "Textures are visible from both sides"
         Top             =   480
         Width           =   1095
      End
      Begin VB.OptionButton OneSided 
         Caption         =   "1-Sided"
         Height          =   255
         Left            =   120
         TabIndex        =   5
         Tag             =   "Textures are visible from front only"
         Top             =   240
         Value           =   -1  'True
         Width           =   1095
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Masking"
      Height          =   915
      Left            =   120
      TabIndex        =   0
      Top             =   480
      Width           =   1455
      Begin VB.OptionButton Masked 
         Caption         =   "Masked"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Tag             =   "Textures are drawn with holes"
         Top             =   540
         Width           =   1215
      End
      Begin VB.OptionButton Regular 
         Caption         =   "Regular"
         Height          =   255
         Left            =   120
         TabIndex        =   3
         Tag             =   "Textures have no holes"
         Top             =   300
         Value           =   -1  'True
         Width           =   1215
      End
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "Predefined:"
      Height          =   255
      Left            =   120
      TabIndex        =   15
      Top             =   120
      Width           =   975
   End
End
Attribute VB_Name = "frmAddSpecial"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Add_Click()
    Dim Flags As Long
    Flags = 0
    '
    If Masked.Value Then
        Flags = Flags + PF_MASKED
    End If
    '
    If Transparent.Value Then
        Flags = Flags + PF_TRANSPARENT
    End If
    '
    If Invisible.Value Then
        Flags = Flags + PF_INVISIBLE
    End If
    '
    If ZonePortal.Value Then
        Flags = Flags + PF_ZONE_PORTAL
    End If
    '
    If SemiSolid.Value Then
        Flags = Flags + PF_SEMISOLID
    ElseIf NonSolid.Value Then
        Flags = Flags + PF_NOTSOLID
    End If
    '
    If TwoSided.Value Then
        Flags = Flags + PF_TWO_SIDED
    End If
    '
    Ed.BeginSlowTask "Adding special brush to world"
    Ed.ServerExec "BRUSH ADD FLAGS=" & Trim(Str(Flags))
    Ed.EndSlowTask
End Sub

Private Sub Close_Click()
    Hide
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "AddSpecial", TOP_NORMAL)
    '
    Settings.AddItem "Invisible collision hull"
    Settings.AddItem "Zone Portal"
    Settings.AddItem "Semisolid Pillar"
    Settings.AddItem "Transparent Window"
    Settings.AddItem "Masked Decoration"
    Settings.AddItem "Masked Wall"
    Settings.AddItem "Water"
    Settings.AddItem "Regular Brush"
    '
    Settings.Text = "Regular Brush"
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Help_Click()
   SendKeys "{F1}"
End Sub

Private Sub Settings_Click()
    Select Case Settings.Text
    Case "Regular Brush":
        Regular.Value = True
        ZonePortal.Value = 0
        Transparent.Value = 0
        Invisible.Value = 0
        Solid.Value = True
        OneSided.Value = True
    Case "Zone Portal":
        Regular.Value = True
        ZonePortal.Value = 1
        Transparent.Value = 0
        Invisible.Value = 1
        NonSolid.Value = True
        TwoSided.Value = True
    Case "Semisolid Pillar":
        Regular.Value = True
        ZonePortal.Value = 0
        Transparent.Value = 0
        Invisible.Value = 0
        SemiSolid.Value = True
        OneSided.Value = True
    Case "Transparent Window":
        Regular.Value = True
        ZonePortal.Value = 0
        Transparent.Value = 1
        Invisible.Value = 0
        NonSolid.Value = True
        OneSided.Value = True
    Case "Masked Decoration":
        Masked.Value = True
        ZonePortal.Value = 0
        Transparent.Value = 0
        Invisible.Value = 0
        NonSolid.Value = True
        OneSided.Value = True
    Case "Masked Wall":
        Masked.Value = True
        ZonePortal.Value = 0
        Transparent.Value = 0
        Invisible.Value = 0
        SemiSolid.Value = True
        OneSided.Value = True
    Case "Water":
        Regular.Value = True
        ZonePortal.Value = 1
        Transparent.Value = 1
        Invisible.Value = 0
        NonSolid.Value = True
        TwoSided.Value = True
    Case "Invisible collision hull":
        Regular.Value = True
        ZonePortal.Value = 0
        Transparent.Value = 0
        Invisible.Value = 1
        SemiSolid.Value = True
        TwoSided.Value = False
    End Select
End Sub

