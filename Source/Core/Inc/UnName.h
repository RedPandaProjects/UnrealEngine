/*=============================================================================
	UnName.h: Unreal global name types.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	Definitions.
----------------------------------------------------------------------------*/

// Maximum size of name in characters.
enum {NAME_SIZE	= 64};

// Name index.
typedef INT NAME_INDEX;

// Enumeration for finding name.
enum EFindName
{
	FNAME_Find,			// Find a name; return 0 if it doesn't exist.
	FNAME_Add,			// Find a name or add it if it doesn't exist.
	FNAME_Intrinsic,	// Find a name or add it intrinsically if it doesn't exist.
};

/*----------------------------------------------------------------------------
	FNameEntry.
----------------------------------------------------------------------------*/

//
// A global name, as stored in the global name table.
//
struct FNameEntry
{
	// Variables.
	NAME_INDEX	Index;				// Index of name in hash.
	DWORD		Flags;				// RF_TagImp, RF_TagExp, RF_Intrinsic.
	FNameEntry*	HashNext;			// Pointer to the next entry in this hash bin's linked list.

	// The name string.
	char		Name[NAME_SIZE];	// Name, variable-sized.

	// Functions.
	friend FArchive& operator<<( FArchive& Ar, FNameEntry& E )
	{
		guard(FNameEntry<<);
		Ar.String( E.Name, NAME_SIZE );
		return Ar << E.Flags;
		unguard;
	}
	friend FNameEntry* AllocateNameEntry( const char* Name, DWORD Index, DWORD Flags, FNameEntry* HashNext )
	{
		guard(AllocateNameEntry);

		FNameEntry *NameEntry = (FNameEntry*)appMalloc( sizeof(FNameEntry) + appStrlen(Name) + 1 - NAME_SIZE, "NameEntry" );
		NameEntry->Index      = Index;
		NameEntry->Flags      = Flags;
		NameEntry->HashNext   = HashNext;
		appStrcpy( NameEntry->Name, Name );
		return NameEntry;

		unguard;
	}
};

/*----------------------------------------------------------------------------
	FName.
----------------------------------------------------------------------------*/

//
// Public name, available to the world.  Names are stored as WORD indices
// into the name table and every name in Unreal is stored once
// and only once in that table.  Names are case-insensitive.
//
class CORE_API FName 
{
public:
	// Accessors.
	const char* operator*() const
	{
		debug(Index < Names.Num());
		debug(Names(Index));
		return Names(Index)->Name;
	}
	NAME_INDEX GetIndex() const
	{
		debug(Index < Names.Num());
		debug(Names(Index));
		return Index;
	}
	DWORD GetFlags() const
	{
		debug(Index < Names.Num());
		debug(Names(Index));
		return Names(Index)->Flags;
	}
	void SetFlags( DWORD Set )
	{
		debug(Index < Names.Num());
		debug(Names(Index));
		Names(Index)->Flags |= Set;
	}
	void ClearFlags( DWORD Clear )
	{
		debug(Index < Names.Num());
		debug(Names(Index));
		Names(Index)->Flags &= ~Clear;
	}
	UBOOL operator==( const FName& Other ) const
	{
		return Index==Other.Index;
	}
	UBOOL operator!=( const FName& Other ) const
	{
		return Index!=Other.Index;
	}
	UBOOL IsValid()
	{
		return Index>=0 && Index<Names.Num() && Names(Index)!=NULL;
	}

	// Constructors.
	FName( const char* Name, EFindName FindType=FNAME_Add );
	FName() {}
	FName( enum EName N )
	{
		Index = N;
	}

	// Name subsystem.
	static void InitTables();
	static void InitSubsystem();
	static void ExitSubsystem();
	static void DeleteEntry( int i );
	static void DisplayHash( class FOutputDevice* Out );
	static void Hardcode( FNameEntry& AutoName );

	// Name subsystem accessors.
	static int GetMaxNames()
	{
		return Names.Num();
	}
	static FNameEntry* GetEntry( int i )
	{
		return Names(i);
	}

private:
	// Name index.
	NAME_INDEX Index;

	// Static subsystem variables.
	static TArray<FNameEntry*>	Names;			 // Table of all names.
	static TArray<INT>          Available;       // Indices of available names.
	static FNameEntry*			NameHash[8192];  // Hashed names.
	static INT					Duplicate;       // Duplicate name, if any.
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
