VERSION 5.00
Object = "{0BA686C6-F7D3-101A-993E-0000C0EF6F5E}#1.0#0"; "THREED32.OCX"
Begin VB.Form frmParSolSpiralStair 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Build a Spiral Stair"
   ClientHeight    =   7470
   ClientLeft      =   6180
   ClientTop       =   1170
   ClientWidth     =   3015
   ClipControls    =   0   'False
   ControlBox      =   0   'False
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H00C0C0C0&
   HelpContextID   =   155
   LinkTopic       =   "Form4"
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   7470
   ScaleWidth      =   3015
   ShowInTaskbar   =   0   'False
   Begin VB.PictureBox Picture1 
      Height          =   7692
      Left            =   0
      ScaleHeight     =   7635
      ScaleWidth      =   2955
      TabIndex        =   19
      Top             =   0
      Width           =   3012
      Begin VB.CheckBox chkAlignSide 
         Caption         =   "Align to Side"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   240
         TabIndex        =   18
         Top             =   3240
         Value           =   1  'Checked
         Width           =   1332
      End
      Begin VB.CommandButton Help 
         BackColor       =   &H00C0C0C0&
         Cancel          =   -1  'True
         Caption         =   "&Help"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   1080
         TabIndex        =   9
         Top             =   6960
         Width           =   855
      End
      Begin VB.CommandButton Build 
         Caption         =   "&Build"
         Default         =   -1  'True
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   120
         TabIndex        =   8
         Top             =   6960
         Width           =   855
      End
      Begin VB.CommandButton Command2 
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
         Height          =   375
         Left            =   2040
         TabIndex        =   10
         Top             =   6960
         Width           =   855
      End
      Begin VB.CommandButton CalcTotalHeight 
         Caption         =   "Calc"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   48
         TabIndex        =   24
         TabStop         =   0   'False
         Top             =   5460
         Width           =   495
      End
      Begin VB.TextBox txtGroup 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   7
         Text            =   "Spiral"
         Top             =   5880
         Width           =   1455
      End
      Begin VB.TextBox txtStepThickness 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   4
         Text            =   "32"
         Top             =   4680
         Width           =   1455
      End
      Begin VB.TextBox txtStepHeight 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   3
         Text            =   "16"
         Top             =   4440
         Width           =   1455
      End
      Begin VB.TextBox txtOuterWallRadius 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   2
         Text            =   "256"
         Top             =   4080
         Width           =   1455
      End
      Begin VB.TextBox txtOuterSRad 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   1
         Text            =   "256-16"
         Top             =   3840
         Width           =   1455
      End
      Begin VB.TextBox txtInnerSRad 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   0
         Text            =   "64"
         Top             =   3600
         Width           =   1455
      End
      Begin VB.TextBox txtNumSteps 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   6
         Text            =   "8"
         Top             =   5280
         Width           =   1455
      End
      Begin VB.TextBox txtCycleNum 
         BackColor       =   &H00FFFFFF&
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00000000&
         Height          =   285
         Left            =   1560
         TabIndex        =   5
         Text            =   "8"
         Top             =   5040
         Width           =   1455
      End
      Begin VB.Frame Frame1 
         Caption         =   "Fill"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   855
         Left            =   1815
         TabIndex        =   23
         Top             =   390
         Width           =   1095
         Begin VB.CheckBox chkIntFill 
            Caption         =   "Interrior"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   12
            Top             =   240
            Width           =   855
         End
         Begin VB.CheckBox chkExtFill 
            Caption         =   "Exterior"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   13
            Top             =   480
            Width           =   855
         End
      End
      Begin VB.Frame Frame2 
         Caption         =   "Ceiling"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   855
         Left            =   1815
         TabIndex        =   22
         Top             =   2310
         Width           =   1095
         Begin VB.OptionButton optCeilingStepped 
            Caption         =   "Steps"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   16
            Top             =   240
            Value           =   -1  'True
            Width           =   855
         End
         Begin VB.OptionButton optCeilingSloped 
            Caption         =   "Sloped"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   17
            Top             =   480
            Width           =   855
         End
      End
      Begin VB.Frame Frame3 
         Caption         =   "Surface"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   855
         Left            =   1815
         TabIndex        =   21
         Top             =   1350
         Width           =   1095
         Begin VB.OptionButton optSurfaceStepped 
            Caption         =   "Steps"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   14
            Top             =   240
            Value           =   -1  'True
            Width           =   855
         End
         Begin VB.OptionButton optSurfaceSloped 
            Caption         =   "Sloped"
            BeginProperty Font 
               Name            =   "MS Sans Serif"
               Size            =   8.25
               Charset         =   0
               Weight          =   400
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Left            =   120
            TabIndex        =   15
            Top             =   480
            Width           =   855
         End
      End
      Begin VB.CheckBox chkClockwise 
         Caption         =   "Clockwise"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   372
         Left            =   1935
         TabIndex        =   11
         Top             =   30
         Value           =   1  'Checked
         Width           =   1212
      End
      Begin Threed.SSPanel SSPanel1 
         Height          =   3135
         Left            =   15
         TabIndex        =   20
         Top             =   30
         Width           =   1695
         _Version        =   65536
         _ExtentX        =   2990
         _ExtentY        =   5530
         _StockProps     =   15
         BackColor       =   12632256
         BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         BevelInner      =   1
         Begin VB.Image Image4 
            Appearance      =   0  'Flat
            Height          =   480
            Left            =   360
            Picture         =   "PsSStair.frx":0000
            Top             =   840
            Width           =   480
         End
         Begin VB.Image Image3 
            Appearance      =   0  'Flat
            Height          =   384
            Left            =   840
            Top             =   960
            Width           =   384
         End
         Begin VB.Image Image2 
            Appearance      =   0  'Flat
            Height          =   384
            Left            =   960
            Top             =   360
            Width           =   384
         End
         Begin VB.Image Image1 
            Appearance      =   0  'Flat
            Height          =   384
            Left            =   360
            Top             =   240
            Width           =   384
         End
         Begin VB.Shape Shape1 
            BackColor       =   &H00C0C0C0&
            BackStyle       =   1  'Opaque
            BorderColor     =   &H0000FFFF&
            FillColor       =   &H0000FFFF&
            FillStyle       =   5  'Downward Diagonal
            Height          =   495
            Left            =   600
            Shape           =   3  'Circle
            Top             =   600
            Width           =   495
         End
         Begin VB.Line Line7 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   1440
            Y1              =   840
            Y2              =   840
         End
         Begin VB.Line Line54 
            BorderColor     =   &H0000FFFF&
            X1              =   840
            X2              =   600
            Y1              =   2520
            Y2              =   2640
         End
         Begin VB.Line Line53 
            BorderColor     =   &H0000FFFF&
            X1              =   840
            X2              =   1080
            Y1              =   2160
            Y2              =   2040
         End
         Begin VB.Line Line52 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   840
            Y1              =   2400
            Y2              =   2520
         End
         Begin VB.Line Line51 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   1080
            Y1              =   2400
            Y2              =   2280
         End
         Begin VB.Line Line50 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   840
            Y1              =   2280
            Y2              =   2160
         End
         Begin VB.Line Line49 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   1080
            Y1              =   2760
            Y2              =   3000
         End
         Begin VB.Line Line48 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   1080
            Y1              =   2640
            Y2              =   2760
         End
         Begin VB.Line Line47 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   1080
            Y1              =   2400
            Y2              =   2640
         End
         Begin VB.Line Line46 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   1080
            Y1              =   2040
            Y2              =   2280
         End
         Begin VB.Line Line45 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   1080
            Y1              =   1920
            Y2              =   2040
         End
         Begin VB.Line Line44 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   1080
            Y1              =   1680
            Y2              =   1920
         End
         Begin VB.Line Line43 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1080
            Y1              =   2760
            Y2              =   2640
         End
         Begin VB.Line Line42 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   600
            Y1              =   2400
            Y2              =   2280
         End
         Begin VB.Line Line41 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1080
            Y1              =   2040
            Y2              =   1920
         End
         Begin VB.Line Line40 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1560
            Y1              =   3000
            Y2              =   3000
         End
         Begin VB.Line Line39 
            BorderColor     =   &H0000FFFF&
            X1              =   120
            X2              =   240
            Y1              =   1680
            Y2              =   1680
         End
         Begin VB.Line Line38 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1440
            Y1              =   2760
            Y2              =   3000
         End
         Begin VB.Line Line37 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1440
            Y1              =   2040
            Y2              =   2280
         End
         Begin VB.Line Line36 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   240
            Y1              =   2400
            Y2              =   2640
         End
         Begin VB.Line Line35 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   240
            Y1              =   1680
            Y2              =   1920
         End
         Begin VB.Line Line34 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   1080
            Y1              =   2760
            Y2              =   3000
         End
         Begin VB.Line Line33 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   1080
            Y1              =   2040
            Y2              =   2280
         End
         Begin VB.Line Line31 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   600
            Y1              =   2400
            Y2              =   2640
         End
         Begin VB.Line Line32 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1440
            Y1              =   2280
            Y2              =   2520
         End
         Begin VB.Line Line30 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1080
            Y1              =   2280
            Y2              =   2400
         End
         Begin VB.Line Line29 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   1440
            Y1              =   2760
            Y2              =   2760
         End
         Begin VB.Line Line28 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1440
            Y1              =   2760
            Y2              =   2520
         End
         Begin VB.Line Line27 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1080
            Y1              =   2280
            Y2              =   2280
         End
         Begin VB.Line Line26 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   1080
            Y1              =   2280
            Y2              =   2760
         End
         Begin VB.Line Line25 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   1440
            Y1              =   3000
            Y2              =   3000
         End
         Begin VB.Line Line24 
            BorderColor     =   &H0000FFFF&
            X1              =   1560
            X2              =   1560
            Y1              =   3000
            Y2              =   1680
         End
         Begin VB.Line Line23 
            BorderColor     =   &H0000FFFF&
            X1              =   1560
            X2              =   1440
            Y1              =   1680
            Y2              =   1680
         End
         Begin VB.Line Line22 
            BorderColor     =   &H0000FFFF&
            X1              =   1440
            X2              =   1440
            Y1              =   1680
            Y2              =   2040
         End
         Begin VB.Line Line21 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   1440
            Y1              =   2040
            Y2              =   2040
         End
         Begin VB.Line Line20 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   1080
            Y1              =   1680
            Y2              =   2040
         End
         Begin VB.Line Line19 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   600
            Y1              =   3000
            Y2              =   3000
         End
         Begin VB.Line Line18 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   600
            Y1              =   2640
            Y2              =   3000
         End
         Begin VB.Line Line17 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   600
            Y1              =   1680
            Y2              =   1680
         End
         Begin VB.Line Line16 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   600
            Y1              =   1920
            Y2              =   1680
         End
         Begin VB.Line Line15 
            BorderColor     =   &H0000FFFF&
            X1              =   1080
            X2              =   600
            Y1              =   1680
            Y2              =   1680
         End
         Begin VB.Line Line14 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   240
            Y1              =   1920
            Y2              =   2160
         End
         Begin VB.Line Line13 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   600
            Y1              =   1920
            Y2              =   1920
         End
         Begin VB.Line Line12 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   240
            Y1              =   2160
            Y2              =   2400
         End
         Begin VB.Line Line11 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   600
            Y1              =   1920
            Y2              =   2400
         End
         Begin VB.Line Line10 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   600
            Y1              =   2400
            Y2              =   2400
         End
         Begin VB.Line Line9 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   600
            Y1              =   1920
            Y2              =   2040
         End
         Begin VB.Line Line6 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   240
            Y1              =   2640
            Y2              =   2880
         End
         Begin VB.Line Line5 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   600
            Y1              =   2640
            Y2              =   2760
         End
         Begin VB.Line Line4 
            BorderColor     =   &H0000FFFF&
            X1              =   600
            X2              =   240
            Y1              =   2640
            Y2              =   2640
         End
         Begin VB.Line Line3 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   240
            Y1              =   2880
            Y2              =   3000
         End
         Begin VB.Line Line2 
            BorderColor     =   &H0000FFFF&
            X1              =   240
            X2              =   120
            Y1              =   3000
            Y2              =   3000
         End
         Begin VB.Line Line1 
            BorderColor     =   &H0000FFFF&
            X1              =   120
            X2              =   120
            Y1              =   1680
            Y2              =   3000
         End
         Begin VB.Line Line8 
            BorderColor     =   &H0000FFFF&
            X1              =   840
            X2              =   840
            Y1              =   240
            Y2              =   1440
         End
         Begin VB.Image Image7 
            Appearance      =   0  'Flat
            Height          =   384
            Left            =   372
            Top             =   948
            Visible         =   0   'False
            Width           =   384
         End
         Begin VB.Image Image6 
            Appearance      =   0  'Flat
            Height          =   384
            Left            =   960
            Top             =   840
            Visible         =   0   'False
            Width           =   384
         End
         Begin VB.Image Image5 
            Appearance      =   0  'Flat
            Height          =   384
            Left            =   840
            Top             =   240
            Visible         =   0   'False
            Width           =   384
         End
         Begin VB.Image Image8 
            Appearance      =   0  'Flat
            Height          =   384
            Left            =   240
            Top             =   360
            Visible         =   0   'False
            Width           =   384
         End
         Begin VB.Shape Shape2 
            BackColor       =   &H00C0C0C0&
            BorderColor     =   &H0000FFFF&
            FillColor       =   &H00C0C0C0&
            FillStyle       =   0  'Solid
            Height          =   1455
            Left            =   240
            Shape           =   3  'Circle
            Top             =   120
            Width           =   1215
         End
         Begin VB.Shape Shape3 
            BackColor       =   &H00808080&
            BorderColor     =   &H0000FFFF&
            FillColor       =   &H0000C0C0&
            FillStyle       =   4  'Upward Diagonal
            Height          =   1695
            Left            =   120
            Shape           =   3  'Circle
            Top             =   0
            Width           =   1455
         End
      End
      Begin VB.Label txtTotalHeight 
         Caption         =   "--"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   1560
         TabIndex        =   36
         Top             =   5592
         Width           =   1212
      End
      Begin VB.Label Label7 
         Alignment       =   1  'Right Justify
         Caption         =   "Total Height"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   480
         TabIndex        =   35
         Top             =   5592
         Width           =   972
      End
      Begin VB.Label Trigger 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         Caption         =   "Trigger"
         ForeColor       =   &H80000008&
         Height          =   252
         Left            =   2160
         TabIndex        =   34
         Top             =   6600
         Visible         =   0   'False
         Width           =   612
      End
      Begin VB.Label Label12 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Group Name"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   480
         TabIndex        =   33
         Top             =   5880
         Width           =   972
      End
      Begin VB.Label Label9 
         Alignment       =   1  'Right Justify
         Caption         =   "Step Thickness"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   240
         TabIndex        =   32
         Top             =   4680
         Width           =   1212
      End
      Begin VB.Label Label11 
         BackStyle       =   0  'Transparent
         Caption         =   "Item Names are: Step, Rise, Ceiling, CenterPost, Inner, Outer,Top, Base, OuterInt."
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   612
         Left            =   120
         TabIndex        =   31
         Top             =   6240
         Width           =   2652
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "Step Height"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   600
         TabIndex        =   30
         Top             =   4440
         Width           =   852
      End
      Begin VB.Label Label6 
         Alignment       =   1  'Right Justify
         Caption         =   "Outer Wall Radius"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   120
         TabIndex        =   29
         Top             =   4080
         Width           =   1332
      End
      Begin VB.Label Label5 
         Alignment       =   1  'Right Justify
         Caption         =   "Outer Stair Radius"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   120
         TabIndex        =   28
         Top             =   3840
         Width           =   1332
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "Inner Stair Radius"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   120
         TabIndex        =   27
         Top             =   3600
         Width           =   1332
      End
      Begin VB.Label Label2 
         Alignment       =   1  'Right Justify
         Caption         =   "Total steps"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   480
         TabIndex        =   26
         Top             =   5280
         Width           =   972
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         Caption         =   "Steps per full circle"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Left            =   0
         TabIndex        =   25
         Top             =   5040
         Width           =   1452
      End
   End
End
Attribute VB_Name = "frmParSolSpiralStair"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Build_Click()
    Dim SOuter, SInner, SHeight, SThickness, NumSteps As Integer
    Dim Cycle As Integer 'Number of steps per 360 Note:Must be large enough to walk through.
    Dim Group As String
    Dim Angle, AngleInc, NextAngle, StartAngle, HalfAngle
    Dim i, N As Integer
    Dim Temp As Double
    
    Dim V As Integer
    Static VC4(4) As Integer
    Static VC3(3) As Integer
    Dim Clockwise As Integer
    
    Dim Pi
    Dim CurrentX As Single
    Dim CurrentY As Single
    Dim CurrentZ As Single

    Dim NextZ As Single '
    Dim ZBase As Single
    Dim ZTop As Single
    Dim topcount As Integer
 
    Dim SideCounter As Integer
    Dim jend As Integer
    Dim Breakpoint As Integer

    Dim CSloped
    Dim CStepped
    Dim SSloped
    Dim SStepped
    Dim FillInt As Integer
    Dim FillExt As Integer
    Dim TotalHeight As Integer
    Dim OWRadius As Integer

    Dim NumSent As Integer

    Call InitBrush("SpiralStair")

    '
    ' Validate parameters
    '
    If Not Eval(txtOuterSRad, Temp) Then Exit Sub
    SOuter = Int(Temp)
    '
    If Not Eval(txtInnerSRad, Temp) Then Exit Sub
    SInner = Int(Temp)
    '
    If Not Eval(txtStepHeight, Temp) Then Exit Sub
    SHeight = Int(Temp)
    '
    If Not Eval(txtStepThickness, Temp) Then Exit Sub
    SThickness = Int(Temp)
    '
    If Not Eval(txtOuterWallRadius, Temp) Then Exit Sub
    OWRadius = Int(Temp)
    '
    If Not Eval(txtNumSteps, Temp) Then Exit Sub
    NumSteps = Int(Temp)
    '
    If Not Eval(txtCycleNum, Temp) Then Exit Sub
    Cycle = Int(Temp)
    '
    CalcTotalHeight_Click
    If Not Eval(txtTotalHeight, Temp) Then Exit Sub
    TotalHeight = Int(Temp)
    '
    Group = UCase$(txtGroup)
    '
    ' Options
    '
    CSloped = optCeilingSloped.Value
    CStepped = optCeilingStepped.Value
    SSloped = optSurfaceSloped.Value
    SStepped = optSurfaceStepped.Value
    FillInt = chkIntFill
    FillExt = chkExtFill
    Clockwise = chkClockwise
    
    MousePointer = 11

    If CSloped And (SThickness < SHeight) Then
        MsgBox "Step Thickness must be Equal to or Greater than the Step Height for Sloped Ceiling."
        MousePointer = 0
        Exit Sub
    End If
    If CStepped And SSloped Then
        MsgBox "You Cannot have a Sloped Step with a Stepped Ceiling."
        MousePointer = 0
        Exit Sub
    End If
    If SInner >= SOuter Then
        MsgBox "The InnerStair Radius must be Greater than the Outer Stair Radius."
        MousePointer = 0
        Exit Sub
    End If
    If FillExt And (SOuter >= OWRadius) Then
        MsgBox "The Outer Wall Radius must be Greater than the Outer Stair Radius."
        MousePointer = 0
        Exit Sub
    End If
    If (SOuter <= 0) Or (SInner <= 0) Or (SThickness <= 0) Or (SHeight <= 0) Or (OWRadius <= 0) Or (NumSteps <= 0) Then
        MsgBox "You must use Positive non-zero values."
        MousePointer = 0
        Exit Sub
    End If
    If (Cycle < 3) Then
        MsgBox "You must have at least 3 Steps per 360 degrees."
        MousePointer = 0
        Exit Sub
    End If
    If (CSloped) And (SHeight > SThickness) Then
        MsgBox "The Step Thickness must be Greater than the Step Height for Sloped Ceilings."
        MousePointer = 0
        Exit Sub
    End If
    If (Cycle * SHeight < 48) And (NumSteps >= Cycle) Then ' 3 ft.
        MsgBox "Normal People will BUMP their heads on the steps above!"
    End If
    
        
    
    ' Setup
    NumSent = 0
    N = 0
    Pi = 4 * Atn(1)
    If Clockwise = 1 Then
        AngleInc = 2 * Pi / Cycle
    Else
        AngleInc = -2 * Pi / Cycle
    End If

    StartAngle = AngleInc / 2
    Angle = StartAngle
    HalfAngle = AngleInc / 2
    
    'Adjust radius for side alignment
    If Me.chkAlignSide.Value = 1 Then '1 = checked 0 = Unchecked
        SInner = 1 / (Cos(HalfAngle) / SInner)
        SOuter = 1 / (Cos(HalfAngle) / SOuter)
        OWRadius = 1 / (Cos(HalfAngle) / OWRadius)
    End If
    
    
    If Clockwise = 1 Then
        VC4(1) = 1
        VC4(2) = 2
        VC4(3) = 3
        VC4(4) = 4

        VC3(1) = 1
        VC3(2) = 2
        VC3(3) = 3

    Else
        VC4(1) = 4
        VC4(2) = 3
        VC4(3) = 2
        VC4(4) = 1
        
        VC3(1) = 3
        VC3(2) = 2
        VC3(3) = 1

    End If


    CurrentX = 0
    CurrentY = 0
    CurrentZ = -(NumSteps * SHeight / 2)

    
    ' *****************************************************************************************
    ' Build Stepped Ceiling
    '
    If SStepped And CStepped Then
        Angle = StartAngle
        For i = 1 To NumSteps

            If (i Mod Cycle) <> 0 Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If

            'Build The Bottom Of the Step
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Step"
            Brush.NumPolys = N
            Brush.Polys(N).NumVertices = 4 'rectangles

            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(NextAngle) '
            CurrentY = SOuter * Sin(NextAngle) '
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(NextAngle) '
            CurrentY = SInner * Sin(NextAngle) '
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If
            
            CurrentZ = CurrentZ + SHeight
        Next i 'end outer

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
    
    
    End If


    '*******************************************************************************************
    ' Build Stepped Surface
    '
    If SStepped = True Then

        Angle = StartAngle
        CurrentZ = -(NumSteps * SHeight / 2)
        For i = 1 To NumSteps
            
            'Build The Top Of the Step
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Step"
            Brush.NumPolys = N
           Brush.Polys(N).NumVertices = 4 'rectangles

            If (i Mod Cycle) <> 0 Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If
            
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ + SThickness)
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
            CurrentX = SOuter * Cos(NextAngle) '
            CurrentY = SOuter * Sin(NextAngle) '
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)
            CurrentX = SInner * Cos(NextAngle) '
            CurrentY = SInner * Sin(NextAngle) '
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ + SThickness)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

            CurrentZ = CurrentZ + SHeight
        Next i 'end outer
    
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            
            NumSent = 1
            N = 0
        End If
    
    
    End If


    '****************************************************************************************
    ' Build Front and Back Stepped Toe Kicks
    '
    If ((SStepped) And (CStepped)) Then ' SStepped
        Angle = StartAngle
        CurrentZ = -(NumSteps * SHeight / 2)
        For i = 1 To NumSteps

            If (i Mod Cycle) <> 0 Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If

            If (i > 1) And (SHeight < SThickness) Then
                ZBase = CurrentZ + (SThickness - SHeight)
            Else
                ZBase = CurrentZ
            End If

            'Build The Front Toe Kick
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Rise"
            Brush.NumPolys = N
            Brush.Polys(N).NumVertices = 4 'rectangles

            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, ZBase)
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)
            CurrentX = SOuter * Cos(Angle) '
            CurrentY = SOuter * Sin(Angle) '
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
            CurrentX = SOuter * Cos(Angle) '
            CurrentY = SOuter * Sin(Angle) '
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, ZBase)


            If (i < NumSteps) And (SHeight < SThickness) Then
                ZTop = CurrentZ + (SHeight)
            Else
                ZTop = CurrentZ + SThickness
            End If
            
            'Build The Back Toe Kick
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Rise"
            Brush.NumPolys = N
            Brush.Polys(N).NumVertices = 4 'rectangles

            CurrentX = SInner * Cos(NextAngle)
            CurrentY = SInner * Sin(NextAngle)
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(NextAngle)
            CurrentY = SInner * Sin(NextAngle)
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, ZTop)
            CurrentX = SOuter * Cos(NextAngle) '
            CurrentY = SOuter * Sin(NextAngle) '
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, ZTop)
            CurrentX = SOuter * Cos(NextAngle) '
            CurrentY = SOuter * Sin(NextAngle) '
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If
            
            CurrentZ = CurrentZ + SHeight
        Next i 'Toe Kicks
    
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
    
    End If

'*************************************************************************************
' Build Sloped Toe Kicks
'
    If (SSloped = True) And (CSloped = True) Then
        Angle = StartAngle
        CurrentZ = -(NumSteps * SHeight / 2)
        For i = 1 To NumSteps

            If i = 1 Then
                'Build The Front Toe Kick
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Rise"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles

                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)

                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
                CurrentX = SOuter * Cos(Angle) '
                CurrentY = SOuter * Sin(Angle) '
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
            End If
            
            If i = NumSteps Then
                'Build The Last Toe Kick
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Rise"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles

                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
            
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)

                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)

                CurrentX = SOuter * Cos(Angle + AngleInc) '
                CurrentY = SOuter * Sin(Angle + AngleInc) '
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            End If

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

            CurrentZ = CurrentZ + SHeight

        Next i 'Toe Kicks
    End If

'***************************************************************************************
' Build Sloped Ceiling/ Stepped Surface Toe Kicks
'
    If (CSloped = True) And (SSloped = False) Then
        Angle = StartAngle
        CurrentZ = -(NumSteps * SHeight / 2)
        For i = 1 To NumSteps

            If (SThickness = SHeight) Or (i = 1) Then

                'Build The Front Toe Kicks Only
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Rise"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles

                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
                
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
                
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
            End If
            
            If (SThickness > SHeight) And (i > 1) Then

                'Build The Front Toe Kicks Only
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Rise"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles

                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ + (SThickness - SHeight))
                
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
                
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ + (SThickness - SHeight))
            End If
            
            If i = NumSteps Then
                'Build The Last Toe Kick
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Rise"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles
    
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
                
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
    
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)
                
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            End If

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

            CurrentZ = CurrentZ + SHeight

        Next i 'Toe Kicks
    End If

'***************************************************************************************

'*************************************************************************************
' Build Inside and OutSide Of Sloped Ceiling/Stepped Surface
'
    If (CSloped = True) And (SSloped = False) Then
        Angle = StartAngle
        CurrentZ = -(NumSteps * SHeight / 2)

        For i = 1 To NumSteps
        
            If (i < NumSteps) And (SHeight = SThickness) And (FillExt = False) Then
                'Build The Outside Slope
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Outer"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 3 'rectangles
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ + SHeight - SThickness)
                
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ + SHeight)
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ + SHeight)
            End If
            
            If (i < NumSteps) And (SThickness > SHeight) And (FillExt = False) Then
                'Build The Outside Slope
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Outer"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
                
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SHeight)
                
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SHeight + (SThickness - SHeight))
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ + SThickness)

            End If


            If (i = NumSteps) And (FillExt = False) Then
                'Build The Last Outside Slope
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Outer"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ + SThickness)
            End If

        
            If (i < NumSteps) And (SHeight = SThickness) And (FillInt = False) Then
                'Build The Inside Slope
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Inner"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 3 'rectangles
    
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ + SHeight - SThickness)
                
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ + SHeight)
    
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ + SHeight)
            End If
            
            If (i < NumSteps) And (SThickness > SHeight) And (FillInt = False) Then
                'Build The Outside Slope
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Inner"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles
    
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
                
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SHeight)
                
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SHeight + (SThickness - SHeight))
    
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ + SThickness)

            End If


            If (i = NumSteps) And (FillInt = False) Then
                'Build The Last Inside Slope
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Inner"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles
    
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ + SThickness)
            End If

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

            CurrentZ = CurrentZ + SHeight


        Next i
    End If

'***************************************************************************************
' Build Stepped Inside and Outside
'
    If (SStepped) And (CStepped) Then
        Angle = StartAngle
        CurrentZ = -(NumSteps * SHeight / 2)
        For i = 1 To NumSteps
    
            If (i Mod Cycle) <> 0 Then
                NextAngle = Angle + AngleInc
            Else
               NextAngle = StartAngle
            End If

            If FillInt = 0 Then
                'Build Inside
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Inner"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles
                
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(NextAngle)
                CurrentY = SInner * Sin(NextAngle)
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(NextAngle) '
                CurrentY = SInner * Sin(NextAngle) '
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ + SThickness)
                CurrentX = SInner * Cos(Angle) '
                CurrentY = SInner * Sin(Angle) '
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ + SThickness)
            End If

            If FillExt = 0 Then
                'Build Outside
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Outer"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 4 'rectangles

                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(NextAngle)
                CurrentY = SOuter * Sin(NextAngle)
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(NextAngle) '
                CurrentY = SOuter * Sin(NextAngle) '
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ + SThickness)
                CurrentX = SOuter * Cos(Angle) '
                CurrentY = SOuter * Sin(Angle) '
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ + SThickness)
           End If

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If
           
            CurrentZ = CurrentZ + SHeight
        Next i 'end outer
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            
            NumSent = 1
            N = 0
        End If
    
    End If ' Stepped

'********************************************************************************************
' Build Sloped Ceiling
'********************************************************************************************
    If (CSloped = True) Then
        Angle = StartAngle 'Angle + AngleInc' Offset the Start Position
        CurrentZ = -(NumSteps * SHeight / 2)
        For i = 1 To NumSteps

            'Build The Bottom Of the Step
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Ceiling"
            Brush.Polys(N).NumVertices = 3 'rectangles

            If i < NumSteps Then
                NextZ = CurrentZ + SHeight
            Else
               NextZ = CurrentZ
            End If


            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc)
            CurrentY = SOuter * Sin(Angle + AngleInc)
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) '
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, NextZ)


            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Ceiling"
            Brush.Polys(N).NumVertices = 3 'rectangles

            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc)
            CurrentY = SInner * Sin(Angle + AngleInc)
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ)

            CurrentX = SInner * Cos(Angle) '
            CurrentY = SInner * Sin(Angle) '
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)



            CurrentZ = CurrentZ + SHeight
            Angle = Angle + AngleInc

        Next i
    End If  ' Sloped Ceiling


'*******************************************************************************************
' Build Sloped Surface
'*******************************************************************************************
    If (SSloped = True) Then
    CurrentZ = -(NumSteps * SHeight / 2) + SThickness
    Angle = StartAngle
    For i = 1 To NumSteps

            'Build The Top Of the Step
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Step"
            Brush.Polys(N).NumVertices = 3 'rectangles

            If i < NumSteps Then
                NextZ = CurrentZ + SHeight
            Else
                NextZ = CurrentZ
            End If


            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc)
            CurrentY = SOuter * Sin(Angle + AngleInc)
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) '
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, NextZ)


            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Step"
            Brush.Polys(N).NumVertices = 3 'triangles

            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc)
            CurrentY = SInner * Sin(Angle + AngleInc)
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ)

            CurrentX = SInner * Cos(Angle) '
            CurrentY = SInner * Sin(Angle) '
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)



            CurrentZ = CurrentZ + SHeight
            Angle = Angle + AngleInc

    Next i
    End If  ' Sloped Surface




'*************************************************************************************
' Build Sloped Inside and Outside
'*************************************************************************************
    
    If (SSloped = True) And (CSloped = True) Then
        Angle = StartAngle
        CurrentZ = -(NumSteps * SHeight / 2)
        For i = 1 To NumSteps

            If i < NumSteps Then
                NextZ = CurrentZ + SHeight
            Else
                NextZ = CurrentZ
            End If

            If (FillInt = False) Then

                'Build Inside
                'First Inside
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Inner"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 3 'rectangles
            
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)
            
                CurrentX = SInner * Cos(Angle + AngleInc)
                CurrentY = SInner * Sin(Angle + AngleInc)
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ)
            
                CurrentX = SInner * Cos(Angle + AngleInc) '
                CurrentY = SInner * Sin(Angle + AngleInc) '
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, NextZ + SThickness)
            
            
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Inner"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 3 'rectangles
                
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)
                
                
                CurrentX = SInner * Cos(Angle + AngleInc) '
                CurrentY = SInner * Sin(Angle + AngleInc) '
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ + SThickness)
                
                CurrentX = SInner * Cos(Angle) '
                CurrentY = SInner * Sin(Angle) '
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ + SThickness)
                
            End If


            If (FillExt = False) Then
                'Build Outside
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Outer"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 3 'rectangles
                
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle + AngleInc)
                CurrentY = SOuter * Sin(Angle + AngleInc)
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ)
                CurrentX = SOuter * Cos(Angle + AngleInc) '
                CurrentY = SOuter * Sin(Angle + AngleInc) '
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, NextZ + SThickness)
            
                'Build Outside
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "Outer"
                Brush.NumPolys = N
                Brush.Polys(N).NumVertices = 3 'rectangles
            
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle + AngleInc) '
                CurrentY = SOuter * Sin(Angle + AngleInc) '
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, NextZ + SThickness)
                CurrentX = SOuter * Cos(Angle) '
                CurrentY = SOuter * Sin(Angle) '
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ + SThickness)

            End If
            
            Angle = Angle + AngleInc
            CurrentZ = CurrentZ + SHeight
        Next i 'end outer
            
    End If ' Sloped Sides



'*************************************************************************************************
' Interior Wall for Stepped Ceiling/ Stepped Surface
'
'*************************************************************************************************


   If (FillInt = 1) And (CStepped = True) Then
       '
        'Build Interior Wall
        '
        Angle = StartAngle + AngleInc 'Skip First step
        V = 0
        For i = 2 To NumSteps '

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "CenterPost"
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i <= Cycle Then
                ZBase = (-NumSteps * SHeight / 2)
            Else
                ZBase = (-NumSteps * SHeight / 2) + ((i - Cycle) * SHeight) - (SHeight - SThickness)
            End If

            ZTop = -(NumSteps * SHeight / 2) + (SHeight * (i - 1))

            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle) 'up to the base of the next step
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

        Next i 'end Under
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        topcount = Cycle - 1 'The top is always 1 less then the cycle

        For i = 1 To topcount
        ' Build the tops
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "CenterPost"
            Brush.Polys(N).NumVertices = 4 'rectangles
       
            ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
            
            ZBase = ZTop - ((topcount - i + 1) * SHeight)

            If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)

            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
       
            CurrentX = SInner * Cos(Angle) '
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
       
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
       
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
       
       
            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If
        
        Next i 'end Upper
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

   End If 'End FillInt




'**************************************************************************************************
'Build Exterior Wall for Stepped Ceiling/ Stepped Surface
'
'**************************************************************************************************

    If (FillExt = 1) And (CStepped = True) Then
        '
        'Build Exterior Wall
        '
        Angle = StartAngle + AngleInc 'Skip First step
        V = 0
        For i = 2 To NumSteps '

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OuterInt"
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i <= Cycle Then
                ZBase = (-NumSteps * SHeight / 2)
            Else
                ZBase = (-NumSteps * SHeight / 2) + ((i - Cycle) * SHeight) - (SHeight - SThickness)
            End If

            ZTop = -(NumSteps * SHeight / 2) + (SHeight * (i - 1))

            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle) 'up to the base of the next step
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

        Next i 'end Under

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        topcount = Cycle - 1 'The top is always 1 less then the cycle

        For i = 1 To topcount
        ' Build the tops
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OuterInt"
            Brush.Polys(N).NumVertices = 4 'rectangles
       
            ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
            
            ZBase = ZTop - ((topcount - i + 1) * SHeight)

            If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)

            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
       
            CurrentX = SOuter * Cos(Angle) '
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
       
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
       
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
       
            Angle = Angle + AngleInc
       
        
        Next i 'end Inner

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

    End If 'FillExt



'************************************************************************************************
'
'                     Fill inside for Sloped Ceiling/Stepped Surface
'
'************************************************************************************************
   
    If (FillInt = 1) And (CSloped = True) And (SSloped = False) Then
        '
        'Build Interior Wall
        '
        Angle = StartAngle '
        V = 0

            '
            ' Build the first Under with only 3 sides
            '
            ZBase = (-NumSteps * SHeight / 2)
            ZTop = (-NumSteps * SHeight / 2) + SHeight

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "CenterPost"
            Brush.Polys(N).NumVertices = 3 'rectangles
            
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc




        For i = 2 To NumSteps '

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "CenterPost"
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i <= Cycle Then
                ZBase = (-NumSteps * SHeight / 2)
            Else
                ZBase = (-NumSteps * SHeight / 2) + ((i - Cycle) * SHeight) - (SHeight - SThickness)
            End If

            ZTop = -(NumSteps * SHeight / 2) + (SHeight * (i - 1))

            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle) 'up to the base of the next step
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            If i < NumSteps Then
                CurrentZ = ZTop + SHeight '
            Else
                CurrentZ = ZTop
            End If
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

        Next i 'end Under
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        topcount = Cycle - 1 'The top is always 1 less then the cycle

        For i = 1 To topcount
        ' Build the tops
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "CenterPost"
            Brush.Polys(N).NumVertices = 4 'rectangles
       
            ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
            ZBase = ZTop - ((topcount - i + 1) * SHeight)
            If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)

            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle) '
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
       
            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If
        Next i 'end Upper
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

    End If


'************************************************************************************************
'
'                     Fill inside for Sloped Ceiling/Sloped Surface
'
'************************************************************************************************
   
    If (FillInt = 1) And (SSloped) And (CSloped) Then
    
    
    
        '
        'Build Interior Wall
        '
        Angle = StartAngle '
        V = 0

            '
            ' Build the first Under with only 3 sides
            '
            ZBase = (-NumSteps * SHeight / 2)
            ZTop = (-NumSteps * SHeight / 2) + SHeight

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "CenterPost"
            Brush.Polys(N).NumVertices = 3 'rectangles
            
            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ)
            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc




        For i = 2 To NumSteps '

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "CenterPost"
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i <= Cycle Then
                ZBase = (-NumSteps * SHeight / 2)
            Else
                ZBase = (-NumSteps * SHeight / 2) + ((i - Cycle) * SHeight) - (SHeight - SThickness)
            End If

            ZTop = -(NumSteps * SHeight / 2) + (SHeight * (i - 1))

            CurrentX = SInner * Cos(Angle)
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle) 'up to the base of the next step
            CurrentY = SInner * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            If i < NumSteps Then
                CurrentZ = ZTop + SHeight '
            Else
                CurrentZ = ZTop
            End If
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = SInner * Cos(Angle + AngleInc) '
            CurrentY = SInner * Sin(Angle + AngleInc) 'Over
            If i <= Cycle Then
                CurrentZ = ZBase
            Else
                CurrentZ = ZBase + SHeight
            End If
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

        Next i 'end Under
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        topcount = Cycle - 1 'The top is always 1 less then the cycle

        For i = 1 To topcount

            If i < topcount Then

                ' Build the tops
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "CenterPost"
                Brush.Polys(N).NumVertices = 4 'rectangles
        
                ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
                ZBase = ZTop - ((topcount - i + 1) * SHeight)
                If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)
    
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                CurrentZ = ZBase
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(Angle) '
                CurrentY = SInner * Sin(Angle)
                CurrentZ = ZTop
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(Angle + AngleInc) '
                CurrentY = SInner * Sin(Angle + AngleInc) 'Over
                CurrentZ = ZTop
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(Angle + AngleInc) '
                CurrentY = SInner * Sin(Angle + AngleInc) 'Down
                CurrentZ = ZBase + SHeight
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
            End If

            If i = topcount Then 'do the last one
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "CenterPost"
                Brush.Polys(N).NumVertices = 3 'rectangles
        
                ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
                ZBase = ZTop - ((topcount - i + 1) * SHeight)
                If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)
    
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)
                CurrentZ = ZBase
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(Angle) '
                CurrentY = SInner * Sin(Angle)
                CurrentZ = ZTop
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ)
                CurrentX = SInner * Cos(Angle + AngleInc) '
                CurrentY = SInner * Sin(Angle + AngleInc) 'Over
                CurrentZ = ZTop
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)

            End If

       
            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If
        Next i 'end Upper
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
            
    End If


'************************************************************************************************
'
'                     Fill OutSide for Sloped Ceiling/Stepped Surface
'
'************************************************************************************************
   
    If (FillExt = 1) And (CSloped = True) And (SSloped = False) Then
        '
        'Build Interior Wall
        '
        Angle = StartAngle '
        V = 0

            '
            ' Build the first Under with only 3 sides
            '
            ZBase = (-NumSteps * SHeight / 2)
            ZTop = (-NumSteps * SHeight / 2) + SHeight

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OuterInt"
            Brush.Polys(N).NumVertices = 3 'rectangles
            
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc




        For i = 2 To NumSteps '

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OuterInt"
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i <= Cycle Then
                ZBase = (-NumSteps * SHeight / 2)
            Else
                ZBase = (-NumSteps * SHeight / 2) + ((i - Cycle) * SHeight) - (SHeight - SThickness)
            End If

            ZTop = -(NumSteps * SHeight / 2) + (SHeight * (i - 1))

            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle) 'up to the base of the next step
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            If i < NumSteps Then
                CurrentZ = ZTop + SHeight '
            Else
                CurrentZ = ZTop
            End If
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

        Next i 'end Under
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        topcount = Cycle - 1 'The top is always 1 less then the cycle

        For i = 1 To topcount
        ' Build the tops
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OuterInt"
            Brush.Polys(N).NumVertices = 4 'rectangles
       
            ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
            ZBase = ZTop - ((topcount - i + 1) * SHeight)
            If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)

            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle) '
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
       
            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If
        Next i 'end Upper
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

    End If


'************************************************************************************************
'
'                     Fill Outside for Sloped Ceiling/Sloped Surface
'
'************************************************************************************************
   
    If (FillExt = 1) And (SSloped) And (CSloped) Then
    
    
    
        '
        'Build Exterior Wall
        '
        Angle = StartAngle '
        V = 0

            '
            ' Build the first Under with only 3 sides
            '
            ZBase = (-NumSteps * SHeight / 2)
            ZTop = (-NumSteps * SHeight / 2) + SHeight

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OuterInt"
            Brush.Polys(N).NumVertices = 3 'rectangles
            
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            CurrentZ = ZTop
            Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ)
            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Down
            CurrentZ = ZBase
            Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc




        For i = 2 To NumSteps '

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "OuterInt"
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i <= Cycle Then
                ZBase = (-NumSteps * SHeight / 2)
            Else
                ZBase = (-NumSteps * SHeight / 2) + ((i - Cycle) * SHeight) - (SHeight - SThickness)
            End If

            ZTop = -(NumSteps * SHeight / 2) + (SHeight * (i - 1))

            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZBase
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle) 'up to the base of the next step
            CurrentY = SOuter * Sin(Angle)
            CurrentZ = ZTop
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            If i < NumSteps Then
                CurrentZ = ZTop + SHeight '
            Else
                CurrentZ = ZTop
            End If
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(Angle + AngleInc) '
            CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
            If i <= Cycle Then
                CurrentZ = ZBase
            Else
                CurrentZ = ZBase + SHeight
            End If
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

        Next i 'end Under
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        topcount = Cycle - 1 'The top is always 1 less then the cycle

        For i = 1 To topcount

            If i < topcount Then

                ' Build the tops
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "OuterInt"
                Brush.Polys(N).NumVertices = 4 'rectangles
        
                ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
                ZBase = ZTop - ((topcount - i + 1) * SHeight)
                If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                CurrentZ = ZBase
                Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle) '
                CurrentY = SOuter * Sin(Angle)
                CurrentZ = ZTop
                Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle + AngleInc) '
                CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
                CurrentZ = ZTop
                Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle + AngleInc) '
                CurrentY = SOuter * Sin(Angle + AngleInc) 'Down
                CurrentZ = ZBase + SHeight
                Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            End If

            If i = topcount Then 'do the last one
                N = N + 1
                InitBrushPoly (N)
                Brush.Polys(N).Group = Group
                Brush.Polys(N).Item = "OuterInt"
                Brush.Polys(N).NumVertices = 3 'rectangles
        
                ZTop = (NumSteps * SHeight / 2) - (SHeight - SThickness)
                ZBase = ZTop - ((topcount - i + 1) * SHeight)
                If ZBase < (-NumSteps * SHeight / 2) Then ZBase = (-NumSteps * SHeight / 2)
    
                CurrentX = SOuter * Cos(Angle)
                CurrentY = SOuter * Sin(Angle)
                CurrentZ = ZBase
                Call PutVertex(N, VC3(1), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle) '
                CurrentY = SOuter * Sin(Angle)
                CurrentZ = ZTop
                Call PutVertex(N, VC3(2), CurrentX, CurrentY, CurrentZ)
                CurrentX = SOuter * Cos(Angle + AngleInc) '
                CurrentY = SOuter * Sin(Angle + AngleInc) 'Over
                CurrentZ = ZTop
                Call PutVertex(N, VC3(3), CurrentX, CurrentY, CurrentZ)

            End If

            If (i Mod Cycle) <> 0 Then
                Angle = Angle + AngleInc
            Else
                Angle = StartAngle
            End If

        Next i 'end Upper
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

    
    End If










'*************************************************************************************************
'
' Add the Interior Top And Bottom Caps
'
'*************************************************************************************************

    If FillInt = 1 Then

        '
        ' The loop: Build the Bottom
        '
        CurrentZ = (-NumSteps * SHeight / 2)
        If Cycle > 12 Then
            Angle = StartAngle
            SideCounter = Cycle
            Breakpoint = 6
            Do While SideCounter > 0

                N = N + 1               ' Init a new Polygon
                InitBrushPoly (N)       '
                Brush.Polys(N).Group = Group  '
                Brush.Polys(N).Item = "Base"  '
                Brush.NumPolys = N       '

                If SideCounter >= Breakpoint Then
                    jend = Breakpoint '
                Else
                    jend = SideCounter + 1 '
                End If

                If Clockwise = 1 Then
                    V = jend + 1
                Else
                    V = 1
                End If

                Brush.Polys(N).NumVertices = jend + 1 'v
                For i = 1 To jend
                    CurrentX = SInner * Cos(Angle)
                    CurrentY = SInner * Sin(Angle)
                    Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)

                    If Clockwise = 1 Then
                        V = V - 1
                    Else
                        V = V + 1
                    End If

                    If i < jend Then Angle = Angle + AngleInc
                    If i < jend Then SideCounter = SideCounter - 1
                Next i

                CurrentX = 0
                CurrentY = 0
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
            Loop
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

            CurrentZ = (NumSteps * SHeight / 2) - (SHeight - SThickness) ' ouch
            SideCounter = Cycle
            Angle = StartAngle
            Breakpoint = 6
            Do While SideCounter > 0
            
                N = N + 1               ' Init a new Polygon
                InitBrushPoly (N)       '
                Brush.Polys(N).Group = Group  '
                Brush.Polys(N).Item = "Top"  '
                Brush.NumPolys = N       '
            
                If SideCounter >= Breakpoint Then
                    jend = Breakpoint '
                Else
                    jend = SideCounter + 1 '
                End If
            
                If Clockwise = 1 Then '
                    V = 1
                Else
                    V = jend + 1
                End If
                
                Brush.Polys(N).NumVertices = jend + 1
                For i = 1 To jend
                    CurrentX = SInner * Cos(Angle)
                    CurrentY = SInner * Sin(Angle)
                    Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
            
                    If Clockwise = 1 Then
                        V = V + 1
                    Else
                        V = V - 1
                    End If
            
                    If i <> jend Then Angle = Angle + AngleInc
                    If i <> jend Then SideCounter = SideCounter - 1
                Next i
            
                CurrentX = 0
                CurrentY = 0
                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
            Loop

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        End If ' End Solid and  > 12 sides

        If Cycle <= 12 Then
            '
            ' The loop: Build the Bottom
            '
            CurrentZ = (-NumSteps * SHeight / 2)
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Base"
            Brush.NumPolys = N
            Brush.Polys(N).NumVertices = Cycle
            
            If Clockwise = 1 Then
                V = Cycle + 1
            Else
                V = 0
            End If

            Angle = StartAngle
            For i = 1 To Cycle
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)

                If Clockwise = 1 Then
                    V = V - 1
                Else
                    V = V + 1
                End If


                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                
                If i < Cycle Then
                    Angle = Angle + AngleInc
                Else
                    Angle = StartAngle
                End If

            Next i
            
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If


            '
            ' The loop: Build the Top
            '
            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Top"
            Brush.NumPolys = N
            Brush.Polys(N).NumVertices = Cycle
            Angle = StartAngle
            CurrentZ = (NumSteps * SHeight / 2) - (SHeight - SThickness)

            If Clockwise = 1 Then
                V = 0
            Else
                V = Cycle + 1
            End If

            For i = 1 To Cycle
                CurrentX = SInner * Cos(Angle)
                CurrentY = SInner * Sin(Angle)

                If Clockwise = 1 Then
                    V = V + 1
                Else
                    V = V - 1
                End If

                Call PutVertex(N, V, CurrentX, CurrentY, CurrentZ)
                
                If i < Cycle Then
                    Angle = Angle + AngleInc
                Else
                    Angle = StartAngle
                End If

            Next i

        End If 'End Solid and < 12 sides
        
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

    End If ' Build Top And Bottom

'*********************************************************************************************
' Build Common Exterior Wall
'
'*********************************************************************************************

    If (FillExt = 1) Then

        Angle = StartAngle
        V = 0
        For i = 1 To Cycle

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Outer"
            Brush.NumPolys = Cycle
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i < Cycle Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If
            
            CurrentX = OWRadius * Cos(Angle)
            CurrentY = OWRadius * Sin(Angle)
            CurrentZ = (-NumSteps * SHeight / 2)
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            
            CurrentZ = (NumSteps * SHeight / 2) - (SHeight - SThickness)
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = OWRadius * Cos(NextAngle) '
            CurrentY = OWRadius * Sin(NextAngle) 'Over
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = OWRadius * Cos(NextAngle) '
            CurrentY = OWRadius * Sin(NextAngle) 'Down
            CurrentZ = (-NumSteps * SHeight / 2)
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc

        Next i 'end outer
    
        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If
       
    End If
       

'*************************************************************************************************
'
' Add the Exterior Top And Bottom Caps
'
'*************************************************************************************************

    If (FillExt = 1) Then

        '*************************************************
        ' Build the Hollow bottom
        Angle = StartAngle
        V = 0
        CurrentZ = (-NumSteps * SHeight / 2)
        For i = 1 To Cycle

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Base"
            Brush.NumPolys = Cycle
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i < Cycle Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If
            
            CurrentX = OWRadius * Cos(Angle)
            CurrentY = OWRadius * Sin(Angle)
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)
            
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(NextAngle) '
            CurrentY = SOuter * Sin(NextAngle) 'Over
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = OWRadius * Cos(NextAngle) '
            CurrentY = OWRadius * Sin(NextAngle) 'Down
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc

        Next i 'end bottom

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            NumSent = 1
            N = 0
        End If

        '**********************************
        ' Build the Hollow Top
        Angle = StartAngle
        V = 0
        CurrentZ = (NumSteps * SHeight / 2) - (SHeight - SThickness)
        For i = 1 To Cycle

            N = N + 1
            InitBrushPoly (N)
            Brush.Polys(N).Group = Group
            Brush.Polys(N).Item = "Top"
            Brush.NumPolys = Cycle
            Brush.Polys(N).NumVertices = 4 'rectangles

            If i < Cycle Then
                NextAngle = Angle + AngleInc
            Else
                NextAngle = StartAngle
            End If
            
            CurrentX = OWRadius * Cos(Angle)
            CurrentY = OWRadius * Sin(Angle)
            Call PutVertex(N, VC4(4), CurrentX, CurrentY, CurrentZ)
            
            CurrentX = SOuter * Cos(Angle)
            CurrentY = SOuter * Sin(Angle)
            Call PutVertex(N, VC4(3), CurrentX, CurrentY, CurrentZ)

            CurrentX = SOuter * Cos(NextAngle) '
            CurrentY = SOuter * Sin(NextAngle) 'Over
            Call PutVertex(N, VC4(2), CurrentX, CurrentY, CurrentZ)

            CurrentX = OWRadius * Cos(NextAngle) '
            CurrentY = OWRadius * Sin(NextAngle) 'Down
            Call PutVertex(N, VC4(1), CurrentX, CurrentY, CurrentZ)

            Angle = Angle + AngleInc

        Next i 'end Top

        If N > 100 Then
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If
            
            NumSent = 1
            N = 0
        End If


    End If













'*************************************************************************************************
' Send the solid
'
       For i = 1 To N
            'Debug.Print "Solid:"; i
            'Debug.Print
            For V = 1 To Brush.Polys(i).NumVertices
                'Debug.Print "X:"; Brush.Polys(i).Vertex(v).X, "Y:"; Brush.Polys(i).Vertex(v).Y, "Z:"; Brush.Polys(i).Vertex(v).Z
            Next V
            'Debug.Print "_________________________________________________________________________"
            'Debug.Print
        Next i

            MousePointer = 0
            Brush.NumPolys = N
            If NumSent > 0 Then
                Call SendBrush(1)
            Else
                Call SendBrush(0)
            End If



'    Brush.NumPolys = n
'    SendBrush (0)
    Call Ed.StatusText("Built a Spiral Stair")
 

End Sub



Private Sub CalcTotalHeight_Click()
    Dim NumSteps As Single
    Dim StepHeight As Single
    Dim StepThickness As Single
    Dim Temp As Double
    '
    txtTotalHeight.Caption = "--"
    '
    If Not Eval(txtNumSteps, Temp) Then Exit Sub
    NumSteps = Temp
    '
    If Not Eval(txtStepHeight, Temp) Then Exit Sub
    StepHeight = Temp
    '
    If Not Eval(txtStepThickness, Temp) Then Exit Sub
    StepThickness = Temp
    '
    txtTotalHeight.Caption = Str(((NumSteps * StepHeight / 2) - (StepHeight - StepThickness)) + (NumSteps * StepHeight / 2))
    '
End Sub

Private Sub Check1_Click()

End Sub

Private Sub chkClockwise_Click()
    If chkClockwise.Value = 1 Then
        Image1.Visible = True
        Image2.Visible = True
        Image3.Visible = True
        Image4.Visible = True

        Image5.Visible = False
        Image6.Visible = False
        Image7.Visible = False
        Image8.Visible = False

    Else
        Image5.Visible = True
        Image6.Visible = True
        Image7.Visible = True
        Image8.Visible = True

        Image1.Visible = False
        Image2.Visible = False
        Image3.Visible = False
        Image4.Visible = False
    
    End If


End Sub

Private Sub chkExtFill_Click()
    DrawLines
End Sub

Private Sub chkIntFill_Click()
    DrawLines
End Sub

Private Sub Command2_Click()
    Hide
End Sub


Private Sub DrawLines()
    '
    Dim Hidden, ShowIt, Showcr As Long
    '
    Hidden = &H808080
    ShowIt = &HC0C0&
    Showcr = &H80FF&
    '
    If (chkIntFill) Then
        Shape1.FillStyle = 5
        Line15.Visible = True
        Line19.Visible = True
        Line18.Visible = True
        Line11.Visible = True
        Line26.Visible = True
        Line20.Visible = True
        '
        Line16.Visible = False
        Line31.Visible = False
        Line33.Visible = False
        Line34.Visible = False
    Else
        Shape1.FillStyle = 1
        Line15.Visible = False
        Line19.Visible = False
        Line18.Visible = False
        Line11.Visible = False
        Line26.Visible = False
        Line20.Visible = False
        '
        Line16.Visible = True
        Line31.Visible = True
        Line33.Visible = True
        Line34.Visible = True

    End If

    If (chkExtFill) Then
        Shape3.FillStyle = 4
        Shape3.BorderStyle = 1
        
        Line35.Visible = False
        Line36.Visible = False
        Line37.Visible = False
        Line38.Visible = False

        Line39.Visible = True
        Line40.Visible = True
        Line1.Visible = True
        Line2.Visible = True
        Line23.Visible = True
        Line24.Visible = True
        Line14.Visible = True
        Line12.Visible = True
        Line6.Visible = True
        Line3.Visible = True
        Line22.Visible = True
        Line32.Visible = True
        Line28.Visible = True

    Else
        Shape3.FillStyle = 1
        Shape3.BorderStyle = 0

        Line35.Visible = True
        Line36.Visible = True
        Line37.Visible = True
        Line38.Visible = True
        Line39.Visible = False
        Line40.Visible = False
        Line1.Visible = False
        Line2.Visible = False
        Line23.Visible = False
        Line24.Visible = False
        Line14.Visible = False
        Line12.Visible = False
        Line6.Visible = False
        Line3.Visible = False
        Line22.Visible = False
        Line32.Visible = False
        Line28.Visible = False


    End If

    If (optCeilingSloped = True) Then
        Line9.Visible = True
        Line5.Visible = True
        Line30.Visible = True
    Else
        Line9.Visible = False
        Line5.Visible = False
        Line30.Visible = False
    End If
    
    If (optSurfaceSloped = True) Then
        Line41.Visible = True
        Line42.Visible = True
        Line43.Visible = True
    Else
        Line41.Visible = False
        Line42.Visible = False
        Line43.Visible = False
    End If

    If (optCeilingSloped = True) And (chkIntFill = False) Then
        Line46.Visible = True
        Line49.Visible = True
        Line52.Visible = True
        Line45.Visible = True
        Line51.Visible = True
        Line48.Visible = True
        If optSurfaceSloped = False Then
            Line54.Visible = True
        Else
            Line54.Visible = False
        End If
    Else
        Line46.Visible = False
        Line49.Visible = False
        Line52.Visible = False
        Line54.Visible = False
        Line45.Visible = False
        Line51.Visible = False
        Line48.Visible = False

    End If
    
    If (optSurfaceSloped = True) And (chkIntFill = False) Then
        Line44.Visible = True
        Line45.Visible = True
        Line47.Visible = True
        Line48.Visible = True
        Line50.Visible = True
        Line51.Visible = True
        If optCeilingSloped = False Then
            Line53.Visible = True
        Else
            Line53.Visible = False
        End If
    Else
        Line44.Visible = False
        Line45.Visible = False
        Line47.Visible = False
        Line48.Visible = False
        Line50.Visible = False
        Line51.Visible = False
        Line53.Visible = False
    End If



End Sub

Private Sub Form_Load()
    Call Ed.SetOnTop(Me, "BuildSpiralStair", TOP_NORMAL)
    DrawLines
    CalcTotalHeight_Click
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call Ed.EndOnTop(Me)
End Sub

Private Sub Help_Click()
    ToolHelp (155)
End Sub

Private Sub optCeilingSloped_Click()
    DrawLines
End Sub

Private Sub optCeilingStepped_Click()
    DrawLines
End Sub

Private Sub optSurfaceSloped_Click()
    DrawLines
End Sub

Private Sub optSurfaceStepped_Click()
    DrawLines
End Sub

Private Sub Trigger_Change()
    Build_Click
End Sub

'
' Focus change highlighting routines.
'
Private Sub txtInnerSRad_GotFocus()
    SelectAll txtInnerSRad
End Sub

Private Sub txtOuterSRad_GotFocus()
    SelectAll txtOuterSRad
End Sub

Private Sub txtOuterWallRadius_GotFocus()
    SelectAll txtOuterWallRadius
End Sub

Private Sub txtStepHeight_GotFocus()
    SelectAll txtStepHeight
End Sub

Private Sub txtStepThickness_GotFocus()
    SelectAll txtStepThickness
End Sub

Private Sub txtCycleNum_GotFocus()
    SelectAll txtCycleNum
End Sub

Private Sub txtNumSteps_GotFocus()
    SelectAll txtNumSteps
End Sub

Private Sub txtGroup_GotFocus()
    SelectAll txtGroup
End Sub
