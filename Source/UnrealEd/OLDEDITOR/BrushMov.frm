VERSION 4.00
Begin VB.Form frmBrushMove 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Brush Keyframes"
   ClientHeight    =   5175
   ClientLeft      =   3885
   ClientTop       =   4440
   ClientWidth     =   7140
   ControlBox      =   0   'False
   Height          =   5655
   Left            =   3825
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5175
   ScaleWidth      =   7140
   ShowInTaskbar   =   0   'False
   Top             =   4020
   Width           =   7260
   Begin VB.Frame Frame3 
      Caption         =   "Relative keyframe positioning"
      Height          =   1455
      Left            =   3780
      TabIndex        =   20
      Top             =   60
      Width           =   3315
      Begin VB.Label Label8 
         Alignment       =   1  'Right Justify
         Caption         =   "Right:"
         Height          =   255
         Left            =   60
         TabIndex        =   26
         Top             =   600
         Width           =   675
      End
      Begin VB.Label Label7 
         Alignment       =   1  'Right Justify
         Caption         =   "Forward:"
         Height          =   255
         Left            =   60
         TabIndex        =   25
         Top             =   840
         Width           =   675
      End
      Begin VB.Label Label6 
         Alignment       =   1  'Right Justify
         Caption         =   "Up:"
         Height          =   255
         Left            =   1740
         TabIndex        =   24
         Top             =   360
         Width           =   555
      End
      Begin VB.Label Label5 
         Alignment       =   1  'Right Justify
         Caption         =   "Up:"
         Height          =   255
         Left            =   1380
         TabIndex        =   23
         Top             =   600
         Width           =   915
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Up:"
         Height          =   255
         Left            =   1380
         TabIndex        =   22
         Top             =   840
         Width           =   915
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Up:"
         Height          =   255
         Left            =   60
         TabIndex        =   21
         Top             =   360
         Width           =   675
      End
   End
   Begin VB.PictureBox HolderRename 
      BorderStyle     =   0  'None
      Enabled         =   0   'False
      Height          =   1515
      Left            =   0
      ScaleHeight     =   1515
      ScaleWidth      =   3735
      TabIndex        =   14
      Top             =   3360
      Width           =   3735
      Begin VB.Frame RenameTitle 
         Caption         =   "Rename a keyframe"
         Height          =   1455
         Left            =   60
         TabIndex        =   17
         Top             =   0
         Width           =   1875
         Begin VB.TextBox RenameName 
            Height          =   285
            Left            =   120
            TabIndex        =   18
            Text            =   "Text1"
            Top             =   840
            Width           =   1575
         End
         Begin VB.Label Label2 
            Caption         =   "New name:"
            Height          =   255
            Left            =   120
            TabIndex        =   19
            Top             =   600
            Width           =   1515
         End
      End
      Begin VB.CommandButton DoRename 
         Caption         =   "&Rename"
         Default         =   -1  'True
         Height          =   315
         Left            =   2040
         TabIndex        =   16
         Top             =   780
         Width           =   1575
      End
      Begin VB.CommandButton CancelRename 
         Cancel          =   -1  'True
         Caption         =   "&Cancel"
         Height          =   315
         Left            =   2040
         TabIndex        =   15
         Top             =   1140
         Width           =   1575
      End
   End
   Begin VB.PictureBox HolderAdd 
      BorderStyle     =   0  'None
      Enabled         =   0   'False
      Height          =   1515
      Left            =   0
      ScaleHeight     =   1515
      ScaleWidth      =   3735
      TabIndex        =   7
      Top             =   1740
      Width           =   3735
      Begin VB.CommandButton CancelAdd 
         Caption         =   "&Cancel"
         Height          =   315
         Left            =   2040
         TabIndex        =   12
         Top             =   1140
         Width           =   1575
      End
      Begin VB.CommandButton DoAdd 
         Caption         =   "&Add"
         Height          =   315
         Left            =   2040
         TabIndex        =   11
         Top             =   780
         Width           =   1575
      End
      Begin VB.Frame Frame2 
         Caption         =   "Add a keyframe"
         Height          =   1455
         Left            =   60
         TabIndex        =   8
         Top             =   0
         Width           =   1875
         Begin VB.TextBox AddName 
            Height          =   285
            Left            =   120
            TabIndex        =   9
            Text            =   "Text1"
            Top             =   780
            Width           =   1575
         End
         Begin VB.Label Label1 
            Caption         =   "Keyframe Name:"
            Height          =   255
            Left            =   180
            TabIndex        =   10
            Top             =   480
            Width           =   1335
         End
      End
   End
   Begin VB.PictureBox HolderMain 
      BorderStyle     =   0  'None
      Height          =   1515
      Left            =   0
      ScaleHeight     =   1515
      ScaleWidth      =   3735
      TabIndex        =   0
      Top             =   0
      Width           =   3735
      Begin VB.CommandButton Rename 
         Caption         =   "&Rename"
         Height          =   315
         Left            =   2040
         TabIndex        =   13
         Top             =   420
         Width           =   1575
      End
      Begin VB.CommandButton Delete 
         Caption         =   "&Delete Keyframe"
         Height          =   315
         Left            =   2040
         TabIndex        =   6
         Top             =   1140
         Width           =   1575
      End
      Begin VB.CommandButton SnagBrush 
         Caption         =   "&Use Current Brush"
         Height          =   315
         Left            =   2040
         TabIndex        =   5
         Top             =   780
         Width           =   1575
      End
      Begin VB.CommandButton Add 
         Caption         =   "&Add Keyframe"
         Height          =   315
         Left            =   2040
         TabIndex        =   4
         Top             =   60
         Width           =   1575
      End
      Begin VB.Frame Frame1 
         Caption         =   "Keyframe Position"
         Height          =   1455
         Left            =   60
         TabIndex        =   1
         Top             =   0
         Width           =   1875
         Begin VB.ComboBox KeyNames 
            Height          =   315
            Left            =   120
            Style           =   2  'Dropdown List
            TabIndex        =   2
            Top             =   540
            Width           =   1635
         End
         Begin VB.Label KeyCount 
            Alignment       =   2  'Center
            Caption         =   "(1 of 2)"
            Height          =   255
            Left            =   120
            TabIndex        =   3
            Top             =   1080
            Width           =   1635
         End
      End
   End
End
Attribute VB_Name = "frmBrushMove"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim Updating As Boolean

'
' Public interface
'

Public Sub Update()
    Dim Num As Integer, i As Integer, Cur As Integer
    Dim CurName As String, S As String
    '
    Updating = True
    '
    HolderMain.Visible = True
    HolderMain.Enabled = True
    '
    HolderAdd.Visible = False
    HolderAdd.Enabled = False
    '
    HolderRename.Visible = False
    HolderRename.Enabled = False
    '
    If KeyNames.ListCount < 4 Then
        Add.Enabled = True
    Else
        Add.Enabled = False
    End If
    '
    If KeyNames.ListCount < 1 Then
        Rename.Enabled = False
    Else
        Rename.Enabled = True
    End If
    '
    ' Get list of keyframes:
    '
    Num = Val(Ed.Server.GetProp("Actor", "Properties NAME=NumKeys Raw=on"))
    CurName = Ed.Server.GetProp("Actor", "Properties Name=InitialKeyName")
    Cur = 0
    '
    KeyNames.Clear
    For i = 0 To Num - 1
        S = Ed.Server.GetProp("Actor", _
            "Properties Name=KeyName Element=" & Trim(Str(i)) & " Raw=on")
        KeyNames.AddItem S
        If S = CurName Then Cur = i + 1
    Next
    '
    If KeyNames.ListCount > 0 Then KeyNames.ListIndex = 0
    '
    KeyCount = "(" & Trim(Str(Cur)) & " of " & Trim(Str(Num)) & ")"
    '
    Updating = False
    '
End Sub

'
' Form stuff
'

Function CurKeyName() As String
    If KeyNames.ListIndex > 0 Then
        CurKeyName = KeyNames.List(KeyNames.ListIndex)
    Else
        CurKeyName = ""
    End If
End Function

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "Keyframes", TOP_NORMAL)
    '
    HolderAdd.Top = HolderMain.Top
    HolderAdd.Left = HolderMain.Left
    '
    HolderRename.Top = HolderMain.Top
    HolderRename.Left = HolderMain.Left
    '
    Width = Width - ScaleWidth + HolderMain.Width
    Height = Height - ScaleHeight + HolderMain.Height
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

'
' Main commands
'

Private Sub Add_Click()
    HolderAdd.Visible = True
    HolderAdd.Enabled = True
    '
    HolderMain.Visible = False
    HolderMain.Enabled = False
    '
    AddName.Text = ""
    AddName.SetFocus
End Sub

Private Sub KeyNames_Change()
    If Not Updating Then
        Ed.Server.Exec "SENDACTORS KEY SHOW NAME=" & CurKeyName()
    End If
End Sub

Private Sub Rename_Click()
    RenameTitle.Caption = "Rename " & CurKeyName()
    '
    HolderRename.Visible = True
    HolderRename.Enabled = True
    '
    HolderMain.Visible = False
    HolderMain.Enabled = False
    '
    RenameName.Text = CurKeyName()
    RenameName.SetFocus
End Sub

Private Sub SnagBrush_Click()
    Ed.Server.Exec "SENDACTORS KEY SNAG"
End Sub

Private Sub Delete_Click()
    Ed.Server.Disable
    If KeyNames.ListCount <= 1 Then
        MsgBox ("You can't delete the last remaining keyframe.")
    ElseIf MsgBox("Are you sure you want to delete keyframe '" & _
        CurKeyName & "'?", _
        vbYesNo, "Delete a keyframe") = vbYes Then
        '
        Ed.Server.Exec "SENDACTORS KEY DELETE NAME=" & CurKeyName()
    End If
    Ed.Server.Enable
End Sub

'
' Add-keyframe commands
'

Private Sub DoAdd_Click()
    Ed.Server.Exec "SENDACTORS KEY ADD NAME=" & AddName
    Call Update
End Sub

Private Sub CancelAdd_Click()
    Call Update
End Sub

'
' Rename-keyframe commands
'
Private Sub DoRename_Click()
    Ed.Server.Exec "SENDACTORS KEY RENAME NAME=" & CurKeyName & " NEWNAME=" & RenameName
    Call Update
End Sub

Private Sub CancelRename_Click()
    Call Update
End Sub

