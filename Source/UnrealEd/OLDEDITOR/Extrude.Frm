VERSION 5.00
Begin VB.Form frmExtrude 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "2D Extruder"
   ClientHeight    =   1860
   ClientLeft      =   4875
   ClientTop       =   7830
   ClientWidth     =   4095
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H00400000&
   HelpContextID   =   109
   Icon            =   "Extrude.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   1860
   ScaleWidth      =   4095
   ShowInTaskbar   =   0   'False
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
      Left            =   2880
      TabIndex        =   9
      Top             =   1440
      Width           =   1095
   End
   Begin VB.TextBox txtDepth 
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
      Left            =   3000
      TabIndex        =   8
      Text            =   "64"
      Top             =   600
      Width           =   972
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
      Left            =   3000
      TabIndex        =   7
      Text            =   "2DLoft"
      Top             =   240
      Width           =   972
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
      Height          =   1095
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Width           =   1755
      Begin VB.OptionButton optLoftZ 
         Caption         =   "Z (Room outline)"
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
         TabIndex        =   4
         Top             =   720
         Value           =   -1  'True
         Width           =   1575
      End
      Begin VB.OptionButton optLoftY 
         Caption         =   "Y (Hall outline)"
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
         TabIndex        =   3
         Top             =   480
         Width           =   1515
      End
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
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   240
         Width           =   1515
      End
   End
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
      TabIndex        =   0
      Top             =   1440
      Width           =   1215
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Depth"
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
      Left            =   2040
      TabIndex        =   6
      Top             =   600
      Width           =   855
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
      Left            =   1920
      TabIndex        =   5
      Top             =   240
      Width           =   975
   End
End
Attribute VB_Name = "frmExtrude"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub btnCancel_Click()
    btnCancel.Default = True
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
Dim Vdir As Integer

Dim Vert1 As Integer
Dim Vert2 As Integer
Dim TempVert As Integer
Dim WorkSide As Integer
Dim Group As String
Dim Depth As Integer
Dim LoftPlane As String
Dim ct As Integer

    '
    ' Get Parameters
    '
    Call InitBrush("2dEditor")
    Group = (frmExtrude.txtGroupName.Text)
    Depth = Val(frmExtrude.txtDepth.Text)
    If frmExtrude.optLoftX = True Then
        LoftPlane = "X"
    ElseIf frmExtrude.optLoftY = True Then
        LoftPlane = "Y"
    ElseIf frmExtrude.optLoftZ = True Then
        LoftPlane = "Z"
    End If

    Hide
    
    If CurMesh = 0 Then CurMesh = 1

    N = 0
    For ct = 1 To NumMesh

        For i = 1 To Mesh(ct).NumTriangles
            If Mesh(ct).Triangle(i).Exists Then
                '
                ' Draw the Bottom
                '

                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = Group & "END"
                Brush.Polys(N).NumVertices = 3 'Triangles
                For j = 1 To 3
                    If LoftPlane = "X" Then
                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentX = -Depth / 2 'for now I just want 1 side
                        Vdir = j
                    ElseIf LoftPlane = "Y" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentY = -Depth / 2 'for now I just want 1 side
                        Vdir = j
                    ElseIf LoftPlane = "Z" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentZ = -Depth / 2 'for now I just want 1 side
                        Vdir = 4 - j
                    End If
                    Call PutVertex(N, Vdir, CurrentX - OX, CurrentY - OY, CurrentZ)
                Next j
    
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
                        CurrentX = Depth / 2 'for now I just want 1 side
                        Vdir = 4 - j
                    ElseIf LoftPlane = "Y" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentY = Depth / 2 'for now I just want 1 side
                        Vdir = 4 - j
                    ElseIf LoftPlane = "Z" Then
                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                        CurrentZ = Depth / 2 'for now I just want 1 side
                        Vdir = j
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
                                CurrentX = -Depth / 2
                                Call PutVertex(N, 4, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentZ = Mesh(ct).Vertex(Vert2).X
                                CurrentY = Mesh(ct).Vertex(Vert2).Y
                                CurrentX = -Depth / 2
                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentZ = Mesh(ct).Vertex(Vert2).X
                                CurrentY = Mesh(ct).Vertex(Vert2).Y
                                CurrentX = Depth / 2
                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentZ = Mesh(ct).Vertex(Vert1).X
                                CurrentY = Mesh(ct).Vertex(Vert1).Y
                                CurrentX = Depth / 2
                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
                            ElseIf LoftPlane = "Y" Then
                                CurrentX = Mesh(ct).Vertex(Vert1).X
                                CurrentZ = Mesh(ct).Vertex(Vert1).Y
                                CurrentY = -Depth / 2
                                Call PutVertex(N, 4, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X
                                CurrentZ = Mesh(ct).Vertex(Vert2).Y
                                CurrentY = -Depth / 2
                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X
                                CurrentZ = Mesh(ct).Vertex(Vert2).Y
                                CurrentY = Depth / 2
                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert1).X
                                CurrentZ = Mesh(ct).Vertex(Vert1).Y
                                CurrentY = Depth / 2
                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
                            ElseIf LoftPlane = "Z" Then
                                CurrentX = Mesh(ct).Vertex(Vert1).X
                                CurrentY = Mesh(ct).Vertex(Vert1).Y
                                CurrentZ = -Depth / 2
                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X
                                CurrentY = Mesh(ct).Vertex(Vert2).Y
                                CurrentZ = -Depth / 2
                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert2).X
                                CurrentY = Mesh(ct).Vertex(Vert2).Y
                                CurrentZ = Depth / 2
                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
                                CurrentX = Mesh(ct).Vertex(Vert1).X
                                CurrentY = Mesh(ct).Vertex(Vert1).Y
                                CurrentZ = Depth / 2
                                Call PutVertex(N, 4, CurrentX - OX, CurrentY - OY, CurrentZ)
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

Private Sub ExtrudeLinear()
End Sub

Private Sub ExtrudePoint()

End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "Extruder", TOP_NORMAL)
End Sub


Private Sub Revolve()

End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub


Private Sub txtCapHeight_Change()

End Sub


