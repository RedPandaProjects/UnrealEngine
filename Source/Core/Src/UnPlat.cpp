/*=============================================================================
	UnPlat.cpp: Platform-specific routines-specific routines.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#pragma warning( disable : 4201 )
#include <windows.h>
#include <commctrl.h>
#include <new.h>
#include <direct.h>
#include <malloc.h>
#include <io.h>
#include <stdio.h>
#include <float.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/utime.h>

#include "Core.h"

#define ENGINE_DIR "Engine"
#define CONFIG_DIR "Config"
#define SCRIPTS_DIR "Scripts"
#define LOCALIZATION_DIR "Localization"

CORE_API FGlobalPlatform GTempPlatform;
INT GSlowTaskCount=0;
FILE* GLogFile=NULL;
char GLogFname[256]="", GReadIni[256]="", GWriteIni[256]="";

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

CORE_API DWORD hWndMain=0, hWndProgressBar=0, hWndProgressText=0, hWndCallback=0;
static int CDECL UnrealAllocationErrorHandler(size_t);

static void Recurse()
{
	guard(Recurse);
	Recurse();
	unguard;
}

/*-----------------------------------------------------------------------------
	USystem.
-----------------------------------------------------------------------------*/

void USystem::InternalClassInitializer( UClass* Class )
{
	guard(USystem::InternalClassInitializer);
	if( appStricmp(Class->GetName(),"System")==0 )
	{
		(new(Class,"PurgeCacheDays",      RF_Public)UIntProperty   (CPP_PROPERTY(PurgeCacheDays    ), "Options", CPF_Config ));
		(new(Class,"Suppress",            RF_Public)UNameProperty  (CPP_PROPERTY(Suppress          ), "Options", CPF_Config ))->ArrayDim = 16;
		(new(Class,"Paths",               RF_Public)UStringProperty(CPP_PROPERTY(Paths             ), "Options", CPF_Config, 96 ))->ArrayDim = 16;
		(new(Class,"SavePath",            RF_Public)UStringProperty(CPP_PROPERTY(SavePath          ), "Options", CPF_Config, 96 ));
		(new(Class,"CachePath",           RF_Public)UStringProperty(CPP_PROPERTY(CachePath         ), "Options", CPF_Config, 96 ));
		(new(Class,"CacheExt",            RF_Public)UStringProperty(CPP_PROPERTY(CacheExt          ), "Options", CPF_Config, 32 ));
	}
	unguard;
}
UBOOL USystem::Exec( const char* Cmd, FOutputDevice* Out )
{
	return 0;
}
IMPLEMENT_CLASS(USystem);

/*-----------------------------------------------------------------------------
	FGlobalPlatform Command line.
-----------------------------------------------------------------------------*/

UBOOL FGlobalPlatform::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(FGlobalPlatform::Exec);
	const char* Str = Cmd;

	if( ParseCommand(&Str,"MEMSTAT") )
	{
		MEMORYSTATUS B; B.dwLength = sizeof(B);
		GlobalMemoryStatus(&B);
		Out->Logf( "Memory available: Phys=%iK Pagef=%iK Virt=%iK", B.dwAvailPhys/1024, B.dwAvailPageFile/1024, B.dwAvailVirtual/1024 );
		Out->Logf( "Memory load = %i%%", B.dwMemoryLoad );
		return 1;
	}
	else if( ParseCommand(&Str,"EXIT") )
	{
		Out->Log( "Closing by request" );
		appRequestExit();
		return 1;
	}
	else if( ParseCommand(&Str,"APP") )
	{
		if( ParseCommand(&Str,"SET") )
		{
			Parse( Str, "PROGRESSBAR=",  hWndProgressBar );
			Parse( Str, "PROGRESSTEXT=", hWndProgressText );
			return 1;
		}
		else return 0;
	}
	else if( ParseCommand( &Cmd, "RELAUNCH" ) )
	{
		Out->Logf( "Relaunch: %s", Cmd );
		char ThisFile[256];
		GetModuleFileName( NULL, ThisFile, ARRAY_COUNT(ThisFile) );
		ShellExecute( NULL, "open", ThisFile, Cmd, appBaseDir(), SW_SHOWNORMAL );
		appRequestExit();
		return 1;
	}
	else if( ParseCommand( &Cmd, "DEBUG" ) )
	{
		if( ParseCommand(&Cmd,"CRASH") )
		{
			appErrorf( "%s", "Unreal crashed at your request" );
			return 1;
		}
		else if( ParseCommand( &Cmd, "GPF" ) )
		{
			Out->Log("Unreal crashing with voluntary GPF");
			*(int *)NULL = 123;
			return 1;
		}
		else if( ParseCommand( &Cmd, "RECURSE" ) )
		{
			Out->Logf( "Recursing" );
			Recurse();
			return 1;
		}
		else if( ParseCommand( &Cmd, "EATMEM" ) )
		{
			Out->Log("Eating up all available memory");
			while( 1 )
			{
				void* Eat = appMalloc(65536,"EatMem");
				memset( Eat, 0, 65536 );
			}
			return 1;
		}
		else return 0;
	}
	else return 0; // Not executed.
	unguard;
}

/*-----------------------------------------------------------------------------
	Exit.
-----------------------------------------------------------------------------*/

CORE_API void appRequestExit()
{
	guard(appRequestExit);
	debugf("appRequestExit");
	PostQuitMessage( 0 );
	GIsRequestingExit=1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Clipboard.
-----------------------------------------------------------------------------*/

CORE_API void ClipboardCopy( const char* Str )
{
	guard(ClipboardCopy);
	if( OpenClipboard(GetActiveWindow()) )
	{
		void* Data = GlobalAlloc( GMEM_DDESHARE, strlen(Str)+1 );
		strcpy( (char*)Data, Str );
		verify(EmptyClipboard());
		verify(SetClipboardData( CF_TEXT, Data ));
		verify(CloseClipboard());
	}
	unguard;
}

CORE_API void ClipboardPaste( FString& Result )
{
	guard(ClipboardPasteString);
	if( OpenClipboard(GetActiveWindow()) )
	{
		void* V=GetClipboardData( CF_TEXT );
		if( V )
			Result = (char*)V;
		else
			Result = "";
		verify(CloseClipboard());
	}
	else Result="";
	unguard;
}

/*-----------------------------------------------------------------------------
	Machine state info.
-----------------------------------------------------------------------------*/

//
// Intel CPUID.
//
static void FGlobalPlatform_CPUID( int i, DWORD *A, DWORD *B, DWORD *C, DWORD *D )
{
#if ASM
	try
	{
 		__asm
		{			
			mov eax,[i]
			_emit 0x0f
			_emit 0xa2

			mov edi,[A]
			mov [edi],eax

			mov edi,[B]
			mov [edi],ebx

			mov edi,[C]
			mov [edi],ecx

			mov edi,[D]
			mov [edi],edx

			mov eax,0
			mov ebx,0
			mov ecx,0
			mov edx,0
			mov esi,0
			mov edi,0
		}
	}
	catch(...)
	{
		debugf( NAME_Init, "CPUID failed!" );
		*A = *B = *C = *D = 0;
	}
#else
	*A=*B=*C=*D=0;
#endif
}

/*-----------------------------------------------------------------------------
	Log.
-----------------------------------------------------------------------------*/

//
// Close the log file.
//
void appCloseLog()
{
	guard(appCloseLog);
	if( GLogFile )
	{
		char Time[32], Date[32], Message[256];
		appSprintf( Message, "Log file closed, %s %s", _strdate(Date), _strtime(Time) );
		debugf( NAME_Log, Message );
		appFclose( GLogFile );
		GLogFile = NULL;
	}
	unguard;
}

//
// Open the log file.
// Not guarded.
//
void appOpenLog( const char* Fname )
{
	guard(appOpenLog);
	if( GLogFile )
		appCloseLog();
	if( Fname )
	{
		strcpy( GLogFname, Fname );
	}
	else
	{
		strcpy( GLogFname, appPackage() );
		strcat( GLogFname, ".log" );
		if( GIsEditor )
			strcpy( GLogFname, "Editor.log" );
		Parse( appCmdLine(), "LOG=", GLogFname, ARRAY_COUNT(GLogFname) );
	}
	GLogFile = appFopen( GLogFname, "w+t" );
	if( GLogFile )
	{
		setvbuf( GLogFile, 0, _IONBF, 4096 );
		char Time[32], Date[32], Message[256];
		appSprintf( Message, "Log file open, %s %s", _strdate(Date), _strtime(Time) );
		debugf( NAME_Log, Message );
	}
	else debugf( NAME_Log, "Failed to open log" );
	unguard;
}

/*-----------------------------------------------------------------------------
	FGlobalPlatform init/exit.
-----------------------------------------------------------------------------*/

void appInit()
{
	// Init.
	appOpenLog( NULL );
	strcpy( GErrorHist, "General protection fault!\r\n\r\nHistory: " );

	// Command line.
	debugf( NAME_Init, "Compiled: " __DATE__ " " __TIME__ );
	debugf( NAME_Init, "Command line: %s", appCmdLine() );
	debugf( NAME_Init, "Base directory: %s", appBaseDir() );

	// Parameters.
	GIsStrict = ParseParam( appCmdLine(), "STRICT" );

	// Write-ini.
	char Temp[256];

	if( Parse( appCmdLine(), "INI=", Temp, ARRAY_COUNT(Temp) ) )
	{
		appStrcpy( GWriteIni, Temp );
	}
	else
	{
		appStrcpy(GWriteIni, appConfigDir(false));
		appStrcat(GWriteIni, appPackage());
		appStrcat(GWriteIni, ".ini");
	}

	if (appFSize(GWriteIni) < 1)
	{
		appStrcpy(GReadIni, appConfigDir(false));
		appStrcat(GReadIni, "Default.ini");
		appCopyFile(GReadIni, GWriteIni);
	}

	FILE* F;
	F = fopen( GWriteIni, "at" );
	if( F )
		fclose( F );
	else
		GSystem->Warnf( LocalizeError("IniReadOnly"), GWriteIni );

	// Read-ini.
	appStrcpy( GReadIni, GWriteIni );
	if( Parse( appCmdLine(), "READINI=", Temp, ARRAY_COUNT(Temp) ) )
	{
		appStrcpy( GReadIni, appBaseDir() );
		appStrcat( GReadIni, Temp );
	}

	// Language.
	if( GetConfigString( "Engine.Engine", "Language", Temp, ARRAY_COUNT(Temp) ) )
		SetLanguage( Temp );

	// Computer name.
	char Comp[MAX_COMPUTERNAME_LENGTH+1]="";
	DWORD Size=MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName( Comp, &Size );
	for( char *c=Comp, *d=GComputerName; *c!=0; c++ )
		if( appIsAlnum(*c) && d<GComputerName+ARRAY_COUNT(GComputerName)-1 )
			*d++ = *c;
	*d++ = 0;
	debugf( NAME_Init, "Computer: %s", GComputerName );

	// Core initialization.
	GObj.Init();
	GMem.Init( 65536 );
	GSys = new USystem;
	GObj.AddToRoot( GSys );
	for( INT i=0; i<ARRAY_COUNT(GSys->Suppress); i++ )
		if( GSys->Suppress[i]!=NAME_None )
			GSys->Suppress[i].SetFlags( RF_Suppress );

	// Randomize.
	srand( (unsigned)time( NULL ) );

/*	// Set heap granularity.
	INT OldHeapGranularity=*p__amblksiz(), NewHeapGranularity=0x10000;
	*__p__amblksiz() = NewHeapGranularity;
	debugf( NAME_Init, "Heap granularity changed from %i to %i", OldHeapGranularity, NewHeapGranularity );*/

	// Get memory.
	MEMORYSTATUS M;
	GlobalMemoryStatus(&M);
	GPhysicalMemory=M.dwTotalPhys;
	debugf( NAME_Init, "Memory total: Phys=%iK Pagef=%iK Virt=%iK", M.dwTotalPhys/1024, M.dwTotalPageFile/1024, M.dwTotalVirtual/1024 );

	// Working set.
	DWORD WsMin=0, WsMax=0;
	GetProcessWorkingSetSize( GetCurrentProcess(), &WsMin, &WsMax );
	debugf( NAME_Init, "Working set: %X / %X", WsMin, WsMax );
	//SetProcessWorkingSetSize( GetCurrentProcess(), 32*1024*1024, 64*1024*1024 );
 
	// Check Windows version.
	OSVERSIONINFO Version; Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&Version);
	if( Version.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		debugf( NAME_Init, "Detected: Microsoft Windows NT %u.%u (Build: %u)",
			Version.dwMajorVersion,Version.dwMinorVersion,Version.dwBuildNumber);
	}
	else if( Version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
		debugf( NAME_Init, "Detected: Microsoft Windows 95 %u.%u (Build: %u)",
			Version.dwMajorVersion,Version.dwMinorVersion,Version.dwBuildNumber);
	}
	else
	{
		debugf( NAME_Init, "Detected: Windows %u.%u (Build: %u)",
			Version.dwMajorVersion,Version.dwMinorVersion,Version.dwBuildNumber);
		appErrorf( "%s", "Unreal requires Windows 95 or Windows NT" );
	}

	// CPU speed.
	LARGE_INTEGER lFreq;
	QueryPerformanceFrequency(&lFreq);
	DOUBLE Frequency = (DOUBLE)(SQWORD)(((QWORD)lFreq.LowPart) + ((QWORD)lFreq.HighPart<<32));
	check(Frequency!=0);
	INT Cycles=0;
	LARGE_INTEGER I1, I2;
	QueryPerformanceCounter(&I1);
	clock(Cycles);
	Sleep(100);
	QueryPerformanceCounter(&I2);
	unclock(Cycles);
	DOUBLE T1 = (DOUBLE)(SQWORD)((QWORD)I1.LowPart + (((QWORD)I1.HighPart)<<32)) / Frequency;
	DOUBLE T2 = (DOUBLE)(SQWORD)((QWORD)I2.LowPart + (((QWORD)I2.HighPart)<<32)) / Frequency;
	GSecondsPerCycle = (T2-T1) / (DOUBLE)Cycles;
	debugf( NAME_Init, "CPU Speed=%f MHz", 0.000001 / GSecondsPerCycle );

	// Get CPU info.
	SYSTEM_INFO SI;
	GetSystemInfo(&SI);
	GPageSize=SI.dwPageSize;
	GProcessorCount=SI.dwNumberOfProcessors;
	debugf( NAME_Init, "CPU Page size=%i, Processors=%i", SI.dwPageSize, SI.dwNumberOfProcessors );

	// Check processor version with CPUID.
	DWORD A=0, B=0, C=0, D=0;
	FGlobalPlatform_CPUID(0,&A,&B,&C,&D);
	char Brand[13]="", *Model, FeatStr[256]="";
	Brand[ 0] = B;
	Brand[ 1] = B>>8;
	Brand[ 2] = B>>16;
	Brand[ 3] = B>>24;
	Brand[ 4] = D;
	Brand[ 5] = D>>8;
	Brand[ 6] = D>>16;
	Brand[ 7] = D>>24;
	Brand[ 8] = C;
	Brand[ 9] = C>>8;
	Brand[10] = C>>16;
	Brand[11] = C>>24;
	Brand[12] = 0;
	FGlobalPlatform_CPUID( 1, &A, &B, &C, &D );
	switch( (A>>8) & 0x000f )
	{
		case 4:  Model="486-class processor";        break;
		case 5:  Model="Pentium-class processor";    break;
		case 6:  Model="PentiumPro-class processor"; break;
		case 7:  Model="P7-class processor";         break;
		default: Model="Unknown processor";          break;
	}
	if( (D & 0x00800000) && !ParseParam(appCmdLine(),"NOMMX") )
	{
		strcat( FeatStr, "MMX " );
		GIsMMX=1;
	}
	if( D & 0x00008000 )
	{
		strcat( FeatStr, "CMov " );
		GIsPentiumPro=1;
	}
	if( D & 0x00000001 ) strcat( FeatStr, "FPU " );
	if( D & 0x00000010 ) strcat( FeatStr, "TimeStamp " );

	// Check for K6 3D instructions.
	if( !ParseParam(appCmdLine(),"NOK6") )
	{
		FGlobalPlatform_CPUID( 0x80000000, &A, &B, &C, &D );
		if( A >= 0x80000001 )
		{
			FGlobalPlatform_CPUID( 0x80000001, &A, &B, &C, &D );
			if( D & 0x80000000 )
			{
				strcat( FeatStr, "AMD-3D " );
				GIsK63D=1;
			}
		}
	}

	// Print feature.
	debugf( NAME_Init, "CPU Detected: %s (%s)", Model, Brand );
	debugf( NAME_Init, "CPU Features: %s", FeatStr );

	// Keyboard layout.
	char KL[KL_NAMELENGTH];
	check(GetKeyboardLayoutName(KL)!=0);
	debugf( NAME_Init, "Keyboard layout: %s", KL );

	// FPU.
	appEnableFastMath( 0 );

	// Handle operator new allocation errors.
	_set_new_handler(UnrealAllocationErrorHandler);

	// Handle malloc allocation errors.
	_set_new_mode( 1 );	
}

//
// Set low precision mode.
//
void appEnableFastMath( UBOOL Enable )
{
	guard(appEnableFastMath);
	_controlfp( Enable ? (_PC_24) : (_PC_64), _MCW_PC );
	unguard;
}

//
// Shut down the platform-specific subsystem.
// Not guarded.
//
void appExit()
{
	debugf( NAME_Exit, "appExit" );
	appDumpAllocs( GSystem );
	appCloseLog();
}

/*-----------------------------------------------------------------------------
	FGlobalPlatform misc.
-----------------------------------------------------------------------------*/

//
// Load a library.
//
CORE_API void* appGetDllHandle( const char* Filename )
{
	guard(appGetDllHandle);
	check(Filename);

	void* Result = (void*)LoadLibrary( Filename );
	if( Result )
	{
		char Temp[256];
		strcpy( Temp, Filename );
		strcat( Temp, ".dll" );
		Result = (void*)LoadLibrary( Filename );
	}

	return Result;
	unguard;
}

//
// Free a library.
//
CORE_API void appFreeDllHandle( void* DllHandle )
{
	guard(appFreeDllHandle);
	check(DllHandle);

	FreeLibrary( (HMODULE)DllHandle );

	unguard;
}

//
// Lookup the address of a DLL function
//
CORE_API void* appGetDllExport( void* DllHandle, const char* ProcName )
{
	guard(appGetDllExport);
	check(DllHandle);
	check(ProcName);

	return (void*)GetProcAddress( (HMODULE)DllHandle, ProcName );

	unguard;
}

//
// Break the debugger.
//
void appDebugBreak()
{
	guard(appDebugBreak);
	::DebugBreak();
	unguard;
}

/*-----------------------------------------------------------------------------
	Timing.
-----------------------------------------------------------------------------*/

//
// Get time in seconds.
//
#if !DEFINED_appSeconds
CORE_API DOUBLE appSeconds()
{
	static DWORD  InitialTime = timeGetTime();
	static DOUBLE TimeCounter = 0.0;

	// Accumulate difference to prevent wraparound.
	DWORD NewTime = timeGetTime();
	TimeCounter += (NewTime - InitialTime) * (1./1000.);
	InitialTime = NewTime;

	return TimeCounter;
}
#endif

#if !DEFINED_appCycles
CORE_API DWORD appCycles()
{
	return appSeconds();
}
#endif

//
// Return the system time.
//
CORE_API void appSystemTime( INT& Year, INT& Month, INT& DayOfWeek, INT& Day, INT& Hour, INT& Min, INT& Sec, INT& MSec )
{
	guard(appSystemTime);

	SYSTEMTIME st;
	GetLocalTime (&st);

	Year		= st.wYear;
	Month		= st.wMonth;
	DayOfWeek	= st.wDayOfWeek;
	Day			= st.wDay;
	Hour		= st.wHour;
	Min			= st.wMinute;
	Sec			= st.wSecond;
	MSec		= st.wMilliseconds;

	unguard;
}

/*-----------------------------------------------------------------------------
	Link functions.
-----------------------------------------------------------------------------*/

//
// Launch a uniform resource locator (i.e. http://www.epicgames.com/unreal).
// This is expected to return immediately as the URL is launched by another
// task.
//
void appLaunchURL( const char* URL, const char* Parms, char* Error256 )
{
	guard(appLaunchURL);
	Error256=0;
	debugf( NAME_Log, "LaunchURL %s", URL );
	INT Code = (INT)ShellExecute(NULL,"open",URL,Parms,"",SW_SHOWNORMAL);
	if( Code<=32 )
		appSprintf( Error256, LocalizeError("UrlFailed") );
	unguard;
}

/*-----------------------------------------------------------------------------
	FGlobalPlatform file finding.
-----------------------------------------------------------------------------*/

//
// Find a file.
//
UBOOL appFindPackageFile( const char* In, const FGuid* Guid, char* Out )
{
	guard(appFindPackageFile);

	// Don't return it if it's a library.
	if( strlen(In)>4 && stricmp( In + strlen(In)-4, ".DLL" )==0 )
		return 0;

	// Try file as specified.
	strcpy( Out, In );
	if( appFSize( Out ) >= 0 )
		return 1;

	// Try all of the predefined paths.
	for( DWORD i=0; i<ARRAY_COUNT(GSys->Paths)+(Guid!=NULL); i++ )
	{
		// Get directory only.
		char Temp[256];
		char* Ext;

		UBOOL Found = 0;


	
		{
			strcpy(Temp, appScriptDir());
			strcpy(Out, Temp);
			strcat(Out, In);
			Found = (appFSize(Out) >= 0);
			if (Found)
			{
				return 1;
			}
			else
			{
				strcpy(Temp, appScriptDir());
				strcpy(Out, Temp);
				strcat(Out, In);
				strcat(Out, ".u");
				Found = (appFSize(Out) >= 0);
				if (Found)
				{
					return 1;
				}
			}
		}


		if( i<ARRAY_COUNT(GSys->Paths) )
		{
			if( *GSys->Paths[i]==0 )
				continue;
	
			strcpy(Temp, appContentDir(true));
			strcat( Temp, GSys->Paths[i] );
			Ext = appStrstr(Temp,"*");
			if( Ext )
				*Ext++ = 0;
			strcpy( Out, Temp );
			strcat( Out, In );

			Found = (appFSize(Out) >= 0);
			if (!Found && Ext)
			{
				strcat(Out, Ext);
				Found = (appFSize(Out) >= 0);
			}
			if (!Found)
			{
				strcpy(Temp, appContentDir(false));
				strcat(Temp, GSys->Paths[i]);
				Ext = appStrstr(Temp, "*");
				if (Ext)
					*Ext++ = 0;
				strcpy(Out, Temp);
				strcat(Out, In);
			}
		}
		else
		{
			strcpy( Temp, GSys->CachePath );
			strcat( Temp, "\\" );
			Ext = GSys->CacheExt;
			strcpy( Out, Temp );
			strcat( Out, Guid->String(Temp) );
		}

		// Check for file.
		if(!Found)
			Found = (appFSize(Out) >= 0);
		if( !Found && Ext )
		{
			strcat( Out, Ext );
			Found = (appFSize( Out )>=0);
		}
		if( Found )
		{
			if( i==ARRAY_COUNT(GSys->Paths) )
			{
				// Update cache access time.
				_utime( Out, NULL );
			}
			return 1;
		}
	}

	// Not found.
	return 0;
	unguard;
}

//
// Clean out the file cache.
//
CORE_API void appCleanFileCache()
{
	guard(appCleanFileCache);
	char Temp[256];

	// Delete all temporary files.
	appSprintf( Temp, "%s\\*.tmp", GSys->CachePath );
	TArray<FString> Found = appFindFiles( Temp );
	for( INT i=0; i<Found.Num(); i++ )
	{
		appSprintf( Temp, "%s\\%s", GSys->CachePath, *Found(i) );
		debugf( "Deleting temporary file: %s", Temp );
		unlink( Temp );
	}

	// Delete cache files that are no longer wanted.
	appSprintf( Temp, "%s\\*%s", GSys->CachePath, GSys->CacheExt );
	Found = appFindFiles( Temp );
	if( GSys->PurgeCacheDays )
	{
		for( INT i=0; i<Found.Num(); i++ )
		{
			struct _stat Buf;
			appSprintf( Temp, "%s\\%s", GSys->CachePath, *Found(i) );
			if( _stat(Temp,&Buf)==0 )
			{
				time_t CurrentTime, FileTime;
				FileTime = Buf.st_mtime;
				time( &CurrentTime );
				DOUBLE DiffSeconds = difftime( CurrentTime, FileTime );
				INT    DiffDays    = DiffSeconds / 60.0 / 60.0 / 24.0;
				if( DiffDays > GSys->PurgeCacheDays )
				{
					debugf( "Purging outdated file from cache: %s (%i days old)", Temp, DiffDays );
					unlink( Temp );
				}
			}
		}
	}
	unguard;
}

//
// Create a temporary file.
//
CORE_API void appCreateTempFilename( const char* Path, char* Result256 )
{
	guard(appCreateTempFilename);
	int i=rand();
	do
		appSprintf( Result256, "%s%04X.tmp", Path, i );
	while( appFSize(Result256)>0 );
	unguard;
}

//
// Move a file and overwrite the destination.
//
CORE_API UBOOL appMoveFile( const char* Src, const char* Dest )
{
	guard(appMoveFile);

	//warning: MoveFileEx is broken on Windows 95 (Microsoft bug).
	unlink( Dest );
	UBOOL Success = MoveFile( Src, Dest )!=0;
	if( !Success )
		debugf( NAME_Warning, "Error moving file '%s' to '%s'", Src, Dest );

	return Success;
	unguard;
}

//
// Copy a file and overwrite the destination.
//
CORE_API UBOOL appCopyFile( const char* Src, const char* Dest )
{
	guard(appCopyFile);

	//warning: MoveFileEx is broken on Windows 95 (Microsoft bug).
	UBOOL Success = CopyFile( Src, Dest, 0 )!=0;
	if( !Success )
		debugf( NAME_Warning, "Error copying file '%s' to '%s'", Src, Dest );

	return Success;
	unguard;
}

/*-----------------------------------------------------------------------------
	FGlobalPlatform Log routines.
-----------------------------------------------------------------------------*/

//
// Print a message on the debugging log.
//
// This code is unguarded because trapped errors will just try
// to log more errors, resulting in a recursive mess.
//
void FGlobalPlatform::WriteBinary( const void* Data, INT Length, EName Event )
{
	try
	{
		FName EventName = FName(Event);
		if( !(EventName.GetFlags() & RF_Suppress) )
		{
			INT FoundIndex=0;
#ifdef _DEBUG
			OutputDebugString( *EventName );
			OutputDebugString( ": " );
			OutputDebugString( (char*)Data );
			OutputDebugString( "\n" );
#endif
			if( GLogFile )
			{
				fwrite( *EventName, strlen(*EventName), 1, GLogFile );
				fwrite( ": ", 2, 1, GLogFile );
				fwrite( Data, strlen((char*)Data), 1, GLogFile );
				fwrite( "\n", 1, 1, GLogFile );
			}
			if( GLogHook )
			{
				GLogHook->WriteBinary( Data, Length, Event );
			}
		}
	}
	catch( ... )
	{
		// Ignore errors here to avoid infinite recursive error reporting.
	}
}

/*-----------------------------------------------------------------------------
	FGlobalPlatform windows specific routines.
-----------------------------------------------------------------------------*/

//
// Display a warning.
//
void VARARGS FGlobalPlatform::Warnf( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);

	guard(Warnf);
	debugf( NAME_Warning, "%s", TempStr );
	::MessageBox( NULL, TempStr, LocalizeError("Warning"), MB_OK|MB_TASKMODAL );
	unguard;
}

//
// Display a yes-no question.
//
UBOOL VARARGS FGlobalPlatform::YesNof( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);

	guard(YesNof);
	return( ::MessageBox( NULL, TempStr, LocalizeError("Question"), MB_YESNO|MB_TASKMODAL ) == IDYES);
	unguard;
}

/*-----------------------------------------------------------------------------
	FGlobalPlatform error handling.
-----------------------------------------------------------------------------*/

//
// Allocation error handler.
//
static int CDECL UnrealAllocationErrorHandler( size_t )
{
	appErrorf( LocalizeError("OutOfMemory") );
	return 0;
}

//
// Immediate exit.
//
CORE_API void appForceExit()
{
	ExitProcess( 1 );
}

//
// Handle a critical error.
//warning: May be called at library startup time.
//
void appError( const char* Msg )
{
#ifdef _DEBUG
	if( GIsStarted )
	{
  		debugf( NAME_Critical, "appError called while debugging:" );
		debugf( NAME_Critical, Msg );
		GObj.ShutdownAfterError();
  		debugf( NAME_Critical, "Breaking debugger" );
	}
	DebugBreak(); 
#else
	if( GIsCriticalError )
	{
		debugf( NAME_Critical, "Error reentered: %s", Msg );
	}
	else
	{
		GIsCriticalError = 1;
		debugf( NAME_Critical, "appError called:" );
		debugf( NAME_Critical, Msg );
		GObj.ShutdownAfterError();
		strncpy( GErrorHist, Msg, ARRAY_COUNT(GErrorHist) );
		strncat( GErrorHist, "\r\n\r\n", ARRAY_COUNT(GErrorHist) );
		strncat( GErrorHist, LocalizeError("History"), ARRAY_COUNT(GErrorHist) );
		strncat( GErrorHist, ": ", ARRAY_COUNT(GErrorHist) );
	}
	throw( 1 );
#endif
}

//
// Global error handler with a formatted message.
//warning: May be called at library startup time.
//
void VARARGS appErrorf( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);
	appError( TempStr );
}

CORE_API void VARARGS appUnwindf( const char* Fmt, ... )
{
	GIsCriticalError = 1;

	char TempStr[4096];
	GET_VARARGS( TempStr, Fmt );

	static INT Count=0;
	if( Count++ )
		strncat( GErrorHist, " <- ", ARRAY_COUNT(GErrorHist) );
	strncat( GErrorHist, TempStr, ARRAY_COUNT(GErrorHist) );

	debugf( NAME_Critical, TempStr );
}

/*-----------------------------------------------------------------------------
	Slow task and progress bar functions.
-----------------------------------------------------------------------------*/

//
// Begin a slow task, optionally bringing up a progress bar.  Nested calls may be made
// to this function, and the dialog will only go away after the last slow task ends.
//
void FGlobalPlatform::BeginSlowTask( const char* Task, UBOOL StatusWindow, UBOOL Cancelable )
{
	guard(FGlobalPlatform::BeginSlowTask);
	if( hWndProgressBar && hWndProgressText )
	{
		SendMessage( (HWND)hWndProgressBar, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, 100) );
		SendMessage( (HWND)hWndProgressText, WM_SETTEXT, (WPARAM)0, (LPARAM)Task );
		UpdateWindow( (HWND)hWndProgressText );
	}
	GIsSlowTask = ++GSlowTaskCount>0;
	unguard;
}

//
// End the slow task.
//
void FGlobalPlatform::EndSlowTask()
{
	guard(FGlobalPlatform::EndSlowTask);
	check(GSlowTaskCount>0);
	GIsSlowTask = --GSlowTaskCount>0;
	unguard;
}

//
// Update the progress bar.
//
UBOOL VARARGS FGlobalPlatform::StatusUpdatef( INT Numerator, INT Denominator, const char* Fmt, ... )
{
	guard(FGlobalPlatform::StatusUpdatef);

	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);

	if( GIsSlowTask && hWndProgressBar && hWndProgressText )
	{
		SendMessage( (HWND)hWndProgressText, WM_SETTEXT, (WPARAM)0, (LPARAM)TempStr );
		SendMessage( (HWND)hWndProgressBar, PBM_SETPOS, (WPARAM)(Denominator ? 100*Numerator/Denominator : 0), (LPARAM)0 );
	}

	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Profile functions.
-----------------------------------------------------------------------------*/

//
// Read a string from the profile.
//
UBOOL GetConfigString
(
	const char*	Section,
	const char*	Key,
    char*		Value,
	INT			Size,
	const char*	Filename
)
{
	guard(GetConfigString);
    *Value = 0;
    if( !Filename )
        Filename = GReadIni;
    return GetPrivateProfileString( Section, Key, "", Value, Size, Filename)>0 && Value[0];
	unguard;
}

CORE_API const char* GetConfigStr( const char* Section, const char* Key, const char* Filename )
{
	guard(GetConfigStr);
	static char Result[256]="";
	GetConfigString( Section, Key, Result, ARRAY_COUNT(Result), Filename );
	return Result;
	unguard;
}

//
// Read an array of strings from the profile.
//
UBOOL GetConfigStringArray
(
	const char*			Section,
	const char*			Key,
    TArray<FString>&	Value,
	const char*			Filename
)
{
	guard(GetConfigStringArray);
	char NewKey[256], NewValue[256];
	Value.Empty();
	for( INT i=0; i<32; i++ )
	{
		appSprintf( NewKey, "%s%i", Key, i );
		if( GetConfigString( Section, NewKey, NewValue, ARRAY_COUNT(NewValue), Filename ) )
			new( Value )FString( NewValue );
		else
			break;
	}
	return Value.Num()>0;
	unguard;
}

//
// Get an integer from the profile.
//
UBOOL GetConfigInt
(
	const char*	Section,
	const char*	Key,
	INT&		Value,
	const char*	Filename
)
{
	guard(GetConfigInt);
    char Text[80]; 
    if( GetConfigString( Section, Key, Text, sizeof(Text), Filename ) )
    {
		Value = appAtoi(Text);
		return 1;
    }
    return 0;
	unguard;
}

//
// Get a floating point number from the profile.
//
UBOOL GetConfigFloat
(
	const char*	Section,
	const char* Key,
	FLOAT&		Value,
	const char* Filename
)
{
	guard(GetConfigFloat);
    char Text[80]; 
    if( GetConfigString( Section, Key, Text, sizeof(Text), Filename ) )
    {
		Value = appAtof(Text);
		return 1;
    }
    return 0;
	unguard;
}

//
// Get a boolean from the profile.
//
UBOOL GetConfigBool
(
    const char*	Section,
	const char*	Key,
	UBOOL&		Value,
	const char* Filename
)
{
	guard(GetConfigBool);
    char Text[80]; 
    if( GetConfigString( Section, Key, Text, sizeof(Text), Filename ) )
    {
        if( stricmp(Text,"true")==0 )
            Value = 1;
		else
			Value = appAtoi(Text)==1;
		return 1;
    }
    return 0;
	unguard;
}

//
// Get an entire section of key-value pairs.
// Places a list of contiguous, null-terminated strings in Result, followed by a second null character.
//
UBOOL GetConfigSection
(
	const char*		Section,
	char*			Result,
	INT				Size,
	const char*		Filename
)
{
	guard(GetConfigSection);
    if( !Filename )
        Filename = GReadIni;
	GetPrivateProfileSection( Section, Result, Size, Filename );
	return *Result != NULL;
	unguard;
}

//
// Write a string to the profile.
//
void SetConfigString
(
    const char* Section,
    const char* Key,
    const char* Value,
    const char* Filename
)
{
	guard(SetConfigString);
    if( Filename == 0 )
        Filename = GWriteIni;
    WritePrivateProfileString( Section, Key, Value, Filename );
	unguard;
}	

//
// Write an integer to the profile.
//
void SetConfigInt
(
    const char* Section,
	const char* Key,
	INT			Value,
	const char* Filename
)
{
	guard(SetConfigInt);
    char Text[30];
    appSprintf( Text, "%i", Value );
    SetConfigString( Section, Key, Text, Filename );
	unguard;
}

//
// Write a float to the profile.
//
void SetConfigFloat
(
    const char*	Section,
	const char*	Key,
	FLOAT		Value,
	const char* Filename
)
{
	guard(SetConfigFloat);
    char Text[30];
    appSprintf( Text, "%f", Value );
    SetConfigString( Section, Key, Text, Filename );
	unguard;
}

//
// Write a boolean to the profile.
//
void SetConfigBool
(
	const char* Section,
	const char* Key,
	UBOOL		Value,
	const char* Filename
)
{
	guard(SetConfigBool);
    SetConfigString( Section, Key, Value ? "True" : "False", Filename );
	unguard;
}

/*-----------------------------------------------------------------------------
	Guids.
-----------------------------------------------------------------------------*/

//
// Create a new globally unique identifier.
//
CORE_API FGuid appCreateGuid()
{
	guard(appCreateGuid);

	FGuid Result;
	check( CoCreateGuid( (GUID*)&Result )==S_OK );
	return Result;

	unguard;
}

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

// Get command line.
CORE_API const char* appCmdLine()
{
	guard(appCmdLine);
	static char* CmdLine=NULL;
	if( !CmdLine )
	{
		CmdLine = GetCommandLine();
		if( *CmdLine=='\"' )
		{
			do CmdLine++;
			while( *CmdLine && *CmdLine!='\"' );
			CmdLine++;
		}
		else
		{
			while( *CmdLine && *CmdLine!=' ' )
				CmdLine++;
		}
		while( *CmdLine==' ' )
			CmdLine++;
	}
	return CmdLine;
	unguard;
}

// Get startup directory.
CORE_API const char* appBaseDir()
{
	static char BaseDir[256]="";
	if( !BaseDir[0] )
	{
		// Get directory this executable was launched from.
		GetModuleFileName( hInstance, BaseDir, ARRAY_COUNT(BaseDir) );
		for( INT i=strlen(BaseDir)-1; i>0; i-- )
			if( BaseDir[i-1]=='\\' || BaseDir[i-1]=='/' )
				break;
		BaseDir[i]=0;
		for (INT i = strlen(BaseDir) - 1; i > 0; i--)
			if (BaseDir[i - 1] == '\\' || BaseDir[i - 1] == '/')
				break;
		BaseDir[i] = 0;
		for (INT i = strlen(BaseDir) - 1; i > 0; i--)
			if (BaseDir[i - 1] == '\\' || BaseDir[i - 1] == '/')
				break;
		BaseDir[i] = 0;
		for (INT i = strlen(BaseDir) - 1; i > 0; i--)
			if (BaseDir[i - 1] == '\\' || BaseDir[i - 1] == '/')
				break;
		BaseDir[i] = 0;
	}
	return BaseDir;
}

// Get launch package base name.
CORE_API const char* appPackage()
{
	static char AppPackage[256]="";
	if( !AppPackage[0] )
	{
		char Tmp[256], *End=Tmp;
		GetModuleFileName( NULL, Tmp, ARRAY_COUNT(Tmp) );
		while( appStrchr(End,'\\') )
			End = appStrchr(End,'\\')+1;
		while( appStrchr(End,'/') )
			End = appStrchr(End,'/')+1;
		if( appStrchr(End,'.') )
			*appStrchr(End,'.') = 0;
		if( appStricmp(End,"UnrealEd")==0 )
			End="Unreal";
		appStrcpy( AppPackage, End );
	}
	return AppPackage;
}

CORE_API const char* appContentDir(bool Game)
{
	static char Path[256];

	appStrcpy(Path, appBaseDir());

	if (Game)
	{
		appStrcat(Path,appPackage());
	}
	else
	{
		appStrcat(Path, ENGINE_DIR);
	}
	appStrcat(Path, "\\");
	return Path;
}

CORE_API const char* appConfigDir(bool Game)
{
	static char Path[256];
	appStrcpy(Path, appContentDir(Game));
	appStrcat(Path, CONFIG_DIR);
	appStrcat(Path, "\\");
	return Path;
}

CORE_API const char* appLocalizationDir(bool Game)
{
	static char Path[256];
	appStrcpy(Path, appContentDir(Game));
	appStrcat(Path, LOCALIZATION_DIR);
	appStrcat(Path, "\\");
	return Path;
}

CORE_API const char* appScriptDir()
{
	static char Path[256];
	appStrcpy(Path, appContentDir(true));
	appStrcat(Path, SCRIPTS_DIR);
	appStrcat(Path, "\\");
	return Path;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
