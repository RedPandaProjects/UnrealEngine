Attribute VB_Name = "Floor"
Option Explicit

Public Const MaxX = 32 + 1
Public Const MaxY = 32 + 1

'Public Const MaxSVertices = 100
Public Const FMaxSides = 100
Public Const FMaxLines = 100
Public Const FMaxVerticies = 100
'
' Definitions and such for the 2D shape editor
'
Type VertexType
    X As Single
    Y As Single
    Z As Single
End Type

Type FloorType
    SizeX As Integer
    SizeY As Integer
    Vertex(MaxX * MaxY) As VertexType
    
End Type

Public Floor1 As FloorType

'Public Const GScale = 16 ' Scale of grid snap


'Public FLoaded(FMaxLines) As Integer


'Public OutVert(FMaxSides) As Integer 'No really! Max Sides

Public FOX As Integer ' Origin
Public FOY As Integer

Public CurFVertex As Integer
Public FloorLevel As Integer

Public FZoom As Single

Public FDragPoint As Integer
Public FGrid As Integer
Public FGScale As Integer

Public FLineRev(MaxX * MaxY) As Boolean
Public FRevChecked(MaxX * MaxY) As Boolean
Public FRevBox(MaxX) As Boolean 'Visuals

Public FPCXData(64000) As Byte

Public BaseDepth As Integer





'Public AngleLimit As Integer


