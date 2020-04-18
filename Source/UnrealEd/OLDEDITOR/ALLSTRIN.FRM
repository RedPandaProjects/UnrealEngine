VERSION 4.00
Begin VB.Form frmActorProperties 
   BorderStyle     =   0  'None
   Caption         =   "Actor Properties"
   ClientHeight    =   6750
   ClientLeft      =   5835
   ClientTop       =   5160
   ClientWidth     =   5115
   ForeColor       =   &H80000008&
   Height          =   7110
   HelpContextID   =   113
   Icon            =   "AllString.frx":0000
   KeyPreview      =   -1  'True
   Left            =   5775
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6750
   ScaleWidth      =   5115
   ShowInTaskbar   =   0   'False
   Top             =   4860
   Width           =   5235
   Begin VB.CommandButton NextCat 
      Caption         =   ">"
      Height          =   285
      Left            =   1635
      TabIndex        =   14
      Top             =   30
      Width           =   180
   End
   Begin VB.CommandButton PrevCat 
      Caption         =   "<"
      Height          =   285
      Left            =   1455
      TabIndex        =   13
      Top             =   30
      Width           =   180
   End
   Begin VB.ComboBox Category 
      Height          =   315
      Left            =   1845
      Style           =   2  'Dropdown List
      TabIndex        =   12
      Top             =   15
      Width           =   3270
   End
   Begin VB.VScrollBar PropScroll 
      Height          =   5895
      Left            =   4875
      TabIndex        =   10
      Top             =   370
      Width           =   240
   End
   Begin VB.PictureBox PropHolder 
      BackColor       =   &H00C0C0C0&
      FillColor       =   &H00FFFFFF&
      Height          =   5910
      Left            =   0
      ScaleHeight     =   5850
      ScaleWidth      =   5070
      TabIndex        =   0
      Top             =   350
      Width           =   5130
      Begin VB.PictureBox peComboHolder 
         Height          =   255
         Left            =   4560
         ScaleHeight     =   195
         ScaleWidth      =   210
         TabIndex        =   7
         Top             =   2700
         Visible         =   0   'False
         Width           =   270
         Begin VB.ComboBox peCombo 
            Height          =   315
            Left            =   -3120
            TabIndex        =   8
            Text            =   "Combo1"
            Top             =   -75
            Width           =   3360
         End
      End
      Begin VB.CommandButton peBrowse 
         Caption         =   ".."
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   0
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   220
         Left            =   4500
         TabIndex        =   4
         Top             =   1830
         Visible         =   0   'False
         Width           =   300
      End
      Begin VB.TextBox peText 
         BackColor       =   &H00FFFFFF&
         BorderStyle     =   0  'None
         ForeColor       =   &H00000000&
         Height          =   210
         Left            =   1515
         MousePointer    =   3  'I-Beam
         TabIndex        =   3
         Text            =   "Text"
         Top             =   2205
         Visible         =   0   'False
         Width           =   3285
      End
      Begin VB.CommandButton peCurrent 
         Caption         =   "Use Current"
         Height          =   220
         Left            =   3495
         TabIndex        =   2
         Top             =   1830
         Visible         =   0   'False
         Width           =   1005
      End
      Begin VB.CommandButton peClear 
         Caption         =   "Clear"
         Height          =   220
         Left            =   2940
         TabIndex        =   1
         Top             =   1830
         Visible         =   0   'False
         Width           =   555
      End
      Begin ComctlLib.Slider peSlider 
         Height          =   240
         Left            =   2145
         TabIndex        =   6
         Top             =   2475
         Visible         =   0   'False
         Width           =   2715
         _Version        =   65536
         _ExtentX        =   4789
         _ExtentY        =   423
         _StockProps     =   64
         LargeChange     =   32
         SmallChange     =   8
         Max             =   255
         TickStyle       =   3
      End
      Begin MSGrid.Grid PropGrid 
         Height          =   6225
         Left            =   -45
         TabIndex        =   5
         Top             =   0
         Width           =   4920
         _Version        =   65536
         _ExtentX        =   8678
         _ExtentY        =   10980
         _StockProps     =   77
         ForeColor       =   0
         BackColor       =   16777215
         BorderStyle     =   0
         Rows            =   1
         Cols            =   4
         FixedRows       =   0
         FixedCols       =   0
         ScrollBars      =   0
      End
   End
   Begin VB.PictureBox NothingButton 
      BorderStyle     =   0  'None
      Height          =   300
      Left            =   3525
      ScaleHeight     =   300
      ScaleWidth      =   435
      TabIndex        =   9
      Top             =   6330
      Width           =   435
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "Properties to edit:"
      Height          =   255
      Left            =   60
      TabIndex        =   11
      Top             =   60
      Width           =   1335
   End
End
Attribute VB_Name = "frmActorProperties"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
'
' Actor properties dialog.
'
Dim Scrolling As Integer
Dim CellHeight As Integer
Dim CurCategory As String
Dim CurProp As Integer
Dim Modified As Boolean
Dim PressedEsc As Boolean
Dim GPropString As String
Dim GCatString As String
Dim Sliding As Integer
Dim Resizing As Integer
Dim ScreenRows As Integer
Dim OldWidth As Integer
Dim SettingCombo As Integer
Dim WasEmpty As Boolean
Dim HolderForm As Form
'
Const MinWidth = 3500
Const MaxWidth = 7000

'
' Re-get properties from list of selected actors
'
Public Sub GetSelectedActors()
    Dim N As Integer
    Dim S As String
    Dim DesiredName As String
    Dim DesiredY As Integer
    '
    EndEdit (True)
    '
    GActorPropsAction = 1
    GPropString = "Properties"
    GCatString = "PropCats"
    N = Val(Ed.Server.GetProp("Actor", "NumSelected"))
    '
    If N = 0 Then
        Caption = "Actor Properties (0 selected)"
        PropGrid.Visible = False
        PropScroll.Visible = False
        peClear.Visible = False
        peCurrent.Visible = False
        peBrowse.Visible = False
        peText.Visible = False
        peSlider.Visible = False
        peComboHolder.Visible = False
        WasEmpty = True
    Else
        S = Ed.Server.GetProp("Actor", "ClassSelected")
        If S <> "" Then
            Caption = S & " Properties (" & Trim(Str(N)) & " selected)"
        Else
            Caption = "Actor Properties (" & Trim(Str(N)) & " selected)"
        End If
        '
        ParsePropList
    End If
    '
End Sub

'
' Set parent window
'
Public Sub SetFormParent(ParentForm As Form, ParentControl As Control)
    Dim R As RECT
    '
    Call SetParent(Me.hwnd, ParentControl.hwnd)
    Call GetClientRect(ParentControl.hwnd, R)
    Call SetWindowPos(Me.hwnd, 0, 0, 0, R.Right - R.Left, R.Bottom - R.Top, 0)
    '
    Set HolderForm = ParentForm
End Sub

'
' Get properties from a class's default actor
'
Public Sub GetClassDefaultActor(Classname As String)
    '
    EndEdit (True)
    '
    GActorPropsAction = 2
    GPropString = "DefaultProperties CLASS=" & Quotes(Classname)
    GCatString = "DefaultPropCats CLASS=" & Quotes(Classname)
    '
    Caption = Classname + ": Default Properties"
    '
    ParsePropList
End Sub

'
' Notifys the actor properties dialog that properties have
' changed, used in script editor refresh logic
'
Public Sub NoteClassChange()
    If GActorPropsAction <> 0 Then
        ParsePropList
    End If
End Sub

Private Sub Category_Click()
    If Scrolling = 0 Then
        If CurCategory <> Category.List(Category.ListIndex) Then
            EndEdit (False)
            CurCategory = Category.List(Category.ListIndex)
            ParsePropList
        End If
    End If
End Sub

Private Sub Form_Load()
    '
    WasEmpty = True
    OldWidth = ScaleWidth - PropScroll.Width
    '
    'Call Ed.SetOnTop(Me, "ActorProperties", TOP_NORMAL)
    CurProp = -1
    PressedEsc = False
    '
    CellHeight = PropGrid.RowHeight(0)
    '
    PropGrid.Width = 8000
    PropGrid.Left = 0
    PropGrid.ColWidth(0) = 1480
    PropGrid.ColWidth(1) = 6000
    PropGrid.ColWidth(2) = 500
    PropGrid.ColWidth(3) = 500
    '
    CurCategory = ""
    '
End Sub

Private Sub FixTopRow()
    If PropGrid.TopRow + ScreenRows > PropGrid.Rows Then
        If PropGrid.Rows - ScreenRows >= 0 Then
            PropGrid.TopRow = PropGrid.Rows - ScreenRows
        Else
            PropGrid.TopRow = 0
        End If
    End If
End Sub

Private Sub SetPropGridHeight()
    Dim CellHeight As Integer
    CellHeight = PropGrid.RowHeight(0) + Screen.TwipsPerPixelY
    '
    If (PropGrid.Rows < ScreenRows) Then
        PropGrid.Height = PropGrid.Rows * CellHeight
    Else
        PropGrid.Height = PropHolder.Height
    End If
End Sub

Private Sub ResizeButtons()
    Dim NewWidth As Integer
    Dim DW As Integer
    '
    NewWidth = ScaleWidth
    If PropScroll.Visible Then NewWidth = NewWidth - PropScroll.Width
    '
    DW = NewWidth - OldWidth
    '
    peText.Width = peText.Width + DW
    peSlider.Width = peSlider.Width + DW
    peComboHolder.Left = peComboHolder.Left + DW
    peCombo.Width = peCombo.Width + DW
    peCombo.Left = peCombo.Left - DW
    peClear.Left = peClear.Left + DW
    peCurrent.Left = peCurrent.Left + DW
    peBrowse.Left = peBrowse.Left + DW
    '
    OldWidth = NewWidth
End Sub

Private Sub Form_Resize()
    Dim PerfectHeight As Integer, CellHeight As Integer
    Dim NonGridHeight As Integer, DW As Integer
    '
    Scrolling = Scrolling + 1
    If Resizing = 0 Then
        Resizing = Resizing + 1
        EndEdit (True)
        '
        CellHeight = PropGrid.RowHeight(0) + Screen.TwipsPerPixelY
        NonGridHeight = Height - ScaleHeight - PropHolder.Top + Screen.TwipsPerPixelY
        '
        If Width < MinWidth Then
            Width = MinWidth
        ElseIf Width > MaxWidth Then
            Width = MaxWidth
        End If
        '
        ScreenRows = (Height - NonGridHeight) / CellHeight
        '
        If ScreenRows < 6 Then ScreenRows = 6
        PerfectHeight = ScreenRows * CellHeight + NonGridHeight
        '
        If Height <> PerfectHeight Then
            Height = PerfectHeight
        End If
        '
        PropHolder.Height = ScaleHeight - PropHolder.Top
        PropHolder.Width = ScaleWidth
        '
        SetPropGridHeight
        '
        PropScroll.Left = ScaleWidth - PropScroll.Width - 1 * Screen.TwipsPerPixelX
        PropScroll.Height = ScaleHeight - PropScroll.Top - 1 * Screen.TwipsPerPixelY
        Category.Width = ScaleWidth - Category.Left
        '
        ScreenRows = PropHolder.ScaleHeight / CellHeight
        '
        UpdateScroller
        PropGrid.LeftCol = 0
        '
        FixTopRow
        '
        ' Resize/move hidden controls
        '
        ResizeButtons
        '
        Resizing = Resizing - 1
    End If
    Scrolling = Scrolling - 1
End Sub

Private Sub Form_Unload(Cancel As Integer)
    GActorPropsAction = 0
    Call Ed.EndOnTop(Me)
    Set HolderForm = Nothing
End Sub

Private Sub NextCat_Click()
    If Category.ListIndex < (Category.ListCount - 1) Then
        Category.ListIndex = Category.ListIndex + 1
    Else
        Category.ListIndex = 0
    End If
End Sub

Private Sub peText_Change()
    If Sliding = 0 Then
        Modified = True
        If peSlider.Visible Then
            Sliding = Sliding + 1
            peSlider.Value = Val(peText.Text)
            Sliding = Sliding - 1
        End If
    End If
End Sub

Private Sub peSlider_Change()
    If Sliding = 0 Then
        Sliding = Sliding + 1
        '
        SetValue (Str(peSlider.Value))
        peSlider.Value = Val(GetValue())
        peText.Text = GetValue()
        '
        Sliding = Sliding - 1
    End If
End Sub

Public Function VectorChange(NewValue As String) As String
    SetValue (NewValue)
    VectorChange = GetValue()
End Function

Public Function RotationChange(NewValue As String) As String
    SetValue (NewValue)
    RotationChange = GetValue()
End Function

Private Sub peSlider_Scroll()
    Sliding = Sliding + 1
    peText.Text = Trim(Str(peSlider.Value))
    Sliding = Sliding - 1
End Sub

Private Sub peCombo_Click()
    If SettingCombo = 0 Then
        SettingCombo = SettingCombo + 1
        If peCombo.ListIndex >= 0 Then SetValue (Str(Val(peCombo.Text)))
        SetGridSelection (0)
        SettingCombo = SettingCombo - 1
    End If
End Sub

Private Sub peText_KeyPress(KeyAscii As Integer)
    If KeyAscii = 13 Then ' Pressed enter
        PropGrid_Click
    ElseIf KeyAscii = 27 Then ' Pressed escape
        PressedEsc = True
        EndEdit (True) ' Abort current editing
        BeginEdit ' Restart editing with original property
    End If
End Sub

Private Sub PrevCat_Click()
    If Category.ListIndex > 0 Then
        Category.ListIndex = Category.ListIndex - 1
    Else
        Category.ListIndex = Category.ListCount - 1
    End If
End Sub

'
' Properties grid functions
'

Private Sub PropGrid_DblClick()
    Dim Temp As String
    If PropGrid.Row = CurProp Then
        Scrolling = Scrolling + 1
        '
        Select Case UCase(PropType())
        Case "BYTE":
            Temp = PropExtra()
            If Temp = "" Then
                peText.SetFocus
                peText.SelStart = 0
                peText.SelLength = Len(peText.Text)
            Else
                ' Advance to next item
                peCombo.SetFocus
                peCombo.ListIndex = (peCombo.ListIndex + 1) Mod peCombo.ListCount
            End If
        Case "INTEGER":
            peText.SetFocus
            peText.SelStart = 0
            peText.SelLength = Len(peText.Text)
        Case "BOOLEAN":
            NothingButton.SetFocus
            If UCase(GetValue()) = "TRUE" Then
                SetValue "False"
            Else
                SetValue "True"
            End If
        Case "REAL":
            peText.SetFocus
            peText.SelStart = 0
            peText.SelLength = Len(peText.Text)
        Case "ACTOR":
        Case "NAME":
            peText.SetFocus
            peText.SelStart = 0
            peText.SelLength = Len(peText.Text)
        Case "STRING":
            peText.SetFocus
            peText.SelStart = 0
            peText.SelLength = Len(peText.Text)
        Case "VECTOR":
            If GetValue() <> "" Then
                Call frmEditVector.BeginEditVector( _
                    PropName(), GetValue(), Me)
            End If
        Case "ROTATION":
            If GetValue() <> "" Then
                Call frmEditRotation.BeginEditRotation( _
                    PropName(), GetValue(), Me)
            End If
        Case Else: ' All resource types
            If UCase(PropName()) = "CLASS" Then
                Call frmClassBrowser.LaunchScriptEd(GetValue(), "", 0, 0)
            Else
                peBrowse_Click
            End If
        End Select
        '
        Scrolling = Scrolling - 1
        SetGridSelection (0)
    End If
End Sub

Private Sub PropGrid_Click()
    Dim Temp As String, N As Double
    If (PropGrid.Row = CurProp) And Modified Then
        '
        Scrolling = Scrolling + 1
        '
        Select Case UCase(PropType())
        Case "BYTE":
            Temp = PropExtra()
            If Temp = "" Then
                If Eval(peText.Text, N) Then SetValue (Str(N))
                peText.Text = GetValue()
                peText.SetFocus
                peSlider = Val(peText.Text)
            Else
                peCombo.SetFocus
            End If
        Case "INTEGER":
            If Eval(peText.Text, N) Then SetValue (Str(N))
            peText.Text = GetValue()
            peText.SetFocus
        Case "REAL":
            If Eval(peText.Text, N) Then SetValue (Str(N))
            peText.Text = GetValue()
            peText.SetFocus
        Case "NAME":
            SetExactValue (peText.Text)
            peText.Text = GetValue()
            peText.SetFocus
        Case "STRING":
            SetValue (Quotes(peText.Text))
            peText.Text = GetValue()
            peText.SetFocus
        Case "BOOLEAN":
        Case "ACTOR":
        Case "VECTOR":
        Case "ROTATION":
        Case Else: ' All resource types
        End Select
        '
        Modified = False
        Scrolling = Scrolling - 1
        SetGridSelection (0)
    End If
End Sub

Private Sub PropScroll_GotFocus()
    If Not PressedEsc Then
        EndEdit (False)
        SetGridSelection (0)
    Else
        PressedEsc = False
    End If
End Sub

Sub SetGridSelection(MouseMoving As Integer)
    If Scrolling = 0 Then
        If MouseMoving Then
            If PropGrid.SelStartRow < PropGrid.Row Then
                PropGrid.Row = PropGrid.SelStartRow
            Else
                PropGrid.Row = PropGrid.SelEndRow
            End If
            '
        End If
        Scrolling = Scrolling + 1
        PropGrid.Col = 3
        PropGrid.SelStartCol = 0
        PropGrid.SelEndCol = 0
        PropGrid.SelStartRow = PropGrid.Row
        PropGrid.SelEndRow = PropGrid.Row
        PropGrid.LeftCol = 0
        FixTopRow
        Scrolling = Scrolling - 1
    End If
End Sub

Private Sub PropGrid_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If (Button <> 0) Then SetGridSelection (1)
End Sub

Private Sub PropGrid_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    EndEdit (False)
    SetGridSelection (1)
End Sub

Private Sub PropGrid_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    SetGridSelection (1)
    If CurProp <> PropGrid.Row Then BeginEdit
End Sub

Private Sub PropGrid_SelChange()
    SetGridSelection (0)
End Sub

'
' Scroller
'

Sub UpdateScroller()
    If PropGrid.Rows > ScreenRows Then
        PropScroll.LargeChange = ScreenRows
        PropScroll.Max = PropGrid.Rows - ScreenRows
        PropScroll.Visible = True
    Else
        PropScroll.Visible = False
        PropScroll.LargeChange = 1
        PropScroll.Max = 0
    End If
    PropScroll.SmallChange = 1
    '
    ResizeButtons
    '
End Sub

Private Sub PropScroll_Change()
    PropScroll_Scroll
End Sub

Private Sub PropScroll_Scroll()
    If Scrolling = 0 Then
        Scrolling = Scrolling + 1
        PropGrid.TopRow = PropScroll.Value
        PropGrid.SetFocus
        Scrolling = Scrolling - 1
    End If
End Sub

'
' Property parsing
'

Private Sub ParseOneProperty(S As String, Add As Boolean)
    Dim i As Integer, j As Integer, k As Integer
    Dim TypeName As String
    '
    i = InStr(S, Chr(13))
    If i > 0 Then S = Left(S, i - 1)
    '
    S = Trim(S)
    If S <> "" Then
        If Add Then
            PropGrid.AddItem ""
            PropGrid.Row = PropGrid.Rows - 1
        End If
        '
        i = InStr(S, " ")
        TypeName = Left(S, i - 1)
        k = InStr(TypeName, ".")
        If k = 0 Then
            PropGrid.Col = 2 ' Type
            PropGrid.Text = TypeName
            PropGrid.Col = 3 ' Extra
            PropGrid.Text = ""
        Else
            PropGrid.Col = 3 ' Extra
            PropGrid.Text = Mid(TypeName, k + 1)
            '
            TypeName = Left(TypeName, k - 1)
            PropGrid.Col = 2 ' Type
            PropGrid.Text = TypeName
        End If
        '
        j = InStr(S, "=")
        PropGrid.Col = 0 ' Name
        PropGrid.Text = Mid(S, i + 1, j - i - 1)
        '
        PropGrid.Col = 1 ' Value
        PropGrid.Text = Mid(S, j + 1)
    End If
End Sub

Private Sub ParsePropList()
    Dim PropList As String, PropCats As String
    Dim iStart As Integer, iEnd As Integer, Y As Integer, i As Integer
    Dim S As String, Add As String
    Dim DesiredName As String
    Dim DesiredY As Integer
    '
    Scrolling = Scrolling + 1
    PropGrid.SelStartCol = 3
    PropGrid.SelEndCol = 3
    '
    ' If was empty previously, set default category
    ' ---------------------------------------------
    '
    If WasEmpty Then
        S = (Ed.Server.GetProp("Actor", GPropString & " NAME=DefaultEdCategory"))
        Call GetString(S, "DefaultEdCategory=", CurCategory)
    End If
    '
    ' Update categories
    ' -----------------
    '
    Category.Clear
    '
    i = 0
    Y = 0
    PropCats = Ed.Server.GetProp("Actor", GCatString)
    While PropCats <> ""
        S = Trim(GrabString(PropCats))
        Category.AddItem S
        If S = CurCategory Then
            Category.ListIndex = i
            Y = 1
        End If
        i = i + 1
    Wend
    '
    Category.AddItem AllString
    If CurCategory = AllString Then
        Category.ListIndex = i
    ElseIf Y = 0 Then
        Category.ListIndex = 0
        CurCategory = Category.List(Category.ListIndex)
    End If
    '
    ' Update properties
    ' -----------------
    '
    If CurCategory = AllString Then
        PropList = Ed.Server.GetProp("Actor", GPropString)
    Else
        PropList = Ed.Server.GetProp("Actor", GPropString & " CATEGORY=" & CurCategory)
    End If
    '
    ' Try not to lose the selection and position of the current
    ' property if one exists:
    '
    If PropGrid.Rows > 1 Then
        PropGrid.Col = 0
        DesiredName = UCase(PropGrid.Text)
        DesiredY = PropGrid.Row - PropGrid.TopRow
    Else
        DesiredName = ""
    End If
    '
    PropGrid.Rows = 1 ' Clear it out
    '
    iStart = 1
ParseLoop:
    Do
        iEnd = InStr(iStart, PropList, Chr(13))
        If iEnd = 0 Then GoTo DoneParsing
        S = Trim(Mid(PropList, iStart, iEnd - iStart))
        iStart = iEnd + 2
        '
        ' Process one line of actor properties:
        '
        Call ParseOneProperty(S, True)
    Loop
    '
DoneParsing:
    If PropGrid.Rows > 0 Then PropGrid.RemoveItem 0
    '
    ' Reposition current property:
    '
    If DesiredName <> "" Then
        PropGrid.Col = 0
        For i = 0 To PropGrid.Rows - 1
            PropGrid.Row = i
            If UCase(PropGrid.Text) = DesiredName Then
                CurProp = PropGrid.Row
                Y = i - DesiredY
                If Y >= 0 And Y < (PropGrid.Rows - ScreenRows) Then
                    PropGrid.TopRow = Y
                End If
                GoTo Done
            End If
        Next i
    End If
    PropGrid.Row = 0
Done:
    SetPropGridHeight
    Scrolling = Scrolling - 1
    '
    PropGrid.Visible = True
    PropScroll.Visible = True
    PropGrid.ZOrder 1
    PropScroll.ZOrder
    Show
    UpdateScroller
    BeginEdit
    WasEmpty = False
    '
End Sub

'
' Properties grid cell editing
'

Private Sub BeginEdit()
    Dim StartPos As Integer, NewProp As Integer, N As Integer
    Dim Temp As String, Tag As String
    '
    Scrolling = Scrolling + 1
    '
    NewProp = PropGrid.Row
    If CurProp >= 0 Then EndEdit (False)
    CurProp = NewProp
    '
    If PropScroll.Value <> PropGrid.TopRow And PropGrid.TopRow <= PropScroll.Max Then
        PropScroll.Value = PropGrid.TopRow
    End If
    '
    StartPos = (CurProp - PropGrid.TopRow) * (CellHeight + 1 * Screen.TwipsPerPixelY) - 1 * Screen.TwipsPerPixelY
    Select Case UCase(PropType())
    Case "BYTE":
        Temp = PropExtra()
        If Temp = "" Then
            peText.Top = StartPos + 2 * Screen.TwipsPerPixelY
            peText.Text = GetValue()
            peText.Visible = True
            peText.SelStart = 0
            peText.SelLength = 0
            peText.SetFocus
            peSlider.Top = StartPos
            peSlider.Value = Val(GetValue())
            peSlider.Visible = True
            peSlider.ZOrder
        Else
            NothingButton.SetFocus
            peComboHolder.Top = StartPos
            '
            ' Fill combo with enum tags:
            '
            SettingCombo = SettingCombo + 1
            peCombo.Clear
            Temp = Ed.Server.GetProp("Enum", Temp)
            Do
                Tag = GrabCommaString(Temp)
                If Tag <> "" Then peCombo.AddItem Tag
            Loop While Temp <> ""
            '
            Temp = GetValue()
            If Temp <> "" Then
                N = Val(GetValue())
                If N >= 0 And N < peCombo.ListCount Then
                    peCombo.ListIndex = N
                Else
                    peCombo.ListIndex = 0
                End If
            Else
                peCombo.ListIndex = -1 ' No selection
            End If
            peComboHolder.Visible = True
            SettingCombo = SettingCombo - 1
        End If
    Case "INTEGER":
        peText.Top = StartPos + 2 * Screen.TwipsPerPixelY
        peText.Text = GetValue()
        peText.Visible = True
        peText.SelStart = 0
        peText.SelLength = 0
        peText.SetFocus
    Case "BOOLEAN":
        PropGrid.SetFocus
    Case "REAL":
        peText.Top = StartPos + 2 * Screen.TwipsPerPixelY
        peText.Text = GetValue()
        peText.Visible = True
        peText.SelStart = 0
        peText.SelLength = 0
        peText.SetFocus
    Case "ACTOR":
    Case "NAME":
        peText.Top = StartPos + 2 * Screen.TwipsPerPixelY
        peText.Text = GetValue()
        peText.Visible = True
        peText.SelStart = 0
        peText.SelLength = 0
        peText.SetFocus
    Case "STRING":
        peText.Top = StartPos + 2 * Screen.TwipsPerPixelY
        peText.Text = GetValue()
        peText.Visible = True
        peText.SelStart = 0
        peText.SelLength = 0
        peText.SetFocus
    Case "VECTOR":
    Case "ROTATION":
    Case Else: ' All resource types
        If UCase(PropName()) <> "CLASS" Then
            peBrowse.Top = StartPos + Screen.TwipsPerPixelY
            peCurrent.Top = StartPos + Screen.TwipsPerPixelY
            peClear.Top = StartPos + Screen.TwipsPerPixelY
            '
            If UCase(PropType()) <> "MODEL" Then peBrowse.Visible = True
            '
            peCurrent.Visible = True
            peClear.Visible = True
        End If
    End Select
    '
    Modified = False
    Scrolling = Scrolling - 1
    SetGridSelection (0)
End Sub

Private Sub EndEdit(Abort As Boolean)
    Dim Temp As String
    '
    If PropGrid.Rows <= 1 Or CurProp < 0 Then Exit Sub
    '
    Scrolling = Scrolling + 1
    PropGrid.RowHeight(CurProp) = CellHeight
    '
    Select Case UCase(PropType())
    Case "BYTE":
        Temp = PropExtra()
        If Temp = "" Then
            If Modified And Not Abort Then SetValue (peText.Text)
            peText.Visible = False
            peSlider.Visible = False
        Else
            peComboHolder.Visible = False
        End If
    Case "INTEGER":
        If Modified And Not Abort Then SetValue (peText.Text)
        peText.Visible = False
    Case "BOOLEAN":
    Case "REAL":
        If Modified And Not Abort Then SetValue (peText.Text)
        peText.Visible = False
    Case "ACTOR":
    Case "NAME":
        If Modified And Not Abort Then SetExactValue (peText.Text)
        peText.Visible = False
    Case "STRING":
        If Modified And Not Abort Then SetValue (Quotes(peText.Text))
        peText.Visible = False
    Case "VECTOR":
        frmEditVector.EndEditVector
    Case "ROTATION":
        frmEditRotation.EndEditRotation
    Case Else: ' All resource types
        peBrowse.Visible = False
        peCurrent.Visible = False
        peClear.Visible = False
    End Select
    '
    Modified = False
    CurProp = -1
    Scrolling = Scrolling - 1
    '
End Sub

Private Function PropType() As String
    Scrolling = Scrolling + 1
    '
    PropGrid.Row = CurProp
    PropGrid.Col = 2
    PropType = PropGrid.Text
    '
    Scrolling = Scrolling - 1
End Function

Private Function PropExtra() As String
    Scrolling = Scrolling + 1
    '
    PropGrid.Row = CurProp
    PropGrid.Col = 3
    PropExtra = PropGrid.Text
    '
    Scrolling = Scrolling - 1
End Function

Private Function PropName() As String
    Scrolling = Scrolling + 1
    '
    PropGrid.Row = CurProp
    PropGrid.Col = 0
    PropName = PropGrid.Text
    '
    Scrolling = Scrolling - 1
End Function

Private Function GetValue() As String
    Scrolling = Scrolling + 1
    '
    PropGrid.Row = CurProp
    PropGrid.Col = 1
    GetValue = PropGrid.Text
    '
    Scrolling = Scrolling - 1
End Function

Private Sub SetExactValue(Value As String)
    Dim UnparsedResult As String
    '
    Scrolling = Scrolling + 1
    '
    Call Ed.Server.SetProp("Actor", GPropString, PropName() & "=" & Value)
    '
    Call ParseOneProperty(Ed.Server.GetProp("Actor", GPropString + " NAME=" & PropName()), False)
    '
    Scrolling = Scrolling - 1
    SetGridSelection (0)
    DoEvents
    Ed.Server.Exec "LEVEL REDRAW"
End Sub

Private Sub SetValue(Value As String)
    If Value <> "" Then SetExactValue (Value)
End Sub

'
' Resources: Browse, use current:
'

Private Sub peCurrent_Click()
    Dim Cur As String
    '
    PropGrid.SetFocus
    If CurProp >= 0 Then
        Scrolling = Scrolling + 1
        Select Case PropType()
        Case "Texture":
            Cur = Ed.GetBrowserCurrentItem("Textures")
            If Cur <> "" Then SetValue (Cur)
        Case "Class":
            Cur = Ed.GetBrowserCurrentItem("Classes")
            If Cur <> "" Then SetValue (Cur)
        Case "Sound":
            Cur = Ed.GetBrowserCurrentItem("SoundFX")
            If Cur <> "" Then SetValue (Cur)
        Case "Ambient":
            Cur = Ed.GetBrowserCurrentItem("Ambient")
            If Cur <> "" Then SetValue (Cur)
        Case "MeshMap":
            Cur = frmMeshViewer.GetCurrent()
            If Cur <> "" Then SetValue (Cur)
        Case "Model":
            Cur = Ed.Server.GetProp("Map", "DuplicateBrush PlaceAt=" & GetValue())
            If Cur <> "" Then SetValue (Cur)
        End Select
        Scrolling = Scrolling - 1
        SetGridSelection (0)
    End If
End Sub

Private Sub peClear_Click()
    PropGrid.SetFocus
    If CurProp >= 0 Then
        Scrolling = Scrolling + 1
        SetValue "None"
        Scrolling = Scrolling - 1
        SetGridSelection (0)
    End If
End Sub

Private Sub peBrowse_Click()
    PropGrid.SetFocus
    If CurProp >= 0 Then
        Scrolling = Scrolling + 1
        Select Case PropType()
        Case "Texture":
            Ed.SetBrowserTopic ("Textures")
        Case "Class":
            Ed.SetBrowserTopic ("Classes")
        Case "Ambient":
            Ed.SetBrowserTopic ("Ambient")
        Case "Sound":
            Ed.SetBrowserTopic ("SoundFX")
        Case "MeshMap":
            frmMeshViewer.Show
        End Select
        Scrolling = Scrolling - 1
        SetGridSelection (0)
    End If
End Sub

