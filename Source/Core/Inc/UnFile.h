/*=============================================================================
	UnFile.h: General-purpose file utilities.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#include <stdarg.h>
/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

// Global variables.
CORE_API extern DWORD GCRCTable[];

/*----------------------------------------------------------------------------
	Byte order conversion.
----------------------------------------------------------------------------*/

// Bitfields.
#if __INTEL__
	#define NEXT_BITFIELD(b) ((b)<<1)
	#define FIRST_BITFIELD   (1)
#else
	#define NEXT_BITFIELD(b) ((b)>>1)
	#define FIRST_BITFIELD   (0x80000000)
#endif

/*-----------------------------------------------------------------------------
	Global init and exit.
-----------------------------------------------------------------------------*/

CORE_API void appInit();
CORE_API void appExit();

/*-----------------------------------------------------------------------------
	Logging and critical errors.
-----------------------------------------------------------------------------*/

CORE_API void appRequestExit();
CORE_API void appForceExit();

CORE_API void VARARGS appErrorf( const char* Fmt, ... );
CORE_API void VARARGS appUnwindf( const char* Fmt, ... );

/*-----------------------------------------------------------------------------
	Misc.
-----------------------------------------------------------------------------*/

CORE_API void* appGetDllHandle( const char* DllName );
CORE_API void appFreeDllHandle( void* DllHandle );
CORE_API void* appGetDllExport( void* DllHandle, const char* ExportName );
CORE_API void appLaunchURL( const char* URL, const char* Parms, char* Error256=NULL );
CORE_API void appEnableFastMath( UBOOL Enable );
CORE_API class FGuid appCreateGuid();
CORE_API void appCreateTempFilename( const char* Path, char* Result256 );
CORE_API UBOOL appMoveFile( const char* Src, const char* Dest );
CORE_API UBOOL appCopyFile( const char* Src, const char* Dest );
CORE_API void appCleanFileCache();
CORE_API UBOOL appFindPackageFile( const char* In, const FGuid* Guid, char* Out );

/*-----------------------------------------------------------------------------
	Config.
-----------------------------------------------------------------------------*/

CORE_API const char* ConfigFilename();
CORE_API UBOOL GetConfigBool( const char* Section, const char* Key, UBOOL& Value, const char* FileName=NULL );
CORE_API UBOOL GetConfigInt( const char* Section, const char* Key, INT& Value, const char* FileName=NULL );
CORE_API UBOOL GetConfigFloat( const char* Section, const char* Key, FLOAT& Value, const char* FileName=NULL );
CORE_API UBOOL GetConfigString( const char* Section, const char* Key, char* Value, INT Size, const char* FileName=NULL );
CORE_API const char* GetConfigStr( const char* Section, const char* Key, const char* FileName=NULL );
CORE_API UBOOL GetConfigSection( const char* Section, char* Value, INT Size, const char* FileName=NULL );
CORE_API void SetConfigBool( const char* Section, const char* Key, UBOOL Value, const char* FileName=NULL );
CORE_API void SetConfigInt( const char* Section, const char* Key, INT Value, const char* FileName=NULL );
CORE_API void SetConfigFloat( const char* Section, const char* Key, const FLOAT Value, const char* FileName=NULL );
CORE_API void SetConfigString( const char* Section, const char* Key, const char* Value, const char* FileName=NULL );

/*-----------------------------------------------------------------------------
	Clipboard.
-----------------------------------------------------------------------------*/

CORE_API void ClipboardCopy( const char* Str );
CORE_API void ClipboardPaste( class FString& Str );

/*-----------------------------------------------------------------------------
	Convenience macros.
-----------------------------------------------------------------------------*/

#define debugf GSystem->Logf
#define warnf GSystem->Warnf
#if DO_SLOW_GUARD
	#define debugfSlow GSystem->Logf
#else
	#define debugfSlow //
#endif

/*-----------------------------------------------------------------------------
	Guard macros for call stack display.
-----------------------------------------------------------------------------*/

//
// guard/unguardf/unguard macros.
// For showing calling stack when errors occur in major functions.
// Meant to be enabled in release builds.
//
#if defined(_DEBUG) || !DO_GUARD
	#define guard(func)			{static const char __FUNC_NAME__[]=#func;
	#define unguard				}
	#define unguardf(msg)		}
#else
	#define guard(func)			{static const char __FUNC_NAME__[]=#func; try{
	#define unguard				}catch(char*Err){throw Err;}catch(...){appUnwindf("%s",__FUNC_NAME__); throw;}}
	#define unguardf(msg)		}catch(char*Err){throw Err;}catch(...){appUnwindf("%s",__FUNC_NAME__); appUnwindf msg; throw;}}
#endif

//
// guardSlow/unguardfSlow/unguardSlow macros.
// For showing calling stack when errors occur in performance-critical functions.
// Meant to be disabled in release builds.
//
#if defined(_DEBUG) || !DO_GUARD || !DO_SLOW_GUARD
	#define guardSlow(func)		{static const char __FUNC_NAME__[]=#func;
	#define unguardfSlow(msg)	}
	#define unguardSlow			}
	#define unguardfSlow(msg)	}
#else
	#define guardSlow(func)		guard(func)
	#define unguardSlow			unguard
	#define unguardfSlow(msg)	unguardf(msg)
#endif

//
// For throwing string-exceptions which safely propagate through guard/unguard.
//
CORE_API void VARARGS appThrowf( const char* Fmt, ... );

/*-----------------------------------------------------------------------------
	Check macros for assertions.
-----------------------------------------------------------------------------*/

//
// "check" expressions are only evaluated if enabled.
// "verify" expressions are always evaluated, but only cause an error if enabled.
//
#if !CHECK_NONE
	#define check(expr)  {if(!(expr)) appErrorf( LocalizeError("Assert","Core"), #expr, __FILE__, __LINE__ );}
	#define verify(expr) {if(!(expr)) appErrorf( LocalizeError("Assert","Core"), #expr, __FILE__, __LINE__ );}
#else
	#define check(expr)
	#define verify(expr) expr
#endif

//
// Check for development only.
//
#if DO_SLOW_GUARD
	#define checkslow(expr)  {if(!(expr)) appErrorf( LocalizeError("Assert","Core"), #expr, __FILE__, __LINE__ );}
	#define verifyslow(expr) {if(!(expr)) appErrorf( LocalizeError("Assert","Core"), #expr, __FILE__, __LINE__ );}
#else
	#define checkslow(expr)
	#define verifyslow(expr) expr
#endif

//
// Debug macro - only active if debugging or CHECK_ALL.
//
#if defined(_DEBUG) || CHECK_ALL
    #define debug(expr) {if(!(expr)) appErrorf( LocalizeError("Debug","Core"), #expr, __FILE__, __LINE__ );}
#else
    #define debug(expr)
#endif

/*-----------------------------------------------------------------------------
	Timing macros.
-----------------------------------------------------------------------------*/

//
// Normal timing.
//
#define clock(Timer)   {Timer -= appCycles();}
#define unclock(Timer) {Timer += appCycles()-34;}

//
// Performance critical timing.
//
#if DO_SLOW_CLOCK
	#define clockSlow(Timer)   {Timer-=appCycles();}
	#define unclockSlow(Timer) {Timer+=appCycles();}
#else
	#define clockSlow(Timer)
	#define unclockSlow(Timer)
#endif

/*-----------------------------------------------------------------------------
	Localization.
-----------------------------------------------------------------------------*/

CORE_API const char* Localize( const char* Section, const char* Key, const char* Package=GPackage, const char* LangExt=NULL );
CORE_API const char* LocalizeError( const char* Key, const char* Package=GPackage, const char* LangExt=NULL );
CORE_API const char* LocalizeProgress( const char* Key, const char* Package=GPackage, const char* LangExt=NULL );
CORE_API const char* LocalizeQuery( const char* Key, const char* Package=GPackage, const char* LangExt=NULL );
CORE_API const char* LocalizeGeneral( const char* Key, const char* Package=GPackage, const char* LangExt=NULL );
CORE_API const char* GetLanguage();
CORE_API void SetLanguage( const char* LanguageExt );

/*-----------------------------------------------------------------------------
	File functions.
-----------------------------------------------------------------------------*/

CORE_API FILE* appFopen( const char* Filename, const char* Mode );
CORE_API INT appFclose( FILE* Stream );
CORE_API INT appFseek( FILE* Stream, INT Offset, INT Origin );
CORE_API INT appFtell( FILE* Stream );
CORE_API INT appFwrite( const void* Buffer, INT Size, INT Count, FILE* Stream );
CORE_API INT appUnlink( const char* Filename );
CORE_API INT appFread( void* Buffer, INT Size, INT Count, FILE* Stream );
CORE_API INT appFSize( const char* Filename );
CORE_API const char* appFExt( const char* Filename );
CORE_API INT appMkdir( const char* Dirname );
CORE_API char* appGetcwd( char* Buffer, INT MaxLen );
CORE_API INT appChdir( const char* Dirname );
CORE_API INT appFprintf( FILE* F, const char* Fmt, ... );
CORE_API INT appFerror( FILE* F );

/*-----------------------------------------------------------------------------
	OS functions.
-----------------------------------------------------------------------------*/

CORE_API const char* appCmdLine();
CORE_API const char* appBaseDir();
CORE_API const char* appPackage();
CORE_API const char* appContentDir(bool Game);
CORE_API const char* appConfigDir(bool Game);
CORE_API const char* appLocalizationDir(bool Game);
CORE_API const char* appScriptDir();
/*-----------------------------------------------------------------------------
	Timing functions.
-----------------------------------------------------------------------------*/

#if !DEFINED_appCycles
CORE_API DWORD appCycles();
#endif

#if !DEFINED_appSeconds
CORE_API DOUBLE appSeconds();
#endif

CORE_API void appSystemTime( INT& Year, INT& Month, INT& DayOfWeek, INT& Day, INT& Hour, INT& Min, INT& Sec, INT& MSec );

/*-----------------------------------------------------------------------------
	Character type functions.
-----------------------------------------------------------------------------*/

inline char appToUpper( char c )
{
	return (c<'a' || c>'z') ? (c) : (c+'A'-'a');
}
inline char appToLower( char c )
{
	return (c<'A' || c>'Z') ? (c) : (c+'a'-'A');
}
inline UBOOL appIsAlpha( char c )
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z');
}
inline UBOOL appIsDigit( char c )
{
	return c>='0' && c<='9';
}
inline UBOOL appIsAlnum( char c )
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9');
}
inline UBOOL appIsLeadNameChar( char c )
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c=='_');
}
inline UBOOL appIsNameChar( char c )
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || (c=='_');
}

/*-----------------------------------------------------------------------------
	String functions.
-----------------------------------------------------------------------------*/

CORE_API char* appStrcpy( char* Dest, const char* Src );
CORE_API INT appStrcpy( const char* String );
CORE_API INT appStrlen( const char* String );
CORE_API char* appStrstr( const char* String, const char* Find );
CORE_API char* appStrchr( const char* String, int c );
CORE_API char* appStrcat( char* Dest, const char* Src );
CORE_API INT appStrcmp( const char* String1, const char* String2 );
CORE_API INT appStricmp( const char* String1, const char* String2 );

CORE_API void* appLargeMemset( void* Dest, int C, INT Count );
CORE_API void* appLargeMemcpy( void* Dest, const void* Src, INT Count );
CORE_API void* appMemmove( void* Dest, const void* Src, INT Count );
CORE_API void  appMemset( void* Dest, int C, INT Count );
CORE_API void* appMemcpy( void* Dest, const void* Src, INT Count );

CORE_API INT   appMemcmp( const void* Buf1, const void* Buf2, INT Count );
CORE_API const char* appSpc( int Num );
CORE_API char* appStrncpy( char* Dest, const char* Src, int Max);
CORE_API char* appStrncat( char* Dest, const char* Src, int Max);
CORE_API char* appStrupr( char* String );
CORE_API const char* appStrfind(const char* Str, const char* Find);
CORE_API UBOOL appMemIsZero( const void* V, int Count );
CORE_API unsigned long appMemCrc( const unsigned char* Data, int Length );
CORE_API INT appAtoi( const char* Str );
CORE_API FLOAT appAtof( const char* Str );
CORE_API INT appStrtoi( const char* Start, char** End, INT Base );
CORE_API void appQsort( void* Base, INT num, INT width, int(CDECL *compare)(const void* A, const void* B ) );
CORE_API INT appStrnicmp( const char* A, const char* B, INT Count );
CORE_API INT appSprintf( char* Dest, const char* Fmt, ... );
CORE_API INT appGetVarArgs( char* Dest,const char*Fmt, va_list va);
CORE_API void appMemswap( void* Ptr1, void* Ptr2, DWORD Size );

//
// Case insensitive string hash function.
//
inline unsigned long appStrihash( const char *Data )
{
	unsigned long Hash = 0;
	while( *Data )
		Hash = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ appToUpper(*Data++)) & 0x000000FF];
	return Hash;
}

/*-----------------------------------------------------------------------------
	Parsing functions.
-----------------------------------------------------------------------------*/

CORE_API UBOOL ParseCommand( const char** Stream, const char* Match );
CORE_API UBOOL Parse( const char* Stream, const char* Match, class FName& Name );
CORE_API UBOOL Parse( const char* Stream, const char* Match, DWORD& Value );
CORE_API UBOOL Parse( const char* Stream, const char* Match, class FGuid& Guid );
CORE_API UBOOL Parse( const char* Stream, const char* Match, char* Value, INT MaxLen );
CORE_API UBOOL Parse( const char* Stream, const char* Match, BYTE& Value );
CORE_API UBOOL Parse( const char* Stream, const char* Match, CHAR& Value );
CORE_API UBOOL Parse( const char* Stream, const char* Match, _WORD& Value );
CORE_API UBOOL Parse( const char* Stream, const char* Match, SWORD& Value );
CORE_API UBOOL Parse( const char* Stream, const char* Match, FLOAT& Value );
CORE_API UBOOL Parse( const char* Stream, const char* Match, INT& Value );
CORE_API UBOOL ParseUBOOL( const char* Stream, const char* Match, UBOOL& OnOff );
CORE_API UBOOL ParseObject( const char* Stream, const char* Match, class UClass* Type, class UObject*& DestRes, class UObject* InParent );
CORE_API UBOOL ParseLine( const char** Stream, char* Result, INT MaxLen, UBOOL Exact=0 );
CORE_API UBOOL ParseToken( const char*& Str, char* Result, INT MaxLen, UBOOL UseEscape );
CORE_API void ParseNext( const char** Stream );
CORE_API UBOOL ParseParam( const char* Stream, const char* Param );

/*-----------------------------------------------------------------------------
	Math functions.
-----------------------------------------------------------------------------*/

CORE_API DOUBLE appExp( DOUBLE Value );
CORE_API DOUBLE appLoge( DOUBLE Value );
CORE_API DOUBLE appFmod( DOUBLE A, DOUBLE B );
CORE_API DOUBLE appSin( DOUBLE Value );
CORE_API DOUBLE appCos( DOUBLE Value );
CORE_API DOUBLE appTan( DOUBLE Value );
CORE_API DOUBLE appAtan( DOUBLE Value );
CORE_API DOUBLE appAtan2( DOUBLE Y, FLOAT X );
CORE_API DOUBLE appSqrt( DOUBLE Value );
CORE_API DOUBLE appPow( DOUBLE A, DOUBLE B );
CORE_API UBOOL appIsNan( DOUBLE Value );
CORE_API INT appRand();
CORE_API FLOAT appFrand();

#if !DEFINED_appRound
CORE_API INT appRound( FLOAT Value );
#endif

#if !DEFINED_appFloor
CORE_API INT appFloor( FLOAT Value );
#endif

#if !DEFINED_appCeil
CORE_API INT appCeil( FLOAT Value );
#endif

/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/

//
// C style memory allocation.
//
CORE_API void* appMalloc( INT Count, const char* Tag );
CORE_API void appFree( void* Original );
CORE_API void* appRealloc( void* Original, INT Count, const char* Tag );
CORE_API void appDumpAllocs( class FOutputDevice* Out );

//
// C++ style memory allocation.
//
inline void* operator new( size_t Size, const char* Tag )
{
	guard( "operator new" );
	return appMalloc( Size, Tag );
	unguard;
}
inline void* operator new(size_t Size )
{
	guard( "operator new" );
	return appMalloc( Size, "new" );
	unguard;
}
inline void operator delete( void* Ptr )
{
	guard( "operator delete" );
	appFree( Ptr );
	unguard;
}

/*-----------------------------------------------------------------------------
	Fast inline memory copy/fill functions.
-----------------------------------------------------------------------------*/

/*

//
// MemSet - fast inline for small tasks, jumps back to big one for larger tasks.
//
inline void* appMemset( void* Dest, int C, INT Count )
{
	if (Count < 96) //#debug !!!
	{
		__asm
		{
			mov		ecx,Count
			mov		ebx,C
			mov		edi,[Dest]
			and     ebx,0xff
			shr     ecx,2
			jz      Wrapup
			// At least 4 copies to be made.
			mov     eax,ebx
			shl     ebx,8
			mov     edx,eax
			add     eax,ebx  //		 8:8
			add     edx,ebx  //		 8:8
			shl     eax,16   //  8:8:0:0
			add     eax,edx
		    // Copy up to 24 dwords.
			rep     stosd
		   WrapUp:
			mov     ecx,Count
			mov     ebx,C  
			and  	ecx,3
			jz		MemSetDone

			mov		byte ptr [edi],bl
			dec		ecx
			jz		MemSetDone

			mov		byte ptr [edi+1],bl
			dec		ecx
			jz		MemSetDone

			mov     byte ptr [edi+2],bl
		   MemSetDone:
		}
		return Dest;
	}
	else return appLargeMemset(Dest,C,Count);
}

inline void* appMemcpy( void* Dest, const void* Src, INT Count )
{
	if (Count < 96) 
	{
		__asm
		{
			mov     esi,[Src]
			mov		ecx,[Count]
			mov		edi,[Dest]
			shr     ecx,2
			// jz      Wrapup
		    // Copy up to 24 dwords.
			rep     movsd

		    // WrapUp:
			mov     ecx,[Count]
			and  	ecx,3
			jz		MemcpyDone

			mov     bl,byte ptr [esi]
			dec		ecx
			mov		byte ptr [edi],bl
			jz		MemcpyDone

			mov     bl,byte ptr [esi+1]
			dec		ecx
			mov		byte ptr [edi+1],bl
			jz		MemcpyDone

			mov     bl,byte ptr [esi+2]
			mov     byte ptr [edi+2],bl

		   MemcpyDone:
		}
		return Dest;
	}
	else return appLargeMemcpy(Dest,Src,Count);
}
*/

/*-----------------------------------------------------------------------------
	Sorting functions.
-----------------------------------------------------------------------------*/

//
// Quicksort an array of items.  Implemented as a template so that the compare
// function may be inlined.
//
template<class T> inline void appSort( T* Array, INT Num, INT SortCutoff=8 )
{
	guard(appSort);
    if( Num < 2 )
        return;

	// Simulated stack.
	T* FirstStack[32];
	T* LastStack[32];
    INT StackIndex = 0;

	// Pointers to first and last.
    T* First = Array;
    T* Last = Array + Num - 1;

Resort:
    INT Size = Last - First + 1;
	if( Size <= SortCutoff )
	{
		// Bubble-sort small arrays.
		T* NewLast = Last;
		while( NewLast > First )
		{
			T* Max = First;
			for( T* P=First+1; P<=NewLast; P++ )
			{
				if( Compare(*P, *Max) > 0 )
					Max = P;
			}
			Exchange( *Max, *NewLast );
			NewLast--;
		}
	}
    else
	{
        T* Middle = First + (Size / 2);
        Exchange( *Middle, *First );
        T* FirstPtr = First;
        T* LastPtr = Last + 1;
        for( ; ; )
		{
            do FirstPtr++; while( FirstPtr <= Last && Compare(*FirstPtr, *First) <= 0 );
            do LastPtr--; while( LastPtr > First && Compare(*LastPtr, *First) >= 0 );
            if( LastPtr < FirstPtr )
                break;
            Exchange( *FirstPtr, *LastPtr );
        }
        Exchange( *First, *LastPtr );
        if( LastPtr-1-First >= Last-FirstPtr )
		{
            if( First + 1 < LastPtr )
			{
				// Sort larger subset later.
                FirstStack[StackIndex] = First;
                LastStack[StackIndex] = LastPtr - 1;
                StackIndex++;
            }
            if( FirstPtr < Last )
			{
				// Sort smaller subset now.
                First = FirstPtr;
                goto Resort;
            }
        }
        else
		{
            if( FirstPtr < Last )
			{
				// Sort larger subset later.
                FirstStack[StackIndex] = FirstPtr;
                LastStack[StackIndex] = Last;
                StackIndex++;
            }
            if( First + 1 < LastPtr )
			{
				// Sort smaller subset now.
                Last = LastPtr - 1;
                goto Resort;
            }
        }
    }
    if( --StackIndex >= 0 )
	{
		// Sort array on stack.
        First = FirstStack[StackIndex];
        Last = LastStack[StackIndex];
        goto Resort;
    }
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
