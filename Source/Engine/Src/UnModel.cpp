/*=============================================================================
	UnModel.cpp: Unreal model functions
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	UDatabase.
-----------------------------------------------------------------------------*/

void* UDatabase::Realloc()
{
	Data = appRealloc( Data, DbMax * GetClass()->ClassRecordSize, GetFullName() );
	return Data;
}
void UDatabase::Empty()
{
	guard(UObject::Empty);
	DbNum = DbMax = 0;
	Realloc();
	unguardobj;
}
void UDatabase::Shrink()
{
	guard(UObject::Shrink);
	DbMax = DbNum;
	Realloc();
	unguardobj;
}
void UDatabase::Serialize( FArchive& Ar )
{
	guard(UDatabase::Serialize);
	UObject::Serialize( Ar );
	check(!GetMainFrame());
	if( Ar.Ver() >= 40 )//oldver
	{
		Ar << DbNum << DbMax;
	}
	else
	{
		DbNum = GObj.GetTempNum();
		DbMax = GObj.GetTempMax();
	}
	if( Ar.IsLoading() )
		Realloc();
	SerializeData( Ar );
	unguardf(( "(%i/%i)", DbNum, DbMax ));
}
void UDatabase::Destroy()
{
	guard(UDatabase::Destroy);
	if( Data )
		appFree( Data );
	UObject::Destroy();
	unguard;
}
INT UDatabase::Add( INT NumToAdd )
{
	guard(UObject::Add);

	// Mark the database modified to transactionally save Num.
	Modify();

	int Result = DbNum;
	if( DbNum + NumToAdd > DbMax )
	{
		// Reallocate.
		DbMax += NumToAdd + 256 + (DbNum/4);
		Realloc();
	}
	DbNum += NumToAdd;
	return Result;

	unguardobj;
}
void UDatabase::Remove( int Index, int Count )
{
	guard(UObject::Remove);
	check(Index>=0 && Index<=DbMax);
	
	// Note that Num is being modified.
	Modify();

	// Note that all items were modified.
	for( INT i=Index+Count; i<DbMax; i++ )
		ModifyItem(i);

	// Remove item from list.
	appMemmove
	(
		(BYTE*)GetData() + (Index      ) * GetClass()->ClassRecordSize,
		(BYTE*)GetData() + (Index+Count) * GetClass()->ClassRecordSize,
		(DbNum - Index - Count         ) * GetClass()->ClassRecordSize
	);
	DbNum -= Count;
	unguardobj;
}
void UDatabase::ModifyAllItems()
{
	guard(UObject::ModifyAllItems);

	for( INT i=0; i<DbNum; i++ )
		ModifyItem(i);
	
	unguardobj;
}
IMPLEMENT_CLASS(UDatabase);

/*-----------------------------------------------------------------------------
	Bit arrays.
-----------------------------------------------------------------------------*/

//
// Serialize helper.
//
static void EmitRunlength( FArchive &Ar, BYTE Code, int RunLength )
{
	BYTE A,B;
	if( RunLength <= 63 )	Ar << (A=Code + 0x00 + RunLength);
	else					Ar << (A=Code + 0x40 + (RunLength>>8)) << (B=RunLength);
}

//
// UBitArray serializer.
//
void UBitArray::Serialize( FArchive &Ar )
{
	guard(UBitArray::Serialize);
	UObject::Serialize( Ar );
	Ar << AR_INDEX(NumBits);
	if( Ar.IsSaving() )
	{
		// Save the bit array compressed.
		UBOOL Value    = 0;
		int  RunLength = 0;
		for( DWORD i=0; i<NumBits; i++ )
		{
			if( Get(i)==Value )
			{
				// No change in value.
				if( ++RunLength == 16383 )
				{
					// Overflow, so emit 01 + RunLength[0-16383] + 00+RunLength[0]
					EmitRunlength(Ar, 0, RunLength);
					Value     = !Value;
					RunLength = 0;
				}
			}
			else
			{
				// Change in value.
				if( i+1==NumBits || Get(i+1)!=Value )
				{
					// Permanent change in value.					
					EmitRunlength(Ar, 0, RunLength);
					Value     = !Value;
					RunLength = 0;
				}
				else
				{
					// Temporary 1-bit change in value.
					EmitRunlength(Ar, 0x80, RunLength);
					RunLength = 0;
				}
			}
		}
	}
	else if( Ar.IsLoading() )
	{
		// Load the bit array.
		DWORD Count = 0, RunLength;
		UBOOL  Value = 0;
		BYTE  A, B;
		while( Count<NumBits )
		{
			Ar << A;
			RunLength = A & 0x3f;
			if( A & 0x40 )
			{
				// Two byte runlength.
				Ar << B;
				RunLength = (RunLength << 8) + B;
			}
			while( RunLength-->0 && Count<NumBits )
			{
				// Fill RunLength.
				Set(Count++,Value);
			}
			if( A & 0x80 && Count<NumBits)
			{
				// Write one opposite bit.
				Set(Count++,!Value);
			}
			else
			{
				// Flip value.
				Value = !Value;
			}
		}
	}
	unguardobj;
}
IMPLEMENT_CLASS(UBitArray);
IMPLEMENT_CLASS(UBitMatrix);

/*---------------------------------------------------------------------------------------
	UBspNodes object implementation.
---------------------------------------------------------------------------------------*/

UBspNodes::UBspNodes()
{
	guard(UBspNodes::UBspNodes);

	// Init zone info.
	NumZones = 0;	
	for (int i=0; i<MAX_ZONES; i++)
	{
		Zones[i].ZoneActor    = NULL;
		Zones[i].Connectivity = ((QWORD)1)<<i;
		Zones[i].Visibility   = ~(QWORD)0;
	}	
	unguard;
}
IMPLEMENT_DB_CLASS(UBspNodes);

/*---------------------------------------------------------------------------------------
	UBspSurfs object implementation.
---------------------------------------------------------------------------------------*/

void UBspSurfs::ModifyItem(int Index, int UpdateMaster)
{
	guard(UBspSurfs::Modify);

	UDatabase::ModifyItem(Index);

	if( UpdateMaster && Element(Index).Actor )
		Element(Index).Actor->Brush->Polys->ModifyItem(Element(Index).iBrushPoly);

	unguard;
}
void UBspSurfs::ModifyAllItems(int UpdateMaster)
{
	guard(UBspSurfs::ModifyAllItems);

	for( int i=0; i<Num(); i++ )
		ModifyItem(i,UpdateMaster);

	unguard;
}
void UBspSurfs::ModifySelected(int UpdateMaster)
{
	guard(UBspSurfs::ModifySelected);

	for( int i=0; i<Num(); i++ )
		if( Element(i).PolyFlags & PF_Selected )
			ModifyItem(i,UpdateMaster);

	unguard;
}
IMPLEMENT_DB_CLASS(UBspSurfs);

/*---------------------------------------------------------------------------------------
	UVectors object implementation.
---------------------------------------------------------------------------------------*/

IMPLEMENT_DB_CLASS(UVectors);

/*---------------------------------------------------------------------------------------
	UVerts object implementation.
---------------------------------------------------------------------------------------*/

IMPLEMENT_DB_CLASS(UVerts);

/*---------------------------------------------------------------------------------------
	UModel object implementation.
---------------------------------------------------------------------------------------*/

struct FBoundingVolume : public FBox//oldver
{
	friend FArchive& operator<<( FArchive& Ar, FBoundingVolume& V )
	{
		FPlane Old;
		return Ar << (FBox&)V << Old;
	}
};

void UModel::Serialize( FArchive& Ar )
{
	guard(UModel::Serialize);

	UPrimitive::Serialize( Ar );

	Ar << Vectors << Points << Nodes << Surfs << Verts << Polys;
	Ar << LightMap << LightBits;
	Ar << Bounds;
	Ar << LeafHulls << Leaves << Lights << LeafZone << LeafLeaf;
	if( Ar.Ver() <= 41 ) //oldver
	{
		INT A,B,C;
		Ar << A << B << C;
		RootOutside=A;
		Linked=C;
	}
	else if( Ar.Ver() <= 42 ) //oldver
	{
		BYTE A, B, C;
		Ar << A << B << C;
		RootOutside = A;
		Linked = C;
	}
	else if( Ar.Ver() <= 43 ) //oldver
	{
		BYTE A,B;
		Ar << A << B;
		RootOutside = A;
		Linked = B;
	}
	else
	{
		Ar << RootOutside << Linked;
	}

	unguard;
}
void UModel::Export( FOutputDevice& Out, const char* FileType, int Indent )
{
	guard(UModel::Export);

	Out.Logf("%sBegin Brush Name=%s\r\n",appSpc(Indent),GetName());
	Polys->Export(Out,FileType,Indent+3);
	Out.Logf("%sEnd Brush\r\n",appSpc(Indent));

	unguard;
}
IMPLEMENT_CLASS(UModel);

/*---------------------------------------------------------------------------------------
	UModel implementation.
---------------------------------------------------------------------------------------*/

//
// Lock a model.
//
void UModel::Modify()
{
	guard(UModel::Modify);

	// Modify all child objects.
	//warning: Don't modify self because model contains a dynamic array.
	if( Nodes ) Nodes->Modify();
	if( Surfs ) Surfs->Modify();
	if( Polys ) Polys->Modify();
	if( Points ) Points->Modify();
	if( Vectors ) Vectors->Modify();
	if( Verts ) Verts->Modify();

	unguard;
}

//
// Allocate subobjects for a model.
//
void UModel::AllocDatabases( UBOOL AllocPolys )
{
	guard(UModel::AllocDatabases);

	// Allocate all objects for model.
	Nodes   = new( GetParent(), NAME_None, RF_Transactional )UBspNodes;
	Surfs   = new( GetParent(), NAME_None, RF_Transactional )UBspSurfs;
	Verts   = new( GetParent(), NAME_None, RF_Transactional )UVerts;
	Vectors = new( GetParent(), NAME_None, RF_Transactional )UVectors;
	Points  = new( GetParent(), NAME_None, RF_Transactional )UVectors;

	Bounds.Empty();
	LeafHulls.Empty();

	if( AllocPolys )
		Polys = new( GetParent(), NAME_None, RF_Transactional )UPolys;

	unguard;
}

//
// Create a new model and allocate all objects needed for it.
//
UModel::UModel( ABrush* Owner, UBOOL InRootOutside )
: RootOutside( InRootOutside )
{
	guard(UModel::UModel);
	SetFlags( RF_Transactional );

	AllocDatabases( 1 );
	if( Owner )
	{
		Owner->Brush = this;
		Owner->InitPosRotScale();
	}
	unguard;
}

//
// Build the model's bounds (min and max).
//
void UModel::BuildBound()
{
	guard(UModel::BuildBound);
	if( Polys && Polys->Num() )
	{
		TArray<FVector> Points;
		for( INT i=0; i<Polys->Num(); i++ )
			for( INT j=0; j<Polys->Element(i).NumVertices; j++ )
				Points.AddItem(Polys->Element(i).Vertex[j]);
		BoundingBox    = FBox( Points );
		BoundingSphere = FSphere( Points );
	}
	else BoundingBox = FBox(0);
	unguard;
}

//
// Transform this model by its coordinate system.
//
void UModel::Transform( ABrush* Owner )
{
	guard(UModel::Transform);
	check(Owner);

	Polys->ModifyAllItems();

	FModelCoords Coords;
	FLOAT Orientation = Owner->BuildCoords( &Coords, NULL );
	for( INT i=0; i<Polys->Num(); i++ )
		Polys->Element( i ).Transform( Coords, Owner->PrePivot, Owner->Location, Orientation );

	unguard;
}

//
// Returns whether a BSP leaf is potentially visible from another leaf.
//
UBOOL UModel::PotentiallyVisible( INT iLeaf1, INT iLeaf2 )
{
	// This is the amazing superfast patent-pending 1 cpu cycle potential visibility 
	// algorithm by Tim Sweeney:
	return 1;
}

/*---------------------------------------------------------------------------------------
	UModel basic implementation.
---------------------------------------------------------------------------------------*/

//
// Empty the contents of a model.
//
void UModel::EmptyModel( int EmptySurfInfo, int EmptyPolys )
{
	guard(UModel::Empty);

	// Init subobjects.
	if( Nodes )
	{
		Nodes->Empty();
		Nodes->NumZones=0;
	}
	Bounds.Empty();
	LeafHulls.Empty();
	Leaves.Empty();
	Lights.Empty();
	LightMap.Empty();
	LightBits.Empty();

	if( EmptyPolys )
		Polys->Empty();

	// Empty bsp surface info.	
	if( EmptySurfInfo )
	{
		if( Vectors )
			Vectors->Empty();
		if( Points )
			Points->Empty();
		if( Surfs )
			Surfs->Empty();
	}

	// First 4 shared sides are view frustrum edges.
	if( Verts )
	{
		Verts->Empty();
		Verts->NumSharedSides = 4;
	}
	unguard;
}

//
// Shrink all stuff to its minimum size.
//
void UModel::ShrinkModel()
{
	guard(UModel::ShrinkModel);

	if( Vectors   ) Vectors  ->Shrink();
	if( Points    ) Points   ->Shrink();
	if( Nodes     ) Nodes    ->Shrink();
	if( Surfs     ) Surfs    ->Shrink();
	if( Verts     ) Verts    ->Shrink();
	if( Polys     ) Polys    ->Shrink();

	Bounds.Shrink();
	LeafHulls.Shrink();

	unguard;
}

/*---------------------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------------------*/
