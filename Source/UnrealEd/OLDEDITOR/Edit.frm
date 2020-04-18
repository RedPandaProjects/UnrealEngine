VERSION 4.00
Begin VB.Form frmEdit 
   Caption         =   "Editor"
   ClientHeight    =   5025
   ClientLeft      =   1635
   ClientTop       =   4275
   ClientWidth     =   8940
   BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   Height          =   5385
   Icon            =   "Edit.frx":0000
   Left            =   1575
   LinkTopic       =   "Form8"
   ScaleHeight     =   5025
   ScaleWidth      =   8940
   ShowInTaskbar   =   0   'False
   Top             =   3975
   Width           =   9060
   Begin VB.TextBox EditBox 
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "Courier New"
         Size            =   9
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00800000&
      Height          =   5055
      Left            =   0
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   0
      Text            =   "Edit.frx":030A
      Top             =   0
      Width           =   8055
   End
   Begin VB.CommandButton Paste 
      Caption         =   "Paste"
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   8160
      TabIndex        =   5
      Top             =   2400
      Width           =   735
   End
   Begin VB.CommandButton Copy 
      Caption         =   "Copy"
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   8160
      TabIndex        =   4
      Top             =   1920
      Width           =   735
   End
   Begin VB.CommandButton Cut 
      Caption         =   "Cut"
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   8160
      TabIndex        =   3
      Top             =   1440
      Width           =   735
   End
   Begin VB.CommandButton Reload 
      Caption         =   "&Reload"
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   8160
      TabIndex        =   2
      Top             =   600
      Width           =   735
   End
   Begin VB.TextBox Item 
      BackColor       =   &H0000FFFF&
      Height          =   285
      Left            =   8160
      TabIndex        =   8
      Text            =   "Item"
      Top             =   2880
      Visible         =   0   'False
      Width           =   735
   End
   Begin VB.TextBox Topic 
      BackColor       =   &H0000FFFF&
      Height          =   285
      Left            =   8160
      TabIndex        =   7
      Text            =   "Topic"
      Top             =   1080
      Visible         =   0   'False
      Width           =   735
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   8160
      TabIndex        =   6
      Top             =   4560
      Width           =   735
   End
   Begin VB.CommandButton Save 
      Caption         =   "&Done"
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   8160
      TabIndex        =   1
      Top             =   120
      Width           =   735
   End
End
Attribute VB_Name = "frmEdit"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim EditorButtonX As Long
Dim EditorButtonY As Long
Dim EditboxX As Long
Dim EditboxY As Long
Dim Loading As Integer

Private Sub Cancel_Click()
    Unload Me
End Sub

Private Sub Copy_Click()
    Editbox.SetFocus
    SendKeys "^{C}"
End Sub

Private Sub Cut_Click()
    Editbox.SetFocus
    SendKeys "^{X}"
End Sub

Private Sub Form_Load()
    Call Ed.MakeFormFit(Me)
    ResizeEditor
End Sub

Private Sub Form_Resize()
    If Loading = 0 Then ResizeEditor
End Sub

Private Sub Item_Change()
    If Left(Item.Text, 1) = "*" Then
        '
        ' Create new
        '
        Call Ed.Server.SetProp(Topic.Text, Mid(Item.Text, 2), "")
        Item.Text = Mid(Item.Text, 2)
    End If
    Editbox.Text = Ed.Server.GetProp(Topic.Text, Item.Text)
End Sub

Private Sub Paste_Click()
    Editbox.SetFocus
    SendKeys "^{V}"
End Sub

Private Sub Reload_Click()
    Editbox.Text = ""
    Editbox.Text = Ed.Server.GetProp(Topic.Text, Item.Text)
End Sub

Private Sub Save_Click()
    Call Ed.Server.SetProp(Topic.Text, Item.Text, Editbox.Text)
    Unload Me
End Sub

Sub ResizeEditor()
    EditorButtonX = ScaleWidth - Cancel.Width - 4 * Screen.TwipsPerPixelX
    EditorButtonY = ScaleHeight - Cancel.Height - 4 * Screen.TwipsPerPixelX
    EditboxX = ScaleWidth - 55 * Screen.TwipsPerPixelX
    If EditboxX < 0 Then EditboxX = 0
    EditboxY = ScaleHeight
    '
    Save.Left = EditorButtonX
    Reload.Left = EditorButtonX
    Cut.Left = EditorButtonX
    Copy.Left = EditorButtonX
    Paste.Left = EditorButtonX
    '
    Cancel.Left = EditorButtonX
    Cancel.Top = EditorButtonY
    '
    Editbox.Width = EditboxX
    Editbox.Height = EditboxY
    '
End Sub
