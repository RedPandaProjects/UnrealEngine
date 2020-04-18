/*=============================================================================
	UnEdAct.cpp: Unreal editor actor-related functions
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"

#pragma DISABLE_OPTIMIZATION /* Not performance-critical */

/*-----------------------------------------------------------------------------
   Actor adding/deleting functions.
-----------------------------------------------------------------------------*/

//
// Copy selected actors to the clipboard.
//
void UEditorEngine::edactCopySelected( ULevel* Level )
{
	guard(UEditorEngine::edactCopySelected);

	// Export the actors.
	FStringOut Out;
	Level->Export( Out, "copy", 0 );
	ClipboardCopy( *Out );

	unguard;
}

//
// Paste selected actors from the clipboard.
//
void UEditorEngine::edactPasteSelected( ULevel* Level )
{
	guard(UEditorEngine::edactPasteSelected);

	// Get pasted text.
	FString PasteString;
	ClipboardPaste( PasteString );
	const char* Paste = *PasteString;

	// Import the actors.
	Level->RememberActors();
	ULevelFactory* Factory = new ULevelFactory;
	Factory->Create( ULevel::StaticClass, Level->GetParent(), Level->GetName(), NULL, "paste", Paste, Paste+appStrlen(Paste), GSystem );
	delete Factory;
	GCache.Flush();
	Level->ReconcileActors();
	ResetSound();

	// Offset them.
	for( INT i=0; i<Level->Num(); i++ )
		if( Level->Element(i) && Level->Element(i)->bSelected )
			Level->Element(i)->Location += FVector(32,32,32);

	// Note change.
	EdCallback( EDC_MapChange, 0 );
	NoteSelectionChange( Level );

	unguard;
}

//
// Delete all selected actors.
//
void UEditorEngine::edactDeleteSelected( ULevel* Level )
{
	guard(UEditorEngine::edactDeleteSelected);
	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if
		(	(Actor)
		&&	(Actor->bSelected)
		&&	(Level->Num()<1 || Actor!=Level->Actors(0))
		&&	(Level->Num()<2 || Actor!=Level->Actors(1))
		&&  (Actor->GetFlags() & RF_Transactional) )
		{
			Level->DestroyActor( Actor );
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

//
// Duplicate all selected actors and select just the duplicated set.
//
void UEditorEngine::edactDuplicateSelected( ULevel* Level )
{
	guard(UEditorEngine::edactDuplicateSelected);
	FVector Delta(32.0, 32.0, 0.0);

	// Untag all actors.
	for( int i=0; i<Level->Num(); i++ )
		if( Level->Actors(i) )
			Level->Actors(i)->bTempEditor = 0;

	// Duplicate and deselect all actors.
	for( i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if
		(	Actor
		&&	Actor->bSelected
		&&  !Actor->bTempEditor
		&&  Actor!=Level->Brush() 
		&&  (Actor->GetFlags() & RF_Transactional) )
		{
			FVector NewLocation = Actor->Location + Delta;
			AActor* NewActor = Level->SpawnActor
			(
				Actor->GetClass(),
				NAME_None, 
				NULL,
				NULL,
				NewLocation,
				Actor->Rotation,
				Actor,
				0,
				1
			);
			if( NewActor )
			{
				NewActor->Modify();
				if( Actor->IsBrush() )
					csgCopyBrush( (ABrush*)NewActor, ((ABrush*)Actor), ((ABrush*)Actor)->PolyFlags, 0, Actor->IsMovingBrush() );
				NewActor->PostEditMove();
				NewActor->bTempEditor = 1;
				NewActor->Location = Actor->Location + FVector(Constraints.GridSize.X,Constraints.GridSize.Y,0);
			}
			Actor->Modify();
			Actor->bSelected = 0;
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

#if 1 //WOT
//
// Replace all selected brushes with the default brush
//
void UEditorEngine::edactReplaceSelectedBrush( ULevel* Level )
{
	guard(UEditorEngine::edactReplaceSelectedBrush);

	// Untag all actors.
	for( int i=0; i<Level->Num(); i++ )
		if( Level->Actors(i) )
			Level->Actors(i)->bTempEditor = 0;

	// Replace all selected brushes
	ABrush* DefaultBrush = Level->Brush();
	for( i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if
		(	Actor
		&&	Actor->bSelected
		&&  !Actor->bTempEditor
		&&  Actor->IsBrush()
		&&  Actor!=DefaultBrush
		&&  (Actor->GetFlags() & RF_Transactional) )
		{
			ABrush* Brush = (ABrush*)Actor;
			ABrush* NewBrush = csgAddOperation( DefaultBrush, Level, Brush->PolyFlags, (ECsgOper)Brush->CsgOper );
			if( NewBrush )
			{
				NewBrush->Modify();
				NewBrush->Group = Brush->Group;
				NewBrush->CopyPosRotScaleFrom( Brush );
				NewBrush->PostEditMove();
				NewBrush->bTempEditor = 1;
				NewBrush->bSelected = 1;
				Level->DestroyActor( Actor );
			}
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

//
// Replace all selected non-brush actors with the specified class
//
void UEditorEngine::edactReplaceSelectedWithClass( ULevel* Level,UClass* Class )
{
	guard(UEditorEngine::edactReplaceSelectedWithClass);

	// Untag all actors.
	for( int i=0; i<Level->Num(); i++ )
		if( Level->Actors(i) )
			Level->Actors(i)->bTempEditor = 0;

	// Replace all selected brushes
	ABrush* DefaultBrush = Level->Brush();
	for( i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if
		(	Actor
		&&	Actor->bSelected
		&&  !Actor->bTempEditor
		&&  !Actor->IsBrush()
		&&  (Actor->GetFlags() & RF_Transactional) )
		{
			AActor* NewActor = Level->SpawnActor
			(
				Class,
				NAME_None, 
				NULL,
				NULL,
				Actor->Location,
				Actor->Rotation,
				NULL,
				0,
				1
			);
			if( NewActor )
			{
				NewActor->Modify();
				NewActor->Group = Actor->Group;
				NewActor->bTempEditor = 1;
				NewActor->bSelected = 1;
				Level->DestroyActor( Actor );
			}
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

/*-----------------------------------------------------------------------------
   Actor hiding functions.
-----------------------------------------------------------------------------*/

//
// Hide selected actors (set their bHiddenEd=true)
//
void UEditorEngine::edactHideSelected( ULevel* Level )
{
	guard(UEditorEngine::edactHideSelected);

	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor && Actor!=Level->Brush() && Actor->bSelected )
		{
			Actor->Modify();
			Actor->bHiddenEd = 1;
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

//
// Hide unselected actors (set their bHiddenEd=true)
//
void UEditorEngine::edactHideUnselected( ULevel* Level )
{
	guard(UEditorEngine::edactHideUnselected);

	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor && !Actor->IsA(ACamera::StaticClass) && Actor!=Level->Brush() && !Actor->bSelected )
		{
			Actor->Modify();
			Actor->bHiddenEd = 1;
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

//
// UnHide selected actors (set their bHiddenEd=true)
//
void UEditorEngine::edactUnHideAll( ULevel* Level )
{
	guard(UEditorEngine::edactUnHideAll);
	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if
		(	Actor
		&&	!Actor->IsA(ACamera::StaticClass)
		&&	Actor!=Level->Brush()
		&&	Actor->GetClass()->GetDefaultActor()->bHiddenEd==0 )
		{
			Actor->Modify();
			Actor->bHiddenEd = 0;
		}
	}
	NoteSelectionChange( Level );
	unguard;
}
#endif

/*-----------------------------------------------------------------------------
   Actor selection functions.
-----------------------------------------------------------------------------*/

//
// Select all actors except cameras and hidden actors.
//
void UEditorEngine::edactSelectAll( ULevel* Level )
{
	guard(UEditorEngine::edactSelectAll);
#if 1 //WOT
	// Add all selected actors' group name to the GroupArray
	TArray<FName> GroupArray;
	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor && !Actor->IsA(ACamera::StaticClass) && !Actor->bHiddenEd )
		{
			if( Actor->bSelected && Actor->Group!=NAME_None )
			{
				GroupArray.AddUniqueItem( Actor->Group );
			}
		}
	}

	// if the default brush is the only brush selected, select objects inside the default brush
	if( GroupArray.Num() == 0 && Level->Brush()->bSelected ) {
		edactSelectInside( Level );
		return;

	// if GroupArray is empty, select all unselected actors (v156 default "Select All" behavior)
	} else if( GroupArray.Num() == 0 ) {
		for( INT i=0; i<Level->Num(); i++ )
		{
			AActor* Actor = Level->Actors(i);
			if( Actor && !Actor->IsA(ACamera::StaticClass) && !Actor->bSelected && !Actor->bHiddenEd )
			{
				Actor->Modify();
				Actor->bSelected=1;
			}
		}

	// otherwise, select all actors that match one of the groups,
	} else {
		// use appStrfind() to allow selection based on hierarchically organized group names
		for( i=0; i<Level->Num(); i++ )
		{
			AActor* Actor = Level->Actors(i);
			if( Actor && !Actor->IsA(ACamera::StaticClass) && !Actor->bSelected && !Actor->bHiddenEd )
			{
				for( INT j=0; j<GroupArray.Num(); j++ ) {
					if( appStrfind( *Actor->Group, *GroupArray(j) ) != NULL ) {
						Actor->Modify();
						Actor->bSelected=1;
						break;
					}
				}
			}
		}
	}
#else
	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor && !Actor->IsA(ACamera::StaticClass) && !Actor->bSelected && !Actor->bHiddenEd )
		{
			Actor->Modify();
			Actor->bSelected=1;
		}
	}
#endif
	NoteSelectionChange( Level );
	unguard;
}

#if 1 //WOT
//
// Select all actors inside the volume of the Default Brush
//
void UEditorEngine::edactSelectInside( ULevel* Level )
{
	guard(UEditorEngine::edactSelectInside);

	// Untag all actors.
	for( INT i=0; i<Level->Num(); i++ )
		if( Level->Actors(i) )
			Level->Actors(i)->bTempEditor = 0;

	// tag all candidate actors
	for( i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor && !Actor->IsA(ACamera::StaticClass) && Actor!=Level->Brush() && !Actor->bHiddenEd )
		{
			Actor->bTempEditor = 1;
		}
	}

	// deselect all actors that are outside the default brush
	UModel* DefaultBrush = Level->Brush()->Brush;
	FCoords DefaultBrushC(Level->Brush()->ToWorld());
	for( i=0; i<DefaultBrush->Polys->Num(); i++ )
	{
		// get the plane for each polygon in the default brush
		FPoly* Poly = &DefaultBrush->Polys->Element( i );
		FPlane Plane( Poly->Base.TransformPointBy(DefaultBrushC), Poly->Normal.TransformVectorBy(DefaultBrushC) );
		for( INT j=0; j<Level->Num(); j++ )
		{
			// deselect all actors that are in front of the plane (outside the brush)
			AActor* Actor = Level->Actors(j);
			if( Actor && Actor->bTempEditor ) {
				// treat non-brush actors as point objects
				if( ! Actor->IsA(ABrush::StaticClass) ) {
					FLOAT Dist = Plane.PlaneDot( Actor->Location );
					if( Dist >= 0.0 ) {
						// actor is in front of the plane (outside the default brush)
						Actor->bTempEditor = 0;
					}

				} else {
					// examine all the points
					UPolys* Polys = Actor->Brush->Polys;
					for( INT k=0; k<Polys->Num(); k++ ) 
					{
						FCoords BrushC(Actor->ToWorld());
						for( INT m=0; m<Polys->Element(k).NumVertices; m++ ) 
						{
							FLOAT Dist = Plane.PlaneDot( Polys->Element(k).Vertex[m].TransformPointBy(BrushC) );
							if( Dist >= 0.0 )
							{
								// actor is in front of the plane (outside the default brush)
								Actor->bTempEditor = 0;
							}
						}
					}
				}
			}
		}
	}

	// update the selection state with the result from above
	for( i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor && Actor->bSelected != Actor->bTempEditor )
		{
			Actor->Modify();
			Actor->bSelected = Actor->bTempEditor;
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

//
// Invert the selection of all actors
//
void UEditorEngine::edactSelectInvert( ULevel* Level )
{
	guard(UEditorEngine::edactSelectInvert);
	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor && !Actor->IsA(ACamera::StaticClass) && Actor!=Level->Brush() && !Actor->bHiddenEd )
		{
			Actor->Modify();
			Actor->bSelected ^= 1;
		}
	}
	NoteSelectionChange( Level );
	unguard;
}
#endif

//
// Select all actors in a particular class.
//
void UEditorEngine::edactSelectOfClass( ULevel* Level, UClass* Class )
{
	guard(UEditorEngine::edactSelectOfClass);
	for( INT i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
#if 1 //WOT
		if( Actor && Actor->GetClass()==Class && !Actor->bSelected && !Actor->bHiddenEd )
#else
		if( Actor && Actor->GetClass()==Class && !Actor->bSelected )
#endif
		{
			Actor->Modify();
			Actor->bSelected=1;
		}
	}
	NoteSelectionChange( Level );
	unguard;
}

/*-----------------------------------------------------------------------------
   The End.
-----------------------------------------------------------------------------*/
