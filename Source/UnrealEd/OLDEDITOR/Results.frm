VERSION 5.00
Begin VB.Form frmResults 
   BorderStyle     =   5  'Sizable ToolWindow
   Caption         =   "Results Window"
   ClientHeight    =   2160
   ClientLeft      =   3735
   ClientTop       =   7635
   ClientWidth     =   8190
   Icon            =   "Results.frx":0000
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   2160
   ScaleWidth      =   8190
   ShowInTaskbar   =   0   'False
   Begin VB.ListBox Results 
      BackColor       =   &H00000000&
      BeginProperty Font 
         Name            =   "Courier New"
         Size            =   9
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FFFF&
      Height          =   1860
      IntegralHeight  =   0   'False
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   8175
   End
End
Attribute VB_Name = "frmResults"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'
' Public
'

'
' Update the compile results by snagging the
' script error property from the server
'
Sub UpdateResults()
    
    Dim S As String, L As String
    S = Ed.ServerGetProp("Text", "Results")
    
    Results.Clear
    L = GrabLine(S)
    While L <> "" Or S <> ""
        Results.AddItem L
        L = GrabLine(S)
    Wend
    
    If (Results.ListCount > 1) Then
        Results.ListIndex = Results.ListCount - 2
    End If
    
    Show
End Sub

Public Sub GoToNext()
    If Not Visible Then UpdateResults
    If Results.ListCount > 0 Then
        If Results.ListIndex + 1 < Results.ListCount Then
            Results.ListIndex = Results.ListIndex + 1
        Else
            Results.ListIndex = 0
        End If
        Results_DblClick
    End If
End Sub

'
' Print a status message
'
Sub UpdateStatus(S As String)
    Results.Clear
    Results.AddItem S
    Show
End Sub

Private Sub Results_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyF4 Then GoToNext
End Sub

'
' Private
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "CompileResults", TOP_PANEL)
    
    Left = 0
    Width = frmMain.ScaleWidth
    Top = frmMain.ScaleHeight - Height
End Sub

Private Sub Form_Resize()
    Results.Width = ScaleWidth
    Results.Height = ScaleHeight
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Public Sub Results_DblClick()
End Sub
