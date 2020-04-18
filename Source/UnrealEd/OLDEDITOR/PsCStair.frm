VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmParSolCurvedStair 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build a Curved Stair"
   ClientHeight    =   6450
   ClientLeft      =   7170
   ClientTop       =   1875
   ClientWidth     =   2970
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
   HelpContextID   =   151
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6450
   ScaleWidth      =   2970
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
      Left            =   720
      TabIndex        =   11
      Top             =   2400
      Value           =   1  'Checked
      Width           =   1812
   End
   Begin VB.CheckBox CCWSelect 
      Caption         =   "Counter Clockwise"
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
      Left            =   720
      TabIndex        =   10
      Top             =   2160
      Width           =   2172
   End
   Begin VB.TextBox FirstStep 
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
      Height          =   288
      Left            =   1440
      TabIndex        =   0
      Text            =   "0"
      Top             =   2880
      Width           =   1452
   End
   Begin Threed.SSPanel SSPanel1 
      Height          =   1935
      Left            =   120
      TabIndex        =   20
      Top             =   120
      Width           =   2775
      _Version        =   65536
      _ExtentX        =   4895
      _ExtentY        =   3413
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
      Begin VB.Line Line34 
         X1              =   2400
         X2              =   2400
         Y1              =   960
         Y2              =   1560
      End
      Begin VB.Line Line2 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   1440
         Y1              =   1080
         Y2              =   840
      End
      Begin VB.Line Line3 
         BorderColor     =   &H0000FFFF&
         X1              =   1440
         X2              =   1800
         Y1              =   600
         Y2              =   480
      End
      Begin VB.Line Line4 
         BorderColor     =   &H0000FFFF&
         X1              =   1800
         X2              =   2400
         Y1              =   240
         Y2              =   240
      End
      Begin VB.Line Line5 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   600
         Y1              =   1080
         Y2              =   1080
      End
      Begin VB.Line Line6 
         BorderColor     =   &H0000FFFF&
         X1              =   600
         X2              =   600
         Y1              =   1080
         Y2              =   1320
      End
      Begin VB.Line Line7 
         BorderColor     =   &H0000FFFF&
         X1              =   600
         X2              =   1200
         Y1              =   1320
         Y2              =   1320
      End
      Begin VB.Line Line8 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   1200
         Y1              =   1080
         Y2              =   1320
      End
      Begin VB.Line Line11 
         BorderColor     =   &H0000FFFF&
         X1              =   1200
         X2              =   1440
         Y1              =   1320
         Y2              =   1080
      End
      Begin VB.Line Line12 
         BorderColor     =   &H0000FFFF&
         X1              =   1440
         X2              =   1800
         Y1              =   1080
         Y2              =   960
      End
      Begin VB.Line Line13 
         BorderColor     =   &H0000FFFF&
         X1              =   1800
         X2              =   2400
         Y1              =   960
         Y2              =   960
      End
      Begin VB.Line Line14 
         BorderColor     =   &H0000FFFF&
         X1              =   1800
         X2              =   1800
         Y1              =   240
         Y2              =   480
      End
      Begin VB.Line Line15 
         BorderColor     =   &H0000FFFF&
         X1              =   1440
         X2              =   1440
         Y1              =   600
         Y2              =   840
      End
      Begin VB.Line Line16 
         BorderColor     =   &H0000FFFF&
         X1              =   2400
         X2              =   2400
         Y1              =   240
         Y2              =   960
      End
      Begin VB.Line Line1 
         BorderColor     =   &H0000FFFF&
         X1              =   1440
         X2              =   960
         Y1              =   840
         Y2              =   720
      End
      Begin VB.Line Line9 
         BorderColor     =   &H0000FFFF&
         X1              =   1440
         X2              =   960
         Y1              =   600
         Y2              =   480
      End
      Begin VB.Line Line10 
         BorderColor     =   &H0000FFFF&
         X1              =   1800
         X2              =   1680
         Y1              =   480
         Y2              =   240
      End
      Begin VB.Line Line17 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   600
         Y1              =   720
         Y2              =   1080
      End
      Begin VB.Line Line18 
         BorderColor     =   &H0000FFFF&
         X1              =   960
         X2              =   960
         Y1              =   480
         Y2              =   720
      End
      Begin VB.Line Line19 
         BorderColor     =   &H0000FFFF&
         X1              =   1680
         X2              =   960
         Y1              =   240
         Y2              =   480
      End
      Begin VB.Line Line20 
         BorderColor     =   &H0000FFFF&
         X1              =   1680
         X2              =   1680
         Y1              =   120
         Y2              =   240
      End
      Begin VB.Line Line21 
         BorderColor     =   &H0000FFFF&
         X1              =   1680
         X2              =   1800
         Y1              =   120
         Y2              =   240
      End
      Begin VB.Line Line22 
         BorderColor     =   &H0000FFFF&
         X1              =   2400
         X2              =   2280
         Y1              =   240
         Y2              =   120
      End
      Begin VB.Line Line23 
         BorderColor     =   &H0000FFFF&
         X1              =   2280
         X2              =   1680
         Y1              =   120
         Y2              =   120
      End
      Begin VB.Line Line24 
         BorderColor     =   &H0000FFFF&
         X1              =   1440
         X2              =   1440
         Y1              =   840
         Y2              =   1080
      End
      Begin VB.Line Line25 
         BorderColor     =   &H0000FFFF&
         X1              =   1800
         X2              =   1800
         Y1              =   480
         Y2              =   960
      End
      Begin VB.Line Line29 
         X1              =   480
         X2              =   600
         Y1              =   1080
         Y2              =   1080
      End
      Begin VB.Line Line30 
         X1              =   480
         X2              =   480
         Y1              =   1320
         Y2              =   1080
      End
      Begin VB.Line Line31 
         X1              =   480
         X2              =   600
         Y1              =   1320
         Y2              =   1320
      End
      Begin VB.Label Label8 
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
         Left            =   1320
         TabIndex        =   23
         Top             =   1560
         Width           =   1215
      End
      Begin VB.Label Label10 
         BackStyle       =   0  'Transparent
         Caption         =   "Step Height"
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
         Left            =   120
         TabIndex        =   22
         Top             =   600
         Width           =   495
      End
      Begin VB.Line Line27 
         X1              =   2400
         X2              =   1200
         Y1              =   1560
         Y2              =   1560
      End
      Begin VB.Label Label1 
         Appearance      =   0  'Flat
         BackColor       =   &H00808080&
         BackStyle       =   0  'Transparent
         Caption         =   "Step Width"
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
         Height          =   255
         Left            =   360
         TabIndex        =   21
         Top             =   1440
         Width           =   975
      End
      Begin VB.Line Line28 
         X1              =   600
         X2              =   600
         Y1              =   1320
         Y2              =   1440
      End
      Begin VB.Line Line32 
         X1              =   600
         X2              =   1200
         Y1              =   1440
         Y2              =   1440
      End
      Begin VB.Line Line33 
         X1              =   1200
         X2              =   1200
         Y1              =   1320
         Y2              =   1560
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
      Left            =   1200
      TabIndex        =   8
      Top             =   6000
      Width           =   735
   End
   Begin VB.TextBox StepWidth 
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
      Text            =   "256"
      Top             =   3960
      Width           =   1455
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
      TabIndex        =   6
      Text            =   "CStair"
      Top             =   5040
      Width           =   1455
   End
   Begin VB.TextBox NumSteps 
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
      TabIndex        =   5
      Text            =   "4"
      Top             =   4680
      Width           =   1455
   End
   Begin VB.TextBox CurveAngle 
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
      Text            =   "90"
      Top             =   4320
      Width           =   1455
   End
   Begin VB.TextBox InnerRad 
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
      Text            =   "256-16"
      Top             =   3600
      Width           =   1455
   End
   Begin VB.TextBox StepHeight 
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
      Text            =   "16"
      Top             =   3240
      Width           =   1455
   End
   Begin VB.CommandButton Close 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
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
      Left            =   2040
      TabIndex        =   9
      Top             =   6000
      Width           =   855
   End
   Begin VB.CommandButton Build 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
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
      Height          =   372
      Left            =   120
      TabIndex        =   7
      Top             =   6000
      Width           =   972
   End
   Begin VB.Label Label9 
      Alignment       =   1  'Right Justify
      Caption         =   "Add to First Step"
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
      TabIndex        =   24
      Top             =   2880
      Width           =   1212
   End
   Begin VB.Label Trigger 
      Alignment       =   2  'Center
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Trigger"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   0
      TabIndex        =   19
      Top             =   5640
      Visible         =   0   'False
      Width           =   732
   End
   Begin VB.Label Label11 
      Alignment       =   1  'Right Justify
      Caption         =   "Step Width"
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
      TabIndex        =   18
      Top             =   3960
      Width           =   1212
   End
   Begin VB.Label Label7 
      Alignment       =   2  'Center
      Caption         =   "Item Names are: Step, Rise, Side, Base, Back"
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
      Left            =   480
      TabIndex        =   17
      Top             =   5400
      Width           =   2292
   End
   Begin VB.Label Label6 
      Alignment       =   1  'Right Justify
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
      Left            =   240
      TabIndex        =   16
      Top             =   5040
      Width           =   1092
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      Caption         =   "Angle of Curve"
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
      Left            =   240
      TabIndex        =   15
      Top             =   4320
      Width           =   1092
   End
   Begin VB.Label Label4 
      Alignment       =   1  'Right Justify
      Caption         =   "Number of Steps"
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
      Top             =   4680
      Width           =   1212
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
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
      TabIndex        =   13
      Top             =   3600
      Width           =   1212
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Step Height"
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
      Left            =   240
      TabIndex        =   12
      Top             =   3240
      Width           =   1092
   End
End
Attribute VB_Name = "frmParSolCurvedStair"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Build_Click()
    
    Dim StepLoop As Integer
    Dim Group As String
    Dim V As Integer         ' Vertex Accumulator
    Dim i As Integer
    Dim N As Integer
    Dim BasePointer As Integer
    Dim MaxLoop As Integer
    Dim Pi
    Dim Temp As Double
    Dim CAngle As Single
    Dim Vdir4(4) As Integer
    
    Dim SInner As Integer
    Dim SOuter As Integer
    Dim SHeight As Integer
    Dim SSteps As Integer
    Dim SFirstStep As Integer
    Dim SAngle As Integer
    Dim Angle, AngleInc, NextAngle, StartAngle, HalfAngle
    
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single
    Dim ZBase As Single
    
    Call InitBrush("CurvedStair")
    Group = UCase$(Group)

    '
    'Initialize the starting point of the first stair
    '
    If Not Eval(InnerRad, Temp) Then Exit Sub
    SInner = Int(Temp)
    '
    If Not Eval(StepWidth, Temp) Then Exit Sub
    SOuter = Int(Temp) + SInner
    '
    If Not Eval(StepHeight, Temp) Then Exit Sub
    SHeight = Int(Temp)
    '
    If Not Eval(NumSteps, Temp) Then Exit Sub
    SSteps = Int(Temp)
    '
    If Not Eval(CurveAngle, Temp) Then Exit Sub
    CAngle = Temp
    '
    If Not Eval(FirstStep, Temp) Then Exit Sub
    SFirstStep = Temp
    
    
    If Me.CCWSelect.Value = 0 Then
        Vdir4(1) = 1
        Vdir4(2) = 2
        Vdir4(3) = 3
        Vdir4(4) = 4
    Else
        Vdir4(1) = 4
        Vdir4(2) = 3
        Vdir4(3) = 2
        Vdir4(4) = 1
    End If
    If (CAngle > 360) Then
        MsgBox ("Use Spiral Staircase for 360 degree or greater angles.")
        Exit Sub
    End If
    If (SFirstStep < 0) Then
        MsgBox ("FirstStep must be zero(0) or greater.")
        Exit Sub
    End If

    If (SInner <= 0) Or (SOuter <= 0) Or (SHeight <= 0) Or (SOuter <= SInner) Or (SSteps <= 0) Then
        MsgBox ("All Values must be greater than 0.")
        Exit Sub
    End If


    Pi = 4 * Atn(1)
   
    If Me.CCWSelect.Value = 1 Then
        AngleInc = -(2 * Pi / (360 / CAngle) / SSteps)
    Else
        AngleInc = 2 * Pi / (360 / CAngle) / SSteps
    End If
        
    HalfAngle = AngleInc / 2
        
    'Adjust radius for side alignment
    If Me.chkAlignSide.Value = 1 Then '1 = checked 0 = Unchecked
        SInner = 1 / (Cos(HalfAngle) / SInner)
        SOuter = 1 / (Cos(HalfAngle) / SOuter)
    End If

    Angle = 0
    CurrentX = 0               '
    CurrentY = 0
    CurrentZ = Int(-(((SHeight * SSteps) + SFirstStep) / 2)) '
    
    N = 0
    
    '****************************************************************************************
    ' Main Step Builder
    '
    ' Stepped Stairs
    CurrentZ = Int(-(((SHeight * SSteps) + SFirstStep) / 2)) + SHeight
    ZBase = Int(-(((SHeight * SSteps) + SFirstStep) / 2)) - SFirstStep
    For StepLoop = 1 To SSteps

        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Step"
        Brush.Polys(N).NumVertices = 4

        CurrentX = SInner * Cos(Angle)
        CurrentY = SInner * Sin(Angle)
        Call PutVertex(N, Vdir4(4), CurrentX, CurrentY, CurrentZ)
        CurrentX = SInner * Cos(Angle + AngleInc)
        CurrentY = SInner * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(3), CurrentX, CurrentY, CurrentZ)
        CurrentX = SOuter * Cos(Angle + AngleInc)
        CurrentY = SOuter * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(2), CurrentX, CurrentY, CurrentZ)
        CurrentX = SOuter * Cos(Angle)
        CurrentY = SOuter * Sin(Angle)
        Call PutVertex(N, Vdir4(1), CurrentX, CurrentY, CurrentZ)
        
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Base"
        Brush.Polys(N).NumVertices = 4

        CurrentX = SInner * Cos(Angle)
        CurrentY = SInner * Sin(Angle)
        Call PutVertex(N, Vdir4(1), CurrentX, CurrentY, ZBase)
        CurrentX = SInner * Cos(Angle + AngleInc)
        CurrentY = SInner * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(2), CurrentX, CurrentY, ZBase)
        CurrentX = SOuter * Cos(Angle + AngleInc)
        CurrentY = SOuter * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(3), CurrentX, CurrentY, ZBase)
        CurrentX = SOuter * Cos(Angle)
        CurrentY = SOuter * Sin(Angle)
        Call PutVertex(N, Vdir4(4), CurrentX, CurrentY, ZBase)
        

        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Rise"
        Brush.Polys(N).NumVertices = 4

        If StepLoop = 1 Then
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, Vdir4(1), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, Vdir4(2), CurrentX, CurrentY, ZBase)
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, Vdir4(3), CurrentX, CurrentY, ZBase)
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, Vdir4(4), CurrentX, CurrentY, CurrentZ)
        Else
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, Vdir4(1), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, Vdir4(2), CurrentX, CurrentY, CurrentZ - SHeight)
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, Vdir4(3), CurrentX, CurrentY, CurrentZ - SHeight)
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, Vdir4(4), CurrentX, CurrentY, CurrentZ)
        End If
        
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Side" ' Inside
        Brush.Polys(N).NumVertices = 4

        CurrentX = SInner * Cos(Angle)
        CurrentY = SInner * Sin(Angle)
        Call PutVertex(N, Vdir4(1), CurrentX, CurrentY, CurrentZ)
        CurrentX = SInner * Cos(Angle + AngleInc)
        CurrentY = SInner * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(2), CurrentX, CurrentY, CurrentZ)
        CurrentX = SInner * Cos(Angle + AngleInc)
        CurrentY = SInner * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(3), CurrentX, CurrentY, ZBase)
        CurrentX = SInner * Cos(Angle)
        CurrentY = SInner * Sin(Angle)
        Call PutVertex(N, Vdir4(4), CurrentX, CurrentY, ZBase)

        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Side" ' OutSide
        Brush.Polys(N).NumVertices = 4

        CurrentX = SOuter * Cos(Angle)
        CurrentY = SOuter * Sin(Angle)
        Call PutVertex(N, Vdir4(4), CurrentX, CurrentY, CurrentZ)
        CurrentX = SOuter * Cos(Angle + AngleInc)
        CurrentY = SOuter * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(3), CurrentX, CurrentY, CurrentZ)
        CurrentX = SOuter * Cos(Angle + AngleInc)
        CurrentY = SOuter * Sin(Angle + AngleInc)
        Call PutVertex(N, Vdir4(2), CurrentX, CurrentY, ZBase)
        CurrentX = SOuter * Cos(Angle)
        CurrentY = SOuter * Sin(Angle)
        Call PutVertex(N, Vdir4(1), CurrentX, CurrentY, ZBase)
        
        Angle = Angle + AngleInc

        CurrentZ = CurrentZ + SHeight
    Next StepLoop

    
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = "Back"
        Brush.Polys(N).NumVertices = 4

        CurrentX = SInner * Cos(Angle)
        CurrentY = SInner * Sin(Angle)
        Call PutVertex(N, Vdir4(1), CurrentX, CurrentY, CurrentZ - SHeight)
        CurrentX = SOuter * Cos(Angle)
        CurrentY = SOuter * Sin(Angle)
        Call PutVertex(N, Vdir4(2), CurrentX, CurrentY, CurrentZ - SHeight)
        CurrentX = SOuter * Cos(Angle)
        CurrentY = SOuter * Sin(Angle)
        Call PutVertex(N, Vdir4(3), CurrentX, CurrentY, ZBase)
        CurrentX = SInner * Cos(Angle)
        CurrentY = SInner * Sin(Angle)
        Call PutVertex(N, Vdir4(4), CurrentX, CurrentY, ZBase)
    
    
    Brush.NumPolys = N
    Call SendBrush(0)
    Call Ed.StatusText("Built a Curved Stair")

End Sub

Private Sub Close_Click()
    frmParSolCurvedStair.Hide
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "BuildCurvedStair", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Help_Click()
    ToolHelp (151)
End Sub

Private Sub Trigger_Change()
    Build_Click
End Sub

'
' Focus change highlighting routines.
'
Private Sub FirstStep_GotFocus()
    SelectAll FirstStep
End Sub

Private Sub StepHeight_GotFocus()
    SelectAll StepHeight
End Sub

Private Sub InnerRad_GotFocus()
    SelectAll InnerRad
End Sub

Private Sub StepWidth_GotFocus()
    SelectAll StepWidth
End Sub

Private Sub CurveAngle_GotFocus()
    SelectAll CurveAngle
End Sub

Private Sub NumSteps_GotFocus()
    SelectAll NumSteps
End Sub

Private Sub Group_GotFocus()
    SelectAll Group
End Sub

