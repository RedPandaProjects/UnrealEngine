VERSION 1.0 CLASS
BEGIN
  MultiUse = 0   'False
END
Attribute VB_Name = "UnrealEdApp"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
'---------------------------------------------
'    Ed OLE automation object
'---------------------------------------------
'
' This contains all public modules and
' variables other programs can access in
' order to create a new instance of the
' UnrealEditor for exclusive access
' via OLE.
'
' This is meant for utilities which want to
' load the editor, do something, and shut it
' down.  Add-on tools, which should coexist with
' other tools and a user, should create
' UnrealEditor objects instead.
'
' This is a creatable, single-use OLE object.
'
Option Explicit

'-----------------
' Public variables
'-----------------

'
' Embedded objects
'
Public Tools As UnrealTools

'
' App names
'
Public EditorAppName As String

'
' Filenames
'
Public MapFname As String
Public BrushFname As String
Public Startup As Integer

'
' State info
'
Public Version As String
Public GameCommandLine As String
Public Licensed As Integer
Public GodMode As Integer
Public hWndEditor As Long

'
' Editor mode info
'
Public ToolMode As String
Public MRUTool As String
Public GridMode As Boolean
Public RotGridMode As Boolean
Public SpeedMode As Integer
Public SnapVertex As Boolean
Public ToolbarPos As Integer  '0=left, 1=rt
Public BrowserPos As Integer  '0=right, 1=left, 2=hide
Public PanelButton As Integer
Public BrowserButton As Integer
'
Public GridX As Integer
Public GridY As Integer
Public GridZ As Integer

Public CameraVertRatio As Single
Public CameraLeftRatio As Single
Public CameraRightRatio As Single
Public CurrentClass As String

'
' Profile directories
'
Public BaseDir     As String ' Base editor directory
Public MapDir      As String ' Maps
Public TextureDir  As String ' Textures
Public ToolDir     As String ' Add-on tools (UTL)
Public ClassDir    As String ' Classes
Public ShapeDir    As String ' 2D Shapes
Public BrushDir    As String ' 3D Solids
Public ModelDir    As String ' 3D Model animations
Public SoundDir    As String ' Sounds
Public MusicDir    As String ' Music

'-----------------
' Private
'-----------------

'
' Extern.
'
Private Declare Sub EdInitServer Lib "Editor.dll" Alias "_EdInitServer@8" (ByVal hWndMain As Long, ByVal hWndCallback As Long)
Private Declare Sub EdExitServer Lib "Editor.dll" Alias "_EdExitServer@0" ()
Private Declare Sub EdExec Lib "Editor.dll" Alias "_EdExec@4" (ByVal Cmd As String)
Private Declare Sub EdSetProp Lib "Editor.dll" Alias "_EdSetProp@12" (ByVal Topic As String, ByVal Item As String, ByVal Value As String)
Private Declare Function EdGetProp Lib "Editor.dll" Alias "_EdGetProp@8" (ByVal Topic As String, ByVal Item As String) As String

'
' Default camera start locations to prevent newly
' opened cameras from being on top of old ones.
'
Private DefCamX As Integer
Private DefCamY As Integer
Private SettingGrid As Integer
Public Gridding As Boolean
Private Const GWW_HWNDPARENT As Integer = -8 ' For SetWindowWord

Private Type WindowRec
    Left As Long
    Top As Long
    Width As Long
    Height As Long
    Name As String
    Loaded As Long
    TheForm As Form
End Type

Private Const MAX_WINDOWS = 200 ' Maximum windows for SetOnTop to track
Dim NumWindowRecs As Integer
Dim WindowRecs(MAX_WINDOWS) As WindowRec

Private Type BrowserTopic
    BrowserForm As Variant
    TopicName As String
    Loaded As Boolean
End Type

Private Const MAX_TOPICS = 20
Private NumBrowserTopics As Integer
Private BrowserTopics(MAX_TOPICS) As BrowserTopic
Public CurBrowserTopic As String
Public InitialBrowserTopic As String
Private SettingTopic As Integer

'
' Initialize all class-specific information:
'
Private Sub Class_Initialize()
    '
    ' Create embedded objects:
    '
    Set Tools = New UnrealTools
    '
    ' Initialize globals:
    '
    Version = "0.48"
    EditorAppName = "UnrealEd"
    hWndEditor = 0
    '
    ToolMode = "CAMERAMOVE"
    MRUTool = "CAMERAMOVE"
    SpeedMode = 1
    GridMode = False
    RotGridMode = False
    SnapVertex = True
    CurrentClass = "Light"
    '
    GridX = 10
    GridY = 10
    GridZ = 10
End Sub

'
' Shut the object down
'
Private Sub Class_Terminate()
    '
End Sub

'
' CreateNewLevel: Close all level-related windows
' and create a new level.
'
Public Sub CreateNewLevel(Title As String, Creator As String)
    '
End Sub


'
' Make sure the editor is visible
'
Public Sub ShowEditor()
    '
End Sub

'
' Hide the editor windows
'
Public Sub HideEditor()
    '
End Sub

'
' Open a camera with specified properties.
'    X,Y   = location on screen, in pixels
'    XR,YR = X,Y size of window, including border and title
'    Menu  = 1 for camera to have a menu, 0 for none.
'
Public Sub OpenCamera(Reopen As Boolean, _
    X As Integer, Y As Integer, _
    XR As Integer, YR As Integer, _
    Flags As Long, _
    RenType As Long, Name As String)
    '
    Dim Cmd As String
    '
    Cmd = "CAMERA OPEN"
    '
    Cmd = Cmd & " HWND=" & Trim(Str(frmCameraHolder.hwnd))
    Cmd = Cmd & " X=" & Trim(Str(X))
    Cmd = Cmd & " Y=" & Trim(Str(Y))
    Cmd = Cmd & " XR=" & Trim(Str(XR))
    Cmd = Cmd & " YR=" & Trim(Str(YR))
    '
    If Not Reopen Then
        Cmd = Cmd & " FLAGS=" & Trim(Str(Flags))
        Cmd = Cmd & " REN=" & Trim(Str(RenType))
    End If
    '
    If Name <> "" Then
        Cmd = Cmd & " NAME=" & Quotes(Name)
    End If
    '
    ServerExec Cmd
    '
End Sub

'
' Open a new camera with the default parameters and
' make it a child window of the editor's main window.
'
Public Sub OpenFreeCamera()
    Dim ShowFlags As Long
    '
    If Screen.Width = 640 * Screen.TwipsPerPixelX Then
        Call Ed.OpenCamera(0, DefCamX + 112 + 13, DefCamY + 76, 320, 200, SHOW_FREE, REN_DYNLIGHT, "")
    ElseIf Screen.Width = 800 * Screen.TwipsPerPixelX Then
        Call Ed.OpenCamera(0, DefCamX + 112 + 13, DefCamY + 76, 400, 300, SHOW_FREE, REN_DYNLIGHT, "")
    ElseIf Screen.Width = 1280 * Screen.TwipsPerPixelX Then
        Call Ed.OpenCamera(0, DefCamX + 112, DefCamY + 76, 640, 480, SHOW_FREE, REN_DYNLIGHT, "")
    Else ' Assume 1024x768
        Call Ed.OpenCamera(0, DefCamX + 112, DefCamY + 76, 560, 400, SHOW_FREE, REN_DYNLIGHT, "")
    End If ' 76
    '
    DefCamX = DefCamX + 32
    If DefCamX > 400 Then
        DefCamX = 0
    End If
    '
    DefCamY = DefCamY + 32
    If DefCamY > 300 Then
        DefCamX = 0
    End If
    '
End Sub

'
' Set the editor's grid size
'
Sub SetGridSize(X As Integer, Y As Integer, Z As Integer)
    If Not Gridding Then
        Gridding = True
        ServerExec "MAP GRID X=" & Trim(Str(X)) & " Y=" & Trim(Str(Y)) & " Z=" & Trim(Str(Z))
        GridX = X
        GridY = Y
        GridZ = Z
        Gridding = False
    End If
End Sub

'
' Turn grid on or off
'
Sub SetGridMode(OnOff As Boolean)
    If GridMode <> OnOff And Not Gridding Then
        Gridding = True
        Call Tools.Handlers("GRID").DoClick("GRID", Ed)
        Gridding = False
    End If
End Sub

'
' Begin a slow task.
'
Public Sub BeginSlowTask(Descr As String)
    frmMain.ProgressBar.Visible = True
    frmMain.ProgressText.Visible = True
    Screen.MousePointer = 11
End Sub

'
' End a slow task.
'
Public Sub EndSlowTask()
    frmMain.ProgressBar.Visible = False
    frmMain.ProgressText.Visible = False
    Screen.MousePointer = 0
End Sub

Public Sub SaveProfile()
    Dim S As String
    Dim X As Integer
    Dim i As Integer
    '
    ' Prototype:
    '
    OpenReg ("UnrealEd")
    '
    SetUserHeading ("NewDirectoriesDirectories")
    Call SetUserInfo("Maps", MapDir)
    Call SetUserInfo("Textures", TextureDir)
    Call SetUserInfo("Tools", ToolDir)
    Call SetUserInfo("Shapes", ShapeDir)
    Call SetUserInfo("Brushes", BrushDir)
    Call SetUserInfo("Models", ModelDir)
    Call SetUserInfo("Sounds", SoundDir)
    Call SetUserInfo("Music", MusicDir)
    '
    Call SetUserHeading("Preferences")
    Call SetUserInfo("ToolbarPos", Str(ToolbarPos))
    Call SetUserInfo("BrowserPos", Str(BrowserPos))
    Call SetUserInfo("PanelButton", Str(PanelButton))
    Call SetUserInfo("BrowserButton", Str(BrowserButton))
    Call SetUserInfo("CameraVertRatio", Str(CameraVertRatio))
    Call SetUserInfo("CameraLeftRatio", Str(CameraLeftRatio))
    Call SetUserInfo("CameraRightRatio", Str(CameraRightRatio))
    Call SetUserInfo("BrowserTopic", InitialBrowserTopic)
    Call SetUserInfo("GodMode", Str(GodMode))
    Call SetUserInfo("GameCommandLine", GameCommandLine)
    '
    S = ""
    Call SetUserHeading("Windows")
    For i = 1 To NumWindowRecs
        If i > 1 Then S = S & ";"
        S = S & _
            WindowRecs(i).Name & "=" & _
            Trim(Str(WindowRecs(i).Left)) & "," & _
            Trim(Str(WindowRecs(i).Top)) & "," & _
            Trim(Str(WindowRecs(i).Width)) & "," & _
            Trim(Str(WindowRecs(i).Height))
    Next i
    Call SetUserInfo("Settings", S)
    '
    CloseReg
    '
End Sub

Public Sub GetProfile()
    Dim S As String
    Dim T As String
    Dim i As Integer
    '
    ' Set up directories and load profile
    '
    If GetString(Command$, "PATH=", S) Then
        If (Mid$(S, 2, 1) = ":") Then
            ChDrive (Left$(S, 2))
        End If
        ChDir S ' Change to specified path
        BaseDir = S + "\"
    Else
        If (Mid$(App.Path, 2, 1) = ":") Then
            ChDrive (Left$(App.Path, 2))
        End If
        ChDir App.Path ' Change to EXE path
        BaseDir = App.Path + "\"
    End If
    App.HelpFile = BaseDir + "Help\" + EditorAppName + ".hlp"
    '
    ' Read registry info
    '
    Call OpenReg("UnrealEd")
    '
    SetUserHeading ("License")
    If GetUserInfo("License", "") <> "" Then
        Licensed = 1
    End If
    '
    SetUserHeading ("NewDirectories")
    MapDir = GetUserInfo("Maps", "..\Maps")
    TextureDir = GetUserInfo("Textures", "..\Textures")
    ToolDir = GetUserInfo("Tools", "..\Tools")
    ShapeDir = GetUserInfo("Shapes", "..\Shapes")
    BrushDir = GetUserInfo("Brushes", "..\Brushes")
    ModelDir = GetUserInfo("Models", "..\Models")
    SoundDir = GetUserInfo("Sounds", "..\Sounds")
    MusicDir = GetUserInfo("Music", "..\Music")
    '
    SetUserHeading ("Preferences")
    GameCommandLine = GetUserInfo("GameCommandLine", "")
    ToolbarPos = Val(GetUserInfo("ToolbarPos", "0"))
    BrowserPos = Val(GetUserInfo("BrowserPos", "0"))
    PanelButton = Val(GetUserInfo("PanelButton", "2"))
    BrowserButton = Val(GetUserInfo("BrowserButton", "0"))
    CameraVertRatio = Val(GetUserInfo("CameraVertRatio", "0.66"))
    CameraLeftRatio = Val(GetUserInfo("CameraLeftRatio", "0.5"))
    CameraRightRatio = Val(GetUserInfo("CameraRightRatio", "0.5"))
    InitialBrowserTopic = GetUserInfo("BrowserTopic", "Textures")
    GodMode = Val(GetUserInfo("GodMode", "0"))
    '
    SetUserHeading ("Windows")
    S = GetUserInfo("Settings", "")
    NumWindowRecs = 0
    While S <> ""
        i = InStr(S, ";")
        If i = 0 Then
            T = S
            S = ""
            If InStr(T, "=") = 0 Then GoTo DoneSettings
        Else
            T = Left(S, i - 1)
            S = Mid(S, i + 1)
        End If
        '
        NumWindowRecs = NumWindowRecs + 1
        i = InStr(T, "=")
        WindowRecs(NumWindowRecs).Name = Left(T, i - 1)
        T = Mid(T, i + 1)
        '
        i = InStr(T, ",")
        WindowRecs(NumWindowRecs).Left = Val(Left(T, i - 1))
        T = Mid(T, i + 1)
        '
        i = InStr(T, ",")
        WindowRecs(NumWindowRecs).Top = Val(Left(T, i - 1))
        T = Mid(T, i + 1)
        '
        i = InStr(T, ",")
        WindowRecs(NumWindowRecs).Width = Val(Left(T, i - 1))
        T = Mid(T, i + 1)
        '
        WindowRecs(NumWindowRecs).Height = Val(T)
        '
        WindowRecs(NumWindowRecs).Loaded = 0
    Wend
DoneSettings:
    '
    Call CloseReg
    '
    ' Set common dialog directories:
    '
    frmDialogs.ExportBrush.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.ImportBrush.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.TexFamLoad.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.TexFamSave.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.TexImport.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.ExportTex.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.BrushOpen.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.ExportMap.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.ImportMap.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.MapOpen.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.MapSaveAs.InitDir = Ed.BaseDir + Ed.MapDir
    frmDialogs.BrushSave.InitDir = Ed.BaseDir + Ed.BrushDir
    frmDialogs.TwoDeeOpen.InitDir = Ed.BaseDir + Ed.ShapeDir
    frmDialogs.TwoDeeSave.InitDir = Ed.BaseDir + Ed.ShapeDir
    frmDialogs.AddFile.InitDir = Ed.BaseDir
    frmDialogs.SoundImportDlg.InitDir = Ed.BaseDir + Ed.SoundDir
    frmDialogs.SoundExportDlg.InitDir = Ed.BaseDir + Ed.SoundDir
    frmDialogs.SoundSaveDlg.InitDir = Ed.BaseDir + Ed.SoundDir
    frmDialogs.SoundLoadDlg.InitDir = Ed.BaseDir + Ed.SoundDir
    frmDialogs.MusicImportDlg.InitDir = Ed.BaseDir + Ed.MusicDir
    frmDialogs.MusicExportDlg.InitDir = Ed.BaseDir + Ed.MusicDir
    frmDialogs.MusicSaveDlg.InitDir = Ed.BaseDir + Ed.MusicDir
    frmDialogs.MusicLoadDlg.InitDir = Ed.BaseDir + Ed.MusicDir
End Sub

Public Sub StatusText(S As String)
    'frmMain.StatusBar1.Panels(1).Text = " " & S
    frmMain.StatusText.Caption = S
End Sub

'
' Force a window to be on top of the editor,
' and make sure it fits on the screen properly.
' F is a form.  Will fail if called from external
' OLE servers.
'
Public Sub SetOnTop(F As Variant, Name As String, Pos As Integer)
    Dim X As Integer, Y As Integer, i As Integer
    Dim R As RECT
    '
    Select Case Pos
    Case TOP_NORMAL:
        '
    Case TOP_PANEL:
        '
    Case TOP_BROWSER:
        Call SetParent(F.hwnd, frmMain.BrowserPanel.hwnd)
        Call GetClientRect(frmMain.BrowserPanel.hwnd, R)
        Call SetWindowPos(F.hwnd, 0, 0, 24, R.Right - R.Left, R.Bottom - R.Top - 24, 0)
        F.Height = frmMain.BrowserPanel.Height - 24 * Screen.TwipsPerPixelY
        Exit Sub
    End Select
    '
    ' See if we've recorded a position for it:
    '
    For i = 1 To NumWindowRecs
        If Name = WindowRecs(i).Name Then
            F.Left = WindowRecs(i).Left
            F.Top = WindowRecs(i).Top
            If F.BorderStyle = 2 Or F.BorderStyle = 5 Then ' Is resizable
                F.Width = WindowRecs(i).Width
                F.Height = WindowRecs(i).Height
            End If
            F.Left = WindowRecs(i).Left
            F.Top = WindowRecs(i).Top
            GoTo MakeItFit
        End If
    Next i
    NumWindowRecs = NumWindowRecs + 1
    i = NumWindowRecs
    '
    WindowRecs(i).Left = F.Left
    WindowRecs(i).Width = F.Width
    WindowRecs(i).Top = F.Top
    WindowRecs(i).Height = F.Height
    '
    ' Make it fit:
    '
MakeItFit:
    If F.Left + F.Width > Screen.Width Then F.Left = Screen.Width - F.Width
    If F.Left < 0 Then F.Left = 0
    '
    F.Tag = Name
    WindowRecs(i).Name = Name
    Set WindowRecs(i).TheForm = F
    WindowRecs(i).Loaded = 1
    '
    Call ToppenFormWindow(F)
    '
End Sub

Public Sub ToppenFormWindow(F As Variant)
    Dim Temp As Long
    Call MakeFormFit(F)
    '
    ' Put it on top:
    '
    If F.MDIChild Then
        F.ZOrder
    Else
        Call SetWindowLong(F.hwnd, -8, frmMain.hwnd)
    End If
    ' This seems to work for everyone, but it causes
    ' problems with window positions on NT 4.0.
    'Temp = GetWindowLong(F.hwnd, -16)
    'Temp = SetWindowLong(F.hwnd, -16, Temp Or WS_OVERLAPPED Or WS_POPUP)
    'Call SetParent(F.hwnd, frmMain.hwnd)
    '
    'Call SetWindowPos(F.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE Or SWP_NOMOVE Or SWP_NOOWNERZORDER)
    F.ZOrder
    ' This is the most recent one!
    'Call SetWindowPos(F.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE Or SWP_NOMOVE Or SWP_NOOWNERZORDER)
    '
    ' This works in Windows 95/NT 4.0 but fails in earlier versions of NT.
    ' However, on NT 4.0, it causes the first click on some
    ' UI elements to go unnoticed.
    'Call SetWindowLong(F.hwnd, -8, frmMain.hwnd)
    '
End Sub

Public Sub MakeFormFit(F As Variant)
    '
    If (F.Width > frmMain.ScaleWidth) And _
        (F.BorderStyle = 2 Or F.BorderStyle = 5) Then
        ' Is resizable and overflowing
        F.Left = 0
        F.Width = frmMain.ScaleWidth
    ElseIf F.WindowState = 0 Then
        ' Not overflowing, not minimized
        If F.Left + F.Width > frmMain.ScaleWidth Then F.Left = frmMain.ScaleWidth - F.Width
        If F.Top < 0 Then F.Top = 0
    End If
    '
    If (F.Height > frmMain.ScaleHeight) And _
        (F.BorderStyle = 2 Or F.BorderStyle = 5) Then
        ' Is resizable and overflowing
        F.Top = 0
        F.Height = frmMain.ScaleHeight
    Else
        ' Not overflowing
        If F.Top + F.Height > frmMain.ScaleHeight Then F.Top = frmMain.ScaleHeight - F.Height
        If F.Top < 0 Then F.Top = 0
    End If
    '
End Sub

Public Sub NoteResize()
   Dim i As Integer
    ' See if we've recorded a position for it.
    For i = 1 To NumWindowRecs
        If WindowRecs(i).Loaded = 1 Then
            Call ToppenFormWindow(WindowRecs(i).TheForm)
        End If
    Next i
 End Sub

Public Function MakeParams() As String
    Dim Params As String
    '
    Params = Params & "CAMERAVERT=" & Trim(Str(CameraVertRatio)) & Chr(13)
    Params = Params & "CAMERALEFT=" & Trim(Str(CameraLeftRatio)) & Chr(13)
    Params = Params & "CAMERARIGHT=" & Trim(Str(CameraRightRatio)) & Chr(13)
    '
    MakeParams = Params
End Function

Public Sub SaveParamsToLevel()
    Dim Params As String
    Params = MakeParams()
    Call Ed.ServerSetProp("Lev", "6", Params)
End Sub

Public Sub SetParams(Params As String)
    Dim S As String
    If GetString(Params, "CAMERAVERT=", S) Then CameraVertRatio = Val(S)
    If GetString(Params, "CAMERALEFT=", S) Then CameraLeftRatio = Val(S)
    If GetString(Params, "CAMERARIGHT=", S) Then CameraRightRatio = Val(S)
End Sub

Public Sub LoadParamsFromLevel()
    Dim Params As String
    Params = Ed.ServerGetProp("Lev", "6")
    SetParams (Params)
End Sub

Public Sub EndOnTop(F As Variant) ' F = form
    Dim i As Integer
    Dim R As RECT
    '
    ' See if we've recorded a position for it:
    '
    If F.WindowState <> 1 Then
        For i = 1 To NumWindowRecs
            If F.Tag = WindowRecs(i).Name Then
                If WindowRecs(i).Loaded = 1 Then
                    '
                    WindowRecs(i).TheForm.Hide
                    Call SetParent(F.hwnd, 0)
                    Call GetWindowRect(F.hwnd, R)
                    '
                    WindowRecs(i).Left = R.Left * Screen.TwipsPerPixelX
                    WindowRecs(i).Top = R.Top * Screen.TwipsPerPixelY
                    WindowRecs(i).Height = (R.Bottom - R.Top) * Screen.TwipsPerPixelY
                    WindowRecs(i).Width = (R.Right - R.Left) * Screen.TwipsPerPixelX
                    '
                    Set WindowRecs(i).TheForm = Nothing
                    '
                    WindowRecs(i).Loaded = 0
                End If
                Exit Sub
            End If
        Next i
    End If
    Debug.Print "EndOnTop: Form " & F.Caption & " not found!"
End Sub

'
' Register a browser topic.  BrowserForm must be a form.
'
' BrowserForm must have:
'    Public BrowswerShow(HolderForm as Form,Top as Integer)
'    Public BrowserHide()
'
Public Sub RegisterBrowserTopic(BrowserForm As Variant, TopicName As String)
    frmMain.BrowserTopicCombo.AddItem TopicName
    '
    Set BrowserTopics(NumBrowserTopics).BrowserForm = BrowserForm
    BrowserTopics(NumBrowserTopics).TopicName = TopicName
    BrowserTopics(NumBrowserTopics).Loaded = False
    '
    NumBrowserTopics = NumBrowserTopics + 1
End Sub

Public Sub SetBrowserTopic(TopicName As String)
    Dim i
    If SettingTopic = 0 Then
        SettingTopic = 1
        If TopicName <> CurBrowserTopic Then
            For i = 0 To NumBrowserTopics - 1
                If BrowserTopics(i).TopicName = CurBrowserTopic Then
                    Call BrowserTopics(i).BrowserForm.BrowserHide
                End If
            Next i
        End If
        For i = 0 To NumBrowserTopics - 1
            If BrowserTopics(i).TopicName = TopicName Then
                Call BrowserTopics(i).BrowserForm.BrowserShow
                BrowserTopics(i).Loaded = True
                frmMain.BrowserTopicCombo.ListIndex = i
                CurBrowserTopic = TopicName
                InitialBrowserTopic = TopicName
                SettingTopic = 0
                Exit Sub
            End If
        Next i
        Call MsgBox("Topic " & TopicName & " doesn't exist", , "SetBrowserTopic problem")
    End If
    SettingTopic = 0
End Sub

Public Sub ReloadBrowser()
    Dim i As Integer
    For i = 0 To NumBrowserTopics - 1
        Unload BrowserTopics(i).BrowserForm
        If BrowserTopics(i).TopicName = CurBrowserTopic Then
            Call BrowserTopics(i).BrowserForm.BrowserShow
        End If
    Next i
End Sub

Public Sub UnloadBrowser()
    Dim i
    For i = 0 To NumBrowserTopics - 1
        Unload BrowserTopics(i).BrowserForm
        Set BrowserTopics(i).BrowserForm = Nothing
    Next i
End Sub

Public Function GetBrowserCurrentItem(TopicName As String) As String
    Dim i As Integer
    '
    GetBrowserCurrentItem = ""
    If CurBrowserTopic = TopicName Then
        For i = 0 To NumBrowserTopics - 1
            If BrowserTopics(i).TopicName = TopicName Then
                GetBrowserCurrentItem = BrowserTopics(i).BrowserForm.GetCurrent
                Exit Function
            End If
        Next i
    End If
End Function

Public Sub ServerEnable()
    'ServerExec "ENABLE"
    ChDir Ed.BaseDir
End Sub

Public Sub ServerDisable()
    'ServerExec "DISABLE"
End Sub

'
' Server functions.
'
Public Sub InitServer(hWndMain As Long, hWndCallback As Long)
    Dim S As String
    Call EdInitServer(hWndMain, hWndCallback)
    ServerExec "HIDELOG"
    ServerExec "APP SET" & _
        " ProgressBar=" & Trim(Str(frmMain.ProgressBar.hwnd)) & _
        " ProgressText=" & Trim(Str(frmMain.ProgressText.hwnd))
    hWndEditor = hWndMain
End Sub

Public Sub ExitServer()
    Call EdExitServer
End Sub

Public Sub ServerExec(S As String)
    Call EdExec(S)
End Sub

Public Sub ServerSetProp(Topic As String, Item As String, Value As String)
    Call EdSetProp(Topic, Item, Value)
End Sub

Public Function ServerGetProp(Topic As String, Item As String) As String
    ServerGetProp = EdGetProp(Topic, Item)
End Function

