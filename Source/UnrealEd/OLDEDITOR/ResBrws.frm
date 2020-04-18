VERSION 5.00
Object = "{BE4F3AC8-AEC9-101A-947B-00DD010F7B46}#1.0#0"; "MSOUTL32.OCX"
Begin VB.Form frmResBrowse 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Resource Browser"
   ClientHeight    =   5730
   ClientLeft      =   1935
   ClientTop       =   2820
   ClientWidth     =   4065
   ClipControls    =   0   'False
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
   HelpContextID   =   122
   Icon            =   "ResBrws.frx":0000
   LinkTopic       =   "Form8"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5730
   ScaleWidth      =   4065
   ShowInTaskbar   =   0   'False
   Begin VB.PictureBox Picture1 
      Height          =   5355
      Left            =   0
      ScaleHeight     =   5295
      ScaleWidth      =   3975
      TabIndex        =   3
      Top             =   0
      Width           =   4035
      Begin MSOutl.Outline List 
         Height          =   5295
         Left            =   0
         TabIndex        =   4
         Top             =   0
         Width           =   3975
         _Version        =   65536
         _ExtentX        =   7011
         _ExtentY        =   9340
         _StockProps     =   77
         BackColor       =   -2147483643
         BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         BorderStyle     =   0
         PicturePlus     =   "ResBrws.frx":030A
         PictureMinus    =   "ResBrws.frx":0404
      End
   End
   Begin VB.CommandButton Command4 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Collect Garbage"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   1320
      TabIndex        =   2
      Top             =   5400
      Width           =   1455
   End
   Begin VB.CommandButton Command2 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Cancel          =   -1  'True
      Caption         =   "&Close"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   3120
      TabIndex        =   1
      Top             =   5400
      Width           =   915
   End
   Begin VB.CommandButton Refresh 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Refresh"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   60
      TabIndex        =   0
      Top             =   5400
      Width           =   915
   End
End
Attribute VB_Name = "frmResBrowse"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Command2_Click()
    Unload frmResBrowse ' Unload to reset, don't just hide
End Sub

Private Sub Command3_Click()
    Ed.Server.Exec "RES DEBUG"
    Refresh_Click
End Sub

Private Sub Command4_Click()
    Ed.Server.Exec "RES GARBAGE"
    Refresh_Click
End Sub

Private Sub Form_Load()
    Call InitOutline
    Call Ed.SetOnTop(Me, "ResourceBrowser", TOP_NORMAL)
    Call Refresh_Click
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub List_Expand(ListIndex As Integer)
    Dim Text As String
    Dim Name As String
    Dim ResType As String
    '
    Text = List.List(ListIndex)
    ResType = GrabString(Text)
    Name = GrabString(Text)
    '
    Call ExpandOutline(List, Ed.Server.GetProp( _
        "RES", "QUERY TYPE=" & ResType & " NAME=" & Name), _
        ListIndex, False)
End Sub

Private Sub List_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button And 2 Then
        'PopupMenu frmPopups.BrowserWin
    End If
End Sub

Private Sub Refresh_Click()
    QuerySource = -1
    List.Clear
    List.AddItem "Unreal Server on this PC"
    List.ListIndex = 0
    '
    QuerySource = List.ListIndex
    Call UpdateOutline(List, Ed.Server.GetProp("RES", "QUERY NAME=Root TYPE=Array"))
    List.ListIndex = 0
    List.Expand(0) = True
End Sub
