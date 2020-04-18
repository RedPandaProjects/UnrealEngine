VERSION 5.00
Begin VB.Form frmNewTex 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Create a new texture"
   ClientHeight    =   3105
   ClientLeft      =   4680
   ClientTop       =   9120
   ClientWidth     =   4245
   Icon            =   "NewTex.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3105
   ScaleWidth      =   4245
   ShowInTaskbar   =   0   'False
   Begin VB.Frame Frame2 
      Caption         =   "Size"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   675
      Left            =   60
      TabIndex        =   3
      Top             =   1980
      Width           =   4095
      Begin VB.ComboBox VSize 
         Height          =   315
         Left            =   2820
         Style           =   2  'Dropdown List
         TabIndex        =   4
         Top             =   240
         Width           =   1035
      End
      Begin VB.ComboBox USize 
         Height          =   315
         Left            =   900
         Style           =   2  'Dropdown List
         TabIndex        =   11
         Top             =   240
         Width           =   1035
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "V-Size:"
         Height          =   195
         Left            =   2040
         TabIndex        =   12
         Top             =   300
         Width           =   675
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "U-Size:"
         Height          =   195
         Left            =   120
         TabIndex        =   10
         Top             =   300
         Width           =   675
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Texture"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1875
      Left            =   60
      TabIndex        =   7
      Top             =   60
      Width           =   4095
      Begin VB.TextBox TexGroup 
         Height          =   315
         Left            =   1020
         TabIndex        =   14
         Text            =   "Text1"
         Top             =   1080
         Width           =   2775
      End
      Begin VB.ComboBox TexClass 
         Height          =   315
         Left            =   1020
         Style           =   2  'Dropdown List
         TabIndex        =   2
         Top             =   1440
         Width           =   2775
      End
      Begin VB.TextBox TexSet 
         Height          =   315
         Left            =   1020
         TabIndex        =   1
         Text            =   "Text1"
         Top             =   720
         Width           =   2775
      End
      Begin VB.TextBox TexName 
         Height          =   315
         Left            =   1020
         TabIndex        =   0
         Text            =   "Text1"
         Top             =   360
         Width           =   2775
      End
      Begin VB.Label Label6 
         Alignment       =   1  'Right Justify
         Caption         =   "Group:"
         Height          =   255
         Left            =   120
         TabIndex        =   15
         Top             =   1140
         Width           =   855
      End
      Begin VB.Label Label5 
         Alignment       =   1  'Right Justify
         Caption         =   "Class:"
         Height          =   255
         Left            =   180
         TabIndex        =   13
         Top             =   1500
         Width           =   795
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Package:"
         Height          =   255
         Left            =   120
         TabIndex        =   9
         Top             =   780
         Width           =   855
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "Name:"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   420
         Width           =   855
      End
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   3300
      TabIndex        =   6
      Top             =   2700
      Width           =   855
   End
   Begin VB.CommandButton Ok 
      Caption         =   "C&reate this texture"
      Default         =   -1  'True
      Height          =   375
      Left            =   60
      TabIndex        =   5
      Top             =   2700
      Width           =   1815
   End
End
Attribute VB_Name = "frmNewTex"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim UniqueCount As Integer

Private Sub Cancel_Click()
    GResult = 0
    Hide
End Sub

Private Sub Form_Activate()
    Dim S As String, T As String

    GResult = 0

    UniqueCount = UniqueCount + 1
    TexName.Text = "MyTex" & Trim(Str(UniqueCount))

    TexSet.Text = frmTexBrowser.TexSet.Text
    If TexSet.Text = AllString Then TexSet.Text = "MyTextures"

    TexGroup.Text = frmTexBrowser.TexGroup.Text
    If TexGroup.Text = AllString Then TexGroup.Text = "None"
    
    TexClass.Clear
    S = Ed.ServerGetProp("Class", "GetChildren Class=Texture Concrete=1")
    T = GrabString(S)
    While T <> ""
        TexClass.AddItem T
        If UCase(T) = "FIRETEXTURE" Then TexClass.ListIndex = TexClass.ListCount - 1
        T = GrabString(S)
    Wend
    If TexClass.ListIndex < 0 Then TexClass.ListIndex = 0
End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "NewClass", TOP_NORMAL)

    USize.AddItem "1"
    USize.AddItem "2"
    USize.AddItem "4"
    USize.AddItem "8"
    USize.AddItem "16"
    USize.AddItem "32"
    USize.AddItem "64"
    USize.AddItem "128"
    USize.AddItem "256"
    USize.ListIndex = 8

    VSize.AddItem "1"
    VSize.AddItem "2"
    VSize.AddItem "4"
    VSize.AddItem "8"
    VSize.AddItem "16"
    VSize.AddItem "32"
    VSize.AddItem "64"
    VSize.AddItem "128"
    VSize.AddItem "256"
    VSize.ListIndex = 8
End Sub



Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Ok_Click()
    Ed.ServerExec "TEXTURE NEW" & _
        " NAME=" & Quotes(TexName.Text) & _
        " CLASS=" & TexClass.Text & _
        " GROUP=" & TexGroup.Text & _
        " USIZE=" & USize.Text & _
        " VSIZE=" & VSize.Text & _
        " PACKAGE=" & Quotes(TexSet.Text)
    GResult = 1
    Hide
End Sub

