VERSION 5.00
Begin VB.Form frmCameraHolder 
   BackColor       =   &H00000000&
   BorderStyle     =   3  'Fixed Dialog
   ClientHeight    =   6990
   ClientLeft      =   2640
   ClientTop       =   3150
   ClientWidth     =   6690
   ControlBox      =   0   'False
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   Moveable        =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6990
   ScaleWidth      =   6690
   ShowInTaskbar   =   0   'False
   Begin VB.OLE LBox 
      BackColor       =   &H00808080&
      Height          =   150
      Left            =   4485
      MousePointer    =   15  'Size All
      TabIndex        =   4
      Top             =   2880
      Visible         =   0   'False
      Width           =   150
   End
   Begin VB.OLE RBox 
      BackColor       =   &H00808080&
      Height          =   165
      Left            =   4560
      MousePointer    =   15  'Size All
      TabIndex        =   3
      Top             =   3090
      Visible         =   0   'False
      Width           =   150
   End
   Begin VB.OLE VBar 
      BackColor       =   &H00808080&
      Height          =   1170
      Left            =   720
      MousePointer    =   9  'Size W E
      TabIndex        =   2
      Top             =   3240
      Visible         =   0   'False
      Width           =   135
   End
   Begin VB.OLE LBar 
      BackColor       =   &H00808080&
      Height          =   150
      Left            =   2220
      MousePointer    =   7  'Size N S
      TabIndex        =   1
      Top             =   4020
      Visible         =   0   'False
      Width           =   1815
   End
   Begin VB.OLE RBar 
      BackColor       =   &H00808080&
      Height          =   150
      Left            =   1800
      MousePointer    =   7  'Size N S
      TabIndex        =   0
      Top             =   3720
      Visible         =   0   'False
      Width           =   1815
   End
End
Attribute VB_Name = "frmCameraHolder"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim VBarHold As Integer, LBarHold As Integer, RBarHold
Dim LBoxHold As Integer, RBoxHold As Integer
Dim VDelta As Integer, HDelta As Integer

Public CX, CY, CXL, CYL As Integer ' Client window dimensions

Private Sub Form_GotFocus()
    ZOrder 1
End Sub

Private Sub Form_Load()
    Dim H As Long
    H = GetWindow(frmMain.hwnd, GW_CHILD)
    Call SetWindowLong(hwnd, -8, H)
End Sub

'
' Vertical bar
'

Private Sub VBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    ZOrder
    If Button = 1 Then
        Ed.ServerExec "CAMERA HIDESTANDARD"
        VBarHold = 1
        VDelta = X
    End If
End Sub

Private Sub VBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewX As Integer
    If VBarHold And Button = 1 Then
        NewX = VBar.Left + X - VDelta
        If NewX < 0 Then
            VBar.Left = 0
        ElseIf NewX > CXL - VBar.Width Then
            VBar.Left = CXL - VBar.Width
        Else
            VBar.Left = NewX
        End If
    LBox.Left = VBar.Left
    RBox.Left = VBar.Left
    LBar.Width = VBar.Left
    RBar.Left = VBar.Left + VBar.Width
    RBar.Width = CXL - (VBar.Left + VBar.Width)
    Ed.CameraVertRatio = CSng(VBar.Left) / (CXL - VBar.Width)
    End If
End Sub

Private Sub VBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If VBarHold Then frmMain.ResizeAll (True)
    VBarHold = 0
End Sub

'
' LBar
'

Private Sub LBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.ServerExec "CAMERA HIDESTANDARD"
        LBarHold = 1
        HDelta = Y
    End If
End Sub

Private Sub LBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewY As Integer
    If LBarHold And Button = 1 Then
        NewY = LBar.Top + Y - HDelta
        If NewY < 0 Then
            LBar.Top = 0
        ElseIf NewY > CYL - LBar.Height Then
            LBar.Top = CYL - LBar.Height
        Else
            LBar.Top = NewY
        End If
        LBox.Top = LBar.Top
        Ed.CameraLeftRatio = CSng(LBar.Top) / CSng(CY + CYL - LBar.Height)
    End If
End Sub

Private Sub LBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBarHold Then frmMain.ResizeAll (True)
    LBarHold = 0
End Sub

'
' RBar
'

Private Sub RBar_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.ServerExec "CAMERA HIDESTANDARD"
        RBarHold = 1
        HDelta = Y
    End If
End Sub

Private Sub RBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim NewY As Integer
    If RBarHold And Button = 1 Then
        NewY = RBar.Top + Y - HDelta
        If NewY < 0 Then
            RBar.Top = 0
        ElseIf NewY > CYL - RBar.Height Then
            RBar.Top = CYL - RBar.Height
        Else
            RBar.Top = NewY
        End If
        RBox.Top = RBar.Top
        Ed.CameraRightRatio = CSng(RBar.Top) / CSng(CYL - RBar.Height)
    End If
End Sub

Private Sub RBar_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBarHold Then frmMain.ResizeAll (True)
    RBarHold = 0
End Sub

'
' L box
'

Private Sub LBox_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.ServerExec "CAMERA HIDESTANDARD"
        LBoxHold = 1
        LBarHold = 1
        VBarHold = 1
        '
        VDelta = X - LBox.Left + VBar.Left
        HDelta = Y - LBox.Top + LBar.Top
    End If
End Sub

Private Sub LBox_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBoxHold And Button = 1 Then
        Call VBar_MouseMove(Button, Shift, X, Y)
        Call LBar_MouseMove(Button, Shift, X, Y)
    End If
End Sub

Private Sub LBox_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If LBoxHold Then frmMain.ResizeAll (True)
    '
    LBoxHold = 0
    LBarHold = 0
    VBarHold = 0
End Sub

'
' R box
'

Private Sub RBox_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = 1 Then
        Ed.ServerExec "CAMERA HIDESTANDARD"
        RBoxHold = 1
        RBarHold = 1
        VBarHold = 1
        '
        VDelta = X - RBox.Left + VBar.Left
        HDelta = Y - RBox.Top + RBar.Top
    End If
End Sub

Private Sub RBox_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBoxHold And Button = 1 Then
        Call VBar_MouseMove(Button, Shift, X, Y)
        Call RBar_MouseMove(Button, Shift, X, Y)
    End If
End Sub

Private Sub RBox_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If RBoxHold Then frmMain.ResizeAll (True)
    RBoxHold = 0
    RBarHold = 0
    VBarHold = 0
End Sub

'
' Open cameras
'

Public Sub OpenCameras(Reopen As Boolean)
    ZOrder 1
    Call Ed.OpenCamera(Reopen, 0, 0, VBar.Left / Screen.TwipsPerPixelX, _
        LBar.Top / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_ORTHXY, "Standard1V")
    Call Ed.OpenCamera(Reopen, (VBar.Left + VBar.Width) / Screen.TwipsPerPixelX, _
        0, (CXL - VBar.Left - VBar.Width) / Screen.TwipsPerPixelX, _
        RBar.Top / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_ORTHXZ, "Standard2V")
    Call Ed.OpenCamera(Reopen, 0, (LBar.Top + LBar.Height) / Screen.TwipsPerPixelY, _
        VBar.Left / Screen.TwipsPerPixelX, (CYL - LBar.Top - LBar.Height) / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_MENU + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_DYNLIGHT, "Standard3V")
    Call Ed.OpenCamera(Reopen, (VBar.Left + VBar.Width) / Screen.TwipsPerPixelX, _
        (RBar.Top + RBar.Height) / Screen.TwipsPerPixelY, _
        (CXL - VBar.Left - VBar.Width) / Screen.TwipsPerPixelX, _
        (CYL - RBar.Top - RBar.Height) / Screen.TwipsPerPixelY, _
        SHOW_NORMAL + SHOW_STANDARD_VIEW + SHOW_AS_CHILD + SHOW_MOVINGBRUSHES, _
        REN_ORTHYZ, "Standard4V")
End Sub

'
' Set positioning
'

Public Sub SetPos()

    VBar.Visible = False
    LBar.Visible = False
    RBar.Visible = False
    LBox.Visible = False
    RBox.Visible = False
    
    Width = frmMain.ScaleWidth
    Height = frmMain.ScaleHeight

    CX = 0
    CY = 0
    CXL = ScaleWidth
    CYL = ScaleHeight

    If ScaleWidth >= 160 * Screen.TwipsPerPixelX And _
        ScaleHeight >= 120 * Screen.TwipsPerPixelY And _
        (Ed.Startup = 0) Then
        
        VBar.Top = 0
        VBar.Height = CYL
        VBar.Left = Ed.CameraVertRatio * (CXL - VBar.Width)
        If VBar.Left < 0 Then VBar.Left = 0
        If VBar.Left > CXL - VBar.Width Then VBar.Left = CXL - VBar.Width
        
        LBar.Left = 0
        LBar.Width = VBar.Left
        LBar.Top = Ed.CameraLeftRatio * (CYL - LBar.Height)
        If LBar.Top < 0 Then LBar.Top = 0
        If LBar.Top > CYL - LBar.Height Then LBar.Top = CYL - LBar.Height
        
        RBar.Left = VBar.Left + VBar.Width
        RBar.Width = CXL - (VBar.Left + VBar.Width)
        RBar.Top = Ed.CameraRightRatio * (CYL - RBar.Height)
        If RBar.Top < 0 Then RBar.Top = 0
        If RBar.Top > CYL - RBar.Height Then RBar.Top = CYL - RBar.Height
        
        LBox.Left = VBar.Left
        LBox.Top = LBar.Top
        
        RBox.Left = VBar.Left
        RBox.Top = RBar.Top
        
        VBar.Visible = True
        LBar.Visible = True
        RBar.Visible = True
        LBox.Visible = True
        RBox.Visible = True
    End If
End Sub

