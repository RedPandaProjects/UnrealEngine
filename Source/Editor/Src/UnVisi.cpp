/*=============================================================================
	UnVisi.cpp: Unreal visibility computation
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Description:
	Visibility and zoining code.
=============================================================================*/

#include "EditorPrivate.h"

/*-----------------------------------------------------------------------------
	Temporary.
-----------------------------------------------------------------------------*/

// Options.
#define DEBUG_PORTALS	0	/* Debugging hull code by generating hull brush */
#define DEBUG_WRAPS		0	/* Debugging sheet wrapping code */
#define DEBUG_BADSHEETS 0   /* Debugging sheet discrepancies */
#define DEBUG_LVS       0   /* Debugging light volumes */
#define WORLD_MAX 65536.0	/* Maximum size of the world */
FPoly BuildInfiniteFPoly( UModel *Model, INT iNode );

// Thresholds.
#define VALID_SIDE         0.1   /* A normal must be at laest this long to be valid */
#define VALID_CROSS        0.001 /* A cross product can be safely normalized if this big */
#define EVOLUTE_VISIBILITY 0     /* Whether to perform view evolute visibility precomputation */

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

//
// Debugging.
//
#if DEBUG_PORTALS || DEBUG_WRAPS || DEBUG_BADSHEETS || DEBUG_LVS
	static ABrush *DEBUG_Brush;
#endif

//
// A portal.
//
class FPortal : public FPoly
{
public:
	// Variables.
	INT	    iFrontLeaf, iBackLeaf, iNode;
	FPortal *GlobalNext, *FrontLeafNext, *BackLeafNext, *NodeNext;
	BYTE	IsTesting, ShouldTest;
	INT		FragmentCount;
	INT	    iZonePortalSurf;

	// Constructor.
	FPortal( FPoly &InPoly, INT iInFrontLeaf, INT iInBackLeaf, INT iInNode, FPortal *InGlobalNext, FPortal *InNodeNext, FPortal *InFrontLeafNext, FPortal *InBackLeafNext )
	:	FPoly			(InPoly),
		iFrontLeaf		(iInFrontLeaf),
		iBackLeaf		(iInBackLeaf),
		iNode			(iInNode),
		GlobalNext		(InGlobalNext),
		NodeNext		(InNodeNext),
		FrontLeafNext	(InFrontLeafNext),
		BackLeafNext	(InBackLeafNext),
		IsTesting		(0),
		ShouldTest		(0),
		FragmentCount	(0),
		iZonePortalSurf (INDEX_NONE)
	{}
	
	// Get the leaf on the opposite side of the specified leaf.
	INT GetNeighborLeafOf( INT iLeaf )
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		if     ( iFrontLeaf == iLeaf )	return iBackLeaf;
		else							return iFrontLeaf;
	}

	// Get the next portal for this leaf in the linked list of portals.
	FLOAT Area()
	{
		guard(FPortal::Area);
		FVector Cross(0,0,0);
		for( INT i=2; i<NumVertices; i++ )
			Cross += (Vertex[i-1]-Vertex[0]) ^ (Vertex[i]-Vertex[0]);
		return Cross.Size();
		unguard;
	}
	FPortal* Next( INT iLeaf )
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		if     ( iFrontLeaf == iLeaf )	return FrontLeafNext;
		else							return BackLeafNext;
	}

	// Return this portal polygon, facing outward from leaf iLeaf.
	void GetPolyFacingOutOf( INT iLeaf, FPoly &Poly )
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		Poly = *(FPoly*)this;
		if( iLeaf == iFrontLeaf ) Poly.Reverse();
	}

	// Return this portal polygon, facing inward to leaf iLeaf.
	void GetPolyFacingInto( INT iLeaf, FPoly &Poly)
	{
		check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		Poly = *(FPoly*)this;
		if( iLeaf == iBackLeaf ) Poly.Reverse();
	}
};

//
// The visibility calculator class.
//
class FEditorVisibility
{
public:
	// Constants.
	enum {MAX_CLIPS=16384};
	enum {CLIP_BACK_FLAG=0x40000000};

	// Types.
	typedef void (FEditorVisibility::*PORTAL_FUNC)(FPoly&,INT,INT,INT,INT);

	// Variables.
	FMemMark		Mark;
	ULevel*			Level;
	UModel*			Model;
	INT				Clips[MAX_CLIPS];
	INT				NumPortals, NumLogicalLeaves;
	INT				NumClips, NumClipTests, NumPassedClips, NumUnclipped;
	INT				NumBspPortals, MaxFragments, NumZonePortals, NumZoneFragments;
	INT				Extra;
	INT				iZonePortalSurf;
	FPortal*		FirstPortal;
	UBitMatrix*		Visibility;
	FPortal**		NodePortals;
	FPortal**		LeafPortals;
	FActorLink**	LeafLights;

	// Constructor.
	FEditorVisibility( ULevel* InLevel, UModel* InModel, INT InDebug );

	// Destructor.
	~FEditorVisibility();

	// Portal functions.
	void AddPortal( FPoly &Poly, INT iFrontLeaf, INT iBackLeaf, INT iGeneratingNode, INT iGeneratingBase );
	void BlockPortal( FPoly &Poly, INT iFrontLeaf, INT iBackLeaf, INT iGeneratingNode, INT iGeneratingBase );
	void TagZonePortalFragment( FPoly &Poly, INT iFrontLeaf, INT iBackLeaf, INT iGeneratingNode, INT iGeneratingBase );
	void FilterThroughSubtree( INT Pass, INT iGeneratingNode, INT iGeneratingBase, INT iParentLeaf, INT iNode, FPoly Poly, PORTAL_FUNC Func, INT iBackLeaf );
	void MakePortalsClip( INT iNode, FPoly Poly, INT Clip, PORTAL_FUNC Func );
	void MakePortals( INT iNode );
	void AssignLeaves( INT iNode, INT Outside );
	int  ClipToMaximalSheetWrapping( FPoly &Poly, const FPoly &A, const FPoly &B, const FLOAT Sign, const FLOAT Phase );
	void CheckVolumeVisibility( const INT iSourceLeaf, const FPoly &Source, const INT iTestLeaf, const FPoly &Clip );
	int  PointToLeaf( FVector Point, INT iLeaf );
	int  ActorVisibility( AActor* Actor, INT iLeaf=INDEX_NONE, FPoly* Clipper=NULL );
	void FilterVolumetricLight( AActor* Actor, INT iNode=0, INT iParent=0, INT IsFront=0 );

	// Zone functions.
	void FormZonesFromLeaves();
	void AssignAllZones( INT iNode, int Outside );
	void BuildConnectivity();
	void BuildZoneInfo();

	// Visibility functions.
	void BspCrossVisibility( INT iFronyPortalLeaf, INT iBackPortalLeaf, INT iFrontLeaf, INT iBackLeaf, FPoly &FrontPoly, FPoly &ClipPoly, FPoly &BackPoly, INT ValidPolys, INT Pass );
	void BspVisibility( INT iNode );
	void TestVisibility();
};

/*-----------------------------------------------------------------------------
	Portal building, a simple recursive hierarchy of functions.
-----------------------------------------------------------------------------*/

//
// Tag a zone portal fragment.
//
void FEditorVisibility::TagZonePortalFragment
(
	FPoly&	Poly,
	INT	    iFrontLeaf,
	INT		iBackLeaf,
	INT		iGeneratingNode,
	INT		iGeneratingBase
)
{
	guard(FEditorVisibility::TagZonePortalFragment);

	// Add this node to the bsp as a coplanar to its generator.
	INT iNewNode = GEditor->bspAddNode( Model, iGeneratingNode, NODE_Plane, Model->Nodes->Element(iGeneratingNode).NodeFlags | NF_IsNew, &Poly );

	// Set the node's zones.
	int Backward = (Poly.Normal | Model->Nodes->Element(iGeneratingBase).Plane) < 0.0;
	Model->Nodes->Element(iNewNode).iZone[Backward^0] = iBackLeaf ==INDEX_NONE ? 0 : Model->Leaves(iBackLeaf ).iZone;
	Model->Nodes->Element(iNewNode).iZone[Backward^1] = iFrontLeaf==INDEX_NONE ? 0 : Model->Leaves(iFrontLeaf).iZone;

	unguard;
}

//
// Mark a portal as blocked.
//
void FEditorVisibility::BlockPortal
(
	FPoly&	Poly,
	INT		iFrontLeaf,
	INT		iBackLeaf,
	INT		iGeneratingNode,
	INT		iGeneratingBase
)
{
	guard(FEditorVisibility::BlockPortal);
	if( iFrontLeaf!=INDEX_NONE && iBackLeaf!=INDEX_NONE )
	{
		for( FPortal* Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
		{
			if
			(	(Portal->iFrontLeaf==iFrontLeaf && Portal->iBackLeaf==iBackLeaf )
			||	(Portal->iFrontLeaf==iBackLeaf  && Portal->iBackLeaf==iFrontLeaf) )
			{
				Portal->iZonePortalSurf = iZonePortalSurf;
				NumZoneFragments++;
			}
		}
	}
	unguard;
}

//
// Add a portal to the portal list.
//
void FEditorVisibility::AddPortal
(
	FPoly&	Poly,
	INT		iFrontLeaf,
	INT		iBackLeaf,
	INT		iGeneratingNode,
	INT		iGeneratingBase
)
{
	guard(FEditorVisibility::AddPortal);

	if( iFrontLeaf!=INDEX_NONE && iBackLeaf!=INDEX_NONE )
	{
		// Add to linked list of all portals.
		FirstPortal						= 
		LeafPortals[iFrontLeaf]			= 
		LeafPortals[iBackLeaf]			= 
		NodePortals[iGeneratingNode]	= 
			new(GMem)FPortal
			(
				Poly,
				iFrontLeaf,
				iBackLeaf,
				iGeneratingNode,
				FirstPortal,
				NodePortals[iGeneratingNode],
				LeafPortals[iFrontLeaf],
				LeafPortals[iBackLeaf]
			);
		NumPortals++;

#if DEBUG_PORTALS
		//debugf("AddPortal: %i verts",Poly.NumVertices);
		Poly.PolyFlags |= PF_NotSolid;
		DEBUG_Brush->Brush->Polys->AddItem(Poly);
#endif
	}
	unguard;
}

//
// Filter a portal through a front or back subtree.
//
void FEditorVisibility::FilterThroughSubtree
(
	INT			Pass,
	INT			iGeneratingNode,
	INT			iGeneratingBase,
	INT			iParentLeaf,
	INT			iNode,
	FPoly		Poly,
	PORTAL_FUNC Func,
	INT			iBackLeaf
)
{
	guard(FEditorVisibility::FilterThroughSubtree);
	while( iNode != INDEX_NONE )
	{
		// If overflow.
		if( Poly.NumVertices > FPoly::VERTEX_THRESHOLD )
		{
			FPoly Half;
			Poly.SplitInHalf( &Half );
			FilterThroughSubtree( Pass, iGeneratingNode, iGeneratingBase, iParentLeaf, iNode, Half, Func, iBackLeaf );
		}

		// Test split.
		FPoly Front,Back;
		int Split = Poly.SplitWithNode( Model, iNode, &Front, &Back, 1 );

		// Recurse with front.
		if( Split==SP_Front || Split==SP_Split )
			FilterThroughSubtree
			(
				Pass,
				iGeneratingNode,
				iGeneratingBase,
				Model->Nodes->Element(iNode).iLeaf[1],
				Model->Nodes->Element(iNode).iFront,
				Split==SP_Front ? Poly : Front,
				Func,
				iBackLeaf
			);

		// Consider back.
		if( Split!=SP_Back && Split!=SP_Split )
			return;

		// Loop with back.
		if( Split == SP_Split )
			Poly = Back;
		iParentLeaf = Model->Nodes->Element(iNode).iLeaf[0];
		iNode       = Model->Nodes->Element(iNode).iBack;
	}

	// We reached a leaf in this subtree.
	if( Pass == 0 ) FilterThroughSubtree
	(
		1,
		iGeneratingNode,
		iGeneratingBase,
		Model->Nodes->Element(iGeneratingBase).iLeaf[1],
		Model->Nodes->Element(iGeneratingBase).iFront,
		Poly,
		Func,
		iParentLeaf
	);
	else (this->*Func)( Poly, iParentLeaf, iBackLeaf, iGeneratingNode, iGeneratingBase );
	unguard;
}

//
// Clip a portal by all parent nodes above it.
//
void FEditorVisibility::MakePortalsClip
(
	INT			iNode,
	FPoly		Poly,
	INT			Clip,
	PORTAL_FUNC Func
)
{
	guard(FEditorVisibility::MakePortalsClip);

	// Clip by all parents.
	while( Clip < NumClips )
	{
		INT		 iClipNode = Clips[Clip] & ~CLIP_BACK_FLAG;
		FBspNode &Node     = Model->Nodes->Element(iClipNode);

		// Subdivide if poly vertices overflow.
		if( Poly.NumVertices >= FPoly::VERTEX_THRESHOLD )
		{
			FPoly TempPoly;
			Poly.SplitInHalf( &TempPoly );
			MakePortalsClip( iNode, TempPoly, Clip, Func );
		}

		// Split by parent.
		FPoly Front,Back;
		int Split = Poly.SplitWithNode(Model,iClipNode,&Front,&Back,1);

		// Make sure we generated a useful fragment.
		if(	(Split==SP_Front &&  (Clips[Clip] & CLIP_BACK_FLAG) )
		||	(Split==SP_Back  && !(Clips[Clip] & CLIP_BACK_FLAG) )
		||	(Split==SP_Coplanar))
		{
			// Clipped to oblivion, or useless coplanar.
			return;
		}

		if( Split==SP_Split )
		{
			// Keep the appropriate piece.
			Poly = (Clips[Clip] & CLIP_BACK_FLAG) ? Back : Front;
		}

		// Clip by next parent.
		Clip++;
	}

	// Filter poly down the back subtree.
	FilterThroughSubtree
	(
		0,
		iNode,
		iNode,
		Model->Nodes->Element(iNode).iLeaf[0],
		Model->Nodes->Element(iNode).iBack,
		Poly,
		Func,
		INDEX_NONE
	);
	unguard;
}

//
// Make all portals.
//
void FEditorVisibility::MakePortals( INT iNode )
{
	guard(FEditorVisibility::MakePortals);
	INT iOriginalNode = iNode;

	// Make an infinite edpoly for this node.
	FPoly Poly = BuildInfiniteFPoly( Model, iNode );

	// Filter the portal through this subtree.
	MakePortalsClip( iNode, Poly, 0, &FEditorVisibility::AddPortal);

	// Make portals for front.
	if( Model->Nodes->Element(iNode).iFront != INDEX_NONE )
	{
		Clips[NumClips++] = iNode;
		MakePortals( Model->Nodes->Element(iNode).iFront );
		NumClips--;
	}

	// Make portals for back.
	if( Model->Nodes->Element(iNode).iBack != INDEX_NONE )
	{
		Clips[NumClips++] = iNode | CLIP_BACK_FLAG;
		MakePortals( Model->Nodes->Element(iNode).iBack );
		NumClips--;
	}

	// For all zone portals at this node, mark the matching FPortals as blocked.
	while( iNode != INDEX_NONE )
	{
		FBspNode& Node = Model->Nodes->Element( iNode      );
		FBspSurf& Surf = Model->Surfs->Element( Node.iSurf );
		if( (Surf.PolyFlags & PF_Portal) && GEditor->bspNodeToFPoly( Model, iNode, &Poly ) )
		{
			NumZonePortals++;
			iZonePortalSurf = Node.iSurf;
			FilterThroughSubtree
			(
				0,
				iNode,
				iOriginalNode,
				Model->Nodes->Element(iOriginalNode).iLeaf[0],
				Model->Nodes->Element(iOriginalNode).iBack,
				Poly,
				&FEditorVisibility::BlockPortal,
				INDEX_NONE
			);
		}
		iNode = Node.iPlane;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Assign leaves.
-----------------------------------------------------------------------------*/

//
// Assign contiguous unique numbers to all front and back leaves in the BSP.
// Stores the leaf numbers in FBspNode::iLeaf[2].
//
void FEditorVisibility::AssignLeaves( INT iNode, INT Outside )
{
	guard(FEditorVisibility::AssignLeaves);

	FBspNode &Node = Model->Nodes->Element(iNode);
	for( int IsFront=0; IsFront<2; IsFront++ )
	{
		if( Node.iChild[IsFront] != INDEX_NONE )
		{
			AssignLeaves( Node.iChild[IsFront], Node.ChildOutside( IsFront, Outside, NF_NotVisBlocking ) );
		}
		else if( Node.ChildOutside( IsFront, Outside, NF_NotVisBlocking ) )
		{
			Node.iLeaf[IsFront] = Model->Leaves.AddItem( FLeaf(Model->Leaves.Num(), INDEX_NONE, INDEX_NONE, ~(QWORD)0) );
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Point visibility tests.
-----------------------------------------------------------------------------*/

//
// Recursively build a list of leaves visible from a point.
// Uses a recursive shadow volume clipper.
//
INT FEditorVisibility::ActorVisibility
(
	AActor*	Actor,
	INT	    iLeaf,
	FPoly*	Clipper
)
{
	guard(FEditorVisibility::ActorVisibility);
	FPortal* Portal=NULL;

	// If leaf not specified, find the leaf corresponding to the point.
	if( iLeaf == INDEX_NONE )
	{
		INT iNode=0, iParent=0, IsFront=0;
		while( iNode != INDEX_NONE )
		{
			IsFront = (Model->Nodes->Element(iNode).Plane.PlaneDot(Actor->Location) > 0.0);
			iParent = iNode;
			iNode   = Model->Nodes->Element(iNode).iChild[IsFront];
		}
		iLeaf = Model->Nodes->Element(iParent).iLeaf[IsFront];
		if( iLeaf == INDEX_NONE )
			return 0;
		goto DistanceOk;
	}

#if DEBUG_LVS
	if( Clipper )
	{
		Clipper->PolyFlags |= PF_NotSolid;
		DEBUG_Brush->Polys->AddItem( *Clipper );
	}
#endif

	// If all of this leaves' portals' vertices are too far from the lightsource,
	// the light doesn't permeate this leaf. !!not always correct
	for( Portal=LeafPortals[iLeaf]; Portal!=NULL; Portal=Portal->Next(iLeaf) )
	{
		FPoly Poly;
		Portal->GetPolyFacingOutOf( iLeaf, Poly );
		for( int i=0; i<Poly.NumVertices; i++ )
			if( (Poly.Vertex[i]-Actor->Location).SizeSquared() < Square(Actor->WorldLightRadius()) )
				goto DistanceOk;
	}
	return 0;
	DistanceOk:;

	// Add this actor to the permeated leaf if it's not already there.
	int Count = 0;
	for( FActorLink* Link=LeafLights[iLeaf]; Link; Link=Link->Next )
		if( Link->Actor == Actor )
			break;
	if( !Link )
	{
		LeafLights[iLeaf] = new FActorLink( Actor, LeafLights[iLeaf] );
		Count++;
	}

	// Recursively check leaves on other side.
	for( Portal=LeafPortals[iLeaf]; Portal!=NULL; Portal=Portal->Next(iLeaf) )
	{
		FPoly Poly;
		Portal->GetPolyFacingOutOf( iLeaf, Poly );
		FLOAT PlaneDot = (Actor->Location - Poly.Base) | Poly.Normal;
		if( PlaneDot<0.0 && PlaneDot > -Actor->WorldLightRadius() )
		{
			INT iOtherLeaf = Portal->GetNeighborLeafOf( iLeaf );
			if( Clipper )
			{
				// Clip Poly by the specified clipping polygon.
				for( int i=0,j=Clipper->NumVertices-1; i<Clipper->NumVertices; j=i++ )
				{
					if( Poly.NumVertices >= FPoly::VERTEX_THRESHOLD )
						break;
					FPoly Front, Back;
					int Split = Poly.SplitWithPlaneFast( FPlane(Actor->Location,Clipper->Vertex[i],Clipper->Vertex[j]), &Front, &Back );
					if( Split == SP_Back )
						goto Oblivion;
					else if( Split == SP_Split )
						Poly = Front;
				}
			}
			if( Poly.NumVertices > 0 )
				Count += ActorVisibility( Actor, iOtherLeaf, &Poly );
			Oblivion:;
		}
	}
	return Count;
	unguard;
}

/*-----------------------------------------------------------------------------
	Volume visibility: maximal sheet wrappings.
-----------------------------------------------------------------------------*/

//
// Clip Test to the maximal sheet wrapping of Source and Clip, and store
// the result in Result.  Returns 1 if the result is nonempty, 0 if the
// polygon was clipped to oblivion.
//
int FEditorVisibility::ClipToMaximalSheetWrapping
(
	FPoly&			Poly,
	const FPoly&	A,
	const FPoly&	B,
	const FLOAT		Sign,
	const FLOAT		Phase
)
{
	guard(FEditorVisibility::ClipToMaximalSheetWrapping);
	NumClipTests++;
	int OriginalVerts=Poly.NumVertices;

	// Traverse all sides of A, with vertex indices iA0, iA1.
	for( int iA1=0,iA0=A.NumVertices-1; iA1<A.NumVertices; iA0=iA1++ )
	{
		// Make vector for measuring furthest cutting point.
		FVector Side = A.Vertex[iA1]-A.Vertex[iA0];
		FLOAT SideSquared = Side.SizeSquared();
		if( SideSquared < Square(VALID_SIDE) )
			continue;

		// Find vertex of B such that the plane formed by it and the first
		// two vertices of A cleanly partitions A and B.
		INT iB0 = 0;
		FVector RefNormal;
		for( iB0=0; iB0<B.NumVertices; iB0++ )
		{
			// Compute normal, and don't clip if we don't have enough precision to clip properly.
			FVector Path = B.Vertex[iB0] - A.Vertex[iA0];
			FLOAT PathSquared = Path.SizeSquared();
			if( PathSquared < Square(VALID_SIDE) )
				continue;

			RefNormal = Side ^ Path;
			FLOAT NormalSquared = RefNormal.SizeSquared();
			if( NormalSquared < Square(VALID_CROSS)*SideSquared*PathSquared )
				continue;

			RefNormal *= Sign / appSqrt(NormalSquared);

			// Test B split to make sure the logic is ok.
			//static const char *Sp[4]={"Plane","Front","Back ","Split"};
			FPoly BB=B; int BSplit = BB.SplitWithPlane(A.Vertex[iA0],Phase*RefNormal,NULL,NULL,1);
			if( BSplit != SP_Back )
			{
				//debugf("Visi: B=%s -- %f : %f/%f",Sp[BSplit],(B.Base-A.Base)|A.Normal,Phase,Sign);
				continue;
			}

			/*
			debugf("Range = %f < %f > %f",Best,Phase * (B.Vertex[iB0] | MyRef),Max);
			if( iB0==iMAX ) debugf("%f %f Max",Sign,Phase);
			else if( iB0==iMIN ) debugf("%f %f Min",Sign,Phase);
			else debugf("%f %f Other",Sign,Phase);
			*/

			// Clip Poly by plane.
			if( !Poly.Split( A.Vertex[iA0], RefNormal, 1 ) )
			{
				// If back, the poly was clipped to oblivion.
				return 0;
			}
			break;
		}
	}
	NumPassedClips++;
	NumUnclipped += OriginalVerts==Poly.NumVertices;
	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Bsp volume visibility.
-----------------------------------------------------------------------------*/

//
// Recursive cross-Bsp visibility.
//
void FEditorVisibility::BspCrossVisibility
(
	INT	    iFrontPortalLeaf,
	INT     iBackPortalLeaf,
	INT     iFrontLeaf,
	INT     iBackLeaf,
	FPoly&	FrontPoly,
	FPoly&	ClipPoly,
	FPoly&	BackPoly,
	INT		ValidPolys,
	INT		Pass
)
{
	guard(FEditorVisibility::BspCrossVisibility);

	// Clip FrontPoly and BackPoly to each others' visibility volumes through ClipPoly.
	FPoly ClippedFrontPoly = FrontPoly;
	FPoly ClippedBackPoly  = BackPoly;
	if
	(	(ValidPolys < 3)
	||	(	ClipToMaximalSheetWrapping( ClippedBackPoly,  ClipPoly,  FrontPoly, +1.0, +1.0 )
		&&	ClipToMaximalSheetWrapping( ClippedBackPoly,  FrontPoly, ClipPoly,  +1.0, -1.0 )
		&&	ClipToMaximalSheetWrapping( ClippedFrontPoly, BackPoly,  ClipPoly,  -1.0, -1.0 ) 
		&&	ClipToMaximalSheetWrapping( ClippedFrontPoly, ClipPoly,  BackPoly,  -1.0, +1.0 ) ) )
	{
		// Note that front leaf sees back leaf (a precondition of this routine being called).
		Visibility->Set( iFrontLeaf, iBackLeaf, 1 );

		// Recurse down the front.
		if( Pass == 0 ) for( FPortal *NewFrontPortal=LeafPortals[iFrontLeaf]; NewFrontPortal; NewFrontPortal=NewFrontPortal->Next(iFrontLeaf) )
		{
			if( !NewFrontPortal->IsTesting )
			{
				INT iNewFrontLeaf = NewFrontPortal->GetNeighborLeafOf(iFrontLeaf);
				if( !Visibility->Get(iNewFrontLeaf, iFrontPortalLeaf) )
					continue;

				FPoly NewFrontPoly;
				NewFrontPortal->GetPolyFacingOutOf( iFrontLeaf, NewFrontPoly );
				if( ClipPoly.Faces( NewFrontPoly ) && BackPoly.Faces( NewFrontPoly ) )
				{
					NewFrontPortal->IsTesting++;
					BspCrossVisibility
					(
						iFrontPortalLeaf,
						iBackPortalLeaf,
						iNewFrontLeaf,
						iBackLeaf,
						NewFrontPoly,
						ClippedFrontPoly,
						ClippedBackPoly,
						ValidPolys+1,
						0
					);
					NewFrontPortal->IsTesting--;
				}
			}
			else NewFrontPortal->FragmentCount++;
		}

		// Recurse down the back.
		for( FPortal* NewBackPortal=LeafPortals[iBackLeaf]; NewBackPortal; NewBackPortal=NewBackPortal->Next(iBackLeaf) )
		{
			if( !NewBackPortal->IsTesting )
			{
				INT iNewBackLeaf = NewBackPortal->GetNeighborLeafOf(iBackLeaf);
				if( !Visibility->Get( iNewBackLeaf, iBackPortalLeaf ) )
					continue;

				FPoly NewBackPoly;
				NewBackPortal->GetPolyFacingInto( iBackLeaf, NewBackPoly );
				if( NewBackPoly.Faces( ClipPoly ) && NewBackPoly.Faces( FrontPoly ) )
				{
					NewBackPortal->IsTesting++;
					BspCrossVisibility
					(
						iFrontPortalLeaf,
						iBackPortalLeaf,
						iFrontLeaf,
						iNewBackLeaf,
						ClippedFrontPoly,
						ClippedBackPoly,
						NewBackPoly,
						ValidPolys+1,
						1
					);
					NewBackPortal->IsTesting--;
				}
			}
			else NewBackPortal->FragmentCount++;
		}
	}
	unguard;
}

//
// Recursive main Bsp visibility.
//
void FEditorVisibility::BspVisibility( INT iNode )
{
	guard(FEditorVisibility::BspVisibility);
	FBspNode& Node = Model->Nodes->Element(iNode);
	INT FragmentCount = 0;

	// Mark this node's portals as partitioners.
	for( FPortal* ClipPortal = NodePortals[iNode]; ClipPortal; ClipPortal=ClipPortal->NodeNext )
		ClipPortal->IsTesting++;

	// Recurse, so that we can use intersubtree visibility to reject intrasubtree
	// visibility calculations.
	for( int i=0; i<2; i++ )
		if( Node.iChild[i]!=INDEX_NONE )
			BspVisibility( Node.iChild[i] );

	// Test all portals at this node.
	for( ClipPortal=NodePortals[iNode]; ClipPortal; ClipPortal=ClipPortal->NodeNext )
	{
		GSystem->StatusUpdatef( NumBspPortals, NumPortals, "Convolving %i/%i", NumBspPortals, NumPortals );
		BspCrossVisibility
		(
			ClipPortal->iFrontLeaf,
			ClipPortal->iBackLeaf,
			ClipPortal->iFrontLeaf,
			ClipPortal->iBackLeaf,
			*ClipPortal,
			*ClipPortal,
			*ClipPortal,
			1,
			0
		);
		NumBspPortals++;
		FragmentCount += ClipPortal->FragmentCount;
	}

	// Unmark testing.
	for( ClipPortal = NodePortals[iNode]; ClipPortal; ClipPortal=ClipPortal->NodeNext )
		ClipPortal->IsTesting--;

	//debugf("Node %i: %i fragments",iNode,FragmentCount);
	MaxFragments = Max(MaxFragments,FragmentCount);
	unguard;
}

/*-----------------------------------------------------------------------------
	Volume visibility check.
-----------------------------------------------------------------------------*/

//
// Recursively check for visibility starting at the source portal Source
// in leaf iSourceLeaf, flowing through the clip portal Clip, and terminating 
// in all immediate neighbor leaves of ClipLeaf.
//
void FEditorVisibility::CheckVolumeVisibility
(
	const INT		iSourceLeaf,
	const FPoly&	Source,
	const INT		iTestLeaf,
	const FPoly&	Clip
)
{
	guard(FEditorVisibility::CheckVolumeVisibility);
	check(iSourceLeaf != iTestLeaf);

	// Note that SourceLeaf can see TestLeaf.
	Visibility->Set( iSourceLeaf, iTestLeaf, 1 );

	// Check each portal in test leaf.
	for( FPortal* TestPortal = LeafPortals[iTestLeaf]; TestPortal; TestPortal=TestPortal->Next(iTestLeaf) )
	{
		// Get TestPortal and leaf on far side of TestPortal.
		INT iNewTestLeaf = TestPortal->GetNeighborLeafOf(iTestLeaf);

		// Don't recurse on portals we're already testing.
		if
		(	!TestPortal->IsTesting
		&&	iNewTestLeaf!=iSourceLeaf  )
		{
			// Get outward-facing Test polygon.
			FPoly Test;
			TestPortal->GetPolyFacingOutOf( iTestLeaf, Test );

			// Only test if facing.
			if( Source.Faces(Test) && Clip.Faces(Test) )
			{
				// Create ClippedSource by clipping Source to the maximal sheet wrapping
				// volume of Clip and Test.
				FPoly ClippedSource = Source;
				FPoly NewClip		= Test;
				if
				(	ClipToMaximalSheetWrapping( ClippedSource, Clip,   Test,   +1.0, +1.0 )
				&&	ClipToMaximalSheetWrapping( ClippedSource, Test,   Clip,   +1.0, -1.0 )
				&&	ClipToMaximalSheetWrapping( NewClip,       Source, Clip,   -1.0, -1.0 ) 
				&&	ClipToMaximalSheetWrapping( NewClip,       Clip,   Source, -1.0, +1.0 ) )
				{
#if DEBUG_WRAPS
					ClippedSource.PolyFlags |= PF_NotSolid;
					DEBUG_Brush->Polys->AddItem(ClippedSource);

					NewClip.PolyFlags |= PF_NotSolid;
					DEBUG_Brush->Polys->AddItem(NewClip);
#endif
					if( iNewTestLeaf != iSourceLeaf )
					{
						// Recursively check visibility.
						TestPortal->IsTesting++;
						CheckVolumeVisibility( iSourceLeaf, ClippedSource, iNewTestLeaf, NewClip );
						TestPortal->IsTesting--;
					}
				}
			}
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Zoning.
-----------------------------------------------------------------------------*/

//
// Form zones from the leaves.
//
void FEditorVisibility::FormZonesFromLeaves()
{
	guard(FEditorVisibility::FormZonesFromLeaves);
	FMemMark Mark(GMem);

	// Go through all portals and merge the adjoining zones.
	for( FPortal* Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
	{
		if( Portal->iZonePortalSurf==INDEX_NONE )//!!&& Abs(Portal->Area())>10.0 )
		{
			INT Original = Model->Leaves(Portal->iFrontLeaf).iZone;
			INT New      = Model->Leaves(Portal->iBackLeaf ).iZone;
			for( INT i=0; i<Model->Leaves.Num(); i++ )
			{
				if( Model->Leaves(i).iZone == Original )
					Model->Leaves(i).iZone = New;
			}
		}
	}
	
	// Renumber the leaves.
	INT NumZones=0;
	for( INT i=0; i<Model->Leaves.Num(); i++ )
	{
		if( Model->Leaves(i).iZone >= NumZones )
		{
			for( int j=i+1; j<Model->Leaves.Num(); j++ )
				if( Model->Leaves(j).iZone == Model->Leaves(i).iZone )
					Model->Leaves(j).iZone = NumZones;
			Model->Leaves(i).iZone = NumZones++;
		}
	}
	debugf( NAME_Log, "Found %i zones", NumZones );

	// Confine the zones to 1-63.
	for( i=0; i<Model->Leaves.Num(); i++ )
		Model->Leaves(i).iZone = (Model->Leaves(i).iZone % 63) + 1;

	// Set official zone count.
	Model->Nodes->NumZones = Clamp(NumZones+1,1,64);

	Mark.Pop();
	unguard;
}

/*-----------------------------------------------------------------------------
	Assigning zone numbers.
-----------------------------------------------------------------------------*/

//
// Go through the Bsp and assign zone numbers to all nodes.  Prior to this
// function call, only leaves have zone numbers.  The zone numbers for the entire
// Bsp can be determined from leaf zone numbers.
//
void FEditorVisibility::AssignAllZones( INT iNode, int Outside )
{
	guard(FEditorVisibility::AssignAllZones);
	INT iOriginalNode = iNode;

	// Recursively assign zone numbers to children.
	if( Model->Nodes->Element(iOriginalNode).iFront != INDEX_NONE )
		AssignAllZones( Model->Nodes->Element(iOriginalNode).iFront, Outside || Model->Nodes->Element(iOriginalNode).IsCsg(NF_NotVisBlocking) );
	
	if( Model->Nodes->Element(iOriginalNode).iBack != INDEX_NONE )
		AssignAllZones( Model->Nodes->Element(iOriginalNode).iBack, Outside && !Model->Nodes->Element(iOriginalNode).IsCsg(NF_NotVisBlocking) );

	// Make sure this node's polygon resides in a single zone.  In other words,
	// find all of the zones belonging to outside Bsp leaves and make sure their
	// zone number is the same, and assign that zone number to this node.
	while( iNode != INDEX_NONE )
	{
		FPoly Poly;
		if( !(Model->Nodes->Element(iNode).NodeFlags & NF_IsNew) && GEditor->bspNodeToFPoly( Model, iNode, &Poly ) )
		{
			// Make sure this node is added to the BSP properly.
			int OriginalNumNodes = Model->Nodes->Num();
			FilterThroughSubtree
			(
				0,
				iNode,
				iOriginalNode,
				Model->Nodes->Element(iOriginalNode).iLeaf [0],
				Model->Nodes->Element(iOriginalNode).iChild[0],
				Poly,
				&FEditorVisibility::TagZonePortalFragment,
				INDEX_NONE
			);

			// See if all of all non-interior added fragments are in the same zone.
			if( Model->Nodes->Num() > OriginalNumNodes )
			{
				int CanMerge=1, iZone[2]={0,0};
				for( int i=OriginalNumNodes; i<Model->Nodes->Num(); i++ )
					for( int j=0; j<2; j++ )
						if( Model->Nodes->Element(i).iZone[j] != 0 )
							iZone[j] = Model->Nodes->Element(i).iZone[j];
				for( i=OriginalNumNodes; i<Model->Nodes->Num(); i++ )
					for( int j=0; j<2; j++ )
						if( Model->Nodes->Element(i).iZone[j]!=0 && Model->Nodes->Element(i).iZone[j]!=iZone[j] )
							CanMerge=0;
				if( CanMerge )
				{
					// All fragments were in the same zone, so keep the original and discard the new fragments.
					for( i=OriginalNumNodes; i<Model->Nodes->Num(); i++ )
						Model->Nodes->Element(i).NumVertices = 0;
					for( i=0; i<2; i++ )
						Model->Nodes->Element(iNode).iZone[i] = iZone[i];
				}
				else
				{
					// Keep the multi-zone fragments and remove the original plus any interior unnecessary polys.
					Model->Nodes->Element(iNode).NumVertices = 0;
					for( i=OriginalNumNodes; i<Model->Nodes->Num(); i++ )
						if( Model->Nodes->Element(i).iZone[0]==0 && Model->Nodes->Element(i).iZone[1]==0 )
							Model->Nodes->Element(i).NumVertices = 0;
				}
			}
		}
		iNode = Model->Nodes->Element(iNode).iPlane;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Bsp zone structure building.
-----------------------------------------------------------------------------*/

//
// Build a 64-bit zone mask for each node, with a bit set for every
// zone that's referenced by the node and its children.  This is used
// during rendering to reject entire sections of the tree when it's known
// that none of the zones in that section are active.
//
QWORD BuildZoneMasks( UModel* Model, INT iNode )
{
	guard(FEditorVisibility::BuildZoneMasks);

	FBspNode& Node = Model->Nodes->Element(iNode);
	QWORD ZoneMask = 0;

	if( Node.iZone[0]!=0 ) ZoneMask |= ((QWORD)1) << Node.iZone[0];
	if( Node.iZone[1]!=0 ) ZoneMask |= ((QWORD)1) << Node.iZone[1];

	if( Node.iFront != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node.iFront );
	if( Node.iBack  != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node.iBack );
	if( Node.iPlane != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node.iPlane );

	Node.ZoneMask = ZoneMask;

	return ZoneMask;
	unguard;
}

//
// Build 64x64 zone connectivity matrix.  Entry(i,j) is set if node i is connected
// to node j.  Entry(i,i) is always set by definition.  This structure is built by
// analyzing all portals in the world and tagging the two zones they connect.
//
// Called by: TestVisibility.
//
void FEditorVisibility::BuildConnectivity()
{
	guard(FEditorVisibility::BuildConnectivity);

	for( int i=0; i<64; i++ )
	{
		// Init to identity.
		Model->Nodes->Zones[i].Connectivity = ((QWORD)1)<<i;
	}
	for( i=0; i<Model->Nodes->Num(); i++ )
	{
		// Process zones connected by portals.
		FBspNode &Node = Model->Nodes->Element(i);
		FBspSurf &Surf = Model->Surfs->Element(Node.iSurf);

		if( Surf.PolyFlags & PF_Portal )
		{
			Model->Nodes->Zones[Node.iZone[1]].Connectivity |= ((QWORD)1) << Node.iZone[0];
			Model->Nodes->Zones[Node.iZone[0]].Connectivity |= ((QWORD)1) << Node.iZone[1];
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Zone info assignment.
-----------------------------------------------------------------------------*/

//
// Attach ZoneInfo actors to the zones that they belong in.
// ZoneInfo actors are a class of actor which level designers may
// place in UnrealEd in order to specify the properties of the zone they
// reside in, such as water effects, zone name, etc.
//
void FEditorVisibility::BuildZoneInfo()
{
	guard(FEditorVisibility::BuildZoneInfo);
	int Infos=0, Duplicates=0, Zoneless=0;
	GSystem->StatusUpdatef( 0, 0, "Computing zones" );

	for( int i=0; i<UBspNodes::MAX_ZONES; i++ )
	{
		// By default, the LevelInfo (actor 0) acts as the ZoneInfo
		// for all zones which don't have individual ZoneInfo's.
		Model->Nodes->Zones[i].ZoneActor = NULL;
	}
	for( INT iActor=0; iActor<Level->Num(); iActor++ )
	{
		if( Level->Actors(iActor) )
			Level->Actors(iActor)->Region = FPointRegion( Level->GetLevelInfo(), INDEX_NONE, 0 );
	}
	for( iActor=0; iActor<Level->Num(); iActor++ )
	{
		AZoneInfo* Actor = Cast<AZoneInfo>( Level->Actors(iActor) );
		if( Actor && !Actor->IsA(ALevelInfo::StaticClass) )
		{
			Actor->Region = Model->PointRegion( Level->GetLevelInfo(), Actor->Location );
			if( Actor->Region.ZoneNumber == 0 )
			{
				Zoneless++;
			}
			else if( Model->Nodes->Zones[Actor->Region.ZoneNumber].ZoneActor )
			{
				Duplicates++;
			}
			else
			{
				// Associate zone info.
				Infos++;
				Model->Nodes->Zones[Actor->Region.ZoneNumber].ZoneActor = (AZoneInfo*)Actor;

				// Perform aural reverb raytracing.
				if( Actor->bReverbZone && Actor->bRaytraceReverb )
				{
					// Sample a bunch of random oriented vectors to get a feeling for how big the zone is.
					const int NumTraces = 256;
					const FLOAT MaxDist = 16384;
					TArray<FVector> Samples;
					for( int i=0; i<NumTraces; i++ )
					{
						FCheckResult Hit(1.0);
						Model->LineCheck( Hit, NULL, Actor->Location + MaxDist*VRand(), Actor->Location, FVector(0,0,0), 0 );
						debugf( NAME_Log, "   dist=%f",Hit.Time * MaxDist );
						new(Samples)FVector(Hit.Time * MaxDist / Actor->SpeedOfSound,1,0);
					}

					// Quantize the reverb times.
					while( Samples.Num() > ARRAY_COUNT(Actor->Delay) )
					{
						// Find best pair to merge.
						INT iBest=0, jBest=1;
						FLOAT BestCost=10000000000000.0;
						for( int i=0; i<Samples.Num(); i++ )
						{
							for( int j=0; j<i; j++ )
							{
								FLOAT Cost = Square(Samples(i).X-Samples(j).X) * (Samples(i).Y + Samples(j).Y);
								if( Cost < BestCost )
								{
									BestCost = Cost;
									iBest    = i;
									jBest    = j;
								}
							}
						}

						// Merge them.
						FVector A = Samples(iBest);
						FVector B = Samples(jBest);
						Samples(iBest) = FVector( (A.X*A.Y + B.X*B.Y)/(A.Y+B.Y), A.Y+B.Y, 0 );
						Samples.Remove(jBest);
					}

					// Sort them.
					for( i=0; i<Samples.Num(); i++ )
						for( int j=0; j<i; j++ )
							if( Samples(i).X < Samples(j).X )
								Exchange( Samples(i), Samples(j) );

					// Copy reverb times to the zone info.
					for( i=0; i<Samples.Num(); i++ )
					{
						Actor->Delay[i] = Clamp( Samples(i).X*1000.0, 0.0, 255.0 );
						Actor->Gain [i] = Clamp( Samples(i).Y*255.0/NumTraces, 0.0, 255.0 );
					}
				}

				// Handle warp zones.
				if
				(	Actor->IsA(AWarpZoneInfo::StaticClass)
				&&	Actor->Region.iLeaf!=INDEX_NONE
				&&	LeafPortals[Actor->Region.iLeaf] 
				&&	LeafPortals[Actor->Region.iLeaf]->iZonePortalSurf != INDEX_NONE )
				{
					// Get warp info.
					AWarpZoneInfo* Warp   = (AWarpZoneInfo*)Actor;
					FPortal*       Portal = LeafPortals[Actor->Region.iLeaf];

					// Find center point of surface for matching up identically-sized portals.
					FBox Box(0);
					for( int i=0; i<Model->Nodes->Num(); i++ )
						if( Model->Nodes->Element(i).iSurf == Portal->iZonePortalSurf )
							for( int j=0; j<Model->Nodes->Element(i).NumVertices; j++ )
								Box += Model->Points->Element( Model->Verts->Element( j + Model->Nodes->Element(i).iVertPool ).pVertex );

					// Establish warp coordinate system.
					FBspSurf& Surf = Model->Surfs->Element( Portal->iZonePortalSurf );

					Warp->WarpCoords.Origin	= (Box.Min/2 + Box.Max/2) - Portal->Normal * ((Box.Min/2 + Box.Max/2 - Portal->Vertex[0]) | Portal->Normal);
					Warp->WarpCoords.ZAxis  = Model->Vectors->Element(Surf.vNormal  ).SafeNormal();
					Warp->WarpCoords.XAxis  = Model->Vectors->Element(Surf.vTextureU).SafeNormal();
					Warp->WarpCoords.YAxis  = Model->Vectors->Element(Surf.vTextureV).SafeNormal();

					// Determine far zone.
					Warp->iWarpZone	= Model->Leaves(Portal->GetNeighborLeafOf(Actor->Region.iLeaf)).iZone;
				}
			}
		}
	}
	for( iActor=0; iActor<Level->Num(); iActor++ )
		if( Level->Actors(iActor) )
			Level->SetActorZone( Level->Actors(iActor), 1, 1 );
	debugf( NAME_Log, "BuildZoneInfo: %i ZoneInfo actors, %i duplicates, %i zoneless", Infos, Duplicates, Zoneless );
	unguard;
}

/*-----------------------------------------------------------------------------
	Find volumetric light leaves.
-----------------------------------------------------------------------------*/

//
// Find leaves hit by a volumetric lightsource.
//
void FEditorVisibility::FilterVolumetricLight( AActor* Actor, INT iNode, INT iParent, INT IsFront )
{
	if( iNode != INDEX_NONE )
	{
		FLOAT Dist = Model->Nodes->Element(iNode).Plane.PlaneDot(Actor->Location);
		if( Dist > -Actor->WorldVolumetricRadius() )
			FilterVolumetricLight( Actor, Model->Nodes->Element(iNode).iFront, iNode, 1 );
		if( Dist < Actor->WorldVolumetricRadius() )
			FilterVolumetricLight( Actor, Model->Nodes->Element(iNode).iBack,  iNode, 0 );
	}
	else if( Model->Nodes->Element(iParent).iLeaf[IsFront] != INDEX_NONE )
	{
		LeafLights[Model->Nodes->Element(iParent).iLeaf[IsFront]] = new(GMem)FActorLink( Actor, LeafLights[Model->Nodes->Element(iParent).iLeaf[IsFront]] );
	}
}

/*-----------------------------------------------------------------------------
	Volume visibility test.
-----------------------------------------------------------------------------*/

//
// Test visibility.
//
void FEditorVisibility::TestVisibility()
{
	guard(FEditorVisibility::TestVisibility);
	DOUBLE VisTime = appSeconds();
	int CountPortals=0;

	GSystem->BeginSlowTask("Zoning",1,0);

	// Init Bsp info.
	for( int i=0; i<Model->Nodes->Num(); i++ )
	{
		for( int j=0; j<2; j++ )
		{
			Model->Nodes->Element(i).iLeaf [j] = INDEX_NONE;
			Model->Nodes->Element(i).iZone [j] = 0;
		}
	}
	for( i=0; i<Model->Surfs->Num(); i++ )
		Model->Surfs->Element(i).iLightMap = INDEX_NONE;

	// Allocate objects.
	Model->Leaves.Empty();
	Model->Lights.Empty();

	// Assign leaf numbers to convex outside volumes.
	AssignLeaves( 0, Model->RootOutside );

	// Allocate leaf info.
	LeafPortals  = new( GMem, MEM_Zeroed, Model->Leaves.Num()      )FPortal*;
	LeafLights   = new( GMem, MEM_Zeroed, Model->Leaves.Num()      )FActorLink*;
	NodePortals  = new( GMem, MEM_Zeroed, Model->Nodes->Num()*2+256)FPortal*; // Allow for 2X expansion from zone portal fragments!!

	// Build all portals, with references to their front and back leaves.
	MakePortals( 0 );

	// Form zones.
	FormZonesFromLeaves();
	AssignAllZones( 0, Model->RootOutside );

	// Cleanup the bsp.
	//!!unsafe: screws up the node portals required for visibility checking.
	//!!but necessary for proper rendering.
#if !EVOLUTE_VISIBILITY
	GEditor->bspCleanup( Model );
	GEditor->bspRefresh( Model, 1 );
#endif
	GEditor->bspBuildBounds( Model );

	// Build zone interconnectivity info.
	BuildZoneMasks( Model, 0 );
	BuildConnectivity();
	BuildZoneInfo();

	debugf( NAME_Log, "Portalized: %i portals, %i zone portals (%i fragments), %i leaves, %i nodes", NumPortals, NumZonePortals, NumZoneFragments, Model->Leaves.Num(), Model->Nodes->Num() );

	// Test visibility of lightsources.
	guard(TestLights);
	INT* VisibleLeaves = new(GMem,Model->Leaves.Num())INT;
	INT NumLights[2]={0,0};
	DOUBLE LightSeconds=appSeconds();
	for( INT Pass=0; Pass<2; Pass++ )
	{
		for( i=0; i<Level->Num(); i++ )
		{
			AActor* Actor = Level->Actors(i);
			if
			(	Actor
			&&	Actor->LightType!=LT_None
			&&	(Actor->bStatic || Actor->bNoDelete) )
			{
				if( Pass == 1 )
				{
					GSystem->StatusUpdatef( NumLights[1], NumLights[0], "%s", "Illumination occluding" );
					INT NumVisibleLeaves = ActorVisibility( Actor );
					debugf( NAME_Log, "Lightsource %s: %i leaves", Actor->GetName(), NumVisibleLeaves );
				}
				NumLights[Pass]++;
			}
		}
	}
	if( NumLights[0] )
		debugf( NAME_Log, "Time = %f msec per light", 1000.0*(appSeconds()-LightSeconds)/NumLights[0] );
	unguard;

	// Form list of leaf-permeating lights.
	guard(FormLights);
	for( i=0; i<Model->Leaves.Num(); i++ )
	{
		check(Model->Leaves(i).iPermeating==INDEX_NONE);
		if( LeafLights[i] )
		{
			Model->Leaves(i).iPermeating = Model->Lights.Num();
			for( FActorLink* Link=LeafLights[i]; Link; Link=Link->Next )
				Model->Lights.AddItem( Link->Actor );
			Model->Lights.AddItem( NULL );
		}
	}
	unguard;

	// Test permeation of volumetric lights.
	guard(TestVolumetrics);
	for( i=0; i<Model->Leaves.Num(); i++ )
		LeafLights[i] = NULL;
	for( i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if
		(	Actor
		&&	Actor->Region.Zone->bFogZone
		&&	Actor->LightType!=LT_None
		&&	Actor->VolumeRadius
		&&	Actor->VolumeBrightness
		&&	(Actor->bStatic || Actor->bNoDelete) )
		{
			FilterVolumetricLight( Actor );
		}
	}
	unguard;

	// Form list of leaf volumetrics.
	guard(FormVolumetrics);
	for( i=0; i<Model->Leaves.Num(); i++ )
	{
		check(Model->Leaves(i).iVolumetric==INDEX_NONE);
		if( LeafLights[i] )
		{
			Model->Leaves(i).iVolumetric = Model->Lights.Num();
			for( FActorLink* Link=LeafLights[i]; Link; Link=Link->Next )
				Model->Lights.AddItem( Link->Actor );
			Model->Lights.AddItem( NULL );
		}
	}
	unguard;

#if EVOLUTE_VISIBILITY /* Test visibility of world */

	// Tag portals which we want to test.
	NumLogicalLeaves = Model->Leaves.Num();

	// Test only portals which partition logical leaves.
	for( FPortal *Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
		Portal->ShouldTest++;

	// Allocate visibility.
	Visibility = new(Model->GetParent(),NAME_None)UBitMatrix(Model->Leaves.Num());
	for( i=0; i<Model->Leaves.Num(); i++ )
		for( int j=0; i<Model->Leaves.Num(); i++ )
			Visibility->Set(i,j,0);

	// Each leaf can see itself.
	for( i=0; i<Model->Leaves.Num(); i++ )
		Visibility->Set(i,i,1);

	// Perform Bsp-based visibility test.
	BspVisibility( 0 );
	check(NumBspPortals==NumPortals);

	// Check for portal test leaks.
	for( Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
		check(!Portal->IsTesting);

	// Compute stats.
	int VisiCount=0,VisiMax=0;
	for( i=0; i<Model->Leaves.Num(); i++ )
	{
		int VisiLeaf = 0;
		for( int j=0; j<Model->Leaves.Num(); j++ )
			VisiLeaf += Visibility->Get(i,j);
		VisiCount += VisiLeaf;
		VisiMax    = Max(VisiMax,VisiLeaf);
	}

	int Bytes=0;
	for( i=0; i<Model->Leaves.Num(); i++ )
	{
		int Value=0, RunLength=0;
		for( int j=0; j<i; j++ )
		{
			if( Visibility->Get(j,i) == Value )
			{
				// No change in value.
				RunLength++;
				if( RunLength==16383 )
				{
					// Overflow, so emit 01 + RunLength[0-16384] + 00+RunLength[0]
					Bytes += 3;
					RunLength=0;
				}
			}
			else
			{
				if( j+1==i || Visibility->Get(j+1,i)!=Value )
				{
					// Permanent change in value.
					if( RunLength<=63 ) Bytes += 1; // 00+RunLength[0-63]
					else Bytes += 2; // 01+RunLength[0-16383]
					Value = !Value;
					RunLength=0;
				}
				else
				{
					// Temporary 1-bit change in value.
					if( RunLength<=63 ) Bytes += 1; // 10+RunLength[0-63]
					else Bytes += 2; // 11+RunLength[0-16383]
					RunLength=0;
				}
			}
		}
	}

	// Stats.
	VisTime = appSeconds() - VisTime;
	debugf(NAME_Log,"Visibility: %i portals, %i leaves, %i nodes",NumPortals,Model->Leaves.Num(),Model->Nodes->Num());

	debugf(NAME_Log,"Visibility: %i avg vis, %i max vis, %iK (%f%% ratio)",VisiCount/(Model->Leaves.Num()+1),VisiMax,Bytes/1024,100.0*8*Bytes/(Model->Leaves.Num()*(Model->Leaves.Num()+1)/2));
	debugf(NAME_Log,"Visibility: %i clip tests (%02.3ff%% passed, %02.3f%% unclipped), %i per leaf",NumClipTests,100.0*NumPassedClips/(NumClipTests+1),100.0*NumUnclipped/(NumClipTests+1),NumClipTests/(NumLogicalLeaves+1));
	debugf(NAME_Log,"Visibility: %i max fragments, %f seconds",MaxFragments,VisTime);
#endif

#if DEBUG_PORTALS || DEBUG_WRAPS || DEBUG_BADSHEETS || DEBUG_LVS
	GEditor->bspMergeCoplanars( Level->Brush()->Brush, 0, 1 );
#endif
	GSystem->EndSlowTask();
	unguard;
}

/*-----------------------------------------------------------------------------
	Visibility constructor/destructor.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
FEditorVisibility::FEditorVisibility( ULevel* InLevel, UModel* InModel, INT InExtra )
:	Mark			(GMem),
	Level			(InLevel),
	Model			(InModel),
	NumPortals		(0),
	NumClips		(0),
	NumClipTests	(0),
	NumPassedClips	(0),
	NumUnclipped	(0),
	NumBspPortals	(0),
	MaxFragments	(0),
	NumZonePortals	(0),
	NumZoneFragments(0),
	Extra			(InExtra),
	FirstPortal		(NULL),
	Visibility		(NULL),
	NodePortals		(NULL),
	LeafPortals		(NULL)
{
	guard(FEditorVisibility::FEditorVisibility);

#if DEBUG_PORTALS || DEBUG_WRAPS || DEBUG_BADSHEETS || DEBUG_LVS
	// Init brush for debugging.
	DEBUG_Brush=InLevel->Brush();
	DEBUG_Brush->Brush->Polys->Num=0;
	DEBUG_Brush->Location=DEBUG_Brush->PrePivot=FVector(0,0,0);
	DEBUG_Brush->Rotation = FRotator(0,0,0);
#endif

	unguard;
}

//
// Destructor.
//
FEditorVisibility::~FEditorVisibility()
{
	guard(FEditorVisibility::FEditorVisibility);
	Mark.Pop();
	//!!Visibility->~FSymmetricBitArray;
	unguard;
}

/*-----------------------------------------------------------------------------
	Main function.
-----------------------------------------------------------------------------*/

//
// Perform visibility testing within the level.
//
void UEditorEngine::TestVisibility( ULevel* Level, UModel* Model, int A, int B )
{
	guard(UEditorEngine::TestVisibility);
	if( Model->Nodes->Num() )
	{
		// Test visibility.
		FEditorVisibility Visi( Level, Model, A );
		Visi.TestVisibility();
	}
	unguard;
}


/*-----------------------------------------------------------------------------
	Bsp node bounding volumes.
-----------------------------------------------------------------------------*/

#if DEBUG_HULLS
	UModel *DEBUG_Brush;
#endif

//
// Update a bounding volume by expanding it to enclose a list of polys.
//
void UpdateBoundWithPolys( FBox& Bound, FPoly** PolyList, INT nPolys )
{
	guard(UpdateBoundWithPolys);
	for( INT i=0; i<nPolys; i++ )
		for( INT j=0; j<PolyList[i]->NumVertices; j++ )
			Bound += PolyList[i]->Vertex[j];
	unguard;
}

//
// Update a convolution hull with a list of polys.
//
void UpdateConvolutionWithPolys( UModel *Model, INT iNode, FPoly **PolyList, int nPolys )
{
	guard(UpdateConvolutionWithPolys);
	FBox Box(0);

	FBspNode &Node = Model->Nodes->Element(iNode);
	Node.iCollisionBound = Model->LeafHulls.Num();
	for( int i=0; i<nPolys; i++ )
	{
		if( PolyList[i]->iBrushPoly != INDEX_NONE )
		{
			for( int j=0; j<i; j++ )
				if( PolyList[j]->iBrushPoly == PolyList[i]->iBrushPoly )
					break;
			if( j >= i )
				Model->LeafHulls.AddItem(PolyList[i]->iBrushPoly);
		}
		for( int j=0; j<PolyList[i]->NumVertices; j++ )
			Box += PolyList[i]->Vertex[j];
	}
	Model->LeafHulls.AddItem(INDEX_NONE);

	// Add bounds.
	Model->LeafHulls.AddItem( *(INT*)&Box.Min.X );
	Model->LeafHulls.AddItem( *(INT*)&Box.Min.Y );
	Model->LeafHulls.AddItem( *(INT*)&Box.Min.Z );
	Model->LeafHulls.AddItem( *(INT*)&Box.Max.X );
	Model->LeafHulls.AddItem( *(INT*)&Box.Max.Y );
	Model->LeafHulls.AddItem( *(INT*)&Box.Max.Z );

	unguard;
}

//
// Cut a partitioning poly by a list of polys, and add the resulting inside pieces to the
// front list and back list.
//
void SplitPartitioner
(
	UModel*	Model,
	FPoly**	PolyList,
	FPoly**	FrontList,
	FPoly**	BackList,
	INT		n,
	INT		nPolys,
	INT&	nFront, 
	INT&	nBack, 
	FPoly	InfiniteEdPoly
)
{
	FPoly FrontPoly,BackPoly;
	while( n < nPolys )
	{
		if( InfiniteEdPoly.NumVertices >= FPoly::VERTEX_THRESHOLD )
		{
			FPoly Half;
			InfiniteEdPoly.SplitInHalf(&Half);
			SplitPartitioner(Model,PolyList,FrontList,BackList,n,nPolys,nFront,nBack,Half);
		}
		FPoly* Poly = PolyList[n];
		switch( InfiniteEdPoly.SplitWithPlane(Poly->Base,Poly->Normal,&FrontPoly,&BackPoly,0) )
		{
			case SP_Coplanar:
				// May occasionally happen.
				debugf( NAME_Log, "FilterBound: Got inficoplanar" );
				break;

			case SP_Front:
				// Shouldn't happen if hull is correct.
				debugf( NAME_Log, "FilterBound: Got infifront" );
				return;

			case SP_Split:
				InfiniteEdPoly = BackPoly;
				break;

			case SP_Back:
				break;
		}
		n++;
	}

	FPoly* New = new(GMem)FPoly;
	*New = InfiniteEdPoly;
	New->Reverse();
	New->iBrushPoly |= 0x40000000;
	FrontList[nFront++] = New;

	New = new(GMem)FPoly;
	*New = InfiniteEdPoly;
	BackList[nBack++] = New;
}

//
// Recursively filter a set of polys defining a convex hull down the Bsp,
// splitting it into two halves at each node and adding in the appropriate
// face polys at splits.
//
void FilterBound
(
	UModel*			Model,
	FBox*			ParentBound,
	INT				iNode,
	FPoly**			PolyList,
	INT				nPolys,
	INT				Outside
)
{
	FMemMark Mark(GMem);
	FBspNode&		Node	= Model->Nodes->Element(iNode);
	FBspSurf&		Surf	= Model->Surfs->Element(Node.iSurf);
	FVector&		Base	= Model->Points->Element(Surf.pBase);
	FVector&		Normal	= Model->Vectors->Element(Surf.vNormal);
	FBox			Bound;

	Bound.Min.X = Bound.Min.Y = Bound.Min.Z = +65536.0;
	Bound.Max.X = Bound.Max.Y = Bound.Max.Z = -65536.0;

	// Split bound into front half and back half.
	FPoly** FrontList = new(GMem,nPolys*2+16)FPoly*; int nFront=0;
	FPoly** BackList  = new(GMem,nPolys*2+16)FPoly*; int nBack=0;

	FPoly* FrontPoly  = new(GMem)FPoly;
	FPoly* BackPoly   = new(GMem)FPoly;

	for( INT i=0; i<nPolys; i++ )
	{
		FPoly *Poly = PolyList[i];
		switch( Poly->SplitWithPlane( Base, Normal, FrontPoly, BackPoly, 0 ) )
		{
			case SP_Coplanar:
				debugf( NAME_Log, "FilterBound: Got coplanar" );
				FrontList[nFront++] = Poly;
				BackList[nBack++] = Poly;
				break;
			
			case SP_Front:
				FrontList[nFront++] = Poly;
				break;
			
			case SP_Back:
				BackList[nBack++] = Poly;
				break;
			
			case SP_Split:
				if( FrontPoly->NumVertices >= FPoly::VERTEX_THRESHOLD )
				{
					FPoly *Half = new(GMem)FPoly;
					FrontPoly->SplitInHalf(Half);
					FrontList[nFront++] = Half;
				}
				FrontList[nFront++] = FrontPoly;

				if( BackPoly->NumVertices >= FPoly::VERTEX_THRESHOLD )
				{
					FPoly *Half = new(GMem)FPoly;
					BackPoly->SplitInHalf(Half);
					BackList[nBack++] = Half;
				}
				BackList [nBack++] = BackPoly;

				FrontPoly = new(GMem)FPoly;
				BackPoly  = new(GMem)FPoly;
				break;

			default:
				appErrorf( "FZoneFilter::FilterToLeaf: Unknown split code" );
		}
	}
	if( nFront && nBack )
	{
		// Add partitioner plane to front and back.
		FPoly InfiniteEdPoly = BuildInfiniteFPoly( Model, iNode );
		InfiniteEdPoly.iBrushPoly = iNode;

		SplitPartitioner(Model,PolyList,FrontList,BackList,0,nPolys,nFront,nBack,InfiniteEdPoly);
	}
	else
	{
		if( !nFront ) debugf( NAME_Log, "FilterBound: Empty fronthull" );
		if( !nBack  ) debugf( NAME_Log, "FilterBound: Empty backhull" );
	}

	// Recursively update all our childrens' bounding volumes.
	if( nFront > 0 )
	{
		if( Node.iFront != INDEX_NONE )
			FilterBound( Model, &Bound, Node.iFront, FrontList, nFront, Outside || Node.IsCsg() );
		else if( Outside || Node.IsCsg() )
			UpdateBoundWithPolys( Bound, FrontList, nFront );
		else
			UpdateConvolutionWithPolys( Model, iNode, FrontList, nFront );
	}
	if( nBack > 0 )
	{
		if( Node.iBack != INDEX_NONE)
			FilterBound( Model, &Bound,Node.iBack, BackList, nBack, Outside && !Node.IsCsg() );
		else if( Outside && !Node.IsCsg() )
			UpdateBoundWithPolys( Bound, BackList, nBack );
		else
			UpdateConvolutionWithPolys( Model, iNode, BackList, nBack );
	}

	// Apply this bound to this node if it's not a leaf.
	if( Node.iRenderBound==INDEX_NONE && (Node.iFront!=INDEX_NONE || Node.iBack!=INDEX_NONE) )
	{
		Node.iRenderBound = Model->Bounds.Add();
		Model->Bounds(Node.iRenderBound) = Bound;
	}

	// Update parent bound to enclose this bound.
	if( ParentBound )
		*ParentBound += Bound;

	Mark.Pop();
}

//
// Build bounding volumes for all Bsp nodes.  The bounding volume of the node
// completely encloses the "outside" space occupied by the nodes.  Note that 
// this is not the same as representing the bounding volume of all of the 
// polygons within the node.
//
// We start with a practically-infinite cube and filter it down the Bsp,
// whittling it away until all of its convex volume fragments land in leaves.
//
void UEditorEngine::bspBuildBounds( UModel* Model )
{
	guard(UEditorEngine::bspBuildBounds);
	if( Model->Nodes->Num()==0 )
		return;

	BuildZoneMasks( Model, 0 );

	FPoly Polys[6], *PolyList[6];
	guard(Init);
	for( int i=0; i<6; i++ )
	{
		PolyList[i] = &Polys[i];
		PolyList[i]->Init();
		PolyList[i]->NumVertices=4;
		PolyList[i]->iBrushPoly = INDEX_NONE;
	}

	Polys[0].Vertex[0]=FVector(-32768.000000,-32768.000000,32768.000000);
	Polys[0].Vertex[1]=FVector( 32768.000000,-32768.000000,32768.000000);
	Polys[0].Vertex[2]=FVector( 32768.000000, 32768.000000,32768.000000);
	Polys[0].Vertex[3]=FVector(-32768.000000, 32768.000000,32768.000000);
	Polys[0].Normal   =FVector( 0.000000,  0.000000,  1.000000 );
	Polys[0].Base     =Polys[0].Vertex[0];

	Polys[1].Vertex[0]=FVector(-32768.000000, 32768.000000,-32768.000000);
	Polys[1].Vertex[1]=FVector( 32768.000000, 32768.000000,-32768.000000);
	Polys[1].Vertex[2]=FVector( 32768.000000,-32768.000000,-32768.000000);
	Polys[1].Vertex[3]=FVector(-32768.000000,-32768.000000,-32768.000000);
	Polys[1].Normal   =FVector( 0.000000,  0.000000, -1.000000 );
	Polys[1].Base     =Polys[1].Vertex[0];

	Polys[2].Vertex[0]=FVector(-32768.000000,32768.000000,-32768.000000);
	Polys[2].Vertex[1]=FVector(-32768.000000,32768.000000, 32768.000000);
	Polys[2].Vertex[2]=FVector( 32768.000000,32768.000000, 32768.000000);
	Polys[2].Vertex[3]=FVector( 32768.000000,32768.000000,-32768.000000);
	Polys[2].Normal   =FVector( 0.000000,  1.000000,  0.000000 );
	Polys[2].Base     =Polys[2].Vertex[0];

	Polys[3].Vertex[0]=FVector( 32768.000000,-32768.000000,-32768.000000);
	Polys[3].Vertex[1]=FVector( 32768.000000,-32768.000000, 32768.000000);
	Polys[3].Vertex[2]=FVector(-32768.000000,-32768.000000, 32768.000000);
	Polys[3].Vertex[3]=FVector(-32768.000000,-32768.000000,-32768.000000);
	Polys[3].Normal   =FVector( 0.000000, -1.000000,  0.000000 );
	Polys[3].Base     =Polys[3].Vertex[0];

	Polys[4].Vertex[0]=FVector(32768.000000, 32768.000000,-32768.000000);
	Polys[4].Vertex[1]=FVector(32768.000000, 32768.000000, 32768.000000);
	Polys[4].Vertex[2]=FVector(32768.000000,-32768.000000, 32768.000000);
	Polys[4].Vertex[3]=FVector(32768.000000,-32768.000000,-32768.000000);
	Polys[4].Normal   =FVector( 1.000000,  0.000000,  0.000000 );
	Polys[4].Base     =Polys[4].Vertex[0];

	Polys[5].Vertex[0]=FVector(-32768.000000,-32768.000000,-32768.000000);
	Polys[5].Vertex[1]=FVector(-32768.000000,-32768.000000, 32768.000000);
	Polys[5].Vertex[2]=FVector(-32768.000000, 32768.000000, 32768.000000);
	Polys[5].Vertex[3]=FVector(-32768.000000, 32768.000000,-32768.000000);
	Polys[5].Normal   =FVector(-1.000000,  0.000000,  0.000000 );
	Polys[5].Base     =Polys[5].Vertex[0];
	unguard;

	// Empty bounds and hulls.
	guard(Empty);
	Model->Bounds.Empty();
	Model->LeafHulls.Empty();
	for( int i=0; i<Model->Nodes->Num(); i++ )
	{
		Model->Nodes->Element(i).iRenderBound     = INDEX_NONE;
		Model->Nodes->Element(i).iCollisionBound  = INDEX_NONE;
	}
	unguard;

	guard(Filter);
	FilterBound( Model, NULL, 0, PolyList, 6, Model->RootOutside );
	Model->Bounds.Shrink();
	unguard;

	debugf( NAME_Log, "bspBuildBounds: Generated %i bounds, %i hulls", Model->Bounds.Num(), Model->LeafHulls.Num() );
	unguard;
}

/*-----------------------------------------------------------------------------
	Non-class functions.
-----------------------------------------------------------------------------*/

//
// Build an FPoly representing an "infinite" plane (which exceeds the maximum
// dimensions of the world in all directions) for a particular Bsp node.
//
FPoly BuildInfiniteFPoly( UModel* Model, INT iNode )
{
	guard(BuildInfiniteFPoly);

	FBspNode &Node   = Model->Nodes->Element(iNode       );
	FBspSurf &Poly   = Model->Surfs->Element(Node.iSurf  );
	FVector  &Base   = Model->Points->Element(Poly.pBase  );
	FVector  &Normal = Model->Vectors->Element(Poly.vNormal);
	FVector	 Axis1,Axis2;

	// Find two non-problematic axis vectors.
	Normal.FindBestAxisVectors( Axis1, Axis2 );

	// Set up the FPoly.
	FPoly EdPoly;
	EdPoly.Init();
	EdPoly.NumVertices = 4;
	EdPoly.Normal      = Normal;
	EdPoly.Base        = Base;
	EdPoly.Vertex[0]   = Base + Axis1*WORLD_MAX + Axis2*WORLD_MAX;
	EdPoly.Vertex[1]   = Base - Axis1*WORLD_MAX + Axis2*WORLD_MAX;
	EdPoly.Vertex[2]   = Base - Axis1*WORLD_MAX - Axis2*WORLD_MAX;
	EdPoly.Vertex[3]   = Base + Axis1*WORLD_MAX - Axis2*WORLD_MAX;

#if CHECK_ALL // Validate the poly
	if( EdPoly.SplitWithNode( Model, iNode, NULL, NULL, 0 )!=SP_Coplanar)
		appError( "BuildInfinitePoly failed" );
#endif

	return EdPoly;
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
