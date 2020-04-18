VERSION 5.00
Begin VB.Form frmFloorLofter 
   BackColor       =   &H00000000&
   Caption         =   "Terra-U"
   ClientHeight    =   5790
   ClientLeft      =   2385
   ClientTop       =   2730
   ClientWidth     =   10230
   ForeColor       =   &H00000000&
   LinkTopic       =   "Form2"
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5790
   ScaleWidth      =   10230
   Begin VB.CommandButton btnAdvInt 
      Caption         =   "Advanced"
      Height          =   375
      Left            =   7680
      TabIndex        =   19
      Top             =   120
      Width           =   975
   End
   Begin VB.CheckBox chkRevTriangle 
      BackColor       =   &H00000000&
      ForeColor       =   &H00000000&
      Height          =   255
      Index           =   100
      Left            =   960
      TabIndex        =   18
      Top             =   2400
      Visible         =   0   'False
      Width           =   255
   End
   Begin VB.TextBox txtRendScale 
      Height          =   285
      Left            =   7200
      TabIndex        =   17
      Text            =   "1"
      Top             =   120
      Width           =   375
   End
   Begin VB.TextBox txtFloorY 
      Height          =   285
      Left            =   4200
      TabIndex        =   13
      Text            =   "4"
      Top             =   120
      Width           =   375
   End
   Begin VB.TextBox txtFloorX 
      Height          =   285
      Left            =   3600
      TabIndex        =   12
      Text            =   "4"
      Top             =   120
      Width           =   375
   End
   Begin VB.TextBox txtCellSize 
      Height          =   285
      Left            =   5520
      TabIndex        =   10
      Text            =   "64"
      Top             =   120
      Width           =   495
   End
   Begin VB.CommandButton btnLevelInc 
      Caption         =   ">>"
      Height          =   375
      Left            =   2280
      TabIndex        =   7
      Top             =   120
      Width           =   375
   End
   Begin VB.CommandButton btnLevelDec 
      Caption         =   "<<"
      Height          =   375
      Left            =   1800
      TabIndex        =   6
      Top             =   120
      Width           =   375
   End
   Begin VB.CommandButton btnLoft 
      Caption         =   "Render"
      Height          =   375
      Left            =   120
      TabIndex        =   5
      Top             =   120
      Width           =   975
   End
   Begin VB.Label Label2 
      Caption         =   "Render Scale :"
      Height          =   375
      Left            =   6120
      TabIndex        =   16
      Top             =   120
      Width           =   1095
   End
   Begin VB.Label Label9 
      Alignment       =   1  'Right Justify
      Caption         =   "Level : "
      Height          =   255
      Left            =   1080
      TabIndex        =   15
      Top             =   120
      Width           =   735
   End
   Begin VB.Label Label7 
      Alignment       =   2  'Center
      Caption         =   "Terra Grid : "
      Height          =   255
      Left            =   2760
      TabIndex        =   11
      Top             =   120
      Width           =   855
   End
   Begin VB.Label Label8 
      Alignment       =   2  'Center
      Caption         =   "X"
      Height          =   255
      Left            =   3960
      TabIndex        =   14
      Top             =   120
      Width           =   255
   End
   Begin VB.Label Label6 
      Alignment       =   2  'Center
      Caption         =   "Cell Size : "
      Height          =   255
      Left            =   4680
      TabIndex        =   9
      Top             =   120
      Width           =   855
   End
   Begin VB.Line RightSide 
      BorderColor     =   &H000000FF&
      X1              =   6600
      X2              =   6600
      Y1              =   4320
      Y2              =   5040
   End
   Begin VB.Line LeftSide 
      BorderColor     =   &H000000FF&
      X1              =   840
      X2              =   840
      Y1              =   4320
      Y2              =   5040
   End
   Begin VB.Line Base 
      BorderColor     =   &H000000FF&
      X1              =   1200
      X2              =   6360
      Y1              =   5040
      Y2              =   5040
   End
   Begin VB.Shape VertexBlock 
      BackColor       =   &H00000000&
      BackStyle       =   1  'Opaque
      BorderColor     =   &H00000000&
      FillColor       =   &H0080FF80&
      FillStyle       =   0  'Solid
      Height          =   135
      Index           =   100
      Left            =   6240
      Top             =   4200
      Visible         =   0   'False
      Width           =   135
   End
   Begin VB.Shape VertexMarker 
      BorderColor     =   &H0080FFFF&
      Height          =   135
      Left            =   1080
      Shape           =   3  'Circle
      Top             =   4200
      Width           =   135
   End
   Begin VB.Line FLine 
      BorderColor     =   &H00FF0000&
      Index           =   100
      Visible         =   0   'False
      X1              =   1200
      X2              =   6360
      Y1              =   4320
      Y2              =   4320
   End
   Begin VB.Line Line1 
      X1              =   9600
      X2              =   0
      Y1              =   0
      Y2              =   0
   End
   Begin VB.Label txtFloorLevel 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0C0C0&
      Caption         =   " - -"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   255
      Left            =   1320
      TabIndex        =   8
      Top             =   360
      Width           =   375
   End
   Begin VB.Label txtVertexZ 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   " - -"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   255
      Left            =   9720
      TabIndex        =   4
      Top             =   240
      Width           =   375
   End
   Begin VB.Label txtVertexY 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   " - -"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   255
      Left            =   9240
      TabIndex        =   3
      Top             =   240
      Width           =   495
   End
   Begin VB.Label txtVertexX 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   " - -"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   255
      Left            =   8760
      TabIndex        =   2
      Top             =   240
      Width           =   495
   End
   Begin VB.Label txtVertexNum 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   " - -"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   255
      Left            =   9480
      TabIndex        =   1
      Top             =   0
      Width           =   495
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "Vertex :"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   255
      Left            =   8760
      TabIndex        =   0
      Top             =   0
      Width           =   735
   End
   Begin VB.Shape Shape1 
      BackColor       =   &H00C0C0C0&
      BackStyle       =   1  'Opaque
      BorderColor     =   &H00FFFFFF&
      Height          =   615
      Left            =   0
      Top             =   0
      Width           =   10215
   End
   Begin VB.Menu File2D 
      Caption         =   "&File"
      Begin VB.Menu New2D 
         Caption         =   "&New"
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
      Begin VB.Menu Sep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuGetPCX 
         Caption         =   "&Get PCX"
      End
      Begin VB.Menu Sep2 
         Caption         =   "-"
      End
      Begin VB.Menu Exit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnuZoom 
      Caption         =   "&Zoom"
      Begin VB.Menu ZoomIn2x 
         Caption         =   "Zoom In 2x"
      End
      Begin VB.Menu ZoomOut2x 
         Caption         =   "Zoom Out 2x"
      End
   End
   Begin VB.Menu mnuLoft 
      Caption         =   "&Render"
      Begin VB.Menu Loft2D 
         Caption         =   "&Render"
      End
   End
   Begin VB.Menu Help 
      Caption         =   "&Help"
      Enabled         =   0   'False
   End
End
Attribute VB_Name = "frmFloorLofter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Const GWW_HWNDPARENT = (-8)
Dim CellSize As Integer
Dim FloorLofterWord As Integer
Dim RenderScale As Integer
Dim AdvInt As Boolean

'
' Most Declarations are in Floor.bas
'
Dim FLLoaded(FMaxLines) As Boolean
Dim FVLoaded(FMaxLines) As Boolean

Const ModuleName = "Floor Lofter"
Dim Fname As String



Private Sub btnAdvInt_Click()
Dim i As Integer
    
If AdvInt = True Then
    AdvInt = False
Else
    AdvInt = True
End If

DispRevBoxes

End Sub


Private Sub btnLevelDec_Click()
    Dim i As Integer
    If FloorLevel - 1 > 0 Then
        'Unload the lines then redraw
        'For i = 1 To Floor1.SizeX
        '    Unload FLine(i)
        'Next i
        FloorLevel = FloorLevel - 1
        DrawFloorLines
        Me.txtFloorLevel.Caption = FloorLevel
    
    Call SetRevBoxes(FloorLevel)
        
    End If
    
End Sub

Private Sub btnLevelInc_Click()
    Dim i As Integer
    
    If FloorLevel + 1 <= Floor1.SizeY + 1 Then
        
        'For i = 1 To Floor1.SizeX
        '    Unload FLine(i)
        'Next i
        FloorLevel = FloorLevel + 1
        DrawFloorLines
        Me.txtFloorLevel.Caption = FloorLevel
    
        Call SetRevBoxes(FloorLevel)
    
    End If
    
    
End Sub

Private Sub btnLoft_Click()
    SendFloor
End Sub



Private Sub chkRevTriangle_Click(index As Integer)
Dim MagicNumber As Integer

MagicNumber = index + (Floor1.SizeX * (FloorLevel - 1))

'FLineRev(MagicNumber) = Not (FLineRev(MagicNumber))
    
If chkRevTriangle(index).Value = 1 Then
   FLineRev(MagicNumber) = True
Else
   FLineRev(MagicNumber) = False
End If


End Sub


Private Sub Exit_Click()
    
    Unload Me
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
Dim Key As Integer


Select Case KeyCode
    Case vbKeyPageUp
        btnLevelInc_Click
    
    Case vbKeyPageDown
        btnLevelDec_Click
        
    Case vbKeyHome
        FloorLevel = 1
        DrawFloorLines
        Me.txtFloorLevel.Caption = FloorLevel
        Call SetRevBoxes(FloorLevel)
    
    Case vbKeyEnd
        FloorLevel = Floor1.SizeX + 1
        DrawFloorLines
        Me.txtFloorLevel.Caption = FloorLevel
        Call SetRevBoxes(FloorLevel)
    
    
End Select


End Sub

Private Sub Form_Load()
    Dim i As Integer
Const vbKeyPageUp = &H21 '    PAGE UP key.
Const vbKeyPageDown = &H22 '    PAGE DOWN key.
    
    KeyPreview = True ' set up keyboard stuff
    
    Fname = ""
    
    Call Ed.SetOnTop(Me, "FloorLofter", TOP_NORMAL)
    '
    ' Init poly data
    '
    ResetAll
  
'    FZoom = 2
    FZoom = Me.txtFloorX.Text / 8
    FGScale = 8 '16
    BaseDepth = 64
    FloorLevel = 1
    RenderScale = Val(Me.txtRendScale.Text)
    AdvInt = False
    
    Floor1.SizeX = Val(Me.txtFloorX.Text)
    Floor1.SizeY = Val(Me.txtFloorY.Text)
    
    SetFViewPort
    InitFloor
    InitVisuals
    InitRevBoxes
    DispRevBoxes
    Call SetRevBoxes(FloorLevel)
    
    DrawFloorLines
    Call DrawVertexMarkers(1)
    
   

End Sub




Private Sub Form_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
Dim i As Integer
Dim MagicNumber As Integer
    Call SetFNearest(X, Y, 0)
    

For i = 1 To Floor1.SizeX + 1 ' +1 is the closing vertex
    MagicNumber = ((FloorLevel * (Floor1.SizeX + 1)) - Floor1.SizeX) + i - 1
    If (Abs(Floor1.Vertex(MagicNumber).X - X) + Abs(Floor1.Vertex(MagicNumber).Z - Y)) < 16 Then
        CurFVertex = MagicNumber
        Call DrawVertexMarkers(i) ' i is the highlighted one
        FDragPoint = 1 ' moving a vertex
        Exit For
    End If
Next i

If (Abs(Me.Base.Y1 - Y) < 4) Then FDragPoint = 3 ' moving the base line - 2 in the origin
    


'Set Text messages
Me.txtVertexNum.Caption = CurFVertex
Me.txtVertexX.Caption = Floor1.Vertex(CurFVertex).X
Me.txtVertexY.Caption = Floor1.Vertex(CurFVertex).Y
Me.txtVertexZ.Caption = Floor1.Vertex(CurFVertex).Z

End Sub


Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
        
Dim GX As Integer
Dim GY As Integer
Dim MagicNumber As Integer

    GX = FGScale * Int((X) / FGScale)
    GY = FGScale * Int((Y) / FGScale)
        
        
        
    Call SetFNearest(X, Y, True)



    If FDragPoint = 1 Then 'move the vertex
        Floor1.Vertex(CurFVertex).Z = GY 'Z
        
        'Set Text messages for movement
        Me.txtVertexX.Caption = Floor1.Vertex(CurFVertex).X
        Me.txtVertexY.Caption = Floor1.Vertex(CurFVertex).Y
        Me.txtVertexZ.Caption = Floor1.Vertex(CurFVertex).Z
        
        DrawFloorLines
        
    End If

    If FDragPoint = 3 Then ' move the base line
        Me.Base.Y1 = GY
        Me.Base.Y2 = GY
        BaseDepth = GY
        
        Me.LeftSide.Y2 = BaseDepth
        Me.RightSide.Y2 = BaseDepth
    End If

End Sub

Private Sub Form_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If FDragPoint = 2 Then
        Call SetFViewPort ' Move screen around origin
        'Call DrawAll
    End If
    FDragPoint = 0
End Sub


Private Sub Form_Resize()
    
    'Set and redraw everything
    'SetViewport
    'DrawAll
    'SetFViewPort
    'InitVisuals
    'DrawFloorLines
    'Call DrawVertexMarkers(1)
End Sub


Private Sub Form_Unload(Cancel As Integer)
    UnloadAll
    Call Ed.EndOnTop(Me)
End Sub











Private Sub Loft2D_Click()
    
    SendFloor

End Sub

Private Sub mnuGetPCX_Click()
    ReadPCX
End Sub

Private Sub New2D_Click()
    Fname = ""
    Caption = ModuleName
    
    ResetAll
    InitFloor
    InitRevBoxes
    
    FZoom = Me.txtFloorX.Text / 8
    
    SetFViewPort
    
    DrawFloorLines
    DispRevBoxes
    Call SetRevBoxes(FloorLevel)
    
End Sub

Private Sub Open2D_Click()
    Dim File
    Dim S As String
    Dim i, j, N As Integer
    Dim Trash As String
    '
    
    
    Ed.ServerDisable
    frmDialogs.FloorOpen.filename = ""
    frmDialogs.FloorOpen.Action = 1 'Modal File-Open Box
    Ed.ServerEnable
    '
    On Error GoTo ErrHand
    If (frmDialogs.FloorOpen.filename <> "") Then
        File = FreeFile
        Open frmDialogs.FloorOpen.filename For Input As #File
        
        ResetAll 'Clear all data


ReadHeading:
        '
        ' Read "Add", "Subtract", or "Origin"
        '
        If EOF(File) Then GoTo Done
        
        Input #File, FOX, FOY
        
        Input #File, Floor1.SizeX, Floor1.SizeY
        Me.txtFloorX.Text = Floor1.SizeX
        Me.txtFloorY.Text = Floor1.SizeY
        
        Input #File, S
        txtCellSize.Text = S
        
        Input #File, S
        txtRendScale.Text = S
        
        Input #File, BaseDepth
        '
        ' Read the points
        '
        For i = 1 To ((Floor1.SizeX + 1) * (Floor1.SizeY + 1))
            Input #File, Floor1.Vertex(i).X, Floor1.Vertex(i).Y, Floor1.Vertex(i).Z
        Next i
        For i = 1 To Floor1.SizeX * Floor1.SizeY
            Input #File, S
            If S = "True" Then
                FLineRev(i) = True
            Else
                FLineRev(i) = False
            End If
        Next i



Done:
    Close #File
    End If

    On Error GoTo 0
    Call SetFOrigin(FOX, FOY)
    
    Fname = frmDialogs.FloorOpen.filename
    Caption = ModuleName + " - " + Fname
    
    
    SetFViewPort
    InitVisuals
    DispRevBoxes
    Call SetRevBoxes(FloorLevel)
    
    DrawFloorLines
    Call DrawVertexMarkers(1)
   
    
    
    Exit Sub

ExitBad:
    On Error GoTo 0
    Close #File
    ResetAll
    Exit Sub

ErrHand:
    On Error GoTo 0
    MsgBox "Couldn't open 2D Shape (Probably bad format)", 48
    ResetAll
    Exit Sub

End Sub


Private Sub Save2D_Click()
    Dim File
    Dim i, j, k As Integer
    '
    If (Fname = "") Then
        Ed.ServerDisable
        frmDialogs.FloorSave.Flags = 2 'Prompt if overwrite
        frmDialogs.FloorSave.Action = 2 'Modal Save-As Box
        Fname = frmDialogs.FloorSave.filename
        Ed.ServerEnable
    End If
    '
    On Error GoTo SaveErrHand
    If (Fname <> "") Then '

        On Error GoTo SaveErrHand
        File = FreeFile
        Open Fname For Output As #File
        '

        Print #File, "   " & FOX & "," & FOY
        '
        Print #File, "   " & Floor1.SizeX & "," & Floor1.SizeY
        '
        Print #File, "   " & txtCellSize.Text
        '
        Print #File, "   " & txtRendScale.Text
        
        '
        Print #File, "   " & BaseDepth
        
        
        For i = 1 To ((Floor1.SizeX + 1) * (Floor1.SizeY + 1))
            Print #File, "   " & Floor1.Vertex(i).X & "," & Floor1.Vertex(i).Y & "," & Floor1.Vertex(i).Z
        Next i
        
        For i = 1 To Floor1.SizeX * Floor1.SizeY
            Print #File, FLineRev(i)
        Next i
        
        
        Close #File


    End If


    Caption = ModuleName + " - " + Fname
    Exit Sub

SaveErrHand:
    MsgBox "Couldn't save Terra Shape", 48
    Exit Sub

End Sub


Private Sub SaveAs2D_Click()
    Fname = ""
    Save2D_Click
End Sub



Public Sub InitFloor()
Dim i As Integer
Dim j As Integer
Dim Counter As Integer

' clear array
For i = 0 To (MaxX * MaxY)
    Floor1.Vertex(i).X = 0
    Floor1.Vertex(i).Y = 0
    Floor1.Vertex(i).Z = 0
Next i

CellSize = Val(Me.txtCellSize.Text)
'starting values
Counter = 1
For i = 0 To Floor1.SizeY
    For j = 0 To Floor1.SizeX
        Floor1.Vertex(Counter).X = j * CellSize - ((Floor1.SizeX * CellSize) / 2)
        Floor1.Vertex(Counter).Y = i * CellSize - ((Floor1.SizeY * CellSize) / 2)
        Floor1.Vertex(Counter).Z = 0
        
'        Debug.Print (Counter), "X: " & Floor1.Vertex(Counter).X, "Y: " & Floor1.Vertex(Counter).Y
        Counter = Counter + 1
    Next j
Next i



' Set the vertex Marker to the first Vertex
Call SetVertexMarker(Int(Floor1.Vertex(1).X), Int(Floor1.Vertex(1).Y))

'Set Text messages
Me.txtVertexNum.Caption = CurFVertex
Me.txtVertexX.Caption = Floor1.Vertex(CurFVertex).X
Me.txtVertexY.Caption = Floor1.Vertex(CurFVertex).Y
Me.txtVertexZ.Caption = Floor1.Vertex(CurFVertex).Z

End Sub

Public Sub SendFloor()
    Dim i As Integer
    Dim j As Integer
    Dim A As Integer
    Dim Counter As Integer
    Dim Group As String
    Dim MagicNumber As Integer
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single
    Dim N As Integer
    Dim Result As Boolean
    Dim NumSent As Integer
    
    '
    ' Get Parameters
    '
    Call InitBrush("FloorLofter")
    Group = "Floor"
    
    Me.MousePointer = 11 ' wait

    RenderScale = Val(Me.txtRendScale.Text)
    
    N = 0
    NumSent = 0
    

        
        Counter = 0
        For i = 1 To Floor1.SizeY
            For j = 1 To Floor1.SizeX
                Counter = Counter + 1
                MagicNumber = ((i * (Floor1.SizeX + 1)) - Floor1.SizeX) + j - 1
                Result = RenderCell(NumSent, Counter, MagicNumber, N)
                
            Next j
        Next i

        For i = 1 To Floor1.SizeX
            'Build the long sides (X)

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = Group & "XSIDE1"
            Brush.Polys(N).NumVertices = 4 'All squares for now

            CurrentX = Floor1.Vertex(i).X
            CurrentY = Floor1.Vertex(i).Y
            CurrentZ = Floor1.Vertex(i).Z
            Call PutVertex(N, 4, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(i + 1).X
            CurrentY = Floor1.Vertex(i + 1).Y
            CurrentZ = Floor1.Vertex(i + 1).Z
            Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(i + 1).X
            CurrentY = Floor1.Vertex(i + 1).Y
            CurrentZ = BaseDepth
            Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(i).X
            CurrentY = Floor1.Vertex(i).Y
            CurrentZ = BaseDepth
            Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
        
        Next i


        For i = 1 To Floor1.SizeX
            'Build the long sides (X)
            MagicNumber = (((Floor1.SizeY + 1) * (Floor1.SizeX + 1)) - Floor1.SizeX) + i - 1
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = Group & "XSIDE2"
            Brush.Polys(N).NumVertices = 4 'All squares for now

            CurrentX = Floor1.Vertex(MagicNumber).X
            CurrentY = Floor1.Vertex(MagicNumber).Y
            CurrentZ = Floor1.Vertex(MagicNumber).Z
            Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(MagicNumber + 1).X
            CurrentY = Floor1.Vertex(MagicNumber + 1).Y
            CurrentZ = Floor1.Vertex(MagicNumber + 1).Z
            Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(MagicNumber + 1).X
            CurrentY = Floor1.Vertex(MagicNumber + 1).Y
            CurrentZ = BaseDepth
            Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(MagicNumber).X
            CurrentY = Floor1.Vertex(MagicNumber).Y
            CurrentZ = BaseDepth
            Call PutVertex(N, 4, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
        
        Next i


        For i = 1 To Floor1.SizeY
            MagicNumber = ((i * (Floor1.SizeX + 1) - Floor1.SizeX))
           'short sides (Y)
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = Group & "YSIDE1"
            Brush.Polys(N).NumVertices = 4 'All squares for now

            CurrentX = Floor1.Vertex(MagicNumber).X
            CurrentY = Floor1.Vertex(MagicNumber).Y
            CurrentZ = Floor1.Vertex(MagicNumber).Z
            Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(MagicNumber + (Floor1.SizeX + 1)).X
            CurrentY = Floor1.Vertex(MagicNumber + (Floor1.SizeX + 1)).Y
            CurrentZ = Floor1.Vertex(MagicNumber + (Floor1.SizeX + 1)).Z
            Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(MagicNumber + (Floor1.SizeX + 1)).X
            CurrentY = Floor1.Vertex(MagicNumber + (Floor1.SizeX + 1)).Y
            CurrentZ = BaseDepth
            Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

            CurrentX = Floor1.Vertex(MagicNumber).X
            CurrentY = Floor1.Vertex(MagicNumber).Y
            CurrentZ = BaseDepth
            Call PutVertex(N, 4, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)


        If N > 50 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If



       'short sides (Y)
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = Group & "YSIDE2"
        Brush.Polys(N).NumVertices = 4 'All squares for now

        CurrentX = Floor1.Vertex(MagicNumber + Floor1.SizeX).X
        CurrentY = Floor1.Vertex(MagicNumber + Floor1.SizeX).Y
        CurrentZ = Floor1.Vertex(MagicNumber + Floor1.SizeX).Z
        Call PutVertex(N, 4, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

        CurrentX = Floor1.Vertex(MagicNumber + (2 * Floor1.SizeX) + 1).X
        CurrentY = Floor1.Vertex(MagicNumber + (2 * Floor1.SizeX) + 1).Y
        CurrentZ = Floor1.Vertex(MagicNumber + (2 * Floor1.SizeX) + 1).Z
        Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

        CurrentX = Floor1.Vertex(MagicNumber + (2 * Floor1.SizeX) + 1).X
        CurrentY = Floor1.Vertex(MagicNumber + (2 * Floor1.SizeX) + 1).Y
        CurrentZ = BaseDepth
        Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

        CurrentX = Floor1.Vertex(MagicNumber + Floor1.SizeX).X
        CurrentY = Floor1.Vertex(MagicNumber + Floor1.SizeX).Y
        CurrentZ = BaseDepth
        Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

        If N > 50 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
    
    Next i



        ' Bottom
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = Group & "BOTTOM"
        Brush.Polys(N).NumVertices = 4 'All squares for now

        CurrentX = Floor1.Vertex(1).X
        CurrentY = Floor1.Vertex(1).Y
        CurrentZ = BaseDepth
        Call PutVertex(N, 4, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)

        CurrentX = Floor1.Vertex(Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(Floor1.SizeX + 1).Y
        CurrentZ = BaseDepth
        Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)


        CurrentX = Floor1.Vertex(Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(Floor1.SizeX + 1).Y + (CellSize * Floor1.SizeY)
        CurrentZ = BaseDepth
        Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)


        CurrentX = Floor1.Vertex(1).X
        CurrentY = Floor1.Vertex(1).Y + (CellSize * Floor1.SizeY)
        CurrentZ = BaseDepth
        Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)


    Me.MousePointer = 1
    Brush.NumPolys = N
    
        If NumSent > 0 Then
            Call SendBrush(1)
        Else
            Call SendBrush(0)
        End If
        
    
'    Brush.NumPolys = N
'    Debug.Print "N = ", N
'    Call SendBrush(0)
    

End Sub
'
Public Sub SetFViewPort()
    Dim W, H As Integer
    Dim SW, SH
    '
    ' Set window to pixel settings, origin in center.
    '
    
    ' Check to see if form is minimized before changing the scale
    
    If (frmTwoDee.WindowState <> 1) And (FZoom <> 0) Then '1 is Minimized
        ScaleMode = 0 ' Twips (reset)
        ScaleMode = 1 ' Pixels
        W = ScaleWidth
        H = ScaleHeight
        '
        ScaleWidth = W * FZoom / Screen.TwipsPerPixelX
        ScaleHeight = H * FZoom / Screen.TwipsPerPixelY

        ScaleLeft = -(ScaleWidth / 2) + FOX
        ScaleTop = -(ScaleHeight / 2) + FOY
        '
        Call SetFOrigin(FOX, FOY)
        '
    End If

End Sub

Public Sub SetFOrigin(NewOX As Integer, NewOY As Integer)
    FOX = NewOX
    FOY = NewOY
    '
End Sub

Private Sub SetVertexMarker(X As Integer, Y As Integer)
    '
    ' Highlight side & vertex:
    '
    VertexMarker.Left = X - 4 * FZoom
    VertexMarker.Top = Y - 4 * FZoom
    VertexMarker.Visible = True

End Sub

Private Sub DrawFloorLines()
Dim i As Integer
Dim MagicNumber As Integer
    

    For i = 1 To Floor1.SizeX
        If Not (FLLoaded(i)) Then
            Load FLine(i)
            FLLoaded(i) = True
        End If
        MagicNumber = ((FloorLevel * (Floor1.SizeX + 1)) - Floor1.SizeX) + i - 1
        'Draws the line from this point to the next
        FLine(i).X1 = Floor1.Vertex(MagicNumber).X
        FLine(i).Y1 = Floor1.Vertex(MagicNumber).Z
        FLine(i).X2 = Floor1.Vertex(MagicNumber + 1).X
        FLine(i).Y2 = Floor1.Vertex(MagicNumber + 1).Z
        FLine(i).Visible = True
        ' Draw the Vertex Block on the first point of the line
        If Not (FVLoaded(i)) Then
            Load VertexBlock(i)
            FVLoaded(i) = True
        End If
        VertexBlock(i).Left = Floor1.Vertex(MagicNumber).X - (4 * FZoom)
        VertexBlock(i).Top = Floor1.Vertex(MagicNumber).Z - (4 * FZoom)
        VertexBlock(i).Visible = True
        

    
    Next i
    
    ' Draw last Vertex Block
    
    If Not (FVLoaded(i)) Then
        Load VertexBlock(i)
        FVLoaded(i) = True
    End If
    MagicNumber = ((FloorLevel * (Floor1.SizeX + 1)) - Floor1.SizeX) + i - 1
    VertexBlock(i).Left = Floor1.Vertex(MagicNumber).X - (4 * FZoom)
    VertexBlock(i).Top = (Floor1.Vertex(MagicNumber).Z - 4 * FZoom) 'Z
    VertexBlock(i).Visible = True
    
    'Draw left, right, and base walls
    

    Me.Base.X1 = Floor1.Vertex(1).X
    Me.Base.Y1 = BaseDepth
    Me.Base.X2 = Floor1.Vertex(Floor1.SizeX + 1).X
    Me.Base.Y2 = BaseDepth
    
    
    MagicNumber = ((FloorLevel * (Floor1.SizeX + 1)) - Floor1.SizeX)
    
    
    Me.LeftSide.X1 = Floor1.Vertex(MagicNumber).X
    Me.LeftSide.Y1 = Floor1.Vertex(MagicNumber).Z
    Me.LeftSide.X2 = Floor1.Vertex(MagicNumber).X
'    Me.LeftSide.Y2 = BaseDepth
    Me.LeftSide.Y2 = Me.Base.Y1
    
    Me.RightSide.X1 = Floor1.Vertex(MagicNumber + Floor1.SizeX).X
    Me.RightSide.Y1 = Floor1.Vertex(MagicNumber + Floor1.SizeX).Z
    Me.RightSide.X2 = Floor1.Vertex(MagicNumber + Floor1.SizeX).X
 '   Me.RightSide.Y2 = BaseDepth
    Me.RightSide.Y2 = Me.Base.Y2

' Set the vertex Marker to the first Vertex
'Call SetVertexMarker(Int(Floor1.Vertex(1).X), Int(Floor1.Vertex(1).Y))
Call SetVertexMarker(FOX, FOY)


End Sub

'
' Look at a mouse-click position and set the nearest
' vertex and side in CurMesh, CurVertex, and CurSide,
' and graphically highlight them.
'
Private Sub SetFNearest(X As Single, Y As Single, JustCursor As Integer)
Dim i As Integer
Dim MagicNumber As Integer

For i = 1 To Floor1.SizeX + 1
    MagicNumber = ((FloorLevel * (Floor1.SizeX + 1)) - Floor1.SizeX) + i - 1
    If ((Abs(Floor1.Vertex(MagicNumber).X - X) + Abs(Floor1.Vertex(MagicNumber).Z - Y)) < 16) Then
        ' Set Cursor to Crosshairs
        MousePointer = 2 ' Crosshairs
        Exit For
    ElseIf (Abs(Me.Base.Y1 - Y) < 4) Then
        MousePointer = 2
    Else
        ' Set Cursor to Normal
        MousePointer = 1 ' Normal
    End If
    
    
            
       
    
    
Next i



End Sub

Private Sub DrawVertexMarkers(Highlight As Integer)
Dim i As Integer


For i = 1 To Floor1.SizeX + 1 'un highlight all
    If FVLoaded(i) Then
        VertexBlock(i).FillColor = &HC000& 'Dark Green
    Else
        Unload VertexBlock(i)
    End If
    
Next i

If FVLoaded(Highlight) Then
    VertexBlock(Highlight).FillColor = &H80FFFF 'light green
Else
    Unload VertexBlock(i)
End If

End Sub

Private Sub InitVisuals()


Me.Base.X1 = 64 - ((Floor1.SizeX * 64) / 2) - 64 * FZoom
Me.Base.Y1 = BaseDepth * FZoom
Me.Base.X2 = 64 + ((Floor1.SizeX * 64) / 2) - 64 * FZoom
Me.Base.Y2 = BaseDepth * FZoom

Me.LeftSide.X1 = ((Floor1.SizeX * 64) / 2) * FZoom
Me.LeftSide.Y1 = 0 * FZoom
Me.LeftSide.X2 = ((Floor1.SizeX * 64) / 2) * FZoom
Me.LeftSide.Y2 = BaseDepth * FZoom
'
Me.RightSide.X1 = ((Floor1.SizeX * 64) / 2) * FZoom
Me.RightSide.Y1 = 0 * FZoom
Me.RightSide.X2 = ((Floor1.SizeX * 64) / 2) * FZoom
Me.RightSide.Y2 = BaseDepth * FZoom

Me.txtFloorLevel.Caption = FloorLevel

Me.Shape1.Width = Me.Width
Me.Line1.X1 = Me.Width

End Sub

Private Function RenderCell(NumSent As Integer, RevCheck As Integer, CellNum As Integer, N As Integer) As Boolean
Dim i As Integer
Dim MagicNumber As Integer
Dim SplitIt As Boolean
Dim CheckNum As Single
Dim Group As String
Dim CurrentX As Single
Dim CurrentY As Single
Dim CurrentZ As Single
'
'
' A cell is the polygon made by 4 points
'
'
' ***A cell is marked by its upper left vertex!***
'
'If all points are equal render a normal cell
'If not, bisect the cell
'Advanced: If all 4 points of a cell are planar, draw a normal Cell
'
'Hmmm..
'
Group = "Floor"

RenderScale = Val(Me.txtRendScale.Text)


'Debug.Print
'Debug.Print "RendCell : ", CellNum
'Debug.Print
    ' Build the Floor Map
    CheckNum = Floor1.Vertex(CellNum).Z
    'If the key vertex is the same height as the other cell verticies
    If ((CheckNum) = (Floor1.Vertex(CellNum + 1).Z)) And ((CheckNum) = (Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Z)) And ((CheckNum) = (Floor1.Vertex(CellNum + Floor1.SizeX + 1).Z)) Then
      
        N = N + 1
        
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = Group & "TOP"
        Brush.Polys(N).NumVertices = 4 'All squares for now
 '       Brush.Polys(N).Flags = PF_NOTSOLID

        CurrentX = Floor1.Vertex(CellNum).X
        CurrentY = Floor1.Vertex(CellNum).Y
        CurrentZ = Floor1.Vertex(CellNum).Z
        Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ

        CurrentX = Floor1.Vertex(CellNum + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + 1).Z
        Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ

        CurrentX = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Y '
        CurrentZ = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Z
        Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ

        CurrentX = Floor1.Vertex(CellNum + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Z
        Call PutVertex(N, 4, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
       
        RenderCell = False
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
                
                
 '   ElseIf ((CheckNum) < (Floor1.Vertex(CellNum + 1).Z)) Or ((CheckNum) < (Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Z)) Or ((CheckNum) < (Floor1.Vertex(CellNum + Floor1.SizeX + 1).Z)) Then
    ElseIf FLineRev(RevCheck) = False Then
        ' 1 2 3
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = Group & "TOP"
        Brush.Polys(N).NumVertices = 3 'All squares for now
  '      Brush.Polys(N).Flags = PF_NOTSOLID
        
        CurrentX = Floor1.Vertex(CellNum).X
        CurrentY = Floor1.Vertex(CellNum).Y
        CurrentZ = Floor1.Vertex(CellNum).Z
        Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
 '       Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        CurrentX = Floor1.Vertex(CellNum + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + 1).Z
        Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
 '       Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
    
        CurrentX = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Y '
        CurrentZ = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Z
        Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
 '       Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
    
        '1 3 4
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = Group & "TOP"
        Brush.Polys(N).NumVertices = 3 'All squares for now
'        Brush.Polys(N).Flags = PF_NOTSOLID
    
        CurrentX = Floor1.Vertex(CellNum).X
        CurrentY = Floor1.Vertex(CellNum).Y
        CurrentZ = Floor1.Vertex(CellNum).Z
        Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
 '       Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
    
        CurrentX = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Y '
        CurrentZ = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Z
        Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
  '      Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        CurrentX = Floor1.Vertex(CellNum + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Z
        Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
   '     Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
    
        RenderCell = True
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

'    ElseIf ((CheckNum) > (Floor1.Vertex(CellNum + 1).Z)) Or ((CheckNum) > (Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Z)) Or ((CheckNum) > (Floor1.Vertex(CellNum + Floor1.SizeX + 1).Z)) Then
    ElseIf FLineRev(RevCheck) = True Then
  '      Debug.Print "Cell is Reversed"
        '2 3 4
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = Group & "TOP"
        Brush.Polys(N).NumVertices = 3 'All squares for now
 '       Brush.Polys(N).Flags = PF_NOTSOLID
        
        CurrentX = Floor1.Vertex(CellNum + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + 1).Z
        Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        CurrentX = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Y '
        CurrentZ = Floor1.Vertex(CellNum + 1 + Floor1.SizeX + 1).Z
        Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        CurrentX = Floor1.Vertex(CellNum + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Z
        Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        ' 2 4 1
        N = N + 1
        InitBrushPoly (N)
        Brush.Polys(N).Group = Group
        Brush.Polys(N).Item = Group & "TOP"
        Brush.Polys(N).NumVertices = 3 'All squares for now
  '      Brush.Polys(N).Flags = PF_NOTSOLID
        
        CurrentX = Floor1.Vertex(CellNum + 1).X
        CurrentY = Floor1.Vertex(CellNum + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + 1).Z
        Call PutVertex(N, 1, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        CurrentX = Floor1.Vertex(CellNum + Floor1.SizeX + 1).X
        CurrentY = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Y
        CurrentZ = Floor1.Vertex(CellNum + Floor1.SizeX + 1).Z
        Call PutVertex(N, 2, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        CurrentX = Floor1.Vertex(CellNum).X
        CurrentY = Floor1.Vertex(CellNum).Y
        CurrentZ = Floor1.Vertex(CellNum).Z
        Call PutVertex(N, 3, CurrentX * RenderScale, CurrentY * RenderScale, -CurrentZ * RenderScale)
'        Debug.Print (CellNum), "X: " & CurrentX, "Y: " & CurrentY, "Z: " & -CurrentZ
        
        RenderCell = True
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
    
    Else
    MsgBox ("Don't know what to do with this!")
    
    End If
    

End Function

Private Sub Text1_Change()

End Sub



Private Sub ResetAll()
Dim i As Integer
    
    For i = 0 To (MaxX * MaxY)
        Floor1.Vertex(i).X = 0
        Floor1.Vertex(i).Y = 0
        Floor1.Vertex(i).Z = 0
    Next i


    For i = 1 To FMaxLines
        If FLLoaded(i) Then
            FLine(i).Visible = False
            Unload FLine(i)
            FLLoaded(i) = 0
        End If
    Next i

    For i = 1 To FMaxVerticies
        If FVLoaded(i) Then
            VertexBlock(i).Visible = False
            Unload VertexBlock(i)
            FVLoaded(i) = 0
        End If
    Next i
    
    InitRevBoxes
    
    FZoom = 1
    FGScale = 8 '16
    FloorLevel = 1
    BaseDepth = 64
    
    Floor1.SizeX = Val(Me.txtFloorX.Text)
    Floor1.SizeY = Val(Me.txtFloorY.Text)
    
    SetFViewPort
    InitVisuals
    
'    DrawFloorLines
'    Call DrawVertexMarkers(1)

End Sub

Private Sub DispRevBoxes()
Dim i As Integer

    For i = 1 To Floor1.SizeX
        If FRevBox(i) = False Then
            Load chkRevTriangle(i)
            'place the box centered
            
            FRevBox(i) = True
        End If
        
            'place the box centered
            Me.chkRevTriangle(i).Left = Floor1.Vertex(i).X + ((Floor1.Vertex(i + 1).X - Floor1.Vertex(i).X) / 2) - 4 * FZoom ' X
            Me.chkRevTriangle(i).Top = (BaseDepth / 2) - 4 * FZoom ' Y

            'Me.chkRevTriangle(i).Visible = True
        
        If AdvInt = True Then
            Me.chkRevTriangle(i).Visible = True
        Else
            Me.chkRevTriangle(i).Visible = False
        End If
        
    Next i

End Sub

Private Sub InitRevBoxes()
Dim i As Integer

'Unload all checkboxes
For i = 1 To MaxX
    If (FRevBox(i) = True) Then
        Me.chkRevTriangle(i).Visible = False
        Unload chkRevTriangle(i)
    End If
    FRevBox(i) = False
Next i

'init all triangle directions
For i = 1 To MaxX * MaxY
    FLineRev(i) = False
Next i


End Sub

Private Sub SetRevBoxes(YLevel As Integer)
Dim i As Integer
Dim MagicNumber As Integer

For i = 1 To Floor1.SizeX
    MagicNumber = (YLevel - 1) * Floor1.SizeX + i
  
    If (FLineRev(MagicNumber) = True) Then
        Me.chkRevTriangle(i).Value = 1 'checked
    Else
        Me.chkRevTriangle(i).Value = 0 ' unchecked
    End If
Next i
  
End Sub

Private Sub ZoomIn2X_Click()
    FZoom = FZoom / 2
    SetFViewPort
    
'    Me.Base.X1 = Floor1.Vertex(1).X
'    Me.Base.X2 = Floor1.Vertex(Floor1.SizeX + 1).X
    
    
    DrawFloorLines
    DispRevBoxes
    'DrawGrid
    'DrawAll

End Sub


Private Sub ZoomOut2X_Click()
    FZoom = FZoom * 2
    SetFViewPort
    
  
'    Me.Base.X1 = Floor1.Vertex(1).X
'    Me.Base.X2 = Floor1.Vertex(Floor1.SizeX + 1).X
    
    
    DrawFloorLines
    DispRevBoxes
    'DrawGrid
    'DrawAll

End Sub



Private Sub ReadPCX()

    Dim FileHandel
    Dim S As String
    Dim i, j, N As Integer
    Dim FileData As Byte
    Dim Runlen As Byte
    Dim ByteWrite As Long
    
    
    Dim Trash As String
    '
    On Error GoTo UserCanceled
    Ed.ServerDisable
    frmDialogs.FHmapOpen.filename = ""
    frmDialogs.FHmapOpen.Action = 1 'Modal File-Open Box
    Ed.ServerEnable
    '
    'On Error GoTo ErrHand
    If (frmDialogs.FHmapOpen.filename <> "") Then
        FileHandel = FreeFile
        Open frmDialogs.FHmapOpen.filename For Binary Access Read As #FileHandel
       

ReadHeading:
      
        
        'Read past the first 128 bytes
        Seek #FileHandel, 129
        
        If EOF(FileHandel) Then GoTo Done
        
        ByteWrite = 1
        
        MousePointer = 11
        
        While (ByteWrite < 64000 + 1) And Not (EOF(FileHandel)) 'assumes 320x200 pcx
            
           ' FileData = CByte(InputB(1, #FileHandel))
           ' Input #FileHandel, FileData
           Get #FileHandel, , FileData
            If FileData >= 192 Then
                'Encoded
                Runlen = FileData - 192
                Get #FileHandel, , FileData
                For i = 1 To Runlen
                    FPCXData(ByteWrite) = FileData
                    'Debug.Print ByteWrite, FPCXData(ByteWrite)
                    ByteWrite = ByteWrite + 1
                Next i
            Else
                FPCXData(ByteWrite) = FileData 'Just write the byte
                    'Debug.Print ByteWrite, FPCXData(ByteWrite)
                ByteWrite = ByteWrite + 1
            End If
        Wend
        
        MousePointer = 0
        
Done:
        Close #FileHandel
    End If
    
    ApplyHmap
    
    Exit Sub

ExitBad:
    On Error GoTo 0
    Close #FileHandel
    Exit Sub

ErrHand:
    On Error GoTo 0
    MsgBox "Couldn't open PCX hight map Check Format (320x200)", 48
    Exit Sub

UserCanceled:
    Ed.ServerEnable
    Exit Sub

End Sub

Private Sub ApplyHmap()

Dim i As Integer
Dim j As Integer
For j = 1 To Floor1.SizeY + 1
    For i = 1 To Floor1.SizeX + 1
        Floor1.Vertex(((j - 1) * (Floor1.SizeX + 1)) + (i)).Z = -FPCXData(((j - 1) * 320) + i) * 16
    Next i
Next j

    SetFViewPort
    InitVisuals
    DispRevBoxes
    Call SetRevBoxes(FloorLevel)
    
    DrawFloorLines


End Sub

Private Sub UnloadAll()
Dim i As Integer

'Unload all checkboxes
For i = 1 To MaxX
    If (FRevBox(i) = True) Then
        Me.chkRevTriangle(i).Visible = False
        Unload chkRevTriangle(i)
    End If
    FRevBox(i) = False
Next i

' Unload all lines
For i = 1 To FMaxLines
    If FLLoaded(i) Then
        FLine(i).Visible = False
        Unload FLine(i)
        FLLoaded(i) = False
    End If
Next i

' Unload all vertex markers
For i = 1 To FMaxVerticies
    If FVLoaded(i) Then
        VertexBlock(i).Visible = False
        Unload VertexBlock(i)
        FVLoaded(i) = False
    End If
Next i


End Sub
