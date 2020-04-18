VERSION 5.00
Begin VB.Form frmBevel 
   Caption         =   "Bevel"
   ClientHeight    =   1935
   ClientLeft      =   6480
   ClientTop       =   6135
   ClientWidth     =   4080
   Icon            =   "Bevel.frx":0000
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   1935
   ScaleWidth      =   4080
   ShowInTaskbar   =   0   'False
   Begin VB.TextBox txtCapHeight 
      BackColor       =   &H00FFFFFF&
      ForeColor       =   &H00000000&
      Height          =   288
      Left            =   2880
      TabIndex        =   8
      Text            =   "16"
      Top             =   960
      Width           =   972
   End
   Begin VB.CommandButton btnCancel 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Cancel          =   -1  'True
      Caption         =   "&Close"
      Height          =   375
      Left            =   2880
      TabIndex        =   7
      Top             =   1440
      Width           =   1095
   End
   Begin VB.TextBox txtDepth 
      BackColor       =   &H00FFFFFF&
      ForeColor       =   &H00000000&
      Height          =   288
      Left            =   2880
      TabIndex        =   6
      Text            =   "64"
      Top             =   600
      Width           =   972
   End
   Begin VB.TextBox txtGroupName 
      BackColor       =   &H00FFFFFF&
      ForeColor       =   &H00000000&
      Height          =   288
      Left            =   2880
      TabIndex        =   5
      Text            =   "2DLoft"
      Top             =   240
      Width           =   972
   End
   Begin VB.Frame Frame1 
      Caption         =   "Loft Plane"
      Height          =   1092
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Width           =   1212
      Begin VB.OptionButton optLoftZ 
         Caption         =   "Z"
         Height          =   252
         Left            =   120
         TabIndex        =   4
         Top             =   720
         Value           =   -1  'True
         Width           =   492
      End
      Begin VB.OptionButton optLoftY 
         Caption         =   "Y"
         Height          =   252
         Left            =   120
         TabIndex        =   3
         Top             =   480
         Width           =   372
      End
      Begin VB.OptionButton optLoftX 
         Caption         =   "X"
         Height          =   252
         Left            =   120
         TabIndex        =   2
         Top             =   240
         Width           =   372
      End
   End
   Begin VB.CommandButton btnLoft 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Build"
      Default         =   -1  'True
      Height          =   375
      Left            =   120
      TabIndex        =   0
      Top             =   1440
      Width           =   1215
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      Caption         =   "Cap Height"
      Height          =   252
      Left            =   1800
      TabIndex        =   11
      Top             =   960
      Width           =   972
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Depth"
      Height          =   252
      Left            =   1560
      TabIndex        =   10
      Top             =   600
      Width           =   1212
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   " Group Name"
      Height          =   252
      Left            =   1560
      TabIndex        =   9
      Top             =   240
      Width           =   1212
   End
End
Attribute VB_Name = "frmBevel"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub btnCancel_Click()
    Hide
End Sub


Private Sub btnLoft_Click()
Dim N As Integer
Dim i As Integer
Dim j As Integer
Dim k As Integer

Dim CurrentX As Single
Dim CurrentY As Single
Dim CurrentZ As Single
Dim ScaleX As Single
Dim ScaleY As Single
Dim Dist As Single
Dim BevelLength As Integer
Dim Vdir As Integer

Dim Vert1 As Integer
Dim Vert2 As Integer
Dim TempVert As Integer
Dim WorkSide As Integer
Dim Group As String
Dim Depth As Integer
Dim LoftPlane As String
Dim ct As Integer
    
    ' Load the Parameter Form
    Call InitBrush("2dEditor")
    Group = (frmBevel.txtGroupName.Text)
    Depth = Val(frmBevel.txtDepth.Text)
    BevelLength = Val(frmBevel.txtCapHeight.Text)

    If frmBevel.optLoftX = True Then
        LoftPlane = "X"
    ElseIf frmBevel.optLoftY = True Then
        LoftPlane = "Y"
    ElseIf frmBevel.optLoftZ = True Then
        LoftPlane = "Z"
    End If

    Hide

    If Depth = 0 Then
        MsgBox " The Depth must not equal 0!"
        Exit Sub
    End If
    If BevelLength = 0 Then
        MsgBox " The Cap Length must not equal 0!"
        Exit Sub
    End If
    
    If CurMesh = 0 Then CurMesh = 1
    N = 0
    Dist = (BevelLength / Depth)

    For ct = 1 To NumMesh
        For i = 1 To Mesh(ct).NumTriangles
            If Mesh(ct).Triangle(i).Exists Then
    
                'Draw the Top
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = Group & "TOP"
                Brush.Polys(N).NumVertices = 3 'Triangles
                For j = 1 To 3
                    If LoftPlane = "X" Then
                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentX = Depth / 2
                        Vdir = 4 - j
                    ElseIf LoftPlane = "Y" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentY = Depth / 2
                        Vdir = 4 - j
                    ElseIf LoftPlane = "Z" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentZ = Depth / 2
                        Vdir = j
                    End If
                    Call PutVertex(N, Vdir, CurrentX - OX, CurrentY - OY, CurrentZ)
                Next j
    
    
    
                'Draw the Bottom
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = Group & "END"
                Brush.Polys(N).NumVertices = 3 'Triangles
                For j = 1 To 3

                    If LoftPlane = "X" Then
                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X * Dist
                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y * Dist
                        CurrentX = (-Depth / 2) + BevelLength 'for now I just want 1 side
                        Vdir = j
                    ElseIf LoftPlane = "Y" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X * Dist
                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y * Dist
                        CurrentY = (-Depth / 2) + BevelLength 'for now I just want 1 side
                        Vdir = j
                    ElseIf LoftPlane = "Z" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X * Dist
                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y * Dist
                        CurrentZ = (-Depth / 2) + BevelLength 'for now I just want 1 side
                        Vdir = 4 - j
                    End If
                    Call PutVertex(N, Vdir, CurrentX - OX, CurrentY - OY, CurrentZ)
                Next j



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
    
                            Debug.Print "Top: ", Vert1, Vert2
                            'Draw the side
                            N = N + 1
                            InitBrushPoly (N)
                            Brush.Polys(N).Group = Group
                            Brush.Polys(N).Item = Group & "SIDE"
                            Brush.Polys(N).NumVertices = 4 'Triangles

                            If LoftPlane = "X" Then
                                CurrentZ = Mesh(ct).Vertex(Vert1).X
                                CurrentY = Mesh(ct).Vertex(Vert1).Y
                                CurrentX = Depth / 2
                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentZ = Mesh(ct).Vertex(Vert2).X
                                CurrentY = Mesh(ct).Vertex(Vert2).Y
                                CurrentX = Depth / 2
                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentZ = Mesh(ct).Vertex(Vert2).X * Dist
                                CurrentY = Mesh(ct).Vertex(Vert2).Y * Dist
                                CurrentX = (-Depth / 2) + BevelLength
                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentZ = Mesh(ct).Vertex(Vert1).X * Dist
                                CurrentY = Mesh(ct).Vertex(Vert1).Y * Dist
                                CurrentX = (-Depth / 2) + BevelLength
                                Call PutVertex(N, 4, CurrentX - OX, CurrentY - OY, CurrentZ)

                            ElseIf LoftPlane = "Y" Then
                                CurrentX = Mesh(ct).Vertex(Vert1).X
                                CurrentZ = Mesh(ct).Vertex(Vert1).Y
                                CurrentY = Depth / 2
                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X
                                CurrentZ = Mesh(ct).Vertex(Vert2).Y
                                CurrentY = Depth / 2
                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X * Dist
                                CurrentZ = Mesh(ct).Vertex(Vert2).Y * Dist
                                CurrentY = (-Depth / 2) + BevelLength
                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert1).X * Dist
                                CurrentZ = Mesh(ct).Vertex(Vert1).Y * Dist
                                CurrentY = (-Depth / 2) + BevelLength
                                Call PutVertex(N, 4, CurrentX - OX, CurrentY - OY, CurrentZ)

                            ElseIf LoftPlane = "Z" Then
                                CurrentX = Mesh(ct).Vertex(Vert1).X
                                CurrentY = Mesh(ct).Vertex(Vert1).Y
                                CurrentZ = Depth / 2
                                Call PutVertex(N, 4, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X
                                CurrentY = Mesh(ct).Vertex(Vert2).Y
                                CurrentZ = Depth / 2
                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X * Dist
                                CurrentY = Mesh(ct).Vertex(Vert2).Y * Dist
                                CurrentZ = (-Depth / 2) + BevelLength
                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert1).X * Dist
                                CurrentY = Mesh(ct).Vertex(Vert1).Y * Dist
                                CurrentZ = (-Depth / 2) + BevelLength
                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)

                            End If

                        End If ' Outside edge exists
                    End If 'Outside edge
                Next j
            End If 'triangle exists
        Next i
    Next ct

    Brush.NumPolys = N
    Call SendBrush(0)

End Sub

