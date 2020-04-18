/*=============================================================================
	UnPrim.h: Unreal UPrimitive definition.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FCheckResult.
-----------------------------------------------------------------------------*/

//
// Results of an actor check.
//
struct FIteratorActorList : public FIteratorList
{
	// Variables.
	AActor* Actor;

	// Functions.
	FIteratorActorList()
	{}
	FIteratorActorList( FIteratorActorList* InNext, AActor* InActor )
	:	FIteratorList	(InNext)
	,	Actor			(InActor)
	{}
	FIteratorActorList* GetNext()
	{ return (FIteratorActorList*) Next; }
};

//
// Results from a collision check.
//
struct FCheckResult : public FIteratorActorList
{
	// Variables.
	FVector		Location;   // Location of the hit in coordinate system of the returner.
	FVector		Normal;     // Normal vector in coordinate system of the returner. Zero=none.
	UPrimitive*	Primitive;  // Actor primitive which was hit, or NULL=none.
	FLOAT       Time;       // Time until hit, if line check.
	INT			Item;       // Primitive data item which was hit, INDEX_NONE=none.

	// Functions.
	FCheckResult()
	{}
	FCheckResult( FLOAT InTime, FCheckResult* InNext=NULL )
	:	FIteratorActorList( InNext, NULL )
	,	Location	(0,0,0)
	,	Normal		(0,0,0)
	,	Primitive	(NULL)
	,	Time		(InTime)
	,	Item		(INDEX_NONE)
	{}
	FCheckResult*& GetNext()
		{ return *(FCheckResult**)&Next; }
	friend INT Compare( const FCheckResult& A, const FCheckResult& B )
		{ return A.Time<B.Time ? -1 : A.Time>B.Time ? 1 : 0; }
};

/*-----------------------------------------------------------------------------
	UPrimitive.
-----------------------------------------------------------------------------*/

//
// UPrimitive, the base class of geometric entities capable of being
// rendered and collided with.
//
class ENGINE_API UPrimitive : public UObject
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UPrimitive,UObject,0)

	// Variables.
	FBox BoundingBox;
	FSphere BoundingSphere;

	// Constructor.
	UPrimitive()
	: BoundingBox(0)
	, BoundingSphere(0)
	{}

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UPrimitive collision interface.
	virtual UBOOL PointCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		FVector			Location,
		FVector			Extent,
		DWORD           ExtraNodeFlags
	);
	virtual UBOOL LineCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		FVector			End,
		FVector			Start,
		FVector			Extent,
		DWORD           ExtraNodeFlags
	);
	virtual FBox GetRenderBoundingBox( const AActor* Owner, UBOOL Exact ) const;
	virtual FSphere GetRenderBoundingSphere( const AActor* Owner, UBOOL Exact ) const;
	virtual FBox GetCollisionBoundingBox( const AActor* Owner ) const;
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
