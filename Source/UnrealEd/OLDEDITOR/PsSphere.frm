VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmParSolSphere 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build a Sphere/Dome"
   ClientHeight    =   5475
   ClientLeft      =   6930
   ClientTop       =   2715
   ClientWidth     =   2640
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
   ForeColor       =   &H00C0C0C0&
   HelpContextID   =   154
   Icon            =   "PsSphere.frx":0000
   LinkTopic       =   "Form5"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5475
   ScaleWidth      =   2640
   ShowInTaskbar   =   0   'False
   Begin Threed.SSPanel SSPanel1 
      Height          =   2175
      Left            =   120
      TabIndex        =   17
      Top             =   120
      Width           =   2415
      _Version        =   65536
      _ExtentX        =   4260
      _ExtentY        =   3836
      _StockProps     =   15
      ForeColor       =   -2147483640
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
      Begin VB.Shape Shape7 
         BackColor       =   &H00808080&
         BorderColor     =   &H000080FF&
         Height          =   255
         Left            =   360
         Shape           =   2  'Oval
         Top             =   1560
         Width           =   1695
      End
      Begin VB.Shape Shape2 
         BackColor       =   &H00808080&
         BorderColor     =   &H0000C0C0&
         Height          =   1455
         Left            =   360
         Shape           =   2  'Oval
         Top             =   360
         Width           =   1695
      End
      Begin VB.Shape Shape6 
         BackColor       =   &H00808080&
         BorderColor     =   &H000080FF&
         Height          =   255
         Left            =   360
         Shape           =   2  'Oval
         Top             =   360
         Width           =   1695
      End
      Begin VB.Shape Shape3 
         BackColor       =   &H00808080&
         BorderColor     =   &H000080FF&
         Height          =   1935
         Left            =   480
         Shape           =   2  'Oval
         Top             =   120
         Width           =   1695
      End
      Begin VB.Shape Shape4 
         BackColor       =   &H00808080&
         BorderColor     =   &H000080FF&
         Height          =   1935
         Left            =   960
         Shape           =   2  'Oval
         Top             =   120
         Width           =   735
      End
      Begin VB.Shape Shape5 
         BackColor       =   &H00808080&
         BorderColor     =   &H000080FF&
         Height          =   495
         Left            =   120
         Shape           =   2  'Oval
         Top             =   840
         Width           =   2175
      End
      Begin VB.Shape Shape1 
         BackColor       =   &H00808080&
         BorderColor     =   &H0000FFFF&
         Height          =   1935
         Left            =   120
         Shape           =   2  'Oval
         Top             =   120
         Width           =   2175
      End
   End
   Begin VB.CommandButton Help 
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
      Left            =   960
      TabIndex        =   6
      Top             =   5040
      Width           =   735
   End
   Begin VB.OptionButton IsSolid 
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
      Left            =   720
      TabIndex        =   8
      Top             =   2400
      Value           =   -1  'True
      Width           =   735
   End
   Begin VB.OptionButton IsHollow 
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
      Left            =   1560
      TabIndex        =   9
      Top             =   2400
      Width           =   855
   End
   Begin VB.TextBox Group 
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
      Left            =   1440
      TabIndex        =   4
      Text            =   "Dome"
      Top             =   4200
      Width           =   1095
   End
   Begin VB.TextBox InnerRadius 
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
      Left            =   1440
      TabIndex        =   1
      Text            =   "512-16"
      Top             =   3120
      Width           =   1095
   End
   Begin VB.TextBox RadialStripes 
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
      Left            =   1440
      TabIndex        =   2
      Text            =   "8"
      Top             =   3480
      Width           =   1095
   End
   Begin VB.TextBox VerticalStripes 
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
      Left            =   1440
      TabIndex        =   3
      Text            =   "4"
      Top             =   3840
      Width           =   1095
   End
   Begin VB.TextBox OuterRadius 
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
      Left            =   1440
      TabIndex        =   0
      Text            =   "512"
      Top             =   2760
      Width           =   1095
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
      Top             =   5040
      Width           =   735
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
      Left            =   1800
      TabIndex        =   7
      Top             =   5040
      Width           =   735
   End
   Begin VB.Label Trigger 
      BackColor       =   &H00FFFFFF&
      Caption         =   "Trigger"
      Height          =   255
      Left            =   360
      TabIndex        =   16
      Top             =   4800
      Visible         =   0   'False
      Width           =   615
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
      Height          =   255
      Left            =   240
      TabIndex        =   15
      Top             =   4200
      Width           =   1095
   End
   Begin VB.Label Label8 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Vertical Stripes"
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
      Left            =   0
      TabIndex        =   14
      Top             =   3840
      Width           =   1335
   End
   Begin VB.Label Label7 
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
      Height          =   255
      Left            =   120
      TabIndex        =   13
      Top             =   2760
      Width           =   1215
   End
   Begin VB.Label Label6 
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
      Height          =   255
      Left            =   120
      TabIndex        =   12
      Top             =   3120
      Width           =   1215
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Radial Sides"
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
      TabIndex        =   11
      Top             =   3480
      Width           =   1215
   End
   Begin VB.Label Label11 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Item Names are: Outside, Inside"
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
      TabIndex        =   10
      Top             =   4560
      Width           =   2535
   End
End
Attribute VB_Name = "frmParSolSphere"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'
' Build a sphere
'
' Requirement #1:  If you're reading this as
' an example of how to make a parametric solid builder,
' then don't forget:  Your solid builder must create
' solids that are truly solid (no holes), and all points
' that are shared by adjacent polys must be 100%
' identical.  If tiny math errors creep in and your
' solid generator generates vertices that aren't
' quite equal (i.e. 10.000 and 10.001), your solid will
' have holes in it and it will do irreparable damage
' to any maps that it's added to!
'
' Requirement #2: All polygons you create must be PLANAR!
' Nothing here prevents you from creating a polygon
' with four nonplanar (or slightly nonplanar) points.
' The server will accept nonplanar polys without fuss,
' and will proceed to corrupt the world in totally
' unpredictable and frustrating ways.  Be planar to
' within the possible limits of numerical precision.
'
Private Sub Build_Click()
    Dim Outer, Inner, RStripes, VStripes As Integer
    Dim Group As String
    Dim Hollow
    Dim Angle, AngleInc, NextAngle
    Dim StartY, EndY, YInc
    Dim i, j, N, V
    Dim Pi
    Dim u1 As Single, u2 As Single
    Dim r1 As Single, r2 As Single
    Dim Temp As Double
    '
    Call InitBrush("Sphere")
    '
    ' Validate parameters
    '
    If Not Eval(OuterRadius, Temp) Then Exit Sub
    Outer = Int(Temp)
    '
    If Not Eval(InnerRadius, Temp) Then Exit Sub
    Inner = Int(Temp)
    '
    If Not Eval(RadialStripes, Temp) Then Exit Sub
    RStripes = Int(Temp)
    '
    If Not Eval(VerticalStripes, Temp) Then Exit Sub
    VStripes = Int(Temp)
    '
    Group = UCase$(frmParSolSphere.Group)
    Hollow = Int(frmParSolSphere.IsHollow.Value)
    '
    If Outer = 0 Or Inner = 0 Or RStripes = 0 Or VStripes = 0 Then
        MsgBox ("You must give all numbers, and they all must be nonzero")
        Exit Sub
    End If
    '
    If Hollow And (Inner + 10) >= Outer Then
        MsgBox ("Inner radius must be 10 larger than outer radius")
        Exit Sub
    End If
    '
    If Outer < 48 Then
        MsgBox ("Outer radius must be at least 48, preferably larger")
    End If
    '
    If RStripes < 3 Then
        MsgBox ("You need at least 3 radial stripes")
        Exit Sub
    End If
    '
    If VStripes < 2 Then
        MsgBox ("You need at least 2 vertical stripes")
        Exit Sub
    End If
    '
    ' Build it: Setup
    '
    N = 0
    Pi = 4 * Atn(1)
    Angle = 0
    AngleInc = 2 * Pi / RStripes
    YInc = StartY / VStripes
    '
    ' The loop: Build all radial stripes
    '
    For i = 1 To RStripes
        '
        ' Calculate next angle (anal to prevent
        ' precision problems from  creeping in and causing
        ' the final Angle+AngleInc to be almost, but
        ' not quite, zero.
        '
        If i < RStripes Then
            NextAngle = Angle + AngleInc
        Else
            NextAngle = 0
        End If
        '
        For j = 1 To VStripes
            '
            u1 = -1 + 2 * (j - 1) / VStripes ' -1 to 1
            u2 = -1 + 2 * (j - 0) / VStripes ' -1 to 1
            '
            r1 = Sqr(1 - u1 ^ 2)
            r2 = Sqr(1 - u2 ^ 2)
            '
            N = N + 1
            InitBrushPoly (N)
            V = 0 ' Counts vertices as we add them
            '
            ' Built vertices #1 and 2 (topmost points),
            ' which are same point at top stripe.
            '
            V = 1 ' One vertex
            Brush.Polys(N).Vertex(V).X = Outer * r1 * Cos(Angle)
            Brush.Polys(N).Vertex(V).Y = Outer * r1 * Sin(Angle)
            Brush.Polys(N).Vertex(V).Z = Outer * u1
            '
            If (j > 1) Then
                V = V + 1
                Brush.Polys(N).Vertex(V).X = Outer * r1 * Cos(NextAngle)
                Brush.Polys(N).Vertex(V).Y = Outer * r1 * Sin(NextAngle)
                Brush.Polys(N).Vertex(V).Z = Outer * u1
            End If
            '
            ' Build vertices #2 and 3 (bottom-most points),
            ' in opposite order so we travel clockwise.
            ' At bottom, these are the same point.
            '
            V = V + 1
            Brush.Polys(N).Vertex(V).X = Outer * r2 * Cos(NextAngle)
            Brush.Polys(N).Vertex(V).Y = Outer * r2 * Sin(NextAngle)
            Brush.Polys(N).Vertex(V).Z = Outer * u2
            '
            If (j < VStripes) Then
                V = V + 1
                Brush.Polys(N).Vertex(V).X = Outer * r2 * Cos(Angle)
                Brush.Polys(N).Vertex(V).Y = Outer * r2 * Sin(Angle)
                Brush.Polys(N).Vertex(V).Z = Outer * u2
            End If
            '
            Brush.Polys(N).NumVertices = V
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OUTSIDE"
            '
            If Hollow Then
                '
                ' Build inner segment
                '
                N = N + 1
                InitBrushPoly (N)
                If j = 1 Or j = VStripes Then
                    V = 3
                Else
                    V = 4
                End If
                '
                Brush.Polys(N).NumVertices = V
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "INSIDE"
                '
                ' Built vertices #1 and 2 (topmost points),
                ' which are same point at top stripe.
                '
                Brush.Polys(N).Vertex(V).X = Inner * r1 * Cos(Angle)
                Brush.Polys(N).Vertex(V).Y = Inner * r1 * Sin(Angle)
                Brush.Polys(N).Vertex(V).Z = Inner * u1
                '
                If (j > 1) Then
                    V = V - 1
                    Brush.Polys(N).Vertex(V).X = Inner * r1 * Cos(NextAngle)
                    Brush.Polys(N).Vertex(V).Y = Inner * r1 * Sin(NextAngle)
                    Brush.Polys(N).Vertex(V).Z = Inner * u1
                End If
                '
                ' Build vertices #2 and 3 (bottom-most points),
                ' in opposite order so we travel clockwise.
                ' At bottom, these are the same point.
                '
                V = V - 1
                Brush.Polys(N).Vertex(V).X = Inner * r2 * Cos(NextAngle)
                Brush.Polys(N).Vertex(V).Y = Inner * r2 * Sin(NextAngle)
                Brush.Polys(N).Vertex(V).Z = Inner * u2
                '
                If (j < VStripes) Then
                    V = V - 1
                    Brush.Polys(N).Vertex(V).X = Inner * r2 * Cos(Angle)
                    Brush.Polys(N).Vertex(V).Y = Inner * r2 * Sin(Angle)
                    Brush.Polys(N).Vertex(V).Z = Inner * u2
                End If
                '
            End If
        Next j
        '
        ' Go to next radial stripe:
        '
        Angle = Angle + AngleInc
    Next i
    '
    Brush.NumPolys = N
    Call SendBrush(0)
    Call Ed.StatusText("Built a Sphere")
End Sub

Private Sub Command2_Click()
    Hide
End Sub

Private Sub DrawLines(Mode As Integer)
    Dim Hidden, ShowIt, Showcr As Long
    '
    Hidden = &H808080
    ShowIt = &HC0C0&
    Showcr = &H80FF&
    '
    If (IsHollow.Value) Then
        Shape2.BorderColor = ShowIt: Shape2.ZOrder
    Else
        Shape2.BorderColor = Hidden
    End If
    '
    If (Mode = 0) Then
        Shape5.BorderColor = Hidden
        Shape6.BorderColor = Hidden
        Shape7.BorderColor = Hidden
        Shape3.BorderColor = Hidden
        Shape4.BorderColor = Hidden
    ElseIf (Mode = 2) Then
        Shape5.BorderColor = Showcr: Shape5.ZOrder
        Shape6.BorderColor = Showcr: Shape6.ZOrder
        Shape7.BorderColor = Showcr: Shape7.ZOrder
        Shape3.BorderColor = Hidden
        Shape4.BorderColor = Hidden
    ElseIf (Mode = 1) Then
        Shape5.BorderColor = Hidden
        Shape6.BorderColor = Hidden
        Shape7.BorderColor = Hidden
        Shape3.BorderColor = Showcr: Shape3.ZOrder
        Shape4.BorderColor = Showcr: Shape4.ZOrder
    End If
    Shape1.ZOrder
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "BuildSphere", TOP_NORMAL)
    DrawLines (0)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Help_Click()
    ToolHelp (154)
End Sub

Private Sub IsHollow_Click()
    DrawLines (0)
End Sub

Private Sub IsSolid_Click()
    DrawLines (1)
End Sub

Private Sub Label5_Click()
    DrawLines (1)
End Sub

Private Sub Label8_Click()
    DrawLines (2)
End Sub

Private Sub RadialStripes_Change()
    DrawLines (1)
End Sub

Private Sub Trigger_Change()
    Build_Click
End Sub

Private Sub VerticalStripes_Change()
    DrawLines (2)
End Sub

'
' Focus change highlighting routines.
'
Private Sub OuterRadius_GotFocus()
    SelectAll OuterRadius
End Sub

Private Sub InnerRadius_GotFocus()
    SelectAll InnerRadius
End Sub

Private Sub RadialStripes_GotFocus()
    SelectAll RadialStripes
End Sub

Private Sub VerticalStripes_GotFocus()
    SelectAll VerticalStripes
End Sub

Private Sub Group_GotFocus()
    SelectAll Group
End Sub

