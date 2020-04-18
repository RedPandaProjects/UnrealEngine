VERSION 5.00
Begin VB.Form frmMeshViewer 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Mesh Browser"
   ClientHeight    =   5460
   ClientLeft      =   945
   ClientTop       =   3150
   ClientWidth     =   6915
   Icon            =   "MeshView.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5460
   ScaleWidth      =   6915
   ShowInTaskbar   =   0   'False
   Begin VB.Frame Frame2 
      Caption         =   "Texture"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   5355
      Left            =   7080
      TabIndex        =   22
      Top             =   60
      Width           =   4275
      Begin VB.CommandButton ApplyTexture 
         Caption         =   "Apply"
         Height          =   255
         Left            =   1560
         TabIndex        =   39
         Top             =   480
         Width           =   1335
      End
      Begin VB.CommandButton Command5 
         Caption         =   "<- Add"
         Height          =   255
         Left            =   3000
         TabIndex        =   30
         Top             =   540
         Width           =   1155
      End
      Begin VB.CommandButton Command4 
         Caption         =   "Remove ->"
         Height          =   255
         Left            =   3000
         TabIndex        =   29
         Top             =   840
         Width           =   1155
      End
      Begin VB.CommandButton Command3 
         Caption         =   "Browse"
         Height          =   255
         Left            =   3000
         TabIndex        =   28
         Top             =   240
         Width           =   1155
      End
      Begin VB.CommandButton Command2 
         Caption         =   "<"
         Height          =   315
         Left            =   60
         TabIndex        =   27
         Tag             =   "Previous mesh"
         Top             =   780
         Width           =   210
      End
      Begin VB.CommandButton Command1 
         Caption         =   ">"
         Height          =   315
         Left            =   270
         TabIndex        =   26
         Tag             =   "Next mesh"
         Top             =   780
         Width           =   210
      End
      Begin VB.ComboBox Combo1 
         Height          =   315
         Left            =   540
         TabIndex        =   24
         Text            =   "Combo1"
         Top             =   780
         Width           =   2355
      End
      Begin VB.PictureBox Picture1 
         BackColor       =   &H00000000&
         Height          =   4110
         Left            =   120
         ScaleHeight     =   270
         ScaleMode       =   3  'Pixel
         ScaleWidth      =   266
         TabIndex        =   23
         Top             =   1140
         Width           =   4050
      End
      Begin VB.Label Label5 
         Caption         =   "Current Texture:"
         Height          =   255
         Left            =   120
         TabIndex        =   25
         Top             =   540
         Width           =   1455
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Animation Control"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   5355
      Left            =   4440
      TabIndex        =   1
      Top             =   60
      Width           =   2415
      Begin VB.CheckBox AutoDolly 
         Caption         =   "Auto Dolly"
         Height          =   255
         Left            =   120
         TabIndex        =   42
         Tag             =   "Continuously rotate the mesh"
         Top             =   3540
         Width           =   1275
      End
      Begin VB.CommandButton Command6 
         Caption         =   "Home"
         Enabled         =   0   'False
         Height          =   255
         Left            =   1500
         TabIndex        =   41
         Top             =   3480
         Width           =   795
      End
      Begin VB.CheckBox ShowSelection 
         Caption         =   "Show selection"
         Enabled         =   0   'False
         Height          =   255
         Left            =   120
         TabIndex        =   40
         Top             =   4020
         Width           =   1755
      End
      Begin VB.CommandButton PanDown 
         Enabled         =   0   'False
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
         Left            =   1740
         TabIndex        =   38
         Top             =   3120
         Width           =   255
      End
      Begin VB.CommandButton PanRight 
         Enabled         =   0   'False
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
         Left            =   1980
         TabIndex        =   37
         Top             =   2880
         Width           =   255
      End
      Begin VB.CommandButton PanLeft 
         Enabled         =   0   'False
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
         Left            =   1500
         TabIndex        =   36
         Top             =   2880
         Width           =   255
      End
      Begin VB.CommandButton PanUp 
         Enabled         =   0   'False
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
         Left            =   1740
         TabIndex        =   35
         Top             =   2640
         Width           =   255
      End
      Begin VB.OptionButton ModeView 
         Caption         =   "View It"
         Enabled         =   0   'False
         Height          =   255
         Left            =   120
         TabIndex        =   33
         Tag             =   "See texture-mapped mesh"
         Top             =   4620
         Width           =   855
      End
      Begin VB.OptionButton ModeTexture 
         Caption         =   "Texture It"
         Enabled         =   0   'False
         Height          =   255
         Left            =   1200
         TabIndex        =   32
         Tag             =   "See mesh polygons"
         Top             =   4620
         Width           =   1095
      End
      Begin VB.CheckBox RotateAround 
         Caption         =   "Rotate Around"
         Enabled         =   0   'False
         Height          =   255
         Left            =   120
         TabIndex        =   31
         Top             =   3780
         Value           =   1  'Checked
         Width           =   1755
      End
      Begin VB.OptionButton ViewPolys 
         Caption         =   "Polygons"
         Height          =   255
         Left            =   120
         TabIndex        =   21
         Tag             =   "See mesh polygons"
         Top             =   2940
         Width           =   1335
      End
      Begin VB.CommandButton Close 
         Cancel          =   -1  'True
         Caption         =   "&Close"
         Height          =   315
         Left            =   1620
         TabIndex        =   3
         Tag             =   "Close this window"
         Top             =   4980
         Width           =   675
      End
      Begin VB.CommandButton SeqMinus 
         Caption         =   "<"
         Height          =   315
         Left            =   60
         TabIndex        =   19
         Tag             =   "Previous sequence"
         Top             =   1320
         Width           =   210
      End
      Begin VB.CommandButton SeqPlus 
         Caption         =   ">"
         Height          =   315
         Left            =   270
         TabIndex        =   18
         Tag             =   "Next sequence"
         Top             =   1320
         Width           =   210
      End
      Begin VB.CommandButton MeshPlus 
         Caption         =   ">"
         Height          =   315
         Left            =   270
         TabIndex        =   16
         Tag             =   "Next mesh"
         Top             =   600
         Width           =   210
      End
      Begin VB.CommandButton MeshMinus 
         Caption         =   "<"
         Height          =   315
         Left            =   60
         TabIndex        =   17
         Tag             =   "Previous mesh"
         Top             =   600
         Width           =   210
      End
      Begin VB.CommandButton Refresh 
         Caption         =   "&Refresh"
         Height          =   315
         Left            =   840
         TabIndex        =   15
         Tag             =   "Refresh the mesh list"
         Top             =   4980
         Width           =   735
      End
      Begin VB.OptionButton ViewTextured 
         Caption         =   "Textured"
         Height          =   255
         Left            =   120
         TabIndex        =   14
         Tag             =   "See texture-mapped mesh"
         Top             =   2700
         Value           =   -1  'True
         Width           =   1335
      End
      Begin VB.CommandButton FramePlay 
         Caption         =   "Play >>"
         Height          =   255
         Left            =   1200
         TabIndex        =   12
         Tag             =   "Play/stop this animation"
         Top             =   2040
         Width           =   855
      End
      Begin VB.CommandButton FramePlus 
         Caption         =   ">"
         Height          =   255
         Left            =   840
         TabIndex        =   11
         Tag             =   "View next frame"
         Top             =   2040
         Width           =   255
      End
      Begin VB.CommandButton FrameMinus 
         Caption         =   "<"
         Height          =   255
         Left            =   600
         TabIndex        =   10
         Tag             =   "View previous frame"
         Top             =   2040
         Width           =   255
      End
      Begin VB.CommandButton FrameZero 
         Caption         =   "0"
         Height          =   255
         Left            =   120
         TabIndex        =   9
         Tag             =   "Go to first frame"
         Top             =   2040
         Width           =   375
      End
      Begin VB.ComboBox AnimSeqCombo 
         Height          =   315
         Left            =   510
         Style           =   2  'Dropdown List
         TabIndex        =   7
         Tag             =   "Animation sequence to view"
         Top             =   1320
         Width           =   1815
      End
      Begin VB.ComboBox MeshCombo 
         Height          =   315
         Left            =   540
         Sorted          =   -1  'True
         Style           =   2  'Dropdown List
         TabIndex        =   5
         Tag             =   "Mesh to view"
         Top             =   600
         Width           =   1815
      End
      Begin VB.CommandButton Help 
         Caption         =   "&Help"
         Height          =   315
         Left            =   120
         TabIndex        =   2
         Tag             =   "Get help"
         Top             =   4980
         Width           =   675
      End
      Begin VB.Label Label6 
         Caption         =   "Mode:"
         Enabled         =   0   'False
         Height          =   255
         Left            =   120
         TabIndex        =   34
         Top             =   4380
         Width           =   1095
      End
      Begin VB.Label Label3 
         Caption         =   "Options:"
         Height          =   255
         Left            =   120
         TabIndex        =   20
         Top             =   3300
         Width           =   1455
      End
      Begin VB.Label Label4 
         Caption         =   "View:"
         Height          =   255
         Left            =   120
         TabIndex        =   13
         Top             =   2460
         Width           =   1095
      End
      Begin VB.Label FrameLabel 
         Caption         =   "Frame 0 of 0"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   1800
         Width           =   1815
      End
      Begin VB.Label Label2 
         Caption         =   "Animation Sequence:"
         Height          =   255
         Left            =   120
         TabIndex        =   6
         Top             =   1080
         Width           =   1575
      End
      Begin VB.Label Label1 
         Caption         =   "Mesh:"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   360
         Width           =   1815
      End
   End
   Begin VB.PictureBox MeshCamHolder 
      BackColor       =   &H00000000&
      Height          =   5430
      Left            =   0
      ScaleHeight     =   358
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   292
      TabIndex        =   0
      Top             =   0
      Width           =   4440
   End
End
Attribute VB_Name = "frmMeshViewer"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim FramePos As Integer
Dim NumFrames As Integer
Dim Playing As Integer

'
' Public:
'

Public Function GetCurrent() As String
    GetCurrent = MeshCombo.Text
End Function

'
' Startup/shutdown
'

Private Sub Close_Click()
    Unload Me
End Sub

Private Sub Form_Load()
    Dim S As String
    Call Ed.SetOnTop(Me, "MeshViewer", TOP_PANEL)
    UpdateMeshList "Open"
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
    Ed.ServerExec "CAMERA CLOSE NAME=MeshViewCam"
End Sub

'
' List refreshing
'

Sub UpdateMeshList(Cmd As String)
    Dim Result As String, S As String
    MeshCombo.Clear
    Result = Ed.ServerGetProp("OBJ", "Query Type=Mesh")
    Do
        S = GrabString(Result)
        If S = "" Then Exit Do
        MeshCombo.AddItem S
    Loop
    If MeshCombo.ListCount > 0 Then MeshCombo.ListIndex = 0
    UpdateFrameList Cmd
End Sub

Sub UpdateFrameList(Cmd As String)
    Dim i As Integer
    Dim N As Integer
    Dim S As String
    '
    AnimSeqCombo.Clear
    If MeshCombo.ListIndex >= 0 Then
        N = Val(Ed.ServerGetProp("MESH", "NUMANIMSEQS" & _
            " NAME=" & Quotes(MeshCombo.List(MeshCombo.ListIndex))))
        For i = 0 To N - 1
            S = Trim(Ed.ServerGetProp("MESH", "ANIMSEQ" & _
                " NAME=" & Quotes(MeshCombo.List(MeshCombo.ListIndex)) & _
                " NUM=" & Trim(Str(i))))
            If S <> "" Then
                AnimSeqCombo.AddItem S
            End If
        Next i
        If AnimSeqCombo.ListCount > 0 Then AnimSeqCombo.ListIndex = 0
    End If
    UpdateFrame Cmd
End Sub

Sub UpdateFrame(Cmd As String)
    FramePos = 0
    If AnimSeqCombo.ListCount > 0 Then
        NumFrames = Val(Right(AnimSeqCombo.Text, 3))
    Else
        NumFrames = 0
    End If
    UpdateFrameCaption Cmd
End Sub

Sub UpdateFrameCaption(Cmd As String)
    Dim Tmp As String
    
    If UCase(Cmd) = "OPEN" Then
        Tmp = " XR=" & Trim(Str(MeshCamHolder.ScaleWidth)) & _
            " YR=" & Trim(Str(MeshCamHolder.ScaleHeight))
    End If
    
    If Playing Then
        FrameLabel.Caption = "Playing " & Trim(Str(NumFrames)) & " frames..."
    Else
        FrameLabel.Caption = "Frame " & Trim(Str(FramePos + 0)) & " of " & Trim(Str(NumFrames))
    End If
    
    If MeshCombo.ListCount <= 0 Then
        Ed.ServerExec "CAMERA CLOSE NAME=MeshViewCam"
    Else
        Ed.ServerExec "CAMERA " & Cmd & _
            " NAME=MeshViewCam X=0 Y=0" & _
            " MESH=" & Quotes(MeshCombo.Text) & _
            " REN=" & Trim(Str(REN_MESHVIEW)) & _
            " FLAGS=" & Trim(Str(SHOW_AS_CHILD + SHOW_NOBUTTONS + IIf(Playing, SHOW_BACKDROP, 0) + IIf(Playing = 1 Or AutoDolly.Value, SHOW_REALTIME, 0) + IIf(AutoDolly.Value, SHOW_BRUSH, 0) + IIf(ViewPolys.Value, SHOW_COORDS, 0))) & _
            " HWND=" & Trim(Str(MeshCamHolder.hwnd)) & _
            " MISC1=" & Trim(Left(Right(AnimSeqCombo.Text, 7), 3)) & _
            " MISC2=" & Trim(Str(FramePos)) & _
            Tmp
    End If
End Sub

'
' Buttons
'

Private Sub FramePlay_Click()
    If Playing = 1 Then
        FramePlay.Caption = "Play >>"
        FrameZero.Enabled = True
        FrameMinus.Enabled = True
        FramePlus.Enabled = True
        Playing = 0
    Else
        FramePlay.Caption = "Stop"
        FrameZero.Enabled = False
        FrameMinus.Enabled = False
        FramePlus.Enabled = False
        Playing = 1
    End If
    UpdateFrameCaption "Update"
End Sub

Private Sub FrameZero_Click()
    FramePos = 0
    UpdateFrameCaption "Update"
End Sub

Private Sub FrameMinus_Click()
    FramePos = FramePos - 1
    If FramePos < 0 Then FramePos = NumFrames - 1
    If FramePos < 0 Then FramePos = 0
    UpdateFrameCaption "Update"
End Sub

Private Sub FramePlus_Click()
    FramePos = FramePos + 1
    If FramePos >= NumFrames Then FramePos = 0
    UpdateFrameCaption "Update"
End Sub

Private Sub Refresh_Click()
    UpdateMeshList "Update"
End Sub

Private Sub MeshCombo_Click()
    UpdateFrameList "Update"
End Sub

Private Sub AnimSeqCombo_Click()
    UpdateFrame "Update"
End Sub

Private Sub MeshMinus_Click()
    If MeshCombo.ListIndex >= 0 Then
        If MeshCombo.ListIndex > 0 Then
            MeshCombo.ListIndex = MeshCombo.ListIndex - 1
        ElseIf MeshCombo.ListCount > 0 Then
            MeshCombo.ListIndex = MeshCombo.ListCount - 1
        End If
    End If
End Sub

Private Sub MeshPlus_Click()
    If MeshCombo.ListIndex >= 0 Then
        If MeshCombo.ListIndex + 1 < MeshCombo.ListCount Then
            MeshCombo.ListIndex = MeshCombo.ListIndex + 1
        Else
            MeshCombo.ListIndex = 0
        End If
    End If
End Sub

Private Sub SeqMinus_Click()
    If AnimSeqCombo.ListIndex >= 0 Then
        If AnimSeqCombo.ListIndex > 0 Then
            AnimSeqCombo.ListIndex = AnimSeqCombo.ListIndex - 1
        ElseIf AnimSeqCombo.ListCount > 0 Then
            AnimSeqCombo.ListIndex = AnimSeqCombo.ListCount - 1
        End If
    End If
End Sub

Private Sub SeqPlus_Click()
    If AnimSeqCombo.ListIndex >= 0 Then
        If AnimSeqCombo.ListIndex + 1 < AnimSeqCombo.ListCount Then
            AnimSeqCombo.ListIndex = AnimSeqCombo.ListIndex + 1
        Else
            AnimSeqCombo.ListIndex = 0
        End If
    End If
End Sub

Private Sub ViewPolys_Click()
    UpdateFrameCaption "Update"
End Sub

Private Sub ViewTextured_Click()
    UpdateFrameCaption "Update"
End Sub

Private Sub ViewWire_Click()
    UpdateFrameCaption "Update"
End Sub

Private Sub AutoDolly_Click()
    UpdateFrameCaption "Update"
End Sub

