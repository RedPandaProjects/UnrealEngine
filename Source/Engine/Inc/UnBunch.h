/*=============================================================================
	UnBunch.h: Unreal bunch class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FBunchHeader.
-----------------------------------------------------------------------------*/

//
// Bunch network subheader.
//
#pragma pack (push,1)
class FBunchHeader
{
public:
	// Variables.
	_WORD ChIndex;		// Channel the bunch is being sent on + channel flags.
	_WORD Sequence;		// This bunch's sequence number      (or SEQ_Initial).

	// Functions.
	FBunchHeader()
	{}
	FBunchHeader( _WORD InChIndex, _WORD InSequence )
	: ChIndex( InChIndex ), Sequence( InSequence )
	{}
};
#pragma pack (pop)

/*-----------------------------------------------------------------------------
	Bunch pooling.
-----------------------------------------------------------------------------*/

// Bunch pooling.
ENGINE_API void* AllocPooledBunch();
ENGINE_API void FreePooledBunch( void* Bunch );

/*-----------------------------------------------------------------------------
	FBunch.
-----------------------------------------------------------------------------*/

//
// Bunch network header.
//
#pragma pack (push,1)
class FBunch : public FBunchHeader
{
public:
	// Network variables.
	_WORD PrevSequence;  // Previous sequence number required (or SEQ_None).
	_WORD DataSize;      // Size of data in bunch.
	BYTE  _ChType;        // Type of channel or special packet.

	// Functions.
	static int GetHeaderSize()
	{
		return sizeof(FBunch);
	}
	INT GetTotalSize()
	{
		return sizeof(FBunch) + DataSize;
	}
};
#pragma pack (pop)

/*-----------------------------------------------------------------------------
	FInBunch.
-----------------------------------------------------------------------------*/

//
// A bunch of data received from a channel.
//
class FInBunch : public FArchive
{
public:
	// Variables.
	FBunch			Header;
	UNetConnection*	Connection;
	UBOOL			Overflowed;
	INT				InPosition;

	// Bunch data.
	BYTE Data[UNetConnection::MAX_PACKET_SIZE];

	// Constructor.
	FInBunch( UNetConnection* InConnection, FBunch* InHeader, BYTE* BunchData )
	:   Header      ( *InHeader )
	,	Connection  ( InConnection )
	,	Overflowed  ( 0 )
	,	InPosition  ( 0 )
	{
		guard(FInBunch::FBunch);
		check(Header.DataSize<=sizeof(Data));
		ArIsNet = 1;

		appMemcpy( Data, BunchData, Header.DataSize );

		unguard;
	}

	// Duplicate the bunch.
	FInBunch* DuplicateBunch()
	{
		guard(FInBunch::DuplicateBunch);
		
		INT Size = sizeof(FInBunch) + Header.DataSize - sizeof(Data);
		FInBunch* Result = (FInBunch*)AllocPooledBunch();
		appMemcpy( Result, this, Size );

		return Result;
		unguard;
	}

	// Read raw data.
	FArchive& Serialize( void *V, int Length )
	{
		guardSlow(FInBunch::Serialize);
		if( InPosition+Length<=Header.DataSize && !Overflowed )
		{
			appMemcpy( V, &Data[InPosition], Length );
			InPosition += Length;
		}
		else Overflowed = 1;
		return *this;
		unguardSlow;
	}

	// Other archivers.
	UBOOL ReceiveProperty( UProperty* Property, BYTE* Data, BYTE* Recent );
	FArchive& operator<<( FName& Name );
	FArchive& operator<<( UObject*& Object );
};

/*-----------------------------------------------------------------------------
	FOutBunch.
-----------------------------------------------------------------------------*/

//
// A bunch of data to send.
//
class FOutBunch : public FArchive
{
public:
	// Variables.
	FBunch		Header;
	FChannel*	Channel;
	UBOOL       Overflowed;
	INT			MaxDataSize;

	// Bunch data.
	BYTE Data[UNetConnection::MAX_PACKET_SIZE];

	// Construct a new bunch for sending.
	FOutBunch( FChannel* InChannel, UBOOL bClose=0 );

	// Duplicate the bunch.
	FOutBunch* DuplicateBunch()
	{
		guard(FOutBunch::DuplicateBunch);

		INT Size = sizeof(FOutBunch) + Header.DataSize - sizeof(Data);
		FOutBunch* Result = (FOutBunch *)AllocPooledBunch();
		appMemcpy( Result, this, Size );

		return Result;
		unguard;
	}

	// Archivers.
	UBOOL SendProperty( UProperty* Property, INT ArrayIndex, BYTE* Data, BYTE* Defaults, UBOOL Named );
	UBOOL SendObject( UObject* Object );
	FArchive& operator<<( FName& Name );
	FArchive& operator<<( UObject*& Object );
	FArchive& Serialize( void* V, int Length );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
