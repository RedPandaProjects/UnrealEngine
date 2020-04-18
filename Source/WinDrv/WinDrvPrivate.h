/*=============================================================================
	WinDrvPrivate.cpp: Unreal Windows viewport and platform driver.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#pragma warning( disable : 4201 )
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <ddraw.h>
#include "Res\WinDrvRes.h"
#include "Engine.h"
#include "Window.h"

/*-----------------------------------------------------------------------------
	Forward declares.
-----------------------------------------------------------------------------*/

class UWindowsViewport;
class UWindowsClient;

void WINDOW_API InitWindowing();

/*-----------------------------------------------------------------------------
	UWindowsViewport.
-----------------------------------------------------------------------------*/

//
// Viewport window status.
//
enum EWinViewportStatus
{
	WIN_ViewportOpening	= 0, // Viewport is opening and hWnd is still unknown.
	WIN_ViewportNormal	= 1, // Viewport is operating normally, hWnd is known.
	WIN_ViewportClosing	= 2, // Viewport is closing and CloseViewport has been called.
};

//
// Blitting types.
//
enum EWindowsBlitType
{
	BLIT_NONE			= 0,
	BLIT_DIBSECTION		= 1,
	BLIT_DIRECTDRAW		= 2,
	BLIT_DEFAULT		= BLIT_DIBSECTION,
};

//
// A Windows viewport.
//
class DLL_EXPORT UWindowsViewport : public UViewport, public WWindow
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UWindowsViewport,UViewport,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(UWindowsViewport)

	// Variables.
	class UWindowsClient* Client;
	EWinViewportStatus  Status;
	HBITMAP				hBitmap;
	HMENU				hMenu;
	HWND				ParentWindow;

	// Info saved during captures and fullscreen sessions.
	POINT				SavedCursor;
	RECT				SavedWindowRect;
	INT					SavedColorBytes;
	INT					SavedCaps;
	INT					SavedX, SavedY;

	// DIB section bitmap info.
	struct
	{
		BITMAPINFOHEADER BitmapHeader;
		RGBQUAD BitmapColors[256];
	};

	// Constructor.
	UWindowsViewport( ULevel* InLevel, UWindowsClient* InClient );

	// UObject interface.
	void Destroy()
	{
		guard(UWindowsViewport::Destroy);
		UViewport::Destroy();
		MaybeDestroy();
		unguard;
	}

	// UViewport interface.
	UBOOL Lock( FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData=NULL, INT* HitSize=0 );
	void Unlock( UBOOL Blit );
	UBOOL Exec( const char* Cmd, FOutputDevice* Out );
	void Repaint();
	void SetModeCursor();
	void UpdateWindow();
	void OpenWindow( DWORD ParentWindow, UBOOL Temporary, INT NewX, INT NewY, INT OpenX, INT OpenY );
	void CloseWindow();
	void UpdateInput( UBOOL Reset );
	void MakeCurrent();
	void MakeFullscreen( INT NewX, INT NewY, UBOOL UpdateProfile );
	void* GetWindow();
	void SetMouseCapture( UBOOL Capture, UBOOL Clip, UBOOL FocusOnly );

	// WWindow interface.
	const char* GetWindowClassName() {return "UnrealUWindowsViewport";}

	// UWindowsViewport interface.
	void FindAvailableModes();
	void SetClientSize( INT NewX, INT NewY, UBOOL UpdateProfile );
	void SetFrameBufferSize( INT NewX, INT NewY, INT NewColorBytes, EWindowsBlitType BlitType );
	void TryHardware3D( UWindowsViewport* Viewport );
	void SetTopness();
	LONG WndProc( UINT Message, UINT wParam, LONG lParam );
	UBOOL CauseInputEvent( INT iKey, EInputAction Action, FLOAT Delta=0.0 );
	UBOOL JoystickInputEvent( FLOAT Delta, EInputKey Key, FLOAT Scale, UBOOL DeadZone );
};

/*-----------------------------------------------------------------------------
	UWindowsClient.
-----------------------------------------------------------------------------*/

//
// Windows implementation of the client.
//
class DLL_EXPORT UWindowsClient : public UClient, public FNotifyHook
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UWindowsClient,UClient,CLASS_Transient|CLASS_Config)

	// Constants.
	enum {MAX_DD=4};
	enum {DD_MAX_MODES=10};
	typedef HRESULT (WINAPI *DD_CREATE_FUNC)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
	typedef HRESULT (WINAPI *DD_ENUM_FUNC  )(LPDDENUMCALLBACK lpCallback,LPVOID lpContext);
	#define DD_POLL_TIME 0.03 /* Seconds between forced locks/unlocks */

	// Configuration.
	UBOOL				UseDirectDraw;
	UBOOL				UseJoystick;
	UBOOL				StartupFullscreen;
	UBOOL				SlowVideoBuffering;
	UBOOL				DeadZoneXYZ;
	UBOOL				DeadZoneRUV;
	UBOOL				InvertVertical;
	FLOAT				ScaleXYZ;
	FLOAT				ScaleRUV;

	// Variables.
	HDC					hMemScreenDC;
	HWND				FullscreenhWndDD;
	UBOOL				InMenuLoop;
	INT					NormalMouseInfo[3];
	INT					CaptureMouseInfo[3];
	GUID				ddGUIDs[MAX_DD];
	INT					NumDD;
	IDirectDraw2*		dd;
	IDirectDrawSurface*	ddFrontBuffer;
	IDirectDrawSurface*	ddBackBuffer;
	DDSURFACEDESC 		ddSurfaceDesc;
	DD_CREATE_FUNC		ddCreateFunc;
	DD_ENUM_FUNC		ddEnumFunc;
	JOYCAPS				JoyCaps;
	INT					ddModeWidth[DD_MAX_MODES], ddModeHeight[DD_MAX_MODES];
	INT					ddNumModes;
	WConfigProperties*	ConfigProperties;

	// Constructors.
	UWindowsClient();
	static void InternalClassInitializer( UClass* Class );

	// FNotifyHook interface.
	void NotifyDestroy( void* Src )
	{
		guard(UWindowsClient::NotifyDestroy);
		if( Src==ConfigProperties )
			ConfigProperties=NULL;
		unguard;
	}

	// UObject interface.
	void Destroy();
	void PostEditChange();
	void ShutdownAfterError();

	// UClient interface.
	void Init( UEngine* InEngine );
	void ShowViewportWindows( DWORD ShowFlags, int DoShow );
	void EnableViewportWindows( DWORD ShowFlags, int DoEnable );
	void Poll();
	UViewport* CurrentViewport();
	UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );
	void Tick();
	class UViewport* NewViewport( class ULevel* InLevel, const FName Name );

	// UWindowsClient interface.
	void EndFullscreen();
	int Toggle( HMENU hMenu, INT Item );
	int ddInit();
	void ddExit();
	char* ddError( HRESULT Result );
	UBOOL ddSetMode( HWND hWndOwner, INT Width, INT Height, INT ColorBytes, INT& Caps );
	void ddEndMode();
	UBOOL ddSetViewport( UViewport* Viewport, INT Width, INT Height, INT ColorBytes, INT RequestedCaps );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
