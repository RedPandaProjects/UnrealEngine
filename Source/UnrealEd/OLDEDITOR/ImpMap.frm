VERSION 5.00
Begin VB.Form frmImportMap 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Import a map"
   ClientHeight    =   1530
   ClientLeft      =   2340
   ClientTop       =   5205
   ClientWidth     =   2925
   Icon            =   "ImpMap.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   1530
   ScaleWidth      =   2925
   ShowInTaskbar   =   0   'False
   Begin VB.Frame Frame1 
      Caption         =   "Import Map Options"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   2
      Top             =   120
      Width           =   2775
      Begin VB.OptionButton ImpExisting 
         Caption         =   "Add contents to existing map"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   600
         Width           =   2415
      End
      Begin VB.OptionButton ImpNew 
         Caption         =   "Import a new map"
         Height          =   255
         Left            =   120
         TabIndex        =   3
         Top             =   360
         Value           =   -1  'True
         Width           =   1695
      End
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   1920
      TabIndex        =   1
      Top             =   1140
      Width           =   975
   End
   Begin VB.CommandButton Import 
      Caption         =   "&Import"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   1140
      Width           =   975
   End
End
Attribute VB_Name = "frmImportMap"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Cancel_Click()
    GResult = False
    Unload Me
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
    If GImportExisting Then
        ImpExisting.Value = True
    Else
        ImpNew.Value = True
    End If
End Sub

Private Sub Import_Click()
    If Not GImportExisting Then
        Ed.MapFname = ""
        frmMain.Caption = Ed.EditorAppName + " - Newly-imported map"
    End If
    '
    GResult = True
    GImportExisting = ImpExisting.Value
    Unload Me
End Sub

