VERSION 5.00
Begin VB.Form frmRevolve 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "2D Revolver"
   ClientHeight    =   1875
   ClientLeft      =   1530
   ClientTop       =   4215
   ClientWidth     =   4620
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
   HelpContextID   =   110
   Icon            =   "Revolve.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   1875
   ScaleWidth      =   4620
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton btnLoft 
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
      Height          =   375
      Left            =   120
      TabIndex        =   8
      Top             =   1440
      Width           =   1215
   End
   Begin VB.Frame Frame1 
      Caption         =   "Loft Plane"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1092
      Left            =   120
      TabIndex        =   4
      Top             =   120
      Width           =   1212
      Begin VB.OptionButton optLoftX 
         Caption         =   "X"
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
         TabIndex        =   7
         Top             =   240
         Width           =   372
      End
      Begin VB.OptionButton optLoftY 
         Caption         =   "Y"
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
         TabIndex        =   6
         Top             =   480
         Width           =   372
      End
      Begin VB.OptionButton optLoftZ 
         Caption         =   "Z"
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
         TabIndex        =   5
         Top             =   720
         Value           =   -1  'True
         Width           =   492
      End
   End
   Begin VB.TextBox txtGroupName 
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
      Height          =   288
      Left            =   3480
      TabIndex        =   3
      Text            =   "2DLoft"
      Top             =   240
      Width           =   972
   End
   Begin VB.CommandButton btnCancel 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Cancel          =   -1  'True
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
      Left            =   3240
      TabIndex        =   2
      Top             =   1440
      Width           =   1215
   End
   Begin VB.TextBox txtCycle 
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
      Left            =   3480
      TabIndex        =   1
      Text            =   "6"
      Top             =   600
      Width           =   975
   End
   Begin VB.TextBox txtNumSteps 
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
      Left            =   3480
      TabIndex        =   0
      Text            =   "6"
      Top             =   960
      Width           =   975
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   " Group Name"
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
      TabIndex        =   9
      Top             =   240
      Width           =   1215
   End
   Begin VB.Label Label3 
      Alignment       =   1  'Right Justify
      Caption         =   "Sides per 360 degrees"
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
      Left            =   1440
      TabIndex        =   11
      Top             =   600
      Width           =   1935
   End
   Begin VB.Label Label4 
      Alignment       =   1  'Right Justify
      Caption         =   "Number of Sides"
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
      Left            =   1800
      TabIndex        =   10
      Top             =   960
      Width           =   1575
   End
End
Attribute VB_Name = "frmRevolve"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'Const MaxMeshes = 40
'Const MaxSides = 100
'Const MaxSVertices = 100
'
'
'Const MaxLines = MaxMeshes * MaxSides
'
'Dim NumMesh As Integer
'Dim NumTriangles As Integer
'Dim Mesh(MaxMeshes) As MeshType

Sub btnCancel_Click()
    'btnCancel.Default = True
    
    Hide
End Sub

Sub btnLoft_Click()
    
    Dim Cycle As Integer 'Number of steps per 360 Note:Must be large enough to walk through.
    Dim Group As String
    Dim Angle, AngleInc, NextAngle, StartAngle
    Dim Radius1 As Single
    Dim Radius2 As Single
    Dim NumSides As Integer
    Dim i As Integer
    Dim j As Integer
    Dim k As Integer
    Dim N As Integer
    Dim Pi
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single
    Dim WorkSide As Integer
    Dim Inside As Integer
    Dim Vert1 As Integer
    Dim Vert2 As Integer
    Dim TempVert As Integer
    Dim ct As Integer
    Dim Positive As Integer
    Dim Negative As Integer
    Dim LoftPlane As String
    Dim Vdir As Integer

    Call InitBrush("2dEditor")

    ' Get Parameters
    Group = (frmRevolve.txtGroupName.Text)
    Cycle = Val(frmRevolve.txtCycle.Text)
    NumSides = Val(frmRevolve.txtNumSteps.Text)

    If frmRevolve.optLoftX = True Then
        LoftPlane = "X"
    ElseIf frmRevolve.optLoftY = True Then
        LoftPlane = "Y"
    ElseIf frmRevolve.optLoftZ = True Then
        LoftPlane = "Z"
    End If

    
    Hide

    Pi = 4 * Atn(1)
    AngleInc = 2 * Pi / Cycle
    Angle = 0
    N = 0
    Positive = False
    Negative = False

    If CurMesh = 0 Then CurMesh = 1

    For ct = 1 To NumMesh

        'Check for left or right of Origin
        Positive = False
        Negative = False
        For i = 1 To Mesh(ct).NumTriangles
            If Mesh(ct).Triangle(i).Exists Then
                For j = 1 To 3
                    If Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX > 0 Then
                        Positive = True
                    ElseIf Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX < 0 Then
                        Negative = True
                    End If
                Next j
            End If
        Next i

        If Positive And Negative Then
            MsgBox "The shape must be completely left or right of the Origin"
            Exit Sub
        End If


        For i = 1 To Mesh(ct).NumTriangles
            If Mesh(ct).Triangle(i).Exists Then

                If NumSides < Cycle Then
                    'Draw the Start Cap
                    N = N + 1
                    InitBrushPoly (N)
                    Brush.Polys(N).Group = Group
                    Brush.Polys(N).Item = Group & "TOP"
                    Brush.Polys(N).NumVertices = 3 'Triangles
                    For j = 1 To 3

                        If LoftPlane = "Y" Then
                            CurrentZ = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Cos(0)
                            CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y - OY
                            CurrentX = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Sin(0)
                            If Positive = True Then Vdir = j Else Vdir = 4 - j

                        ElseIf LoftPlane = "X" Then
                            CurrentZ = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Cos(0)
                            CurrentX = -(Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y - OY)
                            CurrentY = -(Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Sin(0)
                            If Positive = True Then Vdir = 4 - j Else Vdir = 4

                        ElseIf LoftPlane = "Z" Then
                            CurrentX = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Cos(0)
                            CurrentY = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Sin(0)
                            CurrentZ = -(Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y - OY)
                            If Positive = True Then Vdir = 4 - j Else Vdir = j
                        End If


                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                        Debug.Print "First Triangle", CurrentX, CurrentY, CurrentZ
                    Next j
                End If



                'Check the sides to see if any of them are outside edges
                For j = 1 To 3
                    WorkSide = Mesh(ct).Triangle(i).S(j)
                    If Mesh(ct).Side(WorkSide).Share(2) = False Then
                        If (Mesh(ct).Side(WorkSide).Exists = True) Then
                            Call frmTwoDee.DrawSide(ct, WorkSide, 2)
                            'Find and sort the 2 vertices connected to this side
                            Vert1 = 0
                            Vert2 = 0
                            For k = 1 To 3
                                If (Mesh(ct).Triangle(i).V(k) = Mesh(ct).Side(WorkSide).SV(1)) Or (Mesh(ct).Triangle(i).V(k) = Mesh(ct).Side(WorkSide).SV(2)) Then
                                    'This vertex is part of this side
                                    If Vert1 = 0 Then
                                        Vert1 = Mesh(ct).Triangle(i).V(k)
                                    Else
                                        Vert2 = Mesh(ct).Triangle(i).V(k)
                                        'Swap if last pass
                                        If (k = 3) And (j <> 2) Then
                                            TempVert = Vert1
                                            Vert1 = Vert2
                                            Vert2 = TempVert
                                        End If
                                    End If

                                End If

                            Next k

                            Radius1 = Mesh(ct).Vertex(Vert1).X - OX
                            Radius2 = Mesh(ct).Vertex(Vert2).X - OX



                            Debug.Print "Radius 1", Radius1
                            Debug.Print "Radius 2", Radius2
                            Debug.Print "Vert 1", Vert1
                            Debug.Print "Vert 2", Vert2


                            'draw this side all the way around
                            Angle = 0
                            For k = 1 To NumSides
                                If (Radius1 <> 0) And (Radius2 <> 0) Then

                                    N = N + 1
                                    InitBrushPoly (N)
                                    Brush.Polys(N).Group = Group
                                    Brush.Polys(N).Item = Group & "SIDE"
                                    Brush.Polys(N).NumVertices = 4
                                    Debug.Print "Normal Rotate"

                                    If LoftPlane = "Y" Then
                                        CurrentZ = Radius1 * Cos(Angle)
                                        CurrentY = Mesh(ct).Vertex(Vert1).Y - OY
                                        CurrentX = Radius1 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 4 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle)
                                        CurrentY = Mesh(ct).Vertex(Vert2).Y - OY
                                        CurrentX = Radius2 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle + AngleInc)
                                        CurrentY = Mesh(ct).Vertex(Vert2).Y - OY
                                        CurrentX = Radius2 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius1 * Cos(Angle + AngleInc)
                                        CurrentY = Mesh(ct).Vertex(Vert1).Y - OY
                                        CurrentX = Radius1 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 4
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                        Debug.Print
                                    ElseIf LoftPlane = "X" Then
                                        CurrentZ = Radius1 * Cos(Angle)
                                        CurrentX = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        CurrentY = -Radius1 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 4
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle)
                                        CurrentX = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        CurrentY = -Radius2 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle + AngleInc)
                                        CurrentX = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        CurrentY = -Radius2 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius1 * Cos(Angle + AngleInc)
                                        CurrentX = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        CurrentY = -Radius1 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 4 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                        Debug.Print
                                    ElseIf LoftPlane = "Z" Then
                                        CurrentX = Radius1 * Cos(Angle)
                                        CurrentY = Radius1 * Sin(Angle)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 4
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentX = Radius2 * Cos(Angle)
                                        CurrentY = Radius2 * Sin(Angle)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentX = Radius2 * Cos(Angle + AngleInc)
                                        CurrentY = Radius2 * Sin(Angle + AngleInc)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentX = Radius1 * Cos(Angle + AngleInc)
                                        CurrentY = Radius1 * Sin(Angle + AngleInc)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 4 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                        Debug.Print
                                    End If

                                ElseIf (Radius1 = 0) And (Radius2 <> 0) Then

                                    N = N + 1
                                    InitBrushPoly (N)
                                    Brush.Polys(N).Group = Group
                                    Brush.Polys(N).Item = Group & "SIDE"
                                    Brush.Polys(N).NumVertices = 3
                                    Debug.Print "Radius1 = 0"

                                    If LoftPlane = "Y" Then
                                        CurrentZ = Radius1 * Cos(Angle)
                                        CurrentY = Mesh(ct).Vertex(Vert1).Y - OY
                                        CurrentX = Radius1 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle)
                                        CurrentY = Mesh(ct).Vertex(Vert2).Y - OY
                                        CurrentX = Radius2 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle + AngleInc)
                                        CurrentY = Mesh(ct).Vertex(Vert2).Y - OY
                                        CurrentX = Radius2 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                    ElseIf LoftPlane = "X" Then
                                        CurrentZ = Radius1 * Cos(Angle)
                                        CurrentX = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        CurrentY = -Radius1 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle)
                                        CurrentX = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        CurrentY = -Radius2 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle + AngleInc)
                                        CurrentX = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        CurrentY = -Radius2 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                    ElseIf LoftPlane = "Z" Then
                                       CurrentX = Radius1 * Cos(Angle)
                                        CurrentY = Radius1 * Sin(Angle)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentX = Radius2 * Cos(Angle)
                                        CurrentY = Radius2 * Sin(Angle)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentX = Radius2 * Cos(Angle + AngleInc)
                                        CurrentY = Radius2 * Sin(Angle + AngleInc)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                    End If


                                ElseIf (Radius1 <> 0) And (Radius2 = 0) Then
                                    N = N + 1
                                    InitBrushPoly (N)
                                    Brush.Polys(N).Group = Group
                                    Brush.Polys(N).Item = Group & "SIDE"
                                    Brush.Polys(N).NumVertices = 3
                                    Debug.Print "Radius2 = 0"

                                    If LoftPlane = "Y" Then
                                        CurrentZ = Radius1 * Cos(Angle)
                                       CurrentY = Mesh(ct).Vertex(Vert1).Y - OY
                                        CurrentX = Radius1 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle)
                                        CurrentY = Mesh(ct).Vertex(Vert2).Y - OY
                                        CurrentX = Radius2 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius1 * Cos(Angle + AngleInc)
                                        CurrentY = Mesh(ct).Vertex(Vert1).Y - OY
                                        CurrentX = Radius1 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                        Debug.Print
                                    ElseIf LoftPlane = "X" Then
                                        CurrentZ = Radius1 * Cos(Angle)
                                        CurrentX = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        CurrentY = -Radius1 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius2 * Cos(Angle)
                                        CurrentX = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        CurrentY = -Radius2 * Sin(Angle)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentZ = Radius1 * Cos(Angle + AngleInc)
                                        CurrentX = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        CurrentY = -Radius1 * Sin(Angle + AngleInc)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                        Debug.Print

                                    ElseIf LoftPlane = "Z" Then
                                        CurrentX = Radius1 * Cos(Angle)
                                        CurrentY = Radius1 * Sin(Angle)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 1 Else Vdir = 3
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentX = Radius2 * Cos(Angle)
                                        CurrentY = Radius2 * Sin(Angle)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert2).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 2 Else Vdir = 2
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                                        CurrentX = Radius1 * Cos(Angle + AngleInc)
                                        CurrentY = Radius1 * Sin(Angle + AngleInc)
                                        CurrentZ = -(Mesh(ct).Vertex(Vert1).Y - OY)
                                        Debug.Print "X: ", CurrentX, "Y: ", CurrentY, "Z: ", CurrentZ
                                        If Positive Then Vdir = 3 Else Vdir = 1
                                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)
                                        Debug.Print
                                    End If



                                ElseIf (Radius1 = 0) And (Radius2 = 0) Then
                                    Debug.Print "Radius1 and 2 = 0"
                                    'Don't draw this side
                                    'Optimize by taking this out of the loop

                                Else
                                    MsgBox "Something bad happend in Revolve2D"
                                    Exit Sub

                                End If



                                Angle = Angle + AngleInc
                            Next k



                        End If ' Outside edge exists
                    End If 'Outside edge
                Next j

                If NumSides < Cycle Then
                    'Draw the End Cap
                    N = N + 1
                    InitBrushPoly (N)
                    Brush.Polys(N).Group = Group
                    Brush.Polys(N).Item = Group & "END"
                    Brush.Polys(N).NumVertices = 3 'Triangles
                    For j = 1 To 3
                        If LoftPlane = "Y" Then
                            CurrentZ = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Cos(Angle)
                            CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y - OY
                            CurrentX = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Sin(Angle)
                            If Positive = True Then Vdir = 4 - j Else Vdir = j
                        ElseIf LoftPlane = "X" Then
                            CurrentZ = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Cos(Angle)
                            CurrentX = -(Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y - OY)
                            CurrentY = -(Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Sin(Angle)
                            If Positive = True Then Vdir = j Else Vdir = 4 - j
                        ElseIf LoftPlane = "Z" Then
                            CurrentX = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Cos(Angle)
                            CurrentY = (Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X - OX) * Sin(Angle)
                            CurrentZ = -(Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y - OY)
                            If Positive = True Then Vdir = j Else Vdir = 4 - j
                        End If

                        Call PutVertex(N, Vdir, CurrentX, CurrentY, CurrentZ)

                    Next j
                End If

            End If 'triangle exists
        Next i
    Next ct

    Brush.NumPolys = N
    Call SendBrush(0)


End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "Revolver", TOP_NORMAL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub


