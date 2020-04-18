/*=============================================================================
	UnArc.h: Unreal archive class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	The archive class.
-----------------------------------------------------------------------------*/

//
// Archive class. Used for loading, saving, and garbage collecting
// in a byte order neutral way.
//
class CORE_API FArchive
{
public:
	// FArchive interface.
	virtual FArchive& Serialize ( void* V, INT Length ) {return *this;}
	virtual FArchive& operator<<( class FName& N      ) {return *this;}
	virtual FArchive& operator<<( class UObject*& Res ) {return *this;}
	virtual void Preload( UObject* Object ) {}
	virtual INT MapName( FName* Name ) {return 0;}
	virtual INT MapObject( UObject* Object ) {return 0;}
	virtual void CountBytes( INT Count ) {}

	// Hardcoded datatype routines that may not be overridden.
	FArchive& ByteOrderSerialize( void* V, INT Length )
	{
#if __INTEL__
		Serialize( V, Length );
#else
		if( ArIsLoading || ArIsSaving || ArIsNet )
		{
			// Transferring between memory and file, so flip the byte order.
			for( INT i=Length-1; i>=0; i-- )
				Serialize( (BYTE*)V + i, 1 );
		}
		else
		{
			// Transferring around within memory, so keep the byte order.
			Serialize( V, Length );
		}
#endif
		return *this;
	}

	// Other serialization.
	void String( char* S, INT MaxLength );
	void Printf( const char* Fmt, ... );

	// Constructor.
	FArchive()
	:	ArVer		(PACKAGE_FILE_VERSION)
	,	ArIsLoading	(0)
	,	ArIsSaving	(0)
	,   ArIsTrans   (0)
	,	ArIsNet		(0)
	,	ArForEdit	(1)
	,	ArForClient	(1)
	,	ArForServer	(1)
	{}

	// Status accessors.
	INT Ver()			{return ArVer;}
	UBOOL IsLoading()	{return ArIsLoading;}
	UBOOL IsSaving()	{return ArIsSaving;}
	UBOOL IsTrans()	    {return ArIsTrans;}
	UBOOL ForEdit()		{return ArForEdit;}
	UBOOL ForClient()	{return ArForClient;}
	UBOOL ForServer()	{return ArForServer;}
	virtual INT Tell()	{return -1;}

	// Friend archivers.
	friend FArchive& operator<<( FArchive& Ar, BYTE& B )
	{
		return Ar.Serialize( &B, 1 );
	}
	/*friend FArchive& operator<<( FArchive& Ar, UBOOL& B )
	{
		return Ar.Serialize( &B, sizeof(UBOOL) );
	}*/
	friend FArchive& operator<<( FArchive& Ar, CHAR& C )
	{
		return Ar.Serialize( &C, 1 );
	}
	friend FArchive& operator<<( FArchive& Ar, _WORD& W )
	{
		return Ar.ByteOrderSerialize( &W, sizeof(W) );
	}
	friend FArchive& operator<<( FArchive& Ar, SWORD& S )
	{
		return Ar.ByteOrderSerialize( &S, sizeof(S) );
	}
	friend FArchive& operator<<( FArchive& Ar, DWORD& D )
	{
		return Ar.ByteOrderSerialize( &D, sizeof(D) );
	}
	friend FArchive& operator<<( FArchive& Ar, INT& I )
	{
		return Ar.ByteOrderSerialize( &I, sizeof(I) );
	}
	friend FArchive& operator<<( FArchive& Ar, FLOAT& F )
	{
		return Ar.ByteOrderSerialize( &F, sizeof(F) );
	}
	friend FArchive& operator<<( FArchive &Ar, QWORD& Q )
	{
		return Ar.ByteOrderSerialize( &Q, sizeof(Q) );
	}
	friend FArchive& operator<<( FArchive& Ar, SQWORD& S )
	{
		return Ar.ByteOrderSerialize( &S, sizeof(S) );
	}

protected:
	// Status variables.
	UBOOL ArVer;
	UBOOL ArIsLoading;
	UBOOL ArIsSaving;
	UBOOL ArIsTrans;
	UBOOL ArIsNet;
	UBOOL ArForEdit;
	UBOOL ArForClient;
	UBOOL ArForServer;
};

/*-----------------------------------------------------------------------------
	FArchive operators.
-----------------------------------------------------------------------------*/

// Macro to serialize an enumeration as a byte.
#define ARCHIVE_ENUM_AS_BYTE(EnumType) \
	static inline FArchive& operator<<( FArchive &Ar, enum EnumType &E ) \
		{BYTE B = (BYTE)E; Ar << B; E=(EnumType)B; return Ar;}

// Serialize these enums as bytes.
ARCHIVE_ENUM_AS_BYTE(EExprToken);
ARCHIVE_ENUM_AS_BYTE(ECodeToken);

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
