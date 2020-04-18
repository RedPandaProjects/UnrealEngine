/*=============================================================================
	UnFile.cpp: Various file-management functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "CorePrivate.h"

#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <io.h>

/*-----------------------------------------------------------------------------
	Options.
-----------------------------------------------------------------------------*/

// To force memory leak checking.
#undef CHECK_ALLOCS
#define CHECK_ALLOCS 0

/*-----------------------------------------------------------------------------
	FArchive implementation.
-----------------------------------------------------------------------------*/

//
// Serialize a string.
//
void FArchive::String( char* S, INT MaxLength )
{
	guardSlow(FArchive::String);
	check(MaxLength>0);
	for( INT Count=0; Count<MaxLength-1; Count++ )
	{
		Serialize( &S[Count], 1 );
		if( S[Count] == 0 )
			break;
	}
	S[Count] = 0;
	unguardSlow;
}

//
// Printf-style serialization.
//
void FArchive::Printf( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);

	guard(FArchive::Printf);
	check(IsSaving());
	String( TempStr, ARRAY_COUNT(TempStr) );
	unguard;
}

/*-----------------------------------------------------------------------------
	FArray implementation.
-----------------------------------------------------------------------------*/

void FArray::Realloc( INT ElementSize )
{
	guard(FArray::Realloc);
	Data = appRealloc( Data, ArrayMax*ElementSize, "FArray" );
	unguardf(( "%i*%i", ArrayMax, ElementSize ));
}

void FArray::Remove( INT Index, INT Count, INT ElementSize )
{
	if( Count )
	{
		memmove
		(
			(BYTE*)Data + Index         * ElementSize,
			(BYTE*)Data + (Index+Count) * ElementSize,
			(ArrayNum-Index-Count)      * ElementSize
		);
		ArrayNum -= Count;
		//!!need to investigate optimal array shrinking
		//strategy.
		//if( 2*ArrayNum<ArrayMax && ArrayMax-ArrayNum>256 )
		//	Realloc( ElementSize );
	}
	checkarray(ArrayNum>=0);
	checkarray(ArrayMax>=ArrayNum);
}

/*-----------------------------------------------------------------------------
	FString implementation.
-----------------------------------------------------------------------------*/

UBOOL FString::Parse( const char* Stream, const char* Match )
{
	guard(FString::Parse);
	char Temp[4096];
	if( ::Parse( Stream, Match, Temp, ARRAY_COUNT(Temp) ) )
	{
		*this = Temp;
		return 1;
	}
	else return 0;
	unguard;
}

void FString::Appendf( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS( TempStr, Fmt );
	*this += TempStr;
}

void FString::Setf( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS( TempStr, Fmt );
	*this = TempStr;
}

/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/

#if CHECK_ALLOCS
//
// An entry that tracks one allocated memory block.
//
static class FTrackedAllocation
{
public:
	void* Ptr;
	int Size;
	char Name[NAME_SIZE];
	FTrackedAllocation* Next;
} *GTrackedAllocations=NULL;

//
// Add a new allocation to the list of tracked allocations.
//
static void AddTrackedAllocation( void* Ptr, INT Size, const char* Name )
{
	guard(AddTrackedAllocation);
	FTrackedAllocation* A = (FTrackedAllocation*)malloc(sizeof(FTrackedAllocation));

	A->Ptr		= Ptr;
	A->Size		= Size;
	A->Next		= GTrackedAllocations;

	appStrncpy( A->Name, Name, NAME_SIZE );

	GTrackedAllocations = A;
	unguard;
}

//
// Delete an existing allocation from the list.
// Returns the original (pre-alignment) pointer.
//
static void* DeleteTrackedAllocation( void* Ptr )
{
	guard(DeleteTrackedAllocation);
	FTrackedAllocation** PrevLink = &GTrackedAllocations;
	FTrackedAllocation* A = GTrackedAllocations;
	while( A )
	{
		if( A->Ptr == Ptr )
		{
			void* Result = A->Ptr;
			*PrevLink = A->Next;
			free(A);
			return Result;
		}
		PrevLink = &A->Next;
		A        = A->Next;
	}
	appErrorf( "%s", "Allocation not found" );
	return NULL;
	unguard;
}
#endif

//
// Display a list of all tracked allocations that haven't been freed.
//
CORE_API void appDumpAllocs( FOutputDevice* Out )
{
	guard(DumpTrackedAllocations);
#if CHECK_ALLOCS
	INT Count=0;
	for( FTrackedAllocation* A = GTrackedAllocations; A; A=A->Next )
	{
		//Out->Logf( NAME_Exit, "Unfreed: %s (%i)", A->Name, A->Size );
		Count += A->Size;
	}
	debugf( "Total: %fM", Count / 1024.0 / 1024.0 );
#else
	debugf( NAME_Exit, "Allocation checking disabled" );
#endif
	unguard;
}
CORE_API void* appMalloc( INT Size, const char* Tag )
{
	guard(appMalloc);
	check(Size>0);

	void* Ptr = malloc( Size );
	check(Ptr);

#if CHECK_ALLOCS
	AddTrackedAllocation( Ptr, Size, Tag );
#endif

	return Ptr;
	unguard;
}
CORE_API void appFree( void* Ptr )
{
	guard(appFree);
	check(Ptr);

#if CHECK_ALLOCS
	DeleteTrackedAllocation( Ptr );
#endif

	free( Ptr );

	unguard;
}
CORE_API void* appRealloc( void* Ptr, INT NewSize, const char* Tag )
{
	guard(appRealloc);
	check(NewSize>=0);

#if CHECK_ALLOCS
	if( Ptr==NULL )
	{
		if( NewSize==0 )
		{
			return NULL;
		}
		else
		{
			return appMalloc( NewSize, Tag );
		}
	}
	else
	{
		if( NewSize==0 )
		{
			DeleteTrackedAllocation( Ptr );
			return NULL;
		}
		else
		{
			for( FTrackedAllocation *A = GTrackedAllocations; A; A=A->Next )
				if( A->Ptr == Ptr )
					break;
			if( !A )
				appErrorf( "Allocation %s not found ", Tag );
			A->Ptr = realloc( Ptr, NewSize );
			A->Size = NewSize;
			return A->Ptr;
		}
	}
#else
	return realloc( Ptr, NewSize );
#endif

	unguardf(( "%08X %i %s", (INT)Ptr, NewSize, Tag ));
}

/*-----------------------------------------------------------------------------
	Math functions.
-----------------------------------------------------------------------------*/

CORE_API DOUBLE appExp( DOUBLE Value )
{
	return exp(Value);
}
CORE_API DOUBLE appLoge( DOUBLE Value )
{
	return log(Value);
}
CORE_API DOUBLE appFmod( DOUBLE Y, DOUBLE X )
{
	return fmod(Y,X);
}
CORE_API DOUBLE appSin( DOUBLE Value )
{
	return sin(Value);
}
CORE_API DOUBLE appCos( DOUBLE Value )
{
	return cos(Value);
}
CORE_API DOUBLE appTan( DOUBLE Value )
{
	return tan(Value);
}
CORE_API DOUBLE appAtan( DOUBLE Value )
{
	return atan(Value);
}
CORE_API DOUBLE appAtan2( DOUBLE Y, FLOAT X )
{
	return atan2(Y,X);
}
CORE_API DOUBLE appSqrt( DOUBLE Value )
{
	return sqrt(Value);
}
CORE_API DOUBLE appPow( DOUBLE A, DOUBLE B )
{
	return pow(A,B);
}
CORE_API UBOOL appIsNan( DOUBLE A )
{
	return _isnan(A)==1;
}
CORE_API INT appRand()
{
	return rand();
}
CORE_API FLOAT appFrand()
{
	return rand() / (FLOAT)RAND_MAX;
}

#if !DEFINED_appFloor
CORE_API INT DOUBLE appFloor( FLOAT Value )
{
	return floor(Value);
}
#endif

#if !DEFINED_appCeil
CORE_API INT appCeil( FLOAT Value )
{
	return ceil(Value);
}
#endif

/*-----------------------------------------------------------------------------
	File functions.
-----------------------------------------------------------------------------*/

//
// Size of a file.  Returns -1 if doesn't exist.
//
CORE_API INT appFSize( const char* fname )
{
	guard(appFSize);

	FILE* f;
	long result;

	f = fopen( fname, "rb" );
	if( f == NULL )
		return -1;

	if( fseek( f, 0, SEEK_END ) != 0 )
	{
		fclose( f );
		return -1;
	}
	result = ftell( f );
	fclose( f );
	return result;
	unguard;
}

//
// Gets the extension of a file, such as "PCX".  Returns NULL if none.
// string if there's no extension.
//
CORE_API const char* appFExt( const char* fname )
{
	guard(appFExt);

	if( strchr(fname,':') )
		fname = strchr(fname,':')+1;

	while( strchr(fname,'/') )
		fname = strchr(fname,'/')+1;

	while( strchr(fname,'.') )
		fname = strchr(fname,'.')+1;

	return fname;
	unguard;
}

//
// Find files in a directory.
// Example Dir = "c:\\place\\*.ext".
// Example Result = "test.ext"
//
CORE_API TArray<FString> appFindFiles( const char* Spec )
{
	TArray<FString> Result;
	_finddata_t Found;
	long hFind = _findfirst( Spec, &Found );
	if( hFind != -1 )
	{
		do new(Result)FString(Found.name);
		while( _findnext( hFind, &Found )!=-1 );
	}
	return Result;
}

//
// Standard file functions.
//
CORE_API FILE* appFopen( const char* Filename, const char* Mode )
{
	return fopen(Filename,Mode);
}
CORE_API INT appFclose( FILE* Stream )
{
	return fclose(Stream);
}
CORE_API INT appFseek( FILE* Stream, INT Offset, INT Origin )
{
	return fseek(Stream,Offset,Origin);
}
CORE_API INT appFtell( FILE* Stream )
{
	return ftell(Stream);
}
CORE_API INT appFwrite( const void* Buffer, INT Size, INT Count, FILE* Stream )
{
	return fwrite(Buffer,Size,Count,Stream);
}
CORE_API INT appUnlink( const char* Filename )
{
	return unlink(Filename);
}
CORE_API INT appFread( void* Buffer, INT Size, INT Count, FILE* Stream )
{
	return fread(Buffer,Size,Count,Stream);
}
CORE_API INT appFerror( FILE* F )
{
	return ferror(F);
}
CORE_API INT appMkdir( const char* Dirname )
{
	return mkdir( Dirname );
}
CORE_API char* appGetcwd( char* Buffer, INT MaxLen )
{
	return getcwd( Buffer, MaxLen );
}
CORE_API INT appChdir( const char* Dirname )
{
	return chdir( Dirname );
}
CORE_API INT appFprintf( FILE* F, const char* Fmt, ... )
{
	char Temp[32768];
	 GET_VARARGS(Temp,Fmt);
	return appFwrite( Temp, 1, strlen(Temp), F );
}

/*-----------------------------------------------------------------------------
	String functions.
-----------------------------------------------------------------------------*/

//
// Copy a string with length checking.
//
char* appStrncpy( char* Dest, const char* Src, INT MaxLen )
{
	guard(appStrncpy);

	strncpy( Dest, Src, MaxLen );
	Dest[MaxLen-1]=0;
	return Dest;

	unguard;
}

//
// Concatenate a string with length checking
//
char* appStrncat( char* dest, const char* src, int maxlen )
{
	guard(appStrncat);

	char* newdest;
	int   len;

	len      = strlen(dest);
	maxlen  -= len;
	newdest  = dest+len;

	if( maxlen > 0 )
	{
		strncpy(newdest,src,maxlen);
		newdest[maxlen-1]=0;
	}
	return dest;
	unguard;
}

//
// Returns a certain number of spaces.
// Only one return value is valid at a time.
//
const char* appSpc( int num )
{
	guard(spc);

	static char spacing[256];
	static int  oldnum=-1;

	if (num!=oldnum)
	{
		memset( spacing,' ',num );
		spacing [num]=0;
		oldnum=num;
	}
	return spacing;
	unguard;
}

//
// Formatted printing.
//
CORE_API INT appSprintf( char* Dest, const char* Fmt, ... )
{
	INT result = 0;
	 GET_VARARGSR(Dest,Fmt, result);
	 return result;
}

//
// Format a variable arguments expression.
//
CORE_API INT appGetVarArgs( char* Dest, const char*Fmt, va_list va)
{
	INT Result = vsprintf( Dest, Fmt, va );
	return Result;
}

//
// Swap memory in one buffer with another
//
CORE_API void appMemswap( void* Ptr1, void* Ptr2, DWORD Size )
{
	FMemMark Mark(GMem);
	BYTE* Temp = New<BYTE>(GMem,Size);
	appMemcpy( Temp, Ptr1, Size );
	memcpy( Ptr1, Ptr2, Size );
	memcpy( Ptr2, Temp, Size );
	Mark.Pop();
}

//
// Standard string functions.
//
CORE_API INT appStrlen( const char* String )
{
	return strlen(String);
}
CORE_API char* appStrstr( const char* String, const char* Find )
{
	return const_cast<char*>( strstr(String,Find));
}
CORE_API char* appStrchr( const char* String, int c )
{
	return const_cast<char*>(strchr(String,c));
}
CORE_API char* appStrcat( char* Dest, const char* Src )
{
	return strcat(Dest,Src);
}
CORE_API INT appStrcmp( const char* String1, const char* String2 )
{
	return strcmp(String1,String2);
}
CORE_API INT appStricmp( const char* String1, const char* String2 )
{
	return stricmp(String1,String2);
}
CORE_API INT appMemcmp( const void* Buf1, const void* Buf2, INT Count )
{
	return memcmp(Buf1,Buf2,Count);
}
CORE_API char* appStrcpy( char* Dest, const char* Src )
{
	return strcpy(Dest,Src);
}
CORE_API char* appStrupr( char* String )
{
	return strupr(String);
}


//
// Non-inline memory copy/fill functions for larger tasks.
// -> appDeviceMemSet never does any cache warming (eg. for copying
// to video memory )
//


CORE_API void* appLargeMemset( void* Dest, int C, INT Count )
{
	// LARGE Count: larger than 64.
	if (GIsMMX)
	{
		__asm
		{
		// Clear using MMX. Adjusts destination alignment & pads if needed.
			mov		ebx,C
			mov		ecx,Count
			and     ebx,0xff
			mov		edi,Dest
		// expand ebx to 8:8:8:8
			mov     eax,ebx
			shl     ebx,8
			mov     edx,eax
			add     eax,ebx  //		 8:8
			add     edx,ebx  //		 8:8
			shl     eax,16   //  8:8:0:0
			add     eax,edx
		// Expand data to 32:32
			movd    mm0,eax
			movd    mm1,eax
			psllq   mm0,32
			por     mm0,mm1
		//////////////////////////////
			test    edi,7			// Destination alignment on 64 bit boundary ?
			jz      SkipPad64MMX	
			test    edi,3          // non-dword boundary ?
			jz      SkipPad32MMX

			// 1-3 bytes to pad
			mov     [edi],al
			inc     edi
			dec     ecx
			test    edi,3
			jz      SkipPad8MMX

			mov     [edi],al
			inc     edi
			dec     ecx
			test    edi,3
			jz      SkipPad8MMX

			mov     [edi],al
			inc     edi
			dec     ecx

		SkipPad8MMX:
			test    edi,4
			jz      SkipPad64MMX

		SkipPad32MMX: // one dword left to store ?			
			mov     [edi],eax		// Single dword start pad.
			add     edi,4
			sub     ecx,4

		SkipPad64MMX:				
		//////////////////////////////
			mov     ebx,ecx         // store bytes-to-do
			shr     ecx,3           // qwords

			// Store 64 bits/iteration.
			align 16				
		InnerLoop32MMX:				
			movq    [edi],mm0		
			add     edi,8			
			dec     ecx				
			jnz     InnerLoop32MMX	

			// End padding:
			mov		ecx,ebx
			and     ecx,7        // how many bytes left ?
			jz      SetMMX32End  //

			cmp     ecx,4
			jb      Skip32End
			mov     [edi],eax    // single dword
			add     edi,4
			sub     ecx,4
			Skip32End:           // 3-0 bytes left to do.

			cmp     ecx,1
			jb      SetMMX32End
			mov     [edi+0],al     //
			cmp     ecx,2 
			jb      SetMMX32End
			mov     [edi+1],al
			je      SetMMX32End //ecx==2
			mov     [edi+2],al  //ecx==3
		SetMMX32End:
			emms                 
		}
		return Dest;
	}
	else  
	{
		return memset(Dest,C,Count); // VC++, usually the regular movsd		
	}	
}


CORE_API void* appLargeMemcpy( void* Dest, const void* Src, INT Count )
{

	if (GIsMMX)
	{
		__asm
		{
		// Clear using MMX. Adjusts destination alignment & pads if needed.
			mov		esi,Src
			mov		ecx,Count
			mov		edi,Dest
		//////////////////////////////
			test    edi,7			// Destination alignment on 64 bit boundary ?
			jz      SkipPad64MMX	
			test    edi,3           // non-dword boundary ?
			jz      SkipPad32MMX

			// 1-3 bytes to pad
			mov     al,[esi]
			inc     esi
			dec     ecx
			mov     [edi],al
			inc     edi

			test    edi,3
			jz      SkipPad8MMX

			mov     al,[esi]
			inc     esi
			dec     ecx
			mov     [edi],al
			inc     edi

			test    edi,3
			jz      SkipPad8MMX

			mov     al,[esi]
			inc     esi
			dec     ecx			
			mov     [edi],al
			inc     edi
			
		SkipPad8MMX:
			test    edi,4
			jz      SkipPad64MMX

		SkipPad32MMX: // one dword left to store ?			
			mov     eax,[esi]
			add     esi,4
			sub     ecx,4
			mov     [edi],eax		// Single dword start pad.
			add     edi,4

		SkipPad64MMX:				
		//////////////////////////////
			mov     ebx,ecx         // store bytes-to-do
			shr     ecx,3           // qwords

			// Store 64 bits/iteration.
			align 16				
		InnerLoop32MMX:				
			movq    mm0,[esi]
			add     esi,8
			movq    [edi],mm0		
			add     edi,8
			dec     ecx
			jnz     InnerLoop32MMX	

			// End padding:
			mov		ecx,ebx
			and     ecx,7        // how many bytes left ?
			jz      SetMMX32End  //

			cmp     ecx,4
			jb      Skip32End
			mov     eax,[esi]
			add     esi,4
			sub     ecx,4
			mov     [edi],eax    // single dword
			add     edi,4
			Skip32End:           // 3-0 bytes left to do.

			cmp     ecx,1
			jb      SetMMX32End
			mov     al,[esi+0]
			mov     [edi+0],al     //
			cmp     ecx,2 
			jb      SetMMX32End

			mov     al,[esi+1]
			mov     [edi+1],al
			je      SetMMX32End //ecx==2

			mov     al,[esi+2]
			mov     [edi+2],al  //ecx==3

		SetMMX32End:
			emms                 
		}
		return Dest;
	}
	else  
	{
		return memcpy(Dest,Src,Count);
	}	
}

CORE_API void* appMemmove( void* Dest, const void* Src, INT Count )
{
	return memmove( Dest, Src, Count );
}

CORE_API void  appMemset( void* Dest, int C, INT Count )
{
	memset( Dest, C, Count );
}

CORE_API void* appMemcpy( void* Dest, const void* Src, INT Count )
{
	return memcpy( Dest, Src, Count );
}




/*-----------------------------------------------------------------------------
	CRC functions.
-----------------------------------------------------------------------------*/

//
// CRC table.
//
CORE_API DWORD GCRCTable[256] =
{
    0x00000000,  0x77073096, 0x0EE0E612C,  0x990951BA,
    0x076DC419,  0x706AF48F, 0x0E963A535,  0x9E6495A3,
    0x0EDB8832,  0x79DCB8A4, 0x0E0D5E91E,  0x97D2D988,
    0x09B64C2B,  0x7EB17CBD, 0x0E7B82D07,  0x90BF1D91,
    0x1DB71064,  0x6AB020F2, 0x0F3B97148,  0x84BE41DE,
    0x1ADAD47D,  0x6DDDE4EB, 0x0F4D4B551,  0x83D385C7,
    0x136C9856,  0x646BA8C0, 0x0FD62F97A,  0x8A65C9EC,
    0x14015C4F,  0x63066CD9, 0x0FA0F3D63,  0x8D080DF5,
    0x3B6E20C8,  0x4C69105E, 0x0D56041E4,  0x0A2677172,
    0x3C03E4D1,  0x4B04D447, 0x0D20D85FD,  0x0A50AB56B,
    0x35B5A8FA,  0x42B2986C, 0x0DBBBC9D6,  0x0ACBCF940,
    0x32D86CE3,  0x45DF5C75, 0x0DCD60DCF,  0x0ABD13D59,
    0x26D930AC,  0x51DE003A, 0x0C8D75180,  0x0BFD06116,
    0x21B4F4B5,  0x56B3C423, 0x0CFBA9599,  0x0B8BDA50F,
    0x2802B89E,  0x5F058808, 0x0C60CD9B2,  0x0B10BE924,
    0x2F6F7C87,  0x58684C11, 0x0C1611DAB,  0x0B6662D3D,
    0x76DC4190,  0x01DB7106, 0x98D220BC,   0x0EFD5102A,
    0x71B18589,  0x06B6B51F, 0x9FBFE4A5,   0x0E8B8D433,
    0x7807C9A2,  0x0F00F934, 0x9609A88E,   0x0E10E9818,
    0x7F6A0DBB,  0x086D3D2D, 0x91646C97,   0x0E6635C01,
    0x6B6B51F4,  0x1C6C6162, 0x856530D8,   0x0F262004E,
    0x6C0695ED,  0x1B01A57B, 0x8208F4C1,   0x0F50FC457,
    0x65B0D9C6,  0x12B7E950, 0x8BBEB8EA,   0x0FCB9887C,
    0x62DD1DDF,  0x15DA2D49, 0x8CD37CF3,   0x0FBD44C65,
    0x4DB26158,  0x3AB551CE, 0x0A3BC0074,  0x0D4BB30E2,
    0x4ADFA541,  0x3DD895D7, 0x0A4D1C46D,  0x0D3D6F4FB,
    0x4369E96A,  0x346ED9FC, 0x0AD678846,  0x0DA60B8D0,
    0x44042D73,  0x33031DE5, 0x0AA0A4C5F,  0x0DD0D7CC9,
    0x5005713C,  0x270241AA, 0x0BE0B1010,  0x0C90C2086,
    0x5768B525,  0x206F85B3, 0x0B966D409,  0x0CE61E49F,
    0x5EDEF90E,  0x29D9C998, 0x0B0D09822,  0x0C7D7A8B4,
    0x59B33D17,  0x2EB40D81, 0x0B7BD5C3B,  0x0C0BA6CAD,
    0x0EDB88320, 0x9ABFB3B6, 0x03B6E20C,   0x74B1D29A,
    0x0EAD54739, 0x9DD277AF, 0x04DB2615,   0x73DC1683,
    0x0E3630B12, 0x94643B84, 0x0D6D6A3E,   0x7A6A5AA8,
    0x0E40ECF0B, 0x9309FF9D, 0x0A00AE27,   0x7D079EB1,
    0x0F00F9344, 0x8708A3D2, 0x1E01F268,   0x6906C2FE,
    0x0F762575D, 0x806567CB, 0x196C3671,   0x6E6B06E7,
    0x0FED41B76, 0x89D32BE0, 0x10DA7A5A,   0x67DD4ACC,
    0x0F9B9DF6F, 0x8EBEEFF9, 0x17B7BE43,   0x60B08ED5,
    0x0D6D6A3E8, 0x0A1D1937E, 0x38D8C2C4,  0x4FDFF252,
    0x0D1BB67F1, 0x0A6BC5767, 0x3FB506DD,  0x48B2364B,
    0x0D80D2BDA, 0x0AF0A1B4C, 0x36034AF6,  0x41047A60,
    0x0DF60EFC3, 0x0A867DF55, 0x316E8EEF,  0x4669BE79,
    0x0CB61B38C, 0x0BC66831A, 0x256FD2A0,  0x5268E236,
    0x0CC0C7795, 0x0BB0B4703, 0x220216B9,  0x5505262F,
    0x0C5BA3BBE, 0x0B2BD0B28, 0x2BB45A92,  0x5CB36A04,
    0x0C2D7FFA7, 0x0B5D0CF31, 0x2CD99E8B,  0x5BDEAE1D,
    0x9B64C2B0,  0x0EC63F226, 0x756AA39C,  0x026D930A,
    0x9C0906A9,  0x0EB0E363F, 0x72076785,  0x05005713,
    0x95BF4A82,  0x0E2B87A14, 0x7BB12BAE,  0x0CB61B38,
    0x92D28E9B,  0x0E5D5BE0D, 0x7CDCEFB7,  0x0BDBDF21,
    0x86D3D2D4,  0x0F1D4E242, 0x68DDB3F8,  0x1FDA836E,
    0x81BE16CD,  0x0F6B9265B, 0x6FB077E1,  0x18B74777,
    0x88085AE6,  0x0FF0F6A70, 0x66063BCA,  0x11010B5C,
    0x8F659EFF,  0x0F862AE69, 0x616BFFD3,  0x166CCF45,
    0x0A00AE278, 0x0D70DD2EE, 0x4E048354,  0x3903B3C2,
    0x0A7672661, 0x0D06016F7, 0x4969474D,  0x3E6E77DB,
    0x0AED16A4A, 0x0D9D65ADC, 0x40DF0B66,  0x37D83BF0,
    0x0A9BCAE53, 0x0DEBB9EC5, 0x47B2CF7F,  0x30B5FFE9,
    0x0BDBDF21C, 0x0CABAC28A, 0x53B39330,  0x24B4A3A6,
    0x0BAD03605, 0x0CDD70693, 0x54DE5729,  0x23D967BF,
    0x0B3667A2E, 0x0C4614AB8, 0x5D681B02,  0x2A6F2B94,
    0x0B40BBE37, 0x0C30C8EA1, 0x5A05DF1B,  0x2D02EF8D
};

//
// CRC32 computer based on polynomial:
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.
//
unsigned long appMemCrc( const unsigned char* Data, int Length )
{
	guard(appMemCrc);
	unsigned long CRC = 0xFFFFFFFF;
	for( int i=0; i<Length; i++ )
	{
		int Index = (CRC ^ *(Data++)) & 0x000000FF;
		CRC       = ((CRC >> 8) & 0x00FFFFFF) ^ GCRCTable[Index];
	}
	return ~CRC;
	unguard;
}

/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/

//
// See if memory is zero.
//
CORE_API UBOOL appMemIsZero( const void* V, int Count )
{
	guardSlow(appMemIsZero);
	BYTE* B = (BYTE*)V;
	while( Count-- > 0 )
		if( *B++ != 0 )
			return 0;
	return 1;
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	Conversion functions.
-----------------------------------------------------------------------------*/

//
// Conversions.
//
CORE_API INT appAtoi( const char* Str )
{
	return atoi( Str );
}
CORE_API FLOAT appAtof( const char* Str )
{
	return atof( Str );
}
CORE_API INT appStrtoi( const char* Start, char** End, INT Base )
{
	return strtoul( Start, End, Base );
}
CORE_API void appQsort( void* Base, INT Num, INT Width, int(CDECL *Compare)(const void* A, const void* B ) )
{
	qsort( Base, Num, Width, Compare );
}
CORE_API INT appStrnicmp( const char* A, const char* B, INT Count )
{
	return _strnicmp( A, B, Count );
}

/*-----------------------------------------------------------------------------
	Substrings.
-----------------------------------------------------------------------------*/

//
// Find string in string, case insensitive, requires non-alphanumeric lead-in.
//
const char* appStrfind( const char* str, const char* find )
{
	guard(appStrfind);
	
	int alnum,length;
	char f,c;

	alnum  = 0;
	f      = appToUpper(*find);
	length = strlen(find)-1;
	find   ++;
	c      = *str++;
	while( c )
	{
		c = appToUpper(c);
		if( !alnum && c==f && !appStrnicmp(str,find,length) )
			return str-1;
		alnum = (c>='A' && c<='Z') || (c>='0' && c<='9');
		c = *str++;
	}
	return NULL;
	unguard;
}

/*-----------------------------------------------------------------------------
	Exceptions.
-----------------------------------------------------------------------------*/

//
// Throw a string exception with a message.
//
CORE_API void VARARGS appThrowf( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);
	throw( TempStr );
}

/*-----------------------------------------------------------------------------
	Parameter parsing.
-----------------------------------------------------------------------------*/

//
// Get a string from a text string.
//
CORE_API UBOOL Parse
(
	const char*	Stream, 
	const char*	Match,
	char*		Value,
	INT			MaxLen
)
{
	guard(ParseString);

	int i=strlen(Stream);
	const char* Found = appStrfind(Stream,Match);
	const char* Start;

	if( Found )
	{
		Start = Found+strlen(Match);
		if( *Start == '\x22' )
		{
			// Quoted string with spaces.
			strncpy (Value,Start+1,MaxLen);
			Value[MaxLen-1]=0;
			char *Temp = strchr( Value, '\x22' );
			if( Temp != NULL )
				*Temp=0;
		}
		else
		{
			// Non-quoted string without spaces.
			strncpy(Value,Start,MaxLen);
			Value[MaxLen-1]=0;
			char* Temp;
			Temp=strchr( Value, ' '  ); if( Temp ) *Temp=0;
			Temp=strchr( Value, '\r' ); if( Temp ) *Temp=0;
			Temp=strchr( Value, '\n' ); if( Temp ) *Temp=0;
			Temp=strchr( Value, '\t' ); if( Temp ) *Temp=0;
			Temp=strchr( Value, ','  ); if( Temp ) *Temp=0;
		}
		return 1;
	}
	else return 0;
	unguard;
}

//
// See if a command-line parameter exists in the stream.
//
UBOOL CORE_API ParseParam( const char* Stream, const char* Param )
{
	guard(GetParam);
	const char* Start = Stream;
	if( *Stream )
		while( (Start=appStrfind(Start+1,Param)) != NULL )
			if( Start>Stream && (Start[-1]=='-' || Start[-1]=='/') )
				return 1;
	return 0;
	unguard;
}

//
// Get an object from a text stream.
//
CORE_API UBOOL ParseObject( const char* Stream, const char* Match, UClass* Class, UObject*& DestRes, UObject* InParent )
{
	guard(ParseUObject);
	char TempStr[256];
	if( !Parse( Stream, Match, TempStr, NAME_SIZE ) )
	{
		return 0;
	}
	else if( stricmp(TempStr,"NONE")==0 )
	{
		DestRes = NULL;
		return 1;
	}
	else
	{
		// Look this object up.
		UObject* Res;
		Res = GObj.FindObject( Class, InParent, TempStr );
		if( !Res )
			return 0;
		DestRes = Res;
		return 1;
	}
	unguard;
}

//
// Get a name.
//
CORE_API UBOOL Parse
(
	const char*	Stream, 
	const char*	Match, 
	FName&		Name
)
{
	guard(ParseFName);
	char TempStr[NAME_SIZE];

	if( !Parse(Stream,Match,TempStr,NAME_SIZE) )
		return 0; // Match not found.

	Name = FName( TempStr );

	return 1;
	unguard;
}

//
// Get a DWORD.
//
CORE_API UBOOL Parse( const char* Stream, const char* Match, DWORD& Value )
{
	guard(ParseDWORD);

	const char *Temp=appStrfind(Stream,Match);
	if( Temp==NULL )
		return 0;
	Value=(DWORD)atol(Temp+strlen(Match));

	return 1;
	unguard;
}

//
// Get a byte (0-255).
//
UBOOL CORE_API Parse( const char* Stream, const char* Match, BYTE& Value )
{
	guard(ParseBYTE);
	const char* Temp = appStrfind(Stream,Match);
	if( Temp==NULL )
		return 0;
	Temp += appStrlen( Match );
	Value = (BYTE)appAtoi( Temp );
	return Value!=0 || appIsDigit(Temp[0]);
	unguard;
}

//
// Get a signed byte (-128 to 127).
//
UBOOL CORE_API Parse( const char* Stream, const char* Match, CHAR& Value )
{
	guard(ParseCHAR);
	const char* Temp = appStrfind(Stream,Match);
	if( Temp==NULL )
		return 0;
	Temp += appStrlen( Match );
	Value = appAtoi( Temp );
	return Value!=0 || appIsDigit(Temp[0]);
	unguard;
}

//
// Get a word (0-65536).
//
UBOOL CORE_API Parse( const char* Stream, const char* Match, _WORD& Value )
{
	guard(ParseWORD);
	const char* Temp = appStrfind( Stream, Match );
	if( Temp==NULL )
		return 0;
	Temp += appStrlen( Match );
	Value = (_WORD)appAtoi( Temp );
	return Value!=0 || appIsDigit(Temp[0]);
	unguard;
}

//
// Get a signed word (-32768 to 32767).
//
UBOOL CORE_API Parse( const char* Stream, const char* Match, SWORD& Value )
{
	guard(ParseSWORD);
	const char* Temp = appStrfind( Stream, Match );
	if( Temp==NULL )
		return 0;
	Temp += appStrlen( Match );
	Value = (SWORD)appAtoi( Temp );
	return Value!=0 || appIsDigit(Temp[0]);
	unguard;
}

//
// Get a floating-point number.
//
UBOOL CORE_API Parse( const char* Stream, const char* Match, FLOAT& Value )
{
	guard(ParseFLOAT);
	const char* Temp = appStrfind( Stream, Match );
	if( Temp==NULL )
		return 0;
	Value = appAtof( Temp+appStrlen(Match) );
	return 1;
	unguard;
}

//
// Get a signed double word (4 bytes).
//
UBOOL CORE_API Parse( const char* Stream, const char* Match, INT& Value )
{
	guard(ParseINT);
	const char* Temp = appStrfind( Stream, Match );
	if( Temp==NULL )
		return 0;
	Value = appAtoi( Temp + appStrlen(Match) );
	return 1;
	unguard;
}

//
// Get a boolean value.
//
UBOOL CORE_API ParseUBOOL( const char* Stream, const char* Match, UBOOL& OnOff )
{
	guard(ParseUBOOL);
	char TempStr[16];
	if( Parse( Stream, Match, TempStr, 16 ) )
	{
		OnOff = !appStricmp(TempStr,"ON") || !appStricmp(TempStr,"TRUE") || !appStricmp(TempStr,"1");
		return 1;
	}
	else return 0;
	unguard;
}

//
// Get a globally unique identifier.
//
CORE_API UBOOL Parse( const char* Stream, const char* Match, class FGuid& Guid )
{
	guard(ParseGUID);

	char Temp[256];
	if( !Parse( Stream, Match, Temp, ARRAY_COUNT(Temp) ) )
		return 0;

	Guid.A = Guid.B = Guid.C = Guid.D = 0;
	if( strlen(Temp)==32 )
	{
		char* End;
		Guid.D = strtoul( Temp+24, &End, 16 ); Temp[24]=0;
		Guid.C = strtoul( Temp+16, &End, 16 ); Temp[16]=0;
		Guid.B = strtoul( Temp+8,  &End, 16 ); Temp[8 ]=0;
		Guid.A = strtoul( Temp+0,  &End, 16 ); Temp[0 ]=0;
	}
	return 1;

	unguard;
}

//
// Sees if Stream starts with the named command.  If it does,
// skips through the command and blanks past it.  Returns 1 of match,
// 0 if not.
//
CORE_API UBOOL ParseCommand
(
	const char**	Stream, 
	const char*		Match
)
{
	guard(ParseCommand);

	while( (**Stream==' ')||(**Stream==9) )
		(*Stream)++;

	if( appStrnicmp(*Stream,Match,strlen(Match))==0 )
	{
		*Stream += strlen(Match);
		if( !appIsAlnum(**Stream) )
		{
			while ((**Stream==' ')||(**Stream==9)) (*Stream)++;
			return 1; // Success.
		}
		else
		{
			*Stream -= strlen(Match);
			return 0; // Only found partial match.
		}
	}
	else return 0; // No match.
	unguard;
}

//
// Get next command.  Skips past comments and cr's.
//
CORE_API void ParseNext( const char** Stream )
{
	guard(ParseNext);

	// Skip over spaces, tabs, cr's, and linefeeds.
	SkipJunk:
	while( **Stream==' ' || **Stream==9 || **Stream==13 || **Stream==10 )
		++*Stream;

	if( **Stream==';' )
	{
		// Skip past comments.
		while( **Stream!=0 && **Stream!=10 && **Stream!=13 )
			++*Stream;
		goto SkipJunk;
	}

	// Upon exit, *Stream either points to valid Stream or a zero character.
	unguard;
}

//
// Grab the next space-delimited string from the input stream.
// If quoted, gets entire quoted string.
//
CORE_API UBOOL ParseToken( const char*& Str, char* Result, INT MaxLen, UBOOL UseEscape )
{
	guard(ParseToken);
	INT Len=0;

	// Skip spaces and tabs.
	while( *Str==' ' || *Str==9 )
		Str++;

	if( *Str == 34 )
	{
		// Get quoted string.
		Str++;
		while( *Str && *Str!=34 && (Len+1)<MaxLen )
		{
			char c = *Str++;
			if( c=='\\' && UseEscape )
			{
				// Get escaped character.
				c = *Str++;
				if( !c )
					break;
			}
			if( (Len+1)<MaxLen )
				Result[Len++] = c;
		}
		if( *Str==34 )
			Str++;
	}
	else
	{
		// Get unquoted string.
		while( *Str && *Str!=' ' && *Str!=9 )
			if( (Len+1)<MaxLen )
				Result[Len++] = *Str++;
	}
	Result[Len]=0;
	return Len!=0;
	unguard;
}

//
// Get a line of Stream (everything up to, but not including, CR/LF.
// Returns 0 if ok, nonzero if at end of stream and returned 0-length string.
//
CORE_API UBOOL ParseLine
(
	const char**	Stream,
	char*			Result,
	INT				MaxLen,
	UBOOL			Exact
)
{
	guard(ParseLine);
	int GotStream=0;
	int IsQuoted=0;
	int Ignore=0;

	*Result=0;
	while( **Stream!=0 && **Stream!=10 && **Stream!=13 && --MaxLen>0 )
	{
		// Start of comments.
		if( !IsQuoted && !Exact && (*Stream)[0]=='/' && (*Stream)[1]=='/' )
			Ignore = 1;
		
		// Command chaining.
		if( !IsQuoted && !Exact && **Stream=='|' )
			break;

		// Check quoting.
		IsQuoted = IsQuoted ^ (**Stream==34);
		GotStream=1;

		if( !Ignore )
			// Got stuff.
			*(Result++) = *((*Stream)++);
		else
			(*Stream)++;
	}
	if( Exact )
	{
		// Eat up exactly one CR/LF.
		if( **Stream == 13 )
			(*Stream)++;
		if( **Stream == 10 )
			(*Stream)++;
	}
	else
	{
		// Eat up all CR/LF's.
		while( **Stream==10 || **Stream==13 || **Stream=='|' )
			(*Stream)++;
	}
	*Result=0;
	return **Stream!=0 || GotStream;
	unguard;
}

/*----------------------------------------------------------------------------
	Localization.
----------------------------------------------------------------------------*/

static char GLanguage[32]="int";
CORE_API const char* Localize( const char* Section, const char* Key, const char* Package, const char* LangExt )
{
	guard(Localize);
	char Filename[256];
	LangExt = LangExt ? LangExt : GLanguage;
	bool FromGame = true;
TryAgain:
	appSprintf( Filename, "%s%s.%s", appLocalizationDir(FromGame), Package, LangExt );
	static char Results[8][256];
	static INT iResult=0;
	iResult = (iResult+1) % ARRAY_COUNT(Results);
	if( !GetConfigString( Section, Key, Results[iResult], ARRAY_COUNT(Results[0]), Filename ) )
	{
		if (FromGame)
		{
			FromGame = false;
			goto TryAgain;
		}
		if( appStricmp(LangExt,"int")!=0 )
		{
			LangExt = "int";
			goto TryAgain;
		}
		debugf( NAME_Localization, "No localization: %s.%s.%s (%s)", Package, Section, Key, LangExt );
		appSprintf( Results[iResult], "<?%s?%s.%s.%s?>", LangExt, Package, Section, Key );
	}
	return Results[iResult];
	unguard;
}
CORE_API const char* LocalizeError( const char* Key, const char* Package, const char* LangExt )
{
	return Localize( "Errors", Key, Package, LangExt );
}
CORE_API const char* LocalizeProgress( const char* Key, const char* Package, const char* LangExt )
{
	return Localize( "Progress", Key, Package, LangExt );
}
CORE_API const char* LocalizeQuery( const char* Key, const char* Package, const char* LangExt )
{
	return Localize( "Query", Key, Package, LangExt );
}
CORE_API const char* LocalizeGeneral( const char* Key, const char* Package, const char* LangExt )
{
	return Localize( "General", Key, Package, LangExt );
}
CORE_API const char* GetLanguage()
{
	guard(GetLanguage);
	return GLanguage;
	unguard;
}
CORE_API void SetLanguage( const char* LangExt )
{
	guard(SetLanguage);
	appStrcpy( GLanguage, LangExt );
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
