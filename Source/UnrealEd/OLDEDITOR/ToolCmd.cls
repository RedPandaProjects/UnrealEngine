VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ToolCmd"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
'---------------------------------------
' Tool class for all command tools (CMD)
'
' These tools are names so that, in
' most cases, their tool name is
' identical to the UnrealEdServer
' command line they execute when
' clicked.
'---------------------------------------

Option Explicit

Public Sub GetStatus(ByVal Tool As String, ByRef UnrealEdApp As Object, ByRef Status As String, ByRef Value As Boolean)
    Status = ""
    Value = False
End Sub

Public Sub DoHelp(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "ACTOR SELECT INSIDE": ToolHelp (130) 'MWP: Help TBD
    Case "ACTOR SELECT INVERT": ToolHelp (130) 'MWP: Help TBD
    Case "ACTOR REPLACE BRUSH": ToolHelp (130) 'MWP: Help TBD
    Case "ACTOR REPLACE": ToolHelp (130) 'MWP: Help TBD
    Case "ACTOR HIDE SELECTED": ToolHelp (130) 'MWP:Help TBD
    Case "ACTOR HIDE UNSELECTED": ToolHelp (130) 'MWP:Help TBD
    Case "ACTOR UNHIDE ALL": ToolHelp (130) 'MWP:Help TBD
    Case "ACTOR CLIP Z": ToolHelp (130) 'MWP:Help TBD
    'Case "ACTOR CLIP XY": ToolHelp (130) 'MWP:XXX
    'Case "ACTOR CLIP XYZ": ToolHelp (130) 'MWP:XXX
    Case "ACTOR SELECT ALL": ToolHelp (130)
    Case "POLY SELECT ALL": ToolHelp (128)
    Case "SELECT ALL": ToolHelp (402)
    Case "SELECT NONE": ToolHelp (402)
    Case "TRANSACTION UNDO": ToolHelp (409)
    Case "TRANSACTION REDO": ToolHelp (409)
    Case "BRUSH ADD": ToolHelp (401)
    Case "BRUSH SUBTRACT": ToolHelp (401)
    Case "BRUSH FROM INTERSECTION": ToolHelp (401)
    Case "BRUSH FROM DEINTERSECTION": ToolHelp (401)
    Case "BRUSH ADD SPECIAL": ToolHelp (316)
    Case "BRUSH ADDMOVER": ToolHelp (401)
    Case "BRUSH FROM HULL": ToolHelp (401)
    Case "BRUSH FROM INTERIOR": ToolHelp (401)
    Case "BRUSH RESTORE": ToolHelp (401)
    Case "HELP": ToolHelp (126) ' Toolbar help
    Case "TOOLS": ToolHelp (408)
    End Select
End Sub

Public Sub DoProperties(ByVal Tool As String, ByRef UnrealEdApp As Object)
    Select Case (Tool)
    Case "POLY SELECT ALL": PopupNext = 1
    Case "SELECT NONE": PopupNext = 1
    Case "SELECT ALL": PopupNext = 1
    Case "ACTOR SELECT ALL": PopupNext = 1
    End Select
End Sub

Public Sub DoClick(ByVal Tool As String, ByRef UnrealEdApp As Object)
    '
    Call Ed.Tools.Highlight(Tool, True)
    '
    Select Case (Tool)
    Case "HELP":
        Call Ed.Tools.Handlers(Ed.MRUTool).DoHelp(Ed.MRUTool, Ed)
    Case "BRUSH ADD SPECIAL":
        frmAddSpecial.Show
    Case "BRUSH ADDMOVER":
        Ed.ServerExec "BRUSH ADDMOVER CLASS=" & Ed.CurrentClass
    Case "ACTOR REPLACE": 'MWP
        Ed.ServerExec "ACTOR REPLACE CLASS=" & Ed.CurrentClass 'MWP
    Case Else:
        Ed.ServerExec (Tool)
    End Select
    '
    Select Case Tool
    Case "ACTOR SELECT INSIDE": Call Ed.StatusText("Select Inside") 'MWP
    Case "ACTOR SELECT INVERT": Call Ed.StatusText("Invert Selection") 'MWP
    Case "ACTOR REPLACE BRUSH": Call Ed.StatusText("Replace Selected Brushes") 'MWP
    Case "ACTOR REPLACE": Call Ed.StatusText("Replace Selected Actors") 'MWP
    Case "ACTOR HIDE SELECTED": Call Ed.StatusText("Hide Selected Actors") 'MWP
    Case "ACTOR HIDE UNSELECTED": Call Ed.StatusText("Drawing Region: Selected Actors") 'MWP
    Case "ACTOR UNHIDE ALL": Call Ed.StatusText("Drawing Region: Off") 'MWP
    Case "ACTOR CLIP Z": Call Ed.StatusText("Drawing Region: Clip Z") 'MWP
    'Case "ACTOR CLIP XY": Call Ed.StatusText("Drawing Region: Clip XY") 'MWP:XXX
    'Case "ACTOR CLIP XYZ": Call Ed.StatusText("Drawing Region: Clip XYZ") 'MWP:XXX
    Case "ACTOR SELECT ALL": Call Ed.StatusText("Select All Actors")
    Case "POLY SELECT ALL": Call Ed.StatusText("Select All Textures")
    Case "SELECT ALL": Call Ed.StatusText("Select All")
    Case "SELECT NONE": Call Ed.StatusText("Select None")
    Case "TRANSACTION UNDO": Call Ed.StatusText("Undo")
    Case "TRANSACTION REDO": Call Ed.StatusText("Redo")
    Case "BRUSH ADD": Call Ed.StatusText("Add Brush To World")
    Case "BRUSH SUBTRACT": Call Ed.StatusText("Subtract Brush From World")
    Case "BRUSH FROM INTERSECTION": Call Ed.StatusText("Intersect Brush With World")
    Case "BRUSH FROM DEINTERSECTION": Call Ed.StatusText("Deintersect Brush With World")
    Case "BRUSH FROM HULL": Call Ed.StatusText("Brush From Hull")
    Case "BRUSH FROM INTERIOR": Call Ed.StatusText("Brush From Interior")
    Case "BRUSH RESTORE": Call Ed.StatusText("Restore Brush")
    Case "BRUSH ADDMOVER": Call Ed.StatusText("Add Movable Brush")
    End Select
    '
    Call Ed.Tools.Highlight(Tool, False)
End Sub

Property Get HiPic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "ACTOR SELECT INSIDE": V = 64 'MWP
    Case "ACTOR SELECT INVERT": V = 65 'MWP
    Case "ACTOR REPLACE BRUSH": V = 72 'MWP
    Case "ACTOR REPLACE": V = 73 'MWP
    Case "ACTOR HIDE SELECTED": V = 74 'MWP
    Case "ACTOR HIDE UNSELECTED": V = 66 'MWP
    Case "ACTOR UNHIDE ALL": V = 67 'MWP
    Case "ACTOR CLIP Z": V = 68 'MWP
    'Case "ACTOR CLIP XY": V = 69 'MWP:XXX
    'Case "ACTOR CLIP XYZ": V = 70 'MWP:XXX
    Case "POLY SELECT ALL": V = 8
    Case "ACTOR SELECT ALL": V = 11 'MWP
    Case "SELECT NONE": V = 10
    Case "SELECT ALL": V = 11
    Case "TRANSACTION UNDO": V = 38
    Case "TRANSACTION REDO": V = 39
    Case "BRUSH ADD": V = 40
    Case "BRUSH SUBTRACT": V = 41
    Case "BRUSH FROM INTERSECTION": V = 42
    Case "BRUSH FROM DEINTERSECTION": V = 43
    Case "BRUSH ADD SPECIAL": V = 46
    Case "BRUSH FROM HULL": V = 46
    Case "BRUSH FROM INTERIOR": V = 45
    Case "BRUSH RESTORE": V = 44
    Case "BRUSH ADDMOVER": V = 62
    Case "HELP": V = 37
    Case "TOOLS": V = 22
    End Select
    '
    Set HiPic = frmMain.HiPics.GraphicCell(V)
End Property

Property Get Pic(ByVal Tool As String) As Object
    Dim V As Integer
    '
    Select Case (Tool)
    Case "ACTOR SELECT INSIDE": V = 64 'MWP
    Case "ACTOR SELECT INVERT": V = 65 'MWP
    Case "ACTOR REPLACE BRUSH": V = 72 'MWP
    Case "ACTOR REPLACE": V = 73 'MWP
    Case "ACTOR HIDE SELECTED": V = 74 'MWP
    Case "ACTOR HIDE UNSELECTED": V = 66 'MWP
    Case "ACTOR UNHIDE ALL": V = 67 'MWP
    Case "ACTOR CLIP Z": V = 68 'MWP
    'Case "ACTOR CLIP XY": V = 69 'MWP:XXX
    'Case "ACTOR CLIP XYZ": V = 70 'MWP:XXX
    Case "POLY SELECT ALL": V = 8
    Case "ACTOR SELECT ALL": V = 11 'MWP
    Case "SELECT NONE": V = 10
    Case "SELECT ALL": V = 11
    Case "TRANSACTION UNDO": V = 38
    Case "TRANSACTION REDO": V = 39
    Case "BRUSH ADD": V = 40
    Case "BRUSH SUBTRACT": V = 41
    Case "BRUSH FROM INTERSECTION": V = 42
    Case "BRUSH FROM DEINTERSECTION": V = 43
    Case "BRUSH ADD SPECIAL": V = 46
    Case "BRUSH FROM HULL": V = 46
    Case "BRUSH FROM INTERIOR": V = 45
    Case "BRUSH RESTORE": V = 44
    Case "BRUSH ADDMOVER": V = 62
    Case "HELP": V = 37
    Case "TOOLS": V = 22
    End Select
    '
    Set Pic = frmMain.Pics.GraphicCell(V)
End Property

Public Sub InitTools(UnrealEdApp As UnrealEdApp)
    Call UnrealEdApp.Tools.Register(Me, "ACTOR SELECT INSIDE", "Select actors inside brush", "") 'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR SELECT INVERT", "Invert actor selection state", "") 'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR REPLACE BRUSH", "Replace selected brushes", "") 'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR REPLACE", "Replace selected non-brush actors", "")  'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR HIDE SELECTED", "Hide Selected Actors", "") 'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR HIDE UNSELECTED", "Drawing Region: Selected Actors", "") 'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR UNHIDE ALL", "Drawing Region: Off", "") 'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR CLIP Z", "Drawing Region: Brush Z (Depth)", "") 'MWP
    'Call UnrealEdApp.Tools.Register(Me, "ACTOR CLIP XY", "Drawing Region: Brush XY", "") 'MWP
    'Call UnrealEdApp.Tools.Register(Me, "ACTOR CLIP XYZ", "Drawing Region: Brush XYZ", "") 'MWP
    Call UnrealEdApp.Tools.Register(Me, "ACTOR SELECT ALL", "Select all actors", "")
    Call UnrealEdApp.Tools.Register(Me, "POLY SELECT ALL", "Select all polys", "")
    Call UnrealEdApp.Tools.Register(Me, "SELECT ALL", "Select all", "")
    Call UnrealEdApp.Tools.Register(Me, "SELECT NONE", "Select none", "")
    Call UnrealEdApp.Tools.Register(Me, "TRANSACTION UNDO", "Undo last operation", "")
    Call UnrealEdApp.Tools.Register(Me, "TRANSACTION REDO", "Redo operation", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH ADD", "Add brush to world", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH SUBTRACT", "Subtract brush from world", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM INTERSECTION", "Brush intersection", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM DEINTERSECTION", "Brush deintersection", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH ADD SPECIAL", "Add Special Brush", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM HULL", "Brush hull", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH FROM INTERIOR", "Brush interior", "")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH RESTORE", "Restore brush", "Resize/move brush...")
    Call UnrealEdApp.Tools.Register(Me, "BRUSH ADDMOVER", "Add movable brush", "")
    Call UnrealEdApp.Tools.Register(Me, "HELP", "Get Help", "")
    Call UnrealEdApp.Tools.Register(Me, "TOOLS", "Add-On Tools", "")
End Sub
