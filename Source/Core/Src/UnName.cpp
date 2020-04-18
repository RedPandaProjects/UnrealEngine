/*=============================================================================
	UnName.cpp: Unreal global name code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	FName statics.
-----------------------------------------------------------------------------*/

INT					FName::Duplicate =0;
FNameEntry*			FName::NameHash[8192];
TArray<FNameEntry*>	FName::Names;
TArray<INT>         FName::Available;

/*-----------------------------------------------------------------------------
	FName implementation.
-----------------------------------------------------------------------------*/

//
// Hardcode a name.
//warning: Called at library initialization. Be very careful.
//
void FName::Hardcode( FNameEntry& AutoName )
{
	// Add name to name hash.
	int iHash         = appStrihash(AutoName.Name) & (ARRAY_COUNT(NameHash)-1);
	AutoName.HashNext = NameHash[iHash];
	NameHash[iHash]   = (FNameEntry*)&AutoName;

	// Expand the table if needed.
	for( int i=Names.Num(); i<=AutoName.Index; i++ )
		Names.AddItem( NULL );

	// Add name to table.
	if( Names(AutoName.Index) )
		Duplicate = AutoName.Index;
	Names(AutoName.Index) = (FNameEntry*)&AutoName;
}

//
// FName constructor.
//warning: Can be called at library initialization. Be very careful.
//
FName::FName( const char* Name, EFindName FindType )
{
	guard(FName::FName);
	check(Name);

	// Initialize the name hash if needed.
	if( !Names.Num() )
		InitTables();

	// If empty or invalid name was specified, return NAME_None.
	if( !Name[0] )
	{
		Index = NAME_None;
		return;
	}

	// Form a valid name by discarding any invalid characters.
	char ValidName[NAME_SIZE];
	INT Count=0;
	while( *Name && Count<NAME_SIZE )
	{
		ValidName[Count++] = *Name;
		Name++;
	}
	ValidName[Count]=0;

	// Try to find the name in the hash.
	INT iHash = appStrihash(ValidName) & (ARRAY_COUNT(NameHash)-1);
	for( FNameEntry* Hash=NameHash[iHash]; Hash; Hash=Hash->HashNext )
	{
		if( appStricmp( ValidName, Hash->Name )==0 )
		{
			// Found it in the hash.
			Index = Hash->Index;
			return;
		}
	}

	// Didn't find name.
	if( FindType==FNAME_Find )
	{
		// Not found.
		Index = NAME_None;
		return;
	}

	// Find an available entry in the name table.
	if( Available.Num() )
	{
		Index = Available( Available.Num()-1 );
		Available.Remove( Available.Num()-1 );
	}
	else Index = Names.Add();

	// Allocate the name and set it.
	Names(Index) = NameHash[iHash] = AllocateNameEntry( ValidName, Index, 0, NameHash[iHash] );

	// Set intrinsic flag.
	if( FindType==FNAME_Intrinsic )
		Names(Index)->Flags |= RF_Intrinsic;

	unguard;
}

/*-----------------------------------------------------------------------------
	FName subsystem.
-----------------------------------------------------------------------------*/

//
// Initialize the name subsystem.
//
void FName::InitSubsystem()
{
	guard(FName::InitSubsystem);
	check((ARRAY_COUNT(NameHash)&(ARRAY_COUNT(NameHash)-1)) == 0);
	check(Names.Num());
	if( Duplicate )
		appErrorf( "Hardcoded name %i was duplicated", Duplicate );

	// Verify no duplicate names.
	for( int i=0; i<ARRAY_COUNT(NameHash); i++ )
		for( FNameEntry* Hash=NameHash[i]; Hash; Hash=Hash->HashNext )
			for( FNameEntry* Other=Hash->HashNext; Other; Other=Other->HashNext )
				if( appStricmp(Hash->Name,Other->Name)==0 )
					appErrorf( "Name '%s' was duplicated", Hash->Name );

	debugf( NAME_Init, "Name subsystem initialized" );
	unguard;
}

//
// Shut down the name subsystem.
//
void FName::ExitSubsystem()
{
	guard(FName::ExitSubsystem);

	// Kill all names.
	for( int i=0; i<Names.Num(); i++ )
		if( Names(i) && !(Names(i)->Flags & RF_Intrinsic) )
			delete Names(i);

	debugf( NAME_Exit, "Name subsystem shut down" );
	unguard;
}

//
// Display the contents of the global name hash.
//
void FName::DisplayHash( FOutputDevice* Out )
{
	guard(FName::DisplayHash);

	int UsedBins=0, NameCount=0;
	for( int i=0; i<ARRAY_COUNT(NameHash); i++ )
	{
		if( NameHash[i] != NULL ) UsedBins++;
		for( FNameEntry *Hash = NameHash[i]; Hash; Hash=Hash->HashNext )
			NameCount++;
	}
	Out->Logf( "Hash: %i names, %i/%i hash bins", NameCount, UsedBins, ARRAY_COUNT(NameHash) );

	unguard;
}

//
// Delete an name permanently; called by garbage collector.
//
void FName::DeleteEntry( int i )
{
	guard(FName::DeleteEntry);

	// Unhash it.
	FNameEntry* Name = Names(i);
	check(Name!=NULL);
	check(!(Name->Flags & RF_Intrinsic));

	int iHash = appStrihash(Name->Name) & (ARRAY_COUNT(NameHash)-1);
	for( FNameEntry **HashLink=&NameHash[iHash]; *HashLink && *HashLink!=Name; HashLink=&(*HashLink)->HashNext );
	if( !*HashLink )
		appErrorf( "Unhashed name '%s'", Name->Name );
	*HashLink = (*HashLink)->HashNext;

	// Remove it from the global name table.
	Names(i) = NULL;
	Available.AddItem(i);

	// Delete it.
	delete Name;

	unguard;
}

/*-----------------------------------------------------------------------------
	Internal initialization.
-----------------------------------------------------------------------------*/

//
// Init internal name subsystem info.
//warning: Called at library initialization; can't guard. Be very careful.
//
void FName::InitTables()
{
	guard(FName::InitTables);

	// Init the name hash.
	for( int i=0; i<ARRAY_COUNT(FName::NameHash); i++ )
		NameHash[i] = NULL;

	// Register all hardcoded names.
	#define REGISTER_NAME(num,namestr) static FNameEntry namestr##NAME={num,RF_Intrinsic,NULL,#namestr}; Hardcode(namestr##NAME);
	#define REG_NAME_HIGH(num,namestr) static FNameEntry namestr##NAME={num,RF_Intrinsic|RF_HighlightedName,NULL,#namestr}; Hardcode(namestr##NAME);
	#include "UnNames.h"

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
