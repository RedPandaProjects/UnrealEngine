VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmParSolTube 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build a Tube/Cyllinder"
   ClientHeight    =   5400
   ClientLeft      =   7650
   ClientTop       =   1620
   ClientWidth     =   2295
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
   HelpContextID   =   156
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5400
   ScaleWidth      =   2295
   ShowInTaskbar   =   0   'False
   Begin VB.CheckBox chkAlignSide 
      Caption         =   "Align to Side"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   120
      TabIndex        =   10
      Top             =   2160
      Value           =   1  'Checked
      Width           =   1452
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Help"
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
      Left            =   840
      TabIndex        =   6
      Top             =   4920
      Width           =   615
   End
   Begin VB.TextBox txtTubeHeight 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1200
      TabIndex        =   0
      Text            =   "256"
      Top             =   2520
      Width           =   975
   End
   Begin VB.TextBox txtInnerRadius 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1200
      TabIndex        =   2
      Text            =   "512-16"
      Top             =   3240
      Width           =   975
   End
   Begin VB.TextBox txtOuterRadius 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1200
      TabIndex        =   1
      Text            =   "512"
      Top             =   2880
      Width           =   975
   End
   Begin VB.TextBox txtSides 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1200
      TabIndex        =   3
      Text            =   "8"
      Top             =   3600
      Width           =   975
   End
   Begin VB.TextBox txtGroup 
      BackColor       =   &H00FFFFFF&
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   1200
      TabIndex        =   4
      Text            =   "Tube"
      Top             =   3960
      Width           =   975
   End
   Begin VB.OptionButton optSolid 
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
      TabIndex        =   8
      Top             =   1800
      Value           =   -1  'True
      Width           =   735
   End
   Begin VB.OptionButton optHollow 
      Caption         =   "Hollow"
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
      Left            =   1080
      TabIndex        =   9
      Top             =   1800
      Width           =   855
   End
   Begin VB.CommandButton Build 
      Caption         =   "&Build"
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
      Height          =   375
      Left            =   120
      TabIndex        =   5
      Top             =   4920
      Width           =   615
   End
   Begin VB.CommandButton Command2 
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
      Height          =   375
      Left            =   1560
      TabIndex        =   7
      Top             =   4920
      Width           =   615
   End
   Begin Threed.SSPanel SSPanel1 
      Height          =   1575
      Left            =   120
      TabIndex        =   18
      Top             =   120
      Width           =   2055
      _Version        =   65536
      _ExtentX        =   3625
      _ExtentY        =   2778
      _StockProps     =   15
      BackColor       =   12632256
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      BevelInner      =   1
      Begin VB.Line Line1 
         BorderColor     =   &H0000FFFF&
         X1              =   240
         X2              =   240
         Y1              =   360
         Y2              =   1200
      End
      Begin VB.Line Line2 
         BorderColor     =   &H0000FFFF&
         X1              =   1800
         X2              =   1800
         Y1              =   360
         Y2              =   1200
      End
      Begin VB.Line Line3 
         BorderColor     =   &H0000FFFF&
         X1              =   480
         X2              =   480
         Y1              =   360
         Y2              =   1200
      End
      Begin VB.Line Line4 
         BorderColor     =   &H0000FFFF&
         X1              =   1560
         X2              =   1560
         Y1              =   360
         Y2              =   1200
      End
      Begin VB.Shape Shape4 
         BackColor       =   &H00C0C0C0&
         BackStyle       =   1  'Opaque
         BorderStyle     =   0  'Transparent
         Height          =   495
         Left            =   240
         Top             =   720
         Width           =   1575
      End
      Begin VB.Shape Shape5 
         BorderColor     =   &H000080FF&
         Height          =   255
         Left            =   480
         Shape           =   2  'Oval
         Top             =   1080
         Width           =   1095
      End
      Begin VB.Shape Shape6 
         BackColor       =   &H00C0C0C0&
         BackStyle       =   1  'Opaque
         BorderStyle     =   0  'Transparent
         Height          =   255
         Left            =   480
         Top             =   960
         Width           =   1095
      End
      Begin VB.Shape Shape3 
         BorderColor     =   &H000080FF&
         Height          =   495
         Left            =   240
         Shape           =   2  'Oval
         Top             =   120
         Width           =   1575
      End
      Begin VB.Shape Shape2 
         BorderColor     =   &H000080FF&
         Height          =   255
         Left            =   480
         Shape           =   2  'Oval
         Top             =   240
         Width           =   1095
      End
      Begin VB.Shape Shape1 
         BorderColor     =   &H000080FF&
         Height          =   495
         Left            =   240
         Shape           =   2  'Oval
         Top             =   960
         Width           =   1575
      End
   End
   Begin VB.Label Trigger 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Trigger"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1560
      TabIndex        =   17
      Top             =   4680
      Visible         =   0   'False
      Width           =   612
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Inner Radius"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   120
      TabIndex        =   14
      Top             =   3240
      Width           =   972
   End
   Begin VB.Label Label6 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Outer Radius"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   120
      TabIndex        =   16
      Top             =   2880
      Width           =   972
   End
   Begin VB.Label Label7 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Height"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   600
      TabIndex        =   15
      Top             =   2520
      Width           =   492
   End
   Begin VB.Label Label9 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
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
      Height          =   252
      Left            =   600
      TabIndex        =   13
      Top             =   3600
      Width           =   492
   End
   Begin VB.Label Label10 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Group Name"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   120
      TabIndex        =   12
      Top             =   3960
      Width           =   972
   End
   Begin VB.Label Label11 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Item Names are: Outer, Inner, Top, Base"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   492
      Left            =   0
      TabIndex        =   11
      Top             =   4320
      Width           =   2292
   End
End
Attribute VB_Name = "frmParSolTube"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Build_Click()
    Dim Outer, Inner, THeight, NumSides As Integer
    Dim Group As String
    Dim Hollow
    Dim Angle, AngleInc, NextAngle
    Dim HalfAngle
    Dim StartAngle
    Dim i
    Dim Temp As Double

    Dim jend As Integer
    Dim SideCounter As Integer
    Dim Breakpoint As Integer

    Dim N As Integer
    Dim V As Integer
    Dim Pi
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single
    
    
    Call InitBrush("Cylinder")
    
    '
    ' Validate parameters
    '
    If Not Eval(txtOuterRadius, Temp) Then Exit Sub
    Outer = Int(Temp)
    '
    If Not Eval(txtInnerRadius, Temp) Then Exit Sub
    Inner = Int(Temp)
    '
    If Not Eval(txtTubeHeight, Temp) Then Exit Sub
    THeight = Int(Temp)
    '
    If Not Eval(txtSides, Temp) Then Exit Sub
    NumSides = Int(Temp)
    '
    Group = UCase$(frmParSolTube.txtGroup)
    Hollow = Int(frmParSolTube.optHollow.Value)
    
    If (Not Hollow) And (NumSides > 100) Then
        MsgBox "You must have less than 100 Sides for Solid Tube"
        Exit Sub
    End If
    If (Hollow) And (NumSides > 45) Then
        MsgBox "You must have less than 45 Sides for Hollow Tube"
        Exit Sub
    End If
    If (Hollow) And (Inner >= Outer) Then
        MsgBox "The Inner Radius must be smaller than the Outer Radius"
        Exit Sub
    End If
    '
    ' Build it: Setup
    '
    N = 0
    Pi = 4 * Atn(1)
    AngleInc = 2 * Pi / NumSides
    HalfAngle = AngleInc / 2
    StartAngle = AngleInc / 2
    
    'Adjust radius for side alignment
    If Me.chkAlignSide.Value = 1 Then '1 = checked 0 = Unchecked
        Inner = 1 / (Cos(HalfAngle) / Inner)
        Outer = 1 / (Cos(HalfAngle) / Outer)
    End If
    
    
    
    '
    'Init Outer Start Position
    '
    CurrentX = Outer
    CurrentY = 0
    CurrentZ = -THeight / 2 ' Start at the bottom
    '
    ' The loop: Build the Sides
    '
    Angle = StartAngle
    V = 0
    For i = 1 To NumSides
        CurrentX = Outer * Cos(Angle)
        CurrentY = Outer * Sin(Angle)

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Outer"
            Brush.NumPolys = NumSides
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i < NumSides Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If
            
            Call PutVertex(N, 4, CurrentX, CurrentY, CurrentZ)

            CurrentZ = THeight / 2 ' Up
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ)

            CurrentX = Outer * Cos(NextAngle) '
            CurrentY = Outer * Sin(NextAngle) 'Over
            Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ)

            CurrentX = Outer * Cos(NextAngle) '
            CurrentY = Outer * Sin(NextAngle) 'Down
            CurrentZ = -THeight / 2                 '
            Call PutVertex(N, 1, CurrentX, CurrentY, CurrentZ)

        Angle = Angle + AngleInc

    Next i 'end outer

    CurrentZ = -THeight / 2 ' Start at the bottom
    If Not Hollow Then

        '
        ' The loop: Build the Bottom
        '
        If NumSides > 12 Then
            Angle = StartAngle
            SideCounter = NumSides
            Breakpoint = 6
            Do While SideCounter > 0

                N = N + 1               ' Init a new Polygon
                InitBrushPoly (N)       '
                Brush.Polys(N).Group = Group  '
                Brush.Polys(N).Item = "Base"  '
                Brush.NumPolys = N       '

                If SideCounter >= Breakpoint Then
                    jend = Breakpoint '
                Else
                    jend = SideCounter + 1 '
                End If

                V = jend + 1
                Brush.Polys(N).NumVertices = V
                For i = 1 To jend
                    CurrentX = Outer * Cos(Angle)
                    CurrentY = Outer * Sin(Angle)
                    Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                    V = V - 1
                    If i < jend Then Angle = Angle + AngleInc
                    If i < jend Then SideCounter = SideCounter - 1
                Next i

                CurrentX = 0
                CurrentY = 0
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                V = V - 1
            Loop

            CurrentZ = THeight / 2 ' Do the Solid top
            SideCounter = NumSides
            Angle = StartAngle
            Breakpoint = 6
            Do While SideCounter > 0

                N = N + 1               ' Init a new Polygon
                InitBrushPoly (N)       '
                Brush.Polys(N).Group = Group  '
                Brush.Polys(N).Item = "Base"  '
                Brush.NumPolys = N       '

                If SideCounter >= Breakpoint Then
                    jend = Breakpoint '
                Else
                    jend = SideCounter + 1 '
                End If

                V = 1
                For i = 1 To jend
                    Brush.Polys(N).NumVertices = V
                    CurrentX = Outer * Cos(Angle)
                    CurrentY = Outer * Sin(Angle)
                    Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                    V = V + 1
                    If i <> jend Then Angle = Angle + AngleInc
                    If i <> jend Then SideCounter = SideCounter - 1
                Next i
                
                Brush.Polys(N).NumVertices = V
                CurrentX = 0
                CurrentY = 0
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
            Loop


        
        End If ' End Solid and  > 12 sides

        If NumSides <= 12 Then
            '
            ' The loop: Build the Bottom
            '
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Base"
            Brush.NumPolys = N
            Brush.Polys(N).NumVertices = NumSides
            V = NumSides + 1
            Angle = StartAngle
            For i = 1 To NumSides
                CurrentX = Outer * Cos(Angle)
                CurrentY = Outer * Sin(Angle)
                V = V - 1
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                Angle = Angle + AngleInc
            Next i
            '
            ' The loop: Build the Top
            '
            V = 0
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Top"
            Brush.NumPolys = N
            Brush.Polys(N).NumVertices = NumSides
            Angle = StartAngle
            CurrentZ = THeight / 2 ' Start at the Top
            For i = 1 To NumSides
                CurrentX = Outer * Cos(Angle)
                CurrentY = Outer * Sin(Angle)
                V = V + 1
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                Angle = Angle + AngleInc
            Next i

        End If 'End Solid and < 12 sides

    End If 'End Not Hollow

   
    If Hollow Then
        '
        'Init Inner Start Position
        '
        CurrentX = Inner
        CurrentY = 0
        CurrentZ = -THeight / 2 ' Start at the bottom
        Angle = StartAngle
        '
        ' The loop: Build the Sides
        '
        V = 0
        For i = 1 To NumSides
            CurrentX = Inner * Cos(Angle)
            CurrentY = Inner * Sin(Angle)
            
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Inner"
            Brush.NumPolys = NumSides
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i < NumSides Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If
            Call PutVertex(N, 1, CurrentX, CurrentY, CurrentZ)

            CurrentZ = THeight / 2 ' Up
            Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ)

            CurrentX = Inner * Cos(NextAngle) '
            CurrentY = Inner * Sin(NextAngle) 'Over
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ)

            CurrentX = Inner * Cos(NextAngle) '
            CurrentY = Inner * Sin(NextAngle) 'Down
            CurrentZ = -THeight / 2                 '
            Call PutVertex(N, 4, CurrentX, CurrentY, CurrentZ)
 
            Angle = Angle + AngleInc

        Next i
        '
        'Build Hollow Bottom from Outer and Inner frames
        '
        For i = 1 To NumSides
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Bottom"
            Brush.NumPolys = NumSides
            Brush.Polys(N).NumVertices = 4 ' make rectangles
            Call PutVertex(N, 1, Brush.Polys(i).Vertex(4).X, Brush.Polys(i).Vertex(4).Y, Brush.Polys(i).Vertex(4).Z)
            Call PutVertex(N, 2, Brush.Polys(i + NumSides).Vertex(1).X, Brush.Polys(i + NumSides).Vertex(1).Y, Brush.Polys(i + NumSides).Vertex(1).Z)
            Call PutVertex(N, 3, Brush.Polys(i + NumSides).Vertex(4).X, Brush.Polys(i + NumSides).Vertex(4).Y, Brush.Polys(i + NumSides).Vertex(4).Z)
            Call PutVertex(N, 4, Brush.Polys(i).Vertex(1).X, Brush.Polys(i).Vertex(1).Y, Brush.Polys(i).Vertex(1).Z)
        Next i
        '
        'Build Hollow Top from Outer and Inner frames
        '
        For i = 1 To NumSides
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Top"
            Brush.NumPolys = NumSides
            Brush.Polys(N).NumVertices = 4 ' make rectangles
            Call PutVertex(N, 4, Brush.Polys(i).Vertex(3).X, Brush.Polys(i).Vertex(3).Y, Brush.Polys(i).Vertex(3).Z)
            Call PutVertex(N, 3, Brush.Polys(i + NumSides).Vertex(2).X, Brush.Polys(i + NumSides).Vertex(2).Y, Brush.Polys(i + NumSides).Vertex(2).Z)
            Call PutVertex(N, 2, Brush.Polys(i + NumSides).Vertex(3).X, Brush.Polys(i + NumSides).Vertex(3).Y, Brush.Polys(i + NumSides).Vertex(3).Z)
            Call PutVertex(N, 1, Brush.Polys(i).Vertex(2).X, Brush.Polys(i).Vertex(2).Y, Brush.Polys(i).Vertex(2).Z)
        Next i

    End If 'End Hollow

    Brush.NumPolys = N
    Call SendBrush(0)
    Call Ed.StatusText("Built a Tube")
End Sub

Private Sub Command1_Click()
    ToolHelp (156)
End Sub

Private Sub Command2_Click()
    Hide
End Sub

Private Sub DrawLines()
    '
    Dim Hidden, ShowIt, Showcr As Long
    Hidden = &H808080
    ShowIt = &HFFFF&
    Showcr = &H80FF&
    '
    If (optSolid) Then
        Shape2.BorderColor = Hidden
        Shape5.BorderColor = Hidden
        Line3.BorderColor = Hidden
        Line4.BorderColor = Hidden
    Else
        Shape2.BorderColor = Showcr
        Shape5.BorderColor = Showcr
        Line3.BorderColor = ShowIt
        Line4.BorderColor = ShowIt
    End If
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "BuildTube", TOP_NORMAL)
    DrawLines
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub optHollow_Click()
    DrawLines
End Sub

Private Sub Option1_Click()

End Sub

Private Sub optSolid_Click()
    DrawLines
End Sub

Private Sub Trigger_Change()
    Build_Click
End Sub

'
' Focus change highlighting routines.
'
Private Sub txtTubeHeight_GotFocus()
    SelectAll txtTubeHeight
End Sub

Private Sub txtOuterRadius_GotFocus()
    SelectAll txtOuterRadius
End Sub

Private Sub txtInnerRadius_GotFocus()
    SelectAll txtInnerRadius
End Sub

Private Sub txtSides_GotFocus()
    SelectAll txtSides
End Sub

Private Sub txtGroup_GotFocus()
    SelectAll txtGroup
End Sub

