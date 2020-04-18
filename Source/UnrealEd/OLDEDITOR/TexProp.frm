VERSION 5.00
Begin VB.Form frmTexProp 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Texture Properties"
   ClientHeight    =   4695
   ClientLeft      =   3630
   ClientTop       =   3945
   ClientWidth     =   5385
   ForeColor       =   &H80000008&
   HelpContextID   =   123
   Icon            =   "TexProp.frx":0000
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   4695
   ScaleWidth      =   5385
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton Clear 
      Caption         =   "Clear"
      Height          =   315
      Left            =   1500
      TabIndex        =   2
      Top             =   0
      Width           =   975
   End
   Begin VB.ComboBox Zoom 
      Height          =   315
      Left            =   120
      Style           =   2  'Dropdown List
      TabIndex        =   1
      Top             =   0
      Width           =   1335
   End
   Begin VB.PictureBox Holder 
      BackColor       =   &H00000000&
      BorderStyle     =   0  'None
      Height          =   4335
      Left            =   0
      ScaleHeight     =   4335
      ScaleWidth      =   5415
      TabIndex        =   0
      Top             =   360
      Width           =   5415
   End
End
Attribute VB_Name = "frmTexProp"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim Loading As Boolean

Private Sub Clear_Click()
    Ed.ServerExec "TEXTURE CLEAR NAME=" & Quotes(Caption)
End Sub

Private Sub Form_Load()
    Loading = True
    
    Call Ed.SetOnTop(Me, "TextureProperties", TOP_PANEL)
    
    Zoom.AddItem "25%"
    Zoom.AddItem "50%"
    Zoom.AddItem "100%"
    Zoom.AddItem "200%"
    Zoom.ListIndex = 2
    
    Loading = False
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
    Ed.ServerExec "CAMERA CLOSE NAME=TexPropCam"
End Sub

Public Sub SetTexture(Name As String)
    Dim S As String
    Dim F As Double

    Caption = Name

    S = Ed.ServerGetProp("Texture", "SIZE TEXTURE=" & Caption)

    If InStr(S, ",") >= 0 Then
        Holder.Visible = False
        F = Val(Zoom.Text) / 100
        Holder.Width = F * Screen.TwipsPerPixelX * Val(S)
        Holder.Height = F * Screen.TwipsPerPixelY * Val(Mid(S, InStr(S, ",") + 1))
        If Holder.Width < Clear.Left + Clear.Width Then
            Width = Clear.Left + Clear.Width + Width - ScaleWidth
        Else
            Width = Holder.Width + Width - ScaleWidth
        End If
        Height = Holder.Height + Holder.Top + Height - ScaleHeight
        Ed.ServerExec "CAMERA OPEN " & _
            " TEXTURE=" & Quotes(Caption) & _
            " NAME=TexPropCam X=0 Y=0" & _
            " XR=" & Trim(Str(Holder.ScaleWidth / Screen.TwipsPerPixelX)) & _
            " YR=" & Trim(Str(Holder.ScaleHeight / Screen.TwipsPerPixelY)) & _
            " REN=" & Trim(Str(REN_TEXVIEW)) & _
            " FLAGS=" & Trim(Str(SHOW_AS_CHILD + SHOW_NOCAPTURE + SHOW_NOBUTTONS + SHOW_REALTIME)) & _
            " HWND=" & Trim(Str(Holder.hwnd))
        Holder.Visible = True
        Show
    Else
        Unload Me
    End If
End Sub

Private Sub Zoom_Click()
    If Not Loading Then
        SetTexture Caption
    End If
End Sub
