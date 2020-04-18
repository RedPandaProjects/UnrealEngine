/*=============================================================================
	UnWnCam.cpp: Unreal Windows-platform specific window manager implementation.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "WinDrvPrivate.h"
#include "UnRender.h"

#define DD_OTHERLOCKFLAGS 0 /*DDLOCK_NOSYSLOCK*/ /*0x00000800L*/
#define WM_MOUSEWHEEL 0x020A

IMPLEMENT_CLASS(UWindowsClient);

/*-----------------------------------------------------------------------------
	Getting error messages.
-----------------------------------------------------------------------------*/

//
// Convert a windows error message to text.
//
static char* winError()
{
	guard(winError);
	char* lpMsgBuf;
	FormatMessage
	( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	return lpMsgBuf;
	unguard;
}

//
// Try switching to a new rendering device.
//
static void TryRenderDevice( UViewport* Viewport, const char* ClassName, UBOOL Fullscreen )
{
	guard(TryRenderDevice);

	// Shut down current rendering device.
	if( Viewport->RenDev )
	{
		Viewport->RenDev->Exit();
		delete Viewport->RenDev;
		Viewport->RenDev = NULL;
	}

	// Find device driver.
	UClass* RenderClass = GObj.LoadClass( URenderDevice::StaticClass, NULL, ClassName, NULL, LOAD_KeepImports, NULL );
	if( RenderClass )
	{
		Viewport->RenDev = ConstructClassObject<URenderDevice>( RenderClass );
		if( Viewport->Client->Engine->Audio && !GIsEditor )
			Viewport->Client->Engine->Audio->SetViewport( NULL );
		if( Viewport->RenDev->Init( Viewport ) )
		{
			Viewport->Actor->XLevel->DetailChange( Viewport->RenDev->HighDetailActors );
			if( Fullscreen && !Viewport->Client->FullscreenViewport )
				Viewport->MakeFullscreen( Viewport->Client->ViewportX, Viewport->Client->ViewportY, 1 );
		}
		else
		{
			debugf( NAME_Log, LocalizeError("Failed3D") );
			delete Viewport->RenDev;
			Viewport->RenDev = NULL;
		}
		if( Viewport->Client->Engine->Audio && !GIsEditor )
			Viewport->Client->Engine->Audio->SetViewport( Viewport );
	}
	unguard;
}

//
// DirectDraw mode enumeration callback.
//
static HRESULT WINAPI ddEnumModesCallback( DDSURFACEDESC* SurfaceDesc, void* Context )
{
	guard(ddEnumModesCallback);

	UWindowsClient* Client = (UWindowsClient *)Context;
	if( Client->ddNumModes < Client->DD_MAX_MODES )
	{
		Client->ddModeWidth [Client->ddNumModes] = SurfaceDesc->dwWidth;
		Client->ddModeHeight[Client->ddNumModes] = SurfaceDesc->dwHeight;
		for( INT i=0; i<Client->ddNumModes; i++ )
			if
			(	((DWORD)Client->ddModeWidth [i]==SurfaceDesc->dwWidth)
			&&	((DWORD)Client->ddModeHeight[i]==SurfaceDesc->dwHeight))
				break;
		if( i==Client->ddNumModes )
			Client->ddNumModes++;
	}
	return DDENUMRET_OK;

	unguard;
}

/*-----------------------------------------------------------------------------
	UWindowsViewport implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UWindowsViewport::UWindowsViewport( ULevel* InLevel, UWindowsClient* InClient )
:	UViewport( InLevel, InClient )
,	Client( (UWindowsClient*)InClient )
{
	guard(UWindowsViewport::UWindowsViewport);

	// Set color bytes based on screen resolution.
	HWND hwndDesktop = GetDesktopWindow();
	HDC  hdcDesktop  = GetDC(hwndDesktop);
	switch( GetDeviceCaps( hdcDesktop, BITSPIXEL ) )
	{
		case 8:
			ColorBytes  = 2;
			break;
		case 16:
			ColorBytes  = 2;
			Caps       |= CC_RGB565;
			break;
		case 24:
			ColorBytes  = 4;
			break;
		case 32: 
			ColorBytes  = 4;
			break;
		default: 
			ColorBytes  = 2; 
			Caps       |= CC_RGB565;
			break;
	}

	// Init other stuff.
	ReleaseDC( hwndDesktop, hdcDesktop );
	Status = WIN_ViewportOpening;
	SavedCursor.x = -1;

	// Init input.
	if( GIsEditor )
		Input->Init( this, GSystem );

	unguard;
}

//
// Set the mouse cursor according to Unreal or UnrealEd's mode, or to
// an hourglass if a slow task is active.
//
void UWindowsViewport::SetModeCursor()
{
	guard(UWindowsViewport::SetModeCursor);
	enum EEditorMode
	{
		EM_None 			= 0,
		EM_ViewportMove		= 1,
		EM_ViewportZoom		= 2,
		EM_BrushRotate		= 5,
		EM_BrushSheer		= 6,
		EM_BrushScale		= 7,
		EM_BrushStretch		= 8,
		EM_TexturePan		= 11,
		EM_TextureRotate	= 13,
		EM_TextureScale		= 14,
		EM_BrushSnap		= 18,
		EM_TexView			= 19,
		EM_TexBrowser		= 20,
		EM_MeshView			= 21,
	};
	if( GIsSlowTask )
	{
		SetCursor(LoadCursor(NULL,IDC_WAIT));
		return;
	}
	HCURSOR hCursor;
	switch( Client->Engine->edcamMode(this) )
	{
		case EM_None: 			hCursor = LoadCursor(NULL,IDC_CROSS); break;
		case EM_ViewportMove: 	hCursor = LoadCursor(NULL,IDC_CROSS); break;
		case EM_ViewportZoom:	hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_CameraZoom)); break;
		case EM_BrushRotate:	hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_BrushRot)); break;
		case EM_BrushSheer:		hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_BrushSheer)); break;
		case EM_BrushScale:		hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_BrushScale)); break;
		case EM_BrushStretch:	hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_BrushStretch)); break;
		case EM_BrushSnap:		hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_BrushSnap)); break;
		case EM_TexturePan:		hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_TexPan)); break;
		case EM_TextureRotate:	hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_TexRot)); break;
		case EM_TextureScale:	hCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDCURSOR_TexScale)); break;
		case EM_TexView:		hCursor = LoadCursor(NULL,IDC_ARROW); break;
		case EM_TexBrowser:		hCursor = LoadCursor(NULL,IDC_ARROW); break;
		case EM_MeshView:		hCursor = LoadCursor(NULL,IDC_CROSS); break;
		default: 				hCursor = LoadCursor(NULL,IDC_ARROW); break;
	}
	check(hCursor);
	SetCursor (hCursor);
	unguard;
}

//
// Update user viewport interface.
//
void UWindowsViewport::UpdateWindow()
{
	guard(UWindowsViewport::UpdateViewportWindow);

	// If not a window, exit.
	if( hWnd==NULL || OnHold )
		return;

	// Set viewport window's name to show resolution.
	char WindowName[80];
	if( !GIsEditor || (Actor->ShowFlags&SHOW_PlayerCtrl) )
	{
		appSprintf( WindowName, LocalizeGeneral("Product","Core") );
	}
	else switch( Actor->RendMap )
	{
		case REN_Wire:		strcpy(WindowName,LocalizeGeneral("ViewPersp")); break;
		case REN_OrthXY:	strcpy(WindowName,LocalizeGeneral("ViewXY")); break;
		case REN_OrthXZ:	strcpy(WindowName,LocalizeGeneral("ViewXZ")); break;
		case REN_OrthYZ:	strcpy(WindowName,LocalizeGeneral("ViewYZ")); break;
		default:			strcpy(WindowName,LocalizeGeneral("ViewOther")); break;
	}

	// Set window title.
	if( SizeX && SizeY )
	{
		appSprintf(WindowName+strlen(WindowName)," (%i x %i)",SizeX,SizeY);
		if( this == Client->CurrentViewport() )
			strcat( WindowName, " *" );
	}
	SetWindowText( hWnd, WindowName );

	// Set the menu.
	if( Actor->ShowFlags & SHOW_Menu )
	{
		UBOOL MustUpdate = !GetMenu(hWnd);
		SetMenu( hWnd, hMenu );
		if( MustUpdate )
			FindAvailableModes();
	}
	else SetMenu( hWnd, NULL );

	// Update menu, Map rendering.
	CheckMenuItem(hMenu,ID_MAP_PLAINTEX, (Actor->RendMap==REN_PlainTex  ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_DYNLIGHT, (Actor->RendMap==REN_DynLight  ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_WIRE,     (Actor->RendMap==REN_Wire      ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_OVERHEAD, (Actor->RendMap==REN_OrthXY    ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_XZ, 		 (Actor->RendMap==REN_OrthXZ    ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_YZ, 		 (Actor->RendMap==REN_OrthYZ    ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_POLYS,    (Actor->RendMap==REN_Polys     ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_POLYCUTS, (Actor->RendMap==REN_PolyCuts  ? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_MAP_ZONES,    (Actor->RendMap==REN_Zones     ? MF_CHECKED:MF_UNCHECKED));

	// Show-attributes.
	CheckMenuItem(hMenu,ID_SHOW_BRUSH,    ((Actor->ShowFlags&SHOW_Brush			)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_SHOW_BACKDROP, ((Actor->ShowFlags&SHOW_Backdrop  		)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_SHOW_COORDS,   ((Actor->ShowFlags&SHOW_Coords    		)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_SHOW_MOVINGBRUSHES,((Actor->ShowFlags&SHOW_MovingBrushes)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_SHOW_PATHS,    ((Actor->ShowFlags&SHOW_Paths)?MF_CHECKED:MF_UNCHECKED));

	// Actor showing.
	CheckMenuItem(hMenu,ID_ACTORS_ICONS, MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_ACTORS_RADII, MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_ACTORS_SHOW,  MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_ACTORS_HIDE,  MF_UNCHECKED);

	// Actor options.
	DWORD ShowFilter = Actor->ShowFlags & (SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii);
	if		(ShowFilter==(SHOW_Actors | SHOW_ActorIcons)) CheckMenuItem(hMenu,ID_ACTORS_ICONS,MF_CHECKED);
	else if (ShowFilter==(SHOW_Actors | SHOW_ActorRadii)) CheckMenuItem(hMenu,ID_ACTORS_RADII,MF_CHECKED);
	else if (ShowFilter==(SHOW_Actors                  )) CheckMenuItem(hMenu,ID_ACTORS_SHOW,MF_CHECKED);
	else CheckMenuItem(hMenu,ID_ACTORS_HIDE,MF_CHECKED);

	// Color depth.
	CheckMenuItem(hMenu,ID_COLOR_16BIT,((ColorBytes==2)?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_COLOR_32BIT,((ColorBytes==4)?MF_CHECKED:MF_UNCHECKED));

	unguard;
}

//
// Open a viewport window.
//
void UWindowsViewport::OpenWindow( DWORD InParentWindow, UBOOL Temporary, INT NewX, INT NewY, INT OpenX, INT OpenY )
{
	guard(UWindowsViewport::OpenWindow);
	check(Actor);
	check(!OnHold);
	UBOOL DoRepaint=0, DoSetActive=0;
	NewX = Align(NewX,4);

	// User window of launcher if no parent window was specified.
	if( !InParentWindow )
		Parse( appCmdLine(), "HWND=", InParentWindow );

	if( Temporary )
	{
		// Create in-memory data.
		ColorBytes = 2;
		SizeX = NewX;
		SizeY = NewY;
		ScreenPointer = (BYTE*)appMalloc( 2 * NewX * NewY, "TemporaryViewportData" );	
		hWnd = (HWND)NULL;
		debugf( NAME_Log, "Opened temporary viewport" );
   	}
	else
	{
		// Figure out size we must specify to get appropriate client area.
		RECT rTemp;
		rTemp.left   = 100;
		rTemp.top    = 100;
		rTemp.right  = NewX + 100;
		rTemp.bottom = NewY + 100;

		// Get style and proper rectangle.
		DWORD Style;
		if( InParentWindow && (Actor->ShowFlags & SHOW_ChildWindow) )
		{
			Style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS;
   			AdjustWindowRect( &rTemp, Style, 0 );
		}
		else
		{
			Style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX; //Plus WS_THICKFRAME for scalable.
   			AdjustWindowRect( &rTemp, Style, (Actor->ShowFlags & SHOW_Menu) ? TRUE : FALSE );
		}

		// Set position and size.
		if( OpenX==-1 )
			OpenX = CW_USEDEFAULT;
		if( OpenY==-1 )
			OpenY = CW_USEDEFAULT;
		INT OpenXL = rTemp.right  - rTemp.left; 
		INT OpenYL = rTemp.bottom - rTemp.top;

		// Create or update the window.
		if( !hWnd )
		{
			// Creating new viewport.
			ParentWindow	= (HWND)InParentWindow;
			Status			= WIN_ViewportOpening;
			hMenu			= LoadMenu( hInstance, MAKEINTRESOURCE(GIsEditor?IDR_EDITORCAM:IDR_PLAYERCAM) );
			if( ParentWindow && (Actor->ShowFlags & SHOW_ChildWindow) )
			{
				DeleteMenu( hMenu, ID_WIN_TOP, MF_BYCOMMAND );
			}

			// Open the physical window.
			PerformCreateWindowEx
			(
				WS_EX_APPWINDOW,
				"",
				Style,
				OpenX, OpenY,
				OpenXL, OpenYL,
				ParentWindow,
				hMenu,
				hInstance,
				NULL
			);

			// Set parent window.
			if( ParentWindow && (Actor->ShowFlags & SHOW_ChildWindow) )
			{
				// Force this to be a child.
				SetWindowLong( hWnd, GWL_STYLE, WS_VISIBLE | WS_POPUP );
				if( Actor->ShowFlags & SHOW_Menu )
					SetMenu( hWnd, hMenu );
			}
			debugf( NAME_Log, "Opened viewport" );
			DoSetActive = DoRepaint = 1;
		}
		else
		{
			// Resizing existing viewport.
			SetWindowPos( hWnd, NULL, OpenX, OpenY, OpenXL, OpenYL, SWP_NOACTIVATE );
		}
		ShowWindow( hWnd, SW_SHOWNORMAL );
		FindAvailableModes();
	}
	if( !RenDev )
	{
		if( !GIsEditor && !ParseParam(appCmdLine(),"nohard") )
			TryRenderDevice( this, "ini:Engine.Engine.GameRenderDevice", Client->StartupFullscreen );
		if( !RenDev )
			TryRenderDevice( this, "ini:Engine.Engine.WindowedRenderDevice", 0 );
		check(RenDev);
	}
	if( !Temporary )
		UpdateWindow();
	if( DoRepaint )
		Repaint();
	if( DoSetActive )
		SetActiveWindow( hWnd );
	unguard;
}

//
// Close a viewport window.  Assumes that the viewport has been openened with
// OpenViewportWindow.  Does not affect the viewport's object, only the
// platform-specific information associated with it.
//
void UWindowsViewport::CloseWindow()
{
	guard(UWindowsViewport::CloseWindow);

	if( hWnd && Status==WIN_ViewportNormal )
	{
		Status = WIN_ViewportClosing;
		DestroyWindow( hWnd );
	}
	unguard;
}

//
// Lock the viewport window and set the approprite Screen and RealScreen fields
// of Viewport.  Returns 1 if locked successfully, 0 if failed.  Note that a
// lock failing is not a critical error; it's a sign that a DirectDraw mode
// has ended or the user has closed a viewport window.
//
UBOOL UWindowsViewport::Lock( FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData, INT* HitSize )
{
	guard(UWindowsViewport::LockWindow);
	clock(Client->DrawCycles);

	// Make sure window is lockable.
	if( hWnd && !IsWindow(hWnd) )
	{
      	return 0;
    }
	if( OnHold || !SizeX || !SizeY )
	{
		appErrorf( "Failed locking viewport" );
		return 0;
	}

	// Obtain pointer to screen.
	Stride = SizeX;
	if( Client->FullscreenViewport && Client->FullscreenhWndDD )
	{
		HRESULT Result;
  		if( Client->ddFrontBuffer->IsLost() == DDERR_SURFACELOST )
		{
			Result = Client->ddFrontBuffer->Restore();
   			if( Result != DD_OK )
				debugf( NAME_Log, "DirectDraw Lock Restore failed %s", Client->ddError(Result) );
			Client->EndFullscreen();
			return 0;
		}
		ZeroMemory( &Client->ddSurfaceDesc, sizeof(Client->ddSurfaceDesc) );
  		Client->ddSurfaceDesc.dwSize = sizeof(Client->ddSurfaceDesc);
		Result = Client->ddBackBuffer->Lock( NULL, &Client->ddSurfaceDesc, DDLOCK_WAIT|DD_OTHERLOCKFLAGS, NULL );
  		if( Result != DD_OK )
		{
			debugf( NAME_Log, "DirectDraw Lock failed: %s", Client->ddError(Result) );
  			return 0;
		}
		if( Client->ddSurfaceDesc.lPitch )
			Stride = Client->ddSurfaceDesc.lPitch/ColorBytes;
		ScreenPointer = (BYTE*)Client->ddSurfaceDesc.lpSurface;
	}
	check(ScreenPointer);

	// Success.
	unclock(Client->DrawCycles);
	return UViewport::Lock(FlashScale,FlashFog,ScreenClear,RenderLockFlags,HitData,HitSize);
	unguard;
}

//
// Unlock the viewport window.  If Blit=1, blits the viewport's frame buffer.
//
void UWindowsViewport::Unlock( UBOOL Blit )
{
	guard(UWindowsViewport::Unlock);
	Client->DrawCycles=0;
	clock(Client->DrawCycles);

	// Unlock base.
	UViewport::Unlock( Blit );

	// Unlock DirectDraw.
	if( Client->FullscreenViewport && Client->FullscreenhWndDD )
	{
		HRESULT Result;
		Result = Client->ddBackBuffer->Unlock( Client->ddSurfaceDesc.lpSurface );
		if( Result ) 
		 	appErrorf( "DirectDraw Unlock: %s", Client->ddError(Result) );
	}

	// Blit, if desired.
	if( Blit && hWnd && IsWindow(hWnd) && !OnHold )
	{
		if( Client->FullscreenViewport==this && Client->FullscreenhWndDD )
		{
			// Blitting with DirectDraw.
			HRESULT Result = Client->ddFrontBuffer->Flip( NULL, DDFLIP_WAIT );
			if( Result != DD_OK )
				appErrorf( "DirectDraw Flip failed: %s", Client->ddError(Result) );
		}
		else if( RenDev->FrameBuffered && hBitmap )
		{
			// Blitting with CreateDIBSection.
			HDC hDC = GetDC( hWnd );
			if( hDC == NULL )
				appErrorf( "GetDC failed: %s", winError() );
			if( SelectObject( Client->hMemScreenDC, hBitmap ) == NULL )
				appErrorf( "SelectObject failed: %s", winError() );
			if( BitBlt( hDC, 0, 0, SizeX, SizeY, Client->hMemScreenDC, 0, 0, SRCCOPY ) == NULL )
				appErrorf( "BitBlt failed: %s", winError() );
			if( ReleaseDC( hWnd, hDC ) == NULL )
				appErrorf( "ReleaseDC failed: %s", winError() );
		}
	}
	unclock(Client->DrawCycles);
	unguard;
}

//
// Update input for viewport.
//
UBOOL UWindowsViewport::JoystickInputEvent( FLOAT Delta, EInputKey Key, FLOAT Scale, UBOOL DeadZone )
{
	guard(UWindowsViewport::JoystickInputEvent);
	Delta = (Delta-32768.0)/32768.0;
	if( DeadZone )
	{
		if( Delta > 0.2 )
			Delta = (Delta - 0.2) / 0.8;
		else if( Delta < -0.2 )
			Delta = (Delta + 0.2) / 0.8;
		else
			Delta = 0.0;
	}
	return CauseInputEvent( Key, IST_Axis, Scale * Delta );
	unguard;
}
void UWindowsViewport::UpdateInput( UBOOL Reset )
{
	guard(UWindowsViewport::UpdateInput);
	BYTE Processed[256];
	appMemset( Processed, 0, 256 );

	// Joystick.
	if( Client->JoyCaps.wNumButtons )
	{
		JOYINFOEX JoyInfo;
		ZeroMemory( &JoyInfo, sizeof(JoyInfo) );
		JoyInfo.dwSize = sizeof(JoyInfo);
		JoyInfo.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNCENTERED | JOY_RETURNPOV | JOY_RETURNR | JOY_RETURNU | JOY_RETURNV | JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ;
		MMRESULT Result = joyGetPosEx( JOYSTICKID1, &JoyInfo );
		if( Result==JOYERR_NOERROR )
		{ 
			// Pass buttons to app.
			INT Index=0;
			for( Index=0; JoyInfo.dwButtons; Index++,JoyInfo.dwButtons/=2 )
			{
				if( !Input->KeyDown(Index) && (JoyInfo.dwButtons & 1) )
					CauseInputEvent( IK_Joy1+Index, IST_Press );
				else if( Input->KeyDown(Index) && !(JoyInfo.dwButtons & 1) )
					CauseInputEvent( IK_Joy1+Index, IST_Release );
				Processed[IK_Joy1+Index] = 1;
			}

			// Pass axes to app.
			JoystickInputEvent( JoyInfo.dwXpos, IK_JoyX, Client->ScaleXYZ, Client->DeadZoneXYZ );
			JoystickInputEvent( JoyInfo.dwYpos, IK_JoyY, Client->ScaleXYZ * (Client->InvertVertical ? 1.0 : -1.0), Client->DeadZoneXYZ );
			if( Client->JoyCaps.wCaps & JOYCAPS_HASZ )
				JoystickInputEvent( JoyInfo.dwZpos, IK_JoyZ, Client->ScaleXYZ, Client->DeadZoneXYZ );
			if( Client->JoyCaps.wCaps & JOYCAPS_HASR )
				JoystickInputEvent( JoyInfo.dwRpos, IK_JoyR, Client->ScaleRUV, Client->DeadZoneRUV );
			if( Client->JoyCaps.wCaps & JOYCAPS_HASU )
				JoystickInputEvent( JoyInfo.dwUpos, IK_JoyU, Client->ScaleRUV, Client->DeadZoneRUV );
			if( Client->JoyCaps.wCaps & JOYCAPS_HASV )
				JoystickInputEvent( JoyInfo.dwVpos, IK_JoyV, Client->ScaleRUV * (Client->InvertVertical ? 1.0 : -1.0), Client->DeadZoneRUV );
			if( Client->JoyCaps.wCaps & (JOYCAPS_POV4DIR|JOYCAPS_POVCTS) )
			{
				if( JoyInfo.dwPOV==JOY_POVFORWARD )
				{
					if( !Input->KeyDown(IK_JoyPovUp) )
						CauseInputEvent( IK_JoyPovUp, IST_Press );
					Processed[IK_JoyPovUp] = 1;
				}
				else if( JoyInfo.dwPOV==JOY_POVBACKWARD )
				{
					if( !Input->KeyDown(IK_JoyPovDown) )
						CauseInputEvent( IK_JoyPovDown, IST_Press );
					Processed[IK_JoyPovDown] = 1;
				}
				else if( JoyInfo.dwPOV==JOY_POVLEFT )
				{
					if( !Input->KeyDown(IK_JoyPovLeft) )
						CauseInputEvent( IK_JoyPovLeft, IST_Press );
					Processed[IK_JoyPovLeft] = 1;
				}
				else if( JoyInfo.dwPOV==JOY_POVRIGHT )
				{
					if( !Input->KeyDown(IK_JoyPovRight) )
						CauseInputEvent( IK_JoyPovRight, IST_Press );
					Processed[IK_JoyPovRight] = 1;
				}
			}
		}
		else
		{
			debugf( "Joystick read failed" );
			Client->JoyCaps.wNumButtons = 0;
		}
	}

	// Keyboard.
	Reset = Reset && GetFocus()==hWnd;
	for( INT i=0; i<256; i++ )
	{
		if( !Processed[i] )
		{
			if( !Input->KeyDown(i) )
			{
				if( Reset && (GetAsyncKeyState(i) & 0x8000) )
					CauseInputEvent( i, IST_Press );
			}
			else
			{
				if( !(GetAsyncKeyState(i) & 0x8000) )
					CauseInputEvent( i, IST_Release );
			}
		}
	}

	unguard;
}

//
// Make this viewport the current one.
// If Viewport=0, makes no viewport the current one.
//
void UWindowsViewport::MakeCurrent()
{
	guard(UWindowsViewport::MakeCurrent);
	Current = 1;
	for( INT i=0; i<Client->Viewports.Num(); i++ )
	{
		UViewport* OldViewport = Client->Viewports(i);
		if( OldViewport->Current && OldViewport!=this )
		{
			OldViewport->Current = 0;
			OldViewport->UpdateWindow();
		}
	}
	UpdateWindow();	
	unguard;
}

//
// Return the viewport's window.
//
void* UWindowsViewport::GetWindow()
{
	guard(UWindowsViewport::GetWindow);
	return hWnd;
	unguard;
}

//
// Find all available DirectDraw modes for a certain number of color bytes.
//
void UWindowsViewport::FindAvailableModes()
{
	guard(UWindowsViewport::FindAvailableModes);

	// Make sure we have a menu.
	HMENU hMenu;
	if( !hWnd )
		return;
	hMenu = GetMenu(hWnd);
	if( !hMenu )
		return;

	// Get list of DirectDraw modes.
	Client->ddNumModes = 0;
	if( Client->dd )
	{
		/*
		HWND hWndFocus = GetFocus();
		HRESULT Result;
		Result = Client->dd->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT | DDSCL_NOWINDOWCHANGES );
		if( Result != DD_OK )
		{
			debugf( NAME_Warning, "SetCooperativeLevel failed: %s", Client->ddError(Result) );
			return;
		}
		*/
		DDSURFACEDESC SurfaceDesc; 
		memset( &SurfaceDesc, 0, sizeof(DDSURFACEDESC) );
		SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
		SurfaceDesc.dwFlags = DDSD_PIXELFORMAT;
		SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
		SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = ColorBytes*8;
		Client->dd->EnumDisplayModes( 0, &SurfaceDesc, Client, ddEnumModesCallback );
		for( INT i=0; i<Client->ddNumModes; i++ )
			for( INT j=0; j<i; j++ )
				if( Client->ddModeWidth[j]>Client->ddModeWidth[i] || (Client->ddModeWidth[j]==Client->ddModeWidth[i] && Client->ddModeHeight[j]>Client->ddModeHeight[i]) )
					{Exchange(Client->ddModeWidth[i],Client->ddModeWidth[j]); Exchange(Client->ddModeHeight[i],Client->ddModeHeight[j]);}
		/*
		Result = Client->dd->SetCooperativeLevel( hWnd, DDSCL_NORMAL );
		if( Result != DD_OK )
			appErrorf( "SetCooperativeLevel failed: %s", Client->ddError(Result) );
		SetFocus( hWndFocus );
		*/
	}

	// Get menus.
	INT nMenu = GIsEditor ? 3 : 2;
	HMENU hSizes = GetSubMenu( hMenu, nMenu );
	check(hSizes);

	// Completely rebuild the "Size" submenu based on what modes are available.
	int n=GetMenuItemCount( hSizes );
	for( int i=0; i<n; i++ )
		if( !DeleteMenu(hSizes,0,MF_BYPOSITION) )
			appErrorf( "DeleteMenu failed: %s", winError() );

	// Add color depth items.
	AppendMenu( hSizes, MF_STRING, ID_COLOR_16BIT, LocalizeGeneral("Color16") );
	AppendMenu( hSizes, MF_STRING, ID_COLOR_32BIT, LocalizeGeneral("Color32") );

	// Add resolution items.
	if( !(Actor->ShowFlags & SHOW_ChildWindow) )
	{
		// Windows resolution items.
		AppendMenu( hSizes, MF_SEPARATOR, 0, NULL );
		AppendMenu( hSizes, MF_STRING, ID_WIN_320, "320x200" );
		AppendMenu( hSizes, MF_STRING, ID_WIN_400, "400x300" );
		AppendMenu( hSizes, MF_STRING, ID_WIN_512, "512x384" );
		AppendMenu( hSizes, MF_STRING, ID_WIN_640, "640x400" );
		AppendMenu( hSizes, MF_STRING, ID_WIN_640_480, "640x480" );
		AppendMenu( hSizes, MF_STRING, ID_WIN_800, "800x600" );

		// DirectDraw resolution items.
		if( Client->ddNumModes > 0 )
		{
			AppendMenu( hSizes, MF_SEPARATOR, 0, NULL );
			for( i=0; i<Client->ddNumModes; i++ )
			{
				char Text[256];
				appSprintf( Text, "Fullscreen %ix%i", Client->ddModeWidth[i], Client->ddModeHeight[i] );
				if( !AppendMenu( hSizes, MF_STRING, ID_DDMODE0+i, Text ) ) 
					appErrorf( "AppendMenu failed: %s", winError() );
			}
		}
		DrawMenuBar( hWnd );
	}
	unguard;
}

//
// Input event router.
//
UBOOL UWindowsViewport::CauseInputEvent( INT iKey, EInputAction Action, FLOAT Delta )
{
	guard(UWindowsViewport::CauseInputEvent);

	// Route to engine if a valid key; some keyboards produce key
	// codes that go beyond IK_MAX.
	if( iKey>=0 && iKey<IK_MAX )
		return Client->Engine->InputEvent( this, (EInputKey)iKey, Action, Delta );
	else
		return 0;

	unguard;
}

//
// If the cursor is currently being captured, stop capturing, clipping, and 
// hiding it, and move its position back to where it was when it was initially
// captured.
//
void UWindowsViewport::SetMouseCapture( UBOOL Capture, UBOOL Clip, UBOOL OnlyFocus )
{
	guard(UWindowsViewport::SetMouseCapture);

	// If only focus, reject.
	if( OnlyFocus )
		if( hWnd != GetFocus() )
			return;

	// If capturing, windows requires clipping in order to keep focus.
	Clip |= Capture;

	// Get window rectangle.
	RECT TempRect;
	GetClientRect( hWnd, &TempRect );
	MapWindowPoints( hWnd, NULL, (POINT*)&TempRect, 2 );

	// Handle capturing.
	if( Capture )
	{
		if( SavedCursor.x == -1 )
		{
			// Confine cursor to window.
			GetCursorPos( &SavedCursor );
			SetCursorPos( (TempRect.left+TempRect.right)/2, (TempRect.top+TempRect.bottom)/2 );

			// Start capturing cursor.
			SetCapture( hWnd );
			SystemParametersInfo( SPI_SETMOUSE, 0, Client->CaptureMouseInfo, 0 );
			ShowCursor( FALSE );
		}
	}
	else
	{
		// Release captured cursor.
		if( !Client->FullscreenViewport )
		{
			SetCapture( NULL );
			SystemParametersInfo( SPI_SETMOUSE, 0, Client->NormalMouseInfo, 0 );
		}

		// Restore position.
		if( SavedCursor.x != -1 )
		{
			SetCursorPos( SavedCursor.x, SavedCursor.y );
			SavedCursor.x = -1;
			while( ShowCursor(TRUE)<0 );
		}
	}

	// Handle clipping.
	ClipCursor( Clip ? &TempRect : NULL );

	unguard;
}

//
// Set the client size (viewport view size) of a viewport.
//
void UWindowsViewport::SetClientSize( INT NewX, INT NewY, UBOOL UpdateProfile )
{
	guard(UWindowsViewport::SetClientSize);

	// Stop dragging.
	SetDrag( 0 );

	// Compute new window size based on client size.
	UBOOL HasMenu = (Actor->ShowFlags & SHOW_Menu) ? TRUE : FALSE;
	RECT rWindow,rClient;
	GetWindowRect(hWnd,&rWindow);
	rClient.top		= 0;
	rClient.left	= 0;
	rClient.bottom	= NewY;
	rClient.right	= NewX;
	AdjustWindowRect( &rClient, GetWindowLong(hWnd,GWL_STYLE), HasMenu );

	// Resize the window and repaint it.
	MoveWindow
	(
   		hWnd,
		rWindow.left,
		rWindow.top,
		rClient.right-rClient.left,
		rClient.bottom-rClient.top,
		TRUE
	);

	// Optionally save this size in the profile.
	if( UpdateProfile )
	{
		Client->ViewportX = NewX;
		Client->ViewportY = NewY;
		Client->SaveConfig();
	}

	unguard;
}

//
// Repaint the viewport.
//
void UWindowsViewport::Repaint()
{
	guard(UWindowsViewport::Repaint);
	if( !OnHold && RenDev && SizeX && SizeY )
		Client->Engine->Draw( this, 0 );
	unguard;
}

//
// Resize the viewport's frame buffer. Unconditional.
//
void UWindowsViewport::SetFrameBufferSize( INT NewX, INT NewY, INT NewColorBytes, EWindowsBlitType BlitType )
{
	guard(UWindowsViewport::SetFrameBufferSize);
	check(!OnHold);

	// Set size.
	SizeX = NewX;
	SizeY = NewY;
	ColorBytes = NewColorBytes;

	// Free old stuff.
	if( hBitmap )
		DeleteObject( hBitmap );
	hBitmap = NULL;

	// Init BitmapHeader for DIB.
	BitmapHeader.biSize				= sizeof(BITMAPINFOHEADER);
	BitmapHeader.biWidth			= (int)SizeX;
	BitmapHeader.biHeight			= -(int)SizeY;
	BitmapHeader.biPlanes			= 1;
	BitmapHeader.biBitCount			= ColorBytes * 8;
	BitmapHeader.biSizeImage		= SizeX * SizeY * ColorBytes;
	BitmapHeader.biXPelsPerMeter	= 0;
	BitmapHeader.biYPelsPerMeter	= 0;
	BitmapHeader.biClrUsed			= 0;
	BitmapHeader.biClrImportant		= 0;

	// Handle color depth.
	if( ColorBytes==2 && (Caps & CC_RGB565) )
	{
		// 16-bit color (565).
		BitmapHeader.biCompression = BI_BITFIELDS;
		*(DWORD *)&BitmapColors[0] = 0xF800;
		*(DWORD *)&BitmapColors[1] = 0x07E0;
		*(DWORD *)&BitmapColors[2] = 0x001F;
	}
	else if( ColorBytes==2 )
	{
		BitmapHeader.biCompression = BI_BITFIELDS;
		*(DWORD *)&BitmapColors[0] = 0x7C00;
		*(DWORD *)&BitmapColors[1] = 0x03E0;
		*(DWORD *)&BitmapColors[2] = 0x001F;
	}
	else if( ColorBytes==3 || ColorBytes==4 )
	{
		// 24-bit or 32-bit color.
		BitmapHeader.biCompression  = BI_RGB;
		*(DWORD *)&BitmapColors[0]=0;
	}
	else appErrorf( "Invalid color depth %i", ColorBytes );

	// Get TextureData.
	if( BlitType == BLIT_DIBSECTION )
	{
		// Create DIB section.
		if( SizeX && SizeY )
		{
			HDC TempDC = GetDC(0);
			check(TempDC);
			hBitmap = CreateDIBSection( TempDC, (BITMAPINFO*)&BitmapHeader, DIB_RGB_COLORS, (void**)&ScreenPointer, NULL, 0 );
			ReleaseDC( 0, TempDC );
			if( !hBitmap )
				appErrorf( LocalizeError("OutOfMemory","Core") );
		}
	}
	else if( BlitType == BLIT_DIRECTDRAW )
	{
		ScreenPointer = NULL;
	}

	// Update the window.
	UpdateWindow();

	unguard;
}

IMPLEMENT_CLASS(UWindowsViewport);

/*--------------------------------------------------------------------------------
	UWindowsClient general utility functions.
--------------------------------------------------------------------------------*/

//
// Create a new viewport.
//
UViewport* UWindowsClient::NewViewport( class ULevel* InLevel, const FName Name )
{
	guard(UWindowsClient::NewViewport);
	return new( GObj.GetTransientPackage(), Name )UWindowsViewport( InLevel, this );
	unguard;
}


//
// Return the current viewport.  Returns NULL if no viewport has focus.
//
UViewport* UWindowsClient::CurrentViewport()
{
	guard(UWindowsClient::CurrentViewport);
	UWindowsViewport* TestViewport = NULL;
	for( int i=0; i<Viewports.Num(); i++ )
	{
		TestViewport = (UWindowsViewport*)Viewports(i);
     	if( TestViewport->Current )
			break;
	}
	if( i >= Viewports.Num() )
		return NULL;
	HWND hWndActive = GetActiveWindow();
	if
	(	hWndActive == FullscreenhWndDD
	||	hWndActive == TestViewport->hWnd )
	{
		return TestViewport;
	}
	else 
	{
		return NULL;
	}
	unguard;
}

//
// Toggle a menu item and return 0 if it's now off, 1 if it's now on.
//
int UWindowsClient::Toggle( HMENU hMenu, int Item )
{
	guard(UWindowsClient::Toggle);

	if( GetMenuState(hMenu,Item,MF_BYCOMMAND)&MF_CHECKED )
	{
		// Now unchecked.
		CheckMenuItem(hMenu,Item,MF_UNCHECKED);
		return 0;
	}
	else
	{
		// Now checked.
		CheckMenuItem(hMenu,Item,MF_CHECKED);
		return 1;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Command lines.
-----------------------------------------------------------------------------*/

UBOOL UWindowsClient::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(UWindowsClient::Exec);
	if( ParseCommand(&Cmd,"EndFullscreen") )
	{
		EndFullscreen();
		return 1;
	}
	else if( ParseCommand(&Cmd,"KillDirectDraw") )
	{
		ddExit();
		return 1;
	}
	else if( ParseCommand(&Cmd,"ReviveDirectDraw") )
	{
		ddInit();
		return 1;
	}
	else if( ParseCommand(&Cmd,"GetRes") )
	{
		if( FullscreenhWndDD )
		{
			// DirectDraw modes.
			FString Result;
			for( INT i=0; i<ddNumModes; i++ )
				Result.Appendf( "%ix%i ", ddModeWidth[i], ddModeHeight[i] );
			Out->Log( *Result );
		}
		else
		{
			// Fullscreen mode.
			Out->Log( "320x200 400x300 512x384 640x480 800x600" );
		}
		return 1;
	}
	else if( UClient::Exec( Cmd, Out ) )
	{
		return 1;
	}
	return 0;
	unguard;
}

UBOOL UWindowsViewport::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(UWindowsViewport::Exec);
	if( UViewport::Exec( Cmd, Out ) )
	{
		return 1;
	}
	else if( ParseCommand(&Cmd,"ToggleFullscreen") )
	{
		// Toggle fullscreen.
		if( Client->FullscreenViewport )
			Client->EndFullscreen();
		else if( !(Actor->ShowFlags & SHOW_ChildWindow) )
			TryRenderDevice( this, "ini:Engine.Engine.GameRenderDevice", 1 );
		return 1;
	}
	else if( ParseCommand(&Cmd,"GetCurrentRes") )
	{
		Out->Logf( "%ix%i", SizeX, SizeY );
		return 1;
	}
	else if( ParseCommand(&Cmd,"SetRes") )
	{
		INT X=appAtoi(Cmd), Y=appAtoi(appStrchr(Cmd,'x') ? appStrchr(Cmd,'x')+1 : appStrchr(Cmd,'X') ? appStrchr(Cmd,'X')+1 : "");
		if( X && Y )
		{
			if( Client->FullscreenhWndDD )
				MakeFullscreen( X, Y, 1 );
			else
				SetClientSize( X, Y, 1 );
		}
		return 1;
	}
	else if( ParseCommand(&Cmd,"Preferences") )
	{
		if( Client->FullscreenViewport )
			Client->EndFullscreen();
		if( !Client->ConfigProperties )
		{
			Client->ConfigProperties = new WConfigProperties( "Preferences", "Advanced Options" );
			Client->ConfigProperties->SetNotifyHook( Client );
			Client->ConfigProperties->OpenWindow( hWnd );
			Client->ConfigProperties->ForceRefresh();
		}
		ShowWindow( *Client->ConfigProperties, SW_SHOWNORMAL );
		SetFocus( *Client->ConfigProperties );
		return 1;
	}
	else return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	DirectDraw support.
-----------------------------------------------------------------------------*/

//
// Return a DirectDraw error message.
// Error messages commented out are DirectDraw II error messages.
//
char* UWindowsClient::ddError( HRESULT Result )
{
	guard(UWindowsClient::ddError);
	switch( Result )
	{
		case DD_OK:									return "DD_OK";
		case DDERR_ALREADYINITIALIZED:				return "DDERR_ALREADYINITIALIZED";
		case DDERR_BLTFASTCANTCLIP:					return "DDERR_BLTFASTCANTCLIP";
		case DDERR_CANNOTATTACHSURFACE:				return "DDERR_CANNOTATTACHSURFACE";
		case DDERR_CANNOTDETACHSURFACE:				return "DDERR_CANNOTDETACHSURFACE";
		case DDERR_CANTCREATEDC:					return "DDERR_CANTCREATEDC";
		case DDERR_CANTDUPLICATE:					return "DDERR_CANTDUPLICATE";
		case DDERR_CLIPPERISUSINGHWND:				return "DDERR_CLIPPERISUSINGHWND";
		case DDERR_COLORKEYNOTSET:					return "DDERR_COLORKEYNOTSET";
		case DDERR_CURRENTLYNOTAVAIL:				return "DDERR_CURRENTLYNOTAVAIL";
		case DDERR_DIRECTDRAWALREADYCREATED:		return "DDERR_DIRECTDRAWALREADYCREATED";
		case DDERR_EXCEPTION:						return "DDERR_EXCEPTION";
		case DDERR_EXCLUSIVEMODEALREADYSET:			return "DDERR_EXCLUSIVEMODEALREADYSET";
		case DDERR_GENERIC:							return "DDERR_GENERIC";
		case DDERR_HEIGHTALIGN:						return "DDERR_HEIGHTALIGN";
		case DDERR_HWNDALREADYSET:					return "DDERR_HWNDALREADYSET";
		case DDERR_HWNDSUBCLASSED:					return "DDERR_HWNDSUBCLASSED";
		case DDERR_IMPLICITLYCREATED:				return "DDERR_IMPLICITLYCREATED";
		case DDERR_INCOMPATIBLEPRIMARY:				return "DDERR_INCOMPATIBLEPRIMARY";
		case DDERR_INVALIDCAPS:						return "DDERR_INVALIDCAPS";
		case DDERR_INVALIDCLIPLIST:					return "DDERR_INVALIDCLIPLIST";
		case DDERR_INVALIDDIRECTDRAWGUID:			return "DDERR_INVALIDDIRECTDRAWGUID";
		case DDERR_INVALIDMODE:						return "DDERR_INVALIDMODE";
		case DDERR_INVALIDOBJECT:					return "DDERR_INVALIDOBJECT";
		case DDERR_INVALIDPARAMS:					return "DDERR_INVALIDPARAMS";
		case DDERR_INVALIDPIXELFORMAT:				return "DDERR_INVALIDPIXELFORMAT";
		case DDERR_INVALIDPOSITION:					return "DDERR_INVALIDPOSITION";
		case DDERR_INVALIDRECT:						return "DDERR_INVALIDRECT";
		case DDERR_LOCKEDSURFACES:					return "DDERR_LOCKEDSURFACES";
		case DDERR_NO3D:							return "DDERR_NO3D";
		case DDERR_NOALPHAHW:						return "DDERR_NOALPHAHW";
		case DDERR_NOBLTHW:							return "DDERR_NOBLTHW";
		case DDERR_NOCLIPLIST:						return "DDERR_NOCLIPLIST";
		case DDERR_NOCLIPPERATTACHED:				return "DDERR_NOCLIPPERATTACHED";
		case DDERR_NOCOLORCONVHW:					return "DDERR_NOCOLORCONVHW";
		case DDERR_NOCOLORKEY:						return "DDERR_NOCOLORKEY";
		case DDERR_NOCOLORKEYHW:					return "DDERR_NOCOLORKEYHW";
		case DDERR_NOCOOPERATIVELEVELSET:			return "DDERR_NOCOOPERATIVELEVELSET";
		case DDERR_NODC:							return "DDERR_NODC";
		case DDERR_NODDROPSHW:						return "DDERR_NODDROPSHW";
		case DDERR_NODIRECTDRAWHW:					return "DDERR_NODIRECTDRAWHW";
		case DDERR_NOEMULATION:						return "DDERR_NOEMULATION";
		case DDERR_NOEXCLUSIVEMODE:					return "DDERR_NOEXCLUSIVEMODE";
		case DDERR_NOFLIPHW:						return "DDERR_NOFLIPHW";
		case DDERR_NOGDI:							return "DDERR_NOGDI";
		case DDERR_NOHWND:							return "DDERR_NOHWND";
		case DDERR_NOMIRRORHW:						return "DDERR_NOMIRRORHW";
		case DDERR_NOOVERLAYDEST:					return "DDERR_NOOVERLAYDEST";
		case DDERR_NOOVERLAYHW:						return "DDERR_NOOVERLAYHW";
		case DDERR_NOPALETTEATTACHED:				return "DDERR_NOPALETTEATTACHED";
		case DDERR_NOPALETTEHW:						return "DDERR_NOPALETTEHW";
		case DDERR_NORASTEROPHW:					return "DDERR_NORASTEROPHW";
		case DDERR_NOROTATIONHW:					return "DDERR_NOROTATIONHW";
		case DDERR_NOSTRETCHHW:						return "DDERR_NOSTRETCHHW";
		case DDERR_NOT4BITCOLOR:					return "DDERR_NOT4BITCOLOR";
		case DDERR_NOT4BITCOLORINDEX:				return "DDERR_NOT4BITCOLORINDEX";
		case DDERR_NOT8BITCOLOR:					return "DDERR_NOT8BITCOLOR";
		case DDERR_NOTAOVERLAYSURFACE:				return "DDERR_NOTAOVERLAYSURFACE";
		case DDERR_NOTEXTUREHW:						return "DDERR_NOTEXTUREHW";
		case DDERR_NOTFLIPPABLE:					return "DDERR_NOTFLIPPABLE";
		case DDERR_NOTFOUND:						return "DDERR_NOTFOUND";
		case DDERR_NOTLOCKED:						return "DDERR_NOTLOCKED";
		case DDERR_NOTPALETTIZED:					return "DDERR_NOTPALETTIZED";
		case DDERR_NOVSYNCHW:						return "DDERR_NOVSYNCHW";
		case DDERR_NOZBUFFERHW:						return "DDERR_NOZBUFFERHW";
		case DDERR_NOZOVERLAYHW:					return "DDERR_NOZOVERLAYHW";
		case DDERR_OUTOFCAPS:						return "DDERR_OUTOFCAPS";
		case DDERR_OUTOFMEMORY:						return "DDERR_OUTOFMEMORY";
		case DDERR_OUTOFVIDEOMEMORY:				return "DDERR_OUTOFVIDEOMEMORY";
		case DDERR_OVERLAYCANTCLIP:					return "DDERR_OVERLAYCANTCLIP";
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:	return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
		case DDERR_OVERLAYNOTVISIBLE:				return "DDERR_OVERLAYNOTVISIBLE";
		case DDERR_PALETTEBUSY:						return "DDERR_PALETTEBUSY";
		case DDERR_PRIMARYSURFACEALREADYEXISTS:		return "DDERR_PRIMARYSURFACEALREADYEXISTS";
		case DDERR_REGIONTOOSMALL:					return "DDERR_REGIONTOOSMALL";
		case DDERR_SURFACEALREADYATTACHED:			return "DDERR_SURFACEALREADYATTACHED";
		case DDERR_SURFACEALREADYDEPENDENT:			return "DDERR_SURFACEALREADYDEPENDENT";
		case DDERR_SURFACEBUSY:						return "DDERR_SURFACEBUSY";
		case DDERR_SURFACEISOBSCURED:				return "DDERR_SURFACEISOBSCURED";
		case DDERR_SURFACELOST:						return "DDERR_SURFACELOST";
		case DDERR_SURFACENOTATTACHED:				return "DDERR_SURFACENOTATTACHED";
		case DDERR_TOOBIGHEIGHT:					return "DDERR_TOOBIGHEIGHT";
		case DDERR_TOOBIGSIZE:						return "DDERR_TOOBIGSIZE";
		case DDERR_TOOBIGWIDTH:						return "DDERR_TOOBIGWIDTH";
		case DDERR_UNSUPPORTED:						return "DDERR_UNSUPPORTED";
		case DDERR_UNSUPPORTEDFORMAT:				return "DDERR_UNSUPPORTEDFORMAT";
		case DDERR_UNSUPPORTEDMASK:					return "DDERR_UNSUPPORTEDMASK";
		case DDERR_UNSUPPORTEDMODE:					return "DDERR_UNSUPPORTEDMODE";
		case DDERR_VERTICALBLANKINPROGRESS:			return "DDERR_VERTICALBLANKINPROGRESS";
		case DDERR_WASSTILLDRAWING:					return "DDERR_WASSTILLDRAWING";
		case DDERR_WRONGMODE:						return "DDERR_WRONGMODE";
		case DDERR_XALIGN:							return "DDERR_XALIGN";
		case DDERR_CANTPAGELOCK:					return "DDERR_CANTPAGELOCK";
		case DDERR_CANTPAGEUNLOCK:					return "DDERR_CANTPAGEUNLOCK";
		case DDERR_DCALREADYCREATED:				return "DDERR_DCALREADYCREATED";
		case DDERR_INVALIDSURFACETYPE:				return "DDERR_INVALIDSURFACETYPE";
		case DDERR_NOMIPMAPHW:						return "DDERR_NOMIPMAPHW";
		case DDERR_NOTPAGELOCKED:					return "DDERR_NOTPAGELOCKED";
		case DDERR_CANTLOCKSURFACE:					return "DDERR_CANTLOCKSURFACE";
/*
		case D3DERR_BADMAJORVERSION:				return "D3DERR_BADMAJORVERSION";
		case D3DERR_BADMINORVERSION:				return "D3DERR_BADMINORVERSION";
//		case D3DERR_INVALID_DEVICE:					return "D3DERR_INVALID_DEVICE";
		case D3DERR_EXECUTE_CREATE_FAILED:			return "D3DERR_EXECUTE_CREATE_FAILED";
		case D3DERR_EXECUTE_DESTROY_FAILED:			return "D3DERR_EXECUTE_DESTROY_FAILED";
		case D3DERR_EXECUTE_LOCK_FAILED:			return "D3DERR_EXECUTE_LOCK_FAILED";
		case D3DERR_EXECUTE_UNLOCK_FAILED:			return "D3DERR_EXECUTE_UNLOCK_FAILED";
		case D3DERR_EXECUTE_LOCKED:					return "D3DERR_EXECUTE_LOCKED";
		case D3DERR_EXECUTE_NOT_LOCKED:				return "D3DERR_EXECUTE_NOT_LOCKED";
		case D3DERR_EXECUTE_FAILED:					return "D3DERR_EXECUTE_FAILED";
		case D3DERR_EXECUTE_CLIPPED_FAILED:			return "D3DERR_EXECUTE_CLIPPED_FAILED";
		case D3DERR_TEXTURE_NO_SUPPORT:				return "D3DERR_TEXTURE_NO_SUPPORT";
		case D3DERR_TEXTURE_CREATE_FAILED:			return "D3DERR_TEXTURE_CREATE_FAILED";
		case D3DERR_TEXTURE_DESTROY_FAILED:			return "D3DERR_TEXTURE_DESTROY_FAILED";
		case D3DERR_TEXTURE_LOCK_FAILED:			return "D3DERR_TEXTURE_LOCK_FAILED";
		case D3DERR_TEXTURE_UNLOCK_FAILED:			return "D3DERR_TEXTURE_UNLOCK_FAILED";
		case D3DERR_TEXTURE_LOAD_FAILED:			return "D3DERR_TEXTURE_LOAD_FAILED";
		case D3DERR_TEXTURE_SWAP_FAILED:			return "D3DERR_TEXTURE_SWAP_FAILED";
		case D3DERR_TEXTURE_LOCKED:					return "D3DERR_TEXTURE_LOCKED";
		case D3DERR_TEXTURE_NOT_LOCKED:				return "D3DERR_TEXTURE_NOT_LOCKED";
		case D3DERR_TEXTURE_GETSURF_FAILED:			return "D3DERR_TEXTURE_GETSURF_FAILED";
		case D3DERR_MATRIX_CREATE_FAILED:			return "D3DERR_MATRIX_CREATE_FAILED";
		case D3DERR_MATRIX_DESTROY_FAILED:			return "D3DERR_MATRIX_DESTROY_FAILED";
		case D3DERR_MATRIX_SETDATA_FAILED:			return "D3DERR_MATRIX_SETDATA_FAILED";
		case D3DERR_MATRIX_GETDATA_FAILED:			return "D3DERR_MATRIX_GETDATA_FAILED";
		case D3DERR_SETVIEWPORTDATA_FAILED:			return "D3DERR_SETVIEWPORTDATA_FAILED";
		case D3DERR_MATERIAL_CREATE_FAILED:			return "D3DERR_MATERIAL_CREATE_FAILED";
		case D3DERR_MATERIAL_DESTROY_FAILED:		return "D3DERR_MATERIAL_DESTROY_FAILED";
		case D3DERR_MATERIAL_SETDATA_FAILED:		return "D3DERR_MATERIAL_SETDATA_FAILED";
		case D3DERR_MATERIAL_GETDATA_FAILED:		return "D3DERR_MATERIAL_GETDATA_FAILED";
		case D3DERR_LIGHT_SET_FAILED:				return "D3DERR_LIGHT_SET_FAILED";
		case D3DERR_SCENE_IN_SCENE:					return "D3DERR_SCENE_IN_SCENE";
		case D3DERR_SCENE_NOT_IN_SCENE:				return "D3DERR_SCENE_NOT_IN_SCENE";
		case D3DERR_SCENE_BEGIN_FAILED:				return "D3DERR_SCENE_BEGIN_FAILED";
		case D3DERR_SCENE_END_FAILED:				return "D3DERR_SCENE_END_FAILED";
//		case D3DERR_INBEGIN:						return "D3DERR_INBEGIN";
//		case D3DERR_NOTINBEGIN:						return "D3DERR_NOTINBEGIN";
*/
		default:									return "Unknown error";
	}
	unguard;
}

//
// DirectDraw driver enumeration callback.
//
static BOOL WINAPI ddEnumDriversCallback( GUID* GUID, char* DriverDescription, char* DriverName, void* Context )
{
	guard(ddEnumDriversCallback);
	UWindowsClient *Client = (UWindowsClient *)Context;

	debugf(NAME_Log,"   %s (%s)",DriverName,DriverDescription);
	if( Client->NumDD < UWindowsClient::MAX_DD )
	{
		if( GUID )
			Client->ddGUIDs[Client->NumDD] = *GUID;
		Client->NumDD++;
	}

	return DDENUMRET_OK;
	unguard;
}

//
// Init all DirectDraw stuff.
//
UBOOL UWindowsClient::ddInit()
{
	guard(UWindowsClient::ddInit);

	// Skip out.
	char Temp[256]="";
	GetConfigString( "Engine.Engine", "GameRenderDevice", Temp, ARRAY_COUNT(Temp) );
	if
	(	!UseDirectDraw
	||	ParseParam(appCmdLine(),"noddraw")
	||	appStricmp(Temp,"GlideDrv.GlideRenderDevice")==0 )
		return 0;
	debugf( NAME_Init, "Initializing DirectDraw" );

	// Load DirectDraw DLL.
	HINSTANCE Instance = LoadLibrary("ddraw.dll");
	if( Instance == NULL )
	{
		debugf( NAME_Init, "DirectDraw not installed" );
		return 0;
	}
	ddCreateFunc = (DD_CREATE_FUNC)GetProcAddress( Instance, "DirectDrawCreate"     );
	ddEnumFunc   = (DD_ENUM_FUNC  )GetProcAddress( Instance, "DirectDrawEnumerateA" );
	if( !ddCreateFunc || !ddEnumFunc )
	{
		debugf( NAME_Init, "DirectDraw GetProcAddress failed" );
		return 0;
	}

	// Show available DirectDraw drivers.
	NumDD = 0;
	debugf( NAME_Log, "DirectDraw drivers:" );
	ddEnumFunc( ddEnumDriversCallback, this );
	if( NumDD == 0 )
	{
		debugf( NAME_Init, "No DirectDraw drivers found" );
		return 0;
	}

	// Init direct draw and see if it's available.
	IDirectDraw* dd1;
	HRESULT Result = (*ddCreateFunc)( NULL, &dd1, NULL );
	if( Result != DD_OK )
	{
		debugf( NAME_Init, "DirectDraw created failed: %s", ddError(Result) );
   		return 0;
	}
	Result = dd1->QueryInterface( IID_IDirectDraw2, (void**)&dd );
	dd1->Release();
	if( Result != DD_OK )
	{
		debugf( NAME_Init, "DirectDraw2 interface not available" );
   		return 0;
	}
	debugf( NAME_Init, "DirectDraw initialized successfully" );

	return 1;
	unguard;
}

//
// Set DirectDraw to a particular mode, with full error checking
// Returns 1 if success, 0 if failure.
//
UBOOL UWindowsClient::ddSetMode( HWND hWndOwner, int Width, int Height, int ColorBytes, int &Caps )
{
	guard(UWindowsClient::ddSetMode);
	check(dd);
	HRESULT	Result;

	// Grab exclusive access to DirectDraw.
	Result = dd->SetCooperativeLevel( hWndOwner, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT );
	if( Result != DD_OK )
	{
		debugf( NAME_Log, "DirectDraw SetCooperativeLevel: %s", ddError(Result) );
   		return 0;
	}

	// Set the display mode.
	debugf( NAME_Log, "Setting %ix%ix%i", Width, Height, ColorBytes*8 );
	Result = dd->SetDisplayMode( Width, Height, ColorBytes*8, 0, 0 );
	if( Result != DD_OK )
	{
		debugf( NAME_Log, "DirectDraw Failed %ix%ix%i: %s", Width, Height, ColorBytes*8, ddError(Result) );
		ddEndMode();
   		return 0;
	}

	// Create surfaces.
	DDSURFACEDESC SurfaceDesc;
	memset( &SurfaceDesc, 0, sizeof(DDSURFACEDESC) );
	SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	SurfaceDesc.ddsCaps.dwCaps
	=	DDSCAPS_PRIMARYSURFACE
	|	DDSCAPS_FLIP
	|	DDSCAPS_COMPLEX
	|	(SlowVideoBuffering ? DDSCAPS_SYSTEMMEMORY : DDSCAPS_VIDEOMEMORY);

	// Update caps if low-res.
	if( Width==320 )
		SurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_MODEX;

	// Create the best possible surface for rendering.
	char* Descr=NULL;
	if( 1 )
	{
		// Try triple-buffered video memory surface.
		SurfaceDesc.dwBackBufferCount = 2;
		Result = dd->CreateSurface(&SurfaceDesc, &ddFrontBuffer, NULL);
		Descr  = "Triple buffer";
	}
	if( Result != DD_OK )
   	{
		// Try to get a double buffered video memory surface.
		SurfaceDesc.dwBackBufferCount = 1; 
		Result = dd->CreateSurface(&SurfaceDesc, &ddFrontBuffer, NULL);
		Descr  = "Double buffer";
    }
	if( Result != DD_OK )
	{
		// Settle for a main memory surface.
		SurfaceDesc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
		Result = dd->CreateSurface(&SurfaceDesc, &ddFrontBuffer, NULL);
		Descr  = "System memory";
    }
	if( Result != DD_OK )
	{
		debugf( NAME_Log, "DirectDraw, no available modes %s", ddError(Result) );
		ddEndMode();
	   	return 0;
	}
	debugf( NAME_Log, "DirectDraw: %s, %ix%i, Stride=%i", Descr, Width, Height, SurfaceDesc.lPitch );
	debugf( NAME_Log, "DirectDraw: Rate=%i", SurfaceDesc.dwRefreshRate );

	// Get a pointer to the back buffer.
	DDSCAPS caps;
	caps.dwCaps = DDSCAPS_BACKBUFFER;
	if( ddFrontBuffer->GetAttachedSurface( &caps, &ddBackBuffer ) != DD_OK )
	{
		debugf(NAME_Log,"DirectDraw GetAttachedSurface failed %s",ddError(Result));
		ddEndMode();
	}

	// Get pixel format.
	DDPIXELFORMAT PixelFormat;
	PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	Result = ddFrontBuffer->GetPixelFormat( &PixelFormat );
	if( Result != DD_OK )
	{
		ddEndMode();
		appErrorf( "DirectDraw GetPixelFormat failed: %s", ddError(Result) );
	}

	// See if we're in a 16-bit color mode.
	if( ColorBytes==2 && PixelFormat.dwRBitMask==0xf800 ) 
		Caps |= CC_RGB565;
	else 
		Caps &= ~CC_RGB565;

	// Flush the cache.
	GCache.Flush();

	// Success.
	return 1;
	unguard;
}

//
// End the current DirectDraw mode.
//
void UWindowsClient::ddEndMode()
{
	guard(UWindowsClient::ddEndMode);

	HRESULT Result;
	debugf( NAME_Log, "DirectDraw End Mode" );
	if( dd )
	{
		// Release all buffers.
		if( ddBackBuffer )
		{
			ddBackBuffer->Release();
			ddBackBuffer = NULL;
		}
		if( ddFrontBuffer )
		{
			ddFrontBuffer->Release();
			ddFrontBuffer = NULL;
		}

		// Return to normal cooperative level.
		Result = dd->SetCooperativeLevel( NULL, DDSCL_NORMAL );
		if( Result!=DD_OK )
			debugf( NAME_Log, "DirectDraw SetCooperativeLevel: %s", ddError(Result) );

		// Restore the display mode.
		Result = dd->RestoreDisplayMode();
		if( Result!=DD_OK )
			debugf( NAME_Log, "DirectDraw RestoreDisplayMode: %s", ddError(Result) );

		// Flip to GDI surface (may return error code; this is ok).
		dd->FlipToGDISurface();
	}
	if( !GIsCriticalError )
	{
		// Flush the cache unless we're ending DirectDraw due to a crash.
		debugf( NAME_Log, "Flushing cache" );
		GCache.Flush();
	}
	unguard;
}

//
// Shut DirectDraw down.
//
void UWindowsClient::ddExit()
{
	guard(UWindowsClient::ddExit);
	if( dd )
	{
		ddEndMode();
		HRESULT Result = dd->Release();
		if( Result != DD_OK )
			debugf( NAME_Exit, "DirectDraw Release failed: %s", ddError(Result) );
		else
			debugf( NAME_Exit, "DirectDraw released" );
		dd = NULL;
	}
	unguard;
}

//
// Try to set DirectDraw mode with a particular viewport.
// Returns 1 if success, 0 if failure.
//
UBOOL UWindowsClient::ddSetViewport
(
	UViewport*	InViewport,
	INT			Width,
	INT			Height,
	INT			ColorBytes,
	INT			RequestedCaps
)
{
	guard(UWindowsClient::ddSetViewport);
	check(dd);
	UWindowsViewport* Viewport = CastChecked<UWindowsViewport>(InViewport);

	// End any previous fullscreen mode.
	if( FullscreenViewport )
		EndFullscreen();

	// Try to go into DirectDraw.
	Viewport->Hold();
	if( Engine->Audio && !GIsEditor )
		Engine->Audio->SetViewport( NULL );
	UBOOL Result = ddSetMode( Viewport->hWnd, Width, Height, ColorBytes, RequestedCaps );
	Viewport->Unhold();
	if( Engine->Audio && !GIsEditor )
		Engine->Audio->SetViewport( Viewport );
	if( !Result )
	{
		RECT* Rect = &Viewport->SavedWindowRect;
		MoveWindow( Viewport->hWnd, Rect->left, Rect->top, Rect->right-Rect->left, Rect->bottom-Rect->top, 1 );
		Viewport->SetTopness();
		GSystem->Warnf( LocalizeError("DDrawMode") );
		return 0;
	}

	// Set the fullscreen window.
	FullscreenViewport = Viewport;
	FullscreenhWndDD = Viewport->hWnd;
	SetFocus(FullscreenhWndDD);

	// Resize frame buffer without redrawing.
	Viewport->SetFrameBufferSize( Width, Height, ColorBytes, BLIT_DIRECTDRAW );

	// Capture the mouse.
	Viewport->SetDrag( 1 );
	Viewport->SetMouseCapture( 1, 1, 0 );

	// Remember the caps.
	Viewport->Caps = RequestedCaps;
	return 1;
	unguard;
}

//
// Set window position according to menu's on-top setting:
//
void UWindowsViewport::SetTopness()
{
	guard(UWindowsViewport::SetTopness);
	if( GetMenuState(GetMenu(hWnd),ID_WIN_TOP,MF_BYCOMMAND)&MF_CHECKED )
	{
		SetWindowPos(hWnd,(HWND)-1,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(hWnd,(HWND)1,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
		SetWindowPos(hWnd,(HWND)0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Viewport Window WndProc.
-----------------------------------------------------------------------------*/

//
// Main viewport window function.
//
LONG UWindowsViewport::WndProc( UINT iMessage, WPARAM wParam, LPARAM lParam )
{
	guard(UWindowsViewport::WndProc);
	INT Temp;
	if( OnHold || !Client->Viewports.FindItem(this,Temp) || !Actor )
		return DefWindowProc( hWnd, iMessage, wParam, lParam );

	// Statics.
	static UBOOL MovedSinceLeftClick=0;
	static UBOOL MovedSinceRightClick=0;
	static UBOOL MovedSinceMiddleClick=0;
	static DWORD StartTimeLeftClick=0;
	static DWORD StartTimeRightClick=0;
	static DWORD StartTimeMiddleClick=0;

	// Message handler.
	switch( iMessage )
	{
		case WM_CREATE:
		{
			guard(WM_CREATE);

         	// Set status.
			Status = WIN_ViewportNormal; 

			// Make this viewport current and update its title bar.
			MakeCurrent();

			return 0;
			unguard;
		}
		case WM_DESTROY:
		{
			guard(WM_DESTROY);

			// If there's an existing Viewport structure corresponding to
			// this window, deactivate it.
			if( Client->FullscreenViewport )
				Client->EndFullscreen();

			// Restore focus to caller if desired.
			DWORD ParentWindow=0;
			Parse( appCmdLine(), "HWND=", ParentWindow );
			if( ParentWindow )
			{
				::SetParent( hWnd, NULL );
				SetFocus( (HWND)ParentWindow );
			}

			// Free DIB section stuff (if any).
			if( hBitmap )
				DeleteObject( hBitmap );

			// Stop clipping.
			SetDrag( 0 );
			if( Status==WIN_ViewportNormal )
			{
				// Closed by user clicking on window's close button, so delete the viewport.
				Status = WIN_ViewportClosing; // Prevent recursion.
				delete this;
			}
			debugf( NAME_Log, "Closed viewport" );
			return 0;
			unguard;
		}
		case WM_PAINT:
		{
			guard(WM_PAINT);
			if( Client->FullscreenViewport )
			{
				GTickDue = 1;
				return 0;
			}
			else if( IsWindowVisible(hWnd) && SizeX && SizeY && !OnHold && hBitmap )
			{
				PAINTSTRUCT ps;
				BeginPaint( hWnd, &ps );

				HDC hDC = GetDC( hWnd );
				if( hDC == NULL )
					appErrorf( "GetDC failed: %s", winError() );
				if( SelectObject( Client->hMemScreenDC, hBitmap ) == NULL )
					appErrorf( "SelectObject failed: %s", winError() );
				if( BitBlt( hDC, 0, 0, SizeX, SizeY, Client->hMemScreenDC, 0, 0, SRCCOPY ) == NULL )
					appErrorf( "BitBlt failed: %s", winError() );
				if( ReleaseDC( hWnd, hDC ) == NULL )
					appErrorf( "ReleaseDC failed: %s", winError() );

				EndPaint( hWnd, &ps );
				return 0;
			}
			else return -1;
			unguard;
		}
		case WM_COMMAND:
		{
			guard(WM_COMMAND);
      		switch( wParam )
			{
				case ID_MAP_DYNLIGHT:
				{
					Actor->RendMap=REN_DynLight;
					break;
				}
				case ID_MAP_PLAINTEX:
				{
					Actor->RendMap=REN_PlainTex;
					break;
				}
				case ID_MAP_WIRE:
				{
					Actor->RendMap=REN_Wire;
					break;
				}
				case ID_MAP_OVERHEAD:
				{
					Actor->RendMap=REN_OrthXY;
					break;
				}
				case ID_MAP_XZ:
				{
					Actor->RendMap=REN_OrthXZ;
					break;
				}
				case ID_MAP_YZ:
				{
					Actor->RendMap=REN_OrthYZ;
					break;
				}
				case ID_MAP_POLYS:
				{
					Actor->RendMap=REN_Polys;
					break;
				}
				case ID_MAP_POLYCUTS:
				{
					Actor->RendMap=REN_PolyCuts;
					break;
				}
				case ID_MAP_ZONES:
				{
					Actor->RendMap=REN_Zones;
					break;
				}
				case ID_WIN_320:
				{
					SetClientSize(320,200,1);
					break;
				}
				case ID_WIN_400:
				{
					SetClientSize(400,300,1);
					break;
				}
				case ID_WIN_512:
				{
					SetClientSize(512,384,1);
					break;
				}
				case ID_WIN_640:
				{
					SetClientSize(640,400,1);
					break;
				}
				case ID_WIN_640_480:
				{
					SetClientSize(640,480,1);
					break;
				}
				case ID_WIN_800:
				{
					SetClientSize(800,600,1);
					break;
				}
				case ID_COLOR_16BIT:
				{
					SetFrameBufferSize( SizeX, SizeY, 2, BLIT_DEFAULT );
					Repaint();
					FindAvailableModes();
					break;
				}
				case ID_COLOR_32BIT:
				{
					SetFrameBufferSize( SizeX, SizeY, 4, BLIT_DEFAULT );
					Repaint();
					FindAvailableModes();
					break;
				}
				case ID_SHOW_BACKDROP:
				{
					Actor->ShowFlags ^= SHOW_Backdrop;
					break;
				}
				case ID_ACTORS_SHOW:
				{
					Actor->ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii);
					Actor->ShowFlags |= SHOW_Actors; 
					break;
				}
				case ID_ACTORS_ICONS:
				{
					Actor->ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii); 
					Actor->ShowFlags |= SHOW_Actors | SHOW_ActorIcons;
					break;
				}
				case ID_ACTORS_RADII:
				{
					Actor->ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii); 
					Actor->ShowFlags |= SHOW_Actors | SHOW_ActorRadii;
					break;
				}
				case ID_ACTORS_HIDE:
				{
					Actor->ShowFlags &= ~(SHOW_Actors | SHOW_ActorIcons | SHOW_ActorRadii); 
					break;
				}
				case ID_SHOW_PATHS:
				{
					Actor->ShowFlags ^= SHOW_Paths;
					break;
				}
				case ID_SHOW_COORDS:
				{
					Actor->ShowFlags ^= SHOW_Coords;
					break;
				}
				case ID_SHOW_BRUSH:
				{
					Actor->ShowFlags ^= SHOW_Brush;
					break;
				}
				case ID_SHOW_MOVINGBRUSHES:
				{
					Actor->ShowFlags ^= SHOW_MovingBrushes;
					break;
				}
				case ID_SHOWLOG:
				{
					Exec( "SHOWLOG", this );
					break;
				}
				case ID_FILE_EXIT:
				{
					DestroyWindow(hWnd);
					return 0;
				}
				case ID_WIN_TOP:
				{
					Client->Toggle(GetMenu(hWnd),ID_WIN_TOP);
					SetTopness();
					break;
				}
				case ID_PROPERTIES_PROPERTIES2:
				{
					Exec( "Preferences", GSystem );
					break;
				}
				default:
				{
					if( wParam>=ID_DDMODE0 && wParam<=ID_DDMODE9 )
						MakeFullscreen( Client->ddModeWidth[wParam-ID_DDMODE0], Client->ddModeHeight[wParam-ID_DDMODE0], 1 );
					break;
				}
			}
			Repaint();
			UpdateWindow();
			return 0;
			unguard;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			guard(WM_KEYDOWN);

			// Get key code.
			EInputKey Key = (EInputKey)wParam;

			// Send key to input system.
			if( Key==IK_Enter && Input->KeyDown(IK_Alt) )
			{
				Exec( "ToggleFullscreen", this );
			}
			else if( CauseInputEvent( Key, IST_Press ) )
			{	
				// Redraw if the viewport won't be redrawn on timer.
				if( !IsRealtime() )
					Repaint();
			}

			// Send to UnrealEd.
			if( ParentWindow && GIsEditor )
			{
				if( Key==IK_F1 )
					PostMessage( ParentWindow, iMessage, IK_F2, lParam );
				else if( Key!=IK_Tab && Key!=IK_Enter && Key!=IK_Alt )
					PostMessage( ParentWindow, iMessage, wParam, lParam );
			}

			// Set the cursor.
			if( GIsEditor )
				SetModeCursor();

			return 0;
			unguard;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			guard(WM_KEYUP);

			// Send to the input system.
			EInputKey Key = (EInputKey)wParam;
			if( CauseInputEvent( Key, IST_Release ) )
			{	
				// Redraw if the viewport won't be redrawn on timer.
				if( !IsRealtime() )
					Repaint();
			}

			// Pass keystroke on to UnrealEd.
			if( ParentWindow && GIsEditor )
			{				
				if( Key == IK_F1 )
					PostMessage( ParentWindow, iMessage, IK_F2, lParam );
				else if( Key!=IK_Tab && Key!=IK_Enter && Key!=IK_Alt )
					PostMessage( ParentWindow, iMessage, wParam, lParam );
			}
			if( GIsEditor )
				SetModeCursor();
			return 0;
			unguard;
		}
		case WM_SYSCHAR:
		case WM_CHAR:
		{
			guard(WM_CHAR);

			EInputKey Key = (EInputKey)wParam;
			if( Key!=IK_Enter && Client->Engine->Key( this, Key ) )
			{
				// Redraw if needed.
				if( !IsRealtime() )
					Repaint();
				
				if( GIsEditor )
					SetModeCursor();
			}
			else if( iMessage == WM_SYSCHAR )
			{
				// Perform default processing.
				return DefWindowProc( hWnd, iMessage, wParam, lParam );
			}
			return 0;
			unguard;
		}
		case WM_KILLFOCUS:
		{
			guard(WM_KILLFOCUS);
			SetMouseCapture( 0, 0, 0 );
			Exec( "SETPAUSE 1", this );
			SetDrag( 0 );
			Input->ResetInput();
			if( Client->FullscreenViewport )
				Client->EndFullscreen();
			return 0;
			unguard;
		}
		case WM_SETFOCUS:
		{
			guard(WM_SETFOCUS);

			// Reset viewport's input.
			Exec("SETPAUSE 0",this);
			Input->ResetInput();

			// Make this viewport current.
			MakeCurrent();
			SetModeCursor();
            return 0;

			unguard;
		}
		case WM_ERASEBKGND:
		{
			// Prevent Windows from repainting client background in white.
			return 0;
			break;
		}
		case WM_SETCURSOR:
		{
			guard(WM_SETCURSOR);
			if( (LOWORD(lParam)==1) || GIsSlowTask )
			{
				// In client area or processing slow task.
				if( GIsEditor )
					SetModeCursor();
				return 0;
			}
			else
			{
				// Out of client area.
				return DefWindowProc( hWnd, iMessage, wParam, lParam );
			}
			unguard;
		}
		case WM_LBUTTONDBLCLK:
		{
			if( !Client->FullscreenViewport && !OnHold && SizeX && SizeY )
			{
				Client->Engine->Click( this, MOUSE_LeftDouble, LOWORD(lParam), HIWORD(lParam) );
				if( !IsRealtime() )
					Repaint();
			}
			return 0;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			guard(WM_BUTTONDOWN);

			if( Client->InMenuLoop )
				return DefWindowProc( hWnd, iMessage, wParam, lParam );

			if( iMessage == WM_LBUTTONDOWN )
			{
				MovedSinceLeftClick = 0;
				StartTimeLeftClick = GetMessageTime();
				CauseInputEvent( IK_LeftMouse, IST_Press );
			}
			else if( iMessage == WM_RBUTTONDOWN )
			{
				MovedSinceRightClick = 0;
				StartTimeRightClick = GetMessageTime();
				CauseInputEvent( IK_RightMouse, IST_Press );
			}
			else if( iMessage == WM_MBUTTONDOWN )
			{
				MovedSinceMiddleClick = 0;
				StartTimeMiddleClick = GetMessageTime();
				CauseInputEvent( IK_MiddleMouse, IST_Press );
			}
			SetDrag( 1 );
			return 0;
			unguard;
		}
		case WM_MOUSEACTIVATE:
		{
			// Activate this window and send the mouse-down message.
			return MA_ACTIVATE;
		}
		case WM_ACTIVATE:
		{
			guard(WM_ACTIVATE);

			// If window is becoming inactive, release the cursor.
			if( wParam==0 )
				SetDrag( 0 );

			return 0;
			unguard;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			guard(WM_BUTTONUP);

			// Exit if in menu loop.
			if( Client->InMenuLoop )
				return DefWindowProc( hWnd, iMessage, wParam, lParam );

			// Get mouse cursor position.
			POINT TempPoint={0,0};
			ClientToScreen( hWnd, &TempPoint );
			INT MouseX = SavedCursor.x!=-1 ? SavedCursor.x-TempPoint.x : LOWORD(lParam);
			INT MouseY = SavedCursor.x!=-1 ? SavedCursor.y-TempPoint.y : HIWORD(lParam);

			// Get time interval to determine if a click occured.
			INT DeltaTime, Button;
			EInputKey iKey;
			if( iMessage == WM_LBUTTONUP )
			{
				DeltaTime = GetMessageTime() - StartTimeLeftClick;
				iKey      = IK_LeftMouse;
				Button    = MOUSE_Left;
			}
			else if( iMessage == WM_MBUTTONUP )
			{
				DeltaTime = GetMessageTime() - StartTimeMiddleClick;
				iKey      = IK_MiddleMouse;
				Button    = MOUSE_Middle;
			}
			else
			{
				DeltaTime = GetMessageTime() - StartTimeRightClick;
				iKey      = IK_RightMouse;
				Button    = MOUSE_Right;
			}

			// Send to the input system.
			CauseInputEvent( iKey, IST_Release );

			// Release the cursor.
			if
			(	!Input->KeyDown(IK_LeftMouse)
			&&	!Input->KeyDown(IK_MiddleMouse)
			&&	!Input->KeyDown(IK_RightMouse) 
			&&	!Client->FullscreenViewport )
			{
                SetDrag( 0 );
			}

			// Handle viewport clicking.
			if
			(	!Client->FullscreenViewport
			&&	!OnHold && SizeX && SizeY 
			&&	!(MovedSinceLeftClick || MovedSinceRightClick || MovedSinceMiddleClick) )
			{
				Client->Engine->Click( this, Button, MouseX, MouseY );
				if( !IsRealtime() )
					Repaint();
			}

			// Update times.
			if		( iMessage == WM_LBUTTONUP ) MovedSinceLeftClick	= 0;
			else if	( iMessage == WM_RBUTTONUP ) MovedSinceRightClick	= 0;
			else if	( iMessage == WM_MBUTTONUP ) MovedSinceMiddleClick	= 0;

			return 0;
			unguard;
		}
		case WM_ENTERMENULOOP:
		{
			guard(WM_ENTERMENULOOP);
			Client->InMenuLoop = 1;
			SetDrag( 0 );
			UpdateWindow(); // Update checkmarks and such.
			return 0;
			unguard;
		}
		case WM_EXITMENULOOP:
		{
			guard(WM_EXITMENULOOP);
			Client->InMenuLoop = 0;
			return 0;
			unguard;
		}
		case WM_CANCELMODE:
		{
			guard(WM_CANCELMODE);
			SetDrag( 0 );
			return 0;
			unguard;
		}
		case WM_MOUSEWHEEL:
		{
			guard(WM_MOUSEWHEEL);
			WORD  fwKeys = LOWORD(wParam);
			SWORD zDelta = HIWORD(wParam);
			WORD  xPos   = LOWORD(lParam);
			WORD  yPos   = HIWORD(lParam);
			if( zDelta )
			{
				CauseInputEvent( IK_MouseW, IST_Axis, zDelta );
				if( zDelta < 0 )
				{
					CauseInputEvent( IK_MouseWheelDown, IST_Press );
					CauseInputEvent( IK_MouseWheelDown, IST_Release );
				}
				else if( zDelta > 0 )
				{
					CauseInputEvent( IK_MouseWheelUp, IST_Press );
					CauseInputEvent( IK_MouseWheelUp, IST_Release );
				}
			}
			return 0;
			unguard;
		}
		case WM_MOUSEMOVE:
		{
			guard(WM_MOUSEMOVE);

			// If in a window, see if cursor has been captured; if not, ignore mouse movement.
			if( Client->InMenuLoop )
				break;

			// Get window rectangle.
			RECT TempRect;
			GetClientRect( hWnd, &TempRect );
            WORD Buttons = wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);

			// If cursor isn't captured, just do MousePosition.
            if( !Client->FullscreenViewport && SavedCursor.x==-1 )
			{
				// Do mouse messaging.
				POINTS Point = MAKEPOINTS(lParam);
				DWORD ViewportButtonFlags = 0;
				if( Buttons & MK_LBUTTON     ) ViewportButtonFlags |= MOUSE_Left;
				if( Buttons & MK_RBUTTON     ) ViewportButtonFlags |= MOUSE_Right;
				if( Buttons & MK_MBUTTON     ) ViewportButtonFlags |= MOUSE_Middle;
				if( Input->KeyDown(IK_Shift) ) ViewportButtonFlags |= MOUSE_Shift;
				if( Input->KeyDown(IK_Ctrl ) ) ViewportButtonFlags |= MOUSE_Ctrl;
				if( Input->KeyDown(IK_Alt  ) ) ViewportButtonFlags |= MOUSE_Alt;
				Client->Engine->MousePosition( this, Buttons, Point.x-TempRect.left, Point.y-TempRect.top );
				break;
			}

			// Get center of window.			
			POINT TempPoint, Base;
			TempPoint.x = (TempRect.left + TempRect.right )/2;
			TempPoint.y = (TempRect.top  + TempRect.bottom)/2;
			Base = TempPoint;

			// Movement accumulators.
			UBOOL Moved=0;
			INT Cumulative=0;

			// Grab all pending mouse movement.
			INT DX=0, DY=0;
			Loop:
			Buttons		  = wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);
            POINTS Points = MAKEPOINTS(lParam);
			INT X         = Points.x - Base.x;
			INT Y         = Points.y - Base.y;
			Cumulative += Abs(X) + Abs(Y);
			DX += X;
			DY += Y;

			// Process valid movement.
			DWORD ViewportButtonFlags = 0;
			if( Buttons & MK_LBUTTON     ) ViewportButtonFlags |= MOUSE_Left;
			if( Buttons & MK_RBUTTON     ) ViewportButtonFlags |= MOUSE_Right;
			if( Buttons & MK_MBUTTON     ) ViewportButtonFlags |= MOUSE_Middle;
			if( Input->KeyDown(IK_Shift) ) ViewportButtonFlags |= MOUSE_Shift;
			if( Input->KeyDown(IK_Ctrl ) ) ViewportButtonFlags |= MOUSE_Ctrl;
			if( Input->KeyDown(IK_Alt  ) ) ViewportButtonFlags |= MOUSE_Alt;

			// Move viewport with buttons.
			if( X || Y )
			{
				Moved = 1;
				Client->Engine->MouseDelta( this, ViewportButtonFlags, X, Y );
			}

			// Handle any more mouse moves.
			MSG Msg;
			if( PeekMessage( &Msg, hWnd, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ) )
			{
				lParam = Msg.lParam;
				wParam = Msg.wParam;
				Base.x = Points.x;
				Base.y = Points.y;
				goto Loop;
			}

			// Set moved-flags.
			if( Cumulative>4 )
			{
				if( wParam & MK_LBUTTON ) MovedSinceLeftClick   = 1;
				if( wParam & MK_RBUTTON ) MovedSinceRightClick  = 1;
				if( wParam & MK_MBUTTON ) MovedSinceMiddleClick = 1;
			}

			// Send to input subsystem.
			if( DX ) CauseInputEvent( IK_MouseX, IST_Axis, +DX );
			if( DY ) CauseInputEvent( IK_MouseY, IST_Axis, -DY );

			// Put cursor back in middle of window.
			if( DX || DY )
			{
				ClientToScreen( hWnd, &TempPoint );
				RECT Rect;
				GetWindowRect( GetDesktopWindow(), &Rect );
				/*!!slower: mouse_event
				(
					MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE,
					appRound( (TempPoint.x+0.5)*65536.0/Rect.right  ),
					appRound( (TempPoint.y+0.5)*65536.0/Rect.bottom ),
					0,
					0
				);*/
				SetCursorPos( TempPoint.x, TempPoint.y );
			}

			// Viewport isn't realtime, so we must update the frame here and now.
			if( Moved && !IsRealtime() )
			{
				if( Input->KeyDown(IK_Space) )
					for( INT i=0; i<Client->Viewports.Num(); i++ )
						Client->Viewports(i)->Repaint();
				else
					Repaint();
			}

			// Dispatch keyboard events.
			while( PeekMessage( &Msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) )
			{
				TranslateMessage( &Msg );
				DispatchMessage( &Msg );
			}
			GTickDue = 1;

			return 0;
			unguard;
		}
		case WM_SIZE:
		{
			guard(WM_SIZE);
			if( !Client->FullscreenViewport )
			{
				SetFrameBufferSize( LOWORD(lParam), HIWORD(lParam), ColorBytes, BLIT_DEFAULT );
				Repaint();
      			return 0;
        	}
			else return 0;
			unguard;
		}
		case WM_SYSCOMMAND:
		{
			guard(WM_SYSCOMMAND);
			int nID = wParam & 0xFFF0;
			if( (nID==SC_SCREENSAVE) || (nID==SC_MONITORPOWER) )
			{
				// Return 1 to prevent screen saver.
				if( nID==SC_SCREENSAVE )
					debugf( NAME_Log, "Received SC_SCREENSAVE" );
				else
					debugf( NAME_Log, "Received SC_MONITORPOWER" );
				return 0;
			}
			else if( nID==SC_MAXIMIZE )
			{
				// Maximize.
				Exec( "ToggleFullscreen", GSystem );
				return 0;
			}
			else return DefWindowProc(hWnd,iMessage,wParam,lParam);
			break;
			unguard;
		}
		case WM_POWER:
		{
			guard(WM_POWER);
			if( wParam )
			{
				if( wParam == PWR_SUSPENDREQUEST )
				{
					debugf( NAME_Log, "Received WM_POWER suspend" );

					// Prevent powerdown if dedicated server or using joystick.
					if( 1 )
						return PWR_FAIL;
					else
						return PWR_OK;
				}
				else
				{
					debugf( NAME_Log, "Received WM_POWER" );
					return DefWindowProc( hWnd, iMessage, wParam, lParam );
				}
			}
			return 0;
			unguard;
		}
		case WM_DISPLAYCHANGE:
		{
			guard(WM_DISPLAYCHANGE);
			debugf( NAME_Log,"Viewport %s: WM_DisplayChange", GetName() );
			unguard;
			return 0;
		}
		case WM_WININICHANGE:
		{
			guard(WM_WININICHANGE);
			if( !DeleteDC(Client->hMemScreenDC) )
				appErrorf( "DeleteDC failed: %s", winError() );
			Client->hMemScreenDC = CreateCompatibleDC (NULL);
			return 0;
			unguard;
		}
		default:
		{
			guard(WM_UNKNOWN);
			return DefWindowProc( hWnd, iMessage, wParam, lParam );
			unguard;
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	UWindowsClient Init & Exit.
-----------------------------------------------------------------------------*/

//
// UWindowsClient constructor.
//
UWindowsClient::UWindowsClient()
{
	guard(UWindowsClient::UWindowsClient);
	unguard;
}

//
// Static init.
//
void UWindowsClient::InternalClassInitializer( UClass* Class )
{
	guard(UWindowsClient::InternalClassInitializer);
	if( appStricmp( Class->GetName(), "WindowsClient" )==0 )
	{
		new(Class,"UseDirectDraw",     RF_Public)UBoolProperty(CPP_PROPERTY(UseDirectDraw),     "Display",  CPF_Config );
		new(Class,"UseJoystick",       RF_Public)UBoolProperty(CPP_PROPERTY(UseJoystick),       "Display",  CPF_Config );
		new(Class,"StartupFullscreen", RF_Public)UBoolProperty(CPP_PROPERTY(StartupFullscreen), "Display",  CPF_Config );
		new(Class,"SlowVideoBuffering",RF_Public)UBoolProperty(CPP_PROPERTY(SlowVideoBuffering),"Display",  CPF_Config );
		new(Class,"DeadZoneXYZ",       RF_Public)UBoolProperty(CPP_PROPERTY(DeadZoneXYZ),       "Joystick", CPF_Config );
		new(Class,"DeadZoneRUV",       RF_Public)UBoolProperty(CPP_PROPERTY(DeadZoneRUV),       "Joystick", CPF_Config );
		new(Class,"InvertVertical",    RF_Public)UBoolProperty(CPP_PROPERTY(InvertVertical),    "Joystick", CPF_Config );
		new(Class,"ScaleXYZ",          RF_Public)UFloatProperty(CPP_PROPERTY(ScaleXYZ),         "Joystick", CPF_Config );
		new(Class,"ScaleRUV",          RF_Public)UFloatProperty(CPP_PROPERTY(ScaleRUV),         "Joystick", CPF_Config );
	}
	unguard;
}

//
// Configuration change.
//
void UWindowsClient::PostEditChange()
{
	guard(UWindowsClient::PostEditChange);
	Super::PostEditChange();

	// Joystick.
	ZeroMemory( &JoyCaps, sizeof(JoyCaps) );
	if( UseJoystick )
	{
		INT nJoys = joyGetNumDevs();
		if( nJoys )
		{
			if( joyGetDevCaps( JOYSTICKID1, &JoyCaps, sizeof(JoyCaps) )==JOYERR_NOERROR )
			{
				debugf( "Detected joysticks: %i (%s)", nJoys, JoyCaps.szOEMVxD ? JoyCaps.szOEMVxD : "None" );
			}
			else debugf( "joyGetDevCaps failed" );
		}
		else debugf( "No joystick detected" );
	}

	unguard;
}

//
// Initialize the platform-specific viewport manager subsystem.
// Must be called after the Unreal object manager has been initialized.
// Must be called before any viewports are created.
//
void UWindowsClient::Init( UEngine* InEngine )
{
	guard(UWindowsClient::UWindowsClient);

	// Init base.
	UClient::Init( InEngine );

	// Register window class.
	IMPLEMENT_WINDOWCLASS(UWindowsViewport,hInstance,GIsEditor ? CS_DBLCLKS : 0);

	// Create a working DC compatible with the screen, for CreateDibSection.
	hMemScreenDC = CreateCompatibleDC( NULL );
	if( !hMemScreenDC )
		appErrorf( "CreateCompatibleDC failed: %S", winError() );

	// Get mouse info.
	SystemParametersInfo( SPI_GETMOUSE, 0, NormalMouseInfo, 0 );
	debugf( NAME_Init, "Mouse info: %i %i %i", NormalMouseInfo[0], NormalMouseInfo[1], NormalMouseInfo[2] );
	CaptureMouseInfo[0] = 0;     // First threshold.
	CaptureMouseInfo[1] = 0;     // Second threshold.
	CaptureMouseInfo[2] = 65536; // Speed.

	// Initialize DirectDraw after the viewport array is allocated.
	ddInit();

	// Fix up the environment variables for 3dfx.
	_putenv( "SST_RGAMMA=" );
	_putenv( "SST_GGAMMA=" );
	_putenv( "SST_BGAMMA=" );
	_putenv( "FX_GLIDE_NO_SPLASH=1" );

	// Note configuration.
	PostEditChange();

	// Success.
	debugf( NAME_Init, "Client initialized" );
	unguard;
}

//
// Shut down the platform-specific viewport manager subsystem.
//
void UWindowsClient::Destroy()
{
	guard(UWindowsClient::Destroy);

	// Shut down DirectDraw.
	if( UseDirectDraw && !ParseParam(appCmdLine(),"noddraw") )
		ddExit();

	// Stop capture.
	SetCapture( NULL );
	ClipCursor( NULL );
	SystemParametersInfo( SPI_SETMOUSE, 0, NormalMouseInfo, 0 );

	// Clean up Windows resources.
	if( !DeleteDC( hMemScreenDC ) )
		debugf( NAME_Exit, "DeleteDC failed %i", GetLastError() );

	debugf( NAME_Exit, "Windows client shut down" );
	UClient::Destroy();
	unguard;
}

//
// Failsafe routine to shut down viewport manager subsystem
// after an error has occured. Not guarded.
//
void UWindowsClient::ShutdownAfterError()
{
	debugf( NAME_Exit, "Executing UWindowsClient::ShutdownAfterError" );
	SetCapture( NULL );
	ClipCursor( NULL );
	SystemParametersInfo( SPI_SETMOUSE, 0, NormalMouseInfo, 0 );
  	ShowCursor( TRUE );
	if( Engine && Engine->Audio )
	{
		Engine->Audio->ConditionalShutdownAfterError();
	}
	if( FullscreenViewport )
	{
		UViewport* Viewport = FullscreenViewport;
		FullscreenViewport = NULL;
		if( FullscreenhWndDD )
			ddEndMode();
		if( Viewport->RenDev )
			Viewport->RenDev->Exit();
	}
	for( INT i=Viewports.Num()-1; i>=0; i-- )
   	{
		UWindowsViewport* Viewport = (UWindowsViewport*)Viewports(i);
		DestroyWindow( Viewport->hWnd );
	}
	Super::ShutdownAfterError();
}

/*-----------------------------------------------------------------------------
	UWindowsClient Cursor & Update Functions.
-----------------------------------------------------------------------------*/

//
// Enable or disable all viewport windows that have ShowFlags set (or all if ShowFlags=0).
//
void UWindowsClient::EnableViewportWindows( DWORD ShowFlags, int DoEnable )
{
	guard(UWindowsClient::EnableViewportWindows);
  	for( int i=0; i<Viewports.Num(); i++ )
	{
		UWindowsViewport* Viewport = (UWindowsViewport*)Viewports(i);
		if( (Viewport->Actor->ShowFlags & ShowFlags)==ShowFlags )
			EnableWindow( Viewport->hWnd, DoEnable );
	}
	unguard;
}

//
// Show or hide all viewport windows that have ShowFlags set (or all if ShowFlags=0).
//
void UWindowsClient::ShowViewportWindows( DWORD ShowFlags, int DoShow )
{
	guard(UWindowsClient::ShowViewportWindows); 	
	for( int i=0; i<Viewports.Num(); i++ )
	{
		UWindowsViewport* Viewport = (UWindowsViewport*)Viewports(i);
		if( (Viewport->Actor->ShowFlags & ShowFlags)==ShowFlags )
			ShowWindow(Viewport->hWnd,DoShow?SW_SHOWNORMAL:SW_HIDE);
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	UWindowsClient Fullscreen Functions.
-----------------------------------------------------------------------------*/

//
// If in fullscreen mode, end it and return to Windows.
//
void UWindowsClient::EndFullscreen()
{
	guard(UWindowsClient::EndFullscreen);

	UWindowsViewport* Viewport = Cast<UWindowsViewport>(FullscreenViewport);
	if( Viewport )
	{
		// Remember saved info.
		RECT* Rect		 = &Viewport->SavedWindowRect;
		INT   SXR		 =  Viewport->SavedX;
		INT   SYR		 =  Viewport->SavedY;
		INT   ColorBytes =  Viewport->SavedColorBytes;
		INT   Caps		 =  Viewport->SavedCaps;

		// Shut down hardware and DirectDraw.
		Viewport->Hold();
		if( !Viewport->RenDev->FrameBuffered )
		{
			Viewport->RenDev->Exit();
			Viewport->RenDev = NULL;
			TryRenderDevice( Viewport, "ini:Engine.Engine.WindowedRenderDevice", 0 );
			check(Viewport->RenDev);
		}
		if( FullscreenhWndDD )
		{
			debugf( NAME_Log, "DirectDraw session ending" );
			if( Engine->Audio && !GIsEditor )
				Engine->Audio->SetViewport( NULL );
			ddEndMode();
			if( Engine->Audio && !GIsEditor )
				Engine->Audio->SetViewport( Viewport );
		}

		// Restore saved info.
		Viewport->Caps     = Caps;
		FullscreenViewport = NULL;
		FullscreenhWndDD   = NULL;

		// Move the window where it belongs.
		MoveWindow( Viewport->hWnd, Rect->left, Rect->top, Rect->right-Rect->left, Rect->bottom-Rect->top, 1 );

		// Unhold it.
		Viewport->Unhold();
		Viewport->SetFrameBufferSize( SXR, SYR, ColorBytes, BLIT_DIBSECTION );

		// Fix always-on-top status affected by DirectDraw.
		Viewport->SetTopness();

		// Stop mouse capture.
		Viewport->SetDrag( 0 );
	}

	unguard;
}

//
// Try to make this viewport fullscreen, matching the fullscreen
// mode of the nearest x-size to the current window. If already in
// fullscreen, returns to non-fullscreen.
//
void UWindowsViewport::MakeFullscreen( INT NewX, INT NewY, UBOOL UpdateProfile )
{
	guard(UWindowsViewport::MakeFullscreen);

	// If someone else is fullscreen, stop them.
	if( Client->FullscreenViewport )
		Client->EndFullscreen();

	// Save this window.
	GetWindowRect( hWnd, &SavedWindowRect );
	SavedColorBytes	= ColorBytes;
	SavedCaps = Caps;
	SavedX = SizeX;
	SavedY = SizeY;

	// Render device specific mode switch.
	if( !RenDev->FrameBuffered )
	{
		// Fullscreen rendering device.
		if( Client->FullscreenViewport )
			Client->EndFullscreen();
		SizeX = NewX;
		SizeY = NewY;
		Client->FullscreenViewport = this;
		Client->FullscreenhWndDD = NULL;
		SetDrag( 1 );
		SetMouseCapture( 1, 1, 0 );
	}
	else
	{
		// Go into fullscreen, matching closest DirectDraw mode to current window size.
		INT BestMode=-1;
		INT BestDelta=MAXINT;		
		for( INT i=0; i<Client->ddNumModes; i++ )
		{
			INT Delta = Abs(Client->ddModeWidth[i]-NewX) + Abs(Client->ddModeHeight[i]-NewY);
			if( Delta < BestDelta )
			{
				BestMode  = i;
				BestDelta = Delta;
			}
		}
		if( BestMode>=0 )
		{
			Client->ddSetViewport( this, Client->ddModeWidth[BestMode], Client->ddModeHeight[BestMode], ColorBytes, 0 );
		}
	}
	if( UpdateProfile )
	{
		Client->ViewportX = NewX;
		Client->ViewportY = NewY;
		Client->SaveConfig();
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	UWindowsClient Polling & Timing Functions.
-----------------------------------------------------------------------------*/

//
// Perform background processing.  Should be called 100 times
// per second or more for best results.
//
void UWindowsClient::Poll()
{
	guard(UWindowsClient::Poll);

	// Tell DirectDraw to lock its locked surfaces.  When a DirectDraw surface
	// is locked, the Win16Mutex is held, preventing DirectSound mixing from
	// taking place.  If a DirectDraw surface is locked for more than
	// approximately 1/100th of a second, skipping can occur in the audio output.
	static DOUBLE LastTime = appSeconds();
	DOUBLE        Time     = appSeconds();
	if( dd && ddFrontBuffer && Time-LastTime>DD_POLL_TIME )
	{
		HRESULT Result;
		Result   = ddBackBuffer->Unlock(ddSurfaceDesc.lpSurface);
		Result   = ddBackBuffer->Lock( NULL, &ddSurfaceDesc, DDLOCK_WAIT|DD_OTHERLOCKFLAGS,NULL);
		LastTime = Time;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Task functions.
-----------------------------------------------------------------------------*/

//
// Perform timer-tick processing on all visible viewports.  This causes
// all realtime viewports, and all non-realtime viewports which have been
// updated, to be blitted.
//
void UWindowsClient::Tick()
{
	guard(UWindowsClient::Tick);

	// Blit any viewports that need blitting.
	UWindowsViewport* BestViewport = NULL;
  	for( INT i=0; i<Viewports.Num(); i++ )
	{
		UWindowsViewport* Viewport = CastChecked<UWindowsViewport>(Viewports(i));
		if( !IsWindow(Viewport->hWnd) )
		{
			// Window was closed via close button.
			delete Viewport;
			return;
		}
  		else if
		(	Viewport->IsRealtime()
		&&	(Viewport==FullscreenViewport || FullscreenViewport==NULL)
		&&	Viewport->SizeX && Viewport->SizeY && !Viewport->OnHold
		&&	(!BestViewport || Viewport->LastUpdateTime<BestViewport->LastUpdateTime) )
		{
			BestViewport = Viewport;
		}
	}
	if( BestViewport )
		BestViewport->Repaint();
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
