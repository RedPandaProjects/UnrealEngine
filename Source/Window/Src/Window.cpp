/*=============================================================================
	Window.cpp: GUI window management code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#pragma warning( disable : 4201 )
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include "Core.h"
#include "Window.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

WNDPROC WLabel::SuperProc;
WNDPROC WEdit::SuperProc;
WNDPROC WListBox::SuperProc;
WNDPROC WTrackBar::SuperProc;
WNDPROC WComboBox::SuperProc;
WNDPROC WButton::SuperProc;
TArray<WWindow*> WWindow::Windows;
TArray<WProperties*> WProperties::PropertiesWindows;

WINDOW_API HBRUSH hBrushProperty;
WINDOW_API HBRUSH hBrushCategory;
WINDOW_API HBRUSH hBrushObject;
WINDOW_API HBRUSH hBrushClass;
WINDOW_API HBRUSH hBrushStack;
WINDOW_API HBRUSH hBrushBlack;

IMPLEMENT_PACKAGE(Window)

/*-----------------------------------------------------------------------------
	Init windowing.
-----------------------------------------------------------------------------*/

void WINDOW_API InitWindowing()
{
	guard(InitWindowing);

	// Init common controls.
	InitCommonControls();

	// Implement window classes.
	IMPLEMENT_WINDOWSUBCLASS(WListBox,hInstance,"LISTBOX");
	IMPLEMENT_WINDOWSUBCLASS(WItemBox,hInstance,"LISTBOX");
	IMPLEMENT_WINDOWSUBCLASS(WLabel,hInstance,"STATIC");
	IMPLEMENT_WINDOWSUBCLASS(WEdit,hInstance,"EDIT");
	IMPLEMENT_WINDOWSUBCLASS(WComboBox,hInstance,"COMBOBOX");
	IMPLEMENT_WINDOWSUBCLASS(WEditTerminal,hInstance,"EDIT");
	IMPLEMENT_WINDOWSUBCLASS(WButton,hInstance,"BUTTON");
	IMPLEMENT_WINDOWSUBCLASS(WTrackBar,hInstance,TRACKBAR_CLASS);
	IMPLEMENT_WINDOWCLASS(WTerminal,hInstance,CS_DBLCLKS);
	IMPLEMENT_WINDOWCLASS(WLog,hInstance,CS_DBLCLKS);
	IMPLEMENT_WINDOWCLASS(WPasswordDialog,hInstance,CS_DBLCLKS);
	IMPLEMENT_WINDOWCLASS(WProperties,hInstance,CS_DBLCLKS);
	IMPLEMENT_WINDOWCLASS(WObjectProperties,hInstance,CS_DBLCLKS);
	IMPLEMENT_WINDOWCLASS(WConfigProperties,hInstance,CS_DBLCLKS);
	IMPLEMENT_WINDOWCLASS(WClassProperties,hInstance,CS_DBLCLKS);
	//"SCROLLBAR"
	//WC_HEADER (InitCommonControls)
	//WC_LISTVIEW (InitCommonControls)
	//WC_TABCONTROL (InitCommonControls)
	//WC_TREEVIEW (InitCommonControls)
	//PROGRESS_CLASS (InitCommonControls)
	//TOOLTIPS_CLASS (InitCommonControls)
	//TRACKBAR_CLASS (InitCommonControls)
	//UPDOWN_CLASS (InitCommonControls)
	//STATUSCLASSNAME (InitCommonControls)
	//TOOLBARCLASSNAME (InitCommonControls)
	//"RichEdit" (RICHED32.DLL)

	// Create brushes.
	COLORREF Co = GetSysColor( COLOR_BTNFACE );
	hBrushProperty = (HBRUSH)(COLOR_BTNFACE+1);
	hBrushCategory = CreateSolidBrush( RGB(GetRValue(Co),GetGValue(Co),Min(1.2*GetBValue(Co),255.0)) );
	hBrushObject   = CreateSolidBrush( RGB(Min(1.2*GetRValue(Co),255.0),GetGValue(Co),GetBValue(Co)) );
	hBrushClass    = CreateSolidBrush( RGB(GetRValue(Co),Min(1.2*GetGValue(Co),255.0),GetBValue(Co)) );
	hBrushStack    = CreateSolidBrush( RGB(Min(1.2*GetRValue(Co),255.0),Min(1.2*GetGValue(Co),255.0),GetBValue(Co)) );
	hBrushBlack    = CreateSolidBrush( RGB(0,0,0) );

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
