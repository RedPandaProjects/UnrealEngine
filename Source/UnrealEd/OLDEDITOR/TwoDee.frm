VERSION 5.00
Begin VB.Form frmTwoDee 
   Appearance      =   0  'Flat
   BackColor       =   &H00000000&
   Caption         =   "2D Editor"
   ClientHeight    =   5445
   ClientLeft      =   2310
   ClientTop       =   2655
   ClientWidth     =   7755
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
   HelpContextID   =   127
   LinkTopic       =   "Form4"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5439.357
   ScaleMode       =   0  'User
   ScaleWidth      =   7752.72
   Visible         =   0   'False
   Begin VB.CommandButton Command2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Stats"
      Height          =   372
      Left            =   8760
      TabIndex        =   0
      Top             =   7320
      Visible         =   0   'False
      Width           =   972
   End
   Begin VB.CommandButton Command1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Outer"
      Height          =   372
      Left            =   8760
      TabIndex        =   5
      Top             =   6840
      Visible         =   0   'False
      Width           =   972
   End
   Begin VB.Image Image1 
      Appearance      =   0  'Flat
      Enabled         =   0   'False
      Height          =   2052
      Left            =   2880
      Top             =   1920
      Visible         =   0   'False
      Width           =   2532
   End
   Begin VB.Line GridLn 
      BorderColor     =   &H00C00000&
      DrawMode        =   15  'Merge Pen Not
      Index           =   9999
      Visible         =   0   'False
      X1              =   960.717
      X2              =   2160.365
      Y1              =   2399.51
      Y2              =   2399.51
   End
   Begin VB.Line Origin1 
      BorderColor     =   &H0000FF00&
      X1              =   2760.188
      X2              =   2760.188
      Y1              =   2054.868
      Y2              =   2254.661
   End
   Begin VB.Line Origin2 
      BorderColor     =   &H0000FF00&
      X1              =   2655.219
      X2              =   2855.16
      Y1              =   2159.759
      Y2              =   2159.759
   End
   Begin VB.Shape VertexMarker 
      BackColor       =   &H0000FFFF&
      BorderColor     =   &H0000FFFF&
      Height          =   100
      Left            =   1320
      Shape           =   3  'Circle
      Top             =   1440
      Visible         =   0   'False
      Width           =   100
   End
   Begin VB.Line Ln 
      BorderColor     =   &H00FF8080&
      Index           =   9999
      Visible         =   0   'False
      X1              =   1319.612
      X2              =   3000.118
      Y1              =   1439.507
      Y2              =   1439.507
   End
   Begin VB.Label SSExists 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1560
      TabIndex        =   41
      Top             =   7200
      Visible         =   0   'False
      Width           =   252
   End
   Begin VB.Label txtVertexY 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   8760
      TabIndex        =   35
      Top             =   8160
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label txtOriginY 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   8760
      TabIndex        =   40
      Top             =   7800
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label txtVertexX 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   8160
      TabIndex        =   39
      Top             =   8160
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label19 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Vertex"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   6480
      TabIndex        =   38
      Top             =   8160
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label txtOriginX 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   8160
      TabIndex        =   37
      Top             =   7800
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Origin"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   6480
      TabIndex        =   36
      Top             =   7800
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label STs3 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   8160
      TabIndex        =   1
      Top             =   7440
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label STs2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   8160
      TabIndex        =   2
      Top             =   7200
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label STs1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   8160
      TabIndex        =   3
      Top             =   6960
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label18 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Side 3    :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   6480
      TabIndex        =   4
      Top             =   7440
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label Label17 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Side 2    :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   6480
      TabIndex        =   34
      Top             =   7200
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label Label16 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Side 1    :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   6480
      TabIndex        =   33
      Top             =   6960
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label STv3 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   5880
      TabIndex        =   32
      Top             =   8160
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label15 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Vertex 3 :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   4320
      TabIndex        =   31
      Top             =   8160
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label STv2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   5880
      TabIndex        =   30
      Top             =   7920
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label14 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Vertex 2 :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   4320
      TabIndex        =   29
      Top             =   7920
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label STv1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   5880
      TabIndex        =   28
      Top             =   7680
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label13 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Vertex 1 :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   4320
      TabIndex        =   27
      Top             =   7680
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label SCurTri 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   372
      Left            =   5880
      TabIndex        =   26
      Top             =   7320
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label12 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Current Triangle"
      ForeColor       =   &H80000008&
      Height          =   372
      Left            =   4320
      TabIndex        =   25
      Top             =   7320
      Visible         =   0   'False
      Width           =   1572
   End
   Begin VB.Label SShare2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1200
      TabIndex        =   24
      Top             =   8160
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label11 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Share 2 :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   0
      TabIndex        =   23
      Top             =   8160
      Visible         =   0   'False
      Width           =   1212
   End
   Begin VB.Label SShare1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1200
      TabIndex        =   22
      Top             =   7920
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label10 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Share 1 :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   0
      TabIndex        =   21
      Top             =   7920
      Visible         =   0   'False
      Width           =   1212
   End
   Begin VB.Label Ssv2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1200
      TabIndex        =   20
      Top             =   7680
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label9 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Vertex 2 :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   0
      TabIndex        =   19
      Top             =   7680
      Visible         =   0   'False
      Width           =   1212
   End
   Begin VB.Label Ssv1 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1200
      TabIndex        =   18
      Top             =   7440
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label8 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Vertex 1 :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   0
      TabIndex        =   17
      Top             =   7440
      Visible         =   0   'False
      Width           =   1212
   End
   Begin VB.Label SCurSide 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1200
      TabIndex        =   16
      Top             =   7200
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label7 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Current Side"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   0
      TabIndex        =   15
      Top             =   7200
      Visible         =   0   'False
      Width           =   1212
   End
   Begin VB.Label SNumVert 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   3720
      TabIndex        =   14
      Top             =   8160
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label6 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "Number of Verticies :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1920
      TabIndex        =   13
      Top             =   8160
      Visible         =   0   'False
      Width           =   1812
   End
   Begin VB.Label SNumSides 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   3720
      TabIndex        =   12
      Top             =   7920
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label5 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "Number of Sides :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1920
      TabIndex        =   11
      Top             =   7920
      Visible         =   0   'False
      Width           =   1812
   End
   Begin VB.Label SNumTri 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "- -"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   3720
      TabIndex        =   10
      Top             =   7680
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label4 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "Number of Triangles :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1920
      TabIndex        =   9
      Top             =   7680
      Visible         =   0   'False
      Width           =   1812
   End
   Begin VB.Label SNumMesh 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "- - "
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   3720
      TabIndex        =   8
      Top             =   7440
      Visible         =   0   'False
      Width           =   492
   End
   Begin VB.Label Label3 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "Number Of Meshes :"
      ForeColor       =   &H80000008&
      Height          =   252
      Left            =   1920
      TabIndex        =   7
      Top             =   7440
      Visible         =   0   'False
      Width           =   1812
   End
   Begin VB.Label Label2 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "Statistics"
      ForeColor       =   &H80000008&
      Height          =   372
      Left            =   1920
      TabIndex        =   6
      Top             =   7080
      Visible         =   0   'False
      Width           =   2292
   End
   Begin VB.Line Line1 
      BorderColor     =   &H00C0C0C0&
      BorderStyle     =   0  'Transparent
      Visible         =   0   'False
      X1              =   5759.306
      X2              =   10559.89
      Y1              =   7440.281
      Y2              =   7440.281
   End
   Begin VB.Image Grid1 
      Appearance      =   0  'Flat
      Enabled         =   0   'False
      Height          =   7200
      Left            =   0
      Picture         =   "TwoDee.frx":0000
      Top             =   0
      Width           =   9600
   End
   Begin VB.Menu File2D 
      Caption         =   "&File"
      Begin VB.Menu New2D 
         Caption         =   "&New 2D Shape"
      End
      Begin VB.Menu Open2D 
         Caption         =   "&Open"
      End
      Begin VB.Menu Save2D 
         Caption         =   "&Save"
      End
      Begin VB.Menu SaveAs2D 
         Caption         =   "Save &As..."
      End
      Begin VB.Menu fgoi 
         Caption         =   "-"
      End
      Begin VB.Menu DebugShape 
         Caption         =   "&Debug"
      End
   End
   Begin VB.Menu Shape2D 
      Caption         =   "&Shape"
      Begin VB.Menu NewAdd 
         Caption         =   "New &Additive"
      End
      Begin VB.Menu MoveShape 
         Caption         =   "&Move"
         Enabled         =   0   'False
      End
      Begin VB.Menu Rotate 
         Caption         =   "Rotate 90'"
      End
      Begin VB.Menu Scale 
         Caption         =   "Scale"
         Begin VB.Menu ShapeIn2x 
            Caption         =   "In 2X"
         End
         Begin VB.Menu ShapeOut2x 
            Caption         =   "Out 2X"
         End
      End
      Begin VB.Menu Trans 
         Caption         =   "&Translate"
         Begin VB.Menu FlipH 
            Caption         =   "Flip Horizontal"
            Enabled         =   0   'False
         End
         Begin VB.Menu FlipV 
            Caption         =   "Flip Vertical"
            Enabled         =   0   'False
         End
      End
      Begin VB.Menu Delete 
         Caption         =   "&Delete"
      End
   End
   Begin VB.Menu Vertex2D 
      Caption         =   "&Vertex"
      Begin VB.Menu VertIns 
         Caption         =   "&Insert"
         Shortcut        =   ^I
      End
      Begin VB.Menu VertDel 
         Caption         =   "&Delete"
         Shortcut        =   ^D
      End
      Begin VB.Menu LimitAngles 
         Caption         =   "Limit Angles"
      End
   End
   Begin VB.Menu Grid2D 
      Caption         =   "&Grid"
      Begin VB.Menu ShowGrid 
         Caption         =   "Grid Visible"
         Checked         =   -1  'True
      End
      Begin VB.Menu GridSnap 
         Caption         =   "&Snap to grid"
         Checked         =   -1  'True
         Shortcut        =   ^S
      End
   End
   Begin VB.Menu Loft2D 
      Caption         =   "&Loft"
      Begin VB.Menu Sheet2D 
         Caption         =   "Build &Sheet"
      End
      Begin VB.Menu Revolve2D 
         Caption         =   "&Revolve"
      End
      Begin VB.Menu Extrude2D 
         Caption         =   "&Extrude"
      End
      Begin VB.Menu ExtrudePoint 
         Caption         =   "Extrude to &Point"
      End
      Begin VB.Menu ExtrudeBevel 
         Caption         =   "&Bevel"
      End
      Begin VB.Menu ExtrudeMarker 
         Caption         =   "Extrude along Marker Path"
         Enabled         =   0   'False
      End
   End
   Begin VB.Menu Zoom2D 
      Caption         =   "&Zoom"
      Begin VB.Menu ZoomIn2X 
         Caption         =   "&In 2X"
         Shortcut        =   ^Z
      End
      Begin VB.Menu ZoomOut2X 
         Caption         =   "&Out 2X"
         Shortcut        =   ^O
      End
   End
   Begin VB.Menu Image 
      Caption         =   "&Image"
      Begin VB.Menu LoadImage 
         Caption         =   "&Load Image"
      End
      Begin VB.Menu HideImage 
         Caption         =   "&Hide Image"
      End
   End
   Begin VB.Menu Help 
      Caption         =   "&Help"
      Begin VB.Menu Help2D 
         Caption         =   "2D Editor &Help"
      End
   End
   Begin VB.Menu mnuUndo 
      Caption         =   "&Undo"
   End
End
Attribute VB_Name = "frmTwoDee"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Const GWW_HWNDPARENT = (-8)
Dim TwoDeeWord As Integer
Dim Po
Dim Resizing As Boolean

'
' Most Declarations moved to
' TwoDee.BAS
'

Const ModuleName = "2D Shape Editor"
Dim Fname As String

Private Sub AddNewMesh(Add As Integer)
    Dim Offset As Integer
    '
    Offset = NumMesh * 16
    '
    NumMesh = NumMesh + 1
    Mesh(NumMesh).NumVertices = 3
    
    Mesh(NumMesh).NumTriangles = 1

    Mesh(NumMesh).Add = Add
    Mesh(NumMesh).Vertex(3).X = 0 + Offset
    Mesh(NumMesh).Vertex(3).Y = -64 + Offset
    Mesh(NumMesh).Vertex(2).X = -64 + Offset
    Mesh(NumMesh).Vertex(2).Y = 64 + Offset
    Mesh(NumMesh).Vertex(1).X = 64 + Offset
    Mesh(NumMesh).Vertex(1).Y = 64 + Offset

    Mesh(NumMesh).Vertex(1).SidesTouching = 2
    Mesh(NumMesh).Vertex(2).SidesTouching = 2
    Mesh(NumMesh).Vertex(3).SidesTouching = 2
    
    Mesh(NumMesh).Vertex(1).Exists = True
    Mesh(NumMesh).Vertex(2).Exists = True
    Mesh(NumMesh).Vertex(3).Exists = True

    Mesh(NumMesh).Triangle(1).V(1) = 1
    Mesh(NumMesh).Triangle(1).V(2) = 2
    Mesh(NumMesh).Triangle(1).V(3) = 3
    
    Mesh(NumMesh).Triangle(1).S(1) = 1
    Mesh(NumMesh).Triangle(1).S(2) = 2
    Mesh(NumMesh).Triangle(1).S(3) = 3
    
    Mesh(NumMesh).Triangle(1).Exists = True

    ' since this is a new mesh, I might not have to use this "+1"technique
    Mesh(NumMesh).NumSides = Mesh(NumMesh).NumSides + 1
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).SV(1) = 1
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).SV(2) = 2
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).Exists = True

    Mesh(NumMesh).NumSides = Mesh(NumMesh).NumSides + 1
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).SV(1) = 2
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).SV(2) = 3
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).Exists = True

    Mesh(NumMesh).NumSides = Mesh(NumMesh).NumSides + 1
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).SV(1) = 3
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).SV(2) = 1
    Mesh(NumMesh).Side(Mesh(NumMesh).NumSides).Exists = True
    
    
    Mesh(NumMesh).Vertex(1).index(1) = 1
    Mesh(NumMesh).Vertex(1).index(2) = 3
    Mesh(NumMesh).Vertex(2).index(1) = 1
    Mesh(NumMesh).Vertex(2).index(2) = 2
    Mesh(NumMesh).Vertex(3).index(1) = 2
    Mesh(NumMesh).Vertex(3).index(2) = 3
    
    Mesh(NumMesh).Vertex(1).IndexPointer = 1
    Mesh(NumMesh).Vertex(2).IndexPointer = 1
    Mesh(NumMesh).Vertex(3).IndexPointer = 1
    
    Mesh(NumMesh).Side(1).Share(1) = 1     'This is what Triangle
    Mesh(NumMesh).Side(1).Share(2) = False 'It Belongs to.
    Mesh(NumMesh).Side(2).Share(1) = 1     '
    Mesh(NumMesh).Side(2).Share(2) = False '
    Mesh(NumMesh).Side(3).Share(1) = 1     '
    Mesh(NumMesh).Side(3).Share(2) = False '

End Sub

Private Function Arccos(Z As Double) As Double
    Arccos = Atn(-Z / Sqr(1 - Z * Z)) + Pi * 0.5
End Function

Private Function CheckAngles(MeshNum As Integer, TriNum As Integer) As Integer

Dim A As Double  ' side a                   AA [1]
Dim b As Double  ' side b                   /\
Dim c As Double  ' side c                  /  \
                 '                        /    \
Dim AA As Double ' Angle A              b/      \c
                 '                      /        \
                 '               CC [3]/__________\ BB [2]
Dim COSAA As Double '                       a
Dim COSAB As Double
Dim COSAC As Double
Dim D As Double  ' Distance

    '
    ' Get the Lengths of the Sides
    ' D = sqr( ((x2-x1)^2) + ((y2-y1)^2) )
    '
    D = Sqr(((Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(2)).X - Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(1)).X) ^ 2) + ((Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(2)).Y - Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(1)).Y) ^ 2))
    c = D
    D = Sqr(((Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(3)).X - Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(2)).X) ^ 2) + ((Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(3)).Y - Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(2)).Y) ^ 2))
    A = D
    D = Sqr(((Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(1)).X - Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(3)).X) ^ 2) + ((Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(1)).Y - Mesh(MeshNum).Vertex(Mesh(MeshNum).Triangle(TriNum).V(3)).Y) ^ 2))
    b = D

    'Debug.Print "a: ", a
    'Debug.Print "b: ", a
    'Debug.Print "c: ", a
    '
    'Apply the Law of Cosines
    '
    'Only 1 angle needs to be checked to find
    'out if the entire triangle is valid.
    '
    COSAA = (b ^ 2 + c ^ 2 - A ^ 2) / (2 * b * c)
    COSAB = (A ^ 2 + c ^ 2 - b ^ 2) / (2 * A * c)
    COSAC = (A ^ 2 + b ^ 2 - c ^ 2) / (2 * A * b)

    'Debug.Print "Cos A: ", COSAA
    'Debug.Print "Cos B: ", COSAB
    'Debug.Print "Cos C: ", COSAC
    '
    ' Find inverse cosine of COSAA for degrees
    ' Arccos(Z)=Atn(-Z / Sqr(-Z * Z + 1)) + PI*0.5
    '                                  or * 1.5708 ?
    ' AA = Atn(-COSAA / Sqr(-COSAA * COSAA + 1)) + 1.5708
    
    If ((COSAA > 0.98) Or (COSAA < -0.98)) Or ((COSAB > 0.98) Or (COSAB < -0.98)) Or ((COSAC > 0.98) Or (COSAC < -0.98)) Then
        CheckAngles = -1
    Else
        CheckAngles = 1
    End If

 ' Tim had a better idea (go figure!)
 '
 ' 1. Find the sides vectors from the vertices connected to the side
 '
 ' vx1 = X2-X1
 ' vy1 = Y2-Y1
 '
 ' (for both sides)
 ' vx2 = X2-X1
 ' vy2 = Y2-Y1
 '
 '
 ' 2. Find the Dot product of the Vectors?(I'm not sure where this comes in)
 '
 ' dp = (x1 * x2) + (y1 * y2)
 '
 ' 2. Find the Normals of the vectors
 '
 ' nx = x / sqr((X^2) + (Y^2))
 ' ny = y / sqr((X^2) + (Y^2))
 '
 ' 3. Find the Signs of the Normals Using the Cross product stuff
 '
 ' cp = (X1 * Y2) - (X2 * Y1)
 '
 ' the signs will be different if they are facing away from each other.
 '
 '


End Function

Private Sub ClearPolyData()
    Dim i As Integer
    Dim j As Integer
    Dim k As Integer
    '
    ' Init all lines to invisible
    '
    For i = 0 To MaxLines
        If Loaded(i) Then
            Unload Ln(i)
            Loaded(i) = False
        End If
    Next i
    '

    For i = 1 To NumMesh
        Mesh(i).Add = False
        For j = 1 To Mesh(i).NumVertices
            Mesh(i).Vertex(j).X = 0
            Mesh(i).Vertex(j).Y = 0
            Mesh(i).Vertex(j).IndexPointer = 0
            For k = 1 To Mesh(i).Vertex(j).SidesTouching
                Mesh(i).Vertex(j).index(k) = 0
            Next k
            Mesh(i).Vertex(j).SidesTouching = 0
        Next j
        Mesh(i).NumVertices = 0
        
        For j = 1 To Mesh(i).NumSides
            Mesh(i).Side(j).SV(1) = 0
            Mesh(i).Side(j).SV(2) = 0
            Mesh(i).Side(j).Share(1) = False
            Mesh(i).Side(j).Share(2) = False
        Next j
        Mesh(i).NumSides = 0
        
        For j = 1 To Mesh(i).NumTriangles
            Mesh(i).Triangle(j).Exists = False
            For k = 1 To 3
                Mesh(i).Triangle(j).V(k) = 0
                Mesh(i).Triangle(j).S(k) = 0
            Next k
        Next j
        Mesh(i).NumTriangles = 0

    Next i

    NumMesh = 0
    '
    ' Set origin to 0,0
    '
    OX = 0
    OY = 0
    '
End Sub

Private Sub Command1_Click()
Call ShowValidOutside(CurMesh)
End Sub

Private Sub Command2_Click()
Call ShowStats
End Sub

Private Sub DebugShape_Click()
    DebugShape.Checked = Not DebugShape.Checked

    If DebugShape.Checked Then
        ShowDebug
    Else
        HideDebug
    End If

End Sub

Private Sub Delete_Click()
    'If (CurMesh <> 0) Then
        DeletePoly (CurMesh)
        CurMesh = 1
        CurVertex = 1
        CurSide = 1
        DrawAll
    'End If
End Sub

Private Sub DeletePoly(Num As Integer)
    Dim i As Integer
    Dim j As Integer
    Dim L As Integer
    '
    ' Hide the faces and unload all lines:
    '
    For j = 1 To Mesh(Num).NumSides
        L = (Num - 1) + (j - 1) * MaxSides
        If Loaded(L) Then
            Unload Ln(L)
            Loaded(L) = False
        End If
    Next j


    '
    ' Delete the entire polygon
    '

    For i = Num To NumMesh - 1
        Mesh(i) = Mesh(i + 1)
    Next i
    
    '
    ' Hide the faces and unload all lines:
    '
    For j = 1 To Mesh(NumMesh).NumSides
        L = (NumMesh - 1) + (j - 1) * MaxSides
        If Loaded(L) Then
            Unload Ln(L)
            Loaded(L) = False
        End If
    Next j

    ' uninit
    Mesh(NumMesh).NumSides = 0
    Mesh(NumMesh).NumVertices = 0
    Mesh(NumMesh).NumTriangles = 0
    
    NumMesh = NumMesh - 1
    
    DrawAll
    If NumMesh = 0 Then Call AddNewMesh(True)
End Sub

Private Sub DelSideRef(DelSide As Integer)
'Search through all the Vertecis and delete the side referecnces to DelSide
Dim i, j, k, L As Integer
Dim AlreadyFound As Integer
'Debug.Print "DelSide"

    Mesh(CurMesh).Side(DelSide).Exists = False
    For i = 1 To Mesh(CurMesh).NumVertices
        AlreadyFound = False
        For j = 1 To Mesh(CurMesh).Vertex(i).SidesTouching
        'Debug.Print "Index: ", Mesh(CurMesh).Vertex(i).index(1), Mesh(CurMesh).Vertex(i).index(2), Mesh(CurMesh).Vertex(i).index(3), Mesh(CurMesh).Vertex(i).index(4)
            If (AlreadyFound) Or (Mesh(CurMesh).Vertex(i).index(j) = DelSide) Then
                'Delete reference and sort
                AlreadyFound = True
                Mesh(CurMesh).Vertex(i).index(j) = Mesh(CurMesh).Vertex(i).index(j + 1)
                'Debug.Print "Index: ", Mesh(CurMesh).Vertex(i).index(1), Mesh(CurMesh).Vertex(i).index(2), Mesh(CurMesh).Vertex(i).index(3), Mesh(CurMesh).Vertex(i).index(4)
                If j = Mesh(CurMesh).Vertex(i).SidesTouching Then
                    Mesh(CurMesh).Vertex(i).index(Mesh(CurMesh).Vertex(i).SidesTouching) = 0
                    Mesh(CurMesh).Vertex(i).IndexPointer = 1
                    Mesh(CurMesh).Vertex(i).SidesTouching = Mesh(CurMesh).Vertex(i).SidesTouching - 1
                End If
            End If
        Next j
    Next i

    Mesh(CurMesh).Side(DelSide).Exists = False

    L = (CurMesh - 1) + (DelSide - 1) * MaxSides
    If Loaded(L) Then
        Unload Ln(L)
        Loaded(L) = False
    End If


End Sub

Private Sub DelVertRef(V As Integer)
Dim i As Integer
    'delete the Index connected to this Vertex
    For i = 1 To Mesh(CurMesh).Vertex(V).SidesTouching
    Mesh(CurMesh).Vertex(V).index(i) = 0
    Next i
    Mesh(CurMesh).Vertex(V).SidesTouching = 0
    Mesh(CurMesh).Vertex(V).IndexPointer = 0
    'Delete the Vertex
    Mesh(CurMesh).Vertex(V).Exists = False
    Mesh(CurMesh).Vertex(V).X = 0
    Mesh(CurMesh).Vertex(V).Y = 0

End Sub

Private Sub DrawAll()
    Dim i As Integer, j As Integer, k As Integer
    Dim SideToDraw As Integer
    '
    'Draw All the triangles
    '
'Debug.Print "DrawAll"
    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumTriangles
            If (Mesh(i).Triangle(j).Exists = True) Then
                For k = 1 To 3
                    SideToDraw = Mesh(i).Triangle(j).S(k)
                        'Debug.Print "Tried to draw triangle : ", j
                        'Debug.Print "DrawAll", i, j, k
                        Call DrawSide(i, SideToDraw, 1)
                    'End If
                Next k
            End If
        Next j
    Next i

    
    
    
    ' Set the Vertex Marker
    If (CurMesh <> 0) And (CurVertex <> 0) Then
        VertexMarker.Left = Mesh(CurMesh).Vertex(CurVertex).X - 4 * Zoom
        VertexMarker.Top = Mesh(CurMesh).Vertex(CurVertex).Y - 4 * Zoom
        VertexMarker.Visible = True
    Else
        VertexMarker.Visible = False
    End If
    '

    Call ReOrderLayers
    
    'Reset the grid and texture centering
    Me.Grid1.Left = -Me.Grid1.Width / 2
    Me.Grid1.Top = -Me.Grid1.Height / 2
    Me.Image1.Left = -Me.Image1.Width / 2
    Me.Image1.Top = -Me.Image1.Height / 2

End Sub

Private Sub DrawGrid()



Grid1.Visible = True

Call ReOrderLayers


' Old Grid Code
'Dim i As Integer
'Dim j As Integer
'Dim ct As Integer
'
'
'    ct = 1
'    i = 0
'    While i < 20 '(Width / 10)
'        If GLoaded(ct) = False Then
'            GLoaded(ct) = True
'            Load GridLn(ct)
'        End If
'        GridLn(ct).X1 = i * (GScale) - (GScale * 10) '210
'        GridLn(ct).Y1 = Int(-Height / 2)
'        GridLn(ct).X2 = i * (GScale) - (GScale * 10) '210
'        GridLn(ct).Y2 = Int(Height / 2)
'        GridLn(ct).Visible = True
'        i = i + 1
'        ct = ct + 1
'    Wend
'
'
'    i = 0
'    While i < 20 '(Width / 10)
'        If GLoaded(ct) = False Then
'            GLoaded(ct) = True
'            Load GridLn(ct)
'        End If
'        GridLn(ct).Y1 = i * GScale - (GScale * 10)
'        GridLn(ct).X1 = -Width / 2
'        GridLn(ct).Y2 = i * GScale - (GScale * 10)
'        GridLn(ct).X2 = Width / 2
'        GridLn(ct).Visible = True
'        i = i + 1
'        ct = ct + 1
'    Wend
'
End Sub

Public Sub DrawSide(MeshNum As Integer, SideNum As Integer, Thick As Integer)

Dim LNum As Integer

'
' This Procedure is mostly cosmetic (for showing the current side, etc.)
' but may eventually be used when a vertex is moved
'
'Debug.Print "DrawSide"
    LNum = (MeshNum - 1) + (SideNum - 1) * MaxSides

    If (Not Loaded(LNum)) Then
        Load Ln(LNum)
        Loaded(LNum) = True
    End If
    
    
    Ln(LNum).X1 = Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(SideNum).SV(1)).X
    Ln(LNum).Y1 = Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(SideNum).SV(1)).Y
    Ln(LNum).X2 = Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(SideNum).SV(2)).X
    Ln(LNum).Y2 = Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(SideNum).SV(2)).Y

    
    Ln(LNum).Visible = True
    Ln(LNum).BorderWidth = Thick
    
    If Mesh(MeshNum).Add = True Then
        Ln(LNum).BorderColor = &HFF8080
    Else
        Ln(LNum).BorderColor = &HFF&
    End If

End Sub

Private Sub DrawTriangle(MeshNum As Integer, TriNum As Integer)
    '
    'Draw the triangle
    '
'Debug.Print "DrawTriangle"
    Call DrawSide(MeshNum, Mesh(MeshNum).Triangle(TriNum).S(1), 1)
    Call DrawSide(MeshNum, Mesh(MeshNum).Triangle(TriNum).S(2), 1)
    Call DrawSide(MeshNum, Mesh(MeshNum).Triangle(TriNum).S(3), 1)
    
    
    ' Set the Vertex Marker
    If (CurMesh <> 0) And (CurVertex <> 0) Then
        VertexMarker.Left = Mesh(CurMesh).Vertex(CurVertex).X - 4 * Zoom
        VertexMarker.Top = Mesh(CurMesh).Vertex(CurVertex).Y - 4 * Zoom
        VertexMarker.Visible = True
    Else
        VertexMarker.Visible = False
    End If
    '


End Sub

Private Sub Extrude2D_Click()
    frmExtrude.Show
End Sub

Private Sub ExtrudeBevel_Click()
    frmBevel.Show
End Sub

Private Sub ExtrudePoint_Click()
'Dim N As Integer
'Dim i As Integer
'Dim j As Integer
'Dim k As Integer
'
'Dim CurrentX As Single
'Dim CurrentY As Single
'Dim CurrentZ As Single
'Dim Vdir As Integer
'
'Dim Vert1 As Integer
'Dim Vert2 As Integer
'Dim TempVert As Integer
'Dim WorkSide As Integer
'Dim Group As String
'Dim Depth As Integer
'Dim LoftPlane As String
'Dim ct As Integer
'    ' Load the Parameter Form
'    Call InitBrush("2dEditor")
'    Load frmExtrude
    frmExPoint.Show
'    'Grab the Values
'    Group = (frmExtrude.txtGroupName.Text)
'    Depth = Val(frmExtrude.txtDepth.Text)
'    If frmExtrude.optLoftX = True Then
'        LoftPlane = "X"
'    ElseIf frmExtrude.optLoftY = True Then
'        LoftPlane = "Y"
'    ElseIf frmExtrude.optLoftZ = True Then
'        LoftPlane = "Z"
'    End If
'    'If Cancel was pressed
'    If frmExtrude.btnCancel.Default = True Then
'        Unload frmExtrude
'        Exit Sub
'    End If
'    Unload frmExtrude
'    If CurMesh = 0 Then CurMesh = 1
'    N = 0
'    For ct = 1 To NumMesh
'        For i = 1 To Mesh(ct).NumTriangles
'            If Mesh(ct).Triangle(i).Exists Then
'                'Draw the Top
'                N = N + 1
'                InitBrushPoly (N)
'                Brush.Polys(N).Group = Group
'                Brush.Polys(N).Item = Group & "TOP"
'                Brush.Polys(N).NumVertices = 3 'Triangles
'                For j = 1 To 3
'                    If LoftPlane = "X" Then
'                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
'                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
'                        CurrentX = Depth / 2 'for now I just want 1 side
'                        Vdir = 4 - j
'                    ElseIf LoftPlane = "Y" Then
'                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
'                        CurrentZ = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
'                        CurrentY = Depth / 2 'for now I just want 1 side
'                        Vdir = 4 - j
'                    ElseIf LoftPlane = "Z" Then
'                        CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
'                        CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
'                        CurrentZ = Depth / 2 'for now I just want 1 side
'                        Vdir = j
'                    End If
'                    Call PutVertex(N, Vdir, CurrentX - OX, CurrentY - OY, CurrentZ)
'                Next j
'                'Check the sides to see if any of them are outside edges
'                For j = 1 To 3
'                    WorkSide = Mesh(ct).Triangle(i).S(j)
'                    If Mesh(ct).Side(WorkSide).Share(2) = False Then
'                        If (Mesh(ct).Side(WorkSide).Exists = True) Then
'                            Call DrawSide(ct, WorkSide, 2)
'                            'Find and sort the 2 vertices connected to this side
'                            Vert1 = 0
'                            Vert2 = 0
'                            For k = 1 To 3
'                                If (Mesh(ct).Triangle(i).V(k) = Mesh(ct).Side(WorkSide).SV(1)) Or (Mesh(ct).Triangle(i).V(k) = Mesh(ct).Side(WorkSide).SV(2)) Then
'                                    'This vertex is part of this side
'                                    If Vert1 = 0 Then
'                                        Vert1 = Mesh(ct).Triangle(i).V(k)
'                                    Else
'                                        Vert2 = Mesh(ct).Triangle(i).V(k)
'                                        'Swap if last pass
'                                        If (k = 3) And (j <> 2) Then
'                                            TempVert = Vert1
'                                            Vert1 = Vert2
'                                            Vert2 = TempVert
'                                        End If
'                                    End If
'                                End If
'                            Next k
'                            Debug.Print "Top: ", Vert1, Vert2
'                            'Draw the side
'                            N = N + 1
'                            InitBrushPoly (N)
'                            Brush.Polys(N).Group = Group
'                            Brush.Polys(N).Item = Group & "SIDE"
'                            Brush.Polys(N).NumVertices = 3 'Triangles
'                            If LoftPlane = "X" Then
'                                CurrentZ = Mesh(ct).Vertex(Vert1).X
'                                CurrentY = Mesh(ct).Vertex(Vert1).Y
'                                CurrentX = Depth / 2
'                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
'                                CurrentZ = Mesh(ct).Vertex(Vert2).X
'                                CurrentY = Mesh(ct).Vertex(Vert2).Y
'                                CurrentX = Depth / 2
'                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
'                                CurrentZ = OX
'                                CurrentY = OY
'                                CurrentX = -Depth / 2
'                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
'                            ElseIf LoftPlane = "Y" Then
'                                CurrentX = Mesh(ct).Vertex(Vert1).X
'                                CurrentZ = Mesh(ct).Vertex(Vert1).Y
'                                CurrentY = Depth / 2
'                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
'                                CurrentX = Mesh(ct).Vertex(Vert2).X
'                                CurrentZ = Mesh(ct).Vertex(Vert2).Y
'                                CurrentY = Depth / 2
'                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
'                                CurrentX = OX
'                                CurrentZ = OY
'                                CurrentY = -Depth / 2
'                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
'                            ElseIf LoftPlane = "Z" Then
'                                CurrentX = Mesh(ct).Vertex(Vert1).X
'                                CurrentY = Mesh(ct).Vertex(Vert1).Y
'                                CurrentZ = Depth / 2
'                                Call PutVertex(N, 3, CurrentX - OX, CurrentY - OY, CurrentZ)
'                                CurrentX = Mesh(ct).Vertex(Vert2).X
'                                CurrentY = Mesh(ct).Vertex(Vert2).Y
'                                CurrentZ = Depth / 2
'                                Call PutVertex(N, 2, CurrentX - OX, CurrentY - OY, CurrentZ)
'                                CurrentX = OX
'                                CurrentY = OY
'                                CurrentZ = -Depth / 2
'                                Call PutVertex(N, 1, CurrentX - OX, CurrentY - OY, CurrentZ)
'                            End If
'                        End If ' Outside edge exists
'                    End If 'Outside edge
'                Next j
'            End If 'triangle exists
'        Next i
'    Next ct
'
'    Brush.NumPolys = N
'    Call SendBrush(0)
'
End Sub

Private Sub FlipH_Click()
Dim i As Integer
Dim j As Integer
Dim k As Integer
Dim TempV As Integer
    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumVertices
            If Mesh(i).Vertex(j).Exists Then
                    Mesh(i).Vertex(j).Y = -Mesh(i).Vertex(j).Y
            End If
        Next j
    Next i

    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumTriangles
            If Mesh(i).Triangle(j).Exists Then
                'TempV = Mesh(i).Triangle(j).V(1)
                'Mesh(i).Triangle(j).V(1) = Mesh(i).Triangle(j).V(3)
                'Mesh(i).Triangle(j).V(2) = Mesh(i).Triangle(j).V(2)' uhh this is stupid Beavis
                'Mesh(i).Triangle(j).V(3) = TempV
                TempV = Mesh(i).Triangle(j).V(2)
                Mesh(i).Triangle(j).V(2) = Mesh(i).Triangle(j).V(3)
                Mesh(i).Triangle(j).V(3) = TempV

            End If
        Next j
    Next i


DrawAll


End Sub

Private Sub FlipV_Click()
Dim i As Integer
Dim j As Integer
Dim k As Integer
Dim TempV As Integer
    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumVertices
            If Mesh(i).Vertex(j).Exists Then
                    Mesh(i).Vertex(j).X = -Mesh(i).Vertex(j).X
                ''Re order vertices clockwise
                'TempV = Mesh(i).Triangle(j).V(1)
                'Mesh(i).Triangle(j).V(1) = Mesh(i).Triangle(j).V(3)
                'Mesh(i).Triangle(j).V(2) = Mesh(i).Triangle(j).V(2)' uhh this is stupid Beavis
                'Mesh(i).Triangle(j).V(3) = TempV

            End If
        Next j
    Next i


    DrawAll


End Sub

Private Sub Form_Load()
    Dim i As Integer
    Call Ed.SetOnTop(Me, "2DEditor", TOP_NORMAL)
    '
    ' Init poly data
    '
    For i = 0 To MaxLines - 1
        Loaded(i) = 0
    Next i
    
    'Old Grid Code
    'For i = 1 To 40
    '    GLoaded(i) = False
    'Next i
    '
    
    
    Zoom = 1
    Grid = True

    ClearPolyData
    AddNewMesh (True)
    SetViewPort
    DrawAll
    Me.Grid1.Left = 0 - Me.Grid1.Width / 2
    Me.Grid1.Top = 0 - Me.Grid1.Height / 2
      
    '
End Sub

Private Sub Form_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Call SetNearest(X, Y, 0)
    If CurMesh <> 0 Then
        DragPoint = 1 ' Drag point
    ElseIf Abs(X - OX) + Abs(Y - OY) < 16 Then
        DragPoint = 2 ' Drag origin
    Else
        DragPoint = 0 ' Drag nothing
    End If
    
MakeUndo

End Sub

Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim i As Integer, j, k, L, N, GX As Integer, GY As Integer
    Dim OldX As Integer
    Dim OldY As Integer
    '
    If Grid Then
        GX = GScale * Int((X) / GScale)
        GY = GScale * Int((Y) / GScale)
    Else
        GX = X
        GY = Y
    End If
    '


'Debug.Print "Form_MouseMove"
    If DragPoint = 1 Then
        MousePointer = 2 ' Crosshairs
            For i = 1 To Mesh(CurMesh).NumTriangles
                For j = 1 To 3
                    If Mesh(CurMesh).Triangle(i).V(j) = CurVertex Then
                        'This Vertex is part of Triangle i
                        'To simplify things, we'll just draw the
                        'entire triangle with the new CurVertex Position
                    
                        
                        If CurMesh <> 0 And CurVertex <> 0 Then
                            OldX = Mesh(CurMesh).Vertex(CurVertex).X
                            OldY = Mesh(CurMesh).Vertex(CurVertex).Y
                            Mesh(CurMesh).Vertex(CurVertex).X = GX
                            Mesh(CurMesh).Vertex(CurVertex).Y = GY
                            
                                If AngleLimit Then
                                    If (CheckAngles(CurMesh, i) = -1) Then
                                        GX = OldX
                                        GY = OldY
                                        Mesh(CurMesh).Vertex(CurVertex).X = GX
                                        Mesh(CurMesh).Vertex(CurVertex).Y = GY
                                        X = GX
                                        Y = GY
                                        DragPoint = 0 'Stop Draging
                                    End If
                                End If

                            '
                            ' Set lines and marker
                            '
                            '
                            ' This is where I can use the new Vertex(CurVertex).Index(k)
                            ' to move all of the sides that are connected to this vertex
                            
                            For k = 1 To Mesh(CurMesh).Vertex(CurVertex).SidesTouching
                            ' I can also add it and if then to always draw the Thick(current) side.
                                If Mesh(CurMesh).Vertex(CurVertex).index(k) = CurSide Then
                                    Call DrawSide(CurMesh, Mesh(CurMesh).Vertex(CurVertex).index(k), 2)
                                Else
                                    Call DrawSide(CurMesh, Mesh(CurMesh).Vertex(CurVertex).index(k), 1)
                                End If
                            Next k

                            'Call DrawTriangle(CurMesh, i)
                            
                            
                            
                            VertexMarker.Left = GX - 4 * Zoom
                            VertexMarker.Top = GY - 4 * Zoom

                        End If
                    End If
                Next j
            Next i

            'Check all of the triangles in the mesh to see if
            'CurVertex is within each triangle. If it is, draw lines to the other 2
            'verticies in that triangle. It could be as simple as redrawing the entire
            'triangle, but it might be faster to make a new procedure to do this.
            'This is also the area that I need to check angles. If I make sure an angle
            'is never moved past 178 or under 2 degrees then it will never be out of whack.
            '
            ' The formula only has to be used on 1 vertex per triangle.
            ' Use the LAW of Cosines since we can easily derive the the
            ' lengths of the triangles sides.
            '
            ' Distance Formula:
            '      D = sqrt( ((x2-x1)^2) + ((y2 - y1)^2) )
            '
            ' Law of Cosines: (use the middle point[V(2)] in a triangle set)
            '
            '               b^2 - a^2 - c^2
            '      Cos B = ------------------
            '                    -2ac
            '
            ' Ref. pg. 380 College Algebra and Trigonometry

            
            
        'End If





    ElseIf DragPoint = 2 Then
        Call SetOrigin(Int(GX), Int(GY))
    Else
        '
        ' Call SetNearest with JustCursor=true to set
        ' the crosshair cursor when appropriate.
        '
        Call SetNearest(X, Y, True)
    End If
    '
    'NumSidesHere.Caption = Mesh(CurMesh).Vertex(CurVertex).NumSides
    txtOriginX.Caption = OX
    txtOriginY.Caption = OY
    txtVertexX.Caption = Mesh(CurMesh).Vertex(CurVertex).X
    txtVertexY.Caption = Mesh(CurMesh).Vertex(CurVertex).Y
 
End Sub

Private Sub Form_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If DragPoint = 2 Then
        Call SetViewPort ' Move screen around origin
        Call DrawAll
    End If
    DragPoint = 0
End Sub

Private Sub Form_Resize()
    If Not Resizing Then
        Resizing = True
        If WindowState = 1 Then
            ' Minimized.
            Show
            SetFocus
        Else
            ' Set and redraw everything.
            SetViewPort
            DrawAll
        End If
        Resizing = False
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    ClearPolyData
    Call Ed.EndOnTop(Me)
End Sub

Private Sub GridSnap_Click()
    GridSnap.Checked = Not GridSnap.Checked
    Grid = (GridSnap.Checked)
End Sub

Private Sub HideDebug()
    Label1.Visible = False
    Label2.Visible = False
    Label3.Visible = False
    Label4.Visible = False
    Label5.Visible = False
    Label6.Visible = False
    Label7.Visible = False
    Label8.Visible = False
    Label9.Visible = False
    Label10.Visible = False
    Label11.Visible = False
    Label12.Visible = False
    Label13.Visible = False
    Label14.Visible = False
    Label15.Visible = False
    Label16.Visible = False
    Label17.Visible = False
    Label18.Visible = False
    Label19.Visible = False
    SCurSide.Visible = False
    SCurTri.Visible = False
    SNumMesh.Visible = False
    SNumTri.Visible = False
    SNumSides.Visible = False
    SNumVert.Visible = False
    SSExists.Visible = False
    SShare1.Visible = False
    SShare2.Visible = False
    Ssv1.Visible = False
    Ssv2.Visible = False
    STs1.Visible = False
    STs2.Visible = False
    STs3.Visible = False
    STv1.Visible = False
    STv2.Visible = False
    STv3.Visible = False
    txtOriginX.Visible = False
    txtOriginY.Visible = False
    txtVertexX.Visible = False
    txtVertexY.Visible = False
    Command1.Visible = False
    Command2.Visible = False
End Sub

Private Sub HideGrid()

Grid1.Visible = False


' Old Grid Code
'Dim i As Integer
'Dim j As Integer
'Dim ct As Integer
'
'    ct = 1
'    i = 0
'    While i < 20 '(Width / 10)
'        GridLn(ct).Visible = False
'        i = i + 1
'        ct = ct + 1
'    Wend
'
'
'    i = 0
'    While i < 20 '(Width / 10)
'        GridLn(ct).Visible = False
'        i = i + 1
'        ct = ct + 1
'    Wend


End Sub

Private Sub Help2D_Click()
    ToolHelp (127)
End Sub

Private Sub HideImage_Click()
        
    HideImage.Checked = Not HideImage.Checked
    If HideImage.Checked = True Then
        Me.Image1.Visible = False
    Else
        Me.Image1.Visible = True
    End If

End Sub

Private Sub LimitAngles_Click()
    LimitAngles.Checked = Not LimitAngles.Checked
    AngleLimit = (LimitAngles.Checked)
End Sub

Private Sub LoadImage_Click()

Dim TexturePic As String
    '
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.TwoDeeTexture.filename = ""
    frmDialogs.TwoDeeTexture.ShowOpen
    '
    On Error GoTo ErrHand
    If (frmDialogs.TwoDeeTexture.filename <> "") Then
        Me.Image1.Picture = LoadPicture(frmDialogs.TwoDeeTexture.filename)
        If Me.HideImage.Checked = False Then Me.Image1.Visible = True
        
        'Center the image
        Me.Image1.Left = -Me.Image1.Width / 2
        Me.Image1.Top = -Me.Image1.Height / 2
        
        Call ReOrderLayers
      
    End If
Skip:
    Ed.ServerEnable
    Exit Sub

ErrHand:
    On Error GoTo 0
    MsgBox "Couldn't open 2D Shape (Probably bad format)", 48
    Exit Sub
    

End Sub

Private Sub mnuUndo_Click()
Dim i As Integer


Mesh(CurMesh) = MeshUndo(LastMesh)

'erase all the lines
'The real lines will reload in Drawall
For i = 0 To MaxLines
    If Loaded(i) Then
        Unload Ln(i)
        Loaded(i) = False
    End If
Next i


DrawAll

End Sub

Private Sub New2D_Click()
    Fname = ""
    frmDialogs.TwoDeeSave.filename = ""
    Caption = ModuleName
    ClearPolyData
    Call SetOrigin(0, 0)
    AddNewMesh (True)
    DrawAll
   
End Sub

Private Sub NewAdd_Click()
    AddNewMesh (True)
    DrawAll
End Sub

Private Sub NewSub_Click()
    AddNewMesh (False)
    DrawAll
End Sub

Private Sub Open2D_Click()
    Dim File
    Dim S As String
    Dim i, j, N As Integer
    Dim Trash As String
    '
    On Error GoTo Skip
    Ed.ServerDisable
    frmDialogs.TwoDeeOpen.filename = ""
    frmDialogs.TwoDeeOpen.ShowOpen 'Modal File-Open Box
    Ed.ServerEnable
    '
    On Error GoTo ErrHand
    If (frmDialogs.TwoDeeOpen.filename <> "") Then
        File = FreeFile
        Open frmDialogs.TwoDeeOpen.filename For Input As #File
        ClearPolyData
        NumMesh = 1

ReadHeading:
        '
        ' Read "Add", "Subtract", or "Origin"
        '
        If EOF(File) Then GoTo Done
        Input #File, S
        If UCase(Left(S, 3)) = "ADD" Then
            Mesh(NumMesh).Add = True
            S = Mid(S, 4)
        ElseIf UCase(Left(S, 8)) = "SUBTRACT" Then
            Mesh(NumMesh).Add = False
            S = Mid(S, 9)
        ElseIf UCase(Left(S, 6)) = "ORIGIN" Then
            Input #File, OX, OY
            GoTo ReadHeading
        Else
            GoTo ReadHeading
        End If
        '
        ' Read num Vertecies
        '
        Input #File, S
        N = Val(S)
        If (N = 0) Then
            MsgBox "Couldn't open 2D Shape (Probably bad format)", 48
            GoTo ExitBad
        End If

        Mesh(NumMesh).NumVertices = N
        '
        ' Read the points
        '
        For i = 1 To N
            Input #File, Mesh(NumMesh).Vertex(i).Exists
            Input #File, Mesh(NumMesh).Vertex(i).X, Mesh(NumMesh).Vertex(i).Y
            Input #File, Mesh(NumMesh).Vertex(i).IndexPointer
            Input #File, Mesh(NumMesh).Vertex(i).SidesTouching
            For j = 1 To Mesh(NumMesh).Vertex(i).SidesTouching
                Input #File, Mesh(NumMesh).Vertex(i).index(j)
            Next j
        Next i

        Input #File, S
        N = Val(S)
        If (N = 0) Then
            MsgBox "Couldn't open 2D Shape (Probably bad format)", 48
            GoTo ExitBad
        End If

        Mesh(NumMesh).NumSides = N
        '
        ' Read the Sides
        '
        For i = 1 To N
            Input #File, Mesh(NumMesh).Side(i).Exists
            Input #File, Mesh(NumMesh).Side(i).SV(1), Mesh(NumMesh).Side(i).SV(2)
            Input #File, Mesh(NumMesh).Side(i).Share(1), Mesh(NumMesh).Side(i).Share(2)
        Next i


        Input #File, S
        N = Val(S)
        If (N = 0) Then
            MsgBox "Couldn't open 2D Shape (Probably bad format)", 48
            GoTo ExitBad
        End If

        Mesh(NumMesh).NumTriangles = N
        '
        ' Read the Sides
        '
        For i = 1 To N
            Input #File, Mesh(NumMesh).Triangle(i).Exists
            Input #File, Mesh(NumMesh).Triangle(i).V(1), Mesh(NumMesh).Triangle(i).V(2), Mesh(NumMesh).Triangle(i).V(3)
            Input #File, Mesh(NumMesh).Triangle(i).S(1), Mesh(NumMesh).Triangle(i).S(2), Mesh(NumMesh).Triangle(i).S(3)
        Next i



        NumMesh = NumMesh + 1
        GoTo ReadHeading
Done:
        NumMesh = NumMesh - 1
        Close #File
        End If
    
    On Error GoTo 0
    Call SetOrigin(OX, OY)
    SetViewPort
    DrawAll
    Fname = frmDialogs.TwoDeeOpen.filename
    Caption = ModuleName + " - " + Fname
    Exit Sub

ExitBad:
    On Error GoTo 0
    Close #File
    ClearPolyData
    AddNewMesh (True)
    Exit Sub

ErrHand:
    On Error GoTo 0
    MsgBox "Couldn't open 2D Shape (Probably bad format)", 48
    ClearPolyData
    AddNewMesh (True)
Skip:
End Sub

Private Sub Revolve2D_Click()
    frmRevolve.Show
End Sub

Private Sub Rotate_Click()
Dim i As Integer
Dim j As Integer
Dim k As Integer
Dim TempX As Integer

'    For i = 1 To NumMesh
'        For j = 1 To Mesh(i).NumTriangles
'            If Mesh(i).Triangle(j).Exists Then
'                For k = 1 To 3
'                    'Swap Variables
'                    TempX = Mesh(i).Vertex(Mesh(i).Triangle(j).V(k)).X
'                    Mesh(i).Vertex(Mesh(i).Triangle(j).V(k)).X = Mesh(i).Vertex(Mesh(i).Triangle(j).V(k)).Y
'                    Mesh(i).Vertex(Mesh(i).Triangle(j).V(k)).Y = -TempX
'                Next k
'            End If
'            drawall
'            MsgBox "hey"
 '       Next j
'    Next i

    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumVertices
            If Mesh(i).Vertex(j).Exists Then
                'Swap Variables
                TempX = Mesh(i).Vertex(j).X
                Mesh(i).Vertex(j).X = Mesh(i).Vertex(j).Y
                Mesh(i).Vertex(j).Y = -TempX
            End If
            'drawall
            'MsgBox "hey"
        Next j
    Next i

DrawAll


End Sub

Private Sub Save2D_Click()
    Dim File
    Dim i, j, k As Integer
    '
    If (Fname = "") Then
        On Error GoTo Skip
        Ed.ServerDisable
        frmDialogs.TwoDeeSave.Flags = 2 'Prompt if overwrite
        frmDialogs.TwoDeeSave.ShowSave 'Modal Save-As Box
        Fname = frmDialogs.TwoDeeSave.filename
        Ed.ServerEnable
    End If
    '
    On Error GoTo SaveErrHand
    If (Fname <> "") Then

        On Error GoTo SaveErrHand
        File = FreeFile
        Open Fname For Output As #File
        '
        Print #File, "ORIGIN"
        Print #File, "   " & OX & "," & OY
        Print #File,
        '
        For i = 1 To NumMesh
            If Mesh(i).Add Then
                Print #File, "ADD "
            Else
                Print #File, "SUBTRACT "
            End If

            'Save the Vertecies
            Print #File, "   " & Mesh(i).NumVertices
            For j = 1 To Mesh(i).NumVertices
                
                Print #File, "      " & Mesh(i).Vertex(j).Exists
                
                Print #File, "      " & Mesh(i).Vertex(j).X & "," & Mesh(i).Vertex(j).Y
                                    
                Print #File, "      " & Mesh(i).Vertex(j).IndexPointer
                '
                Print #File, "      " & Mesh(i).Vertex(j).SidesTouching
                For k = 1 To Mesh(i).Vertex(j).SidesTouching
                    Print #File, "      " & Mesh(i).Vertex(j).index(k)
                Next k
            Next j


            'Save the Sides
            Print #File, "   " & Mesh(i).NumSides
            For j = 1 To Mesh(i).NumSides
                Print #File, "      " & Mesh(i).Side(j).Exists
                Print #File, "      " & Mesh(i).Side(j).SV(1) & "," & Mesh(i).Side(j).SV(2)
                Print #File, "      " & Mesh(i).Side(j).Share(1) & "," & Mesh(i).Side(j).Share(2)
            Next j
            
            'Save the Triangles
            Print #File, "   " & Mesh(i).NumTriangles
            For j = 1 To Mesh(i).NumTriangles
                Print #File, "      " & Mesh(i).Triangle(j).Exists
                Print #File, "      " & Mesh(i).Triangle(j).V(1) & "," & Mesh(i).Triangle(j).V(2) & "," & Mesh(i).Triangle(j).V(3)
                Print #File, "      " & Mesh(i).Triangle(j).S(1) & "," & Mesh(i).Triangle(j).S(2) & "," & Mesh(i).Triangle(j).S(3)
            Next j



            Print #File,
        Next i
        Close #File
    
    
    
    
    
    End If
    Caption = ModuleName + " - " + Fname
    Exit Sub
SaveErrHand:
    MsgBox "Couldn't save 2D Shape", 48
    Exit Sub
Skip:
    Ed.ServerEnable
End Sub

Private Sub SaveAs2D_Click()
    Fname = ""
    Save2D_Click
End Sub

'
' Look at a mouse-click position and set the nearest
' vertex and side in CurMesh, CurVertex, and CurSide,
' and graphically highlight them.
'
Private Sub SetNearest(X As Single, Y As Single, JustCursor As Integer)
    Dim L, i, j, Del, BestDel, Del1, Del2, VC1, VC2 As Integer
    Dim OldMesh As Integer, OldVertex As Integer, OldSide As Integer
    '
    OldMesh = CurMesh
    OldVertex = CurVertex
    OldSide = CurSide
    '
    ' If any side/vertex is already selected, hide it:
    '
'Debug.Print "SetNearest : 1"
    If (Not JustCursor) Then
        If (CurMesh <> 0) And (CurVertex <> 0) Then
            VertexMarker.Visible = False
        End If
        '
        If (CurMesh <> 0) And (CurSide <> 0) Then
            Call DrawSide(CurMesh, CurSide, 1)
        End If
        '
        CurMesh = 0
        CurVertex = 0
        CurSide = 0
    End If
    '
    BestDel = -1
    '
    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumVertices
            If Mesh(i).Vertex(j).Exists = True Then
                Del = Abs(Mesh(i).Vertex(j).X - X) + Abs(Mesh(i).Vertex(j).Y - Y)
                If (Del < 16) And ((BestDel = -1) Or (Del < BestDel)) Then
                    If (JustCursor) Then
                        MousePointer = 2 ' Crosshairs
                        Exit Sub
                    End If
                    BestDel = Del
                    CurMesh = i
                    CurVertex = j
                End If
            End If
        Next j
    Next i
    '
    If JustCursor Then
        MousePointer = 0 ' Regular
        Exit Sub
    End If
    '
    ' If a point was close, figure out which side is
    ' closest (sides are associated with the vertex
    ' that forms their starting point).
    '
    
'Debug.Print "SetNearest : 2"
    If CurVertex <> 0 Then
        VC1 = Mesh(CurMesh).Vertex(CurVertex).IndexPointer - 1
        If (VC1 < 1) Then VC1 = Mesh(CurMesh).Vertex(CurVertex).SidesTouching
        '
        ' See if dude re-clicked on same vertex (in order
        ' to toggel line selection):
        '
        If (CurMesh = OldMesh) And (CurVertex = OldVertex) And (Mesh(CurMesh).Vertex(CurVertex).Exists) Then
            CurSide = Mesh(CurMesh).Vertex(CurVertex).index(VC1)
            Mesh(CurMesh).Vertex(CurVertex).IndexPointer = VC1
        Else
            CurSide = Mesh(CurMesh).Vertex(CurVertex).index(Mesh(CurMesh).Vertex(CurVertex).IndexPointer)
        End If
        
        '
        ' Highlight side & vertex:
        '
        VertexMarker.Left = Mesh(CurMesh).Vertex(CurVertex).X - 4 * Zoom
        VertexMarker.Top = Mesh(CurMesh).Vertex(CurVertex).Y - 4 * Zoom
        VertexMarker.Visible = True
        '
        If CurSide = 0 Then
            MsgBox "CurSide = 0 ARRRGGG!"
            MsgBox "This Shape will now Self Destruct"
            Exit Sub
        End If

        Call DrawSide(CurMesh, CurSide, 2)
        'Debug.Print "Current Triangle", Mesh(CurMesh).Side(CurSide).Share(1)
        '
    End If
    
    
End Sub

Private Sub SetOrigin(NewOX As Integer, NewOY As Integer)
    OX = NewOX
    OY = NewOY
    '
    Origin1.X1 = OX - 10 * Zoom
    Origin1.X2 = OX + 10 * Zoom
    Origin1.Y1 = OY
    Origin1.Y2 = OY
    '
    Origin2.X1 = OX
    Origin2.X2 = OX
    Origin2.Y1 = OY + 10 * Zoom
    Origin2.Y2 = OY - 10 * Zoom
End Sub

Private Sub SetViewPort()
    Dim W, H As Integer
    Dim SW, SH
    '
    ' Set window to pixel settings, origin in center.
    '
    
    ' Check to see if form is minimized before changing the scale
    
    If (frmTwoDee.WindowState <> 1) And (Zoom <> 0) Then '1 is Minimized
        ScaleMode = 0 ' Twips (reset)
        ScaleMode = 1 ' Pixels
        W = ScaleWidth
        H = ScaleHeight
        '
        ScaleWidth = W * Zoom / Screen.TwipsPerPixelX
        ScaleHeight = H * Zoom / Screen.TwipsPerPixelY

        ScaleLeft = -(ScaleWidth / 2) + OX
        ScaleTop = -(ScaleHeight / 2) + OY
        '
        Call SetOrigin(OX, OY)
        '
    End If
    
End Sub

Private Sub ShapeIn2x_Click()
Dim i As Integer
Dim j As Integer

    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumVertices
            If Mesh(i).Vertex(j).Exists Then
                Mesh(i).Vertex(j).X = Mesh(i).Vertex(j).X / 2
                Mesh(i).Vertex(j).Y = Mesh(i).Vertex(j).Y / 2
            End If
        Next j
    Next i
    DrawAll

End Sub

Private Sub ShapeOut2x_Click()
Dim i As Integer
Dim j As Integer

    For i = 1 To NumMesh
        For j = 1 To Mesh(i).NumVertices
            If Mesh(i).Vertex(j).Exists Then
                Mesh(i).Vertex(j).X = Mesh(i).Vertex(j).X * 2
                Mesh(i).Vertex(j).Y = Mesh(i).Vertex(j).Y * 2
            End If
        Next j
    Next i
    DrawAll
End Sub

Private Sub ShowCurrentTriangle(Num As Integer)
Dim LNum As Integer
Dim SideNum As Integer
Dim SideCCW As Integer
Dim CheckVar As Integer
Dim VertexOpposite As Integer
Dim SidePositionInTriangle As Integer
Dim i As Integer
    
    For i = 1 To 3
    SideNum = Mesh(CurMesh).Triangle(Num).S(i)
    LNum = (CurMesh - 1) + (SideNum - 1) * MaxSides
    
    If (Not Loaded(LNum)) Then
        Load Ln(LNum)
        Loaded(LNum) = True
    End If
    
    Ln(LNum).X1 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(1)).X
    Ln(LNum).Y1 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(1)).Y
    Ln(LNum).X2 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(2)).X
    Ln(LNum).Y2 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(2)).Y
    Ln(LNum).Visible = True
    Ln(LNum).BorderWidth = 1
    Ln(LNum).BorderColor = &H888888
    Next i
    'MsgBox "This is the current triangle."

    'Debug.Print
    'Debug.Print "Show Current Triangle"
    'Debug.Print "====================="
    'Debug.Print "Vertex 1: ", Mesh(CurMesh).Triangle(Num).V(1)
    'Debug.Print "Vertex 2: ", Mesh(CurMesh).Triangle(Num).V(2)
    'Debug.Print "Vertex 3: ", Mesh(CurMesh).Triangle(Num).V(3)
    'Debug.Print "Side 1  : ", Mesh(CurMesh).Triangle(Num).S(1)
    'Debug.Print "Side 2  : ", Mesh(CurMesh).Triangle(Num).S(2)
    'Debug.Print "Side 3  : ", Mesh(CurMesh).Triangle(Num).S(3)
    'Debug.Print "Current Side: ", CurSide
    
    For i = 1 To 3
        If Mesh(CurMesh).Triangle(Num).S(i) = CurSide Then
            SidePositionInTriangle = i
        End If
    Next i
    'Figure out which side is immeadiately CCW to this one
    SideCCW = SidePositionInTriangle - 1
    If SideCCW < 1 Then SideCCW = 3
    'Debug.Print "Side CCW    : ", SideCCW
    
    For i = 1 To 3
        CheckVar = Mesh(CurMesh).Triangle(Num).V(i)
        If (CheckVar <> (Mesh(CurMesh).Side(CurSide).SV(1))) And (CheckVar <> Mesh(CurMesh).Side(CurSide).SV(2)) Then
            VertexOpposite = CheckVar
        End If
    Next i
    
    'Debug.Print "Vertex Opposite: ", VertexOpposite

    'Debug.Print

End Sub

Private Sub ShowDebug()
    Label1.Visible = True
    Label2.Visible = True
    Label3.Visible = True
    Label4.Visible = True
    Label5.Visible = True
    Label6.Visible = True
    Label7.Visible = True
    Label8.Visible = True
    Label9.Visible = True
    Label10.Visible = True
    Label11.Visible = True
    Label12.Visible = True
    Label13.Visible = True
    Label14.Visible = True
    Label15.Visible = True
    Label16.Visible = True
    Label17.Visible = True
    Label18.Visible = True
    Label19.Visible = True
    SCurSide.Visible = True
    SCurTri.Visible = True
    SNumMesh.Visible = True
    SNumTri.Visible = True
    SNumSides.Visible = True
    SNumVert.Visible = True
    SSExists.Visible = True
    SShare1.Visible = True
    SShare2.Visible = True
    Ssv1.Visible = True
    Ssv2.Visible = True
    STs1.Visible = True
    STs2.Visible = True
    STs3.Visible = True
    STv1.Visible = True
    STv2.Visible = True
    STv3.Visible = True
    txtOriginX.Visible = True
    txtOriginY.Visible = True
    txtVertexX.Visible = True
    txtVertexY.Visible = True
    Command1.Visible = True
    Command2.Visible = True

End Sub

Private Sub Sheet2D_Click()
    
    Dim N As Integer
    Dim ct As Integer
    Dim i As Integer
    Dim j As Integer
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single

    If CurMesh = 0 Then CurMesh = 1
    
    N = 0
    
    For ct = 1 To NumMesh
        For i = 1 To Mesh(ct).NumTriangles
            If Mesh(ct).Triangle(i).Exists Then

                'Draw the Sheet
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = "Sheet"
                Brush.Polys(N).Item = "Sheet" & "Custom"
                Brush.Polys(N).NumVertices = 3 'Triangles
                Brush.Polys(N).Flags = PF_NOTSOLID
                For j = 1 To 3
                    CurrentX = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).X
                    CurrentY = Mesh(ct).Vertex(Mesh(ct).Triangle(i).V(j)).Y
                    CurrentZ = 0 'for now I just want 1 side
                    
                    Call PutVertex(N, j, CurrentX - OX, CurrentY - OY, CurrentZ)
                Next j
                
            End If 'triangle exists
        Next i
    Next ct
    
    
    Brush.NumPolys = N
    
    Call SendBrush(0)
    Call Ed.StatusText("Built a Sheet")
    

End Sub

Private Sub ShowGrid_Click()
    ShowGrid.Checked = Not ShowGrid.Checked
    If ShowGrid.Checked = True Then
        DrawGrid
    Else
        HideGrid
    End If
End Sub

Private Sub ShowStats()
Dim Tri As Integer

    SNumMesh.Caption = NumMesh 'CurMesh
    SNumTri.Caption = Mesh(CurMesh).NumTriangles
    SNumSides.Caption = Mesh(CurMesh).NumSides
    SNumVert.Caption = Mesh(CurMesh).NumVertices

    SCurSide.Caption = CurSide
    If Mesh(CurMesh).Side(CurSide).Exists Then
        SSExists.Caption = "T"
    Else
        SSExists.Caption = "F"
    End If

    Ssv1.Caption = Mesh(CurMesh).Side(CurSide).SV(1)
    Ssv2.Caption = Mesh(CurMesh).Side(CurSide).SV(2)
    SShare1.Caption = Mesh(CurMesh).Side(CurSide).Share(1)
    SShare2.Caption = Mesh(CurMesh).Side(CurSide).Share(2)

    Tri = Mesh(CurMesh).Side(CurSide).Share(1)
    SCurTri.Caption = Tri
    STv1.Caption = Mesh(CurMesh).Triangle(Tri).V(1)
    STv2.Caption = Mesh(CurMesh).Triangle(Tri).V(2)
    STv3.Caption = Mesh(CurMesh).Triangle(Tri).V(3)
    STs1.Caption = Mesh(CurMesh).Triangle(Tri).S(1)
    STs2.Caption = Mesh(CurMesh).Triangle(Tri).S(2)
    STs3.Caption = Mesh(CurMesh).Triangle(Tri).S(3)
    

End Sub

Private Sub ShowTriangle(Num As Integer)
Dim LNum As Integer
Dim SideNum As Integer
Dim i As Integer
    
    For i = 1 To 3
    SideNum = Mesh(CurMesh).Triangle(Num).S(i)
    LNum = (CurMesh - 1) + (SideNum - 1) * MaxSides
    
    If (Not Loaded(LNum)) Then
        Load Ln(LNum)
        Loaded(LNum) = True
    End If
    
    Ln(LNum).X1 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(1)).X
    Ln(LNum).Y1 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(1)).Y
    Ln(LNum).X2 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(2)).X
    Ln(LNum).Y2 = Mesh(CurMesh).Vertex(Mesh(CurMesh).Side(SideNum).SV(2)).Y
    Ln(LNum).Visible = True
    Ln(LNum).BorderWidth = 1
    Ln(LNum).BorderColor = &H888888
    Next i
    'MsgBox "This is the current triangle"

    'Debug.Print
    'Debug.Print "Current Triangle"
    'Debug.Print "================"
    'Debug.Print "Vertex 1: ", Mesh(CurMesh).Triangle(Num).V(1)
    'Debug.Print "Vertex 2: ", Mesh(CurMesh).Triangle(Num).V(2)
    'Debug.Print "Vertex 3: ", Mesh(CurMesh).Triangle(Num).V(3)
    'Debug.Print "Side 1  : ", Mesh(CurMesh).Triangle(Num).S(1)
    'Debug.Print "Side 2  : ", Mesh(CurMesh).Triangle(Num).S(2)
    'Debug.Print "Side 3  : ", Mesh(CurMesh).Triangle(Num).S(3)
    'Debug.Print "Current Side: ", CurSide
    

    'Debug.Print


End Sub

Private Sub ShowValidOutside(MeshNum As Integer)
Const MaxOutSide = 100
Const MaxOutVert = 200
Dim i As Integer
Dim j As Integer
Dim OutCounter As Integer
ReDim OutSide(MaxOutSide) As Integer
Dim CheckSide As Integer
Dim Vert1 As Integer
Dim Vert2 As Integer
Dim VertPoint As Integer
Dim FirstTri As Integer

'Get the outside edges of the shape
    OutCounter = 0
    FirstTri = 0
    For i = 1 To Mesh(MeshNum).NumTriangles
        For j = 1 To 3
            CheckSide = Mesh(MeshNum).Triangle(i).S(j)
            If Mesh(MeshNum).Side(CheckSide).Share(2) = False Then
                If (Mesh(MeshNum).Side(CheckSide).Exists = True) Then
                    Call DrawSide(MeshNum, CheckSide, 2)
                    'Build the OutSide array
                    'OutCounter = OutCounter + 1
                    'OutSide(OutCounter) = CheckSide
                    'If FirstTri = 0 Then FirstTri = i
                End If
            End If
        Next j
    Next i


'sort the outside Vertices in order
' I can do this by checking the first side and getting the first
'side in Clockwise order then find the next side that contains the
'second vertex
'    'Put the side in order
'    For i = 1 To 3
'        If (Mesh(CurMesh).Triangle(FirstTri).V(i) = Mesh(CurMesh).Side(OutSide(1)).SV(1)) Or (Mesh(CurMesh).Triangle(FirstTri).V(i) = Mesh(CurMesh).Side(OutSide(1)).SV(2)) Then
'            'This vertex is part of this side
'           If Vert1 = 0 Then
'                Vert1 = Mesh(CurMesh).Triangle(FirstTri).V(i)
'           Else
'                Vert2 = Mesh(CurMesh).Triangle(FirstTri).V(i)
'           End If
'        End If
'   Next i
    
    'OutVert(1) = Vert1
    'OutVert(2) = Vert2
    'VertPoint = 3
    '
    'For i = 2 To OutCounter
    '    For j = i To OutCounter
     '       If (Mesh(CurMesh).Side(OutSide(i)).SV(1) = OutVert(VertPoint - 1)) Or (Mesh(CurMesh).Side(OutSide(i)).SV(2) = OutVert(VertPoint - 1)) Then
    '            'this is the next side
    '            If (Mesh(CurMesh).Side(OutSide(i)).SV(1) = OutVert(VertPoint - 1)) Then
    '                OutVert(VertPoint) = Mesh(CurMesh).Side(OutSide(i)).SV(2)
     '           Else
    '                OutVert(VertPoint) = Mesh(CurMesh).Side(OutSide(i)).SV(1)
    '            End If
     '           VertPoint = VertPoint + 1
     '       End If
     '   Next j
     'Next i

        
'NumOutVert = VertPoint - 1

End Sub

Private Sub SplitFour(MeshNum As Integer, SideNum As Integer)
'
' Code to split 2 Triangles into 4
'
'MsgBox "You can't split 2 triangles yet"
'Exit Sub

Dim NewVertex As Integer
Dim NewSideBase As Integer
Dim NewSideBisect As Integer
Dim NewSideBisect1 As Integer
Dim NewIndex As Integer
Dim NewTriangle As Integer

Dim SidePositionInTriangle
Dim SidePositionInTriangle1
Dim VertexOpposite As Integer
Dim VertexOpposite1 As Integer

Dim CheckVar As Integer 'Generic
Dim i As Integer

Dim CommonV As Integer
Dim OtherV As Integer
Dim OtherV1 As Integer
Dim SideCCW As Integer
Dim SideCCW1 As Integer

Dim TriangleNum1 As Integer ' Triangle on the left is Share 2
Dim TriangleNum2 As Integer ' Triangle on the right os Share 1
Dim OtherSide As Integer
Dim OtherSide1 As Integer

MakeUndo

' Find the Current Triangle
TriangleNum1 = Mesh(MeshNum).Side(CurSide).Share(1)
TriangleNum2 = Mesh(MeshNum).Side(CurSide).Share(2)


'Call ShowCurrentTriangle(TriangleNum2)

'Debug.Print "SplitFour"

    'The Vertex Opposite is the one in the triangle that isn't on the current side
    For i = 1 To 3
        CheckVar = Mesh(MeshNum).Triangle(TriangleNum2).V(i)
        If (CheckVar <> (Mesh(MeshNum).Side(CurSide).SV(1))) And (CheckVar <> Mesh(MeshNum).Side(CurSide).SV(2)) Then
            VertexOpposite = CheckVar
        End If
    Next i
    'Debug.Print "Vertex Opposite1: ", VertexOpposite
    
    If (Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching + 2) > 20 Then 'MaxSideIndecies in TWODEE.BAS
        MsgBox "Too many Sides touching the Vertex Opposite"
        Exit Sub
    End If


    'Figure out the what position in the current triangle the current side is.
    'This Number will be 1, 2, or 3
    '
    For i = 1 To 3
        If Mesh(MeshNum).Triangle(TriangleNum2).S(i) = CurSide Then
            SidePositionInTriangle = i
        End If
    Next i
    'Figure out which side is immeadiately CCW to this one
    CheckVar = SidePositionInTriangle - 1
    If CheckVar < 1 Then CheckVar = 3

    SideCCW = Mesh(MeshNum).Triangle(TriangleNum2).S(CheckVar)

    'Now find out the 3rd side that isn't Current or CCW (to rebuild old triangle)
    For i = 1 To 3
        CheckVar = Mesh(MeshNum).Triangle(TriangleNum2).S(i)
        If (CheckVar <> CurSide) And (CheckVar <> SideCCW) Then
            OtherSide = CheckVar
        End If
    Next i
    
    
'**********************************************************************************************
'Now do the other side!
'**********************************************************************************************

    'The Vertex Opposite is the one in the triangle that isn't on the current side
    For i = 1 To 3
        CheckVar = Mesh(MeshNum).Triangle(TriangleNum1).V(i)
        If (CheckVar <> (Mesh(MeshNum).Side(CurSide).SV(1))) And (CheckVar <> Mesh(MeshNum).Side(CurSide).SV(2)) Then
            VertexOpposite1 = CheckVar
        End If
    Next i
    'Debug.Print "Vertex Opposite the newest 1: ", VertexOpposite1
    
    If (Mesh(MeshNum).Vertex(VertexOpposite1).SidesTouching + 2) > 20 Then 'MaxSideIndecies in TWODEE.BAS
        MsgBox "Too many Sides touching the Vertex Opposite"
        Exit Sub
    End If

    'Figure out the what position in the current triangle the current side is.
    'This Number will be 1, 2, or 3
    '
    For i = 1 To 3
        If Mesh(MeshNum).Triangle(TriangleNum1).S(i) = CurSide Then
            SidePositionInTriangle1 = i
        End If
    Next i
    'Figure out which side is immeadiately CCW to this one
    CheckVar = SidePositionInTriangle1 - 1
    If CheckVar < 1 Then CheckVar = 3

    SideCCW1 = Mesh(MeshNum).Triangle(TriangleNum1).S(CheckVar)

    'Now find out the 3rd side that isn't Current or CCW (to rebuild old triangle)
    For i = 1 To 3
        CheckVar = Mesh(MeshNum).Triangle(TriangleNum1).S(i)
        If (CheckVar <> CurSide) And (CheckVar <> SideCCW1) Then
            OtherSide1 = CheckVar
        End If
    Next i

    
'******************************************************************************
    
    If (Mesh(MeshNum).NumVertices + 1) > MaxSVertices Then
        MsgBox "Slow down Jackson! Adding this Vertex will exceed the Vertex Limit."
        Exit Sub
    End If

    ' Add a new Vertex and initialize
    Mesh(MeshNum).NumVertices = Mesh(MeshNum).NumVertices + 1
    NewVertex = Mesh(MeshNum).NumVertices
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = 0
    Mesh(MeshNum).Vertex(NewVertex).Exists = True

    ' Now Check to see which Vertex Corresponds to Both Sides
    If Mesh(MeshNum).Side(CurSide).SV(1) = Mesh(MeshNum).Side(SideCCW).SV(1) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(1)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(2)
    ElseIf Mesh(MeshNum).Side(CurSide).SV(2) = Mesh(MeshNum).Side(SideCCW).SV(1) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(2)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(1)
    ElseIf Mesh(MeshNum).Side(CurSide).SV(1) = Mesh(MeshNum).Side(SideCCW).SV(2) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(1)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(2)
    ElseIf Mesh(MeshNum).Side(CurSide).SV(2) = Mesh(MeshNum).Side(SideCCW).SV(2) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(2)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(1)
    End If
    
    
    'Get the position of the new vertex
    Mesh(MeshNum).Vertex(NewVertex).X = (Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(1)).X + Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(2)).X) / 2
    Mesh(MeshNum).Vertex(NewVertex).Y = (Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(1)).Y + Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(2)).Y) / 2
    
    
    
'Add a new Side from the CommonV to the newVertex
    Mesh(MeshNum).NumSides = Mesh(MeshNum).NumSides + 1
    NewSideBase = Mesh(MeshNum).NumSides
    Mesh(MeshNum).Side(NewSideBase).SV(1) = NewVertex
    Mesh(MeshNum).Side(NewSideBase).SV(2) = CommonV
    Mesh(MeshNum).Side(NewSideBase).Exists = True
    
'Link CommonV to the New Side in place of the current side
    For i = 1 To Mesh(MeshNum).Vertex(CommonV).SidesTouching
        If Mesh(MeshNum).Vertex(CommonV).index(i) = CurSide Then
            Mesh(MeshNum).Vertex(CommonV).index(i) = NewSideBase
        End If
    Next i
    

    'Link NewVertex to the NewSide (at the base)
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = Mesh(MeshNum).Vertex(NewVertex).SidesTouching + 1
    Mesh(MeshNum).Vertex(NewVertex).index(Mesh(MeshNum).Vertex(NewVertex).SidesTouching) = NewSideBase
    'Link NewVertex to the CursideSide (at the base)
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = Mesh(MeshNum).Vertex(NewVertex).SidesTouching + 1
    Mesh(MeshNum).Vertex(NewVertex).index(Mesh(MeshNum).Vertex(NewVertex).SidesTouching) = CurSide
    
    
    
    'Make new vertex part of the current side
    Mesh(MeshNum).Side(CurSide).SV(1) = OtherV
    Mesh(MeshNum).Side(CurSide).SV(2) = NewVertex
    

'Add a new Side from the newVertex to the Vertex Opposite the original Side
    Mesh(MeshNum).NumSides = Mesh(MeshNum).NumSides + 1
    NewSideBisect = Mesh(MeshNum).NumSides
    Mesh(MeshNum).Side(NewSideBisect).SV(1) = NewVertex
    
    'Debug.Print "Vertex Opposite2: ", VertexOpposite
    Mesh(MeshNum).Side(NewSideBisect).SV(2) = VertexOpposite
    Mesh(MeshNum).Side(NewSideBisect).Exists = True

    'Link VertexOpposite to the NewSide (bisect)
    Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching = Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching + 1
    Mesh(MeshNum).Vertex(VertexOpposite).index(Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching) = NewSideBisect
    Mesh(MeshNum).Vertex(NewVertex).IndexPointer = 1
    'Link NewVertex to the  NewSides (bisect)
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = Mesh(MeshNum).Vertex(NewVertex).SidesTouching + 1
    Mesh(MeshNum).Vertex(NewVertex).index(Mesh(MeshNum).Vertex(NewVertex).SidesTouching) = NewSideBisect
    
    'Debug.Print "Vertex       : ", NewVertex
    'Debug.Print "SidesTouching: ", Mesh(MeshNum).Vertex(NewVertex).SidesTouching
    'Debug.Print "Link 1       : ", Mesh(MeshNum).Vertex(NewVertex).index(1)
    'Debug.Print "Link 2       : ", Mesh(MeshNum).Vertex(NewVertex).index(2)
    
    'rebuild old triangle
    Mesh(MeshNum).Triangle(TriangleNum2).V(1) = OtherV
    Mesh(MeshNum).Triangle(TriangleNum2).V(2) = VertexOpposite
    Mesh(MeshNum).Triangle(TriangleNum2).V(3) = NewVertex

    Mesh(MeshNum).Triangle(TriangleNum2).S(1) = OtherSide
    Mesh(MeshNum).Triangle(TriangleNum2).S(2) = NewSideBisect  'bisect
    Mesh(MeshNum).Triangle(TriangleNum2).S(3) = CurSide
    


    'Build the New Triangle
    Mesh(MeshNum).NumTriangles = Mesh(MeshNum).NumTriangles + 1
    NewTriangle = Mesh(MeshNum).NumTriangles
    Mesh(MeshNum).Triangle(NewTriangle).Exists = True

    Mesh(MeshNum).Triangle(NewTriangle).V(1) = CommonV
    Mesh(MeshNum).Triangle(NewTriangle).V(2) = NewVertex
    Mesh(MeshNum).Triangle(NewTriangle).V(3) = VertexOpposite

    Mesh(MeshNum).Triangle(NewTriangle).S(1) = NewSideBase  'Base
    Mesh(MeshNum).Triangle(NewTriangle).S(2) = NewSideBisect 'Bisect
    Mesh(MeshNum).Triangle(NewTriangle).S(3) = SideCCW      '
    

    ' The New Triangle now exists
    Mesh(MeshNum).Side(NewSideBisect).Share(1) = TriangleNum2
    Mesh(MeshNum).Side(CurSide).Share(1) = TriangleNum2 'just
    Mesh(MeshNum).Side(NewSideBisect).Share(2) = Mesh(MeshNum).NumTriangles
    Mesh(MeshNum).Side(NewSideBase).Share(1) = Mesh(MeshNum).NumTriangles
    
    If Mesh(MeshNum).Side(SideCCW).Share(2) = TriangleNum2 Then
        Mesh(MeshNum).Side(SideCCW).Share(2) = Mesh(MeshNum).NumTriangles
    Else
        Mesh(MeshNum).Side(SideCCW).Share(1) = Mesh(MeshNum).NumTriangles
    End If

    

    'Debug.Print
    'Debug.Print "Old Triangle"
    Call ShowTriangle(TriangleNum2)
    'Debug.Print
    'Debug.Print "New Triangle"
    Call ShowTriangle(Mesh(MeshNum).NumTriangles)

    'Debug.Print
    'Debug.Print "Bisected Side"
    'Debug.Print "Side Num: ", NewSideBisect
    'Debug.Print "Vertex 1: ", Mesh(MeshNum).Side(NewSideBisect).SV(1)
    'Debug.Print "Vertex 2: ", Mesh(MeshNum).Side(NewSideBisect).SV(2)
    'Debug.Print "Vertex Opposite: ", VertexOpposite
    'Debug.Print



'**********************************************************************************************

'Add a new Side from the newVertex to the Vertex Opposite the original Side
    Mesh(MeshNum).NumSides = Mesh(MeshNum).NumSides + 1
    NewSideBisect1 = Mesh(MeshNum).NumSides
    Mesh(MeshNum).Side(NewSideBisect1).SV(1) = NewVertex
    Mesh(MeshNum).Side(NewSideBisect1).Exists = True
'    Debug.Print "Vertex Opposite on this new side: ", VertexOpposite1
     Mesh(MeshNum).Side(NewSideBisect1).SV(2) = VertexOpposite1


    'Link VertexOpposite to the NewSide (bisect)
    Mesh(MeshNum).Vertex(VertexOpposite1).SidesTouching = Mesh(MeshNum).Vertex(VertexOpposite1).SidesTouching + 1
    Mesh(MeshNum).Vertex(VertexOpposite1).index(Mesh(MeshNum).Vertex(VertexOpposite1).SidesTouching) = NewSideBisect1
    Mesh(MeshNum).Vertex(NewVertex).IndexPointer = 1
    'Link NewVertex to the  NewSides (bisect)
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = Mesh(MeshNum).Vertex(NewVertex).SidesTouching + 1
    Mesh(MeshNum).Vertex(NewVertex).index(Mesh(MeshNum).Vertex(NewVertex).SidesTouching) = NewSideBisect1





    'rebuild old triangle
    Mesh(MeshNum).Triangle(TriangleNum1).V(1) = CommonV         'This is right
    Mesh(MeshNum).Triangle(TriangleNum1).V(2) = VertexOpposite1 '
    Mesh(MeshNum).Triangle(TriangleNum1).V(3) = NewVertex       '

    Mesh(MeshNum).Triangle(TriangleNum1).S(1) = OtherSide1
    Mesh(MeshNum).Triangle(TriangleNum1).S(2) = NewSideBisect1  'bisect
    Mesh(MeshNum).Triangle(TriangleNum1).S(3) = NewSideBase



    'Build the New Triangle
    Mesh(MeshNum).NumTriangles = Mesh(MeshNum).NumTriangles + 1
    NewTriangle = Mesh(MeshNum).NumTriangles
    Mesh(MeshNum).Triangle(NewTriangle).Exists = True

    Mesh(MeshNum).Triangle(NewTriangle).V(1) = OtherV
    Mesh(MeshNum).Triangle(NewTriangle).V(2) = NewVertex
    Mesh(MeshNum).Triangle(NewTriangle).V(3) = VertexOpposite1

    Mesh(MeshNum).Triangle(NewTriangle).S(1) = CurSide  'Base WRONG ONE
    Mesh(MeshNum).Triangle(NewTriangle).S(2) = NewSideBisect1 'Bisect
    Mesh(MeshNum).Triangle(NewTriangle).S(3) = SideCCW1      '


    ' The New Triangles now exists
    'Add in ALL the NEW LINKS incl. bisect1, SideCCW1, NewSideBase, and NewSideBisect
    ' List of Number of Sides
    '       NewSideBisect1 = 2 X
    '       NewSideBase = 2    X
    '       SideCCW1 = 1
    '       OtherSide1 =1
    '       CurSide = 2
    Mesh(MeshNum).Side(NewSideBisect1).Share(1) = TriangleNum1
    Mesh(MeshNum).Side(NewSideBisect1).Share(2) = Mesh(MeshNum).NumTriangles
    Mesh(MeshNum).Side(NewSideBase).Share(2) = TriangleNum1
    'Mesh(MeshNum).Side(SideCCW1).Share(1) = Mesh(MeshNum).NumTriangles
    Mesh(MeshNum).Side(OtherSide1).Share(1) = TriangleNum1
    Mesh(MeshNum).Side(CurSide).Share(2) = Mesh(MeshNum).NumTriangles
    
    If Mesh(MeshNum).Side(SideCCW1).Share(2) = TriangleNum1 Then
        Mesh(MeshNum).Side(SideCCW1).Share(2) = Mesh(MeshNum).NumTriangles
    Else
        Mesh(MeshNum).Side(SideCCW1).Share(1) = Mesh(MeshNum).NumTriangles
    End If
    



'    Debug.Print
'    Debug.Print "Old Triangle"
'    Call ShowTriangle(TriangleNum1)
'    Debug.Print
'    Debug.Print "New Triangle"
'    Call ShowTriangle(Mesh(MeshNum).NumTriangles)
'
'    Debug.Print
'    Debug.Print "Bisected Side"
'    Debug.Print "Side Num: ", NewSideBisect1
'    Debug.Print "Vertex 1: ", Mesh(MeshNum).Side(NewSideBisect1).SV(1)
'    Debug.Print "Vertex 2: ", Mesh(MeshNum).Side(NewSideBisect1).SV(2)
'    Debug.Print "Vertex Opposite: ", VertexOpposite1
'    Debug.Print
'
'    For i = 1 To Mesh(MeshNum).NumTriangles
'        Call ShowCurrentTriangle(i)
'        SCurrentTri.Caption = Str(i)
'        DrawAll
'    Next i

End Sub

Private Sub SplitTwo(MeshNum As Integer, SideNum As Integer)


Dim NewVertex As Integer
Dim NewSideBase As Integer
Dim NewSideBisect As Integer
Dim NewIndex As Integer
Dim NewTriangle As Integer

Dim SidePositionInTriangle
Dim VertexOpposite As Integer

Dim CheckVar As Integer 'Generic
Dim i As Integer

Dim CommonV As Integer
Dim OtherV As Integer
Dim SideCCW As Integer

Dim TriangleNum As Integer
Dim OtherSide As Integer

MakeUndo

' Find the Current Triangle
TriangleNum = Mesh(MeshNum).Side(CurSide).Share(1)

'Call ShowCurrentTriangle(TriangleNum)

'Debug.Print "SplitTwo"

    'The Vertex Opposite is the one in the triangle that isn't on the current side
    For i = 1 To 3
        CheckVar = Mesh(MeshNum).Triangle(TriangleNum).V(i)
        If (CheckVar <> (Mesh(MeshNum).Side(CurSide).SV(1))) And (CheckVar <> Mesh(MeshNum).Side(CurSide).SV(2)) Then
            VertexOpposite = CheckVar
        End If
    Next i
    'Debug.Print "Vertex Opposite1: ", VertexOpposite

    If (Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching + 2) > 20 Then 'MaxSideIndecies in TWODEE.BAS
        MsgBox "Too many Sides touching the Vertex Opposite"
        Exit Sub
    End If

    

    'Figure out the what position in the current triangle the current side is.
    'This Number will be 1, 2, or 3
    '
    For i = 1 To 3
        If Mesh(MeshNum).Triangle(TriangleNum).S(i) = CurSide Then
            SidePositionInTriangle = i
        End If
    Next i
    'Figure out which side is immeadiately CCW to this one
    CheckVar = SidePositionInTriangle - 1
    If CheckVar < 1 Then CheckVar = 3

    SideCCW = Mesh(MeshNum).Triangle(TriangleNum).S(CheckVar)

    'Now find out the 3rd side that isn't Current or CCW (to rebuild old triangle)
    For i = 1 To 3
        CheckVar = Mesh(MeshNum).Triangle(TriangleNum).S(i)
        If (CheckVar <> CurSide) And (CheckVar <> SideCCW) Then
            OtherSide = CheckVar
        End If
    Next i


    'Error Checking
    If (Mesh(MeshNum).NumVertices + 1) > MaxSVertices Then
        MsgBox "Slow down Jackson! Adding this Vertex will exceed the Vertex Limit."
        Exit Sub
    End If


    
    ' Add a new Vertex and initialize
    
    Mesh(MeshNum).NumVertices = Mesh(MeshNum).NumVertices + 1
    NewVertex = Mesh(MeshNum).NumVertices
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = 0
    Mesh(MeshNum).Vertex(NewVertex).Exists = True

    ' Now Check to see which Vertex Corresponds to Both Sides
    If Mesh(MeshNum).Side(CurSide).SV(1) = Mesh(MeshNum).Side(SideCCW).SV(1) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(1)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(2)
    ElseIf Mesh(MeshNum).Side(CurSide).SV(2) = Mesh(MeshNum).Side(SideCCW).SV(1) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(2)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(1)
    ElseIf Mesh(MeshNum).Side(CurSide).SV(1) = Mesh(MeshNum).Side(SideCCW).SV(2) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(1)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(2)
    ElseIf Mesh(MeshNum).Side(CurSide).SV(2) = Mesh(MeshNum).Side(SideCCW).SV(2) Then
        CommonV = Mesh(MeshNum).Side(CurSide).SV(2)
        OtherV = Mesh(MeshNum).Side(CurSide).SV(1)
    End If
    

    
    'Get the position of the new vertex
    Mesh(MeshNum).Vertex(NewVertex).X = (Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(1)).X + Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(2)).X) / 2
    Mesh(MeshNum).Vertex(NewVertex).Y = (Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(1)).Y + Mesh(MeshNum).Vertex(Mesh(MeshNum).Side(CurSide).SV(2)).Y) / 2
    
    
    
'Add a new Side from the CommonV to the newVertex
    If Mesh(MeshNum).NumSides + 2 > MaxSides Then
        MsgBox "Adding this Vertex will exceed the Side Limit."
        'undo
        Mesh(MeshNum).NumVertices = Mesh(MeshNum).NumVertices - 1
        Mesh(MeshNum).Vertex(NewVertex).Exists = False
        Exit Sub
    End If

    Mesh(MeshNum).NumSides = Mesh(MeshNum).NumSides + 1
    NewSideBase = Mesh(MeshNum).NumSides
    Mesh(MeshNum).Side(NewSideBase).SV(1) = NewVertex
    Mesh(MeshNum).Side(NewSideBase).SV(2) = CommonV
    Mesh(MeshNum).Side(NewSideBase).Exists = True
    
'Link CommonV to the New Side in place of the current side
    For i = 1 To Mesh(MeshNum).Vertex(CommonV).SidesTouching
        If Mesh(MeshNum).Vertex(CommonV).index(i) = CurSide Then
            Mesh(MeshNum).Vertex(CommonV).index(i) = NewSideBase
        End If
    Next i
    

    'Link NewVertex to the NewSide (at the base)
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = Mesh(MeshNum).Vertex(NewVertex).SidesTouching + 1
    Mesh(MeshNum).Vertex(NewVertex).index(Mesh(MeshNum).Vertex(NewVertex).SidesTouching) = NewSideBase
    'Link NewVertex to the CursideSide (at the base)
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = Mesh(MeshNum).Vertex(NewVertex).SidesTouching + 1
    Mesh(MeshNum).Vertex(NewVertex).index(Mesh(MeshNum).Vertex(NewVertex).SidesTouching) = CurSide
    
    
    
    'Make new vertex part of the current side
    Mesh(MeshNum).Side(CurSide).SV(1) = OtherV
    Mesh(MeshNum).Side(CurSide).SV(2) = NewVertex
    

'Add a new Side from the newVertex to the Vertex Opposite the original Side
    Mesh(MeshNum).NumSides = Mesh(MeshNum).NumSides + 1
    NewSideBisect = Mesh(MeshNum).NumSides
    Mesh(MeshNum).Side(NewSideBisect).SV(1) = NewVertex
    Mesh(MeshNum).Side(NewSideBisect).Exists = True
    
    'Debug.Print "Vertex Opposite2: ", VertexOpposite
    Mesh(MeshNum).Side(NewSideBisect).SV(2) = VertexOpposite

    'Link VertexOpposite to the NewSide (bisect)
    Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching = Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching + 1
    Mesh(MeshNum).Vertex(VertexOpposite).index(Mesh(MeshNum).Vertex(VertexOpposite).SidesTouching) = NewSideBisect
    Mesh(MeshNum).Vertex(NewVertex).IndexPointer = 1
    'Link NewVertex to the  NewSides (bisect)
    Mesh(MeshNum).Vertex(NewVertex).SidesTouching = Mesh(MeshNum).Vertex(NewVertex).SidesTouching + 1
    Mesh(MeshNum).Vertex(NewVertex).index(Mesh(MeshNum).Vertex(NewVertex).SidesTouching) = NewSideBisect
    
    'Debug.Print "Vertex       : ", NewVertex
    'Debug.Print "SidesTouching: ", Mesh(MeshNum).Vertex(NewVertex).SidesTouching
    'Debug.Print "Link 1       : ", Mesh(MeshNum).Vertex(NewVertex).index(1)
    'Debug.Print "Link 2       : ", Mesh(MeshNum).Vertex(NewVertex).index(2)
    
    'rebuild old triangle
    Mesh(MeshNum).Triangle(TriangleNum).V(1) = OtherV
    Mesh(MeshNum).Triangle(TriangleNum).V(2) = VertexOpposite
    Mesh(MeshNum).Triangle(TriangleNum).V(3) = NewVertex

    Mesh(MeshNum).Triangle(TriangleNum).S(1) = OtherSide
    Mesh(MeshNum).Triangle(TriangleNum).S(2) = NewSideBisect  'bisect
    Mesh(MeshNum).Triangle(TriangleNum).S(3) = CurSide
    


    'Build the New Triangle
    Mesh(MeshNum).NumTriangles = Mesh(MeshNum).NumTriangles + 1
    NewTriangle = Mesh(MeshNum).NumTriangles

    Mesh(MeshNum).Triangle(NewTriangle).V(1) = CommonV
    Mesh(MeshNum).Triangle(NewTriangle).V(2) = NewVertex
    Mesh(MeshNum).Triangle(NewTriangle).V(3) = VertexOpposite

    Mesh(MeshNum).Triangle(NewTriangle).S(1) = NewSideBase  'Base
    Mesh(MeshNum).Triangle(NewTriangle).S(2) = NewSideBisect 'Bisect
    Mesh(MeshNum).Triangle(NewTriangle).S(3) = SideCCW      '
    

    ' The New Triangle now exists
    Mesh(MeshNum).Triangle(NewTriangle).Exists = True
    Mesh(MeshNum).Side(NewSideBisect).Share(1) = TriangleNum
    Mesh(MeshNum).Side(NewSideBisect).Share(2) = Mesh(MeshNum).NumTriangles
    Mesh(MeshNum).Side(NewSideBase).Share(1) = Mesh(MeshNum).NumTriangles

    If Mesh(MeshNum).Side(SideCCW).Share(2) = TriangleNum Then
        Mesh(MeshNum).Side(SideCCW).Share(2) = Mesh(MeshNum).NumTriangles
    Else
        Mesh(MeshNum).Side(SideCCW).Share(1) = Mesh(MeshNum).NumTriangles
    End If



    'Debug.Print
    'Debug.Print "Old Triangle"
    'Call ShowTriangle(TriangleNum)
    'Debug.Print
    'Debug.Print "New Triangle"
    'Call ShowTriangle(Mesh(MeshNum).NumTriangles)

    'Debug.Print
    'Debug.Print "Bisected Side"
    'Debug.Print "Side Num: ", NewSideBisect
    'Debug.Print "Vertex 1: ", Mesh(MeshNum).Side(NewSideBisect).SV(1)
    'Debug.Print "Vertex 2: ", Mesh(MeshNum).Side(NewSideBisect).SV(2)
    'Debug.Print "Vertex Opposite: ", VertexOpposite
    'Debug.Print

End Sub

Private Sub VertDel_Click()
    Dim i, j, k, N, L, NextVertex As Integer
    Dim m, o As Integer
    Dim WorkVertex As Integer
    Dim FoundOutside As Integer

    Dim SideD1 As Integer 'Sides connected to vertex
    Dim SideD2 As Integer '

    Dim CheckSide As Integer 'generic
    Dim SaveSide As Integer
    Dim WorkSide As Integer

    Dim WorkTriangle As Integer

    MousePointer = 11
        DrawAll
        
        
    'I need to trap invalid deletion of a vertex up here!
    FoundOutside = False
    For i = 1 To Mesh(CurMesh).NumTriangles
       For j = 1 To 3
            WorkSide = Mesh(CurMesh).Triangle(i).S(j)
            If Mesh(CurMesh).Side(WorkSide).Share(2) = False Then
                If (Mesh(CurMesh).Side(WorkSide).Exists = True) Then
                    'Call DrawSide(CurMesh, WorkSide, 2)
                    If (Mesh(CurMesh).Side(WorkSide).SV(1) = CurVertex) Or (Mesh(CurMesh).Side(WorkSide).SV(2) = CurVertex) Then FoundOutside = True
                End If
            End If
        Next j
    Next i
    If FoundOutside = False Then
        MsgBox "You can only DELETE a vertex from the outside edge."
        MousePointer = 0
        Exit Sub
    End If



        ' Maybe I should dissassociate the Side.Share
        'Values befor anything else. That way I know that the
        'Resulting Mesh will be valid, and I can just
        'delete the other parts. This makes more sense, since
        'it is the most important part of deleting the Vertex.
        'Piece of cake.
        
        

        'delete the Sides connected to this vertex
        'but keep trac of the sides of these triangles
        'that will still be connected to the mesh.
        'I need to search the triangle for the side that isn't connected to this
        'vertex, so I can change the Share variable accordingly.

        'I also need to delete the index reference from the opposite Vertecies
        'in the sides that are being deleted

        '
        'delete the Sides connected to this vertex

        
        N = Mesh(CurMesh).NumTriangles
        For i = 1 To N
            For j = 1 To 3
                If Mesh(CurMesh).Triangle(i).V(j) = CurVertex Then
                    WorkTriangle = i
                    'find and identify the sides in the triangle
                    For k = 1 To 3
                        CheckSide = Mesh(CurMesh).Triangle(i).S(k)
                        If (Mesh(CurMesh).Side(CheckSide).SV(1) <> CurVertex) And (Mesh(CurMesh).Side(CheckSide).SV(2) <> CurVertex) Then
                            'This is the side to save
                            SaveSide = CheckSide
                        Else 'This side needs to be deleted
                            If SideD1 = 0 Then
                                SideD1 = CheckSide
                            Else
                                SideD2 = CheckSide
                            End If
                        End If
                    Next k

                    ' Put "Share" crap in here
                    '
                    If Mesh(CurMesh).Side(SaveSide).Share(1) = WorkTriangle Then
                        Mesh(CurMesh).Side(SaveSide).Share(1) = Mesh(CurMesh).Side(SaveSide).Share(2)
                    End If
                    Mesh(CurMesh).Side(SaveSide).Share(2) = False

                    If Mesh(CurMesh).Side(SaveSide).Share(1) = 0 Then
                        Call DelSideRef(SaveSide)
                        'I also need to erase any invalid vertices
                        'Hmmm... Can't visualize this stuff
                        'I know! Chech to see if either vertex of this side
                        'is shared by SideD1 or D2. Robert is very competent...
                        If Mesh(CurMesh).Side(SideD1).SV(1) = Mesh(CurMesh).Side(SaveSide).SV(1) Then
                            Call DelVertRef(Mesh(CurMesh).Side(SaveSide).SV(1))
                        ElseIf Mesh(CurMesh).Side(SideD1).SV(2) = Mesh(CurMesh).Side(SaveSide).SV(1) Then
                            Call DelVertRef(Mesh(CurMesh).Side(SaveSide).SV(1))
                        ElseIf Mesh(CurMesh).Side(SideD1).SV(1) = Mesh(CurMesh).Side(SaveSide).SV(2) Then
                            Call DelVertRef(Mesh(CurMesh).Side(SaveSide).SV(2))
                        ElseIf Mesh(CurMesh).Side(SideD1).SV(2) = Mesh(CurMesh).Side(SaveSide).SV(2) Then
                            Call DelVertRef(Mesh(CurMesh).Side(SaveSide).SV(2))
                        Else
                            MsgBox "Couldn't delete one of the vertices. Dang!"
                            MsgBox "This Shape will now Self Destruct."
                            MousePointer = 0
                            Exit Sub
                        End If

                        Call DelSideRef(SaveSide)
                        

                    End If
                        
                    
                    '
                    Call DelSideRef(SideD1)
                    Call DelSideRef(SideD2)

'    'Check to make sure None of the lines are referenced by a vertex
'    For i = 1 To Mesh(CurMesh).NumVertecies
'        For j = 1 To Mesh(CurMesh).Vertex(i).SidesTouching
'            If (Mesh(CurMesh).Vertex(i).Index(j) = SideD1) Or (Mesh(CurMesh).Vertex(i).Index(j) = SideD2) Then
'                MsgBox "Ouch! One of the Deleted lines are still connected or existing."
'                MsgBox "This Shape Will Now Self Destruct!"
'                Exit Sub
'           End If
'       Next j
'    Next i
                    
                    SideD1 = 0
                    SideD2 = 0

                End If
            Next j
        Next i

        '
        'Now, delete the Triangles connected to this vertex
        'this has to be done after saving the sides otherwise only the
        'first triangle searched in the above procedure would be correct.
        For i = 1 To N
            For j = 1 To 3
                If Mesh(CurMesh).Triangle(i).V(j) = CurVertex Then 'delete this triangle
                    Mesh(CurMesh).Triangle(i).Exists = False
                    For k = 1 To 3
                        Mesh(CurMesh).Triangle(i).V(k) = 0
                        Mesh(CurMesh).Triangle(i).S(k) = 0
                    Next k
                End If
            Next j
        Next i

        '
        'Delete the Current Vertex
        Call DelVertRef(CurVertex)

        
        CurVertex = 0 'Mesh(CurMesh).Side(SaveSide).SV(1)
        CurSide = 0 'SaveSide

        DrawAll
        MousePointer = 0

End Sub

Private Sub VertIns_Click()
    Dim i, NextVertex As Integer
    If Mesh(CurMesh).Side(CurSide).Share(2) = False Then
        Call SplitTwo(CurMesh, CurSide) '1 is just temporary
    Else
        'Debug.Print "CurSide Before Split 4"
        'Debug.Print Mesh(CurMesh).Side(CurSide).Share(1)
        'Debug.Print Mesh(CurMesh).Side(CurSide).Share(2)
        'MsgBox "!"
        Call SplitFour(CurMesh, CurSide) '1 is just temporary
    End If

 DrawAll

'Call ShowCurrentTriangle(Mesh(CurMesh).Side(CurSide).Share(1))

 DrawAll

End Sub

Private Sub ZoomIn2X_Click()
    Zoom = Zoom / 2
    SetViewPort
    DrawGrid
    DrawAll
End Sub


Private Sub ZoomOut2X_Click()
    Zoom = Zoom * 2
    SetViewPort
    DrawGrid
    DrawAll
End Sub


Public Sub ReOrderLayers()
   
    ' Reset the texture layer in back
    Image1.ZOrder 1
    ' Reset the grid layer in back behind the texture :)
    Grid1.ZOrder 1

End Sub

Private Sub MakeUndo()
Dim i, j As Integer
'Basically, I need to save the entire
'Shape and restore it when asked.
'
' DOH! This is cheap.

'Purge LastMesh
' I don't need to do this

'MeshUndo(LastMesh).NumVertices = 0
'MeshUndo(LastMesh).NumSides = 0
'MeshUndo(LastMesh).NumTriangles = 0
'
'For i = 1 To MaxSVertices
'    MeshUndo(LastMesh).Vertex(i).Exists = 0
'    MeshUndo(LastMesh).Vertex(i).X = 0
'    MeshUndo(LastMesh).Vertex(i).Y = 0
'    MeshUndo(LastMesh).Vertex(i).SidesTouching = 0
'    MeshUndo(LastMesh).Vertex(i).IndexPointer = 0
'    For j = 1 To MaxSideIndecies
'        MeshUndo(LastMesh).Vertex(i).index(j) = 0
'    Next j
'Next i
'
'For i = 1 To MaxSides
'    MeshUndo(LastMesh).Side(i).Exists = 0
'    MeshUndo(LastMesh).Side(i).SV(1) = 0
'    MeshUndo(LastMesh).Side(i).SV(2) = 0
'    MeshUndo(LastMesh).Side(i).Share(1) = 0
'    MeshUndo(LastMesh).Side(i).Share(2) = 0
'Next i
'
'
'For i = 1 To MaxTriangles
'    MeshUndo(LastMesh).Triangle(i).Exists = 0
'    MeshUndo(LastMesh).Triangle(i).V(1) = 0
'    MeshUndo(LastMesh).Triangle(i).V(2) = 0
'    MeshUndo(LastMesh).Triangle(i).V(3) = 0
'    MeshUndo(LastMesh).Triangle(i).S(1) = 0
'    MeshUndo(LastMesh).Triangle(i).S(2) = 0
'    MeshUndo(LastMesh).Triangle(i).S(3) = 0
'Next i
'
'
'
'
'

''erase all the lines
'For i = 0 To MaxSides
'    If Loaded(i) Then
'        Unload Ln(i)
'        Loaded(i) = False
'    End If
'Next i


LastMesh = CurMesh
MeshUndo(LastMesh) = Mesh(CurMesh)



End Sub
