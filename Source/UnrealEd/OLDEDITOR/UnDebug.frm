VERSION 4.00
Begin VB.Form Form1 
   Caption         =   "Unreal World Debugger"
   ClientHeight    =   5355
   ClientLeft      =   3450
   ClientTop       =   2250
   ClientWidth     =   8835
   Height          =   5715
   Icon            =   "UnDebug.frx":0000
   Left            =   3390
   LinkTopic       =   "Form1"
   ScaleHeight     =   5355
   ScaleWidth      =   8835
   ShowInTaskbar   =   0   'False
   Top             =   1950
   Width           =   8955
   Begin VB.CommandButton Command6 
      Caption         =   "&Actor Properties"
      Height          =   255
      Left            =   1920
      TabIndex        =   5
      Top             =   3960
      Width           =   1455
   End
   Begin VB.CommandButton Command5 
      Caption         =   ">>"
      Height          =   255
      Left            =   4800
      TabIndex        =   4
      Top             =   3960
      Width           =   375
   End
   Begin VB.CommandButton Command4 
      Caption         =   "Possess <<"
      Height          =   255
      Left            =   3600
      TabIndex        =   3
      Top             =   3960
      Width           =   1155
   End
   Begin VB.PictureBox Picture1 
      Height          =   3855
      Left            =   0
      ScaleHeight     =   3795
      ScaleWidth      =   5115
      TabIndex        =   2
      Top             =   60
      Width           =   5175
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Restart  |<-"
      Height          =   255
      Left            =   900
      TabIndex        =   1
      Top             =   3960
      Width           =   975
   End
   Begin VB.CommandButton Command2 
      Caption         =   "Play >>"
      Height          =   255
      Left            =   0
      TabIndex        =   0
      Top             =   3960
      Width           =   855
   End
   Begin Threed.SSPanel SSPanel1 
      Height          =   1035
      Left            =   0
      TabIndex        =   6
      Top             =   4260
      Width           =   5175
      _Version        =   65536
      _ExtentX        =   9128
      _ExtentY        =   1826
      _StockProps     =   15
      BackColor       =   12632256
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Begin VB.CommandButton Command8 
         Caption         =   "Properties"
         Height          =   315
         Left            =   4020
         TabIndex        =   14
         Top             =   660
         Width           =   1095
      End
      Begin VB.CommandButton Command7 
         Caption         =   "Possess"
         Height          =   315
         Left            =   3000
         TabIndex        =   13
         Top             =   660
         Width           =   915
      End
      Begin VB.CommandButton Command3 
         Caption         =   "&Event:"
         Height          =   315
         Left            =   60
         TabIndex        =   10
         Top             =   660
         Width           =   795
      End
      Begin VB.ComboBox Combo2 
         Height          =   315
         Left            =   840
         TabIndex        =   9
         Text            =   "Combo2"
         Top             =   660
         Width           =   2055
      End
      Begin VB.Label Label4 
         BackStyle       =   0  'Transparent
         Caption         =   "<none>"
         Height          =   255
         Left            =   1260
         TabIndex        =   12
         Top             =   360
         Width           =   1755
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Class:"
         Height          =   255
         Left            =   0
         TabIndex        =   11
         Top             =   360
         Width           =   1215
      End
      Begin VB.Label Label3 
         Caption         =   "<none>"
         Height          =   255
         Left            =   1260
         TabIndex        =   8
         Top             =   60
         Width           =   1755
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Selected Actor:"
         Height          =   255
         Left            =   0
         TabIndex        =   7
         Top             =   60
         Width           =   1215
      End
   End
End
Attribute VB_Name = "Form1"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

Private Sub OLE2_Updated(Code As Integer)

End Sub

Private Sub WebBrowser1_OnBeginNavigate(ByVal URL As String, ByVal Flags As Long, ByVal TargetFrameName As String, PostData As Variant, ByVal Headers As String, ByVal Referrer As String, Cancel As Boolean)

End Sub
