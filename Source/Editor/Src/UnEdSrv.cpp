/*=============================================================================
	UnEdSrv.cpp: UEditorEngine implementation, the Unreal editing server
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	What's happening: When the Visual Basic level editor is being used,
	this code exchanges messages with Visual Basic.  This lets Visual Basic
	affect the world, and it gives us a way of sending world information back
	to Visual Basic.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"
#include "UnRender.h"
#include "..\..\Engine\Src\UnPath.h"//!!

#pragma DISABLE_OPTIMIZATION /* Not performance-critical */

/*-----------------------------------------------------------------------------
	UnrealEd safe command line.
-----------------------------------------------------------------------------*/

//
// Execute a command that is safe for rebuilds.
//
UBOOL UEditorEngine::SafeExec( const char* InStr, FOutputDevice* Out )
{
	guard(UEditorEngine::SafeExec);
	char TempFname[256], TempStr[256], TempName[NAME_SIZE];
	const char* Str=InStr;
	if( ParseCommand(&Str,"Texture") )
	{
		if( ParseCommand(&Str,"Import") )
		{
			// Texture importing.
			FName PkgName = ParentContext ? ParentContext->GetFName() : NAME_None;
			Parse( Str, "Package=", PkgName );
			if( PkgName!=NAME_None && Parse( Str, "File=", TempFname, ARRAY_COUNT(TempFname) ) )
			{
				UPackage* Pkg = GObj.CreatePackage(NULL,*PkgName);
				if( !Parse( Str, "Name=",  TempName,  NAME_SIZE ) )
				{
					// Deduce package name from filename.
					char* End = TempFname + appStrlen(TempFname);
					while( End>TempFname && End[-1]!='\\' && End[-1]!='/' )
						End--;
					appStrncpy( TempName, End, NAME_SIZE );
					if( appStrchr(TempName,'.') )
						*appStrchr(TempName,'.') = 0;
				}
				GSystem->BeginSlowTask( "Importing texture", 1, 0 );

				UBOOL DoMips=1;
				ParseUBOOL( Str, "Mips=", DoMips );
				extern char* GFile;
				GFile = TempFname;
				FName GroupName = NAME_None;
				if( Parse( Str, "GROUP=", GroupName ) && GroupName!=NAME_None )
					Pkg = GObj.CreatePackage(Pkg,*GroupName);
				UTexture* Texture = ImportObjectFromFile<UTexture>( Pkg, TempName, TempFname );
				if( Texture )
				{
					Texture->SetFlags( RF_Public|RF_Standalone );
					DWORD TexFlags=0;
					Parse( Str, "TexFlags=", TexFlags );
					Parse( Str, "FLAGS=",    Texture->PolyFlags );
					ParseObject<UTexture>( Str, "BUMP=", Texture->BumpMap, ANY_PACKAGE );
					ParseObject<UTexture>( Str, "DETAIL=", Texture->DetailTexture, ANY_PACKAGE );
					ParseObject<UTexture>( Str, "MTEX=", Texture->MacroTexture, ANY_PACKAGE );
					ParseObject<UTexture>( Str, "NEXT=", Texture->AnimNext, ANY_PACKAGE );
					Texture->CreateMips( DoMips, 1 );
					Texture->CreateColorRange();
					debugf( NAME_Log, "Imported %s", Texture->GetFullName() );
				}
				else Out->Logf( NAME_ExecWarning, "Import texture %s from %s failed", TempName, TempFname );
				GSystem->EndSlowTask();
				GCache.Flush( 0, ~0, 1 );
			}
			else Out->Logf( NAME_ExecWarning, "Missing file or name" );
			return 1;
		}
	}
	else if( ParseCommand(&Str,"FONT") )
	{
		if( ParseCommand(&Str,"IMPORT") )
		{
			// Font importing.
			FName PkgName = ParentContext ? ParentContext->GetFName() : NAME_None;
			Parse( Str, "Package=", PkgName );
			if
			(	PkgName!=NAME_None
			&&	Parse( Str, "File=", TempFname, ARRAY_COUNT(TempFname) ) 
			&&	Parse( Str, "Name=",  TempName,  NAME_SIZE ) )
			{
				UFont* Font = ImportObjectFromFile<UFont>( GObj.CreatePackage(NULL,*PkgName), TempName, TempFname );
				if( Font )
				{
					Font->SetFlags( RF_Public|RF_Standalone );
					Font->PolyFlags = PF_Masked;
					GCache.Flush( 0, ~0, 1 );
				}
				else Out->Logf( NAME_ExecWarning, "Failed importing font from file %s", TempFname );
			}
			else Out->Log( NAME_ExecWarning, "Missing file or name" );
			return 1;
		}
	}
	else if( ParseCommand(&Str,"OBJ") )
	{
		if( ParseCommand( &Str, "LOAD" ) )
		{
			// Object file loading.
			if( Parse( Str, "FILE=", TempFname, 80 ) )
			{
				if( !ParentContext )
					Level->RememberActors();
				char PackageName[256];
				UObject* Pkg=NULL;
				if( Parse( Str, "Package=", PackageName, ARRAY_COUNT(PackageName) ) )
					Pkg = GObj.CreatePackage( NULL, PackageName );
				Pkg = GObj.LoadPackage( Pkg, TempFname, LOAD_KeepImports );
				if( !ParseParam(appCmdLine(),"NOREPACKAGE") )
				{
					for( TObjectIterator<USound> ItS; ItS; ++ItS )//oldver
						ItS->FixOld();
					for( TObjectIterator<UTexture> ItT; ItT; ++ItT )//oldver
						ItT->FixOld();
				}
				GCache.Flush();
				if( !ParentContext )
				{
					Level->ReconcileActors();
					RedrawLevel(Level);
				}
			}
			else Out->Log( NAME_ExecWarning, "Missing filename" );
			return 1;
		}
		else if( ParseCommand(&Str,"IMPORT") )
		{
			// Object importing.
			UClass* Type;
			FName Name;
			FName Package = ParentContext ? ParentContext->GetName() : Level->GetParent()->GetFName();
			DWORD Flags=0;
			if( ParseCommand(&Str,"STANDALONE") )
				Flags |= RF_Public | RF_Standalone;
			Parse( Str, "PACKAGE=", Package );
			if
			(	ParseObject<UClass>( Str, "TYPE=", Type, ANY_PACKAGE )
			&&	Parse( Str, "FILE=", TempFname, 80 )
			&&	Parse( Str, "NAME=", Name ) )
			{
				UObject* Obj = GObj.ImportObjectFromFile( Type, GObj.CreatePackage(NULL,*Package), Name, TempFname );
				if( Obj )
					Obj->SetFlags( Flags );
				else
					Out->Logf( NAME_ExecWarning, "Import failed: %s", TempFname );
			}
			else Out->Log( NAME_ExecWarning, "Missing file, name, or type" );
			return 1;
		}
	}
	else if( ParseCommand( &Str, "MESHMAP") )
	{
		if( ParseCommand( &Str, "SCALE" ) )
		{
			// Mesh scaling.
			UMesh* Mesh;
			if( ParseObject<UMesh>( Str, "MESHMAP=", Mesh, ANY_PACKAGE ) )
			{
				GetFVECTOR( Str, Mesh->Scale );
			}
			else Out->Log( NAME_ExecWarning, "Missing meshmap" );
			return 1;
		}
		else if( ParseCommand( &Str, "SETTEXTURE" ) )
		{
			// Mesh texture mapping.
			UMesh *Mesh;
			UTexture *Texture;
			INT Num;
			if
			(	ParseObject<UMesh>( Str, "MESHMAP=", Mesh, ANY_PACKAGE )
			&&	ParseObject<UTexture>( Str, "TEXTURE=", Texture, ANY_PACKAGE )
			&&	Parse( Str, "NUM=", Num ) )
			{
				while( Mesh->Textures.Num() <= Num )
					Mesh->Textures.AddItem( NULL );
				Mesh->Textures( Num ) = Texture;
			}
			else Out->Logf( NAME_ExecWarning, "Missing meshmap, texture, or num (%s)", Str );
			return 1;
		}
	}
	else if( ParseCommand(&Str,"MESH") )
	{
		if( ParseCommand(&Str,"IMPORT") )
		{
			// Mesh importing.
			char TempStr1[256];
			if
			(	Parse( Str, "MESH=", TempName, ARRAY_COUNT(TempName) )
			&&	Parse( Str, "ANIVFILE=", TempStr, ARRAY_COUNT(TempStr) )
			&&	Parse( Str, "DATAFILE=", TempStr1, ARRAY_COUNT(TempStr1) ) )
			{
				UBOOL Unmirror=0, ZeroTex=0;
				ParseUBOOL( Str, "UNMIRROR=", Unmirror );
				ParseUBOOL( Str, "ZEROTEX=", ZeroTex );
				meshImport( TempName, ParentContext, TempStr, TempStr1, Unmirror, ZeroTex );
			}
			else Out->Log(NAME_ExecWarning,"Bad MESH IMPORT");
			return 1;
		}
		else if( ParseCommand(&Str,"ORIGIN") )
		{
			// Mesh origin.
			UMesh *Mesh;
			if( ParseObject<UMesh>(Str,"MESH=",Mesh,ANY_PACKAGE) )
			{
				GetFVECTOR ( Str, Mesh->Origin );
				GetFROTATOR( Str, Mesh->RotOrigin, 256 );
			}
			else Out->Log( NAME_ExecWarning, "Bad MESH ORIGIN" );
			return 1;
		}
		else if( ParseCommand(&Str,"SEQUENCE") )
		{
			// Mesh animation sequences.
			UMesh *Mesh;
			FMeshAnimSeq Seq;
			if
			(	ParseObject<UMesh>( Str, "MESH=", Mesh, ANY_PACKAGE )
			&&	Parse( Str, "SEQ=", Seq.Name )
			&&	Parse( Str, "STARTFRAME=", Seq.StartFrame )
			&&	Parse( Str, "NUMFRAMES=", Seq.NumFrames ) )
			{
				Parse( Str, "RATE=", Seq.Rate );
				Parse( Str, "GROUP=", Seq.Group );
				new( Mesh->AnimSeqs )FMeshAnimSeq( Seq );
				Mesh->AnimSeqs.Shrink();
			}
			else Out->Log(NAME_ExecWarning,"Bad MESH SEQUENCE");
			return 1;
		}
		else if( ParseCommand(&Str,"NOTIFY") )
		{
			// Mesh notifications.
			UMesh* Mesh;
			FName SeqName;
			FMeshAnimNotify Notify;
			if
			(	ParseObject<UMesh>( Str, "MESH=", Mesh, ANY_PACKAGE )
			&&	Parse( Str, "SEQ=", SeqName )
			&&	Parse( Str, "TIME=", Notify.Time )
			&&	Parse( Str, "FUNCTION=", Notify.Function ) )
			{
				FMeshAnimSeq* Seq = Mesh->GetAnimSeq( SeqName );
				if( Seq ) new( Seq->Notifys )FMeshAnimNotify( Notify );
				else Out->Log( NAME_ExecWarning, "Unknown sequence in MESH NOTIFY" );
			}
			else Out->Log( NAME_ExecWarning, "Bad MESH NOTIFY" );
			return 1;
		}
	}
	else if( ParseCommand( &Str, "AUDIO") )
	{
		if( ParseCommand(&Str,"IMPORT") )
		{
			// Audio importing.
			char TempName[NAME_SIZE];
			FName PkgName = ParentContext ? ParentContext->GetName() : Level->GetParent()->GetFName();
			Parse( Str, "PACKAGE=", PkgName );
			if( Parse( Str, "FILE=", TempFname, 80 ) )
			{
				if( !Parse( Str, "NAME=", TempName, ARRAY_COUNT(TempName) ) )
				{
					// Deduce package name from filename.
					char* End = TempFname + appStrlen(TempFname);
					while( End>TempFname && End[-1]!='\\' && End[-1]!='/' )
						End--;
					appStrncpy( TempName, End, ARRAY_COUNT(TempName) );
					for( int i=0; TempName[i]; i++ )
						if( TempName[i]=='.' )
							TempName[i]=0;
				}
				UPackage* Pkg = GObj.CreatePackage(NULL,*PkgName);
				FName Group = NAME_None;
				if( Parse(Str,"GROUP=",Group) && Group!=NAME_None )
					Pkg = GObj.CreatePackage( Pkg, *Group );
				UObject* Temp = ImportObjectFromFile<USound>( Pkg, TempName, TempFname );
				if( Temp )
					Temp->SetFlags( RF_Standalone | RF_Public );
			}
			else Out->Log( NAME_ExecWarning, "Missing file, name, or type" );
			return 1;
		}
	}
	return 0;
	unguardf(( "(%s)", InStr ));
}

/*-----------------------------------------------------------------------------
	UnrealEd command line.
-----------------------------------------------------------------------------*/

//
// Process an incoming network message meant for the editor server
//
UBOOL UEditorEngine::Exec( const char* Stream, FOutputDevice* Out )
{
	char ErrorTemp[256]="Setup: ";
	guard(UEditorEngine::Exec);
	UBOOL Processed=0;

	_WORD	 		Word1,Word2,Word4;
	INT				Index1;
	char	 		TempStr[256],TempFname[256],TempName[256],Temp[256];

	if( appStrlen(Stream)<200 )
	{
		appStrcat( ErrorTemp, Stream );
		debugf( NAME_Cmd, Stream );
	}

	UModel* Brush = Level ? Level->Brush()->Brush : NULL;
	//if( Brush ) check(stricmp(Brush->GetName(),"BRUSH")==0);

	appStrncpy( Temp, Stream, 256 );
	const char* Str = &Temp[0];

	appStrncpy( ErrorTemp, Str, 79 );
	ErrorTemp[79]=0;

	//------------------------------------------------------------------------------------
	// BRUSH
	//
	if( SafeExec( Stream, Out ) )
	{
		return 1;
	}
	else if( ParseCommand(&Str,"BRUSH") )
	{
		if( ParseCommand(&Str,"SET") )
		{
			Trans->Begin( Level, "Brush Set" );
			Brush->Modify();
			Brush->Polys->ModifyAllItems();
			Constraints.Snap( NULL, Level->Brush()->Location, FVector(0,0,0), Level->Brush()->Rotation );
			FModelCoords TempCoords;
			Level->Brush()->BuildCoords( &TempCoords, NULL );
			Level->Brush()->Location -= Level->Brush()->PrePivot.TransformVectorBy( TempCoords.PointXform );
			Level->Brush()->PrePivot = FVector(0,0,0);
			Brush->Polys->Empty();
			TObjectIterator<UPolysFactory> It;
			check(*It);
			It->Create( UPolys::StaticClass, Brush->Polys->GetParent(), Brush->Polys->GetName(), Brush->Polys, "t3d", Stream, Stream+appStrlen(Stream) );
			bspValidateBrush( Brush, 1, 1 );
			Brush->BuildBound();
			Trans->End();
			RedrawLevel( Level );
			NoteSelectionChange( Level );
			Processed = 1;
		}
		else if( ParseCommand(&Str,"MORE") )
		{
			Trans->Continue();
			Brush->Modify();
			Brush->Polys->ModifyAllItems();
			TObjectIterator<UPolysFactory> It;
			check(*It);
			It->Create( UPolys::StaticClass, Brush->Polys->GetParent(), Brush->Polys->GetName(), Brush->Polys, "t3d", Stream, Stream+appStrlen(Stream) );
			bspValidateBrush( Level->Brush()->Brush, 1, 1 );
			Brush->BuildBound();
			Trans->End();	
			RedrawLevel( Level );
			Processed = 1;
		}
		else if( ParseCommand(&Str,"RESET") )
		{
			Trans->Begin( Level, "Brush Reset" );
			Level->Brush()->Modify();
			Level->Brush()->InitPosRotScale();
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if( ParseCommand(&Str,"MIRROR") )
		{
			Trans->Begin(Level,"Brush Mirror");
			Level->Brush()->Modify();
			if (ParseCommand(&Str,"X")) Level->Brush()->MainScale.Scale.X *= -1.0;
			if (ParseCommand(&Str,"Y")) Level->Brush()->MainScale.Scale.Y *= -1.0;
			if (ParseCommand(&Str,"Z")) Level->Brush()->MainScale.Scale.Z *= -1.0;
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if( ParseCommand(&Str,"SCALE") )
		{
			Trans->Begin(Level,"Brush Scale");
			Level->Brush()->Modify();
			if( ParseCommand(&Str,"RESET") )
			{
				Level->Brush()->MainScale = GMath.UnitScale;
				Level->Brush()->PostScale = GMath.UnitScale;
			}
			else
			{
				GetFVECTOR( Str, Level->Brush()->MainScale.Scale );
				Parse( Str, "SHEER=", Level->Brush()->MainScale.SheerRate );
				if( Parse( Str, "SHEERAXIS=", TempStr, 255 ) )
				{
					if      (appStricmp(TempStr,"XY")==0)	Level->Brush()->MainScale.SheerAxis = SHEER_XY;
					else if (appStricmp(TempStr,"XZ")==0)	Level->Brush()->MainScale.SheerAxis = SHEER_XZ;
					else if (appStricmp(TempStr,"YX")==0)	Level->Brush()->MainScale.SheerAxis = SHEER_YX;
					else if (appStricmp(TempStr,"YZ")==0)	Level->Brush()->MainScale.SheerAxis = SHEER_YZ;
					else if (appStricmp(TempStr,"ZX")==0)	Level->Brush()->MainScale.SheerAxis = SHEER_ZX;
					else if (appStricmp(TempStr,"ZY")==0)	Level->Brush()->MainScale.SheerAxis = SHEER_ZY;
					else									Level->Brush()->MainScale.SheerAxis = SHEER_None;
				}
			}
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if( ParseCommand(&Str,"APPLYTRANSFORM") )
		{
			Trans->Begin(Level,"Brush ApplyTransform");
			Brush->Transform(Level->Brush());
			Brush->BuildBound();
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if( ParseCommand(&Str,"ROTATETO") )
		{
			Trans->Begin(Level,"Brush RotateTo");
			Level->Brush()->Modify();
			GetFROTATOR( Str, Level->Brush()->Rotation, 256 );
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if( ParseCommand(&Str,"ROTATEREL") )
		{
			Trans->Begin(Level,"Brush RotateRel");
			Level->Brush()->Modify();
			FRotator TempRotation(0,0,0);
			GetFROTATOR( Str, TempRotation, 256 );
			Level->Brush()->Rotation += TempRotation;
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if( ParseCommand(&Str,"MOVETO") )
		{
			Trans->Begin(Level,"Brush MoveTo");
			Level->Brush()->Modify();
			GetFVECTOR( Str, Level->Brush()->Location );
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if( ParseCommand(&Str,"MOVEREL") )
		{
			Trans->Begin(Level,"Brush MoveRel");
			Level->Brush()->Modify();
			FVector TempVector( 0, 0, 0 );
			GetFVECTOR( Str, TempVector );
			Level->Brush()->Location.AddBounded( TempVector );
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if (ParseCommand(&Str,"ADD"))
		{
			Trans->Begin(Level,"Brush Add");
			FinishAllSnaps(Level);
			INT DWord1=0;
			Parse( Str, "FLAGS=", DWord1 );
			Level->Modify();
			ABrush* NewBrush = csgAddOperation( Level->Brush(), Level, DWord1, CSG_Add );
			if( NewBrush )
				bspBrushCSG( NewBrush, Level->Model, DWord1, CSG_Add, 1 );
			Trans->End();
			RedrawLevel(Level);
			EdCallback(EDC_MapChange,0);
			Processed = 1;
		}
		else if (ParseCommand(&Str,"ADDMOVER")) // BRUSH ADDMOVER
		{
			Trans->Begin( Level, "Brush AddMover" );
			Level->Modify();
			FinishAllSnaps( Level );

			UClass* MoverClass = NULL;
			ParseObject<UClass>( Str, "CLASS=", MoverClass, ANY_PACKAGE );
			if( !MoverClass || !MoverClass->IsChildOf(AMover::StaticClass) )
				MoverClass = AMover::StaticClass;

			Level->Modify();
			AMover* Actor = (AMover*)Level->SpawnActor(MoverClass,NAME_None,NULL,NULL,Level->Brush()->Location);
			if( Actor )
			{
				csgCopyBrush( Actor, Level->Brush(), 0, 0, 1 );
				Actor->PostEditChange();
			}
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
		else if (ParseCommand(&Str,"SUBTRACT")) // BRUSH SUBTRACT
			{
			Trans->Begin(Level,"Brush Subtract");
			FinishAllSnaps(Level);
			Level->Modify();
			ABrush* NewBrush = csgAddOperation(Level->Brush(),Level,0,CSG_Subtract); // Layer
			if( NewBrush )
				bspBrushCSG( NewBrush, Level->Model, 0, CSG_Subtract, 1 );
			Trans->End();
			RedrawLevel(Level);
			EdCallback(EDC_MapChange,0);
			Processed = 1;
			}
		else if (ParseCommand(&Str,"FROM")) // BRUSH FROM ACTOR/INTERSECTION/DEINTERSECTION
			{
			if (ParseCommand(&Str,"INTERSECTION"))
				{
				Out->Log		("Brush from intersection");
				//
				Trans->Begin	(Level,"Brush From Intersection");
				Brush->Modify();
				Brush->Polys->ModifyAllItems();
				//
				FinishAllSnaps (Level);
				//
				bspBrushCSG( Level->Brush(), Level->Model, 0, CSG_Intersect, 0 );
				//
				Trans->End			();
				RedrawLevel(Level);
				Processed = 1;
				}
			else if (ParseCommand(&Str,"DEINTERSECTION"))
				{
				Out->Log		("Brush from deintersection");
				//
				Trans->Begin(Level,"Brush From Deintersection");
				Brush->Modify();
				Brush->Polys->ModifyAllItems();
				//
				FinishAllSnaps (Level);
				//
				bspBrushCSG( Level->Brush(), Level->Model, 0, CSG_Deintersect, 0 );
				Trans->End				();
				RedrawLevel(Level);
				Processed = 1;
				};
			}
		else if (ParseCommand (&Str,"NEW"))
			{
			Trans->Begin( Level, "Brush New" );
			Brush->Modify();
			Brush->Polys->ModifyAllItems();
			Brush->Polys->Empty();
			Trans->End();
			RedrawLevel( Level );
			Processed = 1;
			}
		else if (ParseCommand (&Str,"LOAD")) // BRUSH LOAD
		{
			if( Parse( Str, "FILE=", TempFname, 79 ) )
			{
				Trans->Reset( "loading brush" );

				FVector   TempVector   = Level->Brush()->Location;
				FRotator TempRotation = Level->Brush()->Rotation;

				GObj.LoadPackage( Level->GetParent(), TempFname, LOAD_KeepImports );

				Level->Brush()->Location = TempVector;
				Level->Brush()->Rotation = TempRotation;

				bspValidateBrush( Level->Brush()->Brush, 0, 1 );
				Cleanse( *Out, 1, "loading brush" );
				Processed = 1;
			}
		}
		else if( ParseCommand( &Str, "SAVE" ) )
		{
			if( Parse(Str,"FILE=",TempFname,79) )
			{
				Out->Logf( "Saving %s", TempFname );
				GObj.SavePackage( Level->GetParent(), Brush, 0, TempFname );
			}
			else Out->Log( NAME_ExecWarning, "Missing filename" );
			Processed = 1;
		}
		else if( ParseCommand( &Str, "IMPORT") )
		{
			if( Parse(Str,"FILE=",TempFname,79) )
			{
				GSystem->BeginSlowTask( "Importing brush", 1, 0 );
				Trans->Begin( Level, "Brush Import" );
				Brush->Polys->Modify();
				Brush->Polys->ModifyAllItems();

				DWORD Flags=0;
				UBOOL Merge=0;
				ParseUBOOL( Str, "MERGE=", Merge );
				Parse( Str, "FLAGS=", Flags );

				Brush->Linked = 0;
				ImportObjectFromFile<UPolys>( Brush->Polys->GetParent(), Brush->Polys->GetName(), TempFname );
				if( Flags )
					for( Word2=0; Word2<TempModel->Polys->Num(); Word2++ )
						Brush->Polys->Element(Word2).PolyFlags |= Flags;
				for( INT i=0; i<Brush->Polys->Num(); i++ )
					Brush->Polys->Element(i).iLink = i;
				if( Merge )
				{
					bspMergeCoplanars( Brush, 0, 1 );
					bspValidateBrush( Brush, 0, 1 );
				}
				Trans->End();

				GSystem->EndSlowTask();
			}
			else Out->Log( NAME_ExecWarning, "Missing filename" );
			Processed=1;
		}
		else if (ParseCommand (&Str,"EXPORT"))
		{
			if( Parse(Str,"FILE=",TempFname,79) )
			{
				GSystem->BeginSlowTask	("Exporting brush",1,0);
				Brush->Polys->ExportToFile(TempFname); // Only exports polys
				GSystem->EndSlowTask();
			}
			else Out->Log(NAME_ExecWarning,"Missing filename");
			Processed=1;
		}
	}
	//----------------------------------------------------------------------------------
	// EDIT
	//
	else if( ParseCommand(&Str,"EDIT") )
	{
		if( ParseCommand(&Str,"CUT") )
		{
			Trans->Begin( Level, "Cut" );
			edactCopySelected( Level );
			edactDeleteSelected( Level );
			Trans->End();
			RedrawLevel( Level );
		}
		else if( ParseCommand(&Str,"COPY") )
		{
			edactCopySelected( Level );
		}
		else if( ParseCommand(&Str,"PASTE") )
		{
			Trans->Begin( Level, "Cut" );
			SelectNone( Level, 1 );
			edactPasteSelected( Level );
			Trans->End();
			RedrawLevel( Level );
		}
	}
	//----------------------------------------------------------------------------------
	// PIVOT
	//
	else if( ParseCommand(&Str,"PIVOT") )
	{
		if( ParseCommand(&Str,"HERE") )
		{
			NoteActorMovement( Level );
			SetPivot( ClickLocation, 0, 0 );
			FinishAllSnaps( Level );
			RedrawLevel( Level );
		}
		else if( ParseCommand(&Str,"SNAPPED") )
		{
			NoteActorMovement( Level );
			SetPivot( ClickLocation, 1, 0 );
			FinishAllSnaps( Level );
			RedrawLevel( Level );
		}
	}
	//----------------------------------------------------------------------------------
	// PATHS
	//
	else if( ParseCommand(&Str,"PATHS") )
	{
		if (ParseCommand(&Str,"BUILD"))
		{
			int opt = 1; //assume medium
			if (ParseCommand(&Str,"LOWOPT"))
				opt = 0;
			else if (ParseCommand(&Str,"HIGHOPT"))
				opt = 2;

			FPathBuilder builder;

			Trans->Begin			(Level,"Remove Paths");
			Level->Modify();
			int numpaths = builder.removePaths		(Level);
			Trans->End				();

			Trans->Begin			(Level,"Build Paths");
			Level->Modify();
			numpaths = builder.buildPaths		(Level, opt);
			Trans->End				();
			RedrawLevel(Level);
			Out->Logf("Built Paths: %d", numpaths);
			Processed=1;
		}
		else if (ParseCommand(&Str,"SHOW"))
		{
			FPathBuilder builder;
			Trans->Begin			(Level,"Show Paths");
			Level->Modify();
			int numpaths = builder.showPaths(Level);
			Trans->End				();
			RedrawLevel(Level);
			Out->Logf(" %d Paths are visible!", numpaths);
			Processed=1;
		}
		else if (ParseCommand(&Str,"HIDE"))
		{
			FPathBuilder builder;
			Trans->Begin			(Level,"Hide Paths");
			Level->Modify();
			int numpaths = builder.hidePaths(Level);
			Trans->End				();
			RedrawLevel(Level);
			Out->Logf(" %d Paths are hidden!", numpaths);
			Processed=1;
		}
		else if (ParseCommand(&Str,"REMOVE"))
		{
			FPathBuilder builder;
			Trans->Begin			(Level,"Remove Paths");
			Level->Modify();
			int numpaths = builder.removePaths		(Level);
			Trans->End				();
			RedrawLevel(Level);
			Out->Logf("Removed %d Paths", numpaths);
			Processed=1;
		}
		else if (ParseCommand(&Str,"DEFINE"))
		{
			FPathBuilder builder;
			Trans->Begin			(Level,"UnDefine old Paths");
			Level->Modify();
			builder.undefinePaths	(Level);
			Trans->End				();

			Trans->Begin			(Level,"Define Paths");
			Level->Modify();
			builder.definePaths		(Level);
			Trans->End				();

			RedrawLevel(Level);
			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// Bsp
	//
	else if( ParseCommand( &Str, "BSP" ) )
	{
		if( ParseCommand( &Str, "REBUILD") ) // Bsp REBUILD [LAME/GOOD/OPTIMAL] [BALANCE=0-100] [LIGHTS] [MAPS] [REJECT]
		{
			Trans->Reset("rebuilding Bsp"); // Not tracked transactionally
			Out->Log("Bsp Rebuild");
			EBspOptimization BspOpt;

			if      (ParseCommand(&Str,"LAME")) 		BspOpt=BSP_Lame;
			else if (ParseCommand(&Str,"GOOD"))		BspOpt=BSP_Good;
			else if (ParseCommand(&Str,"OPTIMAL"))	BspOpt=BSP_Optimal;
			else								BspOpt=BSP_Good;

			if( !Parse( Str, "BALANCE=", Word2 ) )
				Word2=50;

			GSystem->BeginSlowTask( "Rebuilding Bsp", 1, 0 );

			GSystem->StatusUpdatef( 0, 0, "%s", "Building polygons" );
			bspBuildFPolys( Level->Model, 1, 0 );

			GSystem->StatusUpdatef( 0, 0, "%s", "Merging planars" );
			bspMergeCoplanars( Level->Model, 0, 0 );

			GSystem->StatusUpdatef( 0, 0, "%s", "Partitioning" );
			bspBuild( Level->Model, BspOpt, Word2, 0, 0 );

			if( Parse( Str, "ZONES", TempStr, 1 ) )
			{
				GSystem->StatusUpdatef( 0, 0, "%s", "Building visibility zones" );
				TestVisibility( Level, Level->Model, 0, 0 );
			}
			if( Parse( Str, "OPTGEOM", TempStr, 1 ) )
			{
				GSystem->StatusUpdatef( 0, 0, "%s", "Optimizing geometry" );
				bspOptGeom( Level->Model );
			}

			// Empty EdPolys.
			Level->Model->Polys->Empty();

			GSystem->EndSlowTask();
			GCache.Flush();
			RedrawLevel(Level);
			EdCallback( EDC_MapChange, 0 );

			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// LIGHT
	//
	else if( ParseCommand( &Str, "LIGHT" ) )
	{
		if( ParseCommand( &Str, "APPLY" ) )
		{
			UBOOL Selected=0;
			ParseUBOOL( Str, "SELECTED=", Selected );
			shadowIlluminateBsp( Level, Selected );
			GCache.Flush();
			RedrawLevel( Level );
			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// MAP
	//
	else if (ParseCommand(&Str,"MAP"))
		{
		//
		// Commands:
		//
		if (ParseCommand(&Str,"GRID")) // MAP GRID [SHOW3D=ON/OFF] [SHOW2D=ON/OFF] [X=..] [Y=..] [Z=..]
			{
			//
			// Before changing grid, force editor to current grid position to avoid jerking:
			//
			FinishAllSnaps (Level);
			//
			UBOOL TempShow2DGrid=Show2DGrid;
			ParseUBOOL(Str,"SHOW2D=",TempShow2DGrid);
			Show2DGrid=TempShow2DGrid;
			UBOOL TempShow3DGrid=Show3DGrid;
			ParseUBOOL(Str,"SHOW3D=",TempShow3DGrid);
			Show3DGrid=TempShow3DGrid;
			GetFVECTOR( Str, Constraints.GridSize );
			//
			RedrawLevel(Level);
			Processed=1;
			}
		else if (ParseCommand(&Str,"ROTGRID")) // MAP ROTGRID [PITCH=..] [YAW=..] [ROLL=..]
			{
			FinishAllSnaps (Level);
			if( GetFROTATOR( Str, Constraints.RotGridSize, 256 ) )
				RedrawLevel(Level);
			Processed=1;
			}
		else if (ParseCommand(&Str,"SELECT")) // MAP SELECT ALL/NONE/INVERSE/NAMEstr
			{
			Trans->Begin (Level,"Select");
			//
			if      (ParseCommand(&Str,"ADDS"))		mapSelectOperation	(Level,CSG_Add);
			else if (ParseCommand(&Str,"SUBTRACTS"))	mapSelectOperation	(Level,CSG_Subtract);
			else if (ParseCommand(&Str,"SEMISOLIDS"))	mapSelectFlags		(Level,PF_Semisolid);
			else if (ParseCommand(&Str,"NONSOLIDS"))	mapSelectFlags		(Level,PF_NotSolid);
			else if (ParseCommand(&Str,"FIRST"))		mapSelectFirst		(Level);
			else if (ParseCommand(&Str,"LAST"))		mapSelectLast		(Level);
			//
			Trans->End 			();
			RedrawLevel(Level);
			Processed=1;
			}
		else if (ParseCommand(&Str,"DELETE")) // MAP DELETE
			{
			Exec("ACTOR DELETE",Out);
			Processed=1;
			}
		else if (ParseCommand(&Str,"BRUSH")) // MAP BRUSH GET/PUT
			{
			if (ParseCommand (&Str,"GET"))
				{
				Trans->Begin		(Level,"Brush Get");
				mapBrushGet			(Level);
				Trans->End			();
				RedrawLevel(Level);
				Processed=1;
				}
			else if (ParseCommand (&Str,"PUT"))
				{
				Trans->Begin		(Level,"Brush Put");
				mapBrushPut			(Level);
				Trans->End			();
				RedrawLevel(Level);
				Processed=1;
				};
			}
		else if (ParseCommand(&Str,"SENDTO")) // MAP SENDTO FRONT/BACK
			{
			if (ParseCommand(&Str,"FIRST"))
				{
				Trans->Begin		(Level,"Map SendTo Front");
				mapSendToFirst		(Level);
				Trans->End			();
				RedrawLevel(Level);
				Processed=1;
				}
			else if (ParseCommand(&Str,"LAST"))
				{
				Trans->Begin		(Level,"Map SendTo Back");
				mapSendToLast		(Level);
				Trans->End			();
				RedrawLevel(Level);
				Processed=1;
				};
			}
		else if (ParseCommand(&Str,"REBUILD")) // MAP REBUILD
			{
			Trans->Reset		("rebuilding map"); 	// Can't be transaction-tracked
			csgRebuild			(Level);				// Revalidates the Bsp
			GCache.Flush		();
			RedrawLevel(Level);
			EdCallback	(EDC_MapChange,0);
			Processed=1;
			}
		else if (ParseCommand (&Str,"NEW")) // MAP NEW
			{
			Trans->Reset( "clearing map" );
			Level->RememberActors();
			Level = new( Level->GetParent(), "MyLevel" )ULevel( this, 0 );
			Level->ReconcileActors();
			ResetSound();
			RedrawLevel(Level);
			NoteSelectionChange( Level );
			EdCallback(EDC_MapChange,0);
			//
			Cleanse( *Out, 1, "starting new map" );
			//
			Processed=1;
			}
		else if( ParseCommand( &Str, "LOAD" ) )
		{
			if( Parse( Str, "FILE=", TempFname, 79 ) )
			{
				Trans->Reset( "loading map" );
				GSystem->BeginSlowTask( "Loading map", 1, 0 );
				Level->RememberActors();
				GObj.ResetLoaders( Level->GetParent() );
				GObj.LoadPackage( Level->GetParent(), TempFname, LOAD_KeepImports );
				Level->Engine = this;
				Level->ReconcileActors();
				ResetSound();
				bspValidateBrush( Level->Brush()->Brush, 0, 1 );
				GSystem->EndSlowTask();
				RedrawLevel(Level);
				EdCallback( EDC_MapChange, 0 );
				NoteSelectionChange( Level );
				Level->SetFlags( RF_Transactional );
				Level->Model->SetFlags( RF_Transactional );
				if( Level->Model->Nodes ) Level->Model->Nodes->SetFlags( RF_Transactional );
				if( Level->Model->Surfs ) Level->Model->Surfs->SetFlags( RF_Transactional );
				if( Level->Model->Verts ) Level->Model->Verts->SetFlags( RF_Transactional );
				if( Level->Model->Vectors ) Level->Model->Vectors->SetFlags( RF_Transactional );
				if( Level->Model->Points ) Level->Model->Points->SetFlags( RF_Transactional );
				if( Level->Model->Polys ) Level->Model->Polys->SetFlags( RF_Transactional );
				for( TObjectIterator<AActor> It; It; ++It )
				{
					for( INT i=0; i<Level->Num(); i++ )
						if( *It==Level->Actors(i) )
							break;
					if( i==Level->Num() )
					{
						It->bDeleteMe=1;
					}
					else
					{
						It->bDeleteMe=0;
						if( It->IsA(ACamera::StaticClass) )
							It->ClearFlags( RF_Transactional );
						else
							It->SetFlags( RF_Transactional );
					}
				}
				GCache.Flush();
				Cleanse( *Out, 0, "loading map" );
			}
			else Out->Log( NAME_ExecWarning, "Missing filename" );
			Processed=1;
		}
		else if( ParseCommand (&Str,"SAVE") )
		{
			if( Parse(Str,"FILE=",TempFname,79) )
			{
				Level->ShrinkLevel();
				for( INT i=0; i<Level->Num(); i++ )
					if( Cast<APlayerPawn>( Level->Element(i) ) )
						Cast<APlayerPawn>( Level->Element(i) )->Song = NULL;
				GSystem->BeginSlowTask( "Saving map", 1, 0 );
				GObj.SavePackage( Level->GetParent(), Level, 0, TempFname );
				GSystem->EndSlowTask();
			}
			else Out->Log( NAME_ExecWarning, "Missing filename" );
			Processed=1;
		}
		else if( ParseCommand( &Str, "IMPORT" ) ) // MAP IMPORT
		{
			Word1=1;
			DoImportMap:
			if( Parse( Str, "FILE=", TempFname, 79 ) )
			{
				Trans->Reset( "importing map" );
				GSystem->BeginSlowTask( "Importing map", 1, 0 );
				Level->RememberActors();
				if( Word1 )
					Level = new( Level->GetParent(), "MyLevel" )ULevel( this, 0 );
				ImportObjectFromFile<ULevel>( Level->GetParent(), Level->GetFName(), TempFname, GSystem );
				Level->SetFlags( RF_Transactional );
				GCache.Flush();
				Level->ReconcileActors();
				ResetSound();
				if( Word1 )
					SelectNone( Level, 0 );
				GSystem->EndSlowTask();
				RedrawLevel(Level);
				EdCallback( EDC_MapChange, 0 );
				NoteSelectionChange( Level );
				Cleanse( *Out, 1, "importing map" );
			}
			else Out->Log(NAME_ExecWarning,"Missing filename");
			Processed=1;
		}
		else if( ParseCommand( &Str, "IMPORTADD" ) )
		{
			Word1=0;
			SelectNone( Level, 0 );
			goto DoImportMap;
		}
		else if (ParseCommand (&Str,"EXPORT"))
			{
			if (Parse(Str,"FILE=",TempFname,79))
				{
				GSystem->BeginSlowTask	("Exporting map",1,0);
				for( FObjectIterator It; It; ++It )
					It->ClearFlags( RF_TagImp | RF_TagExp );
				Level->ExportToFile(TempFname);
				GSystem->EndSlowTask();
				}
			else Out->Log(NAME_ExecWarning,"Missing filename");
			Processed=1;
			}
		else if (ParseCommand (&Str,"SETBRUSH")) // MAP SETBRUSH (set properties of all selected brushes)
			{
			Trans->Begin		(Level,"Set Brush Properties");
			//
			Word1  = 0;  // Properties mask
			INT DWord1 = 0;  // Set flags
			INT DWord2 = 0;  // Clear flags
			//
			FName GroupName=NAME_None;
			if (Parse(Str,"COLOR=",Word2))			Word1 |= MSB_BrushColor;
			if (Parse(Str,"GROUP=",GroupName))		Word1 |= MSB_Group;
			if (Parse(Str,"SETFLAGS=",DWord1))		Word1 |= MSB_PolyFlags;
			if (Parse(Str,"CLEARFLAGS=",DWord2))	Word1 |= MSB_PolyFlags;
			//
			mapSetBrush(Level,(EMapSetBrushFlags)Word1,Word2,GroupName,DWord1,DWord2);
			//
			Trans->End			();
			RedrawLevel(Level);
			//
			Processed=1;
			}
		else if (ParseCommand (&Str,"SAVEPOLYS"))
			{
			if (Parse(Str,"FILE=",TempFname,79))
				{
				UBOOL DWord2=1;
				ParseUBOOL(Str, "MERGE=",DWord2);
				//
				GSystem->BeginSlowTask	("Exporting map polys",1,0);
				GSystem->StatusUpdatef( 0, 0, "%s", "Building polygons" );
				bspBuildFPolys		(Level->Model,0,0);
				//
				if (DWord2)
					{
					GSystem->StatusUpdatef( 0, 0, "%s", "Merging planars" );
					bspMergeCoplanars	(Level->Model,0,1);
					};
				Level->Model->Polys->ExportToFile(TempFname);
				Level->Model->Polys->Empty();
				//
				GSystem->EndSlowTask 	();
				RedrawLevel(Level);
				}
			else Out->Log(NAME_ExecWarning,"Missing filename");
			Processed=1;
			};
		}
	//------------------------------------------------------------------------------------
	// SELECT: Rerouted to mode-specific command
	//
	else if( ParseCommand(&Str,"SELECT") )
	{
		if( ParseCommand(&Str,"NONE") )
		{
			Trans->Begin( Level, "Select None" );
			SelectNone( Level, 1 );
			Trans->End();
			RedrawLevel( Level );
			Processed=1;
		}
		Processed=1;
	}
	//------------------------------------------------------------------------------------
	// DELETE: Rerouted to mode-specific command
	//
	else if (ParseCommand(&Str,"DELETE"))
	{
		return Exec( "ACTOR DELETE" );
	}
	//------------------------------------------------------------------------------------
	// DUPLICATE: Rerouted to mode-specific command
	//
	else if (ParseCommand(&Str,"DUPLICATE"))
	{
		return Exec( "ACTOR DUPLICATE" );
	}
	//------------------------------------------------------------------------------------
	// ACTOR: Actor-related functions
	//
	else if (ParseCommand(&Str,"ACTOR"))
	{
		if( ParseCommand(&Str,"ADD") )
		{
			UClass* Class;
			if( ParseObject<UClass>( Str, "CLASS=", Class, ANY_PACKAGE ) )
			{
				AActor* Default   = Class->GetDefaultActor();
				FVector Collision = FVector(Default->CollisionRadius,Default->CollisionRadius,Default->CollisionHeight);
				FVector Location  = ClickLocation + ClickPlane * (FBoxPushOut(ClickPlane,Collision) + 0.1);
				AddActor( Level, Class, Location );
				RedrawLevel(Level);
				Processed = 1;
			}
		}
		else if( ParseCommand(&Str,"MIRROR") )
		{
			Trans->Begin(Level,"Mirroring Actors");
			FVector V( 1, 1, 1 );
			GetFVECTOR( Str, V );
			for( INT i=0; i<Level->Num(); i++ )
			{
				ABrush* Actor=Cast<ABrush>(Level->Actors(i));
				if( Actor && Actor->bSelected )
				{
					Actor->Modify();
					Actor->MainScale.Scale *= V;
				}
			}
			Trans->End();
			RedrawLevel(Level);
			Processed = 1;
		}
#if 1 //WOT
		else if( ParseCommand(&Str,"HIDE") )
		{
			if( ParseCommand(&Str,"SELECTED") ) // ACTOR HIDE SELECTED
			{
				Trans->Begin( Level, "Hide Selected" );
				Level->Modify();
				edactHideSelected( Level );
				Trans->End();
				RedrawLevel( Level );
				SelectNone( Level, 0 );
				NoteSelectionChange( Level );
				Processed=1;
			}
			else if( ParseCommand(&Str,"UNSELECTED") ) // ACTOR HIDE UNSELECTEED
			{
				Trans->Begin( Level, "Hide Unselected" );
				Level->Modify();
				edactHideUnselected( Level );
				Trans->End();
				RedrawLevel( Level );
				SelectNone( Level, 0 );
				NoteSelectionChange( Level );
				Processed=1;
			}
		}
		else if( ParseCommand(&Str,"UNHIDE") ) // ACTOR UNHIDE ALL
		{
				// "ACTOR UNHIDE ALL" = "Drawing Region: Off": also disables the far (Z) clipping plane
				ResetZClipping();

				Trans->Begin( Level, "UnHide All" );
				Level->Modify();
				edactUnHideAll( Level );
				Trans->End();
				RedrawLevel( Level );
				NoteSelectionChange( Level );
				Processed=1;

		}
		else if( ParseCommand(&Str,"CLIP") ) // ACTOR CLIP Z/XY/XYZ
		{
			if( ParseCommand(&Str,"Z") ) SetZClipping();
			else if( ParseCommand(&Str, "XY" ) ) debugf( "XY clipping: not implemented." ); //XXX
			else if( ParseCommand(&Str, "XYZ" ) ) debugf( "XYZ clipping not implemented." ); //XXX
			RedrawLevel( Level );
			Processed=1;
		}
		else if( ParseCommand(&Str, "REPLACE") )
		{
			UClass* Class;
			if( ParseCommand(&Str, "BRUSH") ) // ACTOR REPLACE BRUSH
			{
				Trans->Begin( Level, "Replace selected brush actors" );
				Level->Modify();
				edactReplaceSelectedBrush( Level );
				Trans->End();
				RedrawLevel( Level );
				NoteSelectionChange( Level );
				Processed=1;
			}
			else if( ParseObject<UClass>( Str, "CLASS=", Class, ANY_PACKAGE ) ) // ACTOR REPLACE CLASS=<class>
			{
				Trans->Begin( Level, "Replace selected non-brush actors" );
				Level->Modify();
				edactReplaceSelectedWithClass( Level, Class );
				Trans->End();
				RedrawLevel( Level );
				NoteSelectionChange( Level );
				Processed=1;
			}
		}
#endif
		else if( ParseCommand(&Str,"SELECT") )
		{
			if( ParseCommand(&Str,"NONE") ) // ACTOR SELECT NONE
			{
				return Exec( "SELECT NONE" );
			}
			else if( ParseCommand(&Str,"ALL") ) // ACTOR SELECT ALL
			{
				Trans->Begin( Level, "Select All" );
				Level->Modify();
#if 1 //WOT
				//WOT-specific modification to select brushes matching the currently selected groups
#else
				SelectNone( Level, 0 );
#endif
				edactSelectAll( Level );
				Trans->End();
				RedrawLevel( Level );
				NoteSelectionChange( Level );
				Processed=1;
			}
#if 1 //WOT
			else if( ParseCommand(&Str,"INSIDE" ) ) // ACTOR SELECT INSIDE
			{
				Trans->Begin( Level, "Select Inside" );
				Level->Modify();
				edactSelectInside( Level );
				Trans->End();
				RedrawLevel( Level );
				NoteSelectionChange( Level );
				Processed=1;
			}
			else if( ParseCommand(&Str,"INVERT" ) ) // ACTOR SELECT INVERT
			{
				Trans->Begin( Level, "Select Invert" );
				Level->Modify();
				edactSelectInvert( Level );
				Trans->End();
				RedrawLevel( Level );
				NoteSelectionChange( Level );
				Processed=1;
			}
#endif
			else if( ParseCommand(&Str,"OFCLASS") ) // ACTOR SELECT OFCLASS CLASS=<class>
			{
				UClass* Class;
				if( ParseObject<UClass>(Str,"CLASS=",Class,ANY_PACKAGE) )
				{
					Trans->Begin( Level, "Select of class" );
					Level->Modify();
					edactSelectOfClass( Level, Class );
					Trans->End();
					RedrawLevel( Level );
					NoteSelectionChange( Level );
				}
				else Out->Log(NAME_ExecWarning,"Missing class");
				Processed=1;
			}
		}
		else if( ParseCommand(&Str,"DELETE") )
		{
			Trans->Begin( Level, "Delete Actors" );
			Level->Modify();
			edactDeleteSelected( Level );
			Trans->End();
			RedrawLevel( Level );
			NoteSelectionChange( Level );
			Processed=1;
		}
		else if( ParseCommand(&Str,"RESET") )
		{
			Trans->Begin( Level, "Reset Actors" );
			Level->Modify();
			UBOOL Location=0;
			UBOOL Rotation=0;
			UBOOL Scale=0;
			if( ParseCommand(&Str,"LOCATION") )
			{
				Location=1;
				ResetPivot();
			}
			else if( ParseCommand(&Str,"ROTATION") )
			{
				Rotation=1;
			}
			else if( ParseCommand(&Str,"SCALE") )
			{
				Scale=1;
			}
			else if( ParseCommand(&Str,"ALL") )
			{
				Location=Rotation=Scale=1;
				ResetPivot();
			}
			for( INT i=0; i<Level->Num(); i++ )
			{
				AActor* Actor=Level->Element(i);
				if( Actor && Actor->bSelected )
				{
					Actor->Modify();
					if( Location ) Actor->Location  = FVector(0,0,0);
					if( Location ) Actor->PrePivot  = FVector(0,0,0);
					if( Rotation ) Actor->Rotation  = FRotator(0,0,0);
					if( Scale    ) Actor->DrawScale = 1.0;
					if( Scale && Cast<ABrush>(Actor) )
					{
						Cast<ABrush>(Actor)->MainScale=GMath.UnitScale;
						Cast<ABrush>(Actor)->PostScale=GMath.UnitScale;
					}
				}
			}
			Trans->End();
			RedrawLevel( Level );
			Processed=1;
		}
		else if( ParseCommand(&Str,"DUPLICATE") )
		{
			Trans->Begin( Level, "Duplicate Actors" );
			Level->Modify();
			edactDuplicateSelected( Level );
			Trans->End();
			RedrawLevel( Level );
			NoteSelectionChange( Level );
			Processed=1;
		}
		else if( ParseCommand(&Str,"KEYFRAME") )
		{
			INT Num=0;
			Parse(Str,"NUM=",Num);
			Trans->Begin( Level, "Set mover keyframe" );
			Level->Modify();
			for( INT i=0; i<Level->Num(); i++ )
			{
				AMover* Mover=Cast<AMover>(Level->Element(i));
				if( Mover && Mover->bSelected )
				{
					Mover->Modify();
					Mover->KeyNum = Num;
					Mover->PostEditChange();
					SetPivot( Mover->Location, 0, 0 );
				}
			}
			Trans->End();
			RedrawLevel( Level );
			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// POLY: Polygon adjustment and mapping
	//
	else if (ParseCommand(&Str,"POLY"))
		{
		if (ParseCommand(&Str,"SELECT")) // POLY SELECT [ALL/NONE/INVERSE] FROM [LEVEL/SOLID/GROUP/ITEM/ADJACENT/MATCHING]
		{
			appSprintf( TempStr, "POLY SELECT %s", Str );
			if( ParseCommand(&Str,"NONE") )
			{
				return Exec( "SELECT NONE" );
				Processed=1;
			}
			else if( ParseCommand(&Str,"ALL") )
			{
				Trans->Begin( Level, TempStr );
				SelectNone( Level, 0 );
				polySelectAll( Level->Model );
				NoteSelectionChange( Level );
				Processed=1;
				Trans->End();
			}
			else if( ParseCommand(&Str,"REVERSE") )
			{
				Trans->Begin( Level, TempStr );
				polySelectReverse (Level->Model);
				EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				Trans->End();
			}
			else if( ParseCommand(&Str,"MATCHING") )
			{
				Trans->Begin( Level, TempStr );
				if 		(ParseCommand(&Str,"GROUPS"))		polySelectMatchingGroups(Level->Model);
				else if (ParseCommand(&Str,"ITEMS"))		polySelectMatchingItems(Level->Model);
				else if (ParseCommand(&Str,"BRUSH"))		polySelectMatchingBrush(Level->Model);
				else if (ParseCommand(&Str,"TEXTURE"))		polySelectMatchingTexture(Level->Model);
				EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				Trans->End();
			}
			else if( ParseCommand(&Str,"ADJACENT") )
			{
				Trans->Begin( Level, TempStr );
				if 	  (ParseCommand(&Str,"ALL"))			polySelectAdjacents( Level->Model );
				else if (ParseCommand(&Str,"COPLANARS"))	polySelectCoplanars( Level->Model );
				else if (ParseCommand(&Str,"WALLS"))		polySelectAdjacentWalls( Level->Model );
				else if (ParseCommand(&Str,"FLOORS"))		polySelectAdjacentFloors( Level->Model );
				else if (ParseCommand(&Str,"CEILINGS"))		polySelectAdjacentFloors( Level->Model );
				else if (ParseCommand(&Str,"SLANTS"))		polySelectAdjacentSlants( Level->Model );
				EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				Trans->End();
			}
			else if( ParseCommand(&Str,"MEMORY") )
			{
				Trans->Begin( Level, TempStr );
				if 		(ParseCommand(&Str,"SET"))			polyMemorizeSet( Level->Model );
				else if (ParseCommand(&Str,"RECALL"))		polyRememberSet( Level->Model );
				else if (ParseCommand(&Str,"UNION"))		polyUnionSet( Level->Model );
				else if (ParseCommand(&Str,"INTERSECT"))	polyIntersectSet( Level->Model );
				else if (ParseCommand(&Str,"XOR"))			polyXorSet( Level->Model );
				EdCallback(EDC_SelPolyChange,0);
				Processed=1;
				Trans->End();
			}
			RedrawLevel(Level);
		}
		else if (ParseCommand(&Str,"DEFAULT")) // POLY DEFAULT <variable>=<value>...
		{
			CurrentTexture=NULL;
			ParseObject<UTexture>(Str,"TEXTURE=",CurrentTexture,ANY_PACKAGE);
			Processed=1;
		}
		else if (ParseCommand(&Str,"SETTEXTURE"))
		{
			Trans->Begin(Level,"Poly SetTexture");
			Level->Model->Surfs->ModifySelected(1);
			for( Index1=0; Index1<Level->Model->Surfs->Num(); Index1++ )
			{
				if( Level->Model->Surfs->Element(Index1).PolyFlags & PF_Selected )
				{
					Level->Model->Surfs->Element(Index1).Texture = CurrentTexture;
					polyUpdateMaster( Level->Model, Index1, 0, 0 );
				}
			}
			Trans->End();
			RedrawLevel(Level);
			Processed=1;
		}
		else if (ParseCommand(&Str,"SET")) // POLY SET <variable>=<value>...
			{
			//
			// Options: TEXTURE=name SETFLAGS=value CLEARFLAGS=value
			//          UPAN=value VPAN=value ROTATION=value XSCALE=value YSCALE=value
			//
			Trans->Begin(Level,"Poly Set");
			Level->Model->Surfs->ModifySelected(1);
			UTexture *Texture;
			if (ParseObject<UTexture>(Str,"TEXTURE=",Texture,ANY_PACKAGE))
				{
				for (Index1=0; Index1<Level->Model->Surfs->Num(); Index1++)
					{
					if (Level->Model->Surfs->Element(Index1).PolyFlags & PF_Selected)
						{
						Level->Model->Surfs->Element(Index1).Texture  = Texture;
						polyUpdateMaster( Level->Model, Index1, 0, 0 );
						};
					};
				};
			Word4  = 0;
			INT DWord1 = 0;
			INT DWord2 = 0;
			if (Parse(Str,"SETFLAGS=",DWord1))   Word4=1;
			if (Parse(Str,"CLEARFLAGS=",DWord2)) Word4=1;
			if (Word4)  polySetAndClearPolyFlags (Level->Model,DWord1,DWord2,1,1); // Update selected polys' flags
			//
			Trans->End();
			RedrawLevel(Level);
			Processed=1;
			}
		else if (ParseCommand(&Str,"TEXSCALE")) // POLY TEXSCALE [U=..] [V=..] [UV=..] [VU=..]
			{
			Trans->Begin 				(Level,"Poly Texscale");
			Level->Model->Surfs->ModifySelected(1);
			//
			Word2 = 1; // Scale absolute
			if( ParseCommand(&Str,"RELATIVE") )
				Word2=0;
			TexScale:
			//
			FLOAT UU,UV,VU,VV;
			UU=1.0; Parse (Str,"UU=",UU);
			UV=0.0; Parse (Str,"UV=",UV);
			VU=0.0; Parse (Str,"VU=",VU);
			VV=1.0; Parse (Str,"VV=",VV);
			//
			polyTexScale( Level->Model, UU, UV, VU, VV, Word2 );
			//
			Trans->End			();
			RedrawLevel(Level);
			Processed=1;
			}
		else if (ParseCommand(&Str,"TEXMULT")) // POLY TEXMULT [U=..] [V=..]
			{
			Trans->Begin( Level, "Poly Texmult" );
			Level->Model->Surfs->ModifySelected( 1 );
			Word2 = 0; // Scale relative;
			goto TexScale;
			}
		else if (ParseCommand(&Str,"TEXPAN")) // POLY TEXPAN [RESET] [U=..] [V=..]
			{
			Trans->Begin( Level, "Poly Texpan" );
			Level->Model->Surfs->ModifySelected(1);
			if( ParseCommand (&Str,"RESET") )
				polyTexPan( Level->Model, 0, 0, 1 );
			Word1 = 0; Parse (Str,"U=",Word1);
			Word2 = 0; Parse (Str,"V=",Word2);
			polyTexPan( Level->Model, Word1, Word2, 0 );
			Trans->End();
			RedrawLevel( Level );
			Processed=1;
			}
		else if (ParseCommand(&Str,"TEXALIGN")) // POLY TEXALIGN [FLOOR/GRADE/WALL/NONE]
		{
			ETexAlign TexAlign;
			if		(ParseCommand (&Str,"DEFAULT"))	TexAlign = TEXALIGN_Default;
			else if (ParseCommand (&Str,"FLOOR"))		TexAlign = TEXALIGN_Floor;
			else if (ParseCommand (&Str,"WALLDIR"))	TexAlign = TEXALIGN_WallDir;
			else if (ParseCommand (&Str,"WALLPAN"))	TexAlign = TEXALIGN_WallPan;
			else if (ParseCommand (&Str,"WALLCOLUMN"))TexAlign = TEXALIGN_WallColumn;
			else if (ParseCommand (&Str,"ONETILE"))	TexAlign = TEXALIGN_OneTile;
			else								goto Skip;
			{
				INT DWord1=0;
				Parse( Str, "TEXELS=", DWord1 );
				Trans->Begin( Level, "Poly Texalign" );
				Level->Model->Surfs->ModifySelected( 1 );
				polyTexAlign( Level->Model, TexAlign, DWord1 );
				Trans->End();
				RedrawLevel( Level );
				Processed=1;
			}
			Skip:;
		}
	}
	//------------------------------------------------------------------------------------
	// TEXTURE management:
	//
	else if( ParseCommand(&Str,"Texture") )
	{
		if( ParseCommand(&Str,"Clear") )
		{
			UTexture* Texture;
			if( ParseObject<UTexture>(Str,"NAME=",Texture,ANY_PACKAGE) )
				Texture->Clear( TCLEAR_Temporal );
		}
		else if( ParseCommand(&Str,"New") )
		{
			FName GroupName=NAME_None;
			FName PackageName;
			UClass* TextureClass;
			INT USize, VSize;
			if
			(	Parse( Str, "NAME=",    TempName, NAME_SIZE )
			&&	ParseObject<UClass>( Str, "CLASS=", TextureClass, ANY_PACKAGE )
			&&	Parse( Str, "USIZE=",   USize )
			&&	Parse( Str, "VSIZE=",   VSize )
			&&	Parse( Str, "PACKAGE=", PackageName )
			&&	TextureClass->IsChildOf( UTexture::StaticClass ) 
			&&	PackageName!=NAME_None )
			{
				UPackage* Pkg = GObj.CreatePackage(NULL,*PackageName);
				if( Parse( Str, "GROUP=", GroupName ) && GroupName!=NAME_None )
					Pkg = GObj.CreatePackage(Pkg,*GroupName);
				if( !GObj.FindObject( TextureClass, Pkg, TempName ) )
				{
					// Create new texture object.
					UTexture* Result = (UTexture*)GObj.ConstructObject( TextureClass, Pkg, TempName, RF_Public|RF_Standalone );
					if( !Result->Palette )
					{
						Result->Palette = new( Result->GetParent(), NAME_None, RF_Public )UPalette;
						Result->Palette->Colors.Add( 256 );
					}
					Result->Init( USize, VSize );
					Result->PostLoad();
					Result->Clear( TCLEAR_Temporal | TCLEAR_Bitmap );
				}
				else Out->Logf( NAME_ExecWarning, "Texture exists" );
			}
			else Out->Logf( NAME_ExecWarning, "Bad TEXTURE NEW" );
			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// MODE management (Global EDITOR mode):
	//
	else if( ParseCommand(&Str,"MODE") )
		{
		Word1 = Mode;  // To see if we should redraw
		Word2 = Mode;  // Destination mode to set
		//
		UBOOL DWord1;
		if( ParseUBOOL(Str,"GRID=", DWord1) )
		{
			FinishAllSnaps (Level);
			Constraints.GridEnabled = DWord1;
			Word1=MAXWORD;
		}
		if( ParseUBOOL(Str,"ROTGRID=", DWord1) )
		{
			FinishAllSnaps (Level);
			Constraints.RotGridEnabled=DWord1;
			Word1=MAXWORD;
		}
		if( ParseUBOOL(Str,"SNAPVERTEX=", DWord1) )
		{
			FinishAllSnaps (Level);
			Constraints.SnapVertices=DWord1;
			Word1=MAXWORD;
		}
		Parse( Str, "SPEED=", MovementSpeed );
		Parse( Str, "SNAPDIST=", Constraints.SnapDistance );
		//
		// Major modes:
		//
		if 		(ParseCommand(&Str,"CAMERAMOVE"))		Word2 = EM_ViewportMove;
		else if	(ParseCommand(&Str,"CAMERAZOOM"))		Word2 = EM_ViewportZoom;
		else if	(ParseCommand(&Str,"BRUSHROTATE"))	Word2 = EM_BrushRotate;
		else if	(ParseCommand(&Str,"BRUSHSHEER"))		Word2 = EM_BrushSheer;
		else if	(ParseCommand(&Str,"BRUSHSCALE"))		Word2 = EM_BrushScale;
		else if	(ParseCommand(&Str,"BRUSHSTRETCH"))	Word2 = EM_BrushStretch;
		else if	(ParseCommand(&Str,"BRUSHSNAP")) 		Word2 = EM_BrushSnap;
		else if	(ParseCommand(&Str,"TEXTUREPAN"))		Word2 = EM_TexturePan;
		else if	(ParseCommand(&Str,"TEXTUREROTATE"))	Word2 = EM_TextureRotate;
		else if	(ParseCommand(&Str,"TEXTURESCALE")) 	Word2 = EM_TextureScale;
		//
		if( Word2 != Word1 )
		{
			edcamSetMode( Word2 );
			RedrawLevel( Level );
		}
		Processed=1;
		}
	//------------------------------------------------------------------------------------
	// Transaction tracking and control
	//
	else if( ParseCommand(&Str,"TRANSACTION") )
	{
		if( ParseCommand(&Str,"UNDO") )
		{
			if( Trans->Undo( Level ) )
				RedrawLevel(Level);
			Processed=1;
		}
		else if( ParseCommand(&Str,"REDO") )
		{
			if( Trans->Redo( Level ) )
				RedrawLevel(Level);
			Processed=1;
		}
		NoteSelectionChange( Level );
		EdCallback( EDC_MapChange, 0 );
	}
	//------------------------------------------------------------------------------------
	// General objects
	//
	else if( ParseCommand(&Str,"OBJ") )
	{
		if( ParseCommand(&Str,"EXPORT") )
		{
			FName Package=NAME_None;
			UClass* Type;
			UObject* Res;
			Parse( Str, "PACKAGE=", Package );
			if
			(	ParseObject<UClass>( Str, "TYPE=", Type, ANY_PACKAGE )
			&&	Parse( Str, "FILE=", TempFname, 80 )
			&&	ParseObject( Str, "NAME=", Type, Res, NULL ) )
			{
				for( FObjectIterator It; It; ++It )
					It->ClearFlags( RF_TagImp | RF_TagExp );
				Res->ExportToFile( TempFname );
			}
			else Out->Log( NAME_ExecWarning, "Missing file, name, or type" );
			Processed = 1;
		}
		else if( ParseCommand(&Str,"SavePackage") )
		{
			UPackage* Pkg;
			if
			(	Parse( Str, "File=", TempFname, 79 ) 
			&&	ParseObject<UPackage>( Str, "PACKAGE=", Pkg, NULL ) )
			{
				GSystem->BeginSlowTask("Saving textures",1,0);
				GObj.SavePackage( Pkg, NULL, RF_Standalone, TempFname );
				GSystem->EndSlowTask();
			}
			else Out->Log( NAME_ExecWarning, "Missing filename" );
			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// CLASS functions
	//
	else if( ParseCommand(&Str,"CLASS") )
	{
		if( ParseCommand(&Str,"SPEW") )
		{
			UBOOL All = ParseCommand(&Str,"ALL");
			for( TObjectIterator<UClass> It; It; ++It )
			{
				if( It->ScriptText && (All || (It->GetFlags() & RF_SourceModified)) )
				{
					// Make package directory.
					appStrcpy( TempFname, "..\\" );
					appStrcat( TempFname, It->GetParent()->GetName() );
					appMkdir( TempFname );

					// Make package\Classes directory.
					appStrcat( TempFname, "\\Classes" );
					appMkdir( TempFname );

					// Save file.
					appStrcat( TempFname, "\\" );
					appStrcat( TempFname, It->GetName() );
					appStrcat( TempFname, ".uc" );
					debugf( NAME_Log, "Spewing: %s", TempFname );
					It->ExportToFile( TempFname );
				}
			}
			Processed=1;
		}
		else if( ParseCommand(&Str,"LOAD") ) // CLASS LOAD FILE=..
		{
			if( Parse( Str, "FILE=", TempFname, 80 ) )
			{
				Out->Logf( "Loading class from %s...", TempFname );
				if( appStrfind(TempFname,"UC") )
				{
					FName PkgName, ObjName;
					if
					(	Parse(Str,"PACKAGE=",PkgName)
					&&	Parse(Str,"NAME=",ObjName) )
					{
						// Import it.
						ImportObjectFromFile<UClass>( GObj.CreatePackage(NULL,*PkgName), ObjName, TempFname, GSystem );
					}
					else Out->Log("Missing package name");
				}
				else if( appStrfind( TempFname, "U") )
				{
					// Load from Unrealfile.
					GObj.LoadPackage( NULL, TempFname, LOAD_KeepImports );
				}
				else Out->Log( NAME_ExecWarning, "Unrecognized file type" );
			}
			else Out->Log(NAME_ExecWarning,"Missing filename");
			Processed=1;
		}
		else if( ParseCommand(&Str,"NEW") ) // CLASS NEW
		{
			UClass *Parent;
			FName PackageName;
			if
			(	ParseObject<UClass>( Str, "PARENT=", Parent, ANY_PACKAGE )
			&&	Parse( Str, "PACKAGE=", PackageName )
			&&	Parse( Str, "NAME=", TempStr, NAME_SIZE ) )
			{
				UPackage* Pkg = GObj.CreatePackage(NULL,*PackageName);
				UClass* Class = new( Pkg, TempStr )UClass( Parent );
				if( Class )
					Class->ScriptText = new( Class->GetParent(), TempStr, RF_NotForClient|RF_NotForServer )UTextBuffer;
				else
					Out->Log( NAME_ExecWarning, "Class not found" );
			}
			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// SCRIPT: script compiler
	//
	else if( ParseCommand(&Str,"SCRIPT") )
	{
		if( ParseCommand(&Str,"MAKE") )
		{
			GSystem->BeginSlowTask( "Compiling scripts", 0, 0 );
			UBOOL All  = ParseCommand(&Str,"ALL");
			UBOOL Boot = ParseCommand(&Str,"BOOT");
			MakeScripts(All,Boot);
			GSystem->EndSlowTask();
			UpdatePropertiesWindows();
			Processed=1;
		}
	}
	//------------------------------------------------------------------------------------
	// CAMERA: cameras
	//
	else if( ParseCommand(&Str,"CAMERA") )
	{
		UBOOL DoUpdate = ParseCommand(&Str,"UPDATE");
		UBOOL DoOpen   = ParseCommand(&Str,"OPEN");
		if( (DoUpdate || DoOpen) && Level )
		{
			UViewport* Viewport;
			int Temp=0;
			char TempStr[NAME_SIZE];
			if( Parse( Str, "NAME=", TempStr,NAME_SIZE ) )
			{
				Viewport = FindObject<UViewport>( GObj.GetTransientPackage(), TempStr );
				if( !Viewport )
				{
					Viewport = Client->NewViewport( Level, TempStr );
					DoOpen = 1;
				}
				else Temp=1;
			}
			else
			{
				Viewport = Client->NewViewport( Level, NAME_None );
				DoOpen = 1;
			}
			check(Viewport->Actor!=NULL);

			DWORD hWndParent=0;
			Parse(Str,"HWND=",hWndParent);

			INT NewX=Viewport->SizeX, NewY=Viewport->SizeY;
			Parse( Str, "XR=", NewX ); if( NewX<0 ) NewX=0;
			Parse( Str, "YR=", NewY ); if( NewY<0 ) NewY=0;
			Viewport->Actor->FovAngle = FovAngle;

			Viewport->Actor->Misc1=0;
			Viewport->Actor->Misc2=0;
			Viewport->MiscRes=NULL;
			Parse(Str,"FLAGS=",Viewport->Actor->ShowFlags);
			Parse(Str,"REN=",  Viewport->Actor->RendMap);
			Parse(Str,"MISC1=",Viewport->Actor->Misc1);
			Parse(Str,"MISC2=",Viewport->Actor->Misc2);
			FName GroupName=NAME_None;
			if( Parse(Str,"GROUP=",GroupName) )
				Viewport->Group = GroupName;
			if( appStricmp(*Viewport->Group,"(All)")==0 )
				Viewport->Group = NAME_None;

			switch( Viewport->Actor->RendMap )
			{
				case REN_TexView:
					ParseObject<UTexture>(Str,"TEXTURE=",*(UTexture **)&Viewport->MiscRes,ANY_PACKAGE); 
					if( !Viewport->MiscRes )
						Viewport->MiscRes = Viewport->Actor->Level->DefaultTexture;
					break;
				case REN_MeshView:
					if( !Temp )
					{
						Viewport->Actor->Location = FVector(100.0,100.0,+60.0);
						Viewport->Actor->ViewRotation.Yaw=0x6000;
					}
					ParseObject<UMesh>( Str, "MESH=", *(UMesh**)&Viewport->MiscRes, ANY_PACKAGE ); 
					break;
				case REN_TexBrowser:
					ParseObject<UPackage>(Str,"PACKAGE=",*(UPackage**)&Viewport->MiscRes,NULL);
					break;
			}
			if( DoOpen )
			{
				INT OpenX = INDEX_NONE;
				INT OpenY = INDEX_NONE;
				Parse( Str, "X=", OpenX );
				Parse( Str, "Y=", OpenY );
				Viewport->OpenWindow( hWndParent, 0, NewX, NewY, OpenX, OpenY );
				if( appStricmp(Viewport->GetName(),"Standard3V")==0 )
					ResetSound();
			}
			else Draw( Viewport, 0 );
			return 1;
		}
		else if( ParseCommand(&Str,"HIDESTANDARD") )
		{
			Client->ShowViewportWindows( SHOW_StandardView, 0 );
			return 1;
		}
		else if( ParseCommand(&Str,"CLOSE") )
		{
			UViewport* Viewport;
			if( ParseCommand(&Str,"ALL") )
			{
				for( int i=Client->Viewports.Num()-1; i; i-- )
					delete Client->Viewports(i);
			}
			else if( ParseCommand(&Str,"FREE") )
			{
				for( int i=Client->Viewports.Num()-1; i; i-- )
					if( appStrstr( Client->Viewports(i)->GetName(), "STANDARD" )==0 )
						delete Client->Viewports(i);
			}
			else if( ParseObject<UViewport>(Str,"NAME=",Viewport,GObj.GetTransientPackage()) )
			{
				delete Viewport;
			}
			else Out->Log( "Missing name" );
			return 1;
		}
		else return 0;
	}
	//------------------------------------------------------------------------------------
	// Level.
	//
	if( ParseCommand(&Str,"LEVEL") )
	{
		if( ParseCommand(&Str,"REDRAW") )
		{
			RedrawLevel(Level);
			return 1;
		}
		else if( ParseCommand(&Str,"LINKS") )
		{
			Results->Text.Empty();
			int Internal=0,External=0;
			Results->Logf("Level links:\r\n");
			for( int i=0; i<Level->Num(); i++ )
			{
				if( Level->Actors(i) && Level->Actors(i)->IsA(ATeleporter::StaticClass) )
				{
					ATeleporter& Teleporter = *(ATeleporter *)Level->Actors(i);
					Results->Logf("   %s\r\n",Teleporter.URL);
					if( appStrchr(Teleporter.URL,'//') )
						External++;
					else
						Internal++;
				}
			}
			Results->Logf("End, %i internal link(s), %i external.\r\n",Internal,External);
			return 1;
		}
		else if( ParseCommand(&Str,"VALIDATE") )
		{
			// Validate the level.
			Results->Text.Empty();
			int Errors=0, Warnings=0;
			Results->Log("Level validation:\r\n");

			// Make sure it's not empty.
			if( Level->Model->Nodes->Num() == 0 )
			{
				Results->Log("Error: Level is empty!\r\n");
				return 1;
			}

			// Find playerstart.
			for( int i=0; i<Level->Num(); i++ )
				if( Level->Actors(i) && Level->Actors(i)->IsA(APlayerStart::StaticClass) )
					break;
			if( i == Level->Num() )
			{
				Results->Log( "Error: Missing PlayerStart actor!\r\n" );
				return 1;
			}

			// Make sure PlayerStarts are outside.
			for( i=0; i<Level->Num(); i++ )
			{
				if( Level->Actors(i) && Level->Actors(i)->IsA(APlayerStart::StaticClass) )
				{
					FCheckResult Hit(0.0);
					if( !Level->Model->PointCheck( Hit, NULL, Level->Actors(i)->Location, FVector(0,0,0), 0 ) )
					{
						Results->Log( "Error: PlayerStart doesn't fit!\r\n" );
						return 1;
					}
				}
			}

			// Check scripts.
			if( GEditor && !GEditor->CheckScripts( UObject::StaticClass, *Results ) )
			{
				Results->Logf( "\r\nError: Scripts need to be rebuilt!\r\n" );
				return 1;
			}

			// Check level title.
			if( Level->GetLevelInfo()->Title[0]==0 )
			{
				Results->Logf( "Error: Level is missing a title!" );
				return 1;
			}
			else if( appStricmp(Level->GetLevelInfo()->Title,"Untitled")==0 )
			{
				Results->Logf( "Warning: Level is untitled\r\n" );
			}

			// Check actors.
			for( i=0; i<Level->Num(); i++ )
			{
				AActor* Actor = Level->Actors(i);
				if( Actor )
				{
					guard(CheckingActors);
					check(Actor->GetClass()!=NULL);
					check(Actor->GetMainFrame());
					check(Actor->GetMainFrame()->Object==Actor);
					check(Actor->Level!=NULL);
					check(Actor->XLevel!=NULL);
					check(Actor->XLevel==Level);
					check(Actor->XLevel->Actors(0)!=NULL);
					check(Actor->XLevel->Actors(0)==Actor->Level);
					unguardf(( "(%i %s)", i, Actor->GetFullName() ));
				}
			}

			// Success.
			Results->Logf("Success: Level validation succeeded!\r\n");
			return 1;
		}
		else
		{
			return 0;
		}
	}
	//------------------------------------------------------------------------------------
	// Other handlers.
	//
	else if( ParseCommand(&Str,"FIX") )
	{
		for( int i=0; i<Level->Num(); i++ )
			if( Level->Actors(i) )
				Level->Actors(i)->SoundRadius = Clamp(4*(INT)Level->Actors(i)->SoundRadius,0,255);
	}
	else if( ParseCommand(&Str,"MAYBEAUTOSAVE") )
	{
		if( AutoSave && ++AutoSaveCount>=AutosaveTimeMinutes )
		{
			AutoSaveIndex = (AutoSaveIndex+1)%10;
			SaveConfig();
			char Cmd[256];
			appSprintf( Cmd, "MAP SAVE FILE=%s..\\Maps\\Auto%i.unr", appBaseDir(), AutoSaveIndex );
			debugf( NAME_Log, "Autosaving '%s'", Cmd );
			Exec( Cmd, Out );
			AutoSaveCount=0;
		}
	}
	else if( ParseCommand(&Str,"HOOK") )
	{
		return HookExec( Str, Out );
	}
	else if( ParseCommand(&Str,"AUDIO") )
	{
		if( ParseCommand(&Str,"PLAY") )
		{
			UViewport* Viewport=NULL;
			for( int i=0; i<Client->Viewports.Num(); i++ )
				if( appStricmp(Client->Viewports(i)->GetName(), "Standard3V")==0 )
					Viewport=Client->Viewports(i);
			if( !Viewport || !Audio )
			{
				Out->Logf( "Can't find viewport for sound" );
			}
			else
			{
				USound* Sound;
				if( ParseObject<USound>( Str, "NAME=", Sound, ANY_PACKAGE ) )
					Audio->PlaySound( Viewport->Actor, 2*SLOT_Misc, Sound, Viewport->Actor->Location, 1.0, 4096.0, 1.0 );
			}
			Processed = 1;
		}
	}
	else if( ParseCommand(&Str,"SETCURRENTCLASS") )
	{
		ParseObject<UClass>( Str, "CLASS=", CurrentClass, ANY_PACKAGE );
	}
	else if( ParseCommand(&Str,"MUSIC") )
	{
		UViewport* Viewport=NULL;
		for( int i=0; i<Client->Viewports.Num(); i++ )
			if( appStricmp(Client->Viewports(i)->GetName(), "Standard3V")==0 )
				Viewport=Client->Viewports(i);
		if( !Viewport || !Audio )
		{
			Out->Logf( "Can't find viewport for music" );
		}
		else if( ParseCommand(&Str,"PLAY") )
		{
			UMusic* Music;
			if( ParseObject<UMusic>(Str,"NAME=",Music,ANY_PACKAGE) )
			{
				Viewport->Actor->Song        = Music;
				Viewport->Actor->SongSection = 0;
				Viewport->Actor->Transition  = MTRAN_Fade;
			}
		}
		Processed = 1;
	}
	else if( Level && Level->Exec(Stream,Out) )
	{
		// The level handled it.
		Processed = 1;
	}
	else if( UEngine::Exec(Stream,Out) )
	{
		// The engine handled it.
		Processed = 1;
	}
	else if( ParseCommand(&Str,"SELECTNAME") )
	{
		FName FindName=NAME_None;
		Parse( Str, "NAME=", FindName );
		for( INT i=0; i<Level->Num(); i++ )
			if( Level->Element(i) )
				Level->Element(i)->bSelected = Level->Element(i)->GetFName()==FindName;
	}
	else if( ParseCommand(&Str,"DUMPINT") )
	{
		while( *Str==' ' )
			Str++;
		UObject* Pkg = GObj.LoadPackage( NULL, Str, LOAD_AllowDll );
		if( Pkg )
		{
			char Tmp[256],Loc[256];
			appStrcpy( Tmp, Str );
			if( appStrchr(Tmp,'.') )
				*appStrchr(Tmp,'.') = 0;
			appStrcat( Tmp, ".int" );
			appStrcpy( Loc, appBaseDir() );
			appStrcat( Loc, Tmp );
			for( FObjectIterator It; It; ++It )
			{
				if( It->IsIn(Pkg) )
				{
					char Value[1024];
					UClass* Class = Cast<UClass>( *It );
					if( Class )
					{
						// Generate localizable class defaults.
						for( TFieldIterator<UProperty> ItP(Class); ItP; ++ItP )
							if( ItP->PropertyFlags & CPF_Localized )
								for( INT i=0; i<ItP->ArrayDim; i++ )
									if( ItP->ExportText( i, Value, &Class->Defaults(0), Class->GetSuperClass() ? &Class->GetSuperClass()->Defaults(0) : NULL, 1 ) )
										SetConfigString( Class->GetName(), ItP->GetName(), Value, Loc );
					}
					else
					{
						// Generate localizable object properties.
						for( TFieldIterator<UProperty> ItP(It->GetClass()); ItP; ++ItP )
							if( ItP->PropertyFlags & CPF_Localized )
								for( INT i=0; i<ItP->ArrayDim; i++ )
									if( ItP->ExportText( i, Value, (BYTE*)*It, &It->GetClass()->Defaults(0), 1 ) )
										for( INT i=0; i<ItP->ArrayDim; i++ )
											SetConfigString( It->GetName(), ItP->GetName(), Value, Loc );
					}
				}
			}
			Out->Logf( "Generated %s", Loc );
		}
		else Out->Logf( "LoadPackage failed" );
		return 1;
	}
	else if( ParseCommand(&Str,"JUMPTO") )
	{
		char A[32], B[32], C[32];
		ParseToken( Str, A, ARRAY_COUNT(A), 0 );
		ParseToken( Str, B, ARRAY_COUNT(B), 0 );
		ParseToken( Str, C, ARRAY_COUNT(C), 0 );
		for( INT i=0; i<Client->Viewports.Num(); i++ )
			Client->Viewports(i)->Actor->Location = FVector(appAtoi(A),appAtoi(B),appAtoi(C));
		return 1;
	}
	else if( ParseCommand(&Str,"TESTME") )
	{
		/*
		INT b=0,c=0,d=0;
		for( INT i=0; i<Level->Model->LightMap.Num(); i++ )
		{
			FLightMapIndex& Index=Level->Model->LightMap(i);
			d+=Index.UClamp*Index.VClamp;
			if( Index.iLightActors!=INDEX_NONE )
			{
				for( AActor** A=&Level->Model->Lights(Index.iLightActors); *A; A++ )
					b++,c+=Index.UClamp*Index.VClamp;
			}
		}
		debugf( NAME_Log, "%i lightsurfs, %i surfs, %i elements, %f factor", b, Level->Model->Surfs->Num(), d, (FLOAT)c/(FLOAT)d );
		*/
		/*
		INT a=0,b=0;
		for( INT i=0; i<Level->Model->LightMap.Num(); i++ )
		{
			FLightMapIndex& Index=Level->Model->LightMap(i);
			a+=Max(Index.UClamp-2,2)*Max(Index.VClamp-2,2);
			b+=FNextPowerOfTwo(Max(Index.UClamp-2,2))*FNextPowerOfTwo(Max(Index.VClamp-2,2));
		}
		debugf( NAME_Log, "%f",FLOAT(b)/FLOAT(a) );
		*/
		/*
		// For testing.
		void TimTim( ULevel* );
		TimTim( Level );
		*/
	}
	return Processed;
	unguardf(( "(%s)%s", ErrorTemp, appStrlen(ErrorTemp)>=69 ? ".." : "" ));
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
