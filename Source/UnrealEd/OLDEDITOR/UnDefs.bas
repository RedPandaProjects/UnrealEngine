Attribute VB_Name = "undefs"
'-----------------------------------------------
'                    undefs.bas
'
' Definitions and routines needed to interface
' with the Unreal World Editor, plus some
' useful utility functions.  This is
' intended for use in UnrealEd add-on tools.
'
' You don't have to modify this file to create
' an UnrealEd add-on tool.
'-----------------------------------------------
Option Explicit

' Constants.
Global Const MAXSWORD = 32767 ' Max signed word
Global Const MAXVERTICES = 12 ' Max vertices per polygon
Global Const MAXBRUSHPOLYS = 250 ' Max polys per brush
Global Const GWW_HWNDPARENT As Integer = -8 ' For SetWindowWord
Global Const Pi As Double = 3.14159265358979

' Types.
Type FVECTOR ' Floating point vector
    X As Single
    Y As Single
    Z As Single
End Type

Type BRUSHPOLY ' A brush polygon
    Origin As FVECTOR      ' X=Maxword means unspecified
    Normal As FVECTOR      ' X=Maxword means unspecified
    TextureU As FVECTOR    ' X=Maxword means unspecified
    TextureV As FVECTOR    ' X=Maxword means unspecified
    NumVertices As Integer ' May be zero
    Vertex(MAXVERTICES) As FVECTOR
    Flags As Long          ' Default is zero
    Group As String        ' May be blank
    Item As String         ' May be blank
End Type

Type BRUSHTYPE
    Polys(MAXBRUSHPOLYS) As BRUSHPOLY
    NumPolys As Integer
    SourceTool As String
    SourceProperties As String
End Type

' Surface flags.
Global Const PF_INVISIBLE = &H1&
Global Const PF_MASKED = &H2&
Global Const PF_TRANSPARENT = &H4&
Global Const PF_NOTSOLID = &H8&
Global Const PF_ENVIRONMENT = &H10&
Global Const PF_SEMISOLID = &H20&
Global Const PF_HURT = &H40&
Global Const PF_FAKE_BACKDROP = &H80&
Global Const PF_TWO_SIDED = &H100&
Global Const PF_AUTOUPAN = &H200&
Global Const PF_AUTOVPAN = &H400&
Global Const PF_NOSMOOTH = &H800&
Global Const PF_BIGWAVY = &H1000&
Global Const PF_SMALLWAVY = &H2000&
Global Const PF_GHOST = &H4000&
Global Const PF_FARCEILING = &H20000
Global Const PF_BLUR = &H40000
Global Const PF_HIGH_LEDGE = &H80000
Global Const PF_GOURAUD = &H200000
Global Const PF_ZONE_PORTAL = &H4000000
Global Const PF_DIRTY = &H8000000
Global Const PF_SPECIALLIT = &H100000

' Show flags for opening cameras.
Global Const SHOW_FRAME = &H1          ' Show world bounding cube
Global Const SHOW_ACTOR_RADII = &H2    ' Show actor radii
Global Const SHOW_BACKDROP = &H4       ' Show background scene
Global Const SHOW_ACTORS = &H8         ' Show actors
Global Const SHOW_COORDS = &H10        ' Show brush/actor coords
Global Const SHOW_ACTOR_MARKERS = &H20 ' Show actors as markers (icons)
Global Const SHOW_BRUSH = &H40         ' Show the brush
Global Const SHOW_STANDARD_VIEW = &H80 ' Camera is a standard view
Global Const SHOW_MENU = &H100         ' Show menu on camera
Global Const SHOW_AS_CHILD = &H200     ' Show as true child window
Global Const SHOW_MOVINGBRUSHES = &H400 ' Show moving brushes
Global Const SHOW_ACTVIEW = &H1000     ' Showing actor/light view in editor
Global Const SHOW_NOBUTTONS = &H2000   ' No menu/view buttons
Global Const SHOW_REALTIME = &H4000    ' Update window in realtime
Global Const SHOW_NOCAPTURE = &H8000   ' No mouse capture

Global Const SHOW_NORMAL = SHOW_FRAME + SHOW_ACTORS + SHOW_BRUSH
Global Const SHOW_FREE = SHOW_FRAME + SHOW_ACTORS + SHOW_MENU + SHOW_BACKDROP

' Scroll Bar Constants
Public Const SB_HORZ = 0
Public Const SB_VERT = 1
Public Const SB_CTL = 2
Public Const SB_BOTH = 3

' Scroll Bar Commands
Public Const SB_LINEUP = 0
Public Const SB_LINELEFT = 0
Public Const SB_LINEDOWN = 1
Public Const SB_LINERIGHT = 1
Public Const SB_PAGEUP = 2
Public Const SB_PAGELEFT = 2
Public Const SB_PAGEDOWN = 3
Public Const SB_PAGERIGHT = 3
Public Const SB_THUMBPOSITION = 4
Public Const SB_THUMBTRACK = 5
Public Const SB_TOP = 6
Public Const SB_LEFT = 6
Public Const SB_BOTTOM = 7
Public Const SB_RIGHT = 7
Public Const SB_ENDSCROLL = 8

'
' Texture flags.
'
Global Const TF_NoTile = &H1
Global Const TF_BumpMap = &H2
Global Const TF_Blur = &H4

'
' Render types for cameras
'
Global Const REN_HIDE = 0         ' Hide completely
Global Const REN_WIRE = 1         ' Wireframe of EdPolys
Global Const REN_POLYS = 3        ' Flat-shaded BSP
Global Const REN_POLYCUTS = 4     ' Flat-shaded BSP with normals displayed
Global Const REN_DYNLIGHT = 5     ' Dynamic lighting, texture mapping
Global Const REN_PLAINTEX = 6     ' Plain texture mapping
Global Const REN_ORTHXY = 13      ' Orthogonal overhead (XY) view
Global Const REN_ORTHXZ = 14      ' Orthogonal XZ view
Global Const REN_ORTHYZ = 15      ' Orthogonal YZ view
Global Const REN_TEXVIEW = 16     ' Viewing a texture (no actor)
Global Const REN_TEXBROWSER = 17  ' Viewing a texture browser (no actor)
Global Const REN_MESHVIEW = 18    ' Viewing a mesh
Global Const REN_MESHBROWSER = 19 ' Viewing a mesh browser (no actor)

'
' Top flags for Ed.SetOnTop
'
Global Const TOP_NORMAL = 0
Global Const TOP_BROWSER = 1
Global Const TOP_PANEL = 2

'
' Ed callback codes:
'
Global Const EDC_GENERAL = 0         ' Nothing
Global Const EDC_CURTEXCHANGE = 10   ' Change in current texture
Global Const EDC_SELPOLYCHANGE = 20  ' Poly selection set changed
Global Const EDC_SELCHANGE = 21      ' Selection set changed
Global Const EDC_RTCLICKTEXTURE = 23 ' Right clicked on a picture
Global Const EDC_RTCLICKPOLY = 24    ' Right clicked on a polygon
Global Const EDC_RTCLICKACTOR = 25   ' Right clicked on an actor
Global Const EDC_RTCLICKWINDOW = 26  ' Right clicked on camera window
Global Const EDC_RTCLICKWINDOWCANADD = 27 ' Right clicked on camera window
Global Const EDC_MODECHANGE = 40     ' Mode has changed, Param=new mode index
Global Const EDC_BRUSHCHANGE = 41    ' Brush settings changed
Global Const EDC_MAPCHANGE = 42      ' Change in map, bsp
Global Const EDC_ACTORCHANGE = 43    ' Change in actors
Global Const EDC_BROWSE = 1
Global Const EDC_USECURRENT = 2

' SetWindowPos Flags
Public Const SWP_NOSIZE = &H1
Public Const SWP_NOMOVE = &H2
Public Const SWP_NOZORDER = &H4
Public Const SWP_NOREDRAW = &H8
Public Const SWP_NOACTIVATE = &H10
Public Const SWP_FRAMECHANGED = &H20        '  The frame changed: send WM_NCCALCSIZE
Public Const SWP_SHOWWINDOW = &H40
Public Const SWP_HIDEWINDOW = &H80
Public Const SWP_NOCOPYBITS = &H100
Public Const SWP_NOOWNERZORDER = &H200      '  Don't do owner Z ordering

' SetWindowPos hWnds
Public Const HWND_TOP = 0
Public Const HWND_BOTTOM = 1
Public Const HWND_TOPMOST = -1
Public Const HWND_NOTOPMOST = -2

' Win32 API imports:
Declare Function GetWindowLong Lib "user32" Alias "GetWindowLongA" (ByVal hwnd As Long, ByVal nIndex As Long) As Long
Declare Function SetWindowWord Lib "user32" (ByVal hwnd As Long, ByVal index As Long, ByVal wNewWord As Integer) As Integer
Declare Function RedrawWindow Lib "user32" (ByVal hwnd As Long, ByVal lprcUpdate As Long, ByVal hrgnUpdate As Long, ByVal fuRedraw As Long) As Long
Declare Function UpdateWindow Lib "user32" (ByVal hwnd As Long) As Long
Declare Function GdiFlush Lib "gdi32" () As Long
Declare Function InvalidateRect Lib "user32" (ByVal hwnd As Long, ByVal lpRect As Long, ByVal bErase As Long) As Long
Declare Function GetWindowRect Lib "user32" (ByVal hwnd As Long, lpRect As RECT) As Long
Declare Function GetClientRect Lib "user32" (ByVal hwnd As Long, lpRect As RECT) As Long
Declare Function GetDesktopWindow Lib "user32" () As Long
Declare Function SetWindowLong Lib "user32" Alias "SetWindowLongA" (ByVal hwnd As Long, ByVal nIndex As Long, ByVal dwNewLong As Long) As Long
Declare Function SetParent Lib "user32" (ByVal hWndChild As Long, ByVal hWndNewParent As Long) As Long
Declare Function SetWindowPos Lib "user32" (ByVal hwnd As Long, ByVal hWndInsertAfter As Long, ByVal X As Long, ByVal Y As Long, ByVal CX As Long, ByVal CY As Long, ByVal wFlags As Long) As Long

' Edit Control Messages
Public Const EM_GETSEL = &HB0
Public Const EM_SETSEL = &HB1
Public Const EM_GETRECT = &HB2
Public Const EM_SETRECT = &HB3
Public Const EM_SETRECTNP = &HB4
Public Const EM_SCROLL = &HB5
Public Const EM_LINESCROLL = &HB6
Public Const EM_SCROLLCARET = &HB7
Public Const EM_GETMODIFY = &HB8
Public Const EM_SETMODIFY = &HB9
Public Const EM_GETLINECOUNT = &HBA
Public Const EM_LINEINDEX = &HBB
Public Const EM_SETHANDLE = &HBC
Public Const EM_GETHANDLE = &HBD
Public Const EM_GETTHUMB = &HBE
Public Const EM_LINELENGTH = &HC1
Public Const EM_REPLACESEL = &HC2
Public Const EM_GETLINE = &HC4
Public Const EM_LIMITTEXT = &HC5
Public Const EM_CANUNDO = &HC6
Public Const EM_UNDO = &HC7
Public Const EM_FMTLINES = &HC8
Public Const EM_LINEFROMCHAR = &HC9
Public Const EM_SETTABSTOPS = &HCB
Public Const EM_SETPASSWORDCHAR = &HCC
Public Const EM_EMPTYUNDOBUFFER = &HCD
Public Const EM_GETFIRSTVISIBLELINE = &HCE
Public Const EM_SETREADONLY = &HCF
Public Const EM_SETWORDBREAKPROC = &HD0
Public Const EM_GETWORDBREAKPROC = &HD1
Public Const EM_GETPASSWORDCHAR = &HD2

' GetWindow() Constants
Public Const GW_HWNDFIRST = 0
Public Const GW_HWNDLAST = 1
Public Const GW_HWNDNEXT = 2
Public Const GW_HWNDPREV = 3
Public Const GW_OWNER = 4
Public Const GW_CHILD = 5
Public Const GW_MAX = 5
Declare Function GetWindow Lib "user32" (ByVal hwnd As Long, ByVal wCmd As Long) As Long

' Window field offsets for GetWindowLong() and GetWindowWord()
Public Const GWL_WNDPROC = (-4)
Public Const GWL_HINSTANCE = (-6)
Public Const GWL_HWNDPARENT = (-8)
Public Const GWL_STYLE = (-16)
Public Const GWL_EXSTYLE = (-20)
Public Const GWL_USERDATA = (-21)
Public Const GWL_ID = (-12)

' Window Styles
Public Const WS_OVERLAPPED = &H0&
Public Const WS_POPUP = &H80000000
Public Const WS_CHILD = &H40000000
Public Const WS_MINIMIZE = &H20000000
Public Const WS_VISIBLE = &H10000000
Public Const WS_DISABLED = &H8000000
Public Const WS_CLIPSIBLINGS = &H4000000
Public Const WS_CLIPCHILDREN = &H2000000
Public Const WS_MAXIMIZE = &H1000000
Public Const WS_CAPTION = &HC00000                  '  WS_BORDER Or WS_DLGFRAME
Public Const WS_BORDER = &H800000
Public Const WS_DLGFRAME = &H400000
Public Const WS_VSCROLL = &H200000
Public Const WS_HSCROLL = &H100000
Public Const WS_SYSMENU = &H80000
Public Const WS_THICKFRAME = &H40000
Public Const WS_GROUP = &H20000
Public Const WS_TABSTOP = &H10000

' Extended Windows styles
Public Const WS_EX_DLGMODALFRAME = &H1
Public Const WS_EX_NOPARENTNOTIFY = &H4
Public Const WS_EX_TOPMOST = &H8
Public Const WS_EX_ACCEPTFILES = &H10
Public Const WS_EX_TRANSPARENT = &H20
Public Const WS_EX_MDICHILD = &H40
Public Const WS_EX_TOOLWINDOW = &H80
Public Const WS_EX_WINDOWEDGE = &H100
Public Const WS_EX_CLIENTEDGE = &H200
Public Const WS_EX_CONTEXTHELP = &H400
Public Const WS_EX_RIGHT = &H1000
Public Const WS_EX_LEFT = &H0
Public Const WS_EX_RTLREADING = &H2000
Public Const WS_EX_LTRREADING = &H0
Public Const WS_EX_LEFTSCROLLBAR = &H4000
Public Const WS_EX_RIGHTSCROLLBAR = &H0
Public Const WS_EX_CONTROLPARENT = &H10000
Public Const WS_EX_STATICEDGE = &H20000
Public Const WS_EX_APPWINDOW = &H40000
Public Const WS_EX_OVERLAPPEDWINDOW = (WS_EX_WINDOWEDGE Or WS_EX_CLIENTEDGE)
Public Const WS_EX_PALETTEWINDOW = (WS_EX_WINDOWEDGE Or WS_EX_TOOLWINDOW Or WS_EX_TOPMOST)

' Messages
Public Const WM_SETREDRAW = 11

' Registry constants
Public NoReadRegistry As Boolean

Public Const HKEY_CLASSES_ROOT = &H80000000
Public Const HKEY_CURRENT_USER = &H80000001
Public Const HKEY_LOCAL_MACHINE = &H80000002
Public Const HKEY_USERS = &H80000003
Public Const HKEY_PERFORMANCE_DATA = &H80000004
Public Const HKEY_CURRENT_CONFIG = &H80000005
Public Const HKEY_DYN_DATA = &H80000006

Public Const REG_SZ = 1

Type ACL
        AclRevision As Byte
        Sbz1 As Byte
        AclSize As Integer
        AceCount As Integer
        Sbz2 As Integer
End Type

Type SECURITY_ATTRIBUTES
        nLength As Long
        lpSecurityDescriptor As Long
        bInheritHandle As Boolean
End Type

Type SECURITY_DESCRIPTOR
        Revision As Byte
        Sbz1 As Byte
        Control As Long
        Owner As Long
        Group As Long
        Sacl As ACL
        Dacl As ACL
End Type

Declare Function RegCloseKey Lib "advapi32.dll" (ByVal hKey As Long) As Long
Declare Function RegCreateKey Lib "advapi32.dll" Alias "RegCreateKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, phkResult As Long) As Long
Declare Function RegDeleteKey Lib "advapi32.dll" Alias "RegDeleteKeyA" (ByVal hKey As Long, ByVal lpSubKey As String) As Long
Declare Function RegEnumKey Lib "advapi32.dll" Alias "RegEnumKeyA" (ByVal hKey As Long, ByVal dwIndex As Long, ByVal lpName As String, ByVal cbName As Long) As Long
Declare Function RegEnumValue Lib "advapi32.dll" Alias "RegEnumValueA" (ByVal hKey As Long, ByVal dwIndex As Long, ByVal lpValueName As String, lpcbValueName As Long, lpReserved As Long, lpType As Long, lpData As Byte, lpcbData As Long) As Long
Declare Function RegLoadKey Lib "advapi32.dll" Alias "RegLoadKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal lpFile As String) As Long
Declare Function RegOpenKey Lib "advapi32.dll" Alias "RegOpenKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, phkResult As Long) As Long
Declare Function RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal ulOptions As Long, ByVal samDesired As Long, phkResult As Long) As Long
Declare Function RegQueryValue Lib "advapi32.dll" Alias "RegQueryValueA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal lpValue As String, lpcbValue As Long) As Long

Declare Function RegQueryValueEx Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal lpReserved As Long, lpType As Long, lpData As Any, lpcbData As Long) As Long         ' Note that if you declare the lpData parameter as String, you must pass it By Value.

Declare Function RegReplaceKey Lib "advapi32.dll" Alias "RegReplaceKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal lpNewFile As String, ByVal lpOldFile As String) As Long
Declare Function RegRestoreKey Lib "advapi32.dll" Alias "RegRestoreKeyA" (ByVal hKey As Long, ByVal lpFile As String, ByVal dwFlags As Long) As Long
Declare Function RegSaveKey Lib "advapi32.dll" Alias "RegSaveKeyA" (ByVal hKey As Long, ByVal lpFile As String, lpSecurityAttributes As SECURITY_ATTRIBUTES) As Long
Declare Function RegSetKeySecurity Lib "advapi32.dll" (ByVal hKey As Long, ByVal SecurityInformation As Long, pSecurityDescriptor As SECURITY_DESCRIPTOR) As Long
Declare Function RegSetValue Lib "advapi32.dll" Alias "RegSetValueA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal dwType As Long, ByVal lpData As String, ByVal cbData As Long) As Long
Declare Function RegSetValueEx Lib "advapi32.dll" Alias "RegSetValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal Reserved As Long, ByVal dwType As Long, lpData As Any, ByVal cbData As Long) As Long         ' Note that if you declare the lpData parameter as String, you must pass it By Value.
Declare Function RegUnLoadKey Lib "advapi32.dll" Alias "RegUnLoadKeyA" (ByVal hKey As Long, ByVal lpSubKey As String) As Long
Declare Function InitiateSystemShutdown Lib "advapi32.dll" Alias "InitiateSystemShutdownA" (ByVal lpMachineName As String, ByVal lpMessage As String, ByVal dwTimeout As Long, ByVal bForceAppsClosed As Long, ByVal bRebootAfterShutdown As Long) As Long
Declare Function AbortSystemShutdown Lib "advapi32.dll" Alias "AbortSystemShutdownA" (ByVal lpMachineName As String) As Long
Declare Function SendMessage Lib "user32" Alias "SendMessageA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, lParam As Long) As Long
Declare Function PostMessage Lib "user32" Alias "PostMessageA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, lParam As Long) As Long

Type DWORDREC
    Value As Long
End Type
Declare Function SendTabsMessage Lib "user32" Alias "SendMessageA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, lParam As DWORDREC) As Long

Type PARAFORMAT
    cbSize As Long
    wpad As Integer
    dwMask As Long
    wNumbering As Integer
    wReserved As Integer
    dxStartIndent As Integer
    dxRightIndent As Integer
    dxOffset As Integer
    wAlignment As Integer
    cTabCount As Integer
    rgxTabs(12) As Long
End Type

Global Const EM_SETPARAFORMAT = &H400 + 71 ' &h400 + 71

Type RECT
    Left As Long
    Top As Long
    Right As Long
    Bottom As Long
End Type

Dim RegUserSoftwareKey As Long
Dim RegUserUnrealKey As Long
Dim RegUserUnrealAppKey As Long
Dim RegUserHeadingKey As Long
'
Dim RegMachineSoftwareKey As Long
Dim RegMachineUnrealKey As Long
Dim RegMachineUnrealAppKey As Long
Dim RegMachineHeadingKey As Long

'
' Globals
'
Global Brush As BRUSHTYPE
Global hWndTop As Long

'
' SendBrush: Send the current brush to the server
'
' Call with More=0 to send an entire brush, or the
' first part of a multi-part brush.
'
' Call with More=1 to send more polygons to be
' added to the existing brush.  If you're building
' a really big brush, you'll have to do this because
' Visual Basic variables are limited to 64K.
'
Sub SendBrush(More As Boolean)
    '
    Dim Text As String ' Really big string
    Dim i As Integer, j As Integer
    Dim CR As String
    '
    CR = Chr(13) + Chr(10)
    '
    If IsMissing(More) Then
        Text = "BRUSH SET" & CR & CR
    ElseIf (More = 1) Then
        Text = "BRUSH MORE" & CR & CR
    Else
        Text = "BRUSH SET" & CR & CR
    End If
    '
    Text = Text & "BEGIN POLYLIST" & CR
    For i = 1 To Brush.NumPolys
        '
        Text = Text & "BEGIN POLYGON"
        If Brush.Polys(i).Group <> "" Then
            Text = Text & " GROUP=" & Brush.Polys(i).Group
            If Brush.Polys(i).Flags <> 0 Then
                Text = Text & " FLAGS=" & Trim(Str(Brush.Polys(i).Flags)) & CR
            End If
        End If
        '
        If Brush.Polys(i).Item <> "" Then
            Text = Text & " ITEM=" & Brush.Polys(i).Item
        End If
        Text = Text & CR
        '
        If Brush.Polys(i).Origin.X <> MAXSWORD Then
            Text = Text & " ORIGIN " & VectorStr(Brush.Polys(i).Origin) & CR
        End If
        '
        For j = 1 To Brush.Polys(i).NumVertices
            Text = Text & " VERTEX " & VectorStr(Brush.Polys(i).Vertex(j)) & CR
        Next j
        '
        ' Texture name, TextureUm TextureV, GouraudValue,
        ' GouraudGradient, Bitflags
        '
        Text = Text & CR
        Text = Text & "END POLYGON" & CR
        '
    Next i
    Text = Text & "END POLYLIST" & CR
    '
    Ed.ServerExec Text
End Sub

'
' Init one polygon of the brush to defaults
'
Sub InitBrushPoly(N As Integer)
    Brush.Polys(N).Origin.X = MAXSWORD ' Engine will guess
    Brush.Polys(N).Normal.X = MAXSWORD ' Engine will compute
    Brush.Polys(N).TextureU.X = MAXSWORD ' No texture mapping
    Brush.Polys(N).TextureV.X = MAXSWORD
    Brush.Polys(N).NumVertices = 0
    Brush.Polys(N).Flags = 0 ' Default flags
    Brush.Polys(N).Group = "" ' No polygon group name
    Brush.Polys(N).Item = "" ' No polygon item name
End Sub

'
' Bound: Force a floating-point number to be within
' a range.
'
Function Bound(X As Double, Min As Double, Max As Double) As Double
    If (X < Min) Then
        Bound = Min
    ElseIf (X > Max) Then
        Bound = Max
    Else
        Bound = X
    End If
End Function



'
' Add a rectangle in the XY plane to the brush
'
Sub MakeRectXY(N As Integer, Vertex1 As Integer, VertexInc As Integer, X As Single, Y As Single, Z As Single, SizeX As Integer, SizeY As Integer, Group As String, Item As String)
    '
    Dim Vertex As Integer
    '
    ' Init this poly & start:
    '
    InitBrushPoly (N)
    Brush.Polys(N).NumVertices = 4
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = Item
    '
    ' Go through all four vertices:
    '
    Vertex = Vertex1
    Call PutVertex(N, Vertex, X, Y, Z)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X + SizeX, Y, Z)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X + SizeX, Y + SizeY, Z)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X, Y + SizeY, Z)
    '
End Sub

'
' Add a rectangle in the XZ plane to the brush
'
Sub MakeRectXZ(N As Integer, Vertex1 As Integer, VertexInc As Integer, X As Single, Y As Single, Z As Single, SizeX As Integer, SizeZ As Integer, Group As String, Item As String)
    '
    Dim Vertex As Integer
    '
    ' Init this poly & start:
    '
    InitBrushPoly (N)
    Brush.Polys(N).NumVertices = 4
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = Item
    '
    ' Go through all four vertices:
    '
    Vertex = Vertex1
    Call PutVertex(N, Vertex, X, Y, Z)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X + SizeX, Y, Z)

    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X + SizeX, Y, Z + SizeZ)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X, Y, Z + SizeZ)

End Sub

'
' Add a rectangle in the YX plane to the brush
'
Sub MakeRectYZ(N As Integer, Vertex1 As Integer, VertexInc As Integer, X As Single, Y As Single, Z As Single, SizeY As Single, SizeZ As Single, Group As String, Item As String)
    '
    Dim Vertex As Integer
    '
    ' Init this poly & start:
    '
    InitBrushPoly (N)
    Brush.Polys(N).NumVertices = 4
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = Item
    '
    ' Go through all four vertices:
    '
    Vertex = Vertex1
    Call PutVertex(N, Vertex, X, Y, Z)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X, Y + SizeY, Z)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X, Y + SizeY, Z + SizeZ)
    
    Vertex = Vertex + VertexInc
    Call PutVertex(N, Vertex, X, Y, Z + SizeZ)

End Sub

'
' Add a rectangle in the XY plane, symmetric about
' the Z axis, to the brush
'
Sub MakeSymRectXY(N As Integer, Vertex1 As Single, VertexInc As Integer, X As Single, Y As Single, Z As Single, Group As String, Item As String)
    Dim Vertex As Integer
    '
    ' Init this poly & stert:
    '
    InitBrushPoly (N)
    Brush.Polys(N).NumVertices = 4
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = Item
    '
    ' Go through all four vertices:
    '
    Vertex = Vertex1
    Brush.Polys(N).Vertex(Vertex).X = -X
    Brush.Polys(N).Vertex(Vertex).Y = -Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = -Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = -X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
End Sub

'
' Add a rectangle in the XZ plane, symmetric about
' the Y axis, to the brush
'
Sub MakeSymRectXZ(N As Integer, Vertex1 As Integer, VertexInc As Integer, X As Single, Y As Single, Z As Single, Group As String, Item As String)
    Dim Vertex As Integer
    '
    ' Init this poly & stert:
    '
    InitBrushPoly (N)
    Brush.Polys(N).NumVertices = 4
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = Item
    '
    ' Go through all four vertices:
    '
    Vertex = Vertex1
    Brush.Polys(N).Vertex(Vertex).X = -X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = -Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = -X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = -Z
    '
End Sub

'
' Add a rectangle in the YZ plane, symmetric about
' the X axis, to the brush
'
Sub MakeSymRectYZ(N As Integer, Vertex1 As Integer, VertexInc As Integer, X As Single, Y As Single, Z As Single, Group As String, Item As String)
    Dim Vertex As Integer
    '
    ' Init this poly & stert:
    '
    InitBrushPoly (N)
    Brush.Polys(N).NumVertices = 4
    Brush.Polys(N).Group = Group
    Brush.Polys(N).Item = Item
    '
    ' Go through all four vertices:
    '
    Vertex = Vertex1
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = -Y
    Brush.Polys(N).Vertex(Vertex).Z = -Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = -Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = Z
    '
    Vertex = Vertex + VertexInc
    Brush.Polys(N).Vertex(Vertex).X = X
    Brush.Polys(N).Vertex(Vertex).Y = Y
    Brush.Polys(N).Vertex(Vertex).Z = -Z
    '
End Sub

'
' Stick an X,Y,Z point into a brush vertex
'
Sub PutVertex(SNum As Integer, VNum As Integer, X As Single, Y As Single, Z As Single)
    Brush.Polys(SNum).Vertex(VNum).X = X
    Brush.Polys(SNum).Vertex(VNum).Y = Y
    Brush.Polys(SNum).Vertex(VNum).Z = Z
End Sub

'
' Convert a vector (with X,Y,Z components) into a
' string like "X=1.00000 Y=1.00000 Z=1.00000".
'
' When send a vector to the server on the command line,
' it must be in this format.
'
Function VectorStr(F As FVECTOR) As String
    '
    Const m = 10000 ' Decimal place straightener
    '
    VectorStr = "X=" & Str(Int(F.X * m) / m) & " Y=" & Str(Int(F.Y * m) / m) & " Z=" & Str(Int(F.Z * m) / m)
    '
End Function

'
' Grab the first filename from a list returned
' my a multi-select Common Dialog.
'
Function GrabFname(ByRef Fnames As String) As String
    Dim X As Long, Y As Long
    Dim Path As String, Result As String
    
    X = InStr(Fnames, Chr(0))
    
    If X = 0 Then
        
        ' No files.
        Result = Fnames
        Fnames = ""
    Else
        Path = Left(Fnames, X - 1)
        Fnames = Trim(Mid(Fnames, X + 1))
        
        Y = InStr(Fnames, Chr(0))
        If Y = 0 Then
            Result = Path + "\" + Fnames
            Fnames = ""
        Else
            Result = Path + "\" + Trim(Left(Fnames, Y - 1))
            Fnames = Path + Trim(Mid(Fnames, Y))
        End If
    End If
    
    GrabFname = Result
End Function

'
' Return only the name part of filename (not including
' path or extension)
'
Function GetFileNameOnly(ByVal Fname As String) As String
    
    While InStr(Fname, "\") <> 0
        Fname = Mid(Fname, InStr(Fname, "\") + 1)
    Wend
    
    If InStr(Fname, ".") <> 0 Then
        Fname = Left(Fname, InStr(Fname, ".") - 1)
    End If
    
    GetFileNameOnly = Fname

End Function

'
' Open the registry at a particular Unreal section.
' This must be called before GetUserInfo, SetUserInfo,
' GetMachineInfo, and SetMachineInfo.  Be sure to call
' CloseReg as soon as you're done.
'
Sub OpenReg(UnrealAppName As String)
    '
    Call RegOpenKey(HKEY_CURRENT_USER, "Software", RegUserSoftwareKey)
    Call RegCreateKey(RegUserSoftwareKey, "Epic MegaGames", RegUserUnrealKey)
    Call RegCreateKey(RegUserUnrealKey, UnrealAppName, RegUserUnrealAppKey)
    RegUserHeadingKey = 0
    '
    Call RegOpenKey(HKEY_LOCAL_MACHINE, "Software", RegMachineSoftwareKey)
    Call RegCreateKey(RegMachineSoftwareKey, "Epic MegaGames", RegMachineUnrealKey)
    Call RegCreateKey(RegMachineUnrealKey, UnrealAppName, RegMachineUnrealAppKey)
    RegMachineHeadingKey = 0
    '
End Sub

'
' Close the registry opened by OpenReg.
'
Sub CloseReg()
    '
    SetUserHeading ("") ' Flush current heading
    SetMachineHeading ("")
    '
    Call RegCloseKey(RegUserUnrealAppKey)
    Call RegCloseKey(RegUserUnrealKey)
    Call RegCloseKey(RegUserSoftwareKey)
    '
    Call RegCloseKey(RegMachineUnrealAppKey)
    Call RegCloseKey(RegMachineUnrealKey)
    Call RegCloseKey(RegMachineSoftwareKey)
    '
End Sub

'
' The the registry heading for user settings
'
Sub SetUserHeading(Heading As String)
    '
    If RegUserHeadingKey <> 0 Then
        Call RegCloseKey(RegUserHeadingKey)
    End If
    '
    If Heading = "" Then
        RegUserHeadingKey = 0
    Else
        Call RegCreateKey(RegUserUnrealAppKey, Heading, RegUserHeadingKey)
    End If
    '
End Sub

'
' Set user-specific registry information.  Use this
' for saving preferences.
'
Sub SetUserInfo(Name As String, Value As String)
    Dim hKey As Long
    '
    hKey = RegUserHeadingKey
    If hKey = 0 Then hKey = RegUserUnrealAppKey
    '
    If Value <> "" Then
        Call RegSetValueEx(hKey, Name, 0, REG_SZ, ByVal Value, Len(Value))
    Else
        Call RegSetValueEx(hKey, Name, 0, REG_SZ, ByVal " ", 1)
    End If
    '
End Sub

'
' Get user-specific registry information.
'
Function GetUserInfo(Name As String, Default As String) As String
    Dim hKey As Long
    Dim LengthVar As Long
    '
    If NoReadRegistry Then
        GetUserInfo = Default
    Else
        hKey = RegUserHeadingKey
        If hKey = 0 Then hKey = RegUserUnrealAppKey
        '
        LengthVar = 1024
        GetUserInfo = Space(1024)
        Call RegQueryValueEx(hKey, Name, 0, REG_SZ, ByVal GetUserInfo, LengthVar)
        '
        GetUserInfo = Trim(GetUserInfo)
        If (GetUserInfo = "") Then
            GetUserInfo = Default
        Else
            GetUserInfo = Left(GetUserInfo, LengthVar)
            If Len(GetUserInfo) > 0 Then
                If Asc(Right(GetUserInfo, 1)) = 0 Then
                    GetUserInfo = Left(GetUserInfo, Len(GetUserInfo) - 1)
                End If
            End If
        End If
    End If
End Function

'
' The the registry heading for user settings
'
Sub SetMachineHeading(Heading As String)
    '
    If RegMachineHeadingKey <> 0 Then
        Call RegCloseKey(RegMachineHeadingKey)
    End If
    '
    If Heading = "" Then
        RegMachineHeadingKey = 0
    Else
        Call RegCreateKey(RegMachineUnrealAppKey, Heading, RegMachineHeadingKey)
    End If
    '
End Sub

'
' Set machine-specific registry information.  Use this
' for installation info.
'
Sub SetMachineInfo(Name As String, Value As String)
    Dim hKey As Long
    '
    hKey = RegMachineHeadingKey
    If hKey = 0 Then hKey = RegMachineUnrealAppKey
    '
    Call RegSetValueEx(hKey, Name, 0, REG_SZ, ByVal Value, Len(Value))
End Sub

'
' Get machine-specific registry information.
'
Function GetMachineInfo(Name As String, Default As String) As String
    Dim hKey As Long
    Dim LengthVar As Long
    '
    If NoReadRegistry Then
        GetMachineInfo = Default
    Else
        hKey = RegMachineHeadingKey
        If hKey = 0 Then hKey = RegMachineUnrealAppKey
        '
        LengthVar = 1024
        GetMachineInfo = Space(1024)
        Call RegQueryValueEx(hKey, Name, 0, REG_SZ, ByVal GetMachineInfo, LengthVar)
        '
        GetMachineInfo = Trim(GetMachineInfo)
        If (GetMachineInfo = "") Then
            GetMachineInfo = Default
        Else
            GetMachineInfo = Left(GetMachineInfo, LengthVar)
            If Len(GetMachineInfo) > 0 Then
                If Asc(Right(GetMachineInfo, 1)) = 0 Then
                    GetMachineInfo = Left(GetMachineInfo, Len(GetMachineInfo) - 1)
                End If
            End If
        End If
    End If
End Function

Function OnOff(ByVal Value As Integer) As String
    If Value = 0 Then
        OnOff = "off"
    Else
        OnOff = "on"
    End If
End Function

Sub InitBrush(Tool As String)
    Brush.NumPolys = 0
    Brush.SourceTool = Tool
    Brush.SourceProperties = ""
End Sub

Sub EnableRedraw(hwnd As Long)
    Call SendMessage(hwnd, WM_SETREDRAW, 1, 0)
    Call InvalidateRect(hwnd, 0, 0)
    Call UpdateWindow(hwnd)
End Sub

Sub DisableRedraw(hwnd As Long)
    Call SendMessage(hwnd, WM_SETREDRAW, 0, 0)
End Sub

Sub RegisterAllAddOnTools()
    '
    Dim Names As String, Name As String, Descr As String
    '
    Descr = App.EXEName + ".EXE" + "," + ToolModule
    '
    Call OpenReg("UnrealEd")
    Call SetUserHeading("Addons")
    '
    Names = ToolNames
    While (Names <> "")
        If (InStr(Names, ",")) Then
            Name = Left(Names, InStr(Names, ",") - 1)
            Names = Mid(Names, InStr(Names, ",") + 1)
        Else
            Name = Names
            Names = ""
        End If
        Call SetUserInfo(Name, Descr)
    Wend
    '
    Call CloseReg
    '
End Sub

'
' Character grabber.  Skips spaces.  Returns
' "" if empty.
'
Function GrabChar(Str As String) As String
    If Str <> "" Then
Regrab:
        GrabChar = Left(Str, 1)
        Str = Mid(Str, 2)
        If GrabChar = " " Then GoTo Regrab
    Else
        GrabChar = ""
    End If
End Function

'
' Hexidecimal value of one digit 0-9, A-F
'
Function HexVal(c As String) As Integer
    If c >= "0" And c <= "9" Then
        HexVal = Asc(c) - Asc("0")
    ElseIf c >= "a" And c <= "f" Then
        HexVal = 10 + Asc(c) - Asc("a")
    ElseIf c >= "A" And c <= "F" Then
        HexVal = 10 + Asc(c) - Asc("A")
    End If
End Function

'
' Subexpression evaluator
'
Function SubEval(Str As String, Result As Double, Prec As Integer) As Boolean
    '
    Dim c As String
    Dim V As Double
    Dim W As Double
    Dim N As Double
    Dim ReturnCode As Integer
    '
    SubEval = False
    '
    c = GrabChar(Str)
    If (c >= "0" And c <= "9") Or c = "." Then ' Number
        V = 0#
        While c >= "0" And c <= "9"
            V = V * 10 + Val(c)
            c = GrabChar(Str)
        Wend
        If c = "." Then
            N = 0.1
            c = GrabChar(Str)
            While c >= "0" And c <= "9"
                V = V + N * Val(c)
                N = N / 10
                c = GrabChar(Str)
            Wend
        End If
    ElseIf c = "$" Then ' Hex number
        c = GrabChar(Str)
        V = 0#
        While (c >= "0" And c <= "9") Or _
            (c >= "a" And c <= "f") Or _
            (c >= "A" And c <= "F")
            V = V * 16 + HexVal(c)
            c = GrabChar(Str)
        Wend
    ElseIf c = "(" Then ' Opening parenthesis
        If SubEval(Str, V, 0) Then
            If GrabChar(Str) <> ")" Then
                Str = "Mismatched Parenthesis"
                Exit Function
            End If
        Else
            Exit Function ' Already have error
        End If
        c = GrabChar(Str)
    ElseIf c = "-" Then ' Negation
        If Not SubEval(Str, V, 1000) Then
            Exit Function
        End If
        V = -V
        c = GrabChar(Str)
    ElseIf c = "+" Then ' Positive
        If Not SubEval(Str, V, 1000) Then
            Exit Function
        End If
        c = GrabChar(Str)
    ElseIf c = "@" Then ' Square root
        If Not SubEval(Str, V, 1000) Then
            Exit Function
        End If
        If V < 0 Then
            Str = "Can't take square root of negative number"
            Exit Function
        Else
            V = Sqr(V)
        End If
        c = GrabChar(Str)
    Else ' Error
        Str = "No value recognized"
    End If
    '
PrecLoop:
    '
    If c = "" Then
        Result = V
        SubEval = True
    ElseIf c = ")" Then
        Str = ")" & Str
        Result = V
        SubEval = True
    ElseIf c = "+" Then
        If Prec > 1 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 2) Then
            V = V + W
            c = GrabChar(Str)
            GoTo PrecLoop
        End If
    ElseIf c = "-" Then
        If Prec > 1 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 2) Then
            V = V - W
            c = GrabChar(Str)
            GoTo PrecLoop
        End If
    ElseIf c = "/" Then
        If Prec > 2 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 3) Then
            If W = 0 Then
                Str = "Division by zero isn't allowed"
            Else
                V = V / W
                c = GrabChar(Str)
                GoTo PrecLoop
            End If
        End If
    ElseIf c = "%" Then
        If Prec > 2 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 3) Then
            If W = 0 Then
                Str = "Modulo zero isn't allowed"
            Else
                V = V Mod W
                c = GrabChar(Str)
                GoTo PrecLoop
            End If
        End If
    ElseIf c = "*" Then
        If Prec > 3 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 4) Then
            V = V * W
            c = GrabChar(Str)
            GoTo PrecLoop
        End If
    ElseIf c = "^" Then
        If Prec > 4 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 5) Then
            V = V ^ W
            c = GrabChar(Str)
            GoTo PrecLoop
        End If
    ElseIf c = "&" Then
        If Prec > 5 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 6) Then
            V = Int(V) And Int(W)
            c = GrabChar(Str)
            GoTo PrecLoop
        End If
    ElseIf c = "|" Then
        If Prec > 5 Then
            Result = V
            Str = c & Str
            SubEval = True
        ElseIf SubEval(Str, W, 6) Then
            V = Int(V) Or Int(W)
            c = GrabChar(Str)
            GoTo PrecLoop
        End If
    Else
        Str = "Unrecognized Operator"
    End If
    '
End Function

'
' Evaluate a numerical expression.
' Returns 1 if ok, 0 if error.
' Sets Result, or 0.0 if error.
' Call with DoError=1 to show error dialog, 0 for silence.
'
' Operators and precedence: 1:+- 2:/% 3:* 4:^ 5:&|
' Unary: -
' Types: Numbers (0-9.), Hex ($0-$f)
' Grouping: ( )
'
Function Eval(Str As String, Result As Double) As Boolean
    Dim OrigStr As String
    '
    OrigStr = Str
    Eval = SubEval(Str, Result, 0)
    '
    If Not Eval Then
        Call MsgBox(Str, 0, "Can't evaluate " & Chr(34) & OrigStr & Chr(34))
    ElseIf Str <> "" Then
        If GrabChar(Str) = ")" Then
            Call MsgBox("Mismatched Parenthesis", 0, "Can't evaluate " & OrigStr)
        Else
            Call MsgBox(Str, 0, "Can't evaluate " & OrigStr)
        End If
    End If
    '
End Function

Public Function GetString(Str As String, Tag As String, ByRef Value As String) As Boolean
    Dim Temp As String
    If InStr(Str, Tag) > 0 Then
        GetString = True
        Temp = Mid(Str, InStr(Str, Tag) + Len(Tag))
        If Left(Temp, 1) = Chr(34) Then
            Temp = Mid(Temp, 2)
            If InStr(Temp, Chr(34)) Then
                Value = Trim(Left(Temp, InStr(Temp, Chr(34)) - 1))
            Else
                Value = Trim(Temp)
            End If
        ElseIf InStr(Temp, " ") Then
            Value = Trim(Left(Temp, InStr(Temp, " ") - 1))
        Else
            Value = Trim(Temp)
        End If
        If InStr(Value, Chr(13)) Then
            Value = Left(Value, InStr(Value, Chr(13)) - 1)
        End If
    Else
        GetString = False
    End If
End Function

Function Quotes(S As String) As String
    Quotes = Chr(34) & S & Chr(34)
End Function

'
' Grab and remove the first element from a string.
' Elements may be separated by spaces or tabs.
'
Function GrabString(S As String) As String
    Dim i As Integer, j As Integer
    S = Trim(S)
    i = InStr(S, " ")
    j = InStr(S, Chr(9))
    If (i = 0) Or ((j <> 0) And (j < i)) Then i = j
    If i <> 0 Then
        GrabString = Left(S, i - 1)
        S = Mid(S, i + 1)
    Else
        GrabString = S
        S = ""
    End If
End Function

'
' Grab and remove the first element from a string.
' Elements may be separated by spaces or tabs.
'
Function GrabCommaString(S As String) As String
    Dim i As Integer
    S = Trim(S)
    i = InStr(S, ",")
    If i <> 0 Then
        GrabCommaString = Left(S, i - 1)
        S = Mid(S, i + 1)
    Else
        GrabCommaString = S
        S = ""
    End If
End Function

'
' Grab and remove the first element from a string.
' Elements may be separated by spaces or tabs.
'
Function GrabLine(S As String) As String
    Dim i As Integer
    i = InStr(S, Chr(13))
    If i <> 0 Then
        GrabLine = Left(S, i - 1)
        S = Mid(S, i + 2)
    Else
        GrabLine = S
        S = ""
    End If
End Function

'
' Select all stuff in a textbox.
'
Sub SelectAll(Ctl As Object)
    Ctl.SelStart = 0
    Ctl.SelLength = Len(Ctl.Text)
End Sub
