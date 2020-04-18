/*=============================================================================
	UnEdCsg.cpp: High-level CSG tracking functions for editor
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"

//
// Globals:
//!!augh wastes 128K
//
BYTE GFlags1 [MAXWORD+1]; // For fast polygon selection
BYTE GFlags2 [MAXWORD+1];

/*-----------------------------------------------------------------------------
	Level brush tracking
-----------------------------------------------------------------------------*/

//
// Prepare a moving brush.
//
void UEditorEngine::csgPrepMovingBrush( ABrush* Actor )
{
	guard(UEditorEngine::csgPrepMovingBrush);
	check(Actor);
	check(Actor->Brush);
	check(Actor->Brush->RootOutside);
	debugf( NAME_Log, "Preparing brush %s", Actor->GetName() );

	// Allocate tables
	Actor->Brush->AllocDatabases( 0 );

	// Build bounding box.
	Actor->Brush->BuildBound();

	// Build BSP for the brush.
	bspBuild( Actor->Brush, BSP_Good, 15, 1, 0 );
	bspRefresh( Actor->Brush, 1 );
	bspBuildBounds( Actor->Brush );

	unguard;
}

//
// Duplicate the specified brush and make it into a CSG-able level brush.
// Returns new brush, or NULL if the original was empty.
//
void UEditorEngine::csgCopyBrush
(
	ABrush*		Dest,
	ABrush*		Src,
	DWORD		PolyFlags, 
	DWORD		ResFlags,
	UBOOL		IsMovingBrush
)
{
	guard(UEditorEngine::csgCopyBrush);
	check(Src);
	check(Src->Brush);
	check(Src->IsA(ABrush::StaticClass));
	check(Dest->IsA(ABrush::StaticClass));

	if( Src->Brush->Polys->Num() == 0 )
	{
		Dest->Brush = NULL;
		return;
	}

	// Duplicate the brush and its polys.
	Dest->Brush              = new( Src->Brush->GetParent(), NAME_None, ResFlags )UModel( NULL, Src->Brush->RootOutside );
	Dest->Brush->Polys       = new( Src->Brush->GetParent(), NAME_None, ResFlags )UPolys;

	// Set new brush's properties.
	Dest->PolyFlags = PolyFlags;

	// Update poly textures.
	Dest->Brush->Polys->Add( Src->Brush->Polys->Num() );
	for( INT i=0; i<Dest->Brush->Polys->Num(); i++ )
	{
		FPoly& Poly     = Dest->Brush->Polys->Element(i);
		Poly            = Src->Brush->Polys->Element(i);
		Poly.Texture    = Poly.Texture ? Poly.Texture : CurrentTexture;
		Poly.iBrushPoly = INDEX_NONE;
	}

	// Build bounding box.
	Dest->Brush->BuildBound();

	// If it's a moving brush, prep it.
	if( IsMovingBrush )
		csgPrepMovingBrush( Dest );

	// Copy vital info.
	Dest->CopyPosRotScaleFrom( Src );

	unguard;
}

//
// Add a brush to the list of CSG brushes in the level, using a CSG operation, and return 
// a newly-created copy of it.
//
ABrush* UEditorEngine::csgAddOperation
(
	ABrush*		Actor,
	ULevel*		Level,
	DWORD		PolyFlags,
	ECsgOper	CsgOper
)
{
	guard(UEditorEngine::csgAddOperation);
	check(Actor);
	check(Actor->Brush);
	check(Actor->Brush->Polys);

	// Can't do this if brush has no polys.
	if( !Actor->Brush->Polys->Num() )
		return NULL;

	// Spawn a new actor for the brush.
	ABrush* Result  = Level->SpawnBrush();
	Result->SetFlags( RF_NotForClient | RF_NotForServer );

	// Duplicate the brush.
	csgCopyBrush
	(
		Result,
		Actor,
		PolyFlags,
		RF_NotForClient | RF_NotForServer,
		0
	);
	check(Result->Brush);

	// Set add-info.
	Result->CsgOper = CsgOper;

	return Result;
	unguard;
}

const char* UEditorEngine::csgGetName( ECsgOper CSG )
{
	guard(UEditorEngine::csgGetName);
	return *(FindObjectChecked<UEnum>( ANY_PACKAGE, "ECsgOper" ) )->Names(CSG);
	unguard;
}

/*-----------------------------------------------------------------------------
	CSG Rebuilding.
-----------------------------------------------------------------------------*/

//
// Rebuild the level's Bsp from the level's CSG brushes
//
// Note: Needs to be expanded to defragment Bsp polygons as needed (by rebuilding
// the Bsp), so that it doesn't slow down to a crawl on complex levels.
//
#if 0
void UEditorEngine::csgRebuild( ULevel* Level )
{
	guard(UEditorEngine::csgRebuild);

	int		NodeCount,PolyCount,LastPolyCount;
	char	TempStr[80];

	GSystem->BeginSlowTask( "Rebuilding geometry", 1, 0 );
	FastRebuild = 1;

	FinishAllSnaps(Level);

	// Empty the model out.
	Level->Lock( LOCK_Trans );
	Level->Model->EmptyModel( 1, 1 );
	Level->Unlock( LOCK_Trans );

	// Count brushes.
	int BrushTotal=0, BrushCount=0;
	for( FStaticBrushIterator TempIt(Level); TempIt; ++TempIt )
		if( *TempIt != Level->Brush() )
			BrushTotal++;

	LastPolyCount = 0;
	for( FStaticBrushIterator It(Level); It; ++It )
	{
		if( *It == Level->Brush() )
			continue;
		BrushCount++;
		appSprintf(TempStr,"Applying brush %i of %i",BrushCount,BrushTotal);
		GSystem->StatusUpdate( TempStr, BrushCount, BrushTotal );

		// See if the Bsp has become badly fragmented and, if so, rebuild.
		PolyCount = Level->Model->Surfs->Num();
		NodeCount = Level->Model->Nodes->Num();
		if( PolyCount>2000 && PolyCount>=3*LastPolyCount )
		{
			appStrcat( TempStr, ": Refreshing Bsp..." );
			GSystem->StatusUpdate( TempStr, BrushCount, BrushTotal );

			debugf 				( NAME_Log, "Map: Rebuilding Bsp" );
			bspBuildFPolys		( Level->Model, 1, 0 );
			bspMergeCoplanars	( Level->Model, 0, 0 );
			bspBuild			( Level->Model, BSP_Lame, 25, 0, 0 );
			debugf				( NAME_Log, "Map: Reduced nodes by %i%%, polys by %i%%", (100*(NodeCount-Level->Model->Nodes->Num()))/NodeCount,(100*(PolyCount-Level->Model->Surfs->Num()))/PolyCount );

			LastPolyCount = Level->Model->Surfs->Num();
		}

		// Perform this CSG operation.
		bspBrushCSG( *It, Level->Model, It->PolyFlags, (ECsgOper)It->CsgOper, 0 );
	}

	// Build bounding volumes.
	Level->Lock( LOCK_Trans );
	bspBuildBounds( Level->Model );
	Level->Unlock( LOCK_Trans );

	// Done.
	FastRebuild = 0;
	GSystem->EndSlowTask();
	unguard;
}
#endif

#if 1

//
// Repartition the bsp.
//
void UEditorEngine::bspRepartition( UModel* Model, INT iNode, INT Simple )
{
	guard(UEditorEngine::bspRepartition);

	bspBuildFPolys( Level->Model, 1, iNode );
	bspMergeCoplanars( Level->Model, 0, 0 );
	bspBuild( Level->Model, BSP_Good, 12, Simple, iNode );
	bspRefresh( Level->Model, 1 );

	unguard;
}

//
// Build list of leaves.
//
static void EnlistLeaves( UModel* Model, TArray<INT>& iFronts, TArray<INT>& iBacks, INT iNode )
{
	guard(EnlistLeaves);
	FBspNode& Node=Model->Nodes->Element(iNode);

	if( Node.iFront==INDEX_NONE ) iFronts.AddItem(iNode);
	else EnlistLeaves( Model, iFronts, iBacks, Node.iFront );

	if( Node.iBack==INDEX_NONE ) iBacks.AddItem(iNode);
	else EnlistLeaves( Model, iFronts, iBacks, Node.iBack );

	unguard;
}

//
// Rebuild the level's Bsp from the level's CSG brushes.
//
void UEditorEngine::csgRebuild( ULevel* Level )
{
	guard(UEditorEngine::csgRebuild);

	GSystem->BeginSlowTask( "Rebuilding geometry", 1, 0 );
	FastRebuild = 1;

	FinishAllSnaps(Level);

	// Empty the model out.
	Level->Model->EmptyModel( 1, 1 );

	// Count brushes.
	INT BrushTotal=0, BrushCount=0;
	for( FStaticBrushIterator TempIt(Level); TempIt; ++TempIt )
		if( *TempIt != Level->Brush() )
			BrushTotal++;

	// Compose all structural brushes and portals.
	for( FStaticBrushIterator It(Level); It; ++It )
	{
		if( *It!=Level->Brush() )
		{
			if
			(  !(It->PolyFlags&PF_Semisolid)
			||	(It->CsgOper!=CSG_Add)
			||	(It->PolyFlags&PF_Portal) )
			{
				// Treat portals as solids for cutting.
				if( It->PolyFlags & PF_Portal )
					It->PolyFlags = (It->PolyFlags & ~PF_Semisolid) | PF_NotSolid;
				BrushCount++;
				GSystem->StatusUpdatef( BrushCount, BrushTotal, "Applying structural brush %i of %i", BrushCount, BrushTotal );
				bspBrushCSG( *It, Level->Model, It->PolyFlags, (ECsgOper)It->CsgOper, 0 );
			}
		}
	}

	// Repartition the structural BSP.
	bspRepartition( Level->Model, 0, 0 );
	TestVisibility( Level, Level->Model, 0, 0 );

	// Remember leaves.
	TArray<INT> iFronts, iBacks;
	if( Level->Model->Nodes->Num() )
		EnlistLeaves( Level->Model, iFronts, iBacks, 0 );

#if 1
	// Compose all detail brushes.
	for( It=FStaticBrushIterator(Level); It; ++It )
	{
		if
		(	*It!=Level->Brush()
		&&	(It->PolyFlags&PF_Semisolid)
		&& !(It->PolyFlags&PF_Portal)
		&&	It->CsgOper==CSG_Add )
		{
			BrushCount++;
			GSystem->StatusUpdatef( BrushCount, BrushTotal, "Applying detail brush %i of %i", BrushCount, BrushTotal );
			bspBrushCSG( *It, Level->Model, It->PolyFlags, (ECsgOper)It->CsgOper, 0 );
		}
	}

	// Optimize the sub-bsp's.
	INT iNode;
	for( TIterator<INT>ItF(iFronts); ItF; ++ItF )
		if( (iNode=Level->Model->Nodes->Element(*ItF).iFront)!=INDEX_NONE )
			bspRepartition( Level->Model, iNode, 2 );
	for( TIterator<INT>ItB(iBacks); ItB; ++ItB )
		if( (iNode=Level->Model->Nodes->Element(*ItB).iBack)!=INDEX_NONE )
			bspRepartition( Level->Model, iNode, 2 );
#endif

	// Build bounding volumes.
	bspOptGeom( Level->Model );
	bspBuildBounds( Level->Model );

	// Done.
	FastRebuild = 0;
	GSystem->EndSlowTask();
	unguard;
}
#endif

/*---------------------------------------------------------------------------------------
	Flag setting and searching
---------------------------------------------------------------------------------------*/

//
// Sets and clears all Bsp node flags.  Affects all nodes, even ones that don't
// really exist.
//
void UEditorEngine::polySetAndClearPolyFlags(UModel *Model, DWORD SetBits, DWORD ClearBits,int SelectedOnly, int UpdateMaster)
{
	guard(UEditorEngine::polySetAndClearPolyFlags);
	for( INT i=0; i<Model->Surfs->Num(); i++ )
	{
		FBspSurf& Poly = Model->Surfs->Element(i);
		if( !SelectedOnly || (Poly.PolyFlags & PF_Selected) )
		{
			DWORD NewFlags = (Poly.PolyFlags & ~ClearBits) | SetBits;
			if( NewFlags != Poly.PolyFlags )
			{
				Model->Surfs->ModifyItem( i, UpdateMaster );
				Poly.PolyFlags = NewFlags;
				if( UpdateMaster )
					polyUpdateMaster( Model, i, 0, 0 );
			}
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Polygon searching
-----------------------------------------------------------------------------*/

//
// Find the Brush EdPoly corresponding to a given Bsp surface.
//
int UEditorEngine::polyFindMaster(UModel *Model, INT iSurf, FPoly &Poly)
{
	guard(UEditorEngine::polyFindMaster);

	FBspSurf &Surf = Model->Surfs->Element(iSurf);
	if( !Surf.Actor )
	{
		return 0;
	}
	else
	{
		Poly = Surf.Actor->Brush->Polys->Element(Surf.iBrushPoly);
		return 1;
	}
	unguard;
}

//
// Update a the master brush EdPoly corresponding to a newly-changed
// poly to reflect its new properties.
//
// Doesn't do any transaction tracking.  Assumes you've called transSelectedBspSurfs.
//
void UEditorEngine::polyUpdateMaster
(
	UModel*	Model,
	INT  	iSurf,
	INT		UpdateTexCoords,
	INT		UpdateBase
)
{
	guard(UEditorEngine::polyUpdateMaster);

	FBspSurf &Poly = Model->Surfs->Element(iSurf);
	if( !Poly.Actor )
		return;

	FModelCoords Uncoords;
	if( UpdateTexCoords || UpdateBase )
		Poly.Actor->BuildCoords( NULL, &Uncoords );

	for( INT iEdPoly = Poly.iBrushPoly; iEdPoly < Poly.Actor->Brush->Polys->Num(); iEdPoly++ )
	{
		FPoly& MasterEdPoly = Poly.Actor->Brush->Polys->Element(iEdPoly);
		if( iEdPoly==Poly.iBrushPoly || MasterEdPoly.iLink==Poly.iBrushPoly )
		{
			Poly.Actor->Brush->Polys->ModifyItem(iEdPoly);

			MasterEdPoly.Texture   = Poly.Texture;
			MasterEdPoly.PanU      = Poly.PanU;
			MasterEdPoly.PanV      = Poly.PanV;
			MasterEdPoly.PolyFlags = Poly.PolyFlags & ~(PF_NoEdit);

			if( UpdateTexCoords || UpdateBase )
			{
				if( UpdateTexCoords )
				{
					MasterEdPoly.TextureU = Model->Vectors->Element(Poly.vTextureU).TransformVectorBy(Uncoords.VectorXform);
					MasterEdPoly.TextureV = Model->Vectors->Element(Poly.vTextureV).TransformVectorBy(Uncoords.VectorXform);
				}
				if( UpdateBase )
				{
					MasterEdPoly.Base
					=	(Model->Points->Element(Poly.pBase) - Poly.Actor->Location)
					.	TransformVectorBy(Uncoords.PointXform)
					+	Poly.Actor->PrePivot;
				}
			}
		}
	}
	unguard;
}

//
// Find all Bsp polys with flags such that SetBits are clear or ClearBits are set.
//
void UEditorEngine::polyFindByFlags(UModel *Model, DWORD SetBits, DWORD ClearBits, POLY_CALLBACK Callback)
	{
	guard(UEditorEngine::polyFindByFlags);
	FBspSurf *Poly = &Model->Surfs->Element(0);
	//
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		if (((Poly->PolyFlags&SetBits)!=0) || ((Poly->PolyFlags&~ClearBits)!=0))
			{
			Callback (Model,i);
			};
		Poly++;
		};
	unguard;
	};

//
// Find all BspSurfs corresponding to a particular editor brush object
// and polygon index. Call with BrushPoly set to INDEX_NONE to find all Bsp 
// polys corresponding to the Brush.
//
void UEditorEngine::polyFindByBrush( UModel* Model, ABrush* Actor, INT iBrushPoly, POLY_CALLBACK Callback )
	{
	guard(UEditorEngine::polyFindByBrush);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf &Poly = Model->Surfs->Element(i);
		if (
			(Poly.Actor == Actor) && 
			((iBrushPoly == INDEX_NONE) || (Poly.iBrushPoly == iBrushPoly))
			)
			{
			Callback (Model,i);
			};
		};
	unguard;
	};

/*-----------------------------------------------------------------------------
   All transactional polygon selection functions
-----------------------------------------------------------------------------*/

void UEditorEngine::polyResetSelection(UModel *Model)
	{
	guard(UEditorEngine::polyResetSelection);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		Poly->PolyFlags |= ~(PF_Selected | PF_Memorized);
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polySelectAll(UModel *Model)
	{
	guard(UEditorEngine::polySelectAll);
	polySetAndClearPolyFlags(Model,PF_Selected,0,0,0);
	unguard;
	};

void UEditorEngine::polySelectMatchingGroups(UModel *Model)
	{
	guard(UEditorEngine::polySelectMatchingGroups);
	appMemset( GFlags1, 0, sizeof(GFlags1) );
	//
	for (INT i=0; i<Model->Surfs->Num(); i++)
	{
		FBspSurf *Surf = &Model->Surfs->Element(i);
		if (Surf->PolyFlags&PF_Selected)
		{
			FPoly Poly; polyFindMaster(Model,i,Poly);
			GFlags1[Poly.Actor->Group.GetIndex()]=1;
		}
	}
	for (i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Surf = &Model->Surfs->Element(i);
		FPoly Poly; polyFindMaster(Model,i,Poly);
		if ((GFlags1[Poly.Actor->Group.GetIndex()]) 
			&& (!(Surf->PolyFlags & PF_Selected)))
			{
			Model->Surfs->ModifyItem(i,0);
			Surf->PolyFlags |= PF_Selected;
			};
		};
	unguard;
	};

void UEditorEngine::polySelectMatchingItems(UModel *Model)
{
	guard(UEditorEngine::polySelectMatchingItems);

	appMemset(GFlags1,0,sizeof(GFlags1));
	appMemset(GFlags2,0,sizeof(GFlags2));

	for( INT i=0; i<Model->Surfs->Num(); i++ )
	{
		FBspSurf *Surf = &Model->Surfs->Element(i);
		if( Surf->Actor )
		{
			if( Surf->PolyFlags & PF_Selected )
				GFlags2[Surf->Actor->Brush->GetIndex()]=1;
		}
		if( Surf->PolyFlags&PF_Selected )
		{
			FPoly Poly; polyFindMaster(Model,i,Poly);
			GFlags1[Poly.ItemName.GetIndex()]=1;
		}
	}
	for( i=0; i<Model->Surfs->Num(); i++ )
	{
		FBspSurf *Surf = &Model->Surfs->Element(i);
		if( Surf->Actor )
		{
			FPoly Poly; polyFindMaster(Model,i,Poly);
			if ((GFlags1[Poly.ItemName.GetIndex()]) &&
				( GFlags2[Surf->Actor->Brush->GetIndex()]) &&
				(!(Surf->PolyFlags & PF_Selected)))
			{
				Model->Surfs->ModifyItem(i,0);
				Surf->PolyFlags |= PF_Selected;
			}
		}
	}
	unguard;
}

enum EAdjacentsType
{
	ADJACENT_ALL,		// All adjacent polys
	ADJACENT_COPLANARS,	// Adjacent coplanars only
	ADJACENT_WALLS,		// Adjacent walls
	ADJACENT_FLOORS,	// Adjacent floors or ceilings
	ADJACENT_SLANTS,	// Adjacent slants
};

//
// Select all adjacent polygons (only coplanars if Coplanars==1) and
// return number of polygons newly selected.
//
int TagAdjacentsType(UModel *Model, EAdjacentsType AdjacentType)
	{
	guard(TagAdjacentsType);
	FVert	*VertPool;
	FVector		*Base,*Normal;
	BYTE		b;
	INT		    i;
	int			Selected,Found;
	//
	Selected = 0;
	appMemset( GFlags1, 0, sizeof(GFlags1) );
	//
	// Find all points corresponding to selected vertices:
	//
	for (i=0; i<Model->Nodes->Num(); i++)
		{
		FBspNode &Node = Model->Nodes->Element(i);
		FBspSurf &Poly = Model->Surfs->Element(Node.iSurf);
		if (Poly.PolyFlags & PF_Selected)
			{
			VertPool = &Model->Verts->Element(Node.iVertPool);
			//
			for (b=0; b<Node.NumVertices; b++) GFlags1[(VertPool++)->pVertex] = 1;
			};
		};
	//
	// Select all unselected nodes for which two or more vertices are selected:
	//
	for (i=0; i<Model->Nodes->Num(); i++)
		{
		FBspNode &Node = Model->Nodes->Element(i);
		FBspSurf &Poly = Model->Surfs->Element(Node.iSurf);
		if (!(Poly.PolyFlags & PF_Selected))
			{
			Found    = 0;
			VertPool = &Model->Verts->Element(Node.iVertPool);
			//
			Base   = &Model->Points->Element(Poly.pBase);
			Normal = &Model->Vectors->Element(Poly.vNormal);
			//
			for (b=0; b<Node.NumVertices; b++) Found += GFlags1[(VertPool++)->pVertex];
			//
			if (AdjacentType == ADJACENT_COPLANARS)
				{
				if (!GFlags2[Node.iSurf]) Found=0;
				}
			else if (AdjacentType == ADJACENT_FLOORS)
				{
				if (Abs(Normal->Z) <= 0.85) Found = 0;
				}
			else if (AdjacentType == ADJACENT_WALLS)
				{
				if (Abs(Normal->Z) >= 0.10) Found = 0;
				}
			else if (AdjacentType == ADJACENT_SLANTS)
				{
				if (Abs(Normal->Z) > 0.85) Found = 0;
				if (Abs(Normal->Z) < 0.10) Found = 0;
				};
			if (Found > 0)
				{
				Model->Surfs->ModifyItem(Node.iSurf,0);
				Poly.PolyFlags |= PF_Selected;
				Selected++;
				};
			};
		};
	return Selected;
	unguard;
	};

void TagCoplanars(UModel *Model)
	{
	guard(TagCoplanars);
	FBspSurf	*SelectedPoly,*Poly;
	FVector		*SelectedBase,*SelectedNormal,*Base,*Normal;
	//
	appMemset(GFlags2,0,sizeof(GFlags2));
	//
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		SelectedPoly = &Model->Surfs->Element(i);
		if (SelectedPoly->PolyFlags & PF_Selected)
			{
			SelectedBase   = &Model->Points->Element(SelectedPoly->pBase);
			SelectedNormal = &Model->Vectors->Element(SelectedPoly->vNormal);
			//
			for (INT j=0; j<Model->Surfs->Num(); j++)
				{
				Poly = &Model->Surfs->Element(j);
				Base   = &Model->Points->Element(Poly->pBase);
				Normal = &Model->Vectors->Element(Poly->vNormal);
				//
				if (FCoplanar(*Base,*Normal,*SelectedBase,*SelectedNormal) && (!(Poly->PolyFlags & PF_Selected)))
					{
					GFlags2[j]=1;
					};
				};
			};
		};
	unguard;
	};

void UEditorEngine::polySelectAdjacents(UModel *Model)
	{
	guard(UEditorEngine::polySelectAdjacents);
	do {} while (TagAdjacentsType (Model,ADJACENT_ALL) > 0);
	unguard;
	};

void UEditorEngine::polySelectCoplanars(UModel *Model)
	{
	guard(UEditorEngine::polySelectCoplanars);
	TagCoplanars(Model);
	do {} while (TagAdjacentsType(Model,ADJACENT_COPLANARS) > 0);
	unguard;
	};

void UEditorEngine::polySelectMatchingBrush(UModel *Model)
	{
	guard(UEditorEngine::polySelectMatchingBrush);
	//
	appMemset( GFlags1, 0, sizeof(GFlags1) );
	//
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if( Poly->Actor->Brush )
			if( Poly->PolyFlags & PF_Selected )
				GFlags1[Poly->Actor->Brush->GetIndex()]=1;
		Poly++;
		};
	for (i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if( Poly->Actor->Brush )
			{
			if ((GFlags1[Poly->Actor->Brush->GetIndex()])&&(!(Poly->PolyFlags&PF_Selected)))
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags |= PF_Selected;
				};
			};
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polySelectMatchingTexture(UModel *Model)
	{
	guard(UEditorEngine::polySelectMatchingTexture);
	INT		i,Blank=0;
	appMemset( GFlags1, 0, sizeof(GFlags1) );
	//
	for (i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if (Poly->Texture && (Poly->PolyFlags&PF_Selected)) GFlags1[Poly->Texture->GetIndex()]=1;
		else if (!Poly->Texture) Blank=1;
		Poly++;
		};
	for (i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if (Poly->Texture && (GFlags1[Poly->Texture->GetIndex()]) && (!(Poly->PolyFlags&PF_Selected)))
			{
			Model->Surfs->ModifyItem(i,0);
			Poly->PolyFlags |= PF_Selected;
			}
		else if (Blank & !Poly->Texture) Poly->PolyFlags |= PF_Selected;
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polySelectAdjacentWalls(UModel *Model)
	{
	guard(UEditorEngine::polySelectAdjacentWalls);
	do {} while (TagAdjacentsType  (Model,ADJACENT_WALLS) > 0);
	unguard;
	};

void UEditorEngine::polySelectAdjacentFloors(UModel *Model)
	{
	guard(UEditorEngine::polySelectAdjacentFloors);
	do {} while (TagAdjacentsType (Model,ADJACENT_FLOORS) > 0);
	unguard;
	};

void UEditorEngine::polySelectAdjacentSlants(UModel *Model)
	{
	guard(UEditorEngine::polySelectAdjacentSlants);
	do {} while (TagAdjacentsType  (Model,ADJACENT_SLANTS) > 0);
	unguard;
	};

void UEditorEngine::polySelectReverse(UModel *Model)
	{
	guard(UEditorEngine::polySelectReverse);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		Model->Surfs->ModifyItem(i,0);
		Poly->PolyFlags ^= PF_Selected;
		//
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polyMemorizeSet(UModel *Model)
	{
	guard(UEditorEngine::polyMemorizeSet);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if (Poly->PolyFlags & PF_Selected) 
			{
			if (!(Poly->PolyFlags & PF_Memorized))
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags |= (PF_Memorized);
				};
			}
		else
			{
			if (Poly->PolyFlags & PF_Memorized)
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags &= (~PF_Memorized);
				};
			};
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polyRememberSet(UModel *Model)
	{
	guard(UEditorEngine::polyRememberSet);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if (Poly->PolyFlags & PF_Memorized) 
			{
			if (!(Poly->PolyFlags & PF_Selected))
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags |= (PF_Selected);
				};
			}
		else
			{
			if (Poly->PolyFlags & PF_Selected)
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags &= (~PF_Selected);
				};
			};
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polyXorSet(UModel *Model)
	{
	int			Flag1,Flag2;
	//
	guard(UEditorEngine::polyXorSet);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		Flag1 = (Poly->PolyFlags & PF_Selected ) != 0;
		Flag2 = (Poly->PolyFlags & PF_Memorized) != 0;
		//
		if (Flag1 ^ Flag2)
			{
			if (!(Poly->PolyFlags & PF_Selected))
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags |= PF_Selected;
				};
			}
		else
			{
			if (Poly->PolyFlags & PF_Selected)
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags &= (~PF_Selected);
				};
			};
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polyUnionSet(UModel *Model)
	{
	guard(UEditorEngine::polyUnionSet);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if (!(Poly->PolyFlags & PF_Memorized))
			{
			if (Poly->PolyFlags | PF_Selected)
				{
				Model->Surfs->ModifyItem(i,0);
				Poly->PolyFlags &= (~PF_Selected);
				};
			};
		Poly++;
		};
	unguard;
	};

void UEditorEngine::polyIntersectSet(UModel *Model)
	{
	guard(UEditorEngine::polyIntersectSet);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if ((Poly->PolyFlags & PF_Memorized) && !(Poly->PolyFlags & PF_Selected))
			{
			Poly->PolyFlags |= PF_Selected;
			};
		Poly++;
		};
	unguard;
	};

/*---------------------------------------------------------------------------------------
   Brush selection functions
---------------------------------------------------------------------------------------*/

//
// Generic selection routines
//

typedef int (*BRUSH_SEL_FUNC)( ABrush* Brush, int Tag );

void MapSelect( ULevel* Level, BRUSH_SEL_FUNC Func, int Tag )
{
	guard(MapSelect);
	for( FStaticBrushIterator It(Level); It; ++It )
	{
		ABrush* Actor = *It;
		if( Func( Actor, Tag ) )
		{
			// Select it.
			if( !Actor->bSelected )
			{
				Actor->Modify();
				Actor->bSelected = 1;
			}
		}
		else
		{
			// Deselect it.
			if( Actor->bSelected )
			{
				Actor->Modify();
				Actor->bSelected = 0;
			}
		}
	}
	unguard;
}

//
// Select none
//
static int BrushSelectNoneFunc( ABrush* Actor, int Tag )
{
	return 0;
}

//
// Select by CSG operation
//
int BrushSelectOperationFunc( ABrush* Actor, int Tag )
{
	return ((ECsgOper)Actor->CsgOper == Tag) && !(Actor->PolyFlags & (PF_NotSolid | PF_Semisolid));
}
void UEditorEngine::mapSelectOperation(ULevel *Level,ECsgOper CsgOper)
{
	guard(UEditorEngine::mapSelectOperation);
	MapSelect( Level, BrushSelectOperationFunc, CsgOper );
	unguard;
}

int BrushSelectFlagsFunc( ABrush* Actor, int Tag )
{
	return Actor->PolyFlags & Tag;
}
void UEditorEngine::mapSelectFlags(ULevel *Level,DWORD Flags)
	{
	guard(UEditorEngine::mapSelectFlags);					   
	MapSelect( Level, BrushSelectFlagsFunc, (int)Flags );
	unguard;
	};

//
// Select first.
//
void UEditorEngine::mapSelectFirst( ULevel *Level )
{
	guard(UEditorEngine::mapSelectFirst);

	MapSelect( Level, BrushSelectNoneFunc, 0 );

	ABrush* Found=NULL;
	for( FStaticBrushIterator It(Level); It; ++It )
	{
		if( *It != Level->Brush() )
		{
			It->Modify();
			It->bSelected = 1;
			break;
		}
	}
	unguard;
}

//
// Select last.
//
void UEditorEngine::mapSelectLast( ULevel *Level )
{
	guard(UEditorEngine::mapSelectLast);

	MapSelect( Level, BrushSelectNoneFunc, 0 );
	
	ABrush* Found=NULL;
	for( FStaticBrushIterator It(Level); It; ++It )
		Found = *It;

	if( Found )
	{
		Found->Modify();
		Found->bSelected = 1;
	}
	unguard;
}

/*---------------------------------------------------------------------------------------
   Other map brush functions
---------------------------------------------------------------------------------------*/

void CopyBrushEdPolys( UModel *DestBrush, UModel *SourceBrush )
{
	guard(CopyBrushEdPolys);

	// Save all old destination EdPolys for undo.
	DestBrush->Polys->Modify();
	DestBrush->Polys->ModifyAllItems();
	DestBrush->Polys->Empty();
	DestBrush->Polys->Add( SourceBrush->Polys->Num() );
	appMemcpy
	(
		&DestBrush->Polys->Element(0),
		&SourceBrush->Polys->Element(0),
		SourceBrush->Polys->Num() * sizeof(FPoly)
	);
	unguard;
}

//
// Put the first selected brush into the current Brush.
//
void UEditorEngine::mapBrushGet( ULevel* Level )
{
	guard(UEditorEngine::mapBrushGet);
	for( INT i=0; i<Level->Num(); i++ )
	{
		ABrush* Actor = Cast<ABrush>(Level->Actors(i));
		if( Actor && Actor!=Level->Brush() && Actor->bSelected )
		{
			Level->Brush()->Modify();
			CopyBrushEdPolys( Level->Brush()->Brush, Actor->Brush );
			Level->Brush()->CopyPosRotScaleFrom( Actor );
			break;
		}
	}
	unguard;
}

//
// Replace all selected brushes with the current Brush->
//
void UEditorEngine::mapBrushPut( ULevel* Level )
{
	guard(UEditorEngine::mapBrushPut);
	for( INT i=0; i<Level->Num(); i++ )
	{
		ABrush* Actor = Cast<ABrush>(Level->Actors(i));
		if( Actor && Actor!=Level->Brush() && Actor->bSelected )
		{
			Actor->Modify();
			CopyBrushEdPolys( Actor->Brush, Level->Brush()->Brush );
			Actor->CopyPosRotScaleFrom( Level->Brush() );
		}
	}
	unguard;
}

//
// Generic private routine for send to front / send to back
//
void SendTo( ULevel* Level, int bSendToFirst )
{
	guard(SendTo);
	FMemMark Mark(GMem);

	// Setup.
	Level->Modify();
	Level->ModifyAllItems();
	AActor** Lists[2];
	int      Count[2];

	// Partition.
	for( int i=0; i<2; i++ )
	{
		Lists[i] = new(GMem,Level->Num())AActor*;
		Count[i] = 0;
		for( int j=2; j<Level->Num(); j++ )
			if( Level->Actors(j) && (Level->Actors(j)->bSelected ^ bSendToFirst ^ i) )
				Lists[i][Count[i]++] = Level->Actors(j);
	}

	// Refill.
	check(Level->Num()>=2);
	Level->Remove(2,Level->Num()-2);
	for( i=0; i<2; i++ )
		for( int j=0; j<Count[i]; j++ )
			Level->AddItem( Lists[i][j] );

	Mark.Pop();
	unguard;
}

//
// Send all selected brushes in a level to the front of the hierarchy
//
void UEditorEngine::mapSendToFirst(ULevel *Level)
{
	guard(UEditorEngine::mapSendToFirst);
	SendTo( Level, 0 );
	unguard;
}

//
// Send all selected brushes in a level to the back of the hierarchy
//
void UEditorEngine::mapSendToLast(ULevel *Level)
{
	guard(UEditorEngine::mapSendToLast);
	SendTo( Level, 1 );
	unguard;
}

void UEditorEngine::mapSetBrush
(
	ULevel*				Level,
	EMapSetBrushFlags	PropertiesMask,
	_WORD				BrushColor,
	FName				GroupName,
	DWORD				SetPolyFlags,
	DWORD				ClearPolyFlags
)
{
	guard(UEditorEngine::mapSetBrush);
	for( FStaticBrushIterator It(Level); It; ++It )
	{
		UModel* Brush = It->Brush;
		if( *It!=Level->Brush() && It->bSelected )
		{
			It->Modify();
			if( PropertiesMask & MSB_PolyFlags )
				It->PolyFlags = (It->PolyFlags & ~ClearPolyFlags) | SetPolyFlags;
		}
	}
	unguard;
}

/*---------------------------------------------------------------------------------------
   Poly texturing operations
---------------------------------------------------------------------------------------*/

//
// Pan textures on selected polys.  Doesn't do transaction tracking.
//
void UEditorEngine::polyTexPan(UModel *Model,int PanU,int PanV,int Absolute)
	{
	guard(UEditorEngine::polyTexPan);
	for (INT i=0; i<Model->Surfs->Num(); i++)
		{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if (Poly->PolyFlags & PF_Selected)
			{
			if (Absolute)
				{
				Poly->PanU = PanU;
				Poly->PanV = PanV;
				}
			else // Relative
				{
				Poly->PanU += PanU;
				Poly->PanV += PanV;
				};
			polyUpdateMaster (Model,i,0,0);
			};
		Poly++;
		};
	unguard;
	};

//
// Scale textures on selected polys. Doesn't do transaction tracking.
//
void UEditorEngine::polyTexScale( UModel* Model, FLOAT UU, FLOAT UV, FLOAT VU, FLOAT VV, INT Absolute )
{
	guard(UEditorEngine::polyTexScale);

	for( INT i=0; i<Model->Surfs->Num(); i++ )
	{
		FBspSurf *Poly = &Model->Surfs->Element(i);
		if (Poly->PolyFlags & PF_Selected)
		{
			FVector OriginalU = Model->Vectors->Element(Poly->vTextureU);
			FVector OriginalV = Model->Vectors->Element(Poly->vTextureV);

			if( Absolute )
			{
				OriginalU *= 1.0/OriginalU.Size();
				OriginalV *= 1.0/OriginalV.Size();
			}

			// Calc new vectors.
			FVector NewU = OriginalU * UU + OriginalV * UV;
			FVector NewV = OriginalU * VU + OriginalV * VV;

			// Update Bsp poly.
			Poly->vTextureU = bspAddVector (Model,&NewU,0); // Add U vector
			Poly->vTextureV = bspAddVector (Model,&NewV,0); // Add V vector

			// Update generating brush poly.
			polyUpdateMaster( Model, i, 1, 0 );
			Poly->iLightMap = INDEX_NONE;
		}
		Poly++;
	}
	unguard;
}

//
// Align textures on selected polys.  Doesn't do any transaction tracking.
//
void UEditorEngine::polyTexAlign( UModel *Model, ETexAlign TexAlignType, DWORD Texels )
{
	guard(UEditorEngine::polyTexAlign);
	FPoly			EdPoly;
	FVector			Base,Normal,U,V,Temp;
	FModelCoords	Coords,Uncoords;
	FLOAT			Orientation,k;

	for( INT i=0; i<Model->Surfs->Num(); i++ )
	{
		FBspSurf* Poly = &Model->Surfs->Element(i);
		if( Poly->PolyFlags & PF_Selected )
		{
			polyFindMaster( Model, i, EdPoly );
			Normal = Model->Vectors->Element( Poly->vNormal );
			switch( TexAlignType )
			{
				case TEXALIGN_Default:

					Orientation = Poly->Actor->BuildCoords(&Coords,&Uncoords);

					EdPoly.TextureU  = FVector(0,0,0);
					EdPoly.TextureV  = FVector(0,0,0);
					EdPoly.Base      = EdPoly.Vertex[0];
					EdPoly.PanU      = 0;
					EdPoly.PanV      = 0;
					EdPoly.Finalize( 0 );
					EdPoly.Transform( Coords, FVector(0,0,0), FVector(0,0,0), Orientation );

		      		Poly->vTextureU 	= bspAddVector (Model,&EdPoly.TextureU,0);
	      			Poly->vTextureV 	= bspAddVector (Model,&EdPoly.TextureV,0);
					Poly->PanU			= EdPoly.PanU;
					Poly->PanV			= EdPoly.PanV;
					Poly->iLightMap     = INDEX_NONE;

					polyUpdateMaster	(Model,i,1,1);
					break;
				case TEXALIGN_Floor:
					if( Abs(Normal.Z) > 0.05 )
					{
						// Shouldn't change base point, just base U,V.
						Base           	= Model->Points->Element( Poly->pBase );
						Base       		= FVector(0,0,(Base | Normal) / Normal.Z);
			      		Poly->pBase 	= bspAddPoint( Model, &Base, 1 );

						Temp			= FVector(1,0,0);
						Temp			= Temp - Normal * (Temp | Normal);
						Poly->vTextureU	= bspAddVector( Model, &Temp, 0 );

						Temp			= FVector(0,1,0);
						Temp			= Temp - Normal * (Temp | Normal);
						Poly->vTextureV	= bspAddVector( Model, &Temp, 0 );

						Poly->PanU      = 0;
						Poly->PanV      = 0;
						Poly->iLightMap = INDEX_NONE;
					}
					polyUpdateMaster( Model, i, 1, 1 );
					break;
				case TEXALIGN_WallDir:
					if( Abs(Normal.Z)<0.95 )
					{
						U.X = +Normal.Y;
						U.Y = -Normal.X;
						U.Z = 0.0;
						U  *= 1.0/U.Size();
						V   = (U ^ Normal);
						V  *= 1.0/V.Size();

						if( V.Z > 0.0 )
						{
							V *= -1.0;
							U *= -1.0;
						}
						Poly->vTextureU = bspAddVector (Model,&U,0);
						Poly->vTextureV = bspAddVector (Model,&V,0);

						Poly->PanU		= 0;
						Poly->PanV		= 0;
						Poly->iLightMap = INDEX_NONE;

						polyUpdateMaster (Model,i,1,0);
					}
					break;
				case TEXALIGN_WallPan:
					Base = Model->Points ->Element(Poly->pBase);
					U    = Model->Vectors->Element(Poly->vTextureU);
					V    = Model->Vectors->Element(Poly->vTextureV);
					if( Abs(Normal.Z)<0.95 && Abs(V.Z)>0.05 )
					{
						k     = -Base.Z/V.Z;
						V    *= k;
						Base += V;
			      		Poly->pBase = bspAddPoint (Model,&Base,1);
						Poly->iLightMap = INDEX_NONE;

						polyUpdateMaster(Model,i,1,1);
					}
					break;
				case TEXALIGN_OneTile:
					Poly->iLightMap = INDEX_NONE;
					polyUpdateMaster (Model,i,1,1);
					break;
			}
		}
		Poly++;
	}
	unguardf(( "(Type=%i,Texels=%i)", TexAlignType, Texels ));
}

/*---------------------------------------------------------------------------------------
   Map geometry link topic handler
---------------------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Map",MapTopicHandler);
void MapTopicHandler::Get( ULevel *Level, const char *Item, FOutputDevice &Out )
{
	guard(MapTopicHandler::Get);

	int NumBrushes  = 0;
	int NumAdd	    = 0;
	int NumSubtract	= 0;
	int NumSpecial  = 0;
	int NumPolys    = 0;

	for( FStaticBrushIterator It(Level); It; ++It )
	{
		NumBrushes++;
		UModel* Brush        = It->Brush;
		UPolys* BrushEdPolys = Brush->Polys;

		if      (It->CsgOper == CSG_Add)		NumAdd++;
		else if (It->CsgOper == CSG_Subtract)	NumSubtract++;
		else									NumSpecial++;

		NumPolys += BrushEdPolys->Num();
	}

	if     ( appStricmp(Item,"Brushes"      )==0 ) Out.Logf("%i",NumBrushes-1);
	else if( appStricmp(Item,"Add"          )==0 ) Out.Logf("%i",NumAdd);
	else if( appStricmp(Item,"Subtract"     )==0 ) Out.Logf("%i",NumSubtract);
	else if( appStricmp(Item,"Special"      )==0 ) Out.Logf("%i",NumSpecial);
	else if( appStricmp(Item,"AvgPolys"     )==0 ) Out.Logf("%i",NumPolys/Max(1,NumBrushes-1));
	else if( appStricmp(Item,"TotalPolys"   )==0 ) Out.Logf("%i",NumPolys);
	else if( appStricmp(Item,"Points"		)==0 ) Out.Logf("%i",Level->Model->Points->Num());
	else if( appStricmp(Item,"Vectors"		)==0 ) Out.Logf("%i",Level->Model->Vectors->Num());
	else if( appStricmp(Item,"Sides"		)==0 ) Out.Logf("%i",Level->Model->Verts->NumSharedSides);
	else if( appStricmp(Item,"Zones"		)==0 ) Out.Logf("%i",Level->Model->Nodes->NumZones-1);
	else if( appStricmp(Item,"Bounds"		)==0 ) Out.Logf("%i",Level->Model->Bounds.Num());
	else if( appStricmp(Item,"DuplicateBrush")==0 )
	{
		// Duplicate brush.
		for( int i=0; i<Level->Num(); i++ )
			if
			(	Level->Actors(i)
			&&	Level->Actors(i)->IsA(ABrush::StaticClass)
			&&	Level->Actors(i)->bSelected )
			{
				ABrush* Actor    = (ABrush*)Level->Actors(i);
				Actor->Location  = Level->Brush()->Location;
				Actor->Rotation  = Level->Brush()->Rotation;
				Actor->PrePivot  = Level->Brush()->PrePivot;
				GEditor->csgCopyBrush( Actor, Level->Brush(), 0, 0, 1 );
				break;
			}
		debugf( NAME_Log, "Duplicated brush" );
	}
	unguard;
}
void MapTopicHandler::Set(ULevel *Level, const char *Item, const char *Data)
{}

/*---------------------------------------------------------------------------------------
   Polys link topic handler
---------------------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Polys",PolysTopicHandler);
void PolysTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(PolysTopicHandler::Get);
	DWORD		OnFlags,OffFlags;

	int n=0, StaticLights=0, Meshels=0, MeshU=0, MeshV=0;
	OffFlags = (DWORD)~0;
	OnFlags  = (DWORD)~0;
	for( INT i=0; i<Level->Model->Surfs->Num(); i++ )
	{
		FBspSurf *Poly = &Level->Model->Surfs->Element(i);
		if( Poly->PolyFlags&PF_Selected )
		{
			OnFlags  &=  Poly->PolyFlags;
			OffFlags &= ~Poly->PolyFlags;
			n++;
			if( Poly->iLightMap != INDEX_NONE )
			{
				FLightMapIndex& Index = Level->Model->LightMap(Poly->iLightMap);
				Meshels			+= Index.UClamp * Index.VClamp;
				MeshU            = Index.UClamp;
				MeshV            = Index.VClamp;
				if( Index.iLightActors != INDEX_NONE )
					for( int j=0; Level->Model->Lights(j+Index.iLightActors); j++ )
						StaticLights++;
			}
		}
	}
	if      (!appStricmp(Item,"NumSelected"))			Out.Logf("%i",n);
	else if (!appStricmp(Item,"StaticLights"))			Out.Logf("%i",StaticLights);
	else if (!appStricmp(Item,"Meshels"))				Out.Logf("%i",Meshels);
	else if (!appStricmp(Item,"SelectedSetFlags"))		Out.Logf("%u",OnFlags  & ~PF_NoEdit);
	else if (!appStricmp(Item,"SelectedClearFlags"))	Out.Logf("%u",OffFlags & ~PF_NoEdit);
	else if (!appStricmp(Item,"MeshSize") && n==1)		Out.Logf("%ix%i",MeshU,MeshV);

	unguard;
}
void PolysTopicHandler::Set(ULevel *Level, const char *Item, const char *Data)
{
	guard(PolysTopicHandler::Set);
	unguard;
}
