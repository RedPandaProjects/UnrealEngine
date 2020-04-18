VERSION 5.00
Object = "{3B7C8863-D78F-101B-B9B5-04021C009402}#1.1#0"; "RICHTX32.OCX"
Begin VB.Form frmScriptEd 
   Caption         =   "Class Script Editor"
   ClientHeight    =   6300
   ClientLeft      =   3570
   ClientTop       =   6435
   ClientWidth     =   7935
   Icon            =   "ClassSc.frx":0000
   LinkTopic       =   "Form1"
   MDIChild        =   -1  'True
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6300
   ScaleWidth      =   7935
   ShowInTaskbar   =   0   'False
   Begin VB.CheckBox ErrorClose 
      Caption         =   "Error:"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      TabStop         =   0   'False
      Top             =   20
      Visible         =   0   'False
      Width           =   735
   End
   Begin VB.TextBox ErrorBox 
      BackColor       =   &H000000FF&
      BeginProperty Font 
         Name            =   "Courier New"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   285
      Left            =   840
      Locked          =   -1  'True
      TabIndex        =   1
      TabStop         =   0   'False
      Text            =   "Error"
      Top             =   0
      Visible         =   0   'False
      Width           =   7095
   End
   Begin RichTextLib.RichTextBox EditBox 
      Height          =   5775
      Left            =   0
      TabIndex        =   0
      Top             =   240
      Width           =   7935
      _ExtentX        =   13996
      _ExtentY        =   10186
      _Version        =   327681
      BackColor       =   4194304
      Enabled         =   -1  'True
      ScrollBars      =   2
      RightMargin     =   26085.17
      AutoVerbMenu    =   -1  'True
      TextRTF         =   $"ClassSc.frx":030A
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "Courier New"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
   End
End
Attribute VB_Name = "frmScriptEd"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim ShiftPressed As Boolean
Dim FirstUndo As Long
Dim LastUndo As Long
Dim CurUndo As Long
Dim Sizing As Boolean
Dim Changed As Boolean
Dim Resizing As Boolean

Dim Undo(20) As String
Dim UndoPos(20) As Long
Dim UndoTop(20) As Long
Dim UndoLength(20) As Long

' Set the editor top.
Public Sub SetTop(DestTop As Long)
    Dim CurTop As Long

    CurTop = SendMessage(EditBox.hwnd, EM_GETFIRSTVISIBLELINE, 0, 0)
    While CurTop > DestTop
        Call SendMessage(EditBox.hwnd, EM_SCROLL, SB_LINEUP, 0)
        CurTop = CurTop - 1
    Wend
    While CurTop < DestTop
        Call SendMessage(EditBox.hwnd, EM_SCROLL, SB_LINEDOWN, 0)
        CurTop = CurTop + 1
    Wend
End Sub

' The set cursor position.
Public Sub LoadAll()

    DisableRedraw (EditBox.hwnd)

    If Visible Then EditBox.SetFocus
    EditBox.TextRTF = Ed.ServerGetProp("RTF", Caption)
    EditBox.SelStart = CLng(Ed.ServerGetProp("SCRIPTPOS", Caption))
    Call SendMessage(EditBox.hwnd, EM_SCROLLCARET, 0, 0)
    SetTop (CLng(Ed.ServerGetProp("SCRIPTTOP", Caption)))

    EnableRedraw (EditBox.hwnd)
    EditBox.Refresh

    DoResize
End Sub

' Save everything.
Public Sub PreSave()
    MarkUndo
    Call Ed.ServerSetProp("SCRIPT", Caption, EditBox.Text)
    Call Ed.ServerSetProp("SCRIPTPOS", Caption, Str(EditBox.SelStart))
    Call Ed.ServerSetProp("SCRIPTTOP", Caption, Str(SendMessage(EditBox.hwnd, EM_GETFIRSTVISIBLELINE, 0, 0)))
End Sub

' Set selection start, end, and length.
Public Sub GotoText(Start As Long, Length As Long)
    Show
    EditBox.SetFocus
    EditBox.SelStart = Start
    EditBox.SelLength = Length
    Call SendMessage(EditBox.hwnd, EM_SCROLLCARET, 0, 0)
End Sub

' Set script editable flag.
Private Sub SetScriptControls(Flag As Boolean, EditFlag As Boolean)
    frmMain.EditFind.Visible = Flag
    frmMain.EditFindNext.Visible = Flag
    frmMain.EditDivider2.Visible = Flag
End Sub

Private Sub EditBox_Change()
    Changed = True
End Sub

Private Sub ErrorClose_Click()
    If Not Resizing Then
        ErrorBox.Text = ""
        DoResize
    End If
End Sub

Private Sub Form_Activate()
    Set frmMain.ScriptForm = Me
    frmMain.hwndScript = hwnd
    Call SetScriptControls(True, Not EditBox.Locked)
End Sub

Private Sub Form_Deactivate()
    If frmMain.hwndScript = Me.hwnd Then
        Set frmMain.ScriptForm = Nothing
        frmMain.hwndScript = 0
        Call SetScriptControls(False, False)
    End If
End Sub

Public Sub ScriptEditDefaults_Click()
    Ed.ServerExec "HOOK CLASSPROPERTIES CLASS=" & Caption ''xyzzy
End Sub

Private Sub SaveState(index As Long)
    UndoPos(index) = EditBox.SelStart
    UndoLength(index) = EditBox.SelLength
    UndoTop(index) = SendMessage(EditBox.hwnd, EM_GETFIRSTVISIBLELINE, 0, 0)
    Undo(index) = EditBox.TextRTF
End Sub

Private Sub MarkUndo()
    LastUndo = (CurUndo + 1) Mod 20

    SaveState (LastUndo)
    If LastUndo = FirstUndo Then
        FirstUndo = (FirstUndo + 1) Mod 20
    End If

    CurUndo = LastUndo
    Changed = False
End Sub

Private Sub PerformDo(index As Long, CursorDelta As Long)
    Dim CursorIndex As Long
    CursorIndex = (index + CursorDelta + 20) Mod 20

    DisableRedraw (EditBox.hwnd)

    EditBox.TextRTF = Undo(index)
    EditBox.SetFocus
    EditBox.SelStart = UndoPos(CursorIndex)
    EditBox.SelLength = UndoLength(CursorIndex)
    Call SendMessage(EditBox.hwnd, EM_SCROLLCARET, 0, 0)
    SetTop (UndoTop(index))

    EnableRedraw (EditBox.hwnd)
    EditBox.Refresh
End Sub

Public Sub EditUndo_Click()
    If CurUndo <> FirstUndo And Not EditBox.Locked Then
        If Changed Then
            MarkUndo
        End If

        CurUndo = (CurUndo + 20 - 1) Mod 20
        Call PerformDo(CurUndo, 0)
    End If
    Changed = False
End Sub

Public Sub EditRedo_Click()
    If CurUndo <> LastUndo And Not EditBox.Locked And Not Changed Then
        CurUndo = (CurUndo + 1) Mod 20
        Call PerformDo(CurUndo, 0)
    End If
    Changed = False
End Sub

Private Sub Editbox_KeyPress(KeyAscii As Integer)
    Dim TabStuff As String
    Dim NewText As String
    Dim i As Integer
    Dim c As Integer
    Dim S, e As Integer
    
    S = EditBox.Text
    
    If KeyAscii = 9 And EditBox.SelLength > 0 Then
        MarkUndo
        DisableRedraw (EditBox.hwnd)
        KeyAscii = 0
        S = EditBox.SelStart
        If ShiftPressed Then
            NewText = EditBox.SelText
            If Left(NewText, 1) = Chr(9) Then NewText = Mid(NewText, 2)
            For i = 1 To Len(NewText)
                If Mid(NewText, i, 2) = Chr(10) + Chr(9) Then
                    NewText = Left(NewText, i) + Mid(NewText, i + 2)
                End If
            Next i
        Else
            NewText = Chr(9) + EditBox.SelText
            For i = 1 To Len(NewText)
                If Mid(NewText, i, 1) = Chr(10) Then
                    NewText = Left(NewText, i) + Chr(9) + Mid(NewText, i + 1)
                    i = i + 1
                End If
            Next i
        End If
        EditBox.SelText = NewText
        EditBox.SelStart = S
        EditBox.SelLength = Len(NewText)
        EnableRedraw (EditBox.hwnd)
        EditBox.Refresh
    End If
End Sub

Private Sub Editbox_KeyDown(KeyCode As Integer, Shift As Integer)
    Dim T As String
    Dim N As Long, S As Long
    Dim Pre As String
    If KeyCode = 13 And Not EditBox.Locked Then
        ' Enter.
        S = EditBox.SelStart
        T = Left(EditBox.Text, S)
        N = Len(T) + 2
        While N > 1
            If Mid(T, N - 1, 1) = Chr(10) Then
                Pre = ""
                While Mid(T, N, 1) = Chr(9) Or Mid(T, N, 1) = " "
                    Pre = Pre + Mid(T, N, 1)
                    N = N + 1
                Wend
                GoTo Done
            End If
            N = N - 1
        Wend
Done:
        MarkUndo
        EditBox.SelText = Chr(13) & Chr(10) & Pre
        EditBox.SelLength = 0
        KeyCode = 0
    ElseIf Changed And (KeyCode = 38 Or KeyCode = 40 Or KeyCode = 40 Or _
        KeyCode = 33 Or KeyCode = 34 Or KeyCode = 36 Or KeyCode = 35) Then
        ' Cursor movement.
        MarkUndo
    ElseIf KeyCode = 46 Then
        ' Del.
        MarkUndo
    ElseIf KeyCode = 86 And (Shift And vbCtrlMask) Then
        ' Ctrl-V
        EditPaste_Click
        KeyCode = 0
    ElseIf KeyCode = 90 And (Shift And vbCtrlMask) Then
        ' Ctrl-Z.
        EditUndo_Click
        KeyCode = 0
    End If

    If (Shift And vbShiftMask) <> 0 Then
        ShiftPressed = True
    Else
        ShiftPressed = False
    End If

End Sub

Private Sub Editbox_KeyUp(KeyCode As Integer, Shift As Integer)
    ShiftPressed = False
End Sub

Public Sub EditCopy_Click()
    Clipboard.Clear
    Clipboard.SetText EditBox.SelText
End Sub

Public Sub EditCut_Click()
    MarkUndo
    Clipboard.Clear
    Clipboard.SetText EditBox.SelText
    EditBox.SelText = ""
End Sub

Public Sub EditPaste_Click()
    MarkUndo
    EditBox.SelText = Clipboard.GetText
End Sub

Public Sub EditFind_Click()
    frmScriptFind.ScriptName = Caption
    frmScriptFind.LineNum.Text = Str(1 + EditBox.GetLineFromChar(1 + EditBox.SelStart))
    frmScriptFind.DoReplace.Enabled = Not EditBox.Locked
    frmScriptFind.DoReplaceAll.Enabled = Not EditBox.Locked
    frmScriptFind.Show 1
    EditFindNext_Click
End Sub

Public Sub EditFindNext_Click()
    Dim Find As String
    Dim Replace As String
    Dim Test As String
    Dim Length As Long
    Dim LineNum As Long
    Dim P As Long, c As Long, D As Long
    Dim DoCaps As Long
    Dim OldStart As Long

    If GFindResult = 4 Then
        ' Go to line number via binary search.
        LineNum = CLng(frmScriptFind.LineNum) - 1
        Length = Len(EditBox.Text)
        c = Length
        P = 0
        While c > 0
            If P < 1 Then P = 1
            If P >= Length Then P = Length
            D = EditBox.GetLineFromChar(P)
            If D < LineNum Then
                P = P + c
            ElseIf D > LineNum Then
                P = P - c
            Else
                GoTo Found
            End If
            c = c / 2
        Wend
        If c = 0 Then Exit Sub
Found:
        Call GotoText(P, 0)
        SendKeys "{END}+{HOME}"
        Exit Sub
    End If
    
    Find = frmScriptFind.FindText
    Replace = frmScriptFind.ReplaceText
    
    If Trim(Find) = "" Then Exit Sub
    
    DoCaps = IIf(frmScriptFind.CaseSensitive.Value, 0, 1)
    
    If DoCaps = 1 Then Find = UCase(Find)

    If GFindResult = 1 Then
        ' Find.
DoFind:
        P = InStr( _
            EditBox.SelStart + EditBox.SelLength + 1, _
            EditBox.Text, _
            Find, _
            DoCaps)
        If P > 0 Then
            Call GotoText(P - 1, Len(Find))
        Else
            Call GotoText(Len(EditBox.Text), 0)
        End If
        If GFindResult = 3 Then GoTo DoRep
        '
    ElseIf GFindResult = 2 Then
        '
        ' Replace:
DoRep:  '
        Test = Mid(EditBox.Text, EditBox.SelStart + 1, Len(Find))
        If DoCaps = 1 Then Test = UCase(Test)
        '
        If Test = Find Then
            OldStart = EditBox.SelStart
            EditBox.Text = _
                Left(EditBox.Text, EditBox.SelStart) & _
                Replace & _
                Mid(EditBox.Text, EditBox.SelStart + Len(Find) + 1)
            Call GotoText(OldStart + Len(Replace), 0)
            GoTo DoFind
        Else
            If GFindResult <> 3 Then GoTo DoFind
        End If
        Call GotoText(Len(EditBox.Text), 0)
        '
    ElseIf GFindResult = 3 Then
        '
        ' Replace All
        '
        GoTo DoFind
    End If
End Sub

Private Sub Form_Load()
    Dim i As Integer
    Dim Tabs As DWORDREC
    Dim Intrinsic As Boolean
    
    Me.Left = (ScriptEdLeft + 140) * Screen.TwipsPerPixelX: ScriptEdLeft = (ScriptEdLeft + 32) Mod 300
    Me.Top = (ScriptEdTop + 140) * Screen.TwipsPerPixelY: ScriptEdTop = (ScriptEdTop + 32) Mod 200
    
    Call Ed.SetOnTop(Me, "ClassScriptEditor" & Str(GNumMiscForms), TOP_NORMAL)
    
    Tabs.Value = 16
    Call SendTabsMessage(EditBox.hwnd, EM_SETTABSTOPS, 1, Tabs)
    
    ' Rich text control.
    EditBox.SelStart = 0
    EditBox.SelLength = Len(EditBox.Text)
    EditBox.SelColor = &HFF00&
    EditBox.SelTabCount = 16
    For i = 0 To 15
        EditBox.SelTabs(i) = 420 * i
    Next i

    DoResize
End Sub

Private Sub Form_Resize()
    If Sizing = False Then
        Sizing = True
        If WindowState <> 1 Then
            ' Maximized or resized.
            DoResize
        Else
            ' Minimized.
            Show
            SetFocus
        End If
        Sizing = False
    End If
End Sub

Public Sub DoResize()
    Resizing = True
    Dim MustExit As Boolean
    If WindowState = 1 Then Exit Sub
    If Width < 320 * Screen.TwipsPerPixelX Then
        Width = 320 * Screen.TwipsPerPixelX
        MustExit = True
    End If
    If Height < 240 * Screen.TwipsPerPixelY Then
        Height = 240 * Screen.TwipsPerPixelY
        MustExit = True
    End If
    If MustExit Then GoTo Out

    EditBox.Width = ScaleWidth
    ErrorBox.Width = ScaleWidth - ErrorBox.Left
    If ErrorBox.Text = "" Then
        EditBox.Top = 0
        EditBox.Height = ScaleHeight - EditBox.Top
        ErrorBox.Visible = False
        ErrorClose.Visible = False
        ErrorClose.Value = 1
    Else
        EditBox.Top = ErrorBox.Top + ErrorBox.Height
        EditBox.Height = ScaleHeight - EditBox.Top
        ErrorBox.Visible = True
        ErrorClose.Visible = True
        ErrorClose.Value = 1
    End If
Out:
    Resizing = False
End Sub

Private Sub Form_Unload(Cancel As Integer)
    PreSave
    Form_Deactivate
    Call Ed.EndOnTop(Me)
    Call RemoveMiscForm(Me)
End Sub

Public Sub ResetUndo()
    UndoPos(LastUndo) = EditBox.SelStart
    Undo(LastUndo) = EditBox.TextRTF
End Sub

Public Sub PostLoad()
    
    DisableRedraw (EditBox.hwnd)
    Call GotoText(0, Len(EditBox.Text))
    EditBox.TextRTF = Ed.ServerGetProp("RTF", Caption)
    Call GotoText(0, 0)
    EnableRedraw (EditBox.hwnd)
    
    EditBox.Refresh
    ResetUndo

End Sub

