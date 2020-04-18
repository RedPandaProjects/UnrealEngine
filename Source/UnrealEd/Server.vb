Public Class Server
    Private Declare Sub EdInitServer Lib "Editor.dll" Alias "_EdInitServer@8" (ByVal hWndMain As IntPtr, ByVal hWndCallback As IntPtr)
    Private Declare Sub EdExitServer Lib "Editor.dll" Alias "_EdExitServer@0" ()
    Private Declare Sub EdExec Lib "Editor.dll" Alias "_EdExec@4" (ByVal Cmd As String)
    Private Declare Sub EdSetProp Lib "Editor.dll" Alias "_EdSetProp@12" (ByVal Topic As String, ByVal Item As String, ByVal Value As String)
    Private Declare Function EdGetProp Lib "Editor.dll" Alias "_EdGetProp@8" (ByVal Topic As String, ByVal Item As String) As String

    Private hWndEditor As IntPtr
    '
    '
    Public Sub InitServer(hWndMain As IntPtr, hWndCallback As IntPtr, ProgressBarhWnd As IntPtr, ProgressTexthWnd As IntPtr)
        Dim S As String
        Call EdInitServer(hWndMain, hWndCallback)
        ServerExec("HIDELOG")
        ServerExec("APP SET" &
        " ProgressBar=" & Trim(Str(ProgressBarhWnd.ToInt32())) &
        " ProgressText=" & Trim(Str(ProgressTexthWnd.ToInt32())))
        hWndEditor = hWndMain
    End Sub

    Public Sub ExitServer()
        Call EdExitServer
    End Sub

    Public Sub ServerExec(S As String)
        Call EdExec(S)
    End Sub

    Public Sub ServerSetProp(Topic As String, Item As String, Value As String)
        Call EdSetProp(Topic, Item, Value)
    End Sub

    Public Function ServerGetProp(Topic As String, Item As String) As String
        ServerGetProp = EdGetProp(Topic, Item)
    End Function
End Class
