/*=============================================================================
	UnEdFact.cpp: Editor class factories.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#include "EditorPrivate.h"

/*------------------------------------------------------------------------------
	UClassFactory implementation.
------------------------------------------------------------------------------*/

UClassFactory::UClassFactory()
{
	guard(UClassFactory::UClassFactory);

	// Init UFactory properties.
	SupportedClass = UClass::StaticClass;
	new(Formats)FString("uc;Unreal class definitions");
	bCreateNew = 0;

	unguard;
}
UObject* UClassFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			Type,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(UClassFactory::Create);
	const char* InBuffer=Buffer;
	char StrLine[256], ClassName[32]="", BaseClassName[32]="";

	// Validate format.
	if( Class != UClass::StaticClass )
	{
		Warn->Warnf( "Can only import classes", Type );
		return NULL;
	}
	if( appStricmp(Type,"UC")!=0 )
	{
		Warn->Warnf( "Can't import classes from files of type '%s'", Type );
		return NULL;
	}

	// Import the script text.
	UTextBuffer* ScriptText = new UTextBuffer;
	UTextBuffer* DefaultPropText = new UTextBuffer;
	while( ParseLine(&Buffer,StrLine,ARRAY_COUNT(StrLine),1) )
	{
		const char* Str = &StrLine[0], *Temp;
		if( ParseCommand(&Str,"defaultproperties") )
		{
			// Get default properties text.
			while( ParseLine(&Buffer,StrLine,256,1) )
			{
				Str = &StrLine[0];
				ParseNext( &Str );
				if( *Str=='}' )
					break;
				DefaultPropText->Logf( "%s\r\n", StrLine );
			}
		}
		else
		{
			// Get script text.
			ScriptText->Logf( "%s\r\n", StrLine );

			// Stub out the comments.
			if( appStrstr(StrLine, "//") )
				*appStrstr(StrLine, "//") = 0;

			// Get class name.
			if( !ClassName[0] && (Temp=appStrfind(Str, "class"))!=0 )
			{
				Temp+=6;
				ParseToken( Temp, ClassName, ARRAY_COUNT(ClassName), 0 );
			}
			if( !BaseClassName[0] && (Temp=appStrfind(Str, "expands"))!=0 )
			{
				Temp+=7;
				ParseToken( Temp, BaseClassName, ARRAY_COUNT(BaseClassName), 0 );
				int c = appStrlen(BaseClassName);
				while( c>0 && BaseClassName[c-1]==';' )
					c--;
				BaseClassName[c] = 0;
			}
		}
	}

	// Handle failure.
	if( !ClassName[0] || (!BaseClassName[0] && !appStricmp(ClassName,"Object")) )
	{
		Warn->Warnf( "Bad class definition '%s'/'%s'/%i/%i", ClassName, BaseClassName, BufferEnd-InBuffer, appStrlen(InBuffer) );
		if( ScriptText )
			delete ScriptText;
		return NULL;
	}
	else if( appStricmp(ClassName,*Name)!=0 )
	{
		Warn->Warnf( "Script vs. class name mismatch (%s/%s)", *Name, ClassName );		
	}

	UClass* ResultClass = FindObject<UClass>( InParent, ClassName );
	if( ResultClass && (ResultClass->GetFlags() & RF_Intrinsic) )
	{
		// Gracefully update an existing hardcoded class.
		debugf( NAME_Log, "Updated intrinsic class '%s'", ResultClass->GetFullName() );
		ResultClass->ClassFlags &= ~(CLASS_Parsed | CLASS_Compiled);
	}
	else
	{
		// Create new class.
		ResultClass = new( InParent, ClassName, RF_Public | RF_Standalone )UClass( NULL );

		// Find or forward-declare base class.
		ResultClass->SuperField = FindObject<UClass>( InParent, BaseClassName );
		if( !ResultClass->SuperField )
			ResultClass->SuperField = FindObject<UClass>( ANY_PACKAGE, BaseClassName );
		if( !ResultClass->SuperField )
			ResultClass->SuperField = new( InParent, BaseClassName )UClass( ResultClass );
		debugf( NAME_Log, "Imported: %s", ResultClass->GetFullName() );
	}

	// Set this class's properties.
	ResultClass->DefaultPropText  = DefaultPropText;
	ResultClass->ScriptText       = new( ResultClass->GetParent(), NAME_None, RF_NotForClient | RF_NotForServer )UTextBuffer;
	ResultClass->ScriptText->Text = ScriptText->Text;
	delete ScriptText;

	return ResultClass;
	unguard;
}
IMPLEMENT_CLASS(UClassFactory);

/*------------------------------------------------------------------------------
	ULevelFactory.
------------------------------------------------------------------------------*/

static void ForceValid( ULevel* Level, UStruct* Struct, BYTE* Data )
{
	guard(ForceValid);
	for( TFieldIterator<UProperty> It(Struct); It; ++It )
	{
		for( INT i=0; i<It->ArrayDim; i++ )
		{
			BYTE* Value = Data + It->Offset + i*It->GetElementSize();
			if( It->IsA(UObjectProperty::StaticClass) )
			{
				UObject*& Obj = *(UObject**)Value;
				if( Obj && Obj->IsA(AActor::StaticClass) )
				{
					for( INT j=0; j<Level->Num(); j++ )
						if( Level->Actors(j)==Obj )
							break;
					if( j==Level->Num() )
					{
						debugf( NAME_Log, "Usurped %s", Obj->GetClassName() );
						Obj = NULL;
					}
				}
			}
			else if( It->IsA(UStructProperty::StaticClass) )
			{
				ForceValid( Level, ((UStructProperty*)*It)->Struct, Value );
			}
		}
	}
	unguard;
}

ULevelFactory::ULevelFactory()
{
	guard(ULevelFactory::ULevelFactory);

	// Init UFactory properties.
	SupportedClass = ULevel::StaticClass;
	new(Formats)FString("t3d;Unreal level text");
	bCreateNew = 0;

	unguard;
}
UObject* ULevelFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			Type,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(ULevelFactory::Create);
	TMap<AActor*,FString> Map;

	// Create (or replace) the level object.
	ULevel* Level = GEditor->Level;
	Level->CompactActors();
	check(Level->Num()>1);
	check(Cast<ALevelInfo>(Level->Actors(0)));
	check(Cast<ABrush>(Level->Actors(1)));

	// Init actors.
	for( INT i=0; i<Level->Num(); i++ )
	{
		if( Level->Actors(i) )
		{
			Level->Actors(i)->bTempEditor = 1;
			Level->Actors(i)->bSelected   = 0;
		}
	}

	// Assumes data is being imported over top of a new, valid map.
	ParseNext( &Buffer );
	if( !GetBEGIN( &Buffer, "MAP") )
		return Level;

	// Import everything.
	int ImportedActive=appStricmp(Type,"paste")==0;
	char StrLine[256];
	while( ParseLine( &Buffer, StrLine, 256 ) )
	{
		const char* Str = StrLine;
		if( GetEND(&Str,"MAP") )
		{
			// End of brush polys.
			break;
		}
		else if( GetBEGIN(&Str,"BRUSH") )
		{
			GSystem->StatusUpdatef( 0, 0, "%s", "Importing Brushes" );
			char BrushName[NAME_SIZE];
			if( Parse(Str,"NAME=",BrushName,NAME_SIZE) )
			{
				ABrush* Actor;
				if( !ImportedActive )
				{
					// Parse the active brush, which has already been allocated.
					Actor          = Level->Brush();
					ImportedActive = 1;
				}
				else
				{
					// Parse a new brush which has not yet been allocated.
					Actor             = Level->SpawnBrush();
					Actor->bSelected  = 1;
					Actor->Brush      = new( InParent, NAME_None, RF_NotForClient|RF_NotForServer )UModel( NULL );			
				}

				// Import.
				Actor->SetFlags       ( RF_NotForClient | RF_NotForServer );
				Actor->Brush->SetFlags( RF_NotForClient | RF_NotForServer );
				TObjectIterator<UModelFactory> It;
				check(*It);
				Actor->Brush = (UModel*)It->Create(UModel::StaticClass,InParent,Actor->Brush->GetFName(),Actor,Type,Buffer,BufferEnd);
				check(Actor->Brush);
				if( (Actor->PolyFlags&PF_Portal) && !(Actor->PolyFlags&PF_Translucent) )
					Actor->PolyFlags |= PF_Invisible;
			}
		}
		else if( GetBEGIN(&Str,"ACTOR") )
		{
			UClass* TempClass;
			if( ParseObject<UClass>( Str, "CLASS=", TempClass, ANY_PACKAGE ) )
			{
				// Get actor name.
				FName ActorName(NAME_None);
				Parse( Str, "NAME=", ActorName );

				// Make sure this name is unique.
				AActor* Found=NULL;
				if( ActorName!=NAME_None )
					Found = FindObject<AActor>( InParent, *ActorName );
				if( Found )
					Found->Rename();

				// Import it.
				AActor* Actor = Level->SpawnActor( TempClass, ActorName, NULL, NULL, FVector(0,0,0), FRotator(0,0,0), NULL, 0, 1 );

				// Get property text.
				FString PropText;
				char StrLine[256];
				while
				(	GetEND( &Buffer, "ACTOR" )==0
				&&	ParseLine( &Buffer,  StrLine, 256 ) )
					PropText.Appendf( "%s\r\n", StrLine );
				Map.Add( Actor, PropText );

				// Handle class.
				if( Actor->IsA(ALevelInfo::StaticClass) )
				{
					// Copy the one LevelInfo the position #0.
					check(Level->Num()>0);
					INT iActor=0; Level->FindItem( Actor, iActor );
					Level->Actors(0)       = Actor;
					Level->Actors(iActor)  = NULL;
				}
				else if( Actor->GetClass()==ABrush::StaticClass && !ImportedActive )
				{
					// Copy the active brush the position #0.
					INT iActor=0; Level->FindItem( Actor, iActor );
					Level->Actors(1)       = Actor;
					Level->Actors(iActor)  = NULL;
					ImportedActive = 1;
				}
			}
		}
	}

	// Import actor properties.
	// We do this after creating all actors so that actor references can be matched up.
	guard(Finishing);
	check(Cast<ALevelInfo>(Level->Actors(0)));
	for( int i=0; i<Level->Num(); i++ )
	{
		AActor* Actor = Level->Actors(i);
		if( Actor )
		{
			Actor->bSelected = !Actor->bTempEditor;
			FString PropText;
			if( Map.Find(Actor,PropText) )
			{
				guard(ImportActorProperties);
				ImportProperties( Actor->GetClass(), (BYTE*)Actor, Level, *PropText, InParent );
				unguard;
			}
			guard(FixupActor);
			Actor->Level  = (ALevelInfo*)Level->Actors(0);
			Actor->Region = FPointRegion( (ALevelInfo*)Level->Actors(0) );
			ForceValid( Level, Actor->GetClass(), (BYTE*)Actor );
			unguard;
		}
	}
	unguard;

	return Level;
	unguard;
}
IMPLEMENT_CLASS(ULevelFactory);

/*-----------------------------------------------------------------------------
	UPolysFactory.
-----------------------------------------------------------------------------*/

UPolysFactory::UPolysFactory()
{
	guard(UPolysFactory::UPolysFactory);

	// Init UFactory properties.
	SupportedClass = UPolys::StaticClass;
	new(Formats)FString("t3d;Unreal brush text");
	bCreateNew = 0;

	unguard;
}
UObject* UPolysFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			Type,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(UPolysFactory::Create);

	// Create polys.
	UPolys* Polys = Context ? CastChecked<UPolys>(Context) : new(InParent,Name)UPolys;

	// Eat up if present.
	GetBEGIN( &Buffer, "POLYLIST" );

	// Parse all stuff.
	int First=1, GotBase=0;
	char StrLine[256], ExtraLine[256];
	FPoly Poly;
	while( ParseLine( &Buffer, StrLine, 256 ) )
	{
		const char* Str = StrLine;
		if( GetEND(&Str,"POLYLIST") )
		{
			// End of brush polys.
			break;
		}
		else if( appStrstr(Str,"ENTITIES") && First )
		{
			// Autocad .DXF file.
			debugf(NAME_Log,"Reading Autocad DXF file");
			INT Started=0, NumPts=0, IsFace=0;
			FVector PointPool[4096];
			FPoly NewPoly; NewPoly.Init();

			while
			(	ParseLine( &Buffer, StrLine, 256, 1 )
			&&	ParseLine( &Buffer, ExtraLine, 256, 1 ) )
			{
				// Handle the line.
				Str = ExtraLine;
				INT Code = appAtoi(StrLine);
				//debugf("DXF: %i: %s",Code,ExtraLine);
				if( Code==0 )
				{
					// Finish up current poly.
					if( Started )
					{
						if( NewPoly.NumVertices == 0 )
						{
							// Got a vertex definition.
							NumPts++;
							//debugf("DXF: Added vertex %i",NewPoly.NumVertices);
						}
						else if( NewPoly.NumVertices>=3 && NewPoly.NumVertices<FPoly::MAX_VERTICES )
						{
							// Got a poly definition.
							if( IsFace ) NewPoly.Reverse();
							NewPoly.Base = NewPoly.Vertex[0];
							NewPoly.Finalize(0);
							Polys->AddItem(NewPoly);
							//debugf("DXF: Added poly %i",Num);
						}
						else
						{
							// Bad.
							Warn->Warnf( "DXF: Bad vertex count %i", NewPoly.NumVertices );
						}
						
						// Prepare for next.
						NewPoly.Init();
					}
					Started=0;

					if( ParseCommand(&Str,"VERTEX") )
					{
						// Start of new vertex.
						//debugf("DXF: Vertex");
						PointPool[NumPts] = FVector(0,0,0);
						Started = 1;
						IsFace  = 0;
					}
					else if( ParseCommand(&Str,"3DFACE") )
					{
						// Start of 3d face definition.
						//debugf("DXF: 3DFace");
						Started = 1;
						IsFace  = 1;
					}
					else if( ParseCommand(&Str,"SEQEND") )
					{
						// End of sequence.
						//debugf("DXF: SEQEND");
						NumPts=0;
					}
					else if( ParseCommand(&Str,"EOF") )
					{
						// End of file.
						//debugf("DXF: End");
						break;
					}
				}
				else if( Started )
				{
					// Replace commas with periods to handle european dxf's.
					for( char* Stupid = appStrchr(ExtraLine,','); Stupid; Stupid=appStrchr(Stupid,',') )
						*Stupid = '.';

					// Handle codes.
					if( Code>=10 && Code<=19 )
					{
						// X coordinate.
						if( IsFace && Code-10==NewPoly.NumVertices )
						{
							//debugf("DXF: NewVertex %i",NewPoly.NumVertices);
							NewPoly.Vertex[NewPoly.NumVertices++] = FVector(0,0,0);
						}
						NewPoly.Vertex[Code-10].X = PointPool[NumPts].X = appAtof(ExtraLine);
					}
					else if( Code>=20 && Code<=29 )
					{
						// Y coordinate.
						NewPoly.Vertex[Code-20].Y = PointPool[NumPts].Y = appAtof(ExtraLine);
					}
					else if( Code>=30 && Code<=39 )
					{
						// Z coordinate.
						NewPoly.Vertex[Code-30].Z = PointPool[NumPts].Z = appAtof(ExtraLine);
					}
					else if( Code>=71 && Code<=79 && (Code-71)==NewPoly.NumVertices )
					{
						INT iPoint = Abs(appAtoi(ExtraLine));
						if( iPoint>0 && iPoint<=NumPts )
							NewPoly.Vertex[NewPoly.NumVertices++] = PointPool[iPoint-1];
						else debugf( NAME_Warning, "DXF: Invalid point index %i/%i", iPoint, NumPts );
					}
				}
			}
		}
		else if( appStrstr(Str,"Tri-mesh,") && First )
		{
			// 3DS .ASC file.
			debugf( NAME_Log, "Reading 3D Studio ASC file" );
			FVector PointPool[4096];

			AscReloop:
			int NumVerts = 0, TempNumPolys=0, TempVerts=0;
			while( ParseLine( &Buffer, StrLine, 256 ) )
			{
				Str=StrLine;

				char VertText[256],FaceText[256];
				appSprintf(VertText,"Vertex %i:",NumVerts);
				appSprintf(FaceText,"Face %i:",TempNumPolys);

				if( appStrstr(Str,VertText) )
				{
					PointPool[NumVerts].X = appAtof(appStrstr(Str,"X:")+2);
					PointPool[NumVerts].Y = appAtof(appStrstr(Str,"Y:")+2);
					PointPool[NumVerts].Z = appAtof(appStrstr(Str,"Z:")+2);
					NumVerts++;
					TempVerts++;
				}
				else if( appStrstr(Str,FaceText) )
				{
					Poly.Init();
					Poly.NumVertices=3;
					Poly.Vertex[0] = PointPool[appAtoi(appStrstr(Str,"A:")+2)];
					Poly.Vertex[1] = PointPool[appAtoi(appStrstr(Str,"B:")+2)];
					Poly.Vertex[2] = PointPool[appAtoi(appStrstr(Str,"C:")+2)];
					Poly.Base = Poly.Vertex[0];
					Poly.Finalize(0);
					Polys->AddItem(Poly);
					TempNumPolys++;
				}
				else if( appStrstr(Str,"Tri-mesh,") )
					goto AscReloop;
			}
			debugf( NAME_Log, "Imported %i vertices, %i faces", TempVerts, Polys->Num() );
		}
		else if( GetBEGIN(&Str,"POLYGON") ) // Unreal .t3d file.
		{
			// Init to defaults and get group/item and texture.
			Poly.Init();
			Parse( Str, "LINK=", Poly.iLink );
			Parse( Str, "ITEM=", Poly.ItemName );
			ParseObject<UTexture>( Str, "TEXTURE=", Poly.Texture, ANY_PACKAGE );
			Parse( Str, "FLAGS=", Poly.PolyFlags );
			Poly.PolyFlags &= ~PF_NoImport;
		}
		else if( ParseCommand(&Str,"PAN") )
		{
			Parse( Str, "U=", Poly.PanU );
			Parse( Str, "V=", Poly.PanV );
		}
		else if( ParseCommand(&Str,"ORIGIN") )
		{
			GotBase=1;
			GetFVECTOR( Str, Poly.Base );
		}
		else if( ParseCommand(&Str,"VERTEX") )
		{
			if( Poly.NumVertices < FPoly::MAX_VERTICES )
			{
				GetFVECTOR( Str, Poly.Vertex[Poly.NumVertices] );
				Poly.NumVertices++;
			}
		}
		else if( ParseCommand(&Str,"TEXTUREU") )
		{
			GetFVECTOR( Str, Poly.TextureU );
		}
		else if( ParseCommand(&Str,"TEXTUREV") )
		{
			GetFVECTOR( Str, Poly.TextureV );
		}
		else if( GetEND(&Str,"POLYGON") )
		{
			if( !GotBase )
				Poly.Base = Poly.Vertex[0];
			if( Poly.Finalize(1)==0 )
				Polys->AddItem(Poly);
			GotBase=0;
		}
	}

	// Success.
	return Polys;
	unguard;
}
IMPLEMENT_CLASS(UPolysFactory);

/*-----------------------------------------------------------------------------
	UModelFactory.
-----------------------------------------------------------------------------*/

UModelFactory::UModelFactory()
{
	guard(UModelFactory::UModelFactory);

	// Init UFactory properties.
	SupportedClass = UModel::StaticClass;
	new(Formats)FString("t3d;Unreal model text");
	bCreateNew = 0;

	unguard;
}
UObject* UModelFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			Type,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(UModelFactory::Create);
	ABrush* TempOwner = (ABrush*)Context;
	UModel* Model = new( InParent, Name )UModel( TempOwner, 1 );

	const char* StrPtr;
	char StrLine[256];
	if( TempOwner )
	{
		TempOwner->InitPosRotScale();
		TempOwner->bSelected   = 0;
		TempOwner->bTempEditor = 0;
	}
	while( ParseLine( &Buffer, StrLine, 256 ) )
	{
		StrPtr = StrLine;
		if( GetEND(&StrPtr,"BRUSH") )
		{
			break;
		}
		else if( GetBEGIN (&StrPtr,"POLYLIST") )
		{
			UFactory* PolysFactory=NULL;
			for( TObjectIterator<UFactory> It; It; ++It )
				if( appStricmp(It->GetClassName(),"PolysFactory")==0 )
					PolysFactory = *It;
			check(PolysFactory);
			Model->Polys = (UPolys*)PolysFactory->Create(UPolys::StaticClass,InParent,NAME_None,NULL,Type,Buffer,BufferEnd);
			check(Model->Polys);
		}
		if( TempOwner )
		{
			if      (ParseCommand(&StrPtr,"PREPIVOT"	)) GetFVECTOR 	(StrPtr,TempOwner->PrePivot);
			else if (ParseCommand(&StrPtr,"SCALE"		)) GetFSCALE 	(StrPtr,TempOwner->MainScale);
			else if (ParseCommand(&StrPtr,"POSTSCALE"	)) GetFSCALE 	(StrPtr,TempOwner->PostScale);
			else if (ParseCommand(&StrPtr,"LOCATION"	)) GetFVECTOR	(StrPtr,TempOwner->Location);
			else if (ParseCommand(&StrPtr,"ROTATION"	)) GetFROTATOR  (StrPtr,TempOwner->Rotation,1);
			if( ParseCommand(&StrPtr,"SETTINGS") )
			{
				Parse( StrPtr, "CSG=", TempOwner->CsgOper );
				Parse( StrPtr, "POLYFLAGS=", TempOwner->PolyFlags );
			}
		}
	}
	if( GEditor )
		GEditor->bspValidateBrush( Model, 1, 0 );

	return Model;
	unguard;
}
IMPLEMENT_CLASS(UModelFactory);

/*-----------------------------------------------------------------------------
	USoundFactory.
-----------------------------------------------------------------------------*/

USoundFactory::USoundFactory()
{
	guard(USoundFactory::USoundFactory);

	// Init UFactory properties.
	SupportedClass = USound::StaticClass;
	new(Formats)FString("wav;Wave audio files");
	bCreateNew = 0;

	unguard;
}
UObject* USoundFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			FileType,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(USoundFactory::Create);
	USound* Sound=NULL;
	if( appStricmp(FileType, "WAV")==0 )
	{
		// Wave file.
		USound* Sound = new(InParent,Name)USound;
		Sound->FileType = FName("WAV");
		Sound->Data.Add( BufferEnd-Buffer );
		appMemcpy( &Sound->Data(0), Buffer, Sound->Data.Num() );
		return Sound;
	}
	else if( appStricmp(FileType, "UFX")==0 )//oldver
	{
		// Ancient ufx file containing (hopefully one) Wave file.
		Warn->Warnf( "Invalid old-format sound %s", *Name );
		return NULL;
	}
	else
	{
		// Unrecognized.
		Warn->Warnf( "Unrecognized sound format '%s' in %s", FileType, *Name );
		return NULL;
	}
	unguard;
}
IMPLEMENT_CLASS(USoundFactory);

/*-----------------------------------------------------------------------------
	UMusicFactory.
-----------------------------------------------------------------------------*/

UMusicFactory::UMusicFactory()
{
	guard(UMusicFactory::UMusicFactory);

	// Init UFactory properties.
	SupportedClass = UMusic::StaticClass;
	new(Formats)FString("mod;Amiga modules;s3m;Scream Tracker 3");
	bCreateNew = 0;

	unguard;
}
UObject* UMusicFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			FileType,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(UMusicFactory::Create);

	UMusic* Music = new( InParent, Name )UMusic;
	Music->FileType = FName(FileType);
	Music->Data.Add( BufferEnd - Buffer );
	appMemcpy( &Music->Data(0), Buffer, Music->Data.Num() );
	return Music;

	unguard;
}
IMPLEMENT_CLASS(UMusicFactory);

/*-----------------------------------------------------------------------------
	UTextureFactory.
-----------------------------------------------------------------------------*/

//
// 128-byte header found at the beginning of a ".PCX" file.
//
class FPCXFileHeader
{
public:
	BYTE	Manufacturer;		// Always 10.
	BYTE	Version;			// PCX file version.
	BYTE	Encoding;			// 1=run-length, 0=none.
	BYTE	BitsPerPixel;		// 1,2,4, or 8.
	_WORD	XMin;				// Dimensions of the image.
	_WORD	YMin;				// Dimensions of the image.
	_WORD	XMax;				// Dimensions of the image.
	_WORD	YMax;				// Dimensions of the image.
	_WORD	hdpi;				// Horizontal printer resolution.
	_WORD	vdpi;				// Vertical printer resolution.
	BYTE	OldColorMap[48];	// Old colormap info data.
	BYTE	Reserved1;			// Must be 0.
	BYTE	NumPlanes;			// Number of color planes (1, 3, 4, etc).
	_WORD	BytesPerLine;		// Number of bytes per scanline.
	_WORD	PaletteType;		// How to interpret palette: 1=color, 2=gray.
	_WORD	HScreenSize;		// Horizontal monitor size.
	_WORD	VScreenSize;		// Vertical monitor size.
	BYTE	Reserved2[54];		// Must be 0.
};

// Headers found at the beginning of a ".BMP" file.
#pragma pack(push,1)
struct FBitmapFileHeader
{
    _WORD bfType;
    DWORD bfSize;
    _WORD bfReserved1;
    _WORD bfReserved2;
    DWORD bfOffBits;
};
#pragma pack(pop)

#pragma pack(push,1)
struct FBitmapInfoHeader
{
    DWORD biSize;
    DWORD biWidth;
    DWORD biHeight;
    _WORD biPlanes;
    _WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    DWORD biXPelsPerMeter;
    DWORD biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
};
#pragma pack(pop)

UTextureFactory::UTextureFactory()
{
	guard(UTextureFactory::UTextureFactory);

	// Init UFactory properties.
	SupportedClass = UTexture::StaticClass;
	new(Formats)FString("bmp;Bitmap files;pcx;PC Painbrush files");
	bCreateNew = 0;

	unguard;
}
char* GFile=NULL;
UObject* UTextureFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			Type,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(UTextureFactory::Create);
	UTexture* Texture = NULL;

	const FPCXFileHeader*    PCX   = (FPCXFileHeader *)Buffer;
    const FBitmapInfoHeader* bmhdr = (FBitmapInfoHeader *)(Buffer + sizeof(FBitmapFileHeader));

	// Validate it.
	int Length = BufferEnd - Buffer;
	if( Length < sizeof(FPCXFileHeader) )
	{
		// Doesn't contain valid header.
		Warn->Warnf( "Texture: Invalid header" );
		return NULL;
	}
    else if( Buffer[0]=='B' && Buffer[1]=='M' )
    {
        // This is a .bmp type data stream.
        if( bmhdr->biPlanes!=1 || bmhdr->biBitCount!=8 )
        {
			// Not a 256 color .bmp image.
            Warn->Warnf( "BMP is not 8-bit color! planes=%i bits=%i", bmhdr->biPlanes, bmhdr->biBitCount );
            return NULL;
        }
		if( (bmhdr->biWidth&(bmhdr->biWidth-1)) || (bmhdr->biHeight&(bmhdr->biHeight-1)) )
		{
			Warn->Warnf( "Texture dimensions are not powers of two" );
			return NULL;
		}

        // Set texture properties.
		Texture = CastChecked<UTexture>(GObj.ConstructObject( Class, InParent, Name ) );
		Texture->Init( bmhdr->biWidth, bmhdr->biHeight );
		Texture->PostLoad();

		// Import it.
        Buffer += sizeof(FBitmapFileHeader); // Skip 1st bmp header.
        Buffer += sizeof(FBitmapInfoHeader); // Skip 2nd bmp header.
        Buffer += 4 * 256;                   // Skip palette data.

		// Copy upside-down scanlines.
		for( int y=0; y<(int)bmhdr->biHeight; y++ )
            appMemcpy
			(
                &Texture->Mips(0).DataArray((bmhdr->biHeight - 1 - y) * bmhdr->biWidth),
                &Buffer[y * Align(bmhdr->biWidth,4)],
                bmhdr->biWidth
			);

		// Do palette.
		UPalette* Palette = new( InParent )UPalette;
		Palette->SetFlags( RF_Public );
		FBitmapInfoHeader* bmhdr = (FBitmapInfoHeader *)(Buffer + sizeof(FBitmapFileHeader));
		BYTE*              bmpal = (BYTE *)(Buffer - 4*256);
		Palette->Colors.Empty();
		Palette->Colors.Add( NUM_PAL_COLORS );
		for( int i=0; i<NUM_PAL_COLORS; i++ )
        {
			Palette->Colors(i).B = *bmpal++;
			Palette->Colors(i).G = *bmpal++;
			Palette->Colors(i).R = *bmpal++;
            *bmpal++;
        }
		Texture->Palette = Palette->ReplaceWithExisting();
    }
	else if( PCX->Manufacturer == 10 )
	{
		// This is a .PCX.
		if( PCX->BitsPerPixel!=8 || PCX->NumPlanes!=1 )
		{
			// Bad format, must have 8 bits per pixel, 1 plane.
			Warn->Warnf("PCX: Bad header (%i/%i)", PCX->BitsPerPixel, PCX->NumPlanes );
			return NULL;
		}

		int NewU = PCX->XMax + 1 - PCX->XMin;
		int NewV = PCX->YMax + 1 - PCX->YMin;
		if( (NewU&(NewU-1)) || (NewV&(NewV-1)) )
		{
			Warn->Warnf( "Texture dimensions are not powers of two" );
			return NULL;
		}

		// Set texture properties.
		Texture = CastChecked<UTexture>(GObj.ConstructObject( Class, InParent, Name ) );
		Texture->Init( NewU, NewV );
		Texture->PostLoad();

		// Import it.
		BYTE* DestPtr	= &Texture->Mips(0).DataArray(0);
		BYTE* DestEnd	= DestPtr + Texture->Mips(0).DataArray.Num();
		Buffer += 128;
		while( DestPtr < DestEnd )
		{
			BYTE Color = *Buffer++;
			if( (Color & 0xc0) == 0xc0 )
			{
				INT RunLength = Color & 0x3f;
				Color     = *Buffer++;
				appMemset( DestPtr, Color, Min(RunLength,(int)(DestEnd - DestPtr)) );
				DestPtr  += RunLength;
			}
			else *DestPtr++ = Color;
		}

		// Do the palette.
		UPalette* Palette = new( InParent )UPalette;
		Palette->SetFlags( RF_Public );
		BYTE* PCXPalette = (BYTE *)(BufferEnd - NUM_PAL_COLORS * 3);
		Palette->Colors.Empty();
		Palette->Colors.Add( NUM_PAL_COLORS );
		for( int i=0; i<NUM_PAL_COLORS; i++ )
		{
			Palette->Colors(i).R = *PCXPalette++;
			Palette->Colors(i).G = *PCXPalette++;
			Palette->Colors(i).B = *PCXPalette++;
		}
		Texture->Palette = Palette->ReplaceWithExisting();
	}
	else
	{
		// Unknown format.
        Warn->Warnf( "Bad image format for texture import" );
        return NULL;
 	}

	// See if part of an animation sequence.
	check(Texture);
	if( appStrlen(Texture->GetName())>=4 )
	{
		char Temp[256];
		appStrcpy( Temp, Texture->GetName() );
		char* End = Temp + appStrlen(Temp) - 4;
		if( End[0]=='_' && appToUpper(End[1])=='A' && appIsDigit(End[2]) && appIsDigit(End[3]) )
		{
			int i = appAtoi( End+2 );
			debugf( NAME_Log, "Texture animation frame %i: %s", i, Texture->GetName() );
			if( i>0 )
			{
				appSprintf( End+2, "%02i", i-1 );
				UTexture* Other = FindObject<UTexture>( Texture->GetParent(), Temp );
				if( Other )
				{
					Other->AnimNext = Texture;
					debugf( NAME_Log, "   Linked to previous: %s", Other->GetName() );
				}
			}
			if( i<99 )
			{
				appSprintf( End+2, "%02i", i+1 );
				UTexture* Other = FindObject<UTexture>( Texture->GetParent(), Temp );
				if( Other )
				{
					Texture->AnimNext = Other;
					debugf( NAME_Log, "   Linked to next: %s", Other->GetName() );
				}
			}
		}
	}

#if 0
	// Handle bumps.
	if( 1 )
	{
		FMemMark Mark(GMem);
		INT      U      = Texture->USize;
		INT      V      = Texture->VSize;
		FColor*  Colors = &Texture->Palette->Colors(0);
		BYTE*    Data   = &Texture->Mips(0).DataArray(0);
		FVector* Grad   = new(GMem)FVector[U*V];
		for( INT i=0,ii=V-1; i<V; ii=i++ )
		{
			for( INT j=0,jj=U-1; j<U; jj=j++ )
			{
				FVector A(0,0,Colors[Data[i  * U + j  ]].FBrightness());
				FVector B(1,0,Colors[Data[i  * U + jj ]].FBrightness());
				FVector C(0,1,Colors[Data[ii * U + j  ]].FBrightness());
				Grad[i*U+j] = (((A-B) ^ (C-A)) * FVector(32,32,1)).UnsafeNormal();
			}
		}

		for( i=0,ii=V-1; i<V; ii=i++ )
			for( INT j=0,jj=U-1; j<U; jj=j++ )
				Grad[i*U+j] = (Grad[i*U+j]+Grad[ii*U+j]+Grad[i*U+jj]+Grad[ii*U+jj]).SafeNormal();

		GFile[appStrlen(GFile)-3]='b';
		GFile[appStrlen(GFile)-2]='m';
		GFile[appStrlen(GFile)-1]='p';
		FILE* F = appFopen( GFile, "w+b" );
		if( F )
		{
			FBitmapFileHeader FH;
			FBitmapInfoHeader IH;

			FH.bfType		= 'B' + 256*'M';
			FH.bfSize		= sizeof(FH) + sizeof(IH) + 3 * U * V;
			FH.bfReserved1	= 0;
			FH.bfReserved2	= 0;
			FH.bfOffBits	= sizeof(FH) + sizeof(IH);
			appFwrite( &FH, sizeof(FH), 1, F );

			IH.biSize			= sizeof(IH);
			IH.biWidth			= U;
			IH.biHeight			= V;
			IH.biPlanes			= 1;
			IH.biBitCount		= 24;
			IH.biCompression	= 0; //BI_RGB
			IH.biSizeImage		= U * V * 3;
			IH.biXPelsPerMeter	= 0;
			IH.biYPelsPerMeter	= 0;
			IH.biClrUsed		= 0;
			IH.biClrImportant	= 0;
			appFwrite( &IH, sizeof(IH), 1, F );

			for( INT i=V-1; i>=0; i-- )
			{
				for( INT j=0; j<U; j++ )
				{
					struct {BYTE B,G,R,A;} C;
					C.R      = Clamp( appFloor(128+Grad[i*U+j].Y*127), 0, 255 );
					C.B      = Clamp( appFloor(128+Grad[i*U+j].X*127), 0, 255 );
					C.G      = Clamp( appFloor(128+Grad[i*U+j].Z*127), 0, 255 );
					appFwrite( &C, 3, 1, F );
				}
			}
			appFclose( F );
		}
		Mark.Pop();
	}
#endif

	return Texture;
	unguard;
}
IMPLEMENT_CLASS(UTextureFactory);

/*------------------------------------------------------------------------------
	UFontFactory.
------------------------------------------------------------------------------*/

//
//	Fast pixel-lookup.
//
static inline BYTE AT( BYTE* Screen, int SXL, int X, int Y )
{
	return Screen[X+Y*SXL];
}

//
//	Find the border around a font character that starts at x,y (it's upper
//	left hand corner).  If it finds a character box, it returns 0 and the
//	character's length (xl,yl).  Otherwise returns -1.
//
static int UFont_ScanFontBox( UTexture* Texture, INT X, INT Y, INT& XL, INT& YL )
{
	guard(UFont_ScanFontBox);
	BYTE* Data = &Texture->GetMip(0)->DataArray(0);
	INT FontXL = Texture->USize;

	// Find x-length.
	INT NewXL = 1;
	while ( AT(Data,FontXL,X+NewXL,Y)==255 && AT(Data,FontXL,X+NewXL,Y+1)!=255 )
		NewXL++;
	
	if( AT(Data,FontXL,X+NewXL,Y)!=255 )
		return -1;

	// Find y-length.
	INT NewYL = 1;
	while( AT(Data,FontXL,X,Y+NewYL)==255 && AT(Data,FontXL,X+1,Y+NewYL)!=255 )
		NewYL++;

	if( AT(Data,FontXL,X,Y+NewYL)!=255 )
		return -1;

	XL = NewXL - 1;
	YL = NewYL - 1;

	return 0;
	unguard;
}

UFontFactory::UFontFactory()
{
	guard(UFontFactory::UFontFactory);

	// Use UTextureFactory properties.
	SupportedClass = UFont::StaticClass;

	unguard;
}
UObject* UFontFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			Type,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(UFontFactory::Create);
	UFont* Font = (UFont*)UTextureFactory::Create( Class, InParent, Name, Context, Type, Buffer, BufferEnd, Warn );
	if( Font )
	{
		// Init.
		BYTE* TextureData = &Font->GetMip(0)->DataArray(0);
		Font->Characters.Add( UFont::NUM_FONT_CHARS );
		for( int i=0; i<Font->Characters.Num(); i++ )
		{
			Font->Characters(i).StartU = 0; Font->Characters(i).USize = 0;
			Font->Characters(i).StartV = 0; Font->Characters(i).VSize = 0;
		}

		// Scan in all fonts, starting at character 32.
		i = 32;
		int Y = 0;
		do
		{
			int X = 0;
			while( AT(TextureData,Font->USize,X,Y)!=255 && Y<Font->VSize )
			{
				X++;
				if( X >= Font->USize )
				{
					X = 0;
					if( ++Y >= Font->VSize )
						break;
				}
			}

			// Scan all characters in this row.
			if( Y < Font->VSize )
			{
				int XL=0, YL=0, MaxYL=0;
				while( i<Font->Characters.Num() && UFont_ScanFontBox(Font,X,Y,XL,YL)==0 )
				{
					Font->Characters(i).StartU = X+1;
					Font->Characters(i).StartV = Y+1;
					Font->Characters(i).USize  = XL;
					Font->Characters(i).VSize  = YL;
					X += XL + 1;
					i++;
					if( YL > MaxYL )
						MaxYL = YL;
				}
				Y += MaxYL + 1;
			}
		} while( i<Font->Characters.Num() && Y<Font->VSize );

		// Cleanup font data.
		for( i=0; i<Font->Mips.Num(); i++ )
			for( INT j=0; j<Font->Mips(i).DataArray.Num(); j++ )
				if( Font->Mips(i).DataArray(j)==255 )
					Font->Mips(i).DataArray(j) = 0;
	}
	return Font;
	unguard;
}
IMPLEMENT_CLASS(UFontFactory);

/*------------------------------------------------------------------------------
	The end.
------------------------------------------------------------------------------*/
