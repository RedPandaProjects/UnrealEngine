VERSION 4.00
Begin VB.Form frmClassResults 
   BorderStyle     =   5  'Sizable ToolWindow
   Caption         =   "Script Compiler Results"
   ClientHeight    =   4635
   ClientLeft      =   2505
   ClientTop       =   7065
   ClientWidth     =   7485
   Height          =   5100
   Left            =   2445
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4635
   ScaleWidth      =   7485
   ShowInTaskbar   =   0   'False
   Top             =   6660
   Width           =   7605
   Begin VB.ListBox Results 
      BackColor       =   &H00000000&
      BeginProperty Font 
         name            =   "Courier New"
         charset         =   0
         weight          =   400
         size            =   9
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FFFF&
      Height          =   3930
      IntegralHeight  =   0   'False
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   7515
   End
End
Attribute VB_Name = "frmClassResults"
Attribute VB_Creatable = False
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
    S = Ed.Server.GetProp("TEXT", "ScriptError")
    '
    Results.Clear
    L = GrabLine(S)
    While L <> ""
        Results.AddItem L
    Wend
    '
    Show
End Sub

'
' Private
'

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "CompileResults", TOP_PANEL)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

