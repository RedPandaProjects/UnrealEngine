/*=============================================================================
	Main.cpp: Unreal main startup/shutdown code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#pragma warning( disable : 4201 )
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include "Engine.h"
#include "UnRender.h"
#include "Window.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

extern CORE_API FGlobalPlatform GTempPlatform;
WINDOW_API WLog* GLog=NULL;
WINDOW_API UBOOL GTickDue=0;

/*-----------------------------------------------------------------------------
	Exec hook.
-----------------------------------------------------------------------------*/

// FExecHook.
class FExecHook : public FExec
{
	UBOOL Exec( const char* Cmd, FOutputDevice* Out )
	{
		if( ParseCommand(&Cmd,"SHOWLOG") )
		{
			if( GLog )
			{
				ShowWindow( *GLog, SW_SHOW );
				SetFocus( *GLog );
			}
			return 1;
		}
		else if( ParseCommand(&Cmd,"EDITACTOR") )
		{
			UClass* Class;
			FLOAT MinDist=999999.0;
			TObjectIterator<UEngine> EngineIt;
			if
			(	EngineIt
			&&	EngineIt->Client
			&&	EngineIt->Client->Viewports.Num()
			&&	ParseObject<UClass>( Cmd, "CLASS=", Class, ANY_PACKAGE ) )
			{
				AActor* Player  = EngineIt->Client->Viewports(0)->Actor;
				AActor* Found   = NULL;
				FLOAT   MinDist = 999999.0;
				for( TObjectIterator<AActor> It; It; ++It )
				{
					FLOAT Dist=FDist(It->Location,Player->Location);
					if
					(	It->XLevel==Player->XLevel
					&&	It->IsA( Class )
					&&	Dist<MinDist )
					{
						MinDist = Dist;
						Found   = *It;
					}
				}
				if( Found )
				{
					WObjectProperties* P = new WObjectProperties( "EditActor", 0, "" );
					P->OpenWindow();
					P->Root.SetObjects( (UObject**)&Found, 1 );
					ShowWindow( *P, SW_SHOW );
				}
				else Out->Logf( "Actor not found" );
			}
			else Out->Logf( "Missing class" );
			return 1;
		}
		else if( ParseCommand(&Cmd,"HIDELOG") )
		{
			if( GLog )
				ShowWindow( *GLog, SW_HIDE );
			return 1;
		}
		/*else if( ParseCommand(&Cmd,"GENPASS") )
		{
			char Name[256];
			UBOOL CheckPassword( const char* Name, const char* Password );
			if( Parse( Cmd, "NAME=", Name, sizeof(Name) ) )
			{
				debugf( NAME_Log, "Generating..." );
				while( 1 )
				{
					char Password[16];
					for( int i=0; i<15; i++ )
					{
						int R = rand() % 36;
						Password[i] = R<10 ? '0'+R : 'A'-10+R;
					}
					Password[i]=0;
					if( CheckPassword( Name, Password ) )
					{
						Out->Logf( "Password: %s", Password );
						break;
					}
				}
			}
			else Out->Log( "Missing name" );
			return 1;
		}*/
		else return 0;
	}
};
FExecHook GLocalHook;
WINDOW_API FExec* GThisExecHook = &GLocalHook;

/*-----------------------------------------------------------------------------
	Passwords.
-----------------------------------------------------------------------------*/

// Check password.
/*WINDOW_API UBOOL CheckPassword( const char* Name, const char* Password )
{
	char Mash[256];
	if( stricmp( Password, "LTQ0XX0O7I74IG8" )==0 )
		return 0;
	if( stricmp( Password, "DJ4ECRDCIABCG7R" )==0 )
		return 0;
	appSprintf( Mash,"ss%ss%ss", Name, Password );
	strupr(Mash);
	return (appMemCrc((BYTE*)Mash,strlen(Mash))&0xffffff)==0x6ab2de;
}*/

/*-----------------------------------------------------------------------------
	Registration.
-----------------------------------------------------------------------------*/

// Not guarded.
static void RegDoSet( HKEY Key, FString Value )
{
	RegSetValue( Key, "", REG_SZ, *Value, Value.Length() );
}

// Not guarded.
static void RegSetEx( HKEY Key, FString SubKey, FString Value )
{
	RegSetValueEx(Key,*SubKey,0,REG_SZ,(BYTE*)*Value,Value.Length());
}

static const char* ReadConfig( const char* Key, const char* Section="Registry" )
{
	static char Result[256];
	*Result = 0;
	GetConfigString( Section, Key, Result, ARRAY_COUNT(Result) );
	return Result;
}

// Not guarded.
WINDOW_API void RegisterFileTypes()
{
	FString Base(appBaseDir());
	HKEY Key1, Key2, Key3, Key4;

	// Register .unr.
	verify(RegCreateKey(HKEY_CLASSES_ROOT, *(FString(".") + ReadConfig("MapExt", "URL")), &Key1) == ERROR_SUCCESS);
		RegDoSet				(Key1,ReadConfig("OleMapType"));
	RegCloseKey					(Key1);

	// Register Unreal.Map.
	RegCreateKey				(HKEY_CLASSES_ROOT,ReadConfig("OleMapType"),&Key1);
		RegDoSet				(Key1,ReadConfig("OleMapDescription"));
		RegCreateKey			(Key1,"DefaultIcon",&Key2);
			RegDoSet			(Key2,Base + appPackage() + ".exe,0");
		RegCloseKey				(Key2);
		RegCreateKey			(Key1,"shell",&Key2);
			RegDoSet			(Key2,"open");
			RegCreateKey		(Key2,"open",&Key3);
				RegDoSet		(Key3,Localize("Windows","PlayCommand",appPackage()));
				RegCreateKey	(Key3,"command",&Key4);
					RegDoSet	(Key4,Base + appPackage() + ".exe \"%1\"");
				RegCloseKey		(Key4);
			RegCloseKey			(Key3);
			if( appFSize(GetConfigStr("Registry","EditorApp"))>0 )
			{
				RegCreateKey	(Key2,"edit",&Key3);
					RegDoSet	(Key3,Localize("Windows","EditCommand",appPackage()));
					RegCreateKey(Key3,"command",&Key4);
						RegDoSet(Key4,Base + GetConfigStr("Registry","EditorApp") + " FILE=\"%1\"");
					RegCloseKey	(Key4);
				RegCloseKey		(Key3);
			}
		RegCloseKey				(Key2);
	RegCloseKey					(Key1);

	// Register unreal protocol with Internet Explorer.
	RegCreateKey				(HKEY_CLASSES_ROOT,ReadConfig("Protocol","Url"),&Key1);
		RegDoSet				(Key1,FString("URL:")+ReadConfig("ProtocolDescription","Url"));
		RegCreateKey			(Key1,"DefaultIcon",&Key2);
			RegDoSet			(Key2,Base + appPackage() + ".exe");
		RegCloseKey				(Key2);
		RegSetEx				(Key1,"URL Protocol","");
		RegCreateKey			(Key1,"shell",&Key2);
			RegDoSet			(Key2,"open");
			RegCreateKey		(Key2,"open",&Key3);
				RegDoSet		(Key3,Localize("Windows","PlayCommand",appPackage()));
				RegCreateKey	(Key3,"command",&Key4);
					RegDoSet	(Key4,Base + appPackage() + ".exe \"%1\"");
				RegCloseKey		(Key4);
			RegCloseKey			(Key3);
		RegCloseKey				(Key2);
	RegCloseKey					(Key1);
}

/*-----------------------------------------------------------------------------
	Startup and shutdown.
-----------------------------------------------------------------------------*/

//
// Handle an error.
//
WINDOW_API void HandleError()
{
	GIsGuarded=0;
	GIsCriticalError=1;
	debugf( NAME_Exit, "Shutting down after catching exception" );
	GObj.ShutdownAfterError();
	debugf( NAME_Exit, "Exiting due to exception" );
	GErrorHist[ARRAY_COUNT(GErrorHist)-1]=0;
	MessageBox( NULL, GErrorHist, LocalizeError("Critical"), MB_OK|MB_ICONERROR|MB_TASKMODAL );
}

//
// Initialize.
//
UEngine* InitEngine()
{
	guard(InitEngine);

	// Create mutex so installer knows we're running.
	CreateMutex( NULL, 0, "UnrealIsRunning" );

	// Platform init.
	appInit();
//	RegisterFileTypes();
	GDynMem.Init( 65536 );

	// Check password.
	/*char Name[256]="", Password[256]="";
	GetConfigString( "Login", "Name",     Name,     sizeof(Name    ), "UnPass.ini" );
	GetConfigString( "Login", "Password", Password, sizeof(Password), "UnPass.ini" );
	if( !CheckPassword( Name, Password ) )
	{
		WPasswordDialog Dlg;
		if( Dlg.DoModal() )
		{
			if( CheckPassword( *Dlg.ResultName, *Dlg.ResultPassword ) )
			{
				SetConfigString( "Login", "Name",     *Dlg.ResultName,     "UnPass.ini"  );
				SetConfigString( "Login", "Password", *Dlg.ResultPassword, "UnPass.ini"  );
			}
			else
			{
				MessageBox( NULL, LocalizeError("Password","Core"), "Failure", MB_OK );
				ExitProcess( 0 );
			}
		}
		else ExitProcess( 0 );
	}*/

	// Init subsystems.
	GSceneMem.Init( 32768 );

	// First-run menu.
	UBOOL FirstRun=0;
	GetConfigBool( "FirstRun", "FirstRun", FirstRun );
	if
	(	(FirstRun || ParseParam(appCmdLine(),"FirstRun"))
	&&	!GIsEditor 
	&&	GIsClient )
	{
		// Get system directory.
		char SysDir[256]="", WinDir[256]="";
		GetSystemDirectory( SysDir, ARRAY_COUNT(SysDir) );
		GetWindowsDirectory( WinDir, ARRAY_COUNT(WinDir) );

		// Low memory detection.
		if( GPhysicalMemory <= 24*1024*1024 )
		{
			char Temp[4096];
			appSprintf( Temp, Localize("FirstRun","LowMemory"), GPhysicalMemory/1024/1024 );
			::MessageBox( NULL, Temp, Localize("FirstRun","Caption"), MB_OK|MB_ICONINFORMATION|MB_TASKMODAL );
			SetConfigBool( "Galaxy.GalaxyAudioSubsystem", "LowSoundQuality", 1 );
			SetConfigBool( "WinDrv.WindowsClient", "LowDetailTextures", 1 );
		}

		// MMX detection.
		if( !GIsMMX )
		{
			::MessageBox( NULL, Localize("FirstRun","NonMMX"), Localize("FirstRun","Caption"), MB_OK|MB_ICONINFORMATION|MB_TASKMODAL );
			SetConfigString( "Galaxy.GalaxyAudioSubsystem", "OutputRate", "11025Hz" );
		}

		// Low res detection.
		if( !GIsMMX || !GIsPentiumPro )
		{
			SetConfigString( "WinDrv.WindowsClient", "ViewportX", "320" );
			SetConfigString( "WinDrv.WindowsClient", "ViewportY", "240" );
		}

		// Autodetect and ask about detected render devices.
		static TArray<FRegistryObjectInfo> RenderDevices;
		GObj.GetRegistryObjects( RenderDevices, UClass::StaticClass, URenderDevice::StaticClass, 0 );
		for( INT i=0; i<RenderDevices.Num(); i++ )
		{
			char File1[256], File2[256];
			appSprintf( File1, "%s\\%s", SysDir, RenderDevices(i).Autodetect );
			appSprintf( File2, "%s\\%s", WinDir, RenderDevices(i).Autodetect );
			if( *RenderDevices(i).Autodetect && (appFSize(File1)>=0 || appFSize(File2)>=0) )
			{
				char Path[256], *Str;
				appStrcpy( Path, RenderDevices(i).Object );
				Str = appStrstr(Path,".");
				if( Str )
				{
					*Str++ = 0;
					if( ::MessageBox( NULL, Localize(Str,"AskInstalled",Path), Localize("FirstRun","Caption"), MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL )==IDYES )
					{
						if( ::MessageBox( NULL, Localize(Str,"AskUse",Path), Localize("FirstRun","Caption"), MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL )==IDYES )
						{
							SetConfigString( "Engine.Engine", "GameRenderDevice", RenderDevices(i).Object );
							SetConfigString( "SoftDrv.SoftwareRenderDevice", "HighDetailActors", "True" );
							break;
						}
					}
				}
			}
		}

		// Display first-run message.
		::MessageBox( NULL, Localize("FirstRun","Starting"), Localize("FirstRun","Caption"), MB_OK|MB_ICONINFORMATION|MB_TASKMODAL );
		SetConfigBool( "FirstRun", "FirstRun", 0 );
	}
	/*char CdPath[256]="", Check[256];
	GetConfigString( "Engine.Engine", "CdPath", CdPath, ARRAY_COUNT(CdPath) );
	appSprintf( Check, "%sTextures\\Palettes.utx", CdPath );
	while( !GIsEditor && appFSize(Check)<=0 )
	{
		if( MessageBox
		(
			NULL,
			"Please insert the Unreal CD-Rom into your drive and press OK to continue, or Cancel to exit.",
			"Cd Required At Startup",
			MB_TASKMODAL|MB_OKCANCEL
		)==IDCANCEL )
		{
			GIsCriticalError = 1;
			ExitProcess( 0 );
		}
	}
	*/
	// Create the global engine object.
	UClass* EngineClass;
	if( !GIsEditor )
	{
		// Create game engine.
		EngineClass = GObj.LoadClass( UGameEngine::StaticClass, NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
	}
	else if( ParseParam( appCmdLine(),"MAKE" ) )
	{
		// Create editor engine.
		EngineClass = GObj.LoadClass( UEngine::StaticClass, NULL, "ini:Engine.Engine.EditorEngine", NULL, LOAD_NoFail | LOAD_DisallowFiles | LOAD_KeepImports, NULL );
	}
	else
	{
		// Editor.
		EngineClass = GObj.LoadClass( UEngine::StaticClass, NULL, "ini:Engine.Engine.EditorEngine", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
	}

	// Init engine.
	UEngine* Engine = ConstructClassObject<UEngine>( EngineClass );
	Engine->Init();

	// Hide the log.
	if
	(	!ParseParam(appCmdLine(),"LOG")
	&&	!ParseParam(appCmdLine(),"SERVER")
	&&	!ParseParam(appCmdLine(),"EDITOR")
	&&	!ParseParam(appCmdLine(),"MAKE") )
		ShowWindow( *GLog, SW_HIDE );

	return Engine;

	unguard;
}

//
// Unreal's main message loop.  All windows in Unreal receive messages
// somewhere below this function on the stack.
//
WINDOW_API void MainLoop( UEngine* Engine )
{
	guard(MainLoop);
	if( GLog )
		GLog->SetExec( Engine );
	GIsRunning = 1;
	DWORD ThreadId = GetCurrentThreadId();
	HANDLE hThread = GetCurrentThread();
	DOUBLE OldTime = appSeconds();
	while( GIsRunning && !GIsRequestingExit )
	{
		// Update the world.
		DOUBLE NewTime = appSeconds();
		Engine->Tick( NewTime - OldTime );
		OldTime = NewTime;

		// Enforce optional maximum tick rate.
		INT MaxTickRate = Engine->GetMaxTickRate();
		if( MaxTickRate )
		{
			FLOAT Delta = (1.0/MaxTickRate) - (appSeconds()-OldTime);
			if( Delta > 0.0 )
				Sleep( Delta * 1000 );
		}

		// Handle all incoming messages.
		MSG Msg;
		GTickDue = 0;
		while( !GTickDue && PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( Msg.message == WM_QUIT )
				GIsRequestingExit = 1;
			TranslateMessage( &Msg );
			DispatchMessage( &Msg );
		}

		// If editor thread doesn't have the focus, don't suck up too much CPU time.
		if( GIsEditor )
		{
			static UBOOL HadFocus=1;
			UBOOL HasFocus = (GetWindowThreadProcessId(GetForegroundWindow(),NULL) == ThreadId );
			if( HadFocus && !HasFocus )
			{
				// Drop our priority to speed up whatever is in the foreground.
				SetThreadPriority( hThread, THREAD_PRIORITY_BELOW_NORMAL );
			}
			else if( HasFocus && !HadFocus )
			{
				// Boost our priority back to normal.
				SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
			}
			if( !HasFocus )
			{
				// Surrender the rest of this timeslice.
				Sleep(0);
			}
			HadFocus = HasFocus;
		}
	}
	GIsRunning = 0;
	if( GLog )
		GLog->SetExec( NULL );
	unguard;
}

//
// Exit the engine.
//
void ExitEngine( UEngine* Engine )
{
	guard(ExitEngine);

	GObj.Exit();
	GMem.Exit();
	GDynMem.Exit();
	GSceneMem.Exit();
	GCache.Exit(1);
	appDumpAllocs( &GTempPlatform );

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
