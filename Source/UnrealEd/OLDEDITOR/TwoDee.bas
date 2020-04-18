Attribute VB_Name = "TWODEE"
Option Explicit

Public Const MaxSideIndecies = 20
Public Const MaxSVertices = 100
Public Const MaxSides = 100
Public Const MaxTriangles = 30

'
' Definitions and such for the 2D shape editor
'
Type VertexType
    Exists As Integer
    X As Single
    Y As Single
    SidesTouching As Integer  'Number of Sides Touching this Vertex
    IndexPointer As Integer ' Keeps track of the Current Index Number
    index(MaxSideIndecies) As Integer 'Index of Sides Touching this Vertex
End Type

Type SideType
    Exists As Integer
    SV(2) As Integer ' each side contains 2 vertecies
    Share(2) As Integer ' This is what triangle it belongs to. Max of 2 triangles per side.
End Type

Type TriangleType
    Exists As Integer
    V(3) As Integer ' 3 Vertecies per Triangle
    S(3) As Integer ' 3 Sides per Triangle
End Type

Type MeshType
   NumVertices As Integer
   NumSides As Integer
   NumTriangles As Integer
   Add As Integer
   Vertex(MaxSVertices) As VertexType
   Side(MaxSides) As SideType
   Triangle(MaxTriangles) As TriangleType
End Type

Const MaxMeshes = 40
'Const MaxSides = 100
'Const MaxSVertices = 100
Public Const GScale = 16 ' Scale of grid snap
Public Const MaxLines = MaxMeshes * MaxSides  'Figure this out logically

Public NumMesh As Integer
Public NumTriangles As Integer
Public Mesh(MaxMeshes) As MeshType
Public MeshUndo(MaxMeshes) As MeshType
Public Loaded(MaxLines) As Integer
'Public GLoaded(40) As Integer 'Grid

Public LastMesh As Integer
Public CurMesh As Integer
Public CurVertex As Integer
Public CurSide As Integer
Public CurTriange As Integer

Public OutVert(MaxSides) As Integer 'No really! Max Sides
Public NumOutVert  As Integer


Public OX As Integer ' Origin
Public OY As Integer

Public Zoom As Single

Public DragPoint As Integer
Public Grid As Integer
Public AngleLimit As Integer


