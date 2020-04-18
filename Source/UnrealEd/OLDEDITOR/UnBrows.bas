Attribute VB_Name = "Browsers"
'
' Various support routines for browsers.
'

Option Explicit

Dim Bogus As String
Dim Spacer As String
Dim Remove As String
Dim TempExpand As Integer

Public QuerySource As Integer

' Init the outline.
Sub InitOutline()
    Bogus = "Loading..." + Chr(9) + Chr(9) + Chr(9) + Chr(9) + Chr(9) + "*"
    Spacer = Chr(9) + Chr(9) + Chr(9) + Chr(9) + Chr(9)
End Sub

' Expand the outline.
Sub ExpandOutline(List As Control, _
    Results As String, _
    ListIndex As Integer, _
    ForceExpand As Boolean)
    
    Dim i As Integer, j As Integer
    Dim Text As String, Temp As String
    
    If TempExpand Then
        ' Don't expand if recursing.
        Exit Sub
    ElseIf Left(List.List(ListIndex), 1) = Bogus Then
        ' Don't expand a bogus item.
        Exit Sub
    End If
    
    ' Force the outline to expand.
    If ForceExpand Then
        Text = List.FullPath(ListIndex) + List.PathSeparator
        j = Len(Text)
        For i = List.ListCount - 1 To 0 Step -1
            If Left(List.FullPath(i), j) = Text Then
                List.RemoveItem i
            End If
        Next
        GoTo DoExpand
    End If

    ' See if this node has a bogus child.  If not,
    ' it has already been expanded.
    Temp = Bogus + List.FullPath(ListIndex)
    For j = 0 To List.ListCount - 1
        If List.List(j) = Temp Then
            ' Found a bogus child, expand it.
            GoTo DoExpand
        End If
    Next j
    
    ' Didn't find bogus child; already expanded.
    GoTo Out

DoExpand:
    Screen.MousePointer = 11
    
    ' Remember to replace bogus child.
    Remove = Bogus & List.FullPath(ListIndex)
    QuerySource = ListIndex
    
    Call UpdateOutline(List, Results)
Out:
    Screen.MousePointer = 0
    TempExpand = 1
    List.Expand(ListIndex) = True
    TempExpand = 0
End Sub

' Add one item to the outline.
Private Sub ProcessOutlineResult(List As Control, Source As String)
    Dim j As Integer
    Dim Descr As String
    Dim Find As String
    
    If (QuerySource <> -1) And Trim(Source) <> "" Then
        Descr = Source
        j = InStr(Descr, "|")
        
        If j <> 0 Then
            Descr = Left(Descr, j - 1) + Chr(9) + Chr(9) + Chr(9) + Chr(9) + Chr(9) + Mid(Descr, j + 1)
        End If
        
        If Remove <> "" Then
            ' Replace the bogus item with this item so that the
            ' sorting order of the list doesn't get messed up.
            For j = 0 To List.ListCount - 1
                If List.List(j) = Remove Then
                    List.ListIndex = j
                    List.List(j) = Descr
                    Remove = ""
                    GoTo Skip
                End If
            Next j
        End If
        
        ' Add new item:
        List.ListIndex = QuerySource
        List.AddItem Descr
Skip:
        Find = List.FullPath(QuerySource) & List.PathSeparator & Descr
        For j = 0 To List.ListCount - 1
            If List.FullPath(j) = Find Then
                List.ListIndex = j
                GoTo Ok
            End If
        Next
        MsgBox "Lost new item"
Ok:
        ' If resource has children, add a bogus entry
        ' to catch them when the list is expanded.
        If Right(Source, 1) = "C" Then
            List.AddItem Bogus & List.FullPath(List.ListIndex)
        End If
    Else
        '
        ' Expand list, preventing recursion:
        '
        TempExpand = TempExpand + 1
        List.Expand(QuerySource) = True
        List.ListIndex = QuerySource
        TempExpand = TempExpand - 1
    End If
End Sub

Sub UpdateOutline(List As Control, Results As String)
    While Results <> ""
        Call ProcessOutlineResult(List, GrabCommaString(Results))
    Wend
End Sub
