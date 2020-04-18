VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.1#0"; "comdlg32.ocx"
Begin VB.Form frmDialogs 
   AutoRedraw      =   -1  'True
   BorderStyle     =   0  'None
   Caption         =   "Common dialogs used throughout"
   ClientHeight    =   6855
   ClientLeft      =   2340
   ClientTop       =   7830
   ClientWidth     =   10275
   Enabled         =   0   'False
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H80000008&
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6855
   ScaleWidth      =   10275
   ShowInTaskbar   =   0   'False
   Begin MSComDlg.CommonDialog MusicLoadDlg 
      Left            =   5340
      Top             =   4320
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "uax"
      DialogTitle     =   "Load Music"
      Filter          =   "Music resources (*.umx)|*.umx|All Files (*.*)|*.*"
      Flags           =   524800
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog MusicSaveDlg 
      Left            =   360
      Top             =   3360
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "uax"
      DialogTitle     =   "Save Music"
      Filter          =   "Unreal music (*.umx)|*.umx|All Files (*.*)|*.*"
      Flags           =   33280
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog MusicExportDlg 
      Left            =   8160
      Top             =   4500
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "ufx"
      DialogTitle     =   "Export A Song"
      Filter          =   "Song (*.s3m)|*.s3m"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog MusicImportDlg 
      Left            =   360
      Top             =   4320
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "ufx"
      DialogTitle     =   "Import music"
      Filter          =   $"Dialogs.frx":0000
      Flags           =   2097664
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog SoundLoadDlg 
      Left            =   6000
      Top             =   3300
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "uax"
      DialogTitle     =   "Load Sound Package"
      Filter          =   "Unreal sound packages (*.uax)|*.uax|All Files (*.*)|*.*"
      Flags           =   524800
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog SoundSaveDlg 
      Left            =   4140
      Top             =   3300
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "uax"
      DialogTitle     =   "Save Sound Package"
      Filter          =   "Unreal sound package (*.uax)|*.uax"
      Flags           =   33280
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog SoundExportDlg 
      Left            =   1560
      Top             =   3360
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "ufx"
      DialogTitle     =   "Export A Sound"
      Filter          =   "Wave file (*.wav)|*.wav"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog SoundImportDlg 
      Left            =   1560
      Top             =   4320
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "ufx"
      DialogTitle     =   "Import sound"
      Filter          =   "Wave files (*.wav)|*.wav|All Files (*.*)|*.*"
      Flags           =   2097664
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog TwoDeeTexture 
      Left            =   2820
      Top             =   2100
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "bmp"
      DialogTitle     =   "Load Texture in Background"
      Filter          =   "BMP files (*.bmp)|*.bmp|All Files (*.*)|*.*"
   End
   Begin MSComDlg.CommonDialog TwoDeeOpen 
      Left            =   420
      Top             =   2100
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "u2d"
      DialogTitle     =   "Open 2D Shape"
      Filter          =   "2D Shapes (*.u2d)|*.u2d|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog TwoDeeSave 
      Left            =   1620
      Top             =   2040
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "u2d"
      DialogTitle     =   "Save 2D Shape"
      Filter          =   "2D Shapes (*.u2d)|*.u2d|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog ClassLoad 
      Left            =   3600
      Top             =   120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "ucx"
      DialogTitle     =   "Load Actor Class File"
      Filter          =   "UnrealScript Classes (*.u;*.uc)|*.u;*.uc|All Files (*.*)|*.*"
      Flags           =   524800
   End
   Begin MSComDlg.CommonDialog ClassSave 
      Left            =   4560
      Top             =   120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "ucx"
      DialogTitle     =   "Save Actor Class File"
      Filter          =   "Unreal Actor Class (*.u)|*.u|Actor class text file (*.uc)|*.uc|C++ header (*.h)|*.h"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog ExportTex 
      Left            =   8820
      Top             =   120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "pcx"
      DialogTitle     =   "Export A Texture"
      Filter          =   "Standard PCX file (*.pcx)|*.pcx"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog AddFile 
      Left            =   4440
      Top             =   2100
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "unr"
      DialogTitle     =   "Load an UnrealEd file at startup"
      Flags           =   2129920
   End
   Begin MSComDlg.CommonDialog ToolHelp 
      Left            =   9360
      Top             =   2100
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      HelpCommand     =   1
      HelpContext     =   105
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin MSComDlg.CommonDialog RelNotes 
      Left            =   6300
      Top             =   2100
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      HelpCommand     =   1
      HelpContext     =   105
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin MSComDlg.CommonDialog About 
      Left            =   8460
      Top             =   2100
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      HelpCommand     =   1
      HelpContext     =   104
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin MSComDlg.CommonDialog HelpContents 
      Left            =   7500
      Top             =   2100
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      HelpCommand     =   11
      HelpFile        =   "help\unrealed.hlp"
   End
   Begin MSComDlg.CommonDialog ImportMap 
      Left            =   2400
      Top             =   1140
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Import Map"
      Filter          =   "Unreal Text (*.t3d)|*.t3d|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog ExportMap 
      Left            =   1440
      Top             =   1140
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Export Map"
      Filter          =   "Unreal Text (*.t3d)|*.t3d|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog TexFamLoad 
      Left            =   5520
      Top             =   120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "utx"
      DialogTitle     =   "Load Texture Package"
      Filter          =   "Unreal Textures (*.utx)|*.utx|All Files (*.*)|*.*"
      Flags           =   524800
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog TexFamSave 
      Left            =   6600
      Top             =   120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "utx"
      DialogTitle     =   "Save Texture Package"
      Filter          =   "Unreal Textures (*.utx)|*.utx|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog TexImport 
      Left            =   7740
      Top             =   120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "pcx"
      DialogTitle     =   "Import A Texture"
      Filter          =   "All graphics files (*.pcx,*.bmp)|*.pcx;*.bmp|PCX files (*.pcx)|*.pcx|BMP files (*.bmp)|*.bmp|All files (*.*)|*.*"
      Flags           =   2097664
      MaxFileSize     =   5000
   End
   Begin MSComDlg.CommonDialog BrushSave 
      Left            =   8220
      Top             =   1200
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "u3d"
      DialogTitle     =   "Save Brush"
      Filter          =   "3D Solids (*.u3d)|*.u3d|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog BrushOpen 
      Left            =   120
      Top             =   1200
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "u3d"
      DialogTitle     =   "Open Brush"
      Filter          =   "3D Solids (*.u3d)|*.u3d|All Files (*.*)|*.*"
      Flags           =   655360
   End
   Begin MSComDlg.CommonDialog ExportBrush 
      Left            =   240
      Top             =   180
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Export A Brush"
      Filter          =   "Unreal Text (*.t3d)|*.t3d|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog ImportBrush 
      Left            =   1440
      Top             =   120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "t3d"
      DialogTitle     =   "Import A Brush"
      Filter          =   "Importable (*.t3d; *.dxf; *.asc)|*.t3d;*.dxf;*.asc|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog MapSaveAs 
      Left            =   7020
      Top             =   1200
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "unr"
      DialogTitle     =   "Save Map As"
      Filter          =   "Unreal Maps (*.unr)|*.unr|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog MapOpen 
      Left            =   5880
      Top             =   1200
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "unr"
      DialogTitle     =   "Open Map File"
      Filter          =   "Unreal Maps (*.unr)|*.unr|All Files (*.*)|*.*"
      Flags           =   2129920
   End
   Begin MSComDlg.CommonDialog FHmapOpen 
      Left            =   2640
      Top             =   5640
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "pcx"
      DialogTitle     =   "Open Terra Height Map"
      Filter          =   "PCX Files (*.pcx)|*.pcx|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin MSComDlg.CommonDialog FloorSave 
      Left            =   1560
      Top             =   5640
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "tra"
      DialogTitle     =   "Open Terra File"
      Filter          =   "Terra Files (*.tra)|*.tra|All Files (*.*)|*.*"
      Flags           =   2097154
   End
   Begin MSComDlg.CommonDialog FloorOpen 
      Left            =   360
      Top             =   5640
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   327681
      CancelError     =   -1  'True
      DefaultExt      =   "tra"
      DialogTitle     =   "Open Terra File"
      Filter          =   "Terra Files (*.tra)|*.tra|All Files (*.*)|*.*"
      Flags           =   2097152
   End
   Begin VB.Label Label23 
      Alignment       =   2  'Center
      Caption         =   "FloorOpen"
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
      TabIndex        =   32
      Top             =   6240
      Width           =   1095
   End
   Begin VB.Label Label18 
      Alignment       =   2  'Center
      Caption         =   "FloorSave"
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
      Left            =   1320
      TabIndex        =   31
      Top             =   6240
      Width           =   1095
   End
   Begin VB.Label Label17 
      Caption         =   "FHmapOpen"
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
      Left            =   2400
      TabIndex        =   30
      Top             =   6240
      Width           =   1095
   End
   Begin VB.Label Label33 
      Caption         =   "MusicLoadDlg"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   5040
      TabIndex        =   29
      Top             =   4860
      Width           =   1155
   End
   Begin VB.Label Label32 
      Caption         =   "MusicSaveDlg"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   0
      TabIndex        =   28
      Top             =   3840
      Width           =   1155
   End
   Begin VB.Label Label31 
      Caption         =   "MusicExportDlg"
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
      Left            =   7860
      TabIndex        =   27
      Top             =   5040
      Width           =   1335
   End
   Begin VB.Label Label25 
      Caption         =   "MusicImportDlg"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   0
      TabIndex        =   26
      Top             =   4920
      Width           =   1215
   End
   Begin VB.Label Label30 
      Caption         =   "SoundLoadDlg"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   5400
      TabIndex        =   25
      Top             =   3840
      Width           =   1755
   End
   Begin VB.Label Label29 
      Caption         =   "SoundSaveDlg"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   3660
      TabIndex        =   24
      Top             =   3840
      Width           =   1575
   End
   Begin VB.Label Label9 
      Caption         =   "SoundExportDlg"
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
      Left            =   1200
      TabIndex        =   23
      Top             =   3840
      Width           =   1335
   End
   Begin VB.Label Label8 
      Caption         =   "SoundImportDlg"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   1320
      TabIndex        =   22
      Top             =   4920
      Width           =   1275
   End
   Begin VB.Label Label2 
      Caption         =   "TwoDeeTexture"
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
      Left            =   2580
      TabIndex        =   21
      Top             =   2640
      Width           =   1335
   End
   Begin VB.Label Label28 
      Caption         =   "ClassLoad"
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
      Left            =   3360
      TabIndex        =   20
      Top             =   720
      Width           =   855
   End
   Begin VB.Label Label27 
      Caption         =   "ClassSave"
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
      Left            =   4320
      TabIndex        =   19
      Top             =   720
      Width           =   855
   End
   Begin VB.Label Label26 
      Caption         =   "ExportTex"
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
      Left            =   8700
      TabIndex        =   18
      Top             =   720
      Width           =   1095
   End
   Begin VB.Label Label24 
      Caption         =   "AddFile"
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
      Left            =   4440
      TabIndex        =   17
      Top             =   2640
      Width           =   735
   End
   Begin VB.Label Label22 
      Caption         =   "ToolHelp"
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
      Left            =   9240
      TabIndex        =   16
      Top             =   2700
      Width           =   855
   End
   Begin VB.Label Label21 
      Caption         =   "RelNotes"
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
      Left            =   6300
      TabIndex        =   15
      Top             =   2700
      Width           =   855
   End
   Begin VB.Label Label20 
      Caption         =   "About"
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
      Left            =   8460
      TabIndex        =   14
      Top             =   2700
      Width           =   615
   End
   Begin VB.Label Label19 
      Caption         =   "HelpContents"
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
      Left            =   7260
      TabIndex        =   13
      Top             =   2700
      Width           =   1095
   End
   Begin VB.Label Label16 
      Caption         =   "ImportMap"
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
      Left            =   2280
      TabIndex        =   12
      Top             =   1740
      Width           =   975
   End
   Begin VB.Label Label15 
      Caption         =   "ExportMap"
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
      Left            =   1200
      TabIndex        =   11
      Top             =   1740
      Width           =   975
   End
   Begin VB.Label Label14 
      Caption         =   "TexFamLoad"
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
      Left            =   5280
      TabIndex        =   10
      Top             =   720
      Width           =   1035
   End
   Begin VB.Label Label13 
      Caption         =   "TexFamSave"
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
      Left            =   6420
      TabIndex        =   9
      Top             =   720
      Width           =   1095
   End
   Begin VB.Label Label12 
      Caption         =   "TexImport"
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
      Left            =   7620
      TabIndex        =   8
      Top             =   720
      Width           =   975
   End
   Begin VB.Label Label11 
      Caption         =   "TwoDeeSave"
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
      Left            =   1440
      TabIndex        =   7
      Top             =   2640
      Width           =   1095
   End
   Begin VB.Label Label10 
      Caption         =   "TwoDeeOpen"
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
      Left            =   180
      TabIndex        =   6
      Top             =   2640
      Width           =   1155
   End
   Begin VB.Label Label7 
      Caption         =   "BrushSave"
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
      Left            =   8220
      TabIndex        =   5
      Top             =   1740
      Width           =   915
   End
   Begin VB.Label Label6 
      Caption         =   "BrushOpen"
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
      TabIndex        =   4
      Top             =   1740
      Width           =   975
   End
   Begin VB.Label Label5 
      Caption         =   "ExportBrush"
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
      TabIndex        =   3
      Top             =   840
      Width           =   975
   End
   Begin VB.Label Label4 
      Caption         =   "ImportBrush"
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
      Left            =   1260
      TabIndex        =   2
      Top             =   720
      Width           =   975
   End
   Begin VB.Label Label3 
      Caption         =   "MapSaveAs"
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
      Left            =   6900
      TabIndex        =   1
      Top             =   1740
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "MapOpen"
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
      Left            =   5760
      TabIndex        =   0
      Top             =   1740
      Width           =   975
   End
End
Attribute VB_Name = "frmDialogs"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Form_Load()
    Dim SaveFlags As Long, LoadSingleFlags As Long, LoadMultiFlags As Long, HelpFlags As Long
       
    ' Flags.
    SaveFlags = cdlOFNLongNames Or cdlOFNHideReadOnly Or cdlOFNNoReadOnlyReturn Or cdlOFNOverwritePrompt
    LoadSingleFlags = cdlOFNLongNames Or cdlOFNFileMustExist Or cdlOFNExplorer Or cdlOFNShareAware
    LoadMultiFlags = cdlOFNLongNames Or cdlOFNFileMustExist Or cdlOFNExplorer Or cdlOFNShareAware Or cdlOFNAllowMultiselect
    HelpFlags = 0
    
    ' Saving single files.
    ExportBrush.Flags = SaveFlags
    ClassSave.Flags = SaveFlags
    TexFamSave.Flags = SaveFlags
    ExportTex.Flags = SaveFlags
    BrushOpen.Flags = SaveFlags
    ExportMap.Flags = SaveFlags
    MapSaveAs.Flags = SaveFlags
    BrushSave.Flags = SaveFlags
    TwoDeeSave.Flags = SaveFlags
    MusicSaveDlg.Flags = SaveFlags
    SoundExportDlg.Flags = SaveFlags
    SoundSaveDlg.Flags = SaveFlags
    MusicExportDlg.Flags = SaveFlags
    FloorSave.Flags = SaveFlags
    
    ' Loading single files.
    ClassLoad.Flags = LoadSingleFlags
    ImportBrush.Flags = LoadSingleFlags
    ImportMap.Flags = LoadSingleFlags
    MapOpen.Flags = LoadSingleFlags
    TwoDeeOpen.Flags = LoadSingleFlags
    TwoDeeTexture.Flags = LoadSingleFlags
    AddFile.Flags = LoadSingleFlags
    FloorOpen.Flags = LoadSingleFlags
    FHmapOpen.Flags = LoadSingleFlags
    
    ' Loading multiple files.
    MusicLoadDlg.Flags = LoadMultiFlags
    MusicImportDlg.Flags = LoadMultiFlags
    TexFamLoad.Flags = LoadMultiFlags
    TexImport.Flags = LoadMultiFlags
    SoundLoadDlg.Flags = LoadMultiFlags
    SoundImportDlg.Flags = LoadMultiFlags
    
    ' Help.
    RelNotes.Flags = HelpFlags
    HelpContents.Flags = HelpFlags
    About.Flags = HelpFlags
    ToolHelp.Flags = HelpFlags
    
End Sub

