/*=============================================================================
    UnPhys.cpp: Simple physics and occlusion testing for editor
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*---------------------------------------------------------------------------------------
   Primitive bounding boxes.
---------------------------------------------------------------------------------------*/

//
// Get collision bounding box.
//
FBox UModel::GetCollisionBoundingBox( const AActor* Owner ) const
{
	guard(UModel::GetCollisionBoundingBox);
	return Owner ? BoundingBox.TransformBy(Owner->ToWorld()) : BoundingBox;
	unguard;
}

//
// Get render bounding box.
//
FBox UModel::GetRenderBoundingBox( const AActor* Owner, UBOOL Exact ) const
{
	guard(UModel::GetRenderBoundingBox);
	if( !Owner )
	{
		return BoundingBox;
	}
	else if( !Exact || !Polys )
	{
		return BoundingBox.TransformBy(Owner->ToWorld());
	}
	else
	{
		FCoords C(Owner->ToWorld());
		FBox B(0);
		for( int i=0; i<Polys->Num(); i++ )
			for( int j=0; j<Polys->Element(i).NumVertices; j++ )
				B += Polys->Element(i).Vertex[j].TransformPointBy(C);
		return B;
	}
	unguard;
}

/*---------------------------------------------------------------------------------------
   Primitive BoxCheck support.
---------------------------------------------------------------------------------------*/

//
// Info used by all hull box collision routines.
//
struct FBoxCheckInfo
{
	// Hull flags.
	enum EConvolveFlags
	{
		CV_XM = 0x01,
		CV_XP = 0x02,
		CV_YM = 0x04,
		CV_YP = 0x08,
		CV_ZM = 0x10,
		CV_ZP = 0x20,
	};

	// Hull variables.
	FCheckResult&	Hit;
	UModel&			Model;
	AActor*			Owner;
	FCoords			Coords;
	FVector			Extent;
	DWORD			ExtraFlags;
	INT				NumHulls;
	FVector			Min,Max,LocalHit;
	FLOAT			T0,T1;

	// Hull info.
	FPlane			Hulls[64];
	INT				Flags[64];
	const INT		*HullNodes;

	// Constructor.
	FBoxCheckInfo
	(
		FCheckResult&	InHit,
		UModel&			InModel,
		AActor*			InOwner,
		FVector			InExtent,
		DWORD			InExtraFlags
	)
	:	Hit				(InHit)
	,	Model			(InModel)
	,	Owner			(InOwner)
	,	Coords			(Owner ? Owner->ToWorld() : GMath.UnitCoords)
	,	Extent			(InExtent)
	,	ExtraFlags		(InExtraFlags)
	{}

	// Functions.
	void SetupHulls( const FBspNode& Node )
	{
		// Get nodes on this leaf's collision hull.
		HullNodes = &Model.LeafHulls( Node.iCollisionBound );
		for( NumHulls=0; HullNodes[NumHulls]!=INDEX_NONE && NumHulls<ARRAY_COUNT(Hulls); NumHulls++ )
		{
			FPlane& Hull = Hulls[NumHulls];
			Hull         = Model.Nodes->Element(HullNodes[NumHulls] & ~0x40000000).Plane;
			if( Owner )
				Hull = Hull.TransformPlaneByOrtho( Coords );
			if( HullNodes[NumHulls] & 0x40000000 )
				Hull = Hull.Flip();
			Flags[NumHulls]
			=	((Hull.X < 0.0) ? CV_XM : (Hull.X > 0.0) ? CV_XP : 0)
			|	((Hull.Y < 0.0) ? CV_YM : (Hull.Y > 0.0) ? CV_YP : 0)
			|	((Hull.Z < 0.0) ? CV_ZM : (Hull.Z > 0.0) ? CV_ZP : 0);
		}

		// Get precomputed maxima.
		const FLOAT *Temp = (FLOAT*)&Model.LeafHulls( Node.iCollisionBound + NumHulls + 1);
		Min.X = Temp[0]; Min.Y = Temp[1]; Min.Z = Temp[2];
		Max.X = Temp[3]; Max.Y = Temp[4]; Max.Z = Temp[5];
	}
};

// Hull edge convolution macro.
#define CONVOLVE_EDGE(a,b,c,mask) \
{ \
	if( (Or2 & (mask)) == (mask) ) \
	{ \
		FVector C1 = FVector(a,b,c)^Hulls[i]; \
		FVector C2 = FVector(a,b,c)^Hulls[j]; \
		if( (C1 | C2) > 0.001 ) \
		{ \
			FVector I,D;\
			FIntersectPlanes2( I, D, Hulls[i], Hulls[j] ); \
			FVector A = (FVector(a,b,c) ^ D).UnsafeNormal(); \
			if( (Hulls[i] | A) < 0.0 ) \
				A *= -1.0; \
			if( !ClipTo( FPlane(I,A), INDEX_NONE ) ) \
				goto NoBlock; \
		} \
	} \
}

// Collision primitive clipper.
#define CLIP_COLLISION_PRIMITIVE \
{ \
	guardSlow(CLIP_COLLISION_PRIMITIVE); \
	/* Check collision against hull planes. */ \
	FVector Hit=FVector(0,0,0); \
	for( int i=0; i<NumHulls; i++ ) \
		if( !ClipTo( Hulls[i], HullNodes[i] & ~0x40000000) ) \
			goto NoBlock; \
	\
	/* Check collision against hull extent, minus small epsilon so flat hit nodes are identified. */ \
	if( !Owner ) \
		if \
		(   !ClipTo( FPlane( 0.0, 0.0,-1.0,0.1-Min.Z), INDEX_NONE ) \
		||	!ClipTo( FPlane( 0.0, 0.0,+1.0,Max.Z+0.1), INDEX_NONE ) \
		||	!ClipTo( FPlane(-1.0, 0.0, 0.0,0.1-Min.X), INDEX_NONE ) \
		||	!ClipTo( FPlane(+1.0, 0.0, 0.0,Max.X-0.1), INDEX_NONE ) \
		||  !ClipTo( FPlane( 0.0,-1.0, 0.0,0.1-Min.Y), INDEX_NONE ) \
		||	!ClipTo( FPlane( 0.0,+1.0, 0.0,Max.Y-0.1), INDEX_NONE ) ) \
			goto NoBlock; \
	\
	/* Check collision against hull edges. */ \
	for( i=0; i<NumHulls; i++ ) \
	{ \
		for( int j=0; j<i; j++ ) \
		{ \
			/* Convolve with edge. */ \
			INT Or2 = Flags[i] | Flags[j]; \
			CONVOLVE_EDGE(1,0,0,CV_XM|CV_XP); \
			CONVOLVE_EDGE(0,1,0,CV_YM|CV_YP); \
			CONVOLVE_EDGE(0,0,1,CV_ZM|CV_ZP); \
		} \
	} \
	unguardSlow; \
}

/*---------------------------------------------------------------------------------------
   Primitive PointCheck.
---------------------------------------------------------------------------------------*/

//
// Box check worker class.
//
struct FBoxPointCheckInfo : public FBoxCheckInfo
{
	// Variables.
	const FVector		Point;
	FLOAT				BestDist;

	// Constructor.
	FBoxPointCheckInfo
	(
		FCheckResult		&InHit,
		UModel				&InModel,
		AActor*				InOwner,
		FVector				InPoint,
		FVector				InExtent,
		FLOAT				InExtraFlags
	)
	:	FBoxCheckInfo	    (InHit, InModel, InOwner, InExtent, InExtraFlags)
	,	BestDist			(100000)
	,	Point				(InPoint)
	{}

	// Functions.
	int ClipTo( const FPlane &Hull, INT Item )
	{
		FLOAT Push = FBoxPushOut( Hull, Extent );
		FLOAT Dist = Hull.PlaneDot(Point);
		if( Dist>0 && Dist<BestDist && Dist<Push )
		{
			BestDist      = Dist;
			Hit.Location  = Point + 1.02 * Hull * (Push - Dist);
			Hit.Normal    = Hull;
			Hit.Actor     = Owner;
			Hit.Primitive = &Model;
			Hit.Item      = Item;
			Hit.Time      = 0.0;
		}
		return Dist < Push;
	}
	UBOOL BoxPointCheck( INT iParent, INT iNode, UBOOL Outside )
	{
		UBOOL Result = 1;
		while( iNode != INDEX_NONE )
		{
			// Compute distance between start and end points and this node's plane.
			const  FBspNode &Node = Model.Nodes->Element( iNode );
			FPlane Plane          = Owner ? Node.Plane.TransformPlaneByOrtho(Coords) : Node.Plane;
			FLOAT  PushOut        = FBoxPushOut   ( Plane, Extent * 1.1 );
			FLOAT  Dist           = Plane.PlaneDot( Point );

			// Recurse with front.
			if( Dist > -PushOut )
				if( !BoxPointCheck( iNode, Node.iFront, Outside || Node.IsCsg(ExtraFlags) ) )
					Result = 0;

			// Loop with back.
			iParent = iNode;
			iNode   = Node.iBack;
			Outside = Outside && !Node.IsCsg(ExtraFlags);
			if( Dist > PushOut )
				goto NoBlock;
		}
		if( !Outside && Model.Nodes->Element(iParent).iCollisionBound!=INDEX_NONE )
		{
			// Reached a solid leaf, so setup hulls.
			SetupHulls(Model.Nodes->Element(iParent));

			// Clip it.
			CLIP_COLLISION_PRIMITIVE;

			// We hit.
			Result = 0;
		}
		NoBlock:;
		return Result;
	}
};

//
// See if a box with the specified collision info fits at Point.
//
// If it doesn't fit, returns 0 and sets SuggestedOutVector to a suggested
// vector which might move the point out of the surface it's colliding with.
// SuggestedOutVector is a good guess but it's not guaranteed to be correct.
//
// If it fits, returns 1.
//
UBOOL UModel::PointCheck
(
	FCheckResult&	Hit,
	AActor*			Owner,
	FVector			Location,
	FVector			Extent,
	DWORD           ExtraNodeFlags
)
{
	guard(UModel::PointCheck);

	// Perform the check.
	Hit.Normal    = FVector(0,0,0);
	Hit.Location  = Location;
	Hit.Primitive = this;
	Hit.Actor     = Owner;
	Hit.Time      = 0.0;
	UBOOL Outside = RootOutside;
	if( Nodes->Num() )
	{
		if( Extent != FVector(0,0,0) )
		{
			// Perform expensive box convolution check.
			FBoxPointCheckInfo Check( Hit, *this, Owner, Location, Extent, ExtraNodeFlags );
			Outside = Check.BoxPointCheck( 0, 0, Outside );
			check(Hit.Actor==Owner);
		}
		else
		{
			// Perform simple point check.
			INT iPrevNode = INDEX_NONE, iNode=0;
			UBOOL IsFront=0;
			FCoords Coords = Owner ? Owner->ToWorld() : GMath.UnitCoords;
			do
			{
				iPrevNode = iNode;
				const FBspNode &Node = Nodes->Element(iNode);
				IsFront = Node.Plane.TransformPlaneByOrtho(Coords).PlaneDot(Location) > 0.0;
				Outside = Node.ChildOutside( IsFront, Outside );
				iNode   = Node.iChild[IsFront];
			} while( iNode != INDEX_NONE );
			Hit.Item = iPrevNode*2 + IsFront;
		}
	}
	return Outside;
	unguard;
}

/*---------------------------------------------------------------------------------------
   LineCheck support.
---------------------------------------------------------------------------------------*/

//
// Recursive minion of UModel::LineCheck.
//
UBOOL GOutOfCorner;
UBOOL LineCheck
(
	FCheckResult&	Hit,
	UModel&			Model,
	const FCoords*	Coords,
	INT  			iHit,
	INT				iNode,
	FVector			End, 
	FVector			Start,
	UBOOL			Outside,
	DWORD			InNodeFlags
)
{
	guardSlow(LineCheck);
	while( iNode != INDEX_NONE )
	{
		const FBspNode*	Node = &Model.Nodes->Element(iNode);

		// Check side-of-plane for both points.
		FLOAT Dist1	= Coords ? Node->Plane.TransformPlaneByOrtho(*Coords).PlaneDot(Start) : Node->Plane.PlaneDot(Start);
		FLOAT Dist2	= Coords ? Node->Plane.TransformPlaneByOrtho(*Coords).PlaneDot(End)   : Node->Plane.PlaneDot(End);

		// Classify line based on both distances.
		if( Dist1 > -0.001 && Dist2 > -0.001 )
		{
			// Both points are in front.
			Outside |= Node->IsCsg(InNodeFlags & ~NF_BrightCorners);
			iNode    = Node->iFront;
		}
		else if( Dist1 < 0.001 && Dist2 < 0.001 )
		{
			// Both points are in back.
			Outside &= !Node->IsCsg(InNodeFlags & ~NF_BrightCorners);
			iNode    = Node->iBack;
		}
		else
		{
			// Line is split and guranteed to be non-parallel to plane, so TimeDenominator != 0.
			FVector Middle     = Start + (Start-End) * (Dist1/(Dist2-Dist1));
			INT     FrontFirst = Dist1 > 0.0;

			// Recurse with front part.
			if( !LineCheck( Hit, Model, Coords, iHit, Node->iChild[FrontFirst], Middle, Start, Node->ChildOutside(FrontFirst,Outside,InNodeFlags), InNodeFlags ) )
				return 0;

			// Loop with back part.
			Outside = Node->ChildOutside( 1-FrontFirst, Outside, InNodeFlags );
			iHit    = iNode;
			iNode   = Node->iChild[1-FrontFirst];
			Start   = Middle;
		}
	}
	if( !Outside )
	{
		// We have encountered the first collision.
		if( GOutOfCorner || !(InNodeFlags&NF_BrightCorners) )
		{
			Hit.Location  = Start;
			Hit.Normal    = Model.Nodes->Element(iHit).Plane;
			Hit.Primitive = &Model;
			Hit.Item      = iHit;		
		}
		else Outside=1;
	}
	else GOutOfCorner=1;
	return Outside;
	unguardSlow;
}

/*---------------------------------------------------------------------------------------
   Primitive LineCheck.
---------------------------------------------------------------------------------------*/

//
// Tracing worker class.
//
struct FBoxLineCheckInfo : public FBoxCheckInfo
{
	// Variables.
	const FVector		End;
	const FVector		Start;
	FVector				Vector;
	FLOAT				Dist;
	UBOOL				DidHit;

	// Constructor.
	FBoxLineCheckInfo
	(
		FCheckResult&	Hit,
		UModel&			InModel,
		AActor*			InOwner,
		FVector			InEnd,
		FVector			InStart,
		FVector			InExtent,
		DWORD			InExtraFlags
	)
	:	FBoxCheckInfo	(Hit, InModel, InOwner, InExtent, InExtraFlags)
	,	End				(InEnd)
	,	Start			(InStart)
	,	Vector			(InEnd-InStart)
	,	Dist			(Vector.Size())
	,	DidHit			(0)
	{}

	// Tracer.
	UBOOL ClipTo( const FPlane& Hull, INT Item )
	{
		guardSlow(ClipTo);
		FLOAT PushOut = FBoxPushOut( Hull, Extent );
		FLOAT D0      = Hull.PlaneDot(Start);
		FLOAT D1      = Hull.PlaneDot(End);

		FLOAT AdjD0 = D0-PushOut;
		if( D0>D1 && AdjD0>=-PushOut && AdjD0<0 )
			AdjD0 = 0.0;

		FLOAT T = (AdjD0)/(D0-D1);

		if     ( (D0-D1) < -0.00001       )	{ if( T < T1 ) { T1 = T; }                  }
		else if( (D0-D1) > +0.00001       ) { if( T > T0 ) { T0 = T; LocalHit = Hull; } }
		else if( D0>PushOut && D1>PushOut ) { return 0;                                 }

		return T0 < T1;
		unguardSlow;
	}
	void BoxLineCheck( INT iParent, INT iNode, UBOOL IsFront, UBOOL Outside )
	{
		guardSlow(BoxLineCheck);
		while( iNode != INDEX_NONE )
		{
			// Compute distance between start and end points and this node's plane.
			const FBspNode& Node       = Model.Nodes->Element(iNode);
			FPlane          Plane      = Owner ? Node.Plane.TransformPlaneByOrtho(Coords) : Node.Plane;
			FLOAT           D0         = Plane.PlaneDot(Start);
			FLOAT           D1         = Plane.PlaneDot(End);
			FLOAT           PushOut    = FBoxPushOut( Plane, Extent *1.1 );
			UBOOL           Use[2]     = {D0<=PushOut || D1<=PushOut, D0>=-PushOut || D1>=-PushOut};
			UBOOL           FrontFirst = D0 >= D1;

			// Traverse down nearest side then furthest side.
			if( Use[FrontFirst] )
				BoxLineCheck( iNode, Node.iChild[FrontFirst], FrontFirst, Node.ChildOutside(FrontFirst, Outside) );
			if( !Use[1-FrontFirst] )
				return;

			iParent = iNode;
			iNode   = Node.iChild[ 1-FrontFirst ];
			Outside = Node.ChildOutside( 1-FrontFirst, Outside );
			IsFront = !FrontFirst;
		}
		const FBspNode& Parent = Model.Nodes->Element(iParent);
		if( Outside==0 && Parent.iCollisionBound!=INDEX_NONE )
		{
			// Init.
			SetupHulls(Parent);
			T0       = -1.0; 
			T1       = Hit.Time;
			LocalHit = FVector(0,0,0);

			// Perform collision clipping.
			CLIP_COLLISION_PRIMITIVE;

			// See if we hit.
			if( T0>-1.0 && T0<T1 && T1>0.0 )
			{
				Hit.Time	  = T0;
				Hit.Normal	  = LocalHit;
				Hit.Actor     = Owner;
				Hit.Primitive = &Model;
				DidHit        = 1;
			}
			NoBlock:;
		}
		unguardSlow;
	}
};

//
// Try moving a collision box from Start to End and see what it collides
// with. Returns 1 if unblocked, 0 if blocked.
//
// Note: This function assumes that Start is a valid starting point which
// is not stuck in solid space (i.e. CollisionPointCheck(Start)=1). If Start is
// stuck in solid space, the returned time will be meaningless.
//
UBOOL UModel::LineCheck
(
	FCheckResult&	Hit,
	AActor*			Owner,
	FVector			End,
	FVector			Start,
	FVector			Extent,
	DWORD           ExtraNodeFlags
)
{
	guard(UModel::LineCheck);

	// Perform the trace.
	if( Nodes->Num() )
	{
		if( Extent == FVector(0,0,0) )
		{
			// Perform simple line trace.
			GOutOfCorner = 0;
			UBOOL Outside = ::LineCheck( Hit, *this, Owner ? &Owner->ToWorld() : NULL, 0, 0, End, Start, RootOutside, ExtraNodeFlags );
			if( !Outside )
			{
				FVector V       = End-Start;
				Hit.Time        = ((Hit.Location-Start)|V)/(V|V);
				Hit.Time		= Clamp( Hit.Time - 0.5f / V.Size(), 0.f, 1.f );
				Hit.Location	= Start + V * Hit.Time;
				Hit.Actor		= Owner;
				if ( Owner )
					Hit.Normal = Hit.Normal.TransformVectorBy(Owner->ToWorld());
			}
			return Outside;	
		}
		else
		{
			// Perform expensive box convolution trace.
			Hit.Time = 2.0;
			FBoxLineCheckInfo Trace( Hit, *this, Owner, End, Start, Extent, ExtraNodeFlags );
			Trace.BoxLineCheck( 0, 0, 0, RootOutside );

			// Truncate by the greater of 10% or 0.1 world units.
			if( Trace.DidHit )
			{
				Hit.Time      = Clamp( Hit.Time - ::Max(0.1f, 0.1f/Trace.Dist),0.f, 1.f );
				Hit.Location  = Start + (End-Start) * Hit.Time;
				return Hit.Time==1.0;
			}
			else return 1;
		}
	}
	else return RootOutside;
	unguard;
}

/*---------------------------------------------------------------------------------------
   Region determination.
---------------------------------------------------------------------------------------*/

//
// Figure out which zone a point is in, and return it.  A value of
// zero indicates that the point doesn't fall into any zone.
//
FPointRegion UModel::PointRegion( AZoneInfo* Zone, FVector Location ) const
{
	guard(UModel::PointRegion);
	check(Zone!=NULL);

	FPointRegion Result( Zone, INDEX_NONE, 0 );
	if( Nodes->Num() ) 
	{
		UBOOL Outside=RootOutside, IsFront=0;
		INT iNode=0, iParent=0;
		while( iNode != INDEX_NONE )
		{
			const FBspNode& Node = Nodes->Element(iNode);
			IsFront = Node.Plane.PlaneDot(Location) >= 0.0;
			Outside = Node.ChildOutside(IsFront,Outside);
			iParent = iNode;
			iNode   = Node.iChild[IsFront];
		}
		Result.iLeaf      = Nodes->Element(iParent).iLeaf[IsFront];
		Result.ZoneNumber = Nodes->NumZones ? Nodes->Element(iParent).iZone[IsFront] : 0;
		Result.Zone       = Nodes->Zones[Result.ZoneNumber].ZoneActor ? Nodes->Zones[Result.ZoneNumber].ZoneActor : Zone;
	}
	return Result;
	unguard;
}

/*---------------------------------------------------------------------------------------
   Point searching.
---------------------------------------------------------------------------------------*/

//
// Find closest vertex to a point at or below a node in the Bsp.  If no vertices
// are closer than MinRadius, returns -1.
//
static FLOAT FindNearestVertex
(
	const UModel	&Model, 
	const FVector	&SourcePoint,
	FVector			&DestPoint, 
	FLOAT			MinRadius, 
	INT				iNode, 
	INT				&pVertex
)
{
	FLOAT ResultRadius = -1.0;
	while( iNode != INDEX_NONE )
	{
		const FBspNode	*Node	= &Model.Nodes->Element(iNode);
		INT			    iBack   = Node->iBack;
		FLOAT PlaneDist = Node->Plane.PlaneDot( SourcePoint );
		if( PlaneDist>=-MinRadius && Node->iFront!=INDEX_NONE )
		{
			// Check front.
			FLOAT TempRadius = FindNearestVertex (Model,SourcePoint,DestPoint,MinRadius,Node->iFront,pVertex);
			if (TempRadius >= 0.0) {ResultRadius = TempRadius; MinRadius = TempRadius;};
		}
		if( PlaneDist>-MinRadius && PlaneDist<=MinRadius )
		{
			// Check this node's poly's vertices.
			while( iNode != INDEX_NONE )
			{
				// Loop through all coplanars.
				Node                    = &Model.Nodes->Element(iNode);
				const FBspSurf* Surf    = &Model.Surfs->Element(Node->iSurf);
				const FVector *Base	    = &Model.Points->Element(Surf->pBase);
				FLOAT TempRadiusSquared	= FDistSquared( SourcePoint, *Base );

				if( TempRadiusSquared < Square(MinRadius) )
				{
					pVertex = Surf->pBase;
					ResultRadius = MinRadius = appSqrt(TempRadiusSquared);
					DestPoint = *Base;
				}

				const FVert *VertPool = &Model.Verts->Element(Node->iVertPool);
				for (BYTE B=0; B<Node->NumVertices; B++)
				{
					const FVector *Vertex   = &Model.Points->Element(VertPool->pVertex);
					FLOAT TempRadiusSquared = FDistSquared( SourcePoint, *Vertex );
					if( TempRadiusSquared < Square(MinRadius) )
					{
						pVertex      = VertPool->pVertex;
						ResultRadius = MinRadius = appSqrt(TempRadiusSquared);
						DestPoint    = *Vertex;
					}
					VertPool++;
				}
				iNode = Node->iPlane;
			}
		}
		if( PlaneDist > MinRadius )
			break;
		iNode = iBack;
	}
	return ResultRadius;
}

//
// Find Bsp node vertex nearest to a point (within a certain radius) and
// set the location.  Returns distance, or -1.0 if no point was found.
//
FLOAT UModel::FindNearestVertex
(
	const FVector	&SourcePoint,
	FVector			&DestPoint,
	FLOAT			MinRadius, 
	INT				&pVertex
) const
{
	guard(UModel::FindNearestVertex);
	return Nodes->Num() ? ::FindNearestVertex( *this,SourcePoint,DestPoint,MinRadius,0,pVertex ) : -1.0;
	unguard;
}

/*---------------------------------------------------------------------------------------
   Bound filter precompute.
---------------------------------------------------------------------------------------*/

//
// Recursive worker function for UModel::PrecomputeSphereFilter.
//
void PrecomputeSphereFilter( UModel& Model, INT iNode, const FPlane& Sphere )
{
	while( iNode != INDEX_NONE )
	{
		FBspNode* Node = &Model.Nodes->Element( iNode );
		FLOAT     Dist = Node->Plane.PlaneDot( Sphere );

		Node->NodeFlags &= ~(NF_IsFront | NF_IsBack);
		if( Dist < -Sphere.W )
		{
			// All back.
			Node->NodeFlags |= NF_IsBack;
			iNode            = Node->iBack;
		}
		else if( Dist > Sphere.W )
		{	
			// All front.
			Node->NodeFlags |= NF_IsFront;
			iNode            = Node->iFront;
		}
		else
		{
			// Both front and back.
			if( Node->iBack != INDEX_NONE )
				PrecomputeSphereFilter( Model, Node->iBack, Sphere );
			iNode = Node->iFront;
		}
	}
}

//
// Precompute the front/back test for a bounding sphere.  Tags all nodes that
// the sphere falls into with a NF_IsBack tag (if the sphere is entirely in back
// of the node), a NF_IsFront tag (if the sphere is entirely in front of the node),
// or neither (if the sphere is split by the node).  This only affects nodes
// that the sphere falls in.  Thus, it is not necessary to perform any cleanup
// after precomputing the filter as long as you're sure the sphere completely
// encloses the object whose filter you're precomputing.
//
void UModel::PrecomputeSphereFilter( const FPlane &Sphere )
{
	guard(UModel::PrecomputeSphereFilter);

	if( Nodes->Num() )
		::PrecomputeSphereFilter( *this, 0, Sphere );

	unguard;
}

/*---------------------------------------------------------------------------------------
   The End.
---------------------------------------------------------------------------------------*/
