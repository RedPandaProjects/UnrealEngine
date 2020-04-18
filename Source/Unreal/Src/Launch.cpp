/*=============================================================================
	Launch.cpp: Unreal launcher.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#include "LaunchPrivate.h"
#include "Window.h"
#include "Res\LaunchRes.h"
#pragma warning( disable: 4355 )

extern CORE_API FGlobalPlatform GTempPlatform;
extern "C" {HINSTANCE hInstance;}
extern "C" {char GPackage[64]="Launch";}

/*-----------------------------------------------------------------------------
	WinMain.
-----------------------------------------------------------------------------*/

//
// Main window entry point.
//
INT WINAPI WinMain( HINSTANCE hInInstance, HINSTANCE hPrevInstance, char* InCmdLine, INT nCmdShow )
{
	/*{
		const rlim_t kStackSize = 16 * 1024 * 1024;   // min stack size = 16 MB
		struct rlimit rl;
		int result;

		result = getrlimit(RLIMIT_STACK, &rl);
		if (result == 0)
		{
			if (rl.rlim_cur < kStackSize)
			{
				rl.rlim_cur = kStackSize;
				result = setrlimit(RLIMIT_STACK, &rl);
				if (result != 0)
				{
					fprintf(stderr, "setrlimit returned result = %d\n", result);
				}
			}
		}
	}*/
	// Remember instance.
	GIsStarted = 1;
	hInstance = hInInstance;

	// Set package name.
	appStrcpy( GPackage, appPackage() );

	// Init mode.
	GIsServer = 1;
	GIsClient = !ParseParam(appCmdLine(),"SERVER") && !ParseParam(appCmdLine(),"MAKE");
	GIsEditor = ParseParam(appCmdLine(),"EDITOR") || ParseParam(appCmdLine(),"MAKE");

	// Init windowing.
	appChdir( appBaseDir() );
	InitWindowing();

	// Init log window.
	GExecHook = GThisExecHook;
	GLog = new WLog( "" );
	GLog->OpenWindow( NULL, 1 );

	// Begin.
#ifndef _DEBUG
	try
	{
#endif
		// Start main loop.
		GIsGuarded=1;
		GSystem = &GTempPlatform;
		UEngine* Engine = InitEngine();
		if( !GIsRequestingExit )
			MainLoop( Engine );
		ExitEngine( Engine );
		GIsGuarded=0;
#ifndef _DEBUG
	}
	catch( ... )
	{
		// Crashed.
		try {HandleError();} catch( ... ) {}
	}
#endif

	// Shut down.
	GExecHook=NULL;
	if( GLog )
		delete GLog;
	appExit();
	GIsStarted = 0;
	return 0;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
