/*=============================================================================
	UnEdClick.cpp: Editor click-detection code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Adding actors.
-----------------------------------------------------------------------------*/

void UEditorEngine::AddActor( ULevel* Level, UClass* Class, FVector V )
{
	guard(AddActor);
	check(Class);
	debugf(NAME_Log,"addactor");

	// Validate everything.
	if( Class->ClassFlags & CLASS_Abstract )
	{
		GSystem->Warnf( "Class %s is abstract.  You can't add actors of this class to the world.", Class->GetName() );
		return;
	}
	else if( Class->ClassFlags & CLASS_Transient )
	{
		GSystem->Warnf( "Class %s is transient.  You can't add actors of this class in UnrealEd.", Class->GetName() );
		return;
	}

	// Transactionally add the actor.
	Trans->Begin( Level, "Add Actor" );
	SelectNone( Level, 0 );
	Level->Modify();
	AActor* Actor = Level->SpawnActor( Class, NAME_None, NULL, NULL, V );
	if( Actor )
	{
		Actor->bDynamicLight = 1;
		Actor->bSelected     = 1;
		if( !Level->FarMoveActor( Actor, V ) )//necessary??!!
		{
			GSystem->Warnf( "Actor doesn't fit there" );
			Level->DestroyActor( Actor );
		}
		else debugf( NAME_Log, "Added actor successfully" );
		if( Class->GetDefaultActor()->IsBrush() )
			csgCopyBrush( (ABrush*)Actor, (ABrush*)Class->GetDefaultActor(), 0, 0, 1 );
		Actor->PostEditMove();
	}
	else GSystem->Warnf( "Actor doesn't fit there" );
	Trans->End();

	NoteSelectionChange( Level );
	unguard;
}

/*-----------------------------------------------------------------------------
	HTextureView.
-----------------------------------------------------------------------------*/

void HTextureView::Click( const FHitCause& Cause )
{
	guard(HTextureView::Click);
	check(Texture);
	Texture->Click( Cause.Buttons, Cause.MouseX*Texture->USize/ViewX, Cause.MouseY*Texture->VSize/ViewY );
	unguard;
}

/*-----------------------------------------------------------------------------
	HBackdrop.
-----------------------------------------------------------------------------*/

void HBackdrop::Click( const FHitCause& Cause )
{
	guard(HBackdrop::Click);
	GEditor->ClickLocation = Location;
	GEditor->ClickPlane    = FPlane(0,0,0,0);
	if( (Cause.Buttons&MOUSE_Left) && Cause.Viewport->Input->KeyDown(IK_A) )
	{
		if( GEditor->CurrentClass )
		{
			char Cmd[256];
			appSprintf( Cmd, "ACTOR ADD CLASS=%s", GEditor->CurrentClass->GetName() );
			GEditor->Exec( Cmd );
		}
	}
	else if( (Cause.Buttons&MOUSE_Left) && Cause.Viewport->Input->KeyDown(IK_L) )
	{
		GEditor->Exec( "ACTOR ADD CLASS=LIGHT" );
	}
	else if( Cause.Buttons & MOUSE_Right )
	{
		if( Cause.Viewport->IsOrtho() )
		{
			GEditor->EdCallback( EDC_RtClickWindowCanAdd, 0 );
		}
		else GEditor->EdCallback( EDC_RtClickWindow, 0 );
	}
	else if( Cause.Buttons & MOUSE_Left )
	{
		if( !(Cause.Buttons & MOUSE_Ctrl) )
		{
			GEditor->Trans->Begin( Cause.Viewport->Actor->XLevel, "Select None" );
			GEditor->SelectNone( Cause.Viewport->Actor->XLevel, 1 );
			GEditor->Trans->End();
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	FEditorHitObserver implementation.
-----------------------------------------------------------------------------*/

void FEditorHitObserver::Click( const FHitCause& Cause, const struct HBspSurf& Hit )
{
	guard(FEditorHitObserver::ClickHBspSurf);
	UModel*   Model = Cause.Viewport->Actor->XLevel->Model;
	FBspSurf& Surf  = Model->Surfs->Element(Hit.iSurf);

	// Adding actor.
	check(Hit.Parent);
	check(Hit.Parent->IsA("HCoords"));
	HCoords* HitCoords     = (HCoords*)Hit.Parent;
	FPlane	Plane		   = FPlane(Model->Points->Element(Surf.pBase),Model->Vectors->Element(Surf.vNormal));
	GEditor->ClickLocation = FLinePlaneIntersection( HitCoords->Coords.Origin, HitCoords->Coords.Origin + HitCoords->Direction, Plane );
	GEditor->ClickPlane    = Plane;

	// Remember hit location for actor-adding.
	if( (Cause.Buttons&MOUSE_Left) && Cause.Viewport->Input->KeyDown(IK_A) )
	{
		if( GEditor->CurrentClass )
		{
			char Cmd[256];
			appSprintf( Cmd, "ACTOR ADD CLASS=%s", GEditor->CurrentClass->GetName() );
			GEditor->Exec( Cmd );
		}
	}
	else if( (Cause.Buttons&MOUSE_Left) && Cause.Viewport->Input->KeyDown(IK_L) )
	{
		GEditor->Exec( "ACTOR ADD CLASS=LIGHT" );
	}
	else if( (Cause.Buttons&MOUSE_Alt) && (Cause.Buttons&MOUSE_Right) )
	{
		// Grab the texture.
		GEditor->CurrentTexture = Surf.Texture;
		GEditor->EdCallback( EDC_CurTexChange, 0 );
	}
	else if( (Cause.Buttons & MOUSE_Shift) && (Cause.Buttons&MOUSE_Left) )
	{
		// Apply texture to all selected.
		GEditor->Trans->Begin( Cause.Viewport->Actor->XLevel, "apply texture to selected surfaces" );
		for( INT i=0; i<Model->Surfs->Num(); i++ )
		{
			if( Model->Surfs->Element(i).PolyFlags & PF_Selected )
			{
				Model->Surfs->ModifyItem( i, 1 );
				Model->Surfs->Element(i).Texture = GEditor->CurrentTexture;
				GEditor->polyUpdateMaster( Model, i, 0, 0 );
			}
		}
		GEditor->Trans->End();
	}
	else if( (Cause.Buttons&MOUSE_Alt) && (Cause.Buttons&MOUSE_Left) )
	{
		// Apply texture to the one polygon clicked on.
		GEditor->Trans->Begin( Cause.Viewport->Actor->XLevel, "apply texture to surface" );
		Model->Surfs->ModifyItem( Hit.iSurf, 0 );
		Surf.Texture = GEditor->CurrentTexture;
		GEditor->polyUpdateMaster( Model, Hit.iSurf, 0, 0 );
		GEditor->Trans->End();
	}
	else if( Cause.Buttons & MOUSE_Right ) 
	{
		// Edit surface properties.
		GEditor->Trans->Begin( Cause.Viewport->Actor->XLevel, "select surface for editing" );
		Model->Surfs->ModifyItem( Hit.iSurf, 0 );
		Surf.PolyFlags |= PF_Selected;
		GEditor->NoteSelectionChange( Cause.Viewport->Actor->XLevel );
		GEditor->EdCallback( EDC_RtClickPoly, 0 );
		GEditor->Trans->End();
	}
	else
	{
		// Select or deselect surfaces.
		GEditor->Trans->Begin( Cause.Viewport->Actor->XLevel, "select surfaces" );
		DWORD SelectMask = Surf.PolyFlags & PF_Selected;
		if( !(Cause.Buttons & MOUSE_Ctrl) )
			GEditor->SelectNone( Cause.Viewport->Actor->XLevel, 0 );
		Model->Surfs->ModifyItem( Hit.iSurf, 0 );
		Surf.PolyFlags = (Surf.PolyFlags & ~PF_Selected) | (SelectMask ^ PF_Selected);
		GEditor->NoteSelectionChange( Cause.Viewport->Actor->XLevel );
		GEditor->Trans->End();
	}
	unguard;
}
void FEditorHitObserver::Click( const FHitCause& Cause, const struct HActor& Hit )
{
	guard(FEditorHitObserver::ClickHActor);

	// Handle selection.
	GEditor->Trans->Begin(Cause.Viewport->Actor->XLevel,"clicking on actors");
	if( Cause.Buttons & MOUSE_Right )
	{
		// Bring up properties of this actor and other selected actors.
		Hit.Actor->Modify();
		Hit.Actor->bSelected = 1;
		GEditor->NoteSelectionChange( Cause.Viewport->Actor->XLevel );
		GEditor->EdCallback( EDC_RtClickActor, 0 );
	}
	else if( Cause.Buttons & MOUSE_LeftDouble )
	{
		if( !(Cause.Buttons & MOUSE_Ctrl) )
			GEditor->SelectNone( Cause.Viewport->Actor->XLevel, 0 );
		Hit.Actor->Modify();
		Hit.Actor->bSelected = 1;
		GEditor->NoteSelectionChange( Cause.Viewport->Actor->XLevel );
		GEditor->Exec( "HOOK ACTORPROPERTIES" );
	}
	else
	{
		// Toggle actor selection.
		Hit.Actor->Modify();
		if( Cause.Buttons & MOUSE_Ctrl )
		{
			Hit.Actor->bSelected ^= 1;
		}
		else
		{
			GEditor->SelectNone( Cause.Viewport->Actor->XLevel, 0 );
			Hit.Actor->bSelected = 1;
		}
		GEditor->NoteSelectionChange( Cause.Viewport->Actor->XLevel );
	}
	GEditor->Trans->End();
	unguard;
}
void FEditorHitObserver::Click( const FHitCause& Cause, const struct HBrushVertex& Hit )
{
	guard(FEditorHitObserver::ClickHBrushVertex);

	// Set new pivot point.
	GEditor->Trans->Begin( GEditor->Level, "brush vertex selection" );
	GEditor->SetPivot( Hit.Location, (Cause.Buttons&MOUSE_Right)!=0, 1 );
	GEditor->Trans->End();

	unguard;
}
void FEditorHitObserver::Click( const FHitCause& Cause, const struct HGlobalPivot& Hit )
{
	guard(FEditorHitObserver::ClickHGlobalPivot);

	// Set new pivot point.
	GEditor->Trans->Begin( GEditor->Level, "brush vertex selection" );
	GEditor->SetPivot( Hit.Location, (Cause.Buttons&MOUSE_Right)!=0, 1 );
	GEditor->Trans->End();

	unguard;
}
void FEditorHitObserver::Click( const FHitCause& Cause, const struct HBrowserTexture& Hit )
{
	guard(FEditorHitObserver::ClickHBrowserTexture);
	if( Cause.Buttons==MOUSE_Left )
	{
		// Select textures.
		char Temp[256];
		appSprintf( Temp, "POLY DEFAULT TEXTURE=%s", Hit.Texture->GetName() );
		GEditor->Exec( Temp );
		appSprintf( Temp, "POLY SET TEXTURE=%s", Hit.Texture->GetName() );
		GEditor->Exec( Temp );
		GEditor->EdCallback( EDC_CurTexChange, 0 );
	}
	else if( Cause.Buttons==MOUSE_Right )
	{
		// Bring up texture popup menu.
		GEditor->CurrentTexture = Hit.Texture;
		GEditor->EdCallback( EDC_RtClickTexture, 0 );
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
