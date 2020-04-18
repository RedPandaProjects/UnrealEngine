/*=============================================================================
	Window.h: GUI window management code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "..\Src\Res\WindowRes.h"

/*-----------------------------------------------------------------------------
	Defines.
-----------------------------------------------------------------------------*/
#ifdef WINDOW_EXPORTS
#define WINDOW_API DLL_EXPORT
#else
#define WINDOW_API DLL_IMPORT
#endif

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// Global functions.
WINDOW_API void InitWindowing();
WINDOW_API UBOOL CheckPassword( const char* Name, const char* Password );
WINDOW_API void RegisterFileTypes( FString Base );
WINDOW_API void HandleError();
WINDOW_API void HandleError();
WINDOW_API void MainLoop( class UEngine* Engine );
WINDOW_API UEngine* InitEngine();
WINDOW_API void ExitEngine( UEngine* Engine );
extern WINDOW_API class WLog* GLog;
extern WINDOW_API class FExec* GThisExecHook;

// Global variables.
extern WINDOW_API UBOOL  GTickDue;
extern WINDOW_API HBRUSH hBrushProperty;
extern WINDOW_API HBRUSH hBrushCategory;
extern WINDOW_API HBRUSH hBrushObject;
extern WINDOW_API HBRUSH hBrushClass;
extern WINDOW_API HBRUSH hBrushStack;
extern WINDOW_API HBRUSH hBrushBlack;

/*-----------------------------------------------------------------------------
	Window class definition macros.
-----------------------------------------------------------------------------*/

#define DECLARE_WINDOWCLASS(cls,parentcls) \
public: \
	const char* GetWindowClassName() {return "Unreal" #cls;} \
	~cls() {MaybeDestroy();}

#define DECLARE_WINDOWSUBCLASS(cls,parentcls ) \
	DECLARE_WINDOWCLASS(cls,parentcls) \
	static WNDPROC SuperProc; \
	WNDPROC GetSuperProc() {return SuperProc;}

#define IMPLEMENT_WINDOWCLASS(cls,hinst,clsf) cls::RegisterWindowClass( "Unreal" #cls, hinst, clsf );
#define IMPLEMENT_WINDOWSUBCLASS(cls,hinst,wincls) cls::SuperProc=cls::RegisterWindowClass( "Unreal" #cls, hinst, wincls );

#define FIRST_AUTO_CONTROL 8192

/*-----------------------------------------------------------------------------
	WWindow.
-----------------------------------------------------------------------------*/

// An operating system window.
class WINDOW_API WWindow
{
public:
	// Variables.
	HWND hWnd;
	FName PersistentName;
	INT TopControlId;
	UBOOL Destroyed;
	WWindow* OwnerWindow;
	FNotifyHook* NotifyHook;
	WNDPROC WindowDefWndProc;
	TArray<class WControl*> Controls;

	// Static.
	static TArray<WWindow*> Windows;
	static LONG APIENTRY StaticProc( WNDPROC DefProc, HWND hWnd, UINT Message, UINT wParam, LONG lParam )
	{
		guard(WWindow::StaticProc);
		for( INT i=0; i<Windows.Num(); i++ )
			if( Windows(i)->hWnd==hWnd )
				break;
		if( i==Windows.Num() && (Message==WM_NCCREATE || Message==WM_INITDIALOG) )
			for( i=0; i<Windows.Num(); i++ )
				if( Windows(i)->hWnd==NULL )
					{Windows(i)->hWnd=hWnd; break;}
		if( i==Windows.Num() || GIsCriticalError )
		{
			return DefProc( hWnd, Message, wParam, lParam );
		}
		else
		{
			return Windows(i)->WndProc( Message, wParam, lParam );			
		}
		check(false)
		unguard;
	}
	static LONG APIENTRY StaticWndProc( HWND hWnd, UINT Message, UINT wParam, LONG lParam )
	{
		guard(WWindow::StaticWndProc);
		return StaticProc( DefWindowProc, hWnd, Message, wParam, lParam );
		unguard;
	}
	static WNDPROC RegisterWindowClass( const char* Name, HINSTANCE hInstance, DWORD Style )
	{
		guard(WWindow::RegisterWindowClass);
		WNDCLASSEX Cls;
		Cls.cbSize			= sizeof(Cls);
		Cls.style			= Style;
		Cls.lpfnWndProc		= StaticWndProc;
		Cls.cbClsExtra		= 0;
		Cls.cbWndExtra		= 0;
		Cls.hInstance		= hInstance;
		Cls.hIcon			= LoadIcon(hInstance,MAKEINTRESOURCE(IDICON_MAINFRAME));
		Cls.hCursor			= NULL;
		Cls.hbrBackground	= NULL;
		Cls.lpszMenuName	= NULL;
		Cls.lpszClassName	= Name;
		Cls.hIconSm			= LoadIcon(hInstance,MAKEINTRESOURCE(IDICON_MAINFRAME));
		verify(RegisterClassEx( &Cls ));
		return NULL;
		unguard;
	}

	// Structors.
	WWindow( FName InPersistentName=NAME_None, WWindow* InOwnerWindow=NULL, WNDPROC InWindowDefWndProc=DefWindowProc )
	:	hWnd				( NULL )
	,	PersistentName		( InPersistentName )
	,	TopControlId		( FIRST_AUTO_CONTROL )
	,	Destroyed			( 0 )
	,	OwnerWindow			( InOwnerWindow )
	,	WindowDefWndProc	( InWindowDefWndProc )
	,	NotifyHook			( 0 )
	{}
	~WWindow()
	{
		guard(WWindow:;~WWindow);
		MaybeDestroy();
		unguard;
	}

	// WWindow interface.
	virtual void DoDestroy()
	{
		guard(WWindow::DoDestroy);
		if( NotifyHook )
			NotifyHook->NotifyDestroy( this );
		if( hWnd )
			DestroyWindow( *this );
		Windows.RemoveItem( this );
		unguard;
	}
	virtual const char* GetWindowClassName()=0;
	virtual void OpenWindow( HWND hWndParent=NULL ) {}
	virtual LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		guard(WWindow::WndProc);
		if( Message==WM_DESTROY )
		{
			OnDestroy();
			return 0;
		}
		else if( Message==WM_CLOSE )
		{
			OnClose();
			return 0;
		}
		else if( Message==WM_INITDIALOG )
		{
			return OnInitDialog( (HWND)wParam );
		}
		else if( Message==WM_SETFOCUS )
		{
			OnSetFocus( (HWND)wParam );
			return 0;
		}
		else if( Message==WM_ACTIVATE )
		{
			OnActivate( LOWORD(wParam)!=0 );
			return 0;
		}
		else if( Message==WM_KILLFOCUS )
		{
			OnKillFocus( (HWND)wParam );
			return 0;
		}
		else if( Message==WM_SIZE )
		{
			OnSize( wParam, LOWORD(lParam), HIWORD(lParam) );
			return 0;
		}
		else if( Message==WM_CHAR )
		{
			OnChar( wParam );
		}
		else if( Message==WM_PASTE )
		{
			OnPaste();
		}
		if( Message==WM_COMMAND )
		{
			return CommandProc( wParam, lParam );		
		}
		else return WindowDefWndProc( hWnd, Message, wParam, lParam );
		unguard;
	}
	virtual LONG CommandProc( UINT wParam, LONG lParam )
	{
		return DefWindowProc( hWnd, WM_COMMAND, wParam, lParam );
	}
	virtual FString GetText()
	{
		guard(WWindow::GetText);
		check(hWnd);
		INT Length = GetLength();
		char* Text = new char[Length+1];
		SendMessage( *this, WM_GETTEXT, Length+1, (LPARAM)Text );
		FString Result = FString( Text );
		delete Text;
		return Result;
		unguard;
	}
	virtual void SetText( const char* Text )
	{
		guard(WWindow::SetText);
		check(hWnd);
		SendMessage( *this, WM_SETTEXT, 0, (LPARAM)Text );
		unguard;
	}
	virtual void SetTextf( const char* Fmt, ... )
	{
		char TempStr[4096];
		GET_VARARGS(TempStr,Fmt);
		SetText( TempStr );
	}
	virtual INT GetLength()
	{
		guard(WWindow::GetLength);
		check(hWnd);
		return SendMessage( *this, WM_GETTEXTLENGTH, 0, 0 );
		unguard;
	}
	void SetNotifyHook( FNotifyHook* InNotifyHook )
	{
		guard(WWindow::SetNotifyHook);
		NotifyHook = InNotifyHook;
		unguard;
	}

	// WWindow notifications.
	virtual void OnSetFocus( HWND hWndLosingFocus )
	{}
	virtual void OnKillFocus( HWND hWndGaininFocus )
	{}
	virtual void OnSize( DWORD Flags, INT NewX, INT NewY )
	{}
	virtual void OnActivate( UBOOL Active )
	{}
	virtual void OnChar( INT Char )
	{}
	virtual void OnPaste()
	{}
	virtual void OnClose()
	{
		guard(WWindow::OnClose);
		if( PersistentName!=NAME_None )
		{
			WINDOWPLACEMENT Place;
			ZeroMemory(&Place,sizeof(Place));
			Place.length = sizeof(Place);
			if( GetWindowPlacement( hWnd, &Place ) )
			{
				char Pos[256];
				appSprintf( Pos, "(X=%i,Y=%i,XL=%i,YL=%i)", Place.rcNormalPosition.left, Place.rcNormalPosition.top, Place.rcNormalPosition.right-Place.rcNormalPosition.left, Place.rcNormalPosition.bottom-Place.rcNormalPosition.top );
				SetConfigString( "WindowPositions", *PersistentName, Pos );
			}
		}
		DestroyWindow( *this );
		unguard;
	}
	virtual UBOOL OnInitDialog( HWND  )
	{
		guard(WWindow::OnInitDialog);
		return TRUE;
		unguard;
	}
	virtual void OnDestroy()
	{
		guard(WWindow::OnDestroy);
		check(hWnd);
		Windows.RemoveItem( this );
		hWnd = NULL;
		unguard;
	}

	// WWindow functions.
	void MaybeDestroy()
	{
		guard(WWindow::MaybeDestroy);
		if( !Destroyed )
		{
			Destroyed=1;
			DoDestroy();
		}
		unguard;
	}
	void CloseWindow()
	{
		guard(WWindow::CloseWindow);
		check(hWnd);
		DestroyWindow( *this );
		unguard;
	}
	operator HWND() const
	{
		return hWnd;
	}
	void PerformCreateWindowEx( DWORD dwExStyle, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam )
	{
		guard(PerformCreateWindowEx);
		check(hWnd==NULL);

		// Retrieve remembered position.
		char Pos[256];
		if
		(	PersistentName!=NAME_None 
		&&	GetConfigString( "WindowPositions", *PersistentName, Pos, ARRAY_COUNT(Pos) ) )
		{
			// Get saved position.
			Parse( Pos, "X=", x );
			Parse( Pos, "Y=", y );
			if( dwStyle & WS_SIZEBOX )
			{
				Parse( Pos, "XL=", nWidth );
				Parse( Pos, "YL=", nHeight );
			}

			// Count identical windows already opened.
			INT Count=0;
			for( INT i=0; i<Windows.Num(); i++ )
			{
				Count += Windows(i)->PersistentName==PersistentName;
			}
			if( Count )
			{
				// Move away.
				x += Count*16;
				y += Count*16;

				// Clip size to screen.
				RECT Desktop;
				GetWindowRect( GetDesktopWindow(), &Desktop );
				if( x+nWidth  > Desktop.right  ) x = Desktop.right  - nWidth;
				if( y+nHeight > Desktop.bottom ) y = Desktop.bottom - nHeight;
				if( x<0 )
				{
					if( dwStyle & WS_SIZEBOX ) nWidth += x;
					x=0;
				}
				if( y<0 )
				{
					if( dwStyle & WS_SIZEBOX ) nHeight += y;
					y=0;
				}
			}
		}

		// Create window.
		Windows.AddItem( this );
		HWND hWndCreated = CreateWindowEx
		(
			dwExStyle,
			GetWindowClassName(),
			lpWindowName,
			dwStyle,
			x,
			y,
			nWidth,
			nHeight,
			hWndParent,
			hMenu,
			hInstance,
			lpParam
		);
		check(hWndCreated);
		check(hWndCreated==hWnd);
		unguard;
	}
	void SetRedraw( UBOOL Redraw )
	{
		guard(WWindow::SetRedraw);
		SendMessage( *this, WM_SETREDRAW, Redraw, 0 );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FControlSnoop.
-----------------------------------------------------------------------------*/

// For forwarding interaction with a control to an object.
class WINDOW_API FControlSnoop
{
public:
	// FControlSnoop interface.
	virtual void SnoopChar( WControl* Src, INT Char ) {}
	virtual void SnoopKeyDown( WControl* Src, INT Char ) {}
	virtual void SnoopLeftMouseDown( WControl* Src, INT X, INT Y ) {}
	virtual void SnoopButtonClick( const char* Text ) {}
	virtual void SnoopThumbTrack() {}
	virtual void SnoopThumbPosition() {}
	virtual void SnoopComboSelEndCancel() {}
	virtual void SnoopComboSelEndOk() {}
};

/*-----------------------------------------------------------------------------
	WControl.
-----------------------------------------------------------------------------*/

// A control which exists inside an owner window.
class WINDOW_API WControl : public WWindow
{
public:
	// Variables.
	INT ControlId;
	virtual WNDPROC GetSuperProc()=0;
	FControlSnoop* Snoop;

	// Structors.
	WControl( WWindow* InOwnerWindow, INT InId )
	:	WWindow		( NAME_None, InOwnerWindow )
	,	ControlId	( InId ? InId : InOwnerWindow->TopControlId++)
	,	Snoop		( NULL )
	{
		check(OwnerWindow);
		OwnerWindow->Controls.AddItem( this );
	}
	~WControl()
	{
		check(OwnerWindow);
		OwnerWindow->Controls.RemoveItem( this );
	}

	// WWindow interface.
	static WNDPROC RegisterWindowClass( const char* Name, HINSTANCE hInstance, const char* WinBaseClass )
	{
		guard(WControl::RegisterWindowClass);
		WNDCLASSEX Cls;
		memset( &Cls, 0, sizeof(Cls) );
		Cls.cbSize        = sizeof(Cls);
		verify( GetClassInfoEx( hInstance, WinBaseClass, &Cls ) );
		WNDPROC SuperProc = Cls.lpfnWndProc;
		Cls.lpfnWndProc   = WWindow::StaticWndProc;
		Cls.lpszClassName = Name;
		Cls.hInstance     = hInstance;
		check(Cls.lpszMenuName==NULL);
		verify(RegisterClassEx( &Cls ));
		return SuperProc;
		unguard;
	}
	virtual LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		if( Snoop )
		{
			if( Message==WM_CHAR )
				Snoop->SnoopChar( this, wParam );
			else if( Message==WM_KEYDOWN )
				Snoop->SnoopKeyDown( this, wParam );
			else if( Message==WM_LBUTTONDOWN )
				Snoop->SnoopLeftMouseDown( this, LOWORD(lParam), HIWORD(lParam) );
			else if( Message==WM_HSCROLL && LOWORD(wParam)==TB_THUMBTRACK )
				Snoop->SnoopThumbTrack();
			else if( Message==WM_HSCROLL )
				Snoop->SnoopThumbPosition();
			else if( Message==WM_COMMAND && HIWORD(wParam)==CBN_SELENDCANCEL )
				Snoop->SnoopComboSelEndCancel();
			else if( Message==WM_COMMAND && HIWORD(wParam)==CBN_SELENDOK )
				Snoop->SnoopComboSelEndOk();
			else if( Message==WM_COMMAND && HIWORD(wParam)==BN_CLICKED )
			{
				char Temp[256]="";
				SendMessage( (HWND)lParam, WM_GETTEXT, ARRAY_COUNT(Temp), (LPARAM)Temp );
				Snoop->SnoopButtonClick( Temp );
			}
		}
		return CallWindowProc( GetSuperProc(), hWnd, Message, wParam, lParam );
	}
	virtual LONG CommandProc( UINT wParam, LONG lParam )
	{
		return CallWindowProc( GetSuperProc(), hWnd, WM_COMMAND, wParam, lParam );
	}
};

/*-----------------------------------------------------------------------------
	WLabel.
-----------------------------------------------------------------------------*/

// A non-interactive label control.
class WINDOW_API WLabel : public WControl
{
	DECLARE_WINDOWSUBCLASS(WLabel,WControl)

	// Constructor.
	WLabel( WWindow* InOwner, INT InId=0 )
	: WControl( InOwner, InId )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible )
	{
		guard(WLabel::OpenWindow);
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0),
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance,
            NULL
		);
		SendMessage( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WButton.
-----------------------------------------------------------------------------*/

// A button.
class WINDOW_API WButton : public WControl
{
	DECLARE_WINDOWSUBCLASS(WButton,WControl)

	// Constructor.
	WButton( WWindow* InOwner, INT InId=0 )
	: WControl( InOwner, InId )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, INT X, INT Y, INT XL, INT YL, const char* Text )
	{
		guard(WButton::OpenWindow);
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0),
            X, Y,
			XL, YL,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance,
            NULL
		);
		SendMessage( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		SetText( Text );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WComboBox.
-----------------------------------------------------------------------------*/

// A combo box control.
class WINDOW_API WComboBox : public WControl
{
	DECLARE_WINDOWSUBCLASS(WComboBox,WControl)

	// Constructor.
	WComboBox( WWindow* InOwner, INT InId=0 )
	: WControl( InOwner, InId )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible )
	{
		guard(WLabel::OpenWindow);
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | (Visible?WS_VISIBLE:0),
            0, 0,
			64, 384,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance,
            NULL
		);
		SendMessage( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		unguard;
	}
	virtual LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		if( Message==WM_KEYDOWN && (wParam==VK_UP || wParam==VK_DOWN) )
		{
			// Suppress arrow keys.
			if( Snoop )
				Snoop->SnoopKeyDown( this, wParam );
			return 1;
		}
		else return WControl::WndProc( Message, wParam, lParam );
	}

	// WComboBox interface.
	virtual void AddString( const void* Str )
	{
		guard(WComboBox::AddString);
		SendMessage( *this, CB_ADDSTRING, 0, (LPARAM)Str );
		unguard;
	}
	virtual FString GetString( INT Index )
	{
		guard(WComboBox::GetString);
		INT Length = SendMessage( *this, CB_GETLBTEXTLEN, Index, 0 );
		if( Length==CB_ERR )
			Length = 0;
		char* Text = new char[Length+1];
		SendMessage( *this, CB_GETLBTEXT, Index, (LPARAM)Text );
		FString Result = FString( Text );
		delete Text;
		return Result;
		unguard;
	}
	virtual INT GetCount()
	{
		guard(WComboBox::GetCount);
		return SendMessage( *this, CB_GETCOUNT, 0, 0 );
		unguard;
	}
	virtual void SetCurrent( INT Index )
	{
		guard(WComboBox::SetCurrent);
		SendMessage( *this, CB_SETCURSEL, Index, 0 );
		unguard;
	}
	virtual INT GetCurrent()
	{
		guard(WComboBox::GetCurrent);
		return SendMessage( *this, CB_GETCURSEL, 0, 0 );
		unguard;
	}
	virtual INT FindString( const char* String )
	{
		guard(WComboBox::FindString);
		INT Index = SendMessage( *this, CB_FINDSTRING, -1, (LPARAM)String );
		return Index!=CB_ERR ? Index : -1;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WEdit.
-----------------------------------------------------------------------------*/

// A single-line or multiline edit control.
class WINDOW_API WEdit : public WControl
{
	DECLARE_WINDOWSUBCLASS(WEdit,WControl)

	// Constructor.
	WEdit( WWindow* InOwner, INT InId=0 )
	: WControl( InOwner, InId )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL Multiline, UBOOL ReadOnly )
	{
		guard(WEdit::OpenWindow);
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0) | ES_LEFT | (Multiline?(ES_MULTILINE|WS_VSCROLL):0) | ES_AUTOVSCROLL | ES_AUTOHSCROLL | (ReadOnly?ES_READONLY:0),
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance,
            NULL
		);
		SendMessage( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		unguard;
	}

	// WEdit interface.
	UBOOL GetReadOnly()
	{
		guard(WEdit::GetReadOnly);
		check(hWnd);
		return (GetWindowLong( *this, GWL_STYLE )&ES_READONLY)!=0;
		unguard;
	}
	void SetReadOnly( UBOOL ReadOnly )
	{
		guard(WEdit::SetReadOnly);
		check(hWnd);
		SendMessage( *this, EM_SETREADONLY, ReadOnly, 0 );
		unguard;
	}
	INT GetLineCount()
	{
		guard(WEdit::GetLineCount);
		check(hWnd);
		return SendMessage( *this, EM_GETLINECOUNT, 0, 0 );
		unguard;
	}
	INT GetLineIndex( INT Line )
	{
		guard(WEdit::GetLineIndex);
		check(hWnd);
		return SendMessage( *this, EM_LINEINDEX, Line, 0 );
		unguard;
	}
	void GetSelection( INT& Start, INT& End )
	{
		guard(WEdit::GetSelection);
		check(hWnd);
		SendMessage( *this, EM_GETSEL, (WPARAM)&Start, (LPARAM)&End );
		unguard;
	}
	void SetSelection( INT Start, INT End )
	{
		guard(WEdit::SetSelection);
		check(hWnd);
		SendMessage( *this, EM_SETSEL, Start, End );
		unguard;
	}
	void SetSelectedText( const char* Text )
	{
		guard(WEdit::SetSelectedText);
		check(hWnd);
		SendMessage( *this, EM_REPLACESEL, 1, (LPARAM)Text );
		unguard;
	}
	UBOOL GetModify()
	{
		guard(WEdit::GetModify);
		return SendMessage( *this, EM_GETMODIFY, 0, 0 )!=0;
		unguard;
	}
	void SetModify( UBOOL Modified )
	{
		guard(WEdit::SetModify);
		SendMessage( *this, EM_SETMODIFY, Modified, 0 );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WTerminal.
-----------------------------------------------------------------------------*/

// Base class of terminal edit windows.
class WINDOW_API WTerminalBase : public WWindow
{
	DECLARE_WINDOWCLASS(WTerminalBase,WWindow)

	// Constructor.
	WTerminalBase( FName InPersistentName )
	: WWindow( InPersistentName )
	{}

	// WTerminalBase interface.
	virtual void TypeChar( char Ch )=0;
	virtual void Paste()=0;
};

// A terminal edit window.
class WINDOW_API WEditTerminal : public WEdit
{
	DECLARE_WINDOWCLASS(WEditTerminal,WEdit)

	// Variables.
	WTerminalBase* OwnerTerminal;

	// Constructor.
	WEditTerminal( WTerminalBase* InOwner )
	: WEdit( InOwner )
	, OwnerTerminal( InOwner )
	{}

	// WWindow interface.
	virtual LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		if( Message==WM_CHAR )
		{
			// Handle typing.
			OwnerTerminal->TypeChar( wParam );
			return 0;
		}
		else if( Message==WM_PASTE )
		{
			OwnerTerminal->Paste();
			return 0;
		}
		else return WEdit::WndProc( Message, wParam, lParam );
	}
};

// A terminal window.
class WINDOW_API WTerminal : public WTerminalBase, public FOutputDevice
{
	DECLARE_WINDOWCLASS(WTerminal,WTerminalBase)

	// Variables.
	WEditTerminal Display;
	FExec* Exec;
	INT MaxLines, SlackLines;
	char Typing[256];

	// Structors.
	WTerminal( FName InPersistentName )
	:	WTerminalBase	( InPersistentName )
	,	Display			( this )
	,	Exec			( NULL )
	,	MaxLines		( 256 )
	,	SlackLines		( 64 )
	{
		strcpy( Typing, ">" );
	}

	// FOutputDevice interface.
	void WriteBinary( const void* Data, INT Length, EName MsgType=NAME_None )
	{
		guard(WTerminal::WriteBinary);

		Display.SetRedraw( 0 );
		INT LineCount = Display.GetLineCount();
		if( LineCount > MaxLines )
		{
			INT NewLineCount = Max(LineCount-SlackLines,0);
			INT Index = Display.GetLineIndex( LineCount-NewLineCount );
			Display.SetSelection( 0, Index );
			Display.SetSelectedText( "" );
			INT Length = Display.GetLength();
			Display.SetSelection( Length, Length );
		}

		char Temp[1024]="";
		strncat( Temp, *FName(MsgType), ARRAY_COUNT(Temp) );
		strncat( Temp, ": ", ARRAY_COUNT(Temp) );
		strncat( Temp, (char*)Data, ARRAY_COUNT(Temp) );
		strncat( Temp, "\r\n", ARRAY_COUNT(Temp) );
		strncat( Temp, Typing, ARRAY_COUNT(Temp) );
		Temp[ARRAY_COUNT(Temp)-1] = 0;
		SelectTyping();
		Display.SetRedraw( 1 );
		Display.SetSelectedText( Temp );

		unguard;
	}

	// WWindow interface.
	void OpenWindow( HWND hWndParent=NULL, UBOOL AppWindow=0 )
	{
		guard(WTerminal::OpenWindow);
		char Title[256];
		appSprintf( Title, LocalizeGeneral("LogWindow","Window"), LocalizeGeneral("Product","Core") );
		PerformCreateWindowEx
		(
			WS_EX_TOOLWINDOW | (AppWindow?WS_EX_APPWINDOW:0),
			Title,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			512,
			256,
			hWndParent,
			NULL,
			hInstance,
			NULL
		);
		Display.OpenWindow(1,1,1);
		Display.SetText( Typing );
		unguard;
	}
	void OnSetFocus( HWND hWndLoser )
	{
		guard(WTerminal::OnSetFocus);
		WWindow::OnSetFocus( hWndLoser );
		SetFocus( Display );
		unguard;
	}
	void OnSize( DWORD Flags, INT NewX, INT NewY )
	{
		guard(WTerminal::OnSize);
		WWindow::OnSize( Flags, NewX, NewY );
		MoveWindow( Display, 0, 0, NewX, NewY, TRUE );
		unguard;
	}

	// WTerminalBase interface.
	void Paste()
	{
		guard(WTerminal::Paste);
		SelectTyping();
		FString Str;
		ClipboardPaste( Str );
		strncat( Typing, *Str, ARRAY_COUNT(Typing) );
		Typing[ARRAY_COUNT(Typing)-1]=0;
		UpdateTyping();
		unguard;
	}
	void TypeChar( char Ch )
	{
		guard(WTerminal::TypeChar);
		SelectTyping();
		INT Length = strlen(Typing);
		if( Ch>=32 )
		{
			if( Length<ARRAY_COUNT(Typing)-1 )
			{
				Typing[Length]=Ch;
				Typing[Length+1]=0;
			}
		}
		else if( Ch==13 && Length>1 )
		{
			UpdateTyping();
			Display.SetSelectedText( "\r\n>" );
			char Temp[ARRAY_COUNT(Typing)];
			strcpy( Temp, Typing+1 );
			strcpy( Typing, ">" );
			if( Exec )
				if( !Exec->Exec( Temp, GSystem ) )
					Log( LocalizeError("Exec","Core") );
			SelectTyping();
		}
		else if( (Ch==8 || Ch==127) && Length>1 )
		{
			Typing[Length-1] = 0;
		}
		else if( Ch==27 )
		{
			strcpy( Typing, ">" );
		}
		UpdateTyping();
		if( Ch==22 )
		{
			Paste();
		}
		unguard;
	}

	// WTerminal interface.
	void SelectTyping()
	{
		guard(WTerminal::SelectTyping);
		INT Length = Display.GetLength();
		Display.SetSelection( Max(Length-strlen(Typing),0U), Length );
		unguard;
	}
	void UpdateTyping()
	{
		guard(WTerminal::UpdateTyping);
		Display.SetSelectedText( Typing );
		unguard;
	}
	void SetExec( FExec* InExec )
	{
		Exec = InExec;
	}
};

/*-----------------------------------------------------------------------------
	WLog.
-----------------------------------------------------------------------------*/

// A log window.
class WINDOW_API WLog : public WTerminal
{
	DECLARE_WINDOWCLASS(WLog,WTerminal)
	WLog( FName InPersistentName )
	: WTerminal( InPersistentName )
	{
		if( !ParseParam(appCmdLine(),"NOLOGWIN") )
			GLogHook = this;
	}
	void OpenWindow( HWND hWndParent, UBOOL Show )
	{
		WTerminal::OpenWindow( hWndParent, !GIsClient );
		ShowWindow( *this, Show ? SW_SHOW : SW_HIDE );
		UpdateWindow( *this );
	}
	void OnDestroy()
	{
		guard(WLog::OnDestroy)
		WTerminal::OnDestroy();
		GLogHook = NULL;
		unguard;
	}
	void OnClose()
	{
		guard(WLog::OnClose);
		if( GIsClient )
		{
			ShowWindow( *this, SW_HIDE );
		}
		else
		{
			debugf( "Closed Log" );
			appRequestExit();
		}
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WDialog.
-----------------------------------------------------------------------------*/

// A dialog window, always based on a Visual C++ dialog template.
//!!does this need to do IsDialogMessage stuff?
class WINDOW_API WDialog : public WWindow
{
public:
	// Variables.
	INT DialogId;

	// Functions.
	WDialog( FName InPersistentName, INT InDialogId, WWindow* InOwnerWindow=NULL )
	: WWindow( InPersistentName, InOwnerWindow, StaticDlgReturnZero )
	, DialogId( InDialogId )
	{}

	// WDialog interface.
	virtual INT DoModal()
	{
		guard(WDialog::DoModal);
		check(hWnd==NULL);
		Windows.AddItem( this );
		return DialogBox( hInstance, MAKEINTRESOURCE(DialogId), NULL, StaticDlgProc );
		unguard;
	}
	virtual UBOOL OnInitDialog( HWND hWndControl )
	{
		guard(WDialog::OnInitDialog);
		for( INT i=0; i<Controls.Num(); i++ )
		{
			// Bind all child controls.
			WControl* Control = Controls(i);
			check(!Control->hWnd);
			Control->hWnd = GetDlgItem( *this, Control->ControlId );
			check(Control->hWnd);
		}
		return 1;
		unguard;
	}
	static LONG APIENTRY StaticDlgReturnZero( HWND hWnd, UINT Message, UINT wParam, LONG lParam )
	{
		return 0;
	}
	static INT APIENTRY StaticDlgProc( HWND hWnd, UINT Message, UINT wParam, LONG lParam )
	{
		guard(WWindow::StaticWndProc);
		return StaticProc( StaticDlgReturnZero, hWnd, Message, wParam, lParam );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WPasswordDialog.
-----------------------------------------------------------------------------*/

// A password dialog box.
class WINDOW_API WPasswordDialog : public WDialog
{
	DECLARE_WINDOWCLASS(WPasswordDialog,WDialog)

	// Controls.
	WEdit Name;
	WEdit Password;
	WLabel Prompt;

	// Output.
	FString ResultName;
	FString ResultPassword;

	// Constructor.
	WPasswordDialog()
	: WDialog	( "PasswordDialog", IDDIALOG_Password )
	, Name		( this, IDEDIT_Name )
	, Password	( this, IDEDIT_Password )
	, Prompt    ( this, IDLABEL_Prompt )
	{}

	// WWindow interface.
	virtual LONG CommandProc( UINT wParam, LONG lParam )
	{
		if( LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL )
		{
			EndDialog( hWnd, LOWORD(wParam)==IDOK ); 
			return 1;
		}
		else return WDialog::CommandProc( wParam, lParam );
	}

	// WDialog interface.
	UBOOL OnInitDialog( HWND hWndControl )
	{
		WDialog::OnInitDialog( hWndControl );
		SetText( LocalizeQuery("PassDlg","Core") );
		Prompt.SetText( LocalizeQuery("PassPrompt","Core") );
		Name.SetText( "" );
		Password.SetText( "" );
		SetFocus( Name );
		return 0;
	}
	void OnDestroy()
	{
		guard(WPasswordDialog::OnDestroy);
		ResultName     = Name.GetText();
		ResultPassword = Password.GetText();
		WDialog::OnDestroy();
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WTrackBar.
-----------------------------------------------------------------------------*/

// A non-interactive label control.
class WINDOW_API WTrackBar : public WControl
{
	DECLARE_WINDOWSUBCLASS(WTrackBar,WControl)

	// Constructor.
	WTrackBar( WWindow* InOwner, INT InId=0 )
	: WControl( InOwner, InId )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible )
	{
		guard(WLabel::OpenWindow);
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | TBS_HORZ | TBS_AUTOTICKS | TBS_BOTTOM | (Visible?WS_VISIBLE:0),
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance,
            NULL
		);
		unguard;
	}

	// WTrackBar interface.
	void SetTicFreq( INT TicFreq )
	{
		guard(WTrackBar::SetTicFreq);
		SendMessage( *this, TBM_SETTICFREQ, TicFreq, 0 );
		unguard;
	}
	void SetRange( INT Min, INT Max )
	{
		guard(WTrackBar::SetRange);
		SendMessage( *this, TBM_SETRANGE, 1, MAKELONG(Min,Max) );
		unguard;
	}
	void SetPos( INT Pos )
	{
		guard(WTrackBar::SetPos);
		SendMessage( *this, TBM_SETPOS, 1, Pos );
		unguard;
	}
	INT GetPos()
	{
		guard(WTrackBar::GetPos);
		return SendMessage( *this, TBM_GETPOS, 0, 0 );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WListBox.
-----------------------------------------------------------------------------*/

// A list box.
class WINDOW_API WListBox : public WControl
{
	DECLARE_WINDOWSUBCLASS(WListBox,WControl)

	// Constructor.
	WListBox( WWindow* InOwner, INT InId=0 )
	: WControl( InOwner, InId )
	{
		check(OwnerWindow);
	}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL Integral, UBOOL MultiSel, UBOOL OwnerDrawVariable )
	{
		guard(WListBox::OpenWindow);
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | WS_BORDER | WS_VSCROLL | WS_CLIPCHILDREN | LBS_NOTIFY | (Visible?WS_VISIBLE:0) | (Integral?0:LBS_NOINTEGRALHEIGHT) | (MultiSel?(LBS_EXTENDEDSEL|LBS_MULTIPLESEL):0) | (OwnerDrawVariable?LBS_OWNERDRAWVARIABLE:0),
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance,
            NULL
		);
		SendMessage( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		unguard;
	}

	// WListBox interface.
	void* GetItemData( INT Index )
	{
		guard(WListBox::GetItemData);
		return (void*)SendMessage( *this, LB_GETITEMDATA, Index, 0 );
		unguard;
	}
	INT GetCurrent()
	{
		guard(WListBox::GetCurrent);
		return SendMessage( *this, LB_GETCARETINDEX, 0, 0 );
		unguard;
	}
	void SetCurrent( INT Index, UBOOL bScrollIntoView )
	{
		guard(WListBox::SetCurrent);
		SendMessage( *this, LB_SETCURSEL, Index, 0 );
		SendMessage( *this, LB_SETCARETINDEX, Index, bScrollIntoView );
		unguard;
	}
	INT GetTop()
	{
		guard(WListBox::GetTop);
		return SendMessage( *this, LB_GETTOPINDEX, 0, 0 );
		unguard;
	}
	void SetTop( INT Index )
	{
		guard(WListBox::SetTop);
		SendMessage( *this, LB_SETTOPINDEX, Index, 0 );
		unguard;
	}
	void DeleteString( INT Index )
	{
		guard(WListBox::DeleteItem);
		SendMessage( *this, LB_DELETESTRING, Index, 0 );
		unguard;
	}
	INT GetCount()
	{
		guard(WListBox::GetCount);
		return SendMessage( *this, LB_GETCOUNT, 0, 0 );
		unguard;
	}
	INT GetItemHeight( INT Index )
	{
		guard(WListBox::GetItemHeight);
		return SendMessage( *this, LB_GETITEMHEIGHT, Index, 0 );
		unguard;
	}
	INT ItemFromPoint( INT X, INT Y )
	{
		guard(WListBox::ItemFromPoint);
		DWORD Result=SendMessage( *this, LB_ITEMFROMPOINT, 0, MAKELPARAM(X,Y) );
		return HIWORD(Result) ? -1 : LOWORD(Result);
		unguard;
	}
	RECT GetItemRect( INT Index )
	{
		guard(WListBox::GetItemRect);
		RECT R; R.left=R.right=R.top=R.bottom=0;
		SendMessage( *this, LB_GETITEMRECT, Index, (LPARAM)&R );
		return R;
		unguard;
	}
	void Empty()
	{
		guard(WListBox::Empty);
		SendMessage( *this, LB_RESETCONTENT, 0, 0 );
		unguard;
	}
	void AddString( const void* C )
	{
		guard(WListBox::AddString);
		SendMessage( *this, LB_ADDSTRING, 0, (LPARAM)C );
		unguard;
	}
	void InsertString( INT Index, void* C )
	{
		guard(WListBox::AddInsert);
		SendMessage( *this, LB_INSERTSTRING, Index, (LPARAM)C );
		unguard;
	}
	INT FindString( void* C )
	{
		guard(WListBox::FindString);
		return SendMessage( *this, LB_FINDSTRING, -1, (LPARAM)C );
		unguard;
	}
	INT FindStringChecked( void* C )
	{
		guard(WListBox::FindStringChecked);
		INT Result=SendMessage( *this, LB_FINDSTRING, -1, (LPARAM)C );
		check(Result!=LB_ERR);
		return Result;
		unguard;
	}
	void InsertStringAfter( void* Existing, void* New )
	{
		guard(WListBox::InsertStringAfter);
		InsertString( FindStringChecked(Existing)+1, New );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WItemBox.
-----------------------------------------------------------------------------*/

// A list box contaning list items.
class WINDOW_API WItemBox : public WListBox
{
	DECLARE_WINDOWCLASS(WItemBox,WListBox)

	// Constructors.
	WItemBox( WWindow* InOwner, INT InId=0)
	: WListBox( InOwner, InId )
	{
		check(OwnerWindow);
	}

	// WWindow interface.
	virtual LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		// Notifications.
		if( Message==WM_ERASEBKGND )
		{
			// Don't erase background.
			return 1;
		}
		else return WControl::WndProc( Message, wParam, lParam );
	}
};

/*-----------------------------------------------------------------------------
	WPropertiesBase.
-----------------------------------------------------------------------------*/

class WINDOW_API WPropertiesBase : public WWindow, public FControlSnoop
{
public:
	// Variables.
	WItemBox List;
	class FTreeItem* FocusItem;

	// Structors.
	WPropertiesBase( FName InPersistentName )
	:	WWindow		( InPersistentName )
	,	List		( this )
	,	FocusItem	( NULL )
	{
		List.Snoop = this;
	}

	// WPropertiesBase interface.
	virtual FTreeItem* GetRoot()=0;
	virtual INT GetDividerWidth()=0;
	virtual void ResizeList()=0;
	virtual void SetItemFocus( UBOOL FocusCurrent )=0;
	virtual void ForceRefresh()=0;
	virtual class FTreeItem* GetListItem( INT i )
	{
		guard(WProperties::GetListItem);
		FTreeItem* Result = (FTreeItem*)List.GetItemData(i);
		check(Result);
		return Result;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FTreeItem.
-----------------------------------------------------------------------------*/

// QSort comparator.
inline INT Compare( const class FTreeItem* T1, const class FTreeItem* T2 );

// Base class of list items.
class WINDOW_API FTreeItem : public FControlSnoop
{
public:
	// Variables.
	class WPropertiesBase*	OwnerProperties;
	FTreeItem*				Parent;
	UBOOL					Expandable;
	UBOOL					Expanded;
	UBOOL					Sorted;
	INT						ButtonWidth;
	TArray<WButton>			Buttons;
	TArray<FTreeItem*>		Children;

	// Structors.
	FTreeItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UBOOL InExpandable )
	:	OwnerProperties	( InOwnerProperties )
	,	Parent			( InParent )
	,	Expandable		( InExpandable )
	,	Expanded		( 0 )
	,	Sorted			( 1 )
	,	ButtonWidth		( 0 )
	,	Buttons			()
	,	Children		()
	{}
	~FTreeItem()
	{
		guard(FTreeItem::~FTreeItem);
		EmptyChildren();
		unguard;;
	}

	// FTreeItem interface.
	void EmptyChildren()
	{
		guard(FTreeItem::EmptyChildren);
		for( INT i=0; i<Children.Num(); i++ )
			delete Children(i);
		Children.Empty();
		unguard;
	}
	virtual RECT GetRect()
	{
		guard(FTreeItem::GetRect);
		return OwnerProperties->List.GetItemRect(OwnerProperties->List.FindStringChecked(this));
		unguard;
	}
	virtual void Redraw()
	{
		guard(FTreeItem::Redraw);
		InvalidateRect( OwnerProperties->List, &GetRect(), 0 );
		UpdateWindow( OwnerProperties->List );
		if( Parent!=OwnerProperties->GetRoot() )
			Parent->Redraw();
		unguard;
	}
	virtual void OnItemSetFocus()
	{
		guard(FTreeItem::OnItemSetFocus);
		if( Parent && Parent!=OwnerProperties->GetRoot() )
			Parent->OnItemSetFocus();
		unguard;
	}
	virtual void OnItemKillFocus( UBOOL Abort )
	{
		guard(FTreeItem::OnItemKillFocus);
		Buttons.Empty();
		ButtonWidth=0;
		Redraw();
		if( Parent && Parent!=OwnerProperties->GetRoot() )
			Parent->OnItemKillFocus( Abort );
		unguard;
	}
	virtual void AddButton( const char* Text )
	{
		guard(FTreeItem::AddButton);
		RECT Rect=GetRect();
		Rect.right -= ButtonWidth;
		SIZE Size;
		HDC hDC=GetDC(*OwnerProperties);
		GetTextExtentPoint32( hDC, Text, appStrlen(Text), &Size ); 
		ReleaseDC(*OwnerProperties,hDC);
		INT Width = Size.cx + 2;
		WButton* Button = new(Buttons)WButton(&OwnerProperties->List);
		Button->OpenWindow( 1, Rect.right-Width, Rect.top, Width, Rect.bottom-Rect.top, Text );
		ButtonWidth += Width;
		unguard;
	}
	virtual void OnItemLeftMouseDown( INT X, INT Y )
	{
		guard(FTreeItem::OnItemLeftMouseDown);
		ToggleExpansion();
		unguard;
	}
	INT GetIndent()
	{
		guard(FTreeItem::GetIndent);
		INT Result=0;
		for( FTreeItem* Test=Parent; Test!=OwnerProperties->GetRoot(); Test=Test->Parent )
			Result++;
		return Result;
		unguard;
	}
	virtual INT GetIndentPixels()
	{
		guard(FTreeItem::GetIndentPixels);
		return 10*GetIndent();
		unguard;
	}
	virtual void DrawTreeLines( HDC hDC, RECT Rect, UBOOL Selected )
	{
		guard(FTreeItem::Draw);
		if( Expandable )
		{
			char* Str = Expanded ? "-" : "+";
			Rect.left += 2 + GetIndentPixels();
			Rect.bottom++;
			DrawTextEx( hDC, Str, appStrlen(Str), &Rect, DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );
			Rect.bottom--;
			Rect.left -= 2 + GetIndentPixels();
		}
		unguard;
	}
	virtual void Collapse()
	{
		guard(FTreeItem::Collapse);
		OwnerProperties->SetItemFocus( 0 );
		INT Index = OwnerProperties->List.FindStringChecked( this );
		INT Count = OwnerProperties->List.GetCount();
		while( Index+1<Count )
		{
			FTreeItem* NextItem = OwnerProperties->GetListItem(Index+1);
			for( FTreeItem* Check=NextItem->Parent; Check && Check!=this; Check=Check->Parent );
			if( !Check )
				break;
			NextItem->Expanded = 0;
			OwnerProperties->List.DeleteString( Index+1 );
			Count--;
		}
		Expanded=0;
		OwnerProperties->ResizeList();
		unguard;
	}
	virtual void Expand()
	{
		guard(FTreeItem::Expand);
		if( Sorted )
			appSort( &Children(0), Children.Num() );
		if( this==OwnerProperties->GetRoot() )
		{
			for( INT i=0; i<Children.Num(); i++ )
				OwnerProperties->List.AddString( Children(i) );
		}
		else
		{
			for( INT i=Children.Num()-1; i>=0; i-- )
				OwnerProperties->List.InsertStringAfter( this, Children(i) );
		}
		OwnerProperties->SetItemFocus( 0 );
		OwnerProperties->ResizeList();
		Expanded=1;
		unguard;
	}
	virtual void ToggleExpansion()
	{
		guard(FTreeItem::ToggleExpansion);
		if( Expandable )
		{
			OwnerProperties->List.SetRedraw( 0 );
			if( Expanded )
				Collapse();
			else
				Expand();
			OwnerProperties->List.SetRedraw( 1 );
			UpdateWindow( OwnerProperties->List );
		}
		OwnerProperties->SetItemFocus( 1 );
		unguard;
	}
	virtual void OnItemDoubleClick()
	{
		guard(FTreeItem::OnItemDoubleClick);
		ToggleExpansion();
		unguard;
	}
	virtual BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
		guard(FTreeList::GetReadAddress);
		return Parent ? Parent->GetReadAddress(Property,Offset) : NULL;
		unguard;
	}
	virtual void SetProperty( UProperty* Property, INT Offset, const char* Value )
	{
		guard(FTreeList::SetProperty);
		if( Parent ) Parent->SetProperty( Property, Offset, Value );
		unguard;
	}
	virtual void GetStates( TArray<FName>& States )
	{
		guard(FTreeList::GetStates);
		if( Parent ) Parent->GetStates( States );
		unguard;
	}
	virtual UBOOL AcceptFlags( DWORD InFlags )
	{
		guard(FTreeList::AcceptFlags);
		return Parent ? Parent->AcceptFlags( InFlags ) : 0;
		unguard;
	}
	virtual void Draw( HDC hDC, RECT Rect, UBOOL Selected )=0;
	virtual INT GetHeight()=0;
	virtual QWORD GetId() const=0;
	virtual char* GetCaption( char* Result ) const=0;
	virtual void OnItemHelp() {}
	virtual void SetFocusToItem() {}
	virtual void SetValue( const char* Value ) {}

	// FControlSnoop interface.
	void SnoopChar( WControl* Src, INT Char )
	{
		guard(FTreeItem::SnoopChar);
		if( Char==13 && Expandable )
			ToggleExpansion();
		unguard;
	}
	void SnoopKeyDown( WControl* Src, INT Char )
	{
		guard(FTreeItem::SnoopChar);
		if( Char==VK_UP || Char==VK_DOWN )
			PostMessage( OwnerProperties->List, WM_KEYDOWN, Char, 0 );
		if( Char==9 )
			PostMessage( OwnerProperties->List, WM_KEYDOWN, (GetKeyState(16)&0x8000)?VK_UP:VK_DOWN, 0 );
		unguard;
	}
};

// QSort comparator.
inline INT Compare( const class FTreeItem* T1, const class FTreeItem* T2 )
{
	char S1[256], S2[256];
	T1->GetCaption( S1 );
	T2->GetCaption( S2 );
	return appStricmp( S1, S2 );
}

// Property list item.
class WINDOW_API FPropertyItem : public FTreeItem
{
public:
	// Variables.
	UProperty*      Property;
	INT				Offset;
	INT				ArrayIndex;
	WEdit*			EditControl;
	WTrackBar*		TrackControl;
	WComboBox*		ComboControl;
	WLabel*			HolderControl;
	UBOOL			ComboChanged;
	FName			Name;

	// Constructors.
	FPropertyItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UProperty* InProperty, FName InName, INT InOffset, INT InArrayIndex )
	:	FTreeItem	    ( InOwnerProperties, InParent, (Cast<UStructProperty>(InProperty) && Cast<UStructProperty>(InProperty)->Struct->GetFName()!=NAME_Color && Cast<UStructProperty>(InProperty)->Struct->GetFName()!=NAME_DynamicString) || (InProperty->ArrayDim>1 && InArrayIndex==-1) )
	,	Property		( InProperty )
	,	Name			( InName )
	,	Offset			( InOffset )
	,	ArrayIndex		( InArrayIndex )
	,	EditControl		( NULL )
	,	TrackControl	( NULL )
	,	ComboControl	( NULL )
	,	HolderControl	( NULL )
	,	ComboChanged	( 0 )
	{}

	// FTreeItem interface.
	QWORD GetId() const
	{
		guard(FPropertyItem::GetId);
		return Name.GetIndex() + ((QWORD)1<<32);
		unguard;
	}
	virtual char* GetCaption( char* Result ) const
	{
		guard(FPropertyItem::GetCaption);
		appStrcpy( Result, *Name );
		return Result;
		unguard;
	}
	void GetPropertyText( char* Str, BYTE* ReadValue )
	{
		guard(GetPropertyText);
		if( Cast<UClassProperty>(Property) && appStricmp(*Property->Category,"Drivers")==0 )
		{
			// Class config.
			char Path[256]="", *Ptr;
			GetConfigString( Property->GetOwnerClass()->GetPathName(), Property->GetName(), Path, 256 );
			Ptr = appStrstr(Path,".");
			appStrcpy( Str, Ptr ? (*Ptr++=0,Localize(Ptr,"ClassCaption",Path)) : Localize("Language","Language","Core",Path) );
		}
		else
		{
			// Regular property.
			Property->ExportText( 0, Str, ReadValue-Property->Offset, ReadValue-Property->Offset, 1 );
		}
		unguard;
	}
	void SetValue( const char* Value )
	{
		guard(FPropertyItem::SetValue);
		SetProperty( Property, Offset, Value );
		ReceiveFromControl();
		Redraw();
		unguard;
	}
	void Draw( HDC hDC, RECT Rect, UBOOL Selected )
	{
		guard(FPropertyItem::Draw);
		RECT LeftRect=Rect, RightRect=Rect;
		char Str[256];

		// Clear the area.
		FillRect( hDC, &Rect, hBrushProperty ); 

		// Draw key background.
		SetBkMode( hDC, TRANSPARENT );
		LeftRect.left += GetIndentPixels();
		LeftRect.right = OwnerProperties->GetDividerWidth();
		if( Selected )
		{
			DrawEdge( hDC, &LeftRect, BDR_SUNKENOUTER | BDR_SUNKENINNER, BF_BOTTOMLEFT | BF_TOPRIGHT); 
		}
		else
		{
			LeftRect.bottom++;
			LeftRect.right++;
			DrawEdge( hDC, &LeftRect, BDR_RAISEDOUTER, BF_FLAT | BF_BOTTOMLEFT | BF_TOPRIGHT); 
			LeftRect.bottom--;
			LeftRect.right--;
		}

		// Draw tree lines.
		DrawTreeLines( hDC, Rect, Selected );

		// Draw key.
		if( ArrayIndex==-1 ) strcpy( Str, *Name );
		else appSprintf( Str, "[%i]", ArrayIndex );
		LeftRect.left += 9;
		LeftRect.bottom++;
		DrawTextEx( hDC, Str, appStrlen(Str), &LeftRect, DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );
		LeftRect.bottom--;

		// Draw value background.
		RightRect.left = OwnerProperties->GetDividerWidth();
		RightRect.bottom++;
		DrawEdge( hDC, &RightRect, BDR_RAISEDOUTER, BF_FLAT | BF_BOTTOMLEFT | BF_TOPRIGHT); 
		RightRect.bottom--;

		// Draw value.
		RightRect.right -= ButtonWidth;
		BYTE* ReadValue = GetReadAddress( Property, Offset );
		if( Property->ArrayDim!=1 && ArrayIndex==-1 )
		{
			// Array expander.
			char* Str="...";
			RightRect.left += 4;
			RightRect.bottom++;
			DrawTextEx( hDC, Str, appStrlen(Str), &RightRect, DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );
			RightRect.bottom--;
			RightRect.left -= 4;
		}
		else if( ReadValue && Cast<UStructProperty>(Property) && Cast<UStructProperty>(Property)->Struct->GetFName()==NAME_Color )
		{
			// Color.
			RightRect.left+=4; RightRect.top+=4; RightRect.right-=4; RightRect.bottom-=3;
			FillRect( hDC, &RightRect, hBrushBlack ); 
			RightRect.left++; RightRect.top++; RightRect.right--; RightRect.bottom--;
			HBRUSH hBrush = CreateSolidBrush(COLORREF(*(DWORD*)ReadValue));
			FillRect( hDC, &RightRect, hBrush ); 
			DeleteObject( hBrush );
		}
		else if( ReadValue )
		{
			// Text.
			*Str=0;
			GetPropertyText( Str, ReadValue );
			RightRect.left += 4;
			RightRect.bottom++;
			DrawTextEx( hDC, Str, appStrlen(Str), &RightRect, DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );
			RightRect.bottom--;
			RightRect.left -= 4;
		}
 		unguard;
	}
	INT GetHeight()
	{
		guard(FPropertyItem::GetHeigth);
		return 16;
		unguard;
	}
	void SetFocusToItem()
	{
		guard(FPropertyItem::SetFocusToItem);
		if( EditControl )
			SetFocus( *EditControl );
		else if( TrackControl )
			SetFocus( *TrackControl );
		else if( ComboControl )
			SetFocus( *ComboControl );
		unguard;
	}
	void OnItemDoubleClick()
	{
		guard(FProperty::OnItemDoubleClick);
		Advance();
		FTreeItem::OnItemDoubleClick();
		unguard;
	}
	void OnItemSetFocus()
	{
		guard(FPropertyItem::OnItemSetFocus);
		check(!EditControl);
		check(!TrackControl);
		FTreeItem::OnItemSetFocus();
		if
		(	(Property->ArrayDim==1 || ArrayIndex!=-1)
		&&	!(Property->PropertyFlags & CPF_EditConst) )
		{
			if( Property->IsA(UByteProperty::StaticClass) && !Cast<UByteProperty>(Property)->Enum )
			{
				// Slider.
				RECT Rect = GetRect();
				Rect.left = 28+OwnerProperties->GetDividerWidth();
				Rect.top--;
				TrackControl = new WTrackBar( &OwnerProperties->List );
				TrackControl->Snoop = this;
				TrackControl->OpenWindow( 0 );
				TrackControl->SetTicFreq( 32 );
				TrackControl->SetRange( 0, 255 );
				MoveWindow( *TrackControl, Rect.left, Rect.top, Rect.right-Rect.left, Rect.bottom-Rect.top, 0 );
			}
			if
			(	(Property->IsA(UBoolProperty::StaticClass))
			||	(Property->IsA(UByteProperty::StaticClass) && Cast<UByteProperty>(Property)->Enum)
			||	(Property->IsA(UNameProperty::StaticClass) && Name==NAME_InitialState)
			||  (Cast<UClassProperty>(Property) && appStricmp(*Property->Category,"Drivers")==0) )
			{
				// Combo box.
				RECT Rect = GetRect();
				Rect.left = 1+OwnerProperties->GetDividerWidth();
				Rect.left--;
				Rect.right--;
				Rect.bottom--;

				RECT HolderRect = Rect;
				HolderRect.bottom++;
				HolderRect.left=HolderRect.right-20;
				HolderControl = new WLabel( &OwnerProperties->List );
				HolderControl->Snoop = this;
				HolderControl->OpenWindow( 0 );
				MoveWindow( *HolderControl, HolderRect.left, HolderRect.top, HolderRect.right-HolderRect.left, HolderRect.bottom-HolderRect.top, 0 );

				Rect.top-=6;
				Rect.left-=2;
				Rect.right-=2;
				ComboControl = new WComboBox( HolderControl );
				ComboControl->Snoop = this;
				ComboControl->OpenWindow( 0 );
				MoveWindow( *ComboControl, Rect.left-HolderRect.left, Rect.top-HolderRect.top, Rect.right-Rect.left, Rect.bottom-Rect.top, 0 );

				if( Property->IsA(UBoolProperty::StaticClass) )
				{
					ComboControl->AddString( LocalizeGeneral("False", "Core") );
					ComboControl->AddString( LocalizeGeneral("True",  "Core") );
				}
				else if( Property->IsA(UByteProperty::StaticClass) )
				{
					for( INT i=0; i<Cast<UByteProperty>(Property)->Enum->Names.Num(); i++ )
						ComboControl->AddString( *Cast<UByteProperty>(Property)->Enum->Names(i) );
				}
				else if( Property->IsA(UNameProperty::StaticClass) && Name==NAME_InitialState )
				{
					TArray<FName> States;
					GetStates( States );
					ComboControl->AddString( *FName(NAME_None) );
					for( INT i=0; i<States.Num(); i++ )
						ComboControl->AddString( *States(i) );
				}
				else if( Cast<UClassProperty>(Property) && appStricmp(*Property->Category,"Drivers")==0 )
				{
					UClassProperty* ClassProp = CastChecked<UClassProperty>(Property);
					TArray<FRegistryObjectInfo> Classes;
					GObj.GetRegistryObjects( Classes, UClass::StaticClass, ClassProp->MetaClass, 0 );
					for( INT i=0; i<Classes.Num(); i++ )
					{
						char Path[256], *Str;
						appStrcpy( Path, Classes(i).Object );
						Str = appStrstr(Path,".");
						ComboControl->AddString( Str ? (*Str++=0,Localize(Str,"ClassCaption",Path)) : Localize("Language","Language","Core",Path) );
					}
					goto SkipTheRest;
				}
			}
			if( Property->IsA(UStructProperty::StaticClass) && appStricmp(Cast<UStructProperty>(Property)->Struct->GetName(),"Color")==0 )
			{
				// Color.
				AddButton( LocalizeGeneral("BrowseButton","Window") );
			}
			else if( Property->IsA(UObjectProperty::StaticClass) )
			{
				// Class.
				AddButton( LocalizeGeneral("BrowseButton","Window") );
				AddButton( LocalizeGeneral("UseButton","Window") );
				AddButton( LocalizeGeneral("ClearButton","Window") );
			}
			if
			(	(Property->IsA(UFloatProperty::StaticClass))
			||	(Property->IsA(UIntProperty::StaticClass))
			||	(Property->IsA(UNameProperty::StaticClass) && Name!=NAME_InitialState)
			||	(Property->IsA(UStringProperty::StaticClass))
			||	(Property->IsA(UObjectProperty::StaticClass))
			||	(Property->IsA(UStructProperty::StaticClass) && Cast<UStructProperty>(Property)->Struct->GetFName()==NAME_DynamicString)
			||	(Property->IsA(UByteProperty::StaticClass) && Cast<UByteProperty>(Property)->Enum==NULL) )
			{
				// Edit control.
				RECT Rect = GetRect();
				Rect.left = 1+OwnerProperties->GetDividerWidth();
				Rect.top--;
				if( Property->IsA(UByteProperty::StaticClass) )
					Rect.right = Rect.left + 28;
				else
					Rect.right -= ButtonWidth;
				EditControl = new WEdit( &OwnerProperties->List );
				EditControl->Snoop = this;
				EditControl->OpenWindow( 0, 0, 0 );
				MoveWindow( *EditControl, Rect.left, Rect.top+1, Rect.right-Rect.left, Rect.bottom-Rect.top, 0 );
			}
			SkipTheRest:
			ReceiveFromControl();
			Redraw();
			if( EditControl )
				ShowWindow( *EditControl, SW_SHOW );
			if( TrackControl )
				ShowWindow( *TrackControl, SW_SHOW );
			if( ComboControl )
				ShowWindow( *ComboControl, SW_SHOW );
			if( HolderControl )
				ShowWindow( *HolderControl, SW_SHOW );
			SetFocusToItem();
		}
		unguard;
	}
	void OnItemKillFocus( UBOOL Abort )
	{
		guard(FPropertyItem::OnKillFocus);
		if( !Abort )
			SendToControl();
		if( EditControl )
			delete EditControl;
		EditControl=NULL;
		if( TrackControl )
			delete TrackControl;
		TrackControl=NULL;
		if( ComboControl )
			delete ComboControl;
		ComboControl=NULL;
		if( HolderControl )
			delete HolderControl;
		HolderControl=NULL;
		FTreeItem::OnItemKillFocus( Abort );
		unguard;
	}
	void Expand()
	{
		guard(FPropertyItem::Expand);
		UStructProperty* StructProperty;
		if( Property->ArrayDim>1 && ArrayIndex==-1 )
		{
			// Expand array.
			Sorted=0;
			for( INT i=0; i<Property->ArrayDim; i++ )
				Children.AddItem( new FPropertyItem( OwnerProperties, this, Property, Name, Offset + i*Property->GetElementSize(), i ) );
		}
		else if( (StructProperty=Cast<UStructProperty>(Property))!=NULL )
		{
			// Expand struct.
			for( TFieldIterator<UProperty> It(StructProperty->Struct); It; ++It )
				if( AcceptFlags( It->PropertyFlags ) )
					Children.AddItem( new FPropertyItem( OwnerProperties, this, *It, It->GetFName(), Offset + It->Offset, -1 ) );
		}
		FTreeItem::Expand();
		unguard;
	}
	void Collapse()
	{
		guard(WPropertyItem::Collapse);
		FTreeItem::Collapse();
		EmptyChildren();
		unguard;
	}

	// FControlSnoop interface.
	void SnoopChar( WControl* Src, INT Char )
	{
		guard(FPropertyItem::SnoopChar);
		if( Char==13 )
			Advance();
		else if( Char==27 )
			ReceiveFromControl();
		FTreeItem::SnoopChar( Src, Char );
		unguard;
	}
	void SnoopComboSelEndCancel()
	{
		guard(FPropertyItem::SnoopComboSelEndCancel);
		ReceiveFromControl();
		unguard;
	}
	void SnoopComboSelEndOk()
	{
		guard(FPropertyItem::SnoopComboSelEndOk);
		ComboChanged=1;
		SendToControl();
		ReceiveFromControl();
		Redraw();
		unguard;
	}
	void SnoopThumbTrack()
	{
		guard(FPropertyItem::SnoopThumbTrack);
		if( TrackControl && EditControl )
		{
			char Tmp[256];
			appSprintf( Tmp, "%i", TrackControl->GetPos() );
			EditControl->SetText( Tmp );
			EditControl->SetModify( 1 );
			UpdateWindow( *EditControl );
		}
		unguard;
	}
	void SnoopThumbPosition()
	{
		guard(FPropertyItem::SnoopThumbPosition);
		SnoopThumbTrack();
		SendToControl();
		if( EditControl )
		{
			SetFocus( *EditControl );
			EditControl->SetSelection( 0, EditControl->GetText().Length() );
			Redraw();
		}
		unguard;
	}
	void SnoopButtonClick( const char* Text )
	{
		guard(FPropertyItem::SnoopButtonClick);
		UStructProperty* StructProperty;
		UObjectProperty* ReferenceProperty;
		if( (StructProperty=Cast<UStructProperty>(Property))!=NULL && appStricmp(StructProperty->Struct->GetName(),"Color")==0 )
		{
			// Choose color.
			BYTE* ReadValue = GetReadAddress( Property, Offset );
			CHOOSECOLOR cc;
			static COLORREF acrCustClr[16];
			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize  = sizeof(cc);
			cc.hwndOwner    = OwnerProperties->List;
			cc.lpCustColors = (LPDWORD)acrCustClr;
			cc.rgbResult    = ReadValue ? *(DWORD*)ReadValue : 0;
			cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
			if( ChooseColor(&cc)==TRUE )
			{
				char Str[256];
				appSprintf( Str, "(R=%i,G=%i,B=%i)", GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult) );
				SetValue( Str );
				InvalidateRect( OwnerProperties->List, NULL, 0 );
				UpdateWindow( OwnerProperties->List );
			}
		}
		else if( (ReferenceProperty=Cast<UObjectProperty>(Property))!=NULL && appStricmp(Text,LocalizeGeneral("BrowseButton","Window"))==0 )
		{
			// Browse for object.
			char Temp[256];
			appSprintf( Temp, "BROWSECLASS CLASS=%s", ReferenceProperty->PropertyClass->GetName() );
			if( OwnerProperties->NotifyHook )
				OwnerProperties->NotifyHook->NotifyExec( OwnerProperties, Temp );
		}
		else if( ReferenceProperty && appStricmp(Text,LocalizeGeneral("UseButton","Window"))==0 )
		{
			// Use object.
			char Temp[256];
			appSprintf( Temp, "USECURRENT CLASS=%s", ReferenceProperty->PropertyClass->GetName() );
			if( OwnerProperties->NotifyHook )
				OwnerProperties->NotifyHook->NotifyExec( OwnerProperties, Temp );
		}
		else if( Property->IsA(UObjectProperty::StaticClass) && appStricmp(Text,LocalizeGeneral("ClearButton","Window"))==0 )
		{
			// Clear object.
			SetValue( "None" );
		}
		else if( Parent && Parent!=OwnerProperties->GetRoot() )
		{
			Parent->SnoopButtonClick( Text );
		}
		Redraw();
		unguard;
	}

	// FPropertyItem interface.
	virtual void Advance()
	{
		guard(FPropertyItem::Advance);
		if( ComboControl && ComboControl->GetCurrent()>=0 )
		{
			ComboControl->SetCurrent( (ComboControl->GetCurrent()+1) % ComboControl->GetCount() );
			ComboChanged=1;
		}
		SendToControl();
		ReceiveFromControl();
		Redraw();
		unguard;
	}
	virtual void SendToControl()
	{
		guard(FPropertyItem::SendToControl);
		if( EditControl )
		{
			if( EditControl->GetModify() )
				SetValue( *EditControl->GetText() );
		}
		else if( ComboControl )
		{
			if( ComboChanged )
				SetValue( *ComboControl->GetString(ComboControl->GetCurrent()) );
			ComboChanged=0;
		}
		unguard;
	}
	virtual void ReceiveFromControl()
	{
		guard(FPropertyItem::ReceiveFromControl);
		ComboChanged=0;
		BYTE* ReadValue = GetReadAddress( Property, Offset );
		if( EditControl )
		{
			char Str[256]="";
			if( ReadValue )
				GetPropertyText( Str, ReadValue );
			EditControl->SetText( Str );
			EditControl->SetSelection( 0, appStrlen(Str) );
		}
		if( TrackControl )
		{
			if( ReadValue )
				TrackControl->SetPos( *(BYTE*)ReadValue );
		}
		if( ComboControl )
		{
			UBoolProperty* BoolProperty;
			if( (BoolProperty=Cast<UBoolProperty>(Property))!=NULL )
			{
				ComboControl->SetCurrent( ReadValue ? (*(DWORD*)ReadValue&BoolProperty->BitMask)!=0 : -1 );
			}
			else if( Property->IsA(UByteProperty::StaticClass) )
			{
				ComboControl->SetCurrent( ReadValue ? *(BYTE*)ReadValue : -1 );
			}
			else if( Property->IsA(UNameProperty::StaticClass) && Name==NAME_InitialState )
			{
				INT Index=ReadValue ? ComboControl->FindString( **(FName*)ReadValue ) : 0;
				ComboControl->SetCurrent( Index>=0 ? Index : 0 );
			}
			ComboChanged=0;
		}
		unguard;
	}
};

// A list header.
class WINDOW_API FHeaderItem : public FTreeItem
{
public:
	// Constructors.
	FHeaderItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UBOOL InExpandable )
	: FTreeItem( InOwnerProperties, InParent, InExpandable )
	{}

	// FTreeItem interface.
	void Draw( HDC hDC, RECT Rect, UBOOL Selected )
	{
		guard(FHeaderItem::Draw);
		RECT NewRect=Rect;

		// Fill background.
		FillRect( hDC, &Rect, GetBackgroundBrush() ); 
		SetBkMode( hDC, TRANSPARENT );
		NewRect.left += GetIndentPixels();

		// Draw borders.
		if( Selected )
		{
			DrawEdge( hDC, &NewRect, BDR_SUNKENOUTER | BDR_SUNKENINNER, BF_BOTTOMLEFT | BF_TOPRIGHT); 
		}
		else
		{
			NewRect.bottom++;
			DrawEdge( hDC, &NewRect, BDR_RAISEDOUTER, BF_FLAT | BF_BOTTOMLEFT | BF_TOPRIGHT); 
			NewRect.bottom--;
		}
		DrawTreeLines( hDC, Rect, Selected );

		// Draw name.
		char C[256];
		GetCaption( C );
		NewRect.left += 9;
		NewRect.top++;
		NewRect.right -= ButtonWidth;
		DrawTextEx( hDC, C, appStrlen(C), &NewRect, DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );

 		unguard;
	}
	INT GetHeight()
	{
		guard(FHeaderItem::GetHeigth);
		return 16;
		unguard;
	}

	// FHeaderItem interface.
	virtual HBRUSH GetBackgroundBrush()=0;
};

// An category header list item.
class WINDOW_API FCategoryItem : public FHeaderItem
{
public:
	// Variables.
	FName Category;
	UClass* BaseClass;

	// Constructors.
	FCategoryItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UClass* InBaseClass, FName InCategory, UBOOL InExpandable )
	:	FHeaderItem( InOwnerProperties, InParent, InExpandable )
	,	Category    ( InCategory )
	,	BaseClass	( InBaseClass )
	{
		check(BaseClass);
	}

	// FTreeItem interface.
	QWORD GetId() const
	{
		guard(FCategoryItem::GetId);
		return Category.GetIndex() + ((QWORD)2<<32);
		unguard;
	}
	virtual char* GetCaption( char* Result ) const
	{
		guard(FCategoryItem::GetText);
		appStrcpy( Result, *Category );
		return Result;
		unguard;
	}
	virtual HBRUSH GetBackgroundBrush()
	{
		guard(FCategoryItem::GetBackgroundBrush);
		return hBrushCategory;
		unguard;
	}
	void Expand()
	{
		guard(FCategoryItem::Expand);
		for( TFieldIterator<UProperty> It(BaseClass); It; ++It )
			if( It->Category==Category && AcceptFlags(It->PropertyFlags) )
				Children.AddItem( new FPropertyItem( OwnerProperties, this, *It, It->GetFName(), It->Offset, -1 ) );
		FTreeItem::Expand();
		unguard;
	}
	void Collapse()
	{
		guard(FCategoryItem::Collapse);
		FTreeItem::Collapse();
		EmptyChildren();
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WProperties.
-----------------------------------------------------------------------------*/

// General property editing control.
class WINDOW_API WProperties : public WPropertiesBase
{
	DECLARE_WINDOWCLASS(WProperties,WWindow)

	// Variables.
	TArray<QWORD> Remembered;
	QWORD SavedTop, SavedCurrent;
	static TArray<WProperties*> PropertiesWindows;

	// Structors.
	WProperties( FName InPersistentName )
	: WPropertiesBase( InPersistentName )
	{
		guard(WProperties::WProperties);
		PropertiesWindows.AddItem( this );
		unguard;
	}

	// WWindow interface.
	void DoDestroy()
	{
		guard(WWindow::DoDestroy);
		PropertiesWindows.RemoveItem( this );
		WWindow::DoDestroy();
		unguard;
	}
	void OnDestroy()
	{
		guard(WObjectProperties::OnDestroy);
		WWindow::OnDestroy();
		delete this;
		unguard;
	}
	void OpenWindow( HWND hWndParent=NULL )
	{
		guard(WProperties::OpenWindow);
		char Caption[256];
		GetRoot()->GetCaption( Caption );
		PerformCreateWindowEx
		(
			WS_EX_TOOLWINDOW,
			Caption,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			256+64+32,
			512,
			hWndParent,
			NULL,
			hInstance,
			NULL
		);
		List.OpenWindow( 1, 0, 0, 1 );
		unguard;
	}
	void OnActivate( UBOOL Active )
	{
		guard(WProperties::OnActivate);
		if( Active==1 )
		{
			SetFocus( List );
			if( !FocusItem )
				SetItemFocus( 1 );
		}
		else
		{
			SetItemFocus( 0 );
		}
		unguard;
	}
	void OnSize( DWORD Flags, INT NewX, INT NewY )
	{
		guard(WProperties::OnSize);
		SetItemFocus( 0 );
		WWindow::OnSize( Flags, NewX, NewY );
		InvalidateRect( List, NULL, FALSE );
		ResizeList();
		SetItemFocus( 1 );
		unguard;
	}
	LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		guard(WProperties::WndProc);
		if( Message==WM_PAINT )
		{
			if( GetUpdateRect( *this, NULL, 0 ) )
			{
				PAINTSTRUCT PS;
				HDC hDC = BeginPaint( *this, &PS );
				RECT Rect, ListRect;
				GetClientRect( *this, &Rect );
				GetClientRect( List, &ListRect );
				Rect.top = ListRect.bottom;
				FillRect( hDC, &Rect, (HBRUSH)(COLOR_BTNFACE+1) ); 
				EndPaint( *this, &PS );
			}
			return 0;
		}
		else if( Message==WM_MEASUREITEM )
		{
			MEASUREITEMSTRUCT* Info = (MEASUREITEMSTRUCT*)lParam;
			if( Info->itemData )
				Info->itemHeight = ((FTreeItem*)Info->itemData)->GetHeight();
			return 1;
		}
        else if( Message==WM_DRAWITEM )
		{
			DRAWITEMSTRUCT* Info=(DRAWITEMSTRUCT*)lParam;
			if( Info->itemData )
				((FTreeItem*)Info->itemData)->Draw( Info->hDC, Info->rcItem, (Info->itemState & ODS_SELECTED)!=0 );
			return 1;
		}
		else return WWindow::WndProc( Message, wParam, lParam );
		unguard;
	}
	virtual LONG CommandProc( UINT wParam, LONG lParam )
	{
		guard(WProperties::CommandProc);
        if( HIWORD(wParam)==LBN_DBLCLK )
		{
			if( FocusItem )
				FocusItem->OnItemDoubleClick();
			return 0;
		}
        else if( HIWORD(wParam)==LBN_SELCHANGE )
		{
			SetItemFocus( 1 );
			return 0;
		}
		else
		{
			return WPropertiesBase::CommandProc( wParam, lParam );
		}
		unguard;
	}

	// FControlSnoop interface.
	void SnoopLeftMouseDown( WControl* Src, INT X, INT Y )
	{
		guard(WProperties::SnoopLeftMouseDown);
		if( Src==&List )
		{
			INT Index = List.ItemFromPoint( X, Y );
			if( Index>=0 )
			{
				FTreeItem* Item = GetListItem(Index);
				RECT Rect = Item->GetRect();
				Item->OnItemLeftMouseDown( X-Rect.left, Y-Rect.top );
			}
		}
		unguard;
	}
	void SnoopThumbTrack()
	{
		guard(WProperties::SnoopThumbTrack);
		if( FocusItem )
			FocusItem->SnoopThumbTrack();
		unguard;
	}
	void SnoopThumbPosition()
	{
		guard(WProperties::SnoopThumbPosition);
		if( FocusItem )
			FocusItem->SnoopThumbPosition();
		unguard;
	}
	void SnoopChar( WControl* Src, INT Char )
	{
		guard(WProperties::SnoopChar);
		if( FocusItem )
			FocusItem->SnoopChar( Src, Char );
		unguard;
	}
	void SnoopKeyDown( WControl* Src, INT Char )
	{
		guard(WProperties::SnoopChar);
		if( Char==9 )
			PostMessage( List, WM_KEYDOWN, (GetKeyState(16)&0x8000)?VK_UP:VK_DOWN, 0 );
		WPropertiesBase::SnoopKeyDown( Src, Char );
		unguard;
	}
	void SnoopButtonClick( const char* Text )
	{
		guard(WProperties::SnoopButtonClick);
		if( FocusItem )
			FocusItem->SnoopButtonClick( Text );
		unguard;
	}

	// WPropertiesBase interface.
	INT GetDividerWidth()
	{
		guard(WProperties::GetDividerWidth);
		return 128;
		unguard;
	}

	// WProperties interface.
	virtual void SetValue( const char* Value )
	{
		guard(WProperties::SetValue);
		if( FocusItem )
			FocusItem->SetValue( Value );
		unguard;
	}
	virtual void SetItemFocus( UBOOL FocusCurrent )
	{
		guard(WProperties::SetItemFocus);
		if( FocusItem )
			FocusItem->OnItemKillFocus( 0 );
		FocusItem = NULL;
		if( FocusCurrent && List.GetCount()>0 )
			FocusItem = GetListItem( List.GetCurrent() );
		if( FocusItem )
			FocusItem->OnItemSetFocus();
		unguard;
	}
	virtual void ResizeList()
	{
		guard(WProperties::ResizeList);
		RECT ClientRect;
		GetClientRect( *this, &ClientRect ); 
		RECT R; R.top=R.bottom=R.left=R.right=0;
		for( INT i=List.GetCount()-1; i>=0; i-- )
			R.bottom += List.GetItemHeight( i );
		AdjustWindowRect( &R, GetWindowLong(List,GWL_STYLE), 0 );
		R.bottom+=4;//!!why?
		MoveWindow
		(
			List,
			0,
			0,
			ClientRect.right-ClientRect.left,
			Min(ClientRect.bottom-ClientRect.top,R.bottom-R.top),
			TRUE
		);
		unguard;
	}
	void ForceRefresh()
	{
		guard(WProperties::ForceRefresh);

		// Disable editing.
		SetItemFocus( 0 );

		// Remember which items were expanded.
		if( List.GetCount() )
		{
			Remembered.Empty();
			SavedTop=GetListItem(List.GetTop())->GetId();
			SavedCurrent=GetListItem(List.GetCurrent())->GetId();
			for( INT i=0; i<List.GetCount(); i++ )
			{
				FTreeItem* Item = GetListItem(i);
				if( Item->Expanded )
					Remembered.AddItem( Item->GetId() );
			}
		}

		// Empty it and add root items.
		List.Empty();
		GetRoot()->EmptyChildren();
		GetRoot()->Expanded=0;
		GetRoot()->Expand();

		// Restore expansion state of the items.
		INT CurrentIndex=-1, TopIndex=-1;
		for( INT i=0; i<List.GetCount(); i++ )
		{
			FTreeItem* Item = GetListItem(i);
			QWORD      Id   = Item->GetId();
			if( Item->Expandable && !Item->Expanded )
			{
				for( INT j=0; j<Remembered.Num(); j++ )
					if( Remembered(j)==Id )
						break;
				if( j<Remembered.Num() )
					Item->Expand();
			}
			if( Id==SavedTop     ) TopIndex     = i;
			if( Id==SavedCurrent ) CurrentIndex = i;
		}

		// Adjust list size.
		ResizeList();

		// Set indices.
		if( TopIndex>=0 ) List.SetTop( TopIndex );
		if( CurrentIndex>=0 ) List.SetCurrent( CurrentIndex, 1 );

		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FPropertyItemBase.
-----------------------------------------------------------------------------*/

class WINDOW_API FPropertyItemBase : public FHeaderItem
{
public:
	// Variables.
	FString Caption;
	DWORD FlagMask;
	UClass* BaseClass;

	// Structors.
	FPropertyItemBase( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const char* InCaption )
	:	FHeaderItem	( InOwnerProperties, InParent, 1 )
	,	Caption		( InCaption )
	,	FlagMask	( InFlagMask )
	,	BaseClass	( NULL )
	{}

	// FHeaderItem interface.
	UBOOL AcceptFlags( DWORD InFlags )
	{
		guard(FObjectsItem::AcceptFlags);
		return (InFlags&FlagMask)==FlagMask;
		unguard;
	}
	void GetStates( TArray<FName>& States )
	{
		guard(FObjectsItem::GetStates);
		if( BaseClass )
			for( TFieldIterator<UState> StateIt(BaseClass); StateIt; ++StateIt )
				if( StateIt->StateFlags & STATE_Editable )
					States.AddUniqueItem( StateIt->GetFName() );
		unguard;
	}
	void Collapse()
	{
		guard(FObjectsItem::Collapse);
		FTreeItem::Collapse();
		EmptyChildren();
		unguard;
	}
	HBRUSH GetBackgroundBrush()
	{
		guard(FObjectsItem::GetBackgroundBrush);
		return hBrushClass;
		unguard;
	}
	char* GetCaption( char* Result ) const
	{
		guard(FObjectsItem::GetCaption);
		appStrcpy( Result, *Caption );
		return Result;
		unguard;
	}
	QWORD GetId() const
	{
		guard(FObjectsItem::GetId);
		return (QWORD)BaseClass + (QWORD)4;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WObjectProperties.
-----------------------------------------------------------------------------*/

// Object properties root.
class WINDOW_API FObjectsItem : public FPropertyItemBase
{
public:
	// Variables.
	TArray<UObject*> _Objects;

	// Structors.
	FObjectsItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const char* InCaption )
	:	FPropertyItemBase( InOwnerProperties, InParent, InFlagMask, InCaption )
	{}

	// FTreeItem interface.
	BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
		guard(FObjectsItem::GetReadAddress);
		if( !_Objects.Num() )
			return NULL;
		for( INT i=1; i<_Objects.Num(); i++ )
			if( !Property->Matches( (BYTE*)_Objects(0)+Offset-Property->Offset, (BYTE*)_Objects(i)+Offset-Property->Offset, 0 ) )
				return NULL;
		return (BYTE*)_Objects(0) + Offset;
		unguard;
	}
	void SetProperty( UProperty* Property, INT Offset, const char* Value )
	{
		guard(FObjectsItem::SetProperty);
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPreChange( OwnerProperties );
		for( INT i=0; i<_Objects.Num(); i++ )
		{
			Property->ImportText( Value, (BYTE*)_Objects(i) + Offset, 1 );
			_Objects(i)->PostEditChange();
		}
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPostChange( OwnerProperties );
		unguard;
	}
	void Expand()
	{
		guard(FObjectsItem::Expand);
		TArray<FName> Categories;
		for( TFieldIterator<UProperty> It(BaseClass); It; ++It )
			if( AcceptFlags( It->PropertyFlags ) )
				Categories.AddUniqueItem( It->Category );
		for( INT i=0; i<Categories.Num(); i++ )
			Children.AddItem( new FCategoryItem(OwnerProperties,this,BaseClass,Categories(i),1) );
		FTreeItem::Expand();
		unguard;
	}

	// FHeaderItem interface.
	char* GetCaption( char* Result ) const
	{
		guard(FObjectsItem::GetCaption);

		if( Caption.Length() )
			appStrcpy( Result, *Caption );			
		else if( !BaseClass )
			appSprintf( Result, LocalizeGeneral("PropNone","Window") );
		else if( _Objects.Num()==1 )
			appSprintf( Result, LocalizeGeneral("PropSingle","Window"), BaseClass->GetName() );
		else
			appSprintf( Result, LocalizeGeneral("PropMulti","Window"), BaseClass->GetName(), _Objects.Num() );

		return Result;
		unguard;
	}

	// FObjectsItem interface.
	virtual void SetObjects( UObject** InObjects, INT Count )
	{
		guard(FObjectsItem::SetSingleObject);

		// Add objects and find lowest common base class.
		_Objects.Empty();
		BaseClass=NULL;
		for( INT i=0; i<Count; i++ )
		{
			if( InObjects[i] )
			{
				check(InObjects[i]->GetClass());
				if( BaseClass==NULL )	
					BaseClass=InObjects[i]->GetClass();
				while( !InObjects[i]->GetClass()->IsChildOf(BaseClass) )
					BaseClass = BaseClass->GetSuperClass();
				_Objects.AddItem( InObjects[i] );
			}
		}

		// Automatically title the window.
		char Caption[256];
		GetCaption( Caption );
		OwnerProperties->SetTextf( Caption );

		// Refresh all properties.
		if( Expanded || this==OwnerProperties->GetRoot() )
			OwnerProperties->ForceRefresh();

		unguard;
	}
};

// Multiple selection object properties.
class WINDOW_API WObjectProperties : public WProperties
{
	DECLARE_WINDOWCLASS(WObjectProperties,WProperties)

	// Variables.
	FObjectsItem Root;

	// Structors.
	WObjectProperties( FName InPersistentName, DWORD InFlagMask, const char* InCaption )
	:	WProperties	( InPersistentName )
	,	Root		( this, NULL, InFlagMask, InCaption )
	{}

	// WPropertiesBase interface.
	FTreeItem* GetRoot()
	{
		guard(WConfigProperties::GetRoot);
		return &Root;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WClassProperties.
-----------------------------------------------------------------------------*/

// Object properties root.
class WINDOW_API FClassItem : public FPropertyItemBase
{
public:
	// Structors.
	FClassItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const char* InCaption, UClass* InBaseClass )
	:	FPropertyItemBase( InOwnerProperties, InParent, InFlagMask, InCaption )
	{
		BaseClass = InBaseClass;
	}

	// FTreeItem interface.
	BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
		guard(FObjectsItem::GetReadAddress);
		return &BaseClass->Defaults(Offset);
		unguard;
	}
	void SetProperty( UProperty* Property, INT Offset, const char* Value )
	{
		guard(FObjectsItem::SetProperty);
		Property->ImportText( Value, &BaseClass->Defaults(Offset), 1 );
		BaseClass->SetFlags( RF_SourceModified );
		unguard;
	}
	void Expand()
	{
		guard(FObjectsItem::Expand);
		TArray<FName> Categories;
		for( TFieldIterator<UProperty> It(BaseClass); It; ++It )
			if( AcceptFlags( It->PropertyFlags ) )
				Categories.AddUniqueItem( It->Category );
		for( INT i=0; i<Categories.Num(); i++ )
			Children.AddItem( new FCategoryItem(OwnerProperties,this,BaseClass,Categories(i),1) );
		FTreeItem::Expand();
		unguard;
	}
};

// Multiple selection object properties.
class WINDOW_API WClassProperties : public WProperties
{
	DECLARE_WINDOWCLASS(WClassProperties,WProperties)

	// Variables.
	FClassItem Root;

	// Structors.
	WClassProperties( FName InPersistentName, DWORD InFlagMask, const char* InCaption, UClass* InBaseClass )
	:	WProperties	( InPersistentName )
	,	Root		( this, NULL, InFlagMask, InCaption, InBaseClass )
	{}

	// WPropertiesBase interface.
	FTreeItem* GetRoot()
	{
		guard(WClassProperties::GetRoot);
		return &Root;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	WConfigProperties.
-----------------------------------------------------------------------------*/

// Object configuration header.
class WINDOW_API FObjectConfigItem : public FPropertyItemBase
{
public:
	// Variables.
	FString  ClassName;
	FName    CategoryFilter;
	UClass*  Class;
	UBOOL	 Failed;
	UBOOL    Immediate;

	// Structors.
	FObjectConfigItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, const char* InCaption, const char* InClass, UBOOL InImmediate, FName InCategoryFilter )
	:	FPropertyItemBase( InOwnerProperties, InParent, CPF_Config, InCaption )
	,	ClassName		( InClass )
	,	Class			( NULL )
	,	Failed			( 0 )
	,	Immediate		( InImmediate )
	,	CategoryFilter	( InCategoryFilter )
	{}

	// FTreeItem interface.
	BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
		guard(FObjectsItem::GetReadAddress);
		check(Class);
		return &Class->Defaults(Offset);
		unguard;
	}
	void SetProperty( UProperty* Property, INT Offset, const char* Value )
	{
		guard(FObjectsItem::SetProperty);
		check(Class);
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPreChange( OwnerProperties );
		if( Cast<UClassProperty>(Property) && appStricmp(*Property->Category,"Drivers")==0 )
		{
			// Save it.
			UClassProperty* ClassProp = CastChecked<UClassProperty>( Property );
			TArray<FRegistryObjectInfo> Classes;
			GObj.GetRegistryObjects( Classes, UClass::StaticClass, ClassProp->MetaClass, 0 );
			for( INT i=0; i<Classes.Num(); i++ )
			{
				char Path[256], *Str;
				appStrcpy( Path, Classes(i).Object );
				Str = appStrstr(Path,".");
				const char* Text = Str ? (*Str++=0,Localize(Str,"ClassCaption",Path)) : Localize("Language","Language","Core",Path);
					if( appStricmp( Text, Value )==0 )
						SetConfigString( Property->GetOwnerClass()->GetPathName(), Property->GetName(), Classes(i).Object );
			}
		}
		else
		{
			// Regular property.
			GObj.GlobalSetProperty( Value, Class, Property, Offset, Immediate );
		}
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPostChange( OwnerProperties );
		unguard;
	}
	void SnoopButtonClick( const char* Text )
	{
		guard(FPropertyItem::SnoopButtonClick);
		if( appStricmp(Text,LocalizeGeneral("DefaultsButton","Window"))==0 )
		{
			LazyLoadClass();
			if( Class )
			{
				GObj.ResetConfig( Class );
				InvalidateRect( OwnerProperties->List, NULL, 1 );
				UpdateWindow( OwnerProperties->List );
			}
		}
		else if( Parent && Parent!=OwnerProperties->GetRoot() )
		{
			Parent->SnoopButtonClick( Text );
		}
		Redraw();
		unguard;
	}
	void OnItemSetFocus()
	{
		FPropertyItemBase::OnItemSetFocus();
		AddButton( LocalizeGeneral("DefaultsButton","Window") );
	}
	void Expand()
	{
		guard(FObjectsItem::Expand);
		LazyLoadClass();
		if( Class )
		{
			if( Children.Num()==0 )
			{
				Class->GetDefaultObject()->LoadConfig(NAME_Config);
				for( TFieldIterator<UProperty> It(Class); It; ++It )
				{
					if
					(	(AcceptFlags(It->PropertyFlags))
					&&	(CategoryFilter==NAME_None || It->Category==CategoryFilter) )
						Children.AddItem( new FPropertyItem( OwnerProperties, this, *It, It->GetFName(), It->Offset, -1 ) );
				}
			}
			FTreeItem::Expand();
		}
		else
		{
			Expandable = 0;
			Redraw();
		}
		unguard;
	}

	// FObjectConfigItem interface.
	void LazyLoadClass()
	{
		guard(FObjectConfigItem::LazyLoadClass);
		if( !Class && !Failed )
		{
			Class = GObj.LoadClass( UObject::StaticClass, NULL, *ClassName, NULL, LOAD_NoWarn | LOAD_KeepImports, NULL );
			if( !Class )
			{
				Failed = 1;
				Caption.Setf( LocalizeError("FailedConfigLoad","Window"), ClassName );
			}
		}
		unguard;
	}
};

// An configuration list item.
class WINDOW_API FConfigItem : public FHeaderItem
{
public:
	// Variables.
	FPreferencesInfo Prefs;

	// Constructors.
	FConfigItem( const FPreferencesInfo& InPrefs, WPropertiesBase* InOwnerProperties, FTreeItem* InParent )
	: FHeaderItem( InOwnerProperties, InParent, 1 )
	, Prefs( InPrefs )
	{}

	// FTreeItem interface.
	QWORD GetId() const
	{
		guard(FConfigItem::GetId);
		return (INT)this + ((QWORD)3<<32);
		unguard;
	}
	virtual char* GetCaption( char* Result ) const
	{
		guard(FConfigItem::GetText);
		appStrcpy( Result, Prefs.Caption );
		return Result;
		unguard;
	}
	virtual HBRUSH GetBackgroundBrush()
	{
		guard(FConfigItem::GetBackgroundBrush);
		return hBrushClass;
		unguard;
	}
	void Expand()
	{
		guard(FConfigItem::Expand);
		TArray<FPreferencesInfo> NewPrefs;
		GObj.GetPreferences( NewPrefs, Prefs.Caption, 0 );
		for( INT i=0; i<NewPrefs.Num(); i++ )
		{
			char Temp[256];
			for( INT j=0; j<Children.Num(); j++ )
			{
				if( appStricmp( Children(j)->GetCaption(Temp), NewPrefs(i).Caption )==0 )
					break;
			}
			if( j==Children.Num() )
			{
				if( *NewPrefs(i).Class )
					Children.AddItem( new FObjectConfigItem( OwnerProperties, this, NewPrefs(i).Caption, NewPrefs(i).Class, NewPrefs(i).Immediate, NewPrefs(i).Category ) );
				else
					Children.AddItem( new FConfigItem( NewPrefs(i), OwnerProperties, this ) );
			}
		}
		FTreeItem::Expand();
		unguard;
	}
	void Collapse()
	{
		guard(FConfigItem::Collapse);
		FTreeItem::Collapse();
		EmptyChildren();
		unguard;
	}
};

// Configuration properties.
class WINDOW_API WConfigProperties : public WProperties
{
	DECLARE_WINDOWCLASS(WConfigProperties,WProperties)

	// Variables.
	FConfigItem Root;

	// Structors.
	WConfigProperties( FName InPersistentName, const char* InTitle )
	:	WProperties	( InPersistentName )
	,	Root		( FPreferencesInfo(), this, NULL )
	{
		appStrcpy( Root.Prefs.Caption, InTitle );
	}

	// WPropertiesBase interface.
	FTreeItem* GetRoot()
	{
		guard(WConfigProperties::GetRoot);
		return &Root;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
