VERSION 4.00
Begin VB.Form frmTexBrowser 
   BorderStyle     =   0  'None
   Caption         =   "Texture Browser"
   ClientHeight    =   4305
   ClientLeft      =   3480
   ClientTop       =   6510
   ClientWidth     =   2445
   Height          =   4770
   Left            =   3420
   LinkTopic       =   "Form1"
   ScaleHeight     =   4305
   ScaleWidth      =   2445
   ShowInTaskbar   =   0   'False
   Top             =   6105
   Width           =   2565
   Begin VB.VScrollBar BrowserScroller 
      Height          =   1650
      Left            =   0
      Max             =   16384
      TabIndex        =   8
      Top             =   720
      Width           =   225
   End
   Begin VB.PictureBox BrowserHolder 
      BackColor       =   &H00C0C0C0&
      Height          =   2415
      Left            =   60
      ScaleHeight     =   157
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   154
      TabIndex        =   7
      Top             =   720
      Width           =   2370
   End
   Begin VB.ComboBox TexFamily 
      BackColor       =   &H00C0C0C0&
      Height          =   315
      Left            =   60
      Sorted          =   -1  'True
      Style           =   2  'Dropdown List
      TabIndex        =   0
      Tag             =   "Current texture family"
      Top             =   0
      Width           =   2310
   End
   Begin Threed.SSPanel TexPanel 
      Height          =   735
      Left            =   0
      TabIndex        =   9
      Top             =   3300
      Width           =   2400
      _Version        =   65536
      _ExtentX        =   4233
      _ExtentY        =   1296
      _StockProps     =   15
      ForeColor       =   -2147483640
      BackColor       =   13154717
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         name            =   "Arial"
         charset         =   0
         weight          =   700
         size            =   9
         underline       =   0   'False
         italic          =   -1  'True
         strikethrough   =   0   'False
      EndProperty
      Alignment       =   2
      Begin VB.CommandButton BroDelete 
         Caption         =   "&Delete"
         Height          =   255
         Left            =   1560
         TabIndex        =   10
         Tag             =   "Delete this texture"
         Top             =   480
         Width           =   855
      End
      Begin VB.CommandButton BroExport 
         Caption         =   "E&xport"
         Height          =   255
         Left            =   840
         TabIndex        =   11
         Tag             =   "Export textures"
         Top             =   480
         Width           =   735
      End
      Begin VB.CommandButton BroImport 
         Caption         =   "&Import"
         Height          =   255
         Left            =   0
         TabIndex        =   12
         Tag             =   "Import textures"
         Top             =   480
         Width           =   855
      End
      Begin VB.CommandButton BroSaveAll 
         Caption         =   "Save &All"
         Height          =   255
         Left            =   1560
         TabIndex        =   13
         Tag             =   "Save all texture families"
         Top             =   240
         Width           =   855
      End
      Begin VB.CommandButton BroSave 
         Caption         =   "&Save"
         Height          =   255
         Left            =   840
         TabIndex        =   14
         Tag             =   "Save texture families"
         Top             =   240
         Width           =   735
      End
      Begin VB.CommandButton BroLoad 
         Caption         =   "&Load"
         Height          =   255
         Left            =   0
         TabIndex        =   15
         Tag             =   "Load texture families"
         Top             =   240
         Width           =   855
      End
      Begin VB.CommandButton Command1 
         Caption         =   "&Help"
         Height          =   255
         Left            =   1560
         TabIndex        =   16
         Tag             =   "Help on texturing"
         Top             =   0
         Width           =   855
      End
      Begin VB.CommandButton BroEdit 
         Caption         =   "&Edit"
         Height          =   255
         Left            =   840
         TabIndex        =   17
         Tag             =   "Edit this texture's properties"
         Top             =   0
         Width           =   735
      End
      Begin VB.CommandButton BroApply 
         Caption         =   "&Apply"
         Height          =   255
         Left            =   0
         TabIndex        =   18
         Tag             =   "Apply this texture to selected polys"
         Top             =   0
         Width           =   855
      End
   End
   Begin Threed.SSRibbon Tex1 
      Height          =   315
      Left            =   825
      TabIndex        =   6
      Tag             =   "View large textures"
      Top             =   360
      Width           =   315
      _Version        =   65536
      _ExtentX        =   556
      _ExtentY        =   556
      _StockProps     =   65
      BackColor       =   12632256
      GroupAllowAllUp =   0   'False
      RoundedCorners  =   0   'False
      BevelWidth      =   1
      PictureUp       =   "TexBr.frx":0000
      PictureDn       =   "TexBr.frx":0112
   End
   Begin Threed.SSRibbon Tex2 
      Height          =   315
      Left            =   1140
      TabIndex        =   5
      Tag             =   "View medium textures"
      Top             =   360
      Width           =   315
      _Version        =   65536
      _ExtentX        =   556
      _ExtentY        =   556
      _StockProps     =   65
      BackColor       =   12632256
      Value           =   -1  'True
      GroupAllowAllUp =   0   'False
      RoundedCorners  =   0   'False
      BevelWidth      =   1
      PictureUp       =   "TexBr.frx":012E
      PictureDn       =   "TexBr.frx":0240
   End
   Begin Threed.SSRibbon Tex4 
      Height          =   315
      Left            =   1455
      TabIndex        =   4
      Tag             =   "View small textures"
      Top             =   360
      Width           =   315
      _Version        =   65536
      _ExtentX        =   556
      _ExtentY        =   556
      _StockProps     =   65
      BackColor       =   12632256
      GroupAllowAllUp =   0   'False
      RoundedCorners  =   0   'False
      BevelWidth      =   1
      PictureUp       =   "TexBr.frx":025C
      PictureDn       =   "TexBr.frx":036E
   End
   Begin Threed.SSRibbon TexSphere 
      Height          =   315
      Left            =   2040
      TabIndex        =   3
      Tag             =   "Show textures as spheres"
      Top             =   360
      Width           =   315
      _Version        =   65536
      _ExtentX        =   556
      _ExtentY        =   556
      _StockProps     =   65
      BackColor       =   12632256
      GroupNumber     =   2
      RoundedCorners  =   0   'False
      BevelWidth      =   1
      PictureUp       =   "TexBr.frx":038A
      PictureDn       =   "TexBr.frx":049C
   End
   Begin Threed.SSCommand FamPlus 
      Height          =   315
      Left            =   315
      TabIndex        =   2
      Tag             =   "Next texture family"
      Top             =   360
      Width           =   270
      _Version        =   65536
      _ExtentX        =   476
      _ExtentY        =   556
      _StockProps     =   78
      Caption         =   ">"
      BevelWidth      =   1
      RoundedCorners  =   0   'False
   End
   Begin Threed.SSCommand FamMinus 
      Height          =   315
      Left            =   60
      TabIndex        =   1
      Tag             =   "Previous texture family"
      Top             =   360
      Width           =   270
      _Version        =   65536
      _ExtentX        =   476
      _ExtentY        =   556
      _StockProps     =   78
      Caption         =   "<"
      BevelWidth      =   1
      RoundedCorners  =   0   'False
   End
End
Attribute VB_Name = "frmTexBrowser"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
'
' Texture Browser
'
Option Explicit

Dim OrigFamily As String
Dim OrigTexture As String

'
' Public
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "TextureBrowser", TOP_BROWSER)
    TexPanel.Top = Height - TexPanel.Height
    BrowserHolder.Height = Height - BrowserHolder.Top - TexPanel.Height
    BrowserScroller.Height = BrowserHolder.Height - 2 * Screen.TwipsPerPixelY
    RefreshTextureFamily ("")
End Sub

Public Sub BrowserShow()
    Show
    DoBrowserCam
End Sub

Public Sub BrowserRefresh()
    DoBrowserCam
End Sub

Public Sub BrowserHide()
    'Hide
    'Unload Me
End Sub

Public Function GetCurrent() As String
    GetCurrent = Ed.Server.GetProp("ED", "CURTEX")
End Function

'
' Private
'

Private Sub BroLoad_Click()
    Dim Fnames As String
    '
    ' Dialog for "Load Texture Family":
    '
    Ed.Server.Disable
    frmDialogs.TexFamLoad.filename = ""
    frmDialogs.TexFamLoad.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.TexFamLoad.Action = 1 'Modal File-Open Box
    Ed.Server.Enable
    '
    If (frmDialogs.TexFamLoad.filename <> "") Then
        Screen.MousePointer = 11
        Fnames = Trim(frmDialogs.TexFamLoad.filename)
        While (Fnames <> "")
            Ed.Server.Exec "TEXTURE LOADFAMILY FILE=" & Quotes(GrabFname(Fnames))
        Wend
        RefreshTextureFamily ("")
        Screen.MousePointer = 0
    End If
    '
End Sub

Private Sub BroSaveAll_Click()
    Dim Fname As String
    '
    Ed.Server.Disable
    frmDialogs.TexFamSave.DialogTitle = "Save all texture families"
    frmDialogs.TexFamSave.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.TexFamSave.DefaultExt = "UTX"
    frmDialogs.TexFamSave.Flags = 2 'Prompt if overwrite
    frmDialogs.TexFamSave.Action = 2 'Modal Save-As Box
    Ed.Server.Enable
    '
    On Error GoTo 0
    '
    Fname = frmDialogs.TexFamSave.filename
    If Fname <> "" Then
        Ed.Server.Exec "TEXTURE SAVEFAMILY ALL FILE=" & Quotes(Fname)
    End If
End Sub

Private Sub BroSave_Click()
    '
    Dim Fname As String
    Dim FamilyName As String
    '
    ' Find family name based on cursor position
    '
    FamilyName = CurTexFamily()
    '
    If FamilyName = "" Then
        SaveTexFamError
    Else
        If InStr(FamilyName, " ") > 0 Then
            Fname = Left(FamilyName, InStr(FamilyName, " ") - 1)
        Else
            Fname = FamilyName
        End If
        '
        frmDialogs.TexFamSave.filename = Trim(Fname) & ".utx"
        '
        On Error GoTo BadFilename
        '
TryAgain:
        '
        Ed.Server.Disable
        frmDialogs.TexFamSave.DialogTitle = "Save " & FamilyName & " texture family"
        frmDialogs.TexFamSave.InitDir = Ed.BaseDir + Ed.TextureDir
        frmDialogs.TexFamSave.DefaultExt = "utx"
        frmDialogs.TexFamSave.Flags = 2 'Prompt if overwrite
        frmDialogs.TexFamSave.Action = 2 'Modal Save-As Box
        Ed.Server.Enable
        '
        On Error GoTo 0
        '
        Fname = frmDialogs.TexFamSave.filename
        '
        If (Fname <> "") Then
            Ed.Server.Exec "TEXTURE SAVEFAMILY FAMILY=" & Quotes(FamilyName) & " FILE=" & Quotes(Fname)
        End If
    End If
    Exit Sub
    '
    ' Bad filename handler:
    '
BadFilename:
    '
    Fname = ""
    frmDialogs.TexFamSave.filename = ""
    On Error GoTo 0
    GoTo TryAgain
    '
End Sub

Private Sub BroImport_Click()
    Dim Family As String
    '
    ' Dialog for "Import Texture":
    '
    Ed.Server.Disable
    frmDialogs.TexImport.filename = ""
    frmDialogs.TexImport.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.TexImport.Action = 1 'Modal File-Open Box
    Ed.Server.Enable
    '
    GString = Trim(frmDialogs.TexImport.filename)
    '
    frmTexImport.TexFamily = CurTexFamily()
    If frmTexImport.TexFamily = "" Then frmTexImport.TexFamily = "Untitled"
    '
    Ed.Server.Disable
    frmTexImport.Show 1 ' Modal import accept/cancel box
    If GlobalAbortedModal = 0 Then RefreshTextureFamily (frmTexImport.TexFamily)
    Ed.Server.Enable
End Sub

Public Sub BroDelete_Click()
    If MsgBox("Delete texture " & frmMain.TextureCombo.List(0) & "?", 4 + 32, "Delete a texture") = 6 Then
        Ed.Server.Exec "TEXTURE KILL NAME=" & Quotes(frmMain.TextureCombo.List(0))
        RefreshTextureFamily ("")
    End If
End Sub

Public Sub BroExport_Click()
    Ed.Server.Disable
    frmDialogs.ExportTex.InitDir = Ed.BaseDir + Ed.TextureDir
    frmDialogs.ExportTex.DialogTitle = "Export texture " & frmMain.TextureCombo.List(0)
    frmDialogs.ExportTex.DefaultExt = "PCX"
    frmDialogs.ExportTex.Flags = 2 'Prompt if overwrite
    frmDialogs.ExportTex.Action = 2 'Modal Save-As Box
    Ed.Server.Enable
    '
    If frmDialogs.ExportTex.filename <> "" Then
        Ed.Server.Exec "RES EXPORT TYPE=TEXTURE NAME=" & Quotes(frmMain.TextureCombo.List(0)) & "FILE=" & Quotes(frmDialogs.ExportTex.filename)
    End If
End Sub

Private Sub RefreshTextureFamily(FamName As String)
    Dim Result As String
    Dim Found As Integer
    Dim i As Integer
    '
    If FamName <> "" Then
        OrigFamily = FamName
    ElseIf TexFamily.ListIndex >= 0 Then
        OrigFamily = TexFamily.List(TexFamily.ListIndex)
    Else
        OrigFamily = ""
    End If
    '
    TexFamily.Clear
    Ed.Server.Exec "TEXTURE QUERY" ' Query all textures
    '
    Do
        Result = Ed.Server.GetProp("Tex", "QueryFam")
        If (Result <> "") Then
            TexFamily.AddItem Result
        End If
    Loop Until Result = ""
    '
    If TexFamily.ListCount = 0 Then
        TexFamily.AddItem NoneString
    Else
        '
        ' Set family
        '
        For i = 0 To TexFamily.ListCount - 1
            If TexFamily.List(i) = OrigFamily Then
                TexFamily.ListIndex = i
                Found = 1
            End If
        Next i
        If Found = 0 Then TexFamily.ListIndex = 1
        DoBrowserCam
    End If
End Sub

Private Function CurTexFamily() As String
    '
    Dim Temp As String
    '
    If TexFamily.ListIndex < 0 Then
        CurTexFamily = ""
    ElseIf TexFamily.List(TexFamily.ListIndex) = NoneString Then
        CurTexFamily = ""
    Else
        CurTexFamily = Trim(TexFamily.List(TexFamily.ListIndex))
    End If
    '
End Function

Public Sub BroApply_Click()
    Ed.Server.Exec "POLY SET TEXTURE=" & frmMain.TextureCombo.List(0)
    Ed.Server.Exec "POLY DEFAULT TEXTURE=" & frmMain.TextureCombo.List(0)
End Sub

Public Sub BrowserEdit_Click()
    '
End Sub

Private Sub SaveTexFamError()
    MsgBox "You must position the cursor on a texture family name first", 16
End Sub

Private Sub TexFamily_Click()
    BrowserScroller.Value = 0
    DoBrowserCam
End Sub

Private Sub BrowserScroller_Scroll()
    DoBrowserCam
End Sub

Private Sub BrowserScroller_Change()
    DoBrowserCam
    TexFamily.SetFocus
End Sub

Public Sub BroEdit_Click()
    frmTexProp.SetTexture frmMain.TextureCombo.Text
    frmTexProp.Show
End Sub

Sub DoBrowserCam()
    Dim Size As Integer
    Dim Temp As Long
    '
    If Tex1.Value Then
        Size = 128
    ElseIf Tex2.Value Then
        Size = 64
    ElseIf Tex4.Value Then
        Size = 32
    Else
        Size = 0
    End If
    '
    If (Size = 0) Then
        BrowserScroller.Visible = False
        Ed.Server.Exec "CAMERA CLOSE NAME=BrowserCam"
    Else
        Ed.Server.Exec "CAMERA OPEN X=10 Y=0" & _
            " XR=" & Trim(Str(BrowserHolder.ScaleWidth - 10)) & _
            " YR=" & Trim(Str(BrowserHolder.ScaleHeight)) & _
            " FLAGS=" & Trim(Str(SHOW_NORMAL + SHOW_AS_CHILD + SHOW_NOBUTTONS + SHOW_NOCAPTURE)) & _
            " HWND=" & Trim(Str(BrowserHolder.hwnd)) & _
            " MISC1=" & Trim(Str(Size)) & _
            " MISC2=" & Trim(Str(BrowserScroller.Value)) & _
            " REN=" & Trim(Str(REN_TEXBROWSER)) & _
            " NAME=BrowserCam FAMILY=" & Quotes(TexFamily.List(TexFamily.ListIndex))
       BrowserScroller.Max = Val(Ed.Server.GetProp("ED", "LASTSCROLL"))
       BrowserScroller.LargeChange = 512
       BrowserScroller.SmallChange = 64
       BrowserScroller.Visible = True
   End If
End Sub

Private Sub FamMinus_Click()
    If TexFamily.ListIndex = 0 Then
        TexFamily.ListIndex = TexFamily.ListCount - 1
    Else
        TexFamily.ListIndex = TexFamily.ListIndex - 1
    End If
End Sub

Private Sub FamPlus_Click()
    If TexFamily.ListIndex = TexFamily.ListCount - 1 Then
        TexFamily.ListIndex = 0
    Else
        TexFamily.ListIndex = TexFamily.ListIndex + 1
    End If
End Sub

Private Sub Tex1_Click(Value As Integer)
    DoBrowserCam
End Sub

Private Sub Tex2_Click(Value As Integer)
    DoBrowserCam
End Sub

Private Sub Tex4_Click(Value As Integer)
    DoBrowserCam
End Sub

Private Sub TexSphere_Click(Value As Integer)
    DoBrowserCam
End Sub

Private Sub TexText_Click(Value As Integer)
    DoBrowserCam
End Sub

Public Sub TextureList_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 2 Then
        frmPopups.TBProperties.Caption = "&Edit " & frmMain.TextureCombo.List(0) & " properties..."
        PopupMenu frmPopups.TexBrowser
    End If
End Sub

