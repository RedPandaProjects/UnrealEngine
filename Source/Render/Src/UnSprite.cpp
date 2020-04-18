/*=============================================================================
	UnSprite.cpp: Unreal sprite rendering functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "RenderPrivate.h"

// Parameters.
#define SPRITE_PROJECTION_FORWARD 32.f /* Move sprite projection planes forward */

/*------------------------------------------------------------------------------
	Dynamics setup and rendering.
------------------------------------------------------------------------------*/

//
// Begin filtering dynamic objects through the top of the Bsp.
//
void URender::SetupDynamics( FSceneNode* Frame, AActor* Exclude )
{
	guard(URender::SetupDynamics);
	if
	(	!(Frame->Level->Model->Nodes->Num())
	||	!(Frame->Viewport->Actor->ShowFlags & SHOW_Actors) )
		return;
	STAT(clock(GStat.FilterTime));
	UBOOL HighDetailActors=Frame->Viewport->RenDev->HighDetailActors;

	// Traverse entire actor list.
	for( INT iActor=0; iActor<Frame->Level->Num(); iActor++ )
	{
		// Add this actor to dynamics if it's renderable.
		AActor* Actor = Frame->Level->Actors(iActor);
		if
		(	Actor
		&&	(!Actor->bHighDetail || HighDetailActors) 
		&&	(Frame->Recursion!=0 || Actor!=Frame->Viewport->Actor->ViewTarget) )
		{
			if
			(	(Actor != Exclude)
			&&	(GIsEditor ? !Actor->bHiddenEd : !Actor->bHidden)
			&&	(!Actor->bOnlyOwnerSee || (Actor->IsOwnedBy(Frame->Viewport->Actor) && !Frame->Viewport->Actor->bBehindView)) )
			{
				// Add the sprite proxy.
				if( !Actor->IsMovingBrush() )
				{
					new(GDynMem)FDynamicSprite( Frame, 0, Actor );
				}
				else if( Frame->Level->BrushTracker )
				{
					Frame->Level->BrushTracker->Update( Actor );
				}
			}
			if
			(	(Actor->LightType)
			&&	(!(Actor->bStatic || Actor->bNoDelete) || Actor->bDynamicLight)
			&&	(Actor->LightBrightness)
			&&	(Actor->LightRadius) )
			{
				// Add the dynamic light.
				FLOAT MaxRadius = Max( Actor->WorldLightRadius(), Actor->WorldVolumetricRadius() );
				for( int i=0; i<4; i++ )
					if( Frame->ViewPlanes[i].PlaneDot(Actor->Location) < -MaxRadius )
						break;
				if( i==4 )
				{
					UBOOL IsVolumetric = Actor->Region.Zone->bFogZone && Actor->VolumeRadius && Actor->VolumeBrightness;
					for( i=0; IsVolumetric && i<4; i++ )
						if( Frame->ViewPlanes[i].PlaneDot(Actor->Location) < -Actor->WorldVolumetricRadius() )
							IsVolumetric = 0;
					new(GDynMem)FDynamicLight( 0, Actor, IsVolumetric, 0 );
					STAT(GStat.DynLightActors++);
				}
			}
		}
	}
	STAT(unclock(GStat.FilterTime));
	unguard;
}

/*------------------------------------------------------------------------------
	FDynamicItem implementation.
------------------------------------------------------------------------------*/

FDynamicItem::FDynamicItem( INT iNode )
{
	guardSlow(FDynamicItem::FDynamicItem);

	if( !GRender->Dynamic(iNode,0) && !GRender->Dynamic(iNode,1) )
		GRender->PostDynamics[GRender->NumPostDynamics++] = &GRender->DynamicsCache[iNode];

	unguardSlow;
}

/*------------------------------------------------------------------------------
	FDynamicSprite implementation.
------------------------------------------------------------------------------*/

FDynamicSprite::FDynamicSprite( FSceneNode* Frame, INT iNode, AActor* InActor )
:	FDynamicItem	( iNode )
,	Actor			( InActor )
,	SpanBuffer		( NULL )
,	RenderNext		( NULL )
,	Volumetrics		( NULL )
,	LeafLights		( NULL )
{
	guardSlow(FDynamicSprite::FDynamicSprite);

	if( Setup( Frame ) )
	{
		// Add at start of list.
		FilterNext = GRender->Dynamic( iNode, 0 );
		GRender->Dynamic( iNode, 0 ) = this;

		// Compute four projection-plane points from sprite extents and viewport.
		FLOAT FloatX1 = X1; 
		FLOAT FloatX2 = X2;
		FLOAT FloatY1 = Y1; 
		FLOAT FloatY2 = Y2;

		// Move closer to prevent actors from slipping into floor.
		FLOAT PlaneZRD	= Z * Frame->RProj.Z;
		FLOAT PlaneX1   = PlaneZRD * (FloatX1 - Frame->FX2);
		FLOAT PlaneX2   = PlaneZRD * (FloatX2 - Frame->FX2);
		FLOAT PlaneY1   = PlaneZRD * (FloatY1 - Frame->FY2);
		FLOAT PlaneY2   = PlaneZRD * (FloatY2 - Frame->FY2);

		// Generate four screen-aligned box vertices.
		ProxyVerts[0].Point = FVector(PlaneX1, PlaneY1, Z).TransformPointBy( Frame->Uncoords );
		ProxyVerts[1].Point = FVector(PlaneX2, PlaneY1, Z).TransformPointBy( Frame->Uncoords );
		ProxyVerts[2].Point = FVector(PlaneX2, PlaneY2, Z).TransformPointBy( Frame->Uncoords );
		ProxyVerts[3].Point = FVector(PlaneX1, PlaneY2, Z).TransformPointBy( Frame->Uncoords );

		// Screen coords.
		ProxyVerts[0].ScreenX = FloatX1; ProxyVerts[0].ScreenY = FloatY1;
		ProxyVerts[1].ScreenX = FloatX2; ProxyVerts[1].ScreenY = FloatY1;
		ProxyVerts[2].ScreenX = FloatX2; ProxyVerts[2].ScreenY = FloatY2;
		ProxyVerts[3].ScreenX = FloatX1; ProxyVerts[3].ScreenY = FloatY2;

		// Generate a full rasterization for this box, which we'll filter down the Bsp.
		//!!inefficient when in box
		check(Y1>=0);
		check(Y2<=Frame->Y);
		check(Y1<Y2);
		FRasterPoly* Raster = (FRasterPoly *)New<BYTE>(GDynMem,sizeof(FRasterPoly) + (Y2-Y1)*sizeof(FRasterSpan));
		Raster->StartY	    = Y1;
		Raster->EndY	    = Y2;

		FRasterSpan* Line = &Raster->Lines[0];
		for( INT i=Raster->StartY; i<Raster->EndY; i++ )
		{
			Line->X[0] = X1;
			Line->X[1] = X2;
			Line++;
		}

		// Add first sprite chunk at end of dynamics list, and cause it to be filtered, since
		// it's being added at the start.
		FDynamicChunk* Chunk = new(GDynMem)FDynamicChunk( iNode, this, Raster );
	}
	STAT(GStat.NumSprites++);
	unguardSlow;
}

UBOOL FDynamicSprite::Setup( FSceneNode* Frame )
{
	guardSlow(FDynamicSprite::Setup);

	// Handle the actor based on its type.
	if( Actor->DrawType==DT_Sprite || Actor->DrawType==DT_SpriteAnimOnce||  (Frame->Viewport->Actor->ShowFlags & SHOW_ActorIcons) )
	{
		// Make sure we have something to draw.
		FLOAT     DrawScale = Actor->DrawScale;
		UTexture* Texture   = Actor->Texture;

		if( Frame->Viewport->Actor->ShowFlags & SHOW_ActorIcons )
		{
			DrawScale = 1.0;
			if( !Texture )
				Texture = GetDefault<AActor>()->Texture;
		}
		if( !Texture )
			return 0;

		// Setup projection plane.
		Z = ((Actor->Location - Frame->Coords.Origin) | Frame->Coords.ZAxis) - SPRITE_PROJECTION_FORWARD;
		if( Z<-2*SPRITE_PROJECTION_FORWARD && !Frame->Viewport->IsOrtho() )
			return 0;

		// See if this is occluded.
		if( !GRender->Project( Frame, Actor->Location, ScreenX, ScreenY, &Persp ) )
			return 0;

		// X extent.
		FLOAT XSize = Persp * DrawScale * Texture->USize;//!!expensive
		X1          = appRound(appCeil(ScreenX-XSize/2));
		X2          = appRound(appCeil(ScreenX+XSize/2));
		if( X1 > X2 )
		{
			Exchange( X1, X2 );
		}
		if( X1 < 0 )
		{
			X1 = 0;
			if( X2 < 0 )
				X2 = 0;
		}
		if( X2 > Frame->X )
		{
			X2 = Frame->X;
			if( X1 > Frame->X )
				X1 = Frame->X;
		}
		if( X2<=0 || X1>=Frame->X-1 )
			return 0;

		// Y extent.
		FLOAT YSize = Persp * DrawScale * Texture->VSize;
		Y1          = appRound(appCeil(ScreenY-YSize/2));
		Y2          = appRound(appCeil(ScreenY+YSize/2));
		if( Y1 > Y2 )
		{
			Exchange( Y1, Y2 );
		}
		if( Y1 < 0 )
		{
			Y1 = 0;
			if( Y2 < 0 )
				Y2 = 0;
		}
		if( Y2 > Frame->Y )
		{
			Y2 = Frame->Y;
			if( Y1 > Frame->Y )
				Y1 = Frame->Y;
		}
		if( Y2<=0 || Y1>=Frame->Y || Y1>=Y2 )
			return 0;
		return 1;
	}
	else if( Actor->DrawType==DT_Mesh )
	{
		// Verify mesh.
		if( !Actor->Mesh )
			return 0;

		// Setup projection plane.
		Z = ((Actor->Location - Frame->Coords.Origin) | Frame->Coords.ZAxis) - SPRITE_PROJECTION_FORWARD;
		if( Z<-2*SPRITE_PROJECTION_FORWARD && !Frame->Viewport->IsOrtho() )
			return 0;

		FScreenBounds ScreenBounds;
		FBox Bounds = Actor->Mesh->GetRenderBoundingBox( Actor, 0 );
		if( !GRender->BoundVisible( Frame, &Bounds, NULL, ScreenBounds ) )
			return 0;

		X1 = ScreenBounds.MinX;
		X2 = ScreenBounds.MaxX;
		Y1 = ScreenBounds.MinY;
		Y2 = ScreenBounds.MaxY;
		if( Y1>=Y2 )
			return 0;

		return 1;
	}
	else return 0;
	unguardSlow;
}

/*------------------------------------------------------------------------------
	FDynamicChunk implementation.
------------------------------------------------------------------------------*/

FDynamicChunk::FDynamicChunk( INT iNode, FDynamicSprite* InSprite, FRasterPoly* InRaster )
:	FDynamicItem	( iNode )
,	Raster			( InRaster )
,	Sprite			( InSprite )
{
	guardSlow(FDynamicChunk::FDynamicChunk);

	// Add at start of list.
	FilterNext = GRender->Dynamic( iNode, 0 );
	GRender->Dynamic( iNode, 0 ) = this;

	STAT(GStat.NumChunks++);
	unguardSlow;
}

void FDynamicChunk::Filter( UViewport* Viewport, FSceneNode* Frame, INT iNode, INT Outside )
{
	guardSlow(FDynamicChunk::Filter);
	FBspNode& Node = Frame->Level->Model->Nodes->Element(iNode);

	// Setup.
	FRasterPoly *FrontRaster, *BackRaster;

	// Find point-to-plane distances for all four vertices (side-of-plane classifications).
	INT Front=0, Back=0;
	FLOAT Dist[4];
	for( INT i=0; i<4; i++ )
	{
		Dist[i] = Node.Plane.PlaneDot( Sprite->ProxyVerts[i].Point );
		Front  += Dist[i] > +0.01;
		Back   += Dist[i] < -0.01;
	}
	if( Front && Back )
	{	
		// Find intersection points.
		FTransform	Intersect[4];
		FTransform* I  = &Intersect	         [0];
		FTransform* V1 = &Sprite->ProxyVerts [3]; 
		FTransform* V2 = &Sprite->ProxyVerts [0];
		FLOAT*      D1 = &Dist			     [3];
		FLOAT*      D2 = &Dist			     [0];
		INT			NumInt = 0;

		for( INT i=0; i<4; i++ )
		{
			if( (*D1)*(*D2) < 0.0 )
			{	
				// At intersection point.
				FLOAT Alpha = *D1 / (*D1 - *D2);
				I->ScreenX  = V1->ScreenX + Alpha * (V2->ScreenX - V1->ScreenX);
				I->ScreenY  = V1->ScreenY + Alpha * (V2->ScreenY - V1->ScreenY);

				I++;
				NumInt++;
			}
			V1 = V2++;
			D1 = D2++;
		}
		if( NumInt < 2 )
			goto NoSplit;

		// Allocate front and back rasters.
		INT	Size	= sizeof (FRasterPoly) + (Raster->EndY - Raster->StartY) * sizeof( FRasterSpan );
		FrontRaster	= (FRasterPoly *)New<BYTE>(GDynMem,Size);
		BackRaster	= (FRasterPoly *)New<BYTE>(GDynMem,Size);

		// Make sure that first intersection point is on top.
		if( Intersect[0].ScreenY > Intersect[1].ScreenY )
			Exchange( Intersect[0], Intersect[1] );
		INT Y0 = Max( appFloor(Intersect[0].ScreenY), Raster->StartY );
		INT Y1 = Min( appFloor(Intersect[1].ScreenY), Raster->EndY   );
		if( Y0>Y1 )
			goto NoSplit;

		// Find TopRaster.
		FRasterPoly* TopRaster = NULL;
		if( Y0 > Raster->StartY )
		{
			if( Dist[0] >= 0 ) TopRaster = FrontRaster;
			else               TopRaster = BackRaster;
		}

		// Find BottomRaster.
		FRasterPoly* BottomRaster = NULL;
		if( Y1 < Raster->EndY )
		{
			if( Dist[2] >= 0 ) BottomRaster = FrontRaster;
			else               BottomRaster = BackRaster;
		}

		// Find LeftRaster and RightRaster.
		FRasterPoly *LeftRaster, *RightRaster;
		if( Intersect[1].ScreenX >= Intersect[0].ScreenX )
		{
			if (Dist[1] >= 0.0) {LeftRaster = BackRaster;  RightRaster = FrontRaster;}
			else	   			{LeftRaster = FrontRaster; RightRaster = BackRaster; };
		}
		else // Intersect[1].ScreenX < Intersect[0].ScreenX
		{
			if (Dist[0] >= 0.0) {LeftRaster = FrontRaster; RightRaster = BackRaster; }
			else                {LeftRaster = BackRaster;  RightRaster = FrontRaster;};
		}

		// Set left and right raster defaults (may be overwritten by TopRaster or BottomRaster).
		debug(Y0>=0);
		debug(Y1<=Frame->Y);
		LeftRaster->StartY = Y0; RightRaster->StartY = Y0;
		LeftRaster->EndY   = Y1; RightRaster->EndY   = Y1;

		// Copy TopRaster section.
		if( TopRaster )
		{
			TopRaster->StartY = Raster->StartY;

			FRasterSpan* SourceLine	= &Raster->Lines    [0];
			FRasterSpan* Line		= &TopRaster->Lines [0];

			for( i=TopRaster->StartY; i<Y0; i++ )
				*Line++ = *SourceLine++;
		}

		// Copy BottomRaster section.
		if( BottomRaster )
		{
			BottomRaster->EndY = Raster->EndY;

			FRasterSpan* SourceLine	= &Raster->Lines       [Y1 - Raster->StartY];
			FRasterSpan* Line       = &BottomRaster->Lines [Y1 - BottomRaster->StartY];

			for( i=Y1; i<BottomRaster->EndY; i++ )
				*Line++ = *SourceLine++;
		}

		// Split middle raster section.
		if( Y1 != Y0 )
		{
			FLOAT	FloatYAdjust	= (FLOAT)Y0 + 1.0 - Intersect[0].ScreenY;
			FLOAT	FloatFixDX 		= 65536.0 * (Intersect[1].ScreenX - Intersect[0].ScreenX) / (Intersect[1].ScreenY - Intersect[0].ScreenY);
			INT		FixDX			= FloatFixDX;
			INT		FixX			= 65536.0 * Intersect[0].ScreenX + FloatFixDX * FloatYAdjust;

			if( Raster->StartY > Y0 ) 
			{
				FixX   += (Raster->StartY-Y0) * FixDX;
				Y0		= Raster->StartY;
			}
			if( Raster->EndY < Y1 )
			{
				Y1      = Raster->EndY;
			}
			
			FRasterSpan	*SourceLine = &Raster->Lines      [Y0 - Raster->StartY];
			FRasterSpan	*LeftLine   = &LeftRaster->Lines  [Y0 - LeftRaster->StartY];
			FRasterSpan	*RightLine  = &RightRaster->Lines [Y0 - RightRaster->StartY];

			while( Y0++ < Y1 )
			{
				*LeftLine  = *SourceLine;
				*RightLine = *SourceLine;

				INT X = Unfix(FixX);
				if (X < LeftLine->X[1])    LeftLine->X[1] = X;
				if (X > RightLine->X[0]) RightLine->X[0] = X;

				FixX       += FixDX;
				SourceLine ++;
				LeftLine   ++;
				RightLine  ++;
			}
		}

		// Discard any rasters that are completely empty.
		if( BackRaster->EndY <= BackRaster->StartY )
			BackRaster = NULL;
		if( FrontRaster->EndY <= FrontRaster->StartY )
			FrontRaster = NULL;
	}
	else
	{
		// Don't have to split the rasterization.
		NoSplit:
		FrontRaster = BackRaster = Raster;
	}

	// Filter it down.
	INT CSG = Node.IsCsg();
	if( Front && FrontRaster )
	{
		if( Node.iFront != INDEX_NONE )
			new(GDynMem)FDynamicChunk( Node.iFront, Sprite, FrontRaster );
		else if( Outside || CSG )
			new(GDynMem)FDynamicFinalChunk( iNode, Sprite, FrontRaster, 0 );
	}
	if( Back && BackRaster )
	{
		if( Node.iBack != INDEX_NONE  )
			new(GDynMem)FDynamicChunk( Node.iBack, Sprite, BackRaster );
		else if( Outside && !CSG )
			new(GDynMem)FDynamicFinalChunk( iNode, Sprite, BackRaster, 1 );
	}
	unguardSlow;
}

/*------------------------------------------------------------------------------
	FDynamicFinalChunk implementation.
------------------------------------------------------------------------------*/

FDynamicFinalChunk::FDynamicFinalChunk( INT iNode, FDynamicSprite* InSprite, FRasterPoly* InRaster, INT IsBack )
:	FDynamicItem( iNode )
,	Raster( InRaster )
,	Sprite( InSprite )
{
	guardSlow(FDynamicFinalChunk::FDynamicFinalChunk);

	// Set Z.
	Z = InSprite->Z;

	// Add into list z-sorted.
	for( FDynamicItem** Item=&GRender->Dynamic( iNode, IsBack ); *Item && (*Item)->Z<Z; Item=&(*Item)->FilterNext );
	FilterNext = *Item;
	*Item      = this;

	STAT(GStat.NumFinalChunks++);
	unguardSlow;
}

void FDynamicFinalChunk::PreRender( UViewport* Viewport, FSceneNode* Frame, FSpanBuffer* SpanBuffer, INT iNode, FVolActorLink* Volumetrics )
{
	guardSlow(FDynamicFinalChunk::PreRender);
	UBOOL Drawn=0;
	if( !Sprite->SpanBuffer )
	{
		// Creating a new span buffer for this sprite.
		Sprite->SpanBuffer = New<FSpanBuffer>(GDynMem);
		Sprite->SpanBuffer->AllocIndex( Raster->StartY, Raster->EndY, &GDynMem );

		if( Sprite->SpanBuffer->CopyFromRaster( *SpanBuffer, Raster->StartY, Raster->EndY, (FRasterSpan*)Raster->Lines ) )
		{
			// Span buffer is non-empty, so keep it and put it on the to-draw list.
			STAT(GStat.ChunksDrawn++);
			Drawn                = 1;
			Sprite->RenderNext	 = Frame->Sprite;
			Frame->Sprite        = Sprite;
		}
		else
		{
			// Span buffer is empty, so ditch it.
			Sprite->SpanBuffer->Release();
			Sprite->SpanBuffer = NULL;
		}
	}
	else
	{
		// Merging with the sprite's existing span buffer.
		FMemMark Mark(GMem);
		FSpanBuffer* Span = New<FSpanBuffer>(GMem);
		Span->AllocIndex(Raster->StartY,Raster->EndY,&GMem);
		if( Span->CopyFromRaster( *SpanBuffer, Raster->StartY, Raster->EndY, (FRasterSpan*)Raster->Lines ) )
		{
			// Temporary span buffer is non-empty, so merge it into sprite's.
			Drawn = 1;
			Sprite->SpanBuffer->MergeWith(*Span);
			STAT(GStat.ChunksDrawn++);
		}

		// Release the temporary memory.
		Mark.Pop();
	}

	// Add volumetrics to list.
	if( Drawn )
	{
		for( Volumetrics; Volumetrics; Volumetrics=Volumetrics->Next )
		{
			if( Volumetrics->Volumetric )
			{
				for( FActorLink* Link=Sprite->Volumetrics; Link; Link=Link->Next )
					if( Link->Actor==Volumetrics->Actor )
						break;
				if( !Link )
					Sprite->Volumetrics = new(GDynMem)FActorLink(Volumetrics->Actor,Sprite->Volumetrics);
			}
		}
	}

	unguardSlow;
}

/*-----------------------------------------------------------------------------
	FDynamicLight implementation.
-----------------------------------------------------------------------------*/

FDynamicLight::FDynamicLight( INT iNode, AActor* InActor, UBOOL InIsVol, UBOOL InHitLeaf )
:	FDynamicItem( iNode )
,	Actor( InActor )
,	IsVol( InIsVol )
,	HitLeaf( InHitLeaf )
{
	guardSlow(FDynamicLight::FDynamicLight);

	// Add at start of list.
	FilterNext = GRender->Dynamic( iNode, 0 );
	GRender->Dynamic( iNode, 0 ) = this;

	STAT(GStat.NumMovingLights++);
	unguardSlow;
}

void FDynamicLight::Filter( UViewport* Viewport, FSceneNode* Frame, INT iNode, INT Outside )
{
	guardSlow(FDynamicLight::Filter);

	// Filter down.
	FBspNode& Node = Viewport->Actor->XLevel->Model->Nodes->Element(iNode);
	FLOAT Dist   = Node.Plane.PlaneDot( Actor->Location );
	FLOAT Radius = Actor->WorldLightRadius();
	if( Dist > -Radius )
	{
		// Filter down front.
		UBOOL ThisHitLeaf=HitLeaf;
		if( !HitLeaf )
		{
			INT iLeaf=Node.iLeaf[1];
			if( iLeaf!=INDEX_NONE )
			{
				if( !GRender->LeafLights[iLeaf] )
					GRender->DynLightLeaves[GRender->NumDynLightLeaves++] = iLeaf;
				GRender->LeafLights[iLeaf] = new( GMem )FVolActorLink( Frame->Coords, Actor, GRender->LeafLights[iLeaf], IsVol && Dist>-Actor->WorldVolumetricRadius() );
				ThisHitLeaf=1;
			}
		}
		if( Node.iFront!=INDEX_NONE )
			new(GDynMem)FDynamicLight( Node.iFront, Actor, IsVol && Dist>-Actor->WorldVolumetricRadius(), ThisHitLeaf );

		// Handle planars.
		if( Dist < Radius )
		{
			for( INT iPlane=iNode; iPlane!=INDEX_NONE; iPlane = Viewport->Actor->XLevel->Model->Nodes->Element(iPlane).iPlane )
			{
				FBspNode&       Node  = Viewport->Actor->XLevel->Model->Nodes->Element(iPlane);
				FBspSurf&       Surf  = Viewport->Actor->XLevel->Model->Surfs->Element(Node.iSurf);
				FLightMapIndex* Index = Viewport->Actor->XLevel->Model->GetLightMapIndex(Node.iSurf);

				if
				(	(Index)
				&&	(GRender->NumDynLightSurfs < URender::MAX_DYN_LIGHT_SURFS)
				&&	(Actor->bSpecialLit ? (Surf.PolyFlags&PF_SpecialLit) : !(Surf.PolyFlags&PF_SpecialLit)) )
				{
					// Don't apply a light twice.
					for( FActorLink* Link = GRender->SurfLights[Node.iSurf]; Link; Link=Link->Next )
						if( Link->Actor == Actor )
							break;
					if( !Link )
					{
						if( !GRender->SurfLights[Node.iSurf] )
							GRender->DynLightSurfs[GRender->NumDynLightSurfs++] = Node.iSurf;
						GRender->SurfLights[Node.iSurf] = new(GMem)FActorLink( Actor, GRender->SurfLights[Node.iSurf] );
					}
				}
			}
		}
	}
	if( Dist < Radius )
	{
		UBOOL ThisHitLeaf=HitLeaf;
		if( !HitLeaf )
		{
			INT iLeaf=Node.iLeaf[0];
			if( iLeaf!=INDEX_NONE )
			{
				if( !GRender->LeafLights[iLeaf] )
					GRender->DynLightLeaves[GRender->NumDynLightLeaves++] = iLeaf;
				GRender->LeafLights[iLeaf] = new( GMem )FVolActorLink( Frame->Coords, Actor, GRender->LeafLights[iLeaf], IsVol && Dist<Actor->WorldVolumetricRadius() );
				ThisHitLeaf=1;
			}
		}
		if( Node.iBack!=INDEX_NONE )
			new(GDynMem)FDynamicLight( Node.iBack, Actor, IsVol && Dist<Actor->WorldVolumetricRadius(), ThisHitLeaf );
	}
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	Actor drawing.
-----------------------------------------------------------------------------*/

//
// Get inherent poly flags for an actor.
//
static DWORD GetPolyFlags( FSceneNode* Frame, AActor* Owner )
{
	guard(GetPolyFlags);
	DWORD PolyFlags=0;

	if     ( Owner->Style==STY_Masked      ) PolyFlags |= PF_Masked;
	else if( Owner->Style==STY_Translucent ) PolyFlags |= PF_Translucent;
	else if( Owner->Style==STY_Modulated   ) PolyFlags |= PF_Modulated;

	if	   ( Owner->bNoSmooth              ) PolyFlags |= PF_NoSmooth;
	if     ( Owner->bSelected              ) PolyFlags |= PF_Selected;
	if     ( Owner->bMeshEnviroMap         ) PolyFlags |= PF_Environment;
	if     (!Owner->bMeshCurvy             ) PolyFlags |= PF_Flat;
	if     ( Owner->bUnlit || Owner->Region.ZoneNumber==0 || Frame->Viewport->Actor->RendMap!=REN_DynLight || Frame->Viewport->Client->NoLighting ) PolyFlags |= PF_Unlit;

	return PolyFlags;
	unguard;
}

//
// Draw an actor defined by a FDynamicSprite.
//
void URender::DrawActorSprite( FSceneNode* Frame, FDynamicSprite* Sprite )
{
	guard(URender::DrawActorSprite);
	PUSH_HIT(Frame,HActor, Sprite->Actor);
	DWORD PolyFlags = GetPolyFlags(Frame,Sprite->Actor);

	// Draw the actor.
	if
	(	(Sprite->Actor->DrawType==DT_Sprite || Sprite->Actor->DrawType==DT_SpriteAnimOnce || (Frame->Viewport->Actor->ShowFlags & SHOW_ActorIcons)) 
	&&	(Sprite->Actor->Texture) )
	{
		// Sprite.
		guard(DrawSprite);
		FPlane    Color     = (GIsEditor && Sprite->Actor->bSelected) ? FPlane(.5,.9,.5,0) : FPlane(1,1,1,0);
		UTexture* Texture   = Sprite->Actor->Texture;
		FLOAT     DrawScale = Sprite->Actor->DrawScale;
		UTexture* SavedNext = NULL;
		UTexture* SavedCur  = NULL;
		if( Sprite->Actor->ScaleGlow!=1.0 )
		{
			Color *= Sprite->Actor->ScaleGlow;
			if( Color.R>1.0 ) Color.R=1.0;
			if( Color.G>1.0 ) Color.G=1.0;
			if( Color.B>1.0 ) Color.B=1.0;
		}
		if( Sprite->Actor->DrawType==DT_SpriteAnimOnce )
		{
			INT Count=1;
			for( UTexture* Test=Texture->AnimNext; Test && Test!=Texture; Test=Test->AnimNext )
				Count++;
			INT Num = Clamp( appFloor(Sprite->Actor->LifeFraction()*Count), 0, Count-1 );
			while( Num-- > 0 )
				Texture = Texture->AnimNext;
			SavedNext         = Texture->AnimNext;//sort of a hack!!
			SavedCur          = Texture->AnimCur;
			Texture->AnimNext = NULL;
			Texture->AnimCur  = NULL;
		}
		if( Frame->Viewport->Actor->ShowFlags & SHOW_ActorIcons )
		{
			DrawScale = 1.0;
			if( !Texture )
				Texture = GetDefault<AActor>()->Texture;
		}
		FLOAT XScale = Sprite->Persp * DrawScale * Texture->USize;
		FLOAT YScale = Sprite->Persp * DrawScale * Texture->VSize;
		if( Texture ) Frame->Viewport->Canvas->DrawIcon
		(
			Texture->Get( Frame->Viewport->CurrentTime ),
			Sprite->ScreenX - XScale/2,
			Sprite->ScreenY - YScale/2,
			XScale,
			YScale,
			Sprite->SpanBuffer,
			Sprite->Z,
			Color,
			FPlane(0,0,0,0),
			PolyFlags | PF_TwoSided | Texture->PolyFlags
		);
		if( Sprite->Actor->DrawType==DT_SpriteAnimOnce )
		{
			Texture->AnimNext = SavedNext;
			Texture->AnimCur  = SavedCur;
		}
		unguard;
	}
	else if
	(	Sprite->Actor->DrawType==DT_Mesh
	&&	Sprite->Actor->Mesh )
	{
		// Mesh.
		guard(DrawMesh);
		if( Frame->Viewport->Actor->RendMap==REN_Polys || Frame->Viewport->Actor->RendMap==REN_PolyCuts || Frame->Viewport->Actor->RendMap==REN_Zones || Frame->Viewport->Actor->RendMap==REN_Wire )
			PolyFlags |= PF_FlatShaded;
		DrawMesh
		(
			Frame,
			Sprite->Actor,
			Sprite->SpanBuffer,
			Sprite->Actor->Region.Zone,
			Frame->Coords,
			Sprite->LeafLights,
			Sprite->Volumetrics,
			PolyFlags
		);
		extern UBOOL HasSpecialCoords;
		extern FCoords SpecialCoords;
		if( HasSpecialCoords && Sprite->Actor->IsA(APawn::StaticClass) && ((APawn*)Sprite->Actor)->Weapon )
		{
			AInventory* Weapon = ((APawn*)Sprite->Actor)->Weapon;
			if( Weapon->ThirdPersonMesh )
			{
				Exchange( Weapon->ThirdPersonMesh, Weapon->Mesh );
				Exchange( Weapon->ThirdPersonScale, Weapon->DrawScale );
				Weapon->Rotation = FRotator(0,0,0);
				FLOAT Mirror  = Frame->Mirror;
				Frame->Mirror = 1;
				DrawMesh
				(
					Frame,
					Weapon,
					Sprite->SpanBuffer,
					Sprite->Actor->Region.Zone,
					SpecialCoords / Weapon->Rotation / Weapon->Location,
					Sprite->LeafLights,
					Sprite->Volumetrics,
					GetPolyFlags(Frame,Weapon)
				);
				Frame->Mirror = Mirror;
				Exchange( Weapon->ThirdPersonMesh, Weapon->Mesh );
				Exchange( Weapon->ThirdPersonScale, Weapon->DrawScale );
			}
		}
		unguard;
	}

	// Done.
	POP_HIT(Frame);
	unguard;
}

/*-----------------------------------------------------------------------------
	Wireframe view drawing.
-----------------------------------------------------------------------------*/

//
// Just draw an actor, no span occlusion.
//
void URender::DrawActor( FSceneNode* Frame, AActor* Actor)
{
	guard(URender::DrawActor);
	FDynamicSprite Sprite(Actor);
	if( Sprite.Setup( Frame ) )
		DrawActorSprite( Frame, &Sprite );
	unguard;
}

/*------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------*/
