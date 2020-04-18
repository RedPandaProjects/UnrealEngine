VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmParSolCone 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build a Cone/Spire"
   ClientHeight    =   6795
   ClientLeft      =   1695
   ClientTop       =   1200
   ClientWidth     =   2850
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
   HelpContextID   =   150
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6795
   ScaleWidth      =   2850
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
      TabIndex        =   12
      Top             =   3120
      Value           =   1  'Checked
      Width           =   1572
   End
   Begin Threed.SSPanel SSPanel1 
      Height          =   2535
      Left            =   120
      TabIndex        =   21
      Top             =   120
      Width           =   2655
      _Version        =   65536
      _ExtentX        =   4683
      _ExtentY        =   4471
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
      Begin VB.Line Line8 
         X1              =   720
         X2              =   720
         Y1              =   1560
         Y2              =   1920
      End
      Begin VB.Line Line9 
         X1              =   1320
         X2              =   720
         Y1              =   1920
         Y2              =   1920
      End
      Begin VB.Line Line10 
         X1              =   1320
         X2              =   1320
         Y1              =   1560
         Y2              =   2040
      End
      Begin VB.Line Line11 
         X1              =   2400
         X2              =   2400
         Y1              =   1560
         Y2              =   2040
      End
      Begin VB.Line Line12 
         X1              =   1320
         X2              =   2400
         Y1              =   2040
         Y2              =   2040
      End
      Begin VB.Line Line14 
         X1              =   120
         X2              =   240
         Y1              =   1560
         Y2              =   1560
      End
      Begin VB.Line Line15 
         X1              =   120
         X2              =   120
         Y1              =   1560
         Y2              =   120
      End
      Begin VB.Line Line16 
         X1              =   120
         X2              =   1200
         Y1              =   120
         Y2              =   120
      End
      Begin VB.Line Line17 
         X1              =   1440
         X2              =   1920
         Y1              =   120
         Y2              =   120
      End
      Begin VB.Line Line18 
         X1              =   1920
         X2              =   1800
         Y1              =   960
         Y2              =   960
      End
      Begin VB.Line Line19 
         X1              =   1920
         X2              =   1920
         Y1              =   120
         Y2              =   960
      End
      Begin VB.Label Label1 
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Cap Height"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   495
         Left            =   2040
         TabIndex        =   25
         Top             =   120
         Width           =   615
      End
      Begin VB.Label Label2 
         BackColor       =   &H00C0C0C0&
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
         Left            =   1320
         TabIndex        =   24
         Top             =   2160
         Width           =   1215
      End
      Begin VB.Label Label3 
         BackColor       =   &H00C0C0C0&
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
         Left            =   240
         TabIndex        =   23
         Top             =   2040
         Width           =   1095
      End
      Begin VB.Label Label4 
         BackColor       =   &H00C0C0C0&
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
         Height          =   255
         Left            =   240
         TabIndex        =   22
         Top             =   600
         Width           =   615
      End
      Begin VB.Shape Shape1 
         BorderColor     =   &H0000FFFF&
         Height          =   495
         Left            =   240
         Shape           =   2  'Oval
         Top             =   1320
         Width           =   2175
      End
      Begin VB.Shape Shape2 
         BorderColor     =   &H000080FF&
         Height          =   255
         Left            =   720
         Shape           =   2  'Oval
         Top             =   1440
         Width           =   1215
      End
      Begin VB.Shape Shape3 
         BorderColor     =   &H000080FF&
         Height          =   255
         Left            =   960
         Shape           =   2  'Oval
         Top             =   840
         Width           =   735
      End
      Begin VB.Line Line7 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   1680
         Y1              =   960
         Y2              =   960
      End
      Begin VB.Line Line1 
         BorderColor     =   &H0000FFFF&
         X1              =   1320
         X2              =   240
         Y1              =   120
         Y2              =   1560
      End
      Begin VB.Line Line2 
         BorderColor     =   &H0000FFFF&
         X1              =   1320
         X2              =   2400
         Y1              =   120
         Y2              =   1560
      End
      Begin VB.Line Line20 
         BorderColor     =   &H0000FFFF&
         X1              =   720
         X2              =   1920
         Y1              =   1560
         Y2              =   1560
      End
      Begin VB.Line Line4 
         BorderColor     =   &H0000FFFF&
         X1              =   720
         X2              =   960
         Y1              =   1560
         Y2              =   960
      End
      Begin VB.Line Line5 
         BorderColor     =   &H0000FFFF&
         X1              =   1680
         X2              =   1920
         Y1              =   960
         Y2              =   1560
      End
      Begin VB.Line Line21 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   1320
         Y1              =   960
         Y2              =   120
      End
      Begin VB.Line Line22 
         BorderColor     =   &H0000FFFF&
         X1              =   1680
         X2              =   1320
         Y1              =   960
         Y2              =   120
      End
      Begin VB.Line Line3 
         BorderColor     =   &H0000FFFF&
         X1              =   720
         X2              =   240
         Y1              =   1560
         Y2              =   1560
      End
      Begin VB.Line Line6 
         BorderColor     =   &H0000FFFF&
         X1              =   2400
         X2              =   1920
         Y1              =   1560
         Y2              =   1560
      End
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
      Left            =   1080
      TabIndex        =   7
      Top             =   6360
      Width           =   615
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
      Left            =   1200
      TabIndex        =   5
      Text            =   "Cone"
      Top             =   5400
      Width           =   1455
   End
   Begin VB.OptionButton optCapped 
      Caption         =   "Capped"
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
      Left            =   1920
      TabIndex        =   11
      Top             =   2760
      Width           =   975
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
      Left            =   960
      TabIndex        =   10
      Top             =   2760
      Width           =   855
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
      TabIndex        =   9
      Top             =   2760
      Value           =   -1  'True
      Width           =   735
   End
   Begin VB.CommandButton Command3 
      BackColor       =   &H00C0C0C0&
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
      TabIndex        =   8
      Top             =   6360
      Width           =   855
   End
   Begin VB.CommandButton Build 
      BackColor       =   &H00C0C0C0&
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
      TabIndex        =   6
      Top             =   6360
      Width           =   855
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
      TabIndex        =   4
      Text            =   "8"
      Top             =   4920
      Width           =   1455
   End
   Begin VB.TextBox txtCapHeight 
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
      Text            =   "256"
      Top             =   3840
      Width           =   1455
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
      TabIndex        =   2
      Text            =   "512"
      Top             =   4200
      Width           =   1455
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
      TabIndex        =   3
      Text            =   "512-16"
      Top             =   4560
      Width           =   1455
   End
   Begin VB.TextBox txtHeight 
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
      Text            =   "512"
      Top             =   3480
      Width           =   1455
   End
   Begin VB.Label Trigger 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Trigger"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   2040
      TabIndex        =   20
      Top             =   6000
      Visible         =   0   'False
      Width           =   612
   End
   Begin VB.Label Label11 
      Alignment       =   2  'Center
      BackColor       =   &H00C0C0C0&
      BackStyle       =   0  'Transparent
      Caption         =   "Item Names are: Outer, Base, Inner, Cap"
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
      Left            =   360
      TabIndex        =   19
      Top             =   5760
      Width           =   2292
   End
   Begin VB.Label Label10 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
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
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   120
      TabIndex        =   18
      Top             =   5400
      Width           =   972
   End
   Begin VB.Label Label9 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
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
      ForeColor       =   &H00000000&
      Height          =   252
      Left            =   600
      TabIndex        =   17
      Top             =   4920
      Width           =   492
   End
   Begin VB.Label Label8 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
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
      ForeColor       =   &H00000000&
      Height          =   252
      Left            =   120
      TabIndex        =   16
      Top             =   4560
      Width           =   972
   End
   Begin VB.Label Label7 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
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
      ForeColor       =   &H00000000&
      Height          =   252
      Left            =   600
      TabIndex        =   15
      Top             =   3480
      Width           =   492
   End
   Begin VB.Label Label6 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
      BackStyle       =   0  'Transparent
      Caption         =   "Cap Height"
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
      Height          =   252
      Left            =   240
      TabIndex        =   14
      Top             =   3840
      Width           =   852
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
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
      ForeColor       =   &H00000000&
      Height          =   252
      Left            =   120
      TabIndex        =   13
      Top             =   4200
      Width           =   972
   End
End
Attribute VB_Name = "frmParSolCone"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Build_Click()
    Dim Outer, Inner, CHeight, NumSides As Integer
    Dim CapRadius As Integer '
    Dim CapHeight As Integer
    Dim Group As String
    Dim Hollow, CSolid, Capped
    Dim Angle, AngleInc, NextAngle, StartAngle, HalfAngle
    Dim Temp As Double
    Dim i           'Generic Loop
    
    Dim jend As Integer
    Dim SideCounter As Integer
    Dim Breakpoint As Integer
    
    Dim N As Integer  'Polygon Counter
    Dim V As Integer  'Vertex Counter
    Dim Pi
    
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single
    
    Call InitBrush("Cone")
    
    '
    ' Validate parameters
    '
    If Not Eval(txtOuterRadius, Temp) Then Exit Sub
    Outer = Int(Temp)
    '
    If Not Eval(txtInnerRadius, Temp) Then Exit Sub
    Inner = Int(Temp)
    '
    If Not Eval(txtHeight, Temp) Then Exit Sub
    CHeight = Int(Temp)
    '
    If Not Eval(txtCapHeight, Temp) Then Exit Sub
    CapHeight = Int(Temp)
    '
    If Not Eval(txtSides, Temp) Then Exit Sub
    NumSides = Int(Temp)
    '
    Group = UCase$(frmParSolCone.Group)
    CSolid = Int(frmParSolCone.optSolid.Value)
    Hollow = Int(frmParSolCone.optHollow.Value)
    Capped = Int(frmParSolCone.optCapped.Value)
    '
    If CSolid And (NumSides > 210) Then
        MsgBox "Number of Sides Cannot exceed 210 on Solid Cone."
        Exit Sub
    End If
    If Hollow And (NumSides > 80) Then
        MsgBox "Number of Sides Cannot exceed 80 on Hollow Cone."
        Exit Sub
    End If
    If Capped And (NumSides > 50) Then
        MsgBox "Number of Sides Cannot exceed 70 on Capped Cone."
        Exit Sub
    End If
    
    If (Outer <= 0) Or (CHeight <= 0) Or (NumSides <= 0) Then
        MsgBox "Please use positive numbers greater than 0."
        Exit Sub
    End If
    If (Hollow Or Capped) And (Inner >= Outer) Then
        MsgBox "Outer Radius must be larger than Inner Radius."
        Exit Sub
    End If
    If (Hollow Or Capped) And (Inner <= 0) Then
        MsgBox "Inner Radius Must be Larger than 0."
        Exit Sub
    End If
    If (Hollow Or Capped) And (CapHeight <= 0) Then
        MsgBox "Cap Height Must be Larger than 0."
        Exit Sub
    End If

    If (Hollow Or Capped) And (CapHeight >= CHeight) Then
        MsgBox "Cap Height must be smaller than the Cone Height"
        Exit Sub
    End If
    '
    ' Build it: Setup
    '
    N = 0
    Pi = 4 * Atn(1)
    AngleInc = 2 * Pi / NumSides
    StartAngle = AngleInc / 2
    HalfAngle = AngleInc / 2
    
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
    CurrentZ = -CHeight / 2 ' Start at the bottom
    Angle = StartAngle
    '
    ' The loop: Build the Sides
    '
    V = 0

    For i = 1 To NumSides
        '
        CurrentX = Outer * Cos(Angle)
        CurrentY = Outer * Sin(Angle)
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Outer"
        Brush.Polys(N).NumVertices = 3 'Triangles

            If i < NumSides Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If
        
        Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ)

        CurrentX = 0
        CurrentY = 0
        CurrentZ = CHeight / 2 ' Tip
        Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ)

        CurrentX = Outer * Cos(NextAngle) 'Angle + AngleInc)'
        CurrentY = Outer * Sin(NextAngle) 'Angle + AngleInc)'Over
        CurrentZ = -CHeight / 2
        Call PutVertex(N, 1, CurrentX, CurrentY, CurrentZ)

        Angle = Angle + AngleInc

    Next i 'end outer
    
    If CSolid Then
        '
        ' The loop: Build the Bottom
        '
        'Angle = StartAngle
        If NumSides > 12 Then
            SideCounter = NumSides

            Breakpoint = 6 ' figure it out later for bigger numbers
                            ' Actually after a certain point, you don't
                            'need to worry about it! they will all be convex! yee ha!


            Do While SideCounter > 0
                N = N + 1               ' Init a new Polygon
                InitBrushPoly (N)       '
                Brush.Polys(N).Group = Group  '
                Brush.Polys(N).Item = "Base"  '
                
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

        End If

        If NumSides <= 12 Then
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Base"
            Brush.Polys(N).NumVertices = NumSides
            
            V = NumSides + 1
            For i = 1 To NumSides
                CurrentX = Outer * Cos(Angle)
                CurrentY = Outer * Sin(Angle)
                V = V - 1
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                
                If i < NumSides Then
                    Angle = Angle + AngleInc
                Else
                    Angle = 0
                End If
            Next i

        End If

    End If ' end solid bottom

    If Capped Then
        '
        ' Build the insides
        ' Build the Cap Bottom
        '
        CapRadius = Inner * (CapHeight / CHeight)
        '
        ' The loop: Build the Sides
        '
        For i = 1 To NumSides
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Inner"
            Brush.Polys(N).NumVertices = 4 'Rectangle

            If i < NumSides Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If

            CurrentX = Inner * Cos(Angle)
            CurrentY = Inner * Sin(Angle)
            CurrentZ = -CHeight / 2
            Call PutVertex(N, 1, CurrentX, CurrentY, CurrentZ)
            
            CurrentX = CapRadius * Cos(Angle)
            CurrentY = CapRadius * Sin(Angle)
            CurrentZ = (CHeight / 2) - CapHeight
            Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ)

            CurrentX = CapRadius * Cos(NextAngle) 'Angle + AngleInc)
            CurrentY = CapRadius * Sin(NextAngle) 'Angle + AngleInc)
            CurrentZ = (CHeight / 2) - CapHeight
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ)
            
            CurrentX = Inner * Cos(NextAngle) 'Angle + AngleInc)
            CurrentY = Inner * Sin(NextAngle) 'Angle + AngleInc)
            CurrentZ = -CHeight / 2
            Call PutVertex(N, 4, CurrentX, CurrentY, CurrentZ)


        Angle = Angle + AngleInc

    Next i 'end inner with cap

        '
        ' The loop: Build the Cap Bottom
        '
        If NumSides <= 12 Then
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Cap"
            Brush.Polys(N).NumVertices = NumSides
            V = NumSides + 1

            CurrentZ = (CHeight / 2) - CapHeight
            
            For i = 1 To NumSides
            CurrentX = CapRadius * Cos(Angle)
            CurrentY = CapRadius * Sin(Angle)
                
                V = V - 1
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                Angle = Angle + AngleInc
            Next i

        End If

        If NumSides > 12 Then
            CurrentZ = (CHeight / 2) - CapHeight
            SideCounter = NumSides
            Breakpoint = 6

            Do While SideCounter > 0
                N = N + 1               ' Init a new Polygon
                InitBrushPoly (N)       '
                Brush.Polys(N).Group = Group  '
                Brush.Polys(N).Item = "Cap"   '

                If SideCounter >= Breakpoint Then
                    jend = Breakpoint '
                Else
                    jend = SideCounter + 1 '
                End If

                V = jend + 1
                Brush.Polys(N).NumVertices = V '!!
                For i = 1 To jend
                    CurrentX = CapRadius * Cos(Angle)
                    CurrentY = CapRadius * Sin(Angle)
                    Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                    V = V - 1
                    If i <> jend Then Angle = Angle + AngleInc
                    If i <> jend Then SideCounter = SideCounter - 1
                Next i

                CurrentX = 0
                CurrentY = 0
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                V = V - 1
            Loop

        End If

        For i = 1 To NumSides 'Make hollow bottom
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Bottom"
            Brush.Polys(N).NumVertices = 4 ' make Trapezoids
            Call PutVertex(N, 1, Brush.Polys(i).Vertex(3).X, Brush.Polys(i).Vertex(3).Y, Brush.Polys(i).Vertex(3).Z)
            Call PutVertex(N, 2, Brush.Polys(i + NumSides).Vertex(1).X, Brush.Polys(i + NumSides).Vertex(1).Y, Brush.Polys(i + NumSides).Vertex(1).Z)
            Call PutVertex(N, 3, Brush.Polys(i + NumSides).Vertex(4).X, Brush.Polys(i + NumSides).Vertex(4).Y, Brush.Polys(i + NumSides).Vertex(4).Z)
            Call PutVertex(N, 4, Brush.Polys(i).Vertex(1).X, Brush.Polys(i).Vertex(1).Y, Brush.Polys(i).Vertex(1).Z)
        Next i

    End If ' End Cap

    If Hollow Then
        '
        'Init Inner Start Position
        '
        CurrentX = Inner * Cos(Angle)
        CurrentY = Inner * Sin(Angle)
        CurrentZ = -CHeight / 2 ' Start at the bottom
        '
        ' The loop: Build the Sides
        '
        For i = 1 To NumSides
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Inner"
            Brush.Polys(N).NumVertices = 3 'Triangles

            Call PutVertex(N, 1, CurrentX, CurrentY, CurrentZ)

            CurrentX = 0
            CurrentY = 0
            CurrentZ = (CHeight / 2) - CapHeight ' Tip
            Call PutVertex(N, 2, CurrentX, CurrentY, CurrentZ)

            CurrentX = Inner * Cos(Angle + AngleInc) '
            CurrentY = Inner * Sin(Angle + AngleInc) 'Over
            CurrentZ = -CHeight / 2
            Call PutVertex(N, 3, CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc
        Next i 'end inner
        '
        'Build Hollow Bottom from Outer and Inner frames
        '
        For i = 1 To NumSides
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Bottom"
            Brush.Polys(N).NumVertices = 4 ' make Trapezoids
            Call PutVertex(N, 1, Brush.Polys(i).Vertex(3).X, Brush.Polys(i).Vertex(3).Y, Brush.Polys(i).Vertex(3).Z)
            Call PutVertex(N, 2, Brush.Polys(i + NumSides).Vertex(1).X, Brush.Polys(i + NumSides).Vertex(1).Y, Brush.Polys(i + NumSides).Vertex(1).Z)
            Call PutVertex(N, 3, Brush.Polys(i + NumSides).Vertex(3).X, Brush.Polys(i + NumSides).Vertex(3).Y, Brush.Polys(i + NumSides).Vertex(3).Z)
            Call PutVertex(N, 4, Brush.Polys(i).Vertex(1).X, Brush.Polys(i).Vertex(1).Y, Brush.Polys(i).Vertex(1).Z)
        Next i

    End If ' End Hollow

    Brush.NumPolys = N
    
    For N = 1 To Brush.NumPolys

        'Debug.Print "Solid: "; n, " Vertices: "; Brush.Polys(n).NumVertices
        For i = 1 To Brush.Polys(N).NumVertices
            'Debug.Print "Vertex: "; i, " X:"; Brush.Polys(n).Vertex(i).X, " Y:"; Brush.Polys(n).Vertex(i).Y, " Z:"; Brush.Polys(n).Vertex(i).Z
        Next i
        'Debug.Print
    Next N

    Call SendBrush(0)
    Call Ed.StatusText("Built Cone")

End Sub


Private Sub Command1_Click()
    ToolHelp (150)
End Sub

Private Sub Command3_Click()
    Hide
End Sub

Private Sub DrawLines()
    Dim Hidden, ShowIt, Showcr As Long
    '
    Hidden = &H808080
    ShowIt = &HFFFF&
    Showcr = &H80FF&
    '
    If (optSolid.Value) Then
        Line4.BorderColor = Hidden
        Line5.BorderColor = Hidden
        Line7.BorderColor = Hidden
        Line20.BorderColor = ShowIt
        Line21.BorderColor = Hidden
        Line22.BorderColor = Hidden
        Shape2.BorderColor = Hidden
        Shape3.BorderColor = Hidden
    ElseIf (optHollow.Value) Then
        Line4.BorderColor = ShowIt
        Line5.BorderColor = ShowIt
        Line7.BorderColor = Hidden
        Line20.BorderColor = Hidden
        Line21.BorderColor = ShowIt
        Line22.BorderColor = ShowIt
        Shape2.BorderColor = Showcr
        Shape3.BorderColor = Hidden
    ElseIf (optCapped.Value) Then
        Line4.BorderColor = ShowIt
        Line5.BorderColor = ShowIt
        Line7.BorderColor = ShowIt
        Line20.BorderColor = Hidden
        Line21.BorderColor = Hidden
        Line22.BorderColor = Hidden
        Shape2.BorderColor = Showcr
        Shape3.BorderColor = Showcr
    End If
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "BuildCone", TOP_NORMAL)
    DrawLines
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub optCapped_Click()
    DrawLines
End Sub

Private Sub optHollow_Click()
    DrawLines
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
Private Sub txtHeight_GotFocus()
    SelectAll txtHeight
End Sub

Private Sub txtCapHeight_GotFocus()
    SelectAll txtCapHeight
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

Private Sub Group_GotFocus()
    SelectAll Group
End Sub
