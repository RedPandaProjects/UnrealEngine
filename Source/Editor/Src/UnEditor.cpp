/*=============================================================================
	UnEditor.cpp: Unreal editor main file
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

EDITOR_API class UEditorEngine* GEditor;

/*-----------------------------------------------------------------------------
	UEditorEngine.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UEditorEngine);

/*-----------------------------------------------------------------------------
	Init & Exit.
-----------------------------------------------------------------------------*/

//
// Construct the editor.
//
UEditorEngine::UEditorEngine()
{}

//
// Init the editor.
//
void UEditorEngine::Init()
{
	guard(UEditorEngine::Init);
	GEditor = this;

	// Call base.
	UEngine::Init();

	// Topics.
	GTopics.Init();

	// Make sure properties match up.
	VERIFY_CLASS_OFFSET(A,Actor,Owner);
	VERIFY_CLASS_OFFSET(A,PlayerPawn,Player);

	// Transaction tracking system.
	Trans = new UTransBuffer
	(
		80,       // Max trans.
		12000,    // Max changes.
		2048*1024 // Undo data.
	);

	// Allocate temporary model.
	TempModel = new UModel( NULL, 1 );

	// Settings.
	Mode			= EM_None;
	MovementSpeed	= 4.0;
	FastRebuild		= 0;
	Bootstrapping	= 0;

	// Create importers.
	Tools.AddItem( new UClassFactory   );
	Tools.AddItem( new ULevelFactory   );
	Tools.AddItem( new UPolysFactory   );
	Tools.AddItem( new UModelFactory   );
	Tools.AddItem( new UMusicFactory   );
	Tools.AddItem( new USoundFactory   );
	Tools.AddItem( new UTextureFactory );
	Tools.AddItem( new UFontFactory    );

	// Load the classes.
	if( ParseParam( appCmdLine(),"MAKE" ) )
	{
		// Load classes for editing.
		for( int i=0; i<32; i++ )
		{
			char Key[256], Pkg[NAME_SIZE];
			appSprintf( Key, "EditPackages[%i]", i );
			if( GetConfigString( "Editor.EditorEngine", Key, Pkg, ARRAY_COUNT(Pkg) ) )
			{
				// Try to load class.
				char Filename[256];
				appSprintf( Filename, "%s.u", Pkg );
				if( !GObj.LoadPackage( NULL, Filename, LOAD_NoWarn|LOAD_KeepImports ) )
				{
					// Create package.
					UPackage* PkgObject = GObj.CreatePackage(NULL,Pkg);

					// Rebuild the class from its directory.
					char Spec[256];
					appSprintf( Spec, "%sSource\\%s\\Classes\\*.uc",appBaseDir(), Pkg );
					TArray<FString> Files = appFindFiles( Spec );
					if( Files.Num() == 0 )
						appErrorf( "Can't find files matching %s", Spec );
					for( INT i=0; i<Files.Num(); i++ )
					{
						// Import class.
						appSprintf( Filename, "%sSource\\%s\\Classes\\%s", appBaseDir(), Pkg, Files(i) );
						char Temp[256];
						appStrcpy( Temp, *Files(i) );
						*appStrchr(Temp,'.') = 0;
						ImportObjectFromFile<UClass>( PkgObject, FName(Temp,FNAME_Add), Filename, GSystem );
					}

					// Verify that all script declared superclasses exist.
					for( TObjectIterator<UClass> ItC; ItC; ++ItC )
						if( ItC->ScriptText && ItC->GetSuperClass() )
							if( !ItC->GetSuperClass()->ScriptText )
								appErrorf( "Superclass %s of class %s not found", ItC->GetSuperClass()->GetName(), ItC->GetName() );

					// Bootstrap-recompile changed scripts.
					Bootstrapping = 1;
					MakeScripts( 0, 1 );
					Bootstrapping = 0;

					// If desired, export C++ header.
					if( ParseParam( appCmdLine(), "H" ) )
					{
						// Tag AActor-derived classes in this package.
						for( INT i=0; i<FName::GetMaxNames(); i++ )
							if( FName::GetEntry(i) )
								FName::GetEntry(i)->Flags &= ~RF_TagExp;
						for( TObjectIterator<UClass> It; It; ++It )
							It->ClearFlags( RF_TagImp | RF_TagExp );
						for( It=TObjectIterator<UClass>(); It; ++It )
							if( It->GetParent()==PkgObject )
								It->SetFlags( RF_TagExp );

						// Export the header.
						appSprintf( Filename, "%sSource\\%s\\Inc\\%sClasses.h", appBaseDir(), Pkg, Pkg );
						debugf( NAME_Log, "Generating C++ header: %s", Filename );
						if( !AActor::StaticClass->ExportToFile( Filename ) )
							appErrorf( "Failed to export: %s", Filename );
					}

					// Save package.
					appSprintf( Filename, "%s%s.u",appScriptDir(), Pkg );
					GObj.SavePackage( PkgObject, NULL, RF_Standalone, Filename );
				}
			}
		}
		GIsRequestingExit=1;
	}
	else
	{
		// Load classes for editing.
		GObj.BeginLoad();
		for( INT i=0; i<ARRAY_COUNT(EditPackages); i++ )
			if( EditPackages[i][0] )
				if( !GObj.LoadPackage( NULL, EditPackages[i], LOAD_NoWarn|LOAD_KeepImports ) )
					appErrorf( "Can't find edit package '%s'", EditPackages[i] );
		GObj.EndLoad();

		// Init the client.
		UClass* ClientClass = GObj.LoadClass( UClient::StaticClass, NULL, "ini:Engine.Engine.ViewportManager", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
		Client = (UClient*)GObj.ConstructObject( ClientClass );
		Client->Init( this );
		check(Client);

		// Checks.
		VERIFY_CLASS_OFFSET(U,EditorEngine,ParentContext);
		if( sizeof(*this) !=GetClass()->GetPropertiesSize() )
			appErrorf( "Editor size mismatch: C++ %i / UnrealScript %i", sizeof(*this), GetClass()->GetPropertiesSize() );
		check(sizeof(*this)==GetClass()->GetPropertiesSize());

		// Init rendering.
		UClass* RenderClass = LoadClass<URenderBase>( NULL, "ini:Engine.Engine.Render", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
		Render = (URenderBase*)GObj.ConstructObject( RenderClass );
		Render->Init( this );

		// Set editor mode.
		edcamSetMode( EM_ViewportMove );

		// Info.
		UPackage* LevelPkg = GObj.CreatePackage(NULL,"MyLevel");
		Level = new( LevelPkg, "MyLevel" )ULevel( this, 0 );

		// Objects.
		Cylinder = new UPrimitive;
		Results  = new( GObj.GetTransientPackage(), "Results" )UTextBuffer;

		// Purge garbage.
		Cleanse( *GSystem, 0, "startup" );

		// Subsystem init messsage.
		debugf( NAME_Init, "Editor engine initialized" );
	}
	unguard;
};
void UEditorEngine::Destroy()
{
	guard(UEditorEngine::Destroy);

	// Shut down transaction tracking system.
	if( Trans )
	{
		if( GUndo )
			debugf( NAME_Warning, "Warning: A transaction is active" );

		// Purge any unused objects held in undo-limbo.
		Trans->Reset ("shutdown");
		delete Trans;
		Trans = NULL;

		debugf( NAME_Exit, "Transaction tracking system closed" );
	}

	// Topics.
	GTopics.Exit();
	Level = NULL;

	// Remove editor array from root.
	debugf( NAME_Exit, "Editor closed" );

	UEngine::Destroy();
	unguard;
}
void UEditorEngine::Serialize( FArchive& Ar )
{
	guard(UEditorEngine::Serialize);
	UEngine::Serialize(Ar);
	Ar << Tools;
	unguard;
}
void UEditorEngine::RedrawLevel( ULevel* Level )
{
	guard(UEditorEngine::RedrawLevel);
	if( Client && !ParentContext )
		for( int i=0; i<Client->Viewports.Num(); i++ )
			if( Client->Viewports(i)->Actor->XLevel == Level )
				Client->Viewports(i)->Repaint();
	unguard;
}
void UEditorEngine::ResetSound()
{
	guard(UEditorEngine::ResetSound);

	if( Audio )
		for( int i=0; i<Client->Viewports.Num(); i++ )
			if( appStricmp(Client->Viewports(i)->GetName(), "Standard3V")==0 )
				Audio->SetViewport( Client->Viewports(i) );

	unguard;
}

/*-----------------------------------------------------------------------------
	Tick.
-----------------------------------------------------------------------------*/

//
// Time passes...
//
void UEditorEngine::Tick( float DeltaSeconds )
{
	guard(UEditorEngine::Tick);

	// Update subsystems.
	GObj.Tick();				
	GCache.Tick();

	// Find active realtime camera.
	UViewport* RealtimeViewport = NULL;
	for( INT i=0; i<Client->Viewports.Num(); i++ )
	{
		UViewport* Viewport = Client->Viewports(i);
		if( Viewport->Current && Viewport->IsRealtime() )
			RealtimeViewport = Viewport;
	}

	// Update the level.
	if( Level )
		Level->Tick( RealtimeViewport ? LEVELTICK_ViewportsOnly : LEVELTICK_TimeOnly, DeltaSeconds );

	// Update audio.
	if( Audio )
	{
		clock(Level->AudioTickCycles);
		UViewport* AudioViewport = FindObject<UViewport>( GObj.GetTransientPackage(), "Standard3V" );
		FCoords C = GMath.ViewCoords;
		FPointRegion Region(NULL);
		if( AudioViewport )
		{
			C = C / AudioViewport->Actor->Rotation  / AudioViewport->Actor->Location;
			Region = AudioViewport->Actor->Region;
		}
		Audio->Update( Region, C );
		unclock(Level->AudioTickCycles);
	}

	// Render everything.
	if( Client )
		Client->Tick();

	unguard;
}

/*-----------------------------------------------------------------------------
	Garbage collection.
-----------------------------------------------------------------------------*/

//
// Clean up after a major event like loading a file.
//
void UEditorEngine::Cleanse( FOutputDevice& Out, UBOOL Redraw, const char* TransReset )
{
	guard(UEditorEngine::Cleanse);
	check(TransReset);

	// Perform an object purge.
	if( GIsRunning && !Bootstrapping )
	{
		GObj.CollectGarbage( &Out, RF_Intrinsic | RF_Standalone );

		// Reset the transaction tracking system if desired.
		Trans->Reset( TransReset );

		// Flush the cache.
		GCache.Flush();

		// Redraw the levels.
		if( Redraw )
			RedrawLevel( Level );
	}
	unguard;
}

/*---------------------------------------------------------------------------------------
	Topics.
---------------------------------------------------------------------------------------*/

void UEditorEngine::Get( const char *Topic, const char *Item, FOutputDevice &Out )
{
	guard(UEditorEngine::Get);
	GTopics.Get( Level, Topic, Item, Out );
	unguard;
}
void UEditorEngine::Set( const char *Topic, const char *Item, const char *Value )
{
	guard(UEditorEngine::Set);
	GTopics.Set( Level, Topic, Item, Value );
	unguard;
}

/*---------------------------------------------------------------------------------------
	Link topics.
---------------------------------------------------------------------------------------*/

// Enum.
AUTOREGISTER_TOPIC("Enum",EnumTopicHandler);
void EnumTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(EnumTopicHandler::Get);

	UEnum* Enum = FindObject<UEnum>( ANY_PACKAGE, Item );
	if( Enum )
	{
		for( int i=0; i<Enum->Names.Num(); i++ )
		{
			if( i > 0 )
				Out.Logf(",");
			Out.Logf( "%i - %s", i, *Enum->Names(i) );
		}
	}
	unguard;
}
void EnumTopicHandler::Set(ULevel *Level, const char *Item, const char *Value)
{}

// Music.
AUTOREGISTER_TOPIC("Music",MusicTopicHandler);
void MusicTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(MusicTopicHandler::Get);
	if( ParseCommand(&Item,"FILETYPE") )
	{
		char Name[NAME_SIZE];
		UPackage* Package=ANY_PACKAGE;
		ParseObject<UPackage>( Item, "PACKAGE=", Package, NULL );
		if( Parse( Item, "NAME=", Name, ARRAY_COUNT(Name) ) )
		{
			UMusic* Music = FindObject<UMusic>( Package, Name );
			if( Music )
				Out.Log( *Music->FileType );
		}
	}
	unguard;
}
void MusicTopicHandler::Set(ULevel *Level, const char *Item, const char *Data)
{}

// Sound.
AUTOREGISTER_TOPIC("Sound",SoundTopicHandler);
void SoundTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(SoundTopicHandler::Get);
	if( ParseCommand(&Item,"FILETYPE") )
	{
		char Name[NAME_SIZE];
		UPackage* Package=ANY_PACKAGE;
		ParseObject<UPackage>( Item, "PACKAGE=", Package, NULL );
		if( Parse( Item, "NAME=", Name, ARRAY_COUNT(Name) ) )
		{
			USound* Sound = FindObject<USound>( Package, Name );
			if( Sound )
				Out.Log( *Sound->FileType );
		}
	}
	unguard;
}
void SoundTopicHandler::Set(ULevel *Level, const char *Item, const char *Data)
{}

// Text.
AUTOREGISTER_TOPIC("Text",TextTopicHandler);
void TextTopicHandler::Get( ULevel *Level, const char *Item, FOutputDevice &Out )
{
	guard(TextTopicHandler::Get);
	UTextBuffer* Text = FindObject<UTextBuffer>( ANY_PACKAGE, Item );
	if( Text && Text->Text.Length() )
		Out.Log( *Text->Text );
	unguard;
}
void TextTopicHandler::Set( ULevel *Level, const char *Item, const char *Data )
{
	guard(TextTopicHandler::Set);
	UTextBuffer* Text = FindObject<UTextBuffer>( ANY_PACKAGE, Item );
	if( Text )
	{
		Text->SetFlags( RF_SourceModified );
		Text->Text.Empty();
		Text->Log( Data );
	}
	unguard;
}

// Script.
AUTOREGISTER_TOPIC("Script",ScriptTopicHandler);
void ScriptTopicHandler::Get( ULevel *Level, const char *Item, FOutputDevice &Out )
{
	guard(ScriptTopicHandler::Get);
	UClass* Class = FindObject<UClass>( ANY_PACKAGE, Item );
	UTextBuffer* Text = Class ? Class->ScriptText : NULL;
	if( Text && Text->Text.Length() )
		Out.Log( *Text->Text );
	unguard;
}
void ScriptTopicHandler::Set( ULevel* Level, const char* Item, const char* Data )
{
	guard(ScriptTopicHandler::Set);
	UClass* Class = FindObject<UClass>( ANY_PACKAGE, Item );
	if( Class && Class->ScriptText )
	{
		if( appStrcmp( *Class->ScriptText->Text, Data ) )
		{
			Class->ScriptText->Text = Data;
			Class->SetFlags( RF_SourceModified );
		}
	}
	unguard;
}

// ScriptPos.
AUTOREGISTER_TOPIC("ScriptPos",ScriptPosTopicHandler);
void ScriptPosTopicHandler::Get( ULevel* Level, const char* Item, FOutputDevice& Out)
{
	guard(ScriptPosTopicHandler::Get);
	UClass* Class = FindObject<UClass>( ANY_PACKAGE, Item );
	UTextBuffer* Text = Class ? Class->ScriptText : NULL;
	if( Text )
		Out.Logf( "%i", Text->Pos );
	unguard;
}
void ScriptPosTopicHandler::Set( ULevel* Level, const char* Item, const char* Data )
{
	guard(ScriptPosTopicHandler::Set);
	UClass* Class = FindObject<UClass>( ANY_PACKAGE, Item );
	UTextBuffer* Text = Class ? Class->ScriptText : NULL;
	if( Text ) Text->Pos = appAtoi(Data);
	unguard;
}


// ScriptTop.
AUTOREGISTER_TOPIC("ScriptTop",ScriptTopTopicHandler);
void ScriptTopTopicHandler::Get( ULevel* Level, const char* Item, FOutputDevice& Out )
{
	guard(ScriptTopTopicHandler::Get);
	UClass* Class = FindObject<UClass>( ANY_PACKAGE, Item );
	UTextBuffer* Text = Class ? Class->ScriptText : NULL;
	if( Text ) Out.Logf("%i",Text->Top);
	unguard;
}
void ScriptTopTopicHandler::Set( ULevel* Level, const char* Item, const char* Data )
{
	guard(ScriptTopTopicHandler::Set);
	UClass* Class = FindObject<UClass>( ANY_PACKAGE, Item );
	UTextBuffer* Text = Class ? Class->ScriptText : NULL;
	if( Text ) Text->Top = appAtoi(Data);
	unguard;
}

// Class.
int CDECL ClassSortCompare( const void *elem1, const void *elem2 )
{
	return appStricmp((*(UClass**)elem1)->GetName(),(*(UClass**)elem2)->GetName());
}
AUTOREGISTER_TOPIC( "Class", ClassTopicHandler );
void ClassTopicHandler::Get( ULevel *Level, const char *Item, FOutputDevice &Out )
{
	guard(ClassTopicHandler::Get);
	enum	{MAX_RESULTS=1024};
	int		NumResults = 0;
	UClass	*Results[MAX_RESULTS];

	if( ParseCommand(&Item,"PACKAGE") )
	{
		UClass* Class = NULL;
		if( ParseObject<UClass>(Item,"CLASS=",Class,ANY_PACKAGE) )
			Out.Log( Class->GetParent()->GetName() );
	}
	else if( ParseCommand(&Item,"QUERY") )
	{
		UClass *Parent = NULL;
		ParseObject<UClass>(Item,"PARENT=",Parent,ANY_PACKAGE);

		// Make a list of all child classes.
		for( TObjectIterator<UClass> It; It && NumResults<MAX_RESULTS; ++It )
			if( It->GetSuperClass()==Parent )
				Results[NumResults++] = *It;

		// Sort them by name.
		appQsort( Results, NumResults, sizeof(UClass*), ClassSortCompare );

		// Return the results.
		for( INT i=0; i<NumResults; i++ )
		{
			// See if this item has children.
			INT Children = 0;
			for( TObjectIterator<UClass> It; It; ++It )
				if( It->GetSuperClass()==Results[i] )
					Children++;

			// Add to result string.
			if( i>0 ) Out.Log(",");
			Out.Logf
			(
				"%s%s|%s",
				(	Results[i]->GetParent()->GetFName()==NAME_Engine
				||	Results[i]->GetParent()->GetFName()==NAME_UnrealI
				||	Results[i]->GetParent()->GetFName()==NAME_Core) ? "*" : "",
				Results[i]->GetName(),
				Children ? "C" : "X"
			);
		}
	}
	if( ParseCommand(&Item,"GETCHILDREN") )
	{
		UClass *Parent = NULL;
		ParseObject<UClass>(Item,"CLASS=",Parent,ANY_PACKAGE);
		UBOOL Concrete=0; ParseUBOOL( Item, "CONCRETE=", Concrete );

		// Make a list of all child classes.
		for( TObjectIterator<UClass> It; It && NumResults<MAX_RESULTS; ++It )
			if( It->IsChildOf(Parent) && (!Concrete || !(It->ClassFlags & CLASS_Abstract)) )
				Results[NumResults++] = *It;

		// Sort them by name.
		appQsort( Results, NumResults, sizeof(UClass*), ClassSortCompare );

		// Return the results.
		for( int i=0; i<NumResults; i++ )
		{
			if( i>0 )
				Out.Log( " " );
			Out.Log( Results[i]->GetName() );
		}
	}
	else if( ParseCommand(&Item,"EXISTS") )
	{
		UClass* Class;
		if (ParseObject<UClass>(Item,"NAME=",Class,ANY_PACKAGE)) Out.Log("1");
		else Out.Log("0");
	}
	else if( ParseCommand(&Item,"PACKAGE") )
	{
		UClass *Class;
		if( ParseObject<UClass>( Item, "CLASS=", Class, ANY_PACKAGE ) )
			Out.Log( Class->GetParent()->GetName() );
	}
	unguard;
}
void ClassTopicHandler::Set(ULevel *Level, const char *Item, const char *Data)
{}

// Actor.
AUTOREGISTER_TOPIC("Actor",ActorTopicHandler);
void ActorTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(ActorTopicHandler::Get);

	// Summarize the level actors.
	int		 n			= 0;
	INT	    AnyClass	= 0;
	UClass*	AllClass	= NULL;
	for( int i=0; i<Level->Num(); i++ )
	{
		if( Level->Actors(i) && Level->Actors(i)->bSelected )
		{
			if( AnyClass && Level->Actors(i)->GetClass()!=AllClass ) 
				AllClass = NULL;
			else 
				AllClass = Level->Actors(i)->GetClass();
			AnyClass=1;
			n++;
		}
	}
	if( !appStricmp(Item,"NumSelected") )
	{
		Out.Logf("%i",n);
	}
	else if( !appStricmp(Item,"ClassSelected") )
	{
		if( AnyClass && AllClass )
			Out.Logf( "%s", AllClass->GetName() );
	}
	else if( !appStrnicmp(Item,"IsKindOf",8) )
	{
		// Sees if the one selected actor belongs to a class.
		UClass *Class;
		Out.Logf( "%i", ParseObject<UClass>(Item,"CLASS=",Class,ANY_PACKAGE) && AllClass && AllClass->IsChildOf(Class) );
	}
	unguard;
}
void ActorTopicHandler::Set( ULevel *Level, const char *Item, const char *Data )
{}

// Lev.
AUTOREGISTER_TOPIC("Lev",LevTopicHandler);
void LevTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(LevTopicHandler::Get);

	INT ItemNum = appAtoi( Item );
	if( ItemNum>=0 && ItemNum<ULevel::NUM_LEVEL_TEXT_BLOCKS && Level->TextBlocks[ItemNum] )
		Out.Log( *Level->TextBlocks[ItemNum]->Text );
	unguard;
}
void LevTopicHandler::Set(ULevel *Level, const char *Item, const char *Data)
{
	guard(LevTopicHandler::Set);

	if( !appIsDigit(Item[0]) )
		return; // Item isn't a number.

	int ItemNum = appAtoi( Item );
	if ((ItemNum < 0) || (ItemNum >= ULevel::NUM_LEVEL_TEXT_BLOCKS)) return; // Invalid text block number

	if( !Level->TextBlocks[ItemNum] )
		Level->TextBlocks[ItemNum] = new( Level->GetParent(), NAME_None, RF_NotForClient|RF_NotForServer )UTextBuffer;
	
	Level->TextBlocks[ItemNum]->Text = Data;

	unguard;
}

// Mesh.
AUTOREGISTER_TOPIC("Mesh",MeshTopicHandler);
void MeshTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(MeshTopicHandler::Get);

	if( !appStrnicmp(Item,"NUMANIMSEQS",11) )
	{
		UMesh *Mesh;
		if( ParseObject<UMesh>(Item,"NAME=",Mesh,ANY_PACKAGE) )
			Out.Logf( "%i",Mesh->AnimSeqs.Num() );
	}
	else if( !appStrnicmp(Item,"ANIMSEQ",7) )
	{
		UMesh *Mesh;
		INT   SeqNum;
		if( ParseObject<UMesh>(Item,"NAME=",Mesh,ANY_PACKAGE) &&
			(Parse(Item,"NUM=",SeqNum)) )
		{
			FMeshAnimSeq &Seq = Mesh->AnimSeqs(SeqNum);
			if( Seq.Name!=NAME_None )
			{
				Out.Logf
				(
					"%s                                        %03i %03i",
					*Seq.Name,
					SeqNum,
					Seq.NumFrames
 				);
			}
		}
	}
	unguard;
}
void MeshTopicHandler::Set(ULevel *Level, const char *Item, const char *Data)
{
	guard(MeshTopicHandler::Set);
	unguard;
}

// Texture.
AUTOREGISTER_TOPIC("Texture",TextureTopicHandler);
void TextureTopicHandler::Get( ULevel *Level, const char *Item, FOutputDevice &Out )
{
	guard(TextureTopicHandler::Get);
	UTexture* Texture;
	if( ParseCommand(&Item,"CURRENTTEXTURE") )
	{
		if( GEditor->CurrentTexture )
			Out.Log( GEditor->CurrentTexture->GetPathName() );
	}
	else if( ParseObject<UTexture>(Item,"TEXTURE=",Texture,ANY_PACKAGE) )
	{
		if( ParseCommand(&Item,"PALETTE") )
		{
			Out.Logf( "%s", Texture->Palette->GetPathName() );
		}
		else if( ParseCommand(&Item,"SIZE") )
		{
			Out.Logf( "%i,%i", Texture->USize, Texture->VSize );
		}
	}
	unguard;
}
void TextureTopicHandler::Set(ULevel *Level, const char *Item, const char *Value)
{}

/*-----------------------------------------------------------------------------
	Object property porting.
-----------------------------------------------------------------------------*/

//
// Import text properties.
//
EDITOR_API const char* ImportProperties
(
	UClass*			ObjectClass,
	BYTE*			Object,
	ULevel*			Level,
	const char*		Data,
	UObject*		InParent
)
{
	guard(ImportProperties);
	check(ObjectClass!=NULL);
	check(Object!=NULL);

	FMemMark Mark(GMem);
	char* PropText	   = new(GMem,65536)char;
	char* Top		   = &PropText[0];
	UBOOL ImportedBrush = 0;

	// Parse all objects stored in the actor.
	// Build list of all text properties.
	*PropText = 0;
	char StrLine[256];
	while( ParseLine( &Data, StrLine, 256 ) )
	{
		const char *Str = &StrLine[0];
		if( GetBEGIN(&Str,"BRUSH") && ObjectClass->IsChildOf(ABrush::StaticClass) )
		{
			// Parse brush on this line.
			guard(Brush);
			char BrushName[NAME_SIZE];
			if( Parse( Str, "NAME=", BrushName, NAME_SIZE ) )
			{
				// If a brush with this name already exists in the
				// level, rename the existing one.  This is necessary
				// because we can't rename the brush we're importing without
				// losing our ability to associate it with the actor properties
				// that reference it.
				UModel* ExistingBrush = FindObject<UModel>( InParent, BrushName );
				if( ExistingBrush )
					ExistingBrush->Rename();

				// Find model factory.!!
				UFactory* ModelFactory=NULL;
				for( TObjectIterator<UFactory> It; It; ++It )
					if( appStricmp( It->GetClassName(), "ModelFactory" )==0 )
						ModelFactory = *It;
				check(ModelFactory);

				// Create model.
				UModel* Brush = (UModel*)ModelFactory->Create( UModel::StaticClass, InParent, BrushName, NULL, "t3d", Data, Data+appStrlen(Data) );
				ImportedBrush = 1;
			}
			unguard;
		}
		else if( GetEND(&Str,"ACTOR") || GetEND(&Str,"DEFAULTPROPERTIES") )
		{
			// End of properties.
			break;
		}
		else
		{
			// More properties.
			appStrcpy( Top, Str );
			Top += appStrlen( Top );
		}
	}

	// Parse all text properties.
	guard(ImportAll);
	for( TFieldIterator<UProperty> It(ObjectClass); It; ++It )
	{
		UProperty* Property = *It;
		if( Property->Port() && appStricmp(Property->GetName(),"Zone")!=0 )//oldver
		{
			char LookFor[80];
			for( INT j=0; j<Property->ArrayDim; j++ )
			{
				BYTE* Value = (BYTE*)Object + Property->Offset + j*Property->GetElementSize();

				if( Property->ArrayDim == 1 )
					appSprintf( LookFor, "%s=", Property->GetName() );
				else
					appSprintf( LookFor, "%s(%i)=", Property->GetName(), j );
				UByteProperty*      ByteProperty      = Cast<UByteProperty>( Property );
				UBoolProperty*      BoolProperty      = Cast<UBoolProperty>( Property );
				UObjectProperty*    ReferenceProperty = Cast<UObjectProperty>( Property );
				UStringProperty*    StringProperty    = Cast<UStringProperty>( Property );
				UStructProperty*    StructProperty    = Cast<UStructProperty>( Property );
				if( ByteProperty )
				{
					if( !Parse( PropText, LookFor, *(BYTE *)Value ) && ByteProperty->Enum )
					{
						// Try to get enum tag.
						char TempTag[NAME_SIZE];
						if( Parse( PropText, LookFor, TempTag, NAME_SIZE ) )
						{
							// Map tag to enum index.
							INT iEnum;
							if( ByteProperty->Enum->Names.FindItem( TempTag, iEnum ) )
								*(BYTE*)Value = iEnum;
						}
					}
				}
				else if( Property->GetClass()==UIntProperty::StaticClass )
				{
					Parse( PropText, LookFor, *(INT *)Value );
				}
				else if( BoolProperty )
				{
					UBOOL Result;
					if( ParseUBOOL( PropText, LookFor, Result ) )
					{
						if (Result) *(DWORD *)Value |=  BoolProperty->BitMask;
						else		*(DWORD *)Value &= ~BoolProperty->BitMask;
					}
				}
				else if( Property->GetClass()==UFloatProperty::StaticClass )
				{
					Parse( PropText, LookFor, *(FLOAT *)Value );
				}
				else if( ReferenceProperty )
				{
					char Temp[256];
					if( Parse( PropText, LookFor, Temp, ARRAY_COUNT(Temp) ) )
						ReferenceProperty->ImportText( Temp, Value, 0 );
				}
				else if( Property->GetClass()==UNameProperty::StaticClass )
				{
					if( !Parse(PropText,LookFor,*(FName*)Value) )
					{
						if( Property->GetFName()==NAME_Tag )//oldver: Update name to tag.
							Parse(PropText,"Name=",*(FName*)Value);
						else if( Property->GetFName()==NAME_InitialState )//oldver: Update State to InitialState
							Parse(PropText,"State=",*(FName*)Value);							
					}
				}
				else if( StringProperty )
				{
					Parse( PropText, LookFor, (char*)Value,StringProperty->StringSize );
				}
				else if( StructProperty )
				{
					if( StructProperty->Struct->GetFName()==NAME_Vector )
					{
						GetFVECTOR( PropText, LookFor, *(FVector*)Value    );
					}
					else if( StructProperty->Struct->GetFName()==NAME_Rotator )
					{
						GetFROTATOR( PropText, LookFor, *(FRotator *)Value, 1 );
					}
					else
					{
						const char* Found = appStrfind( PropText, LookFor );
						if( Found )
							StructProperty->ImportText( Found + appStrlen(LookFor), Value, 0 );
					}
				}
				else appErrorf
				(
					"Bad property type %s in %s of %s (%s)",
					Property->GetClassName(),
					Property->GetName(),
					It.GetStruct()->GetName(),
					ObjectClass->GetName()
				);
			}
		}
	}
	unguard;

	// Upgrade old-version stuff.
	FVector PostPivot;
	if( GetFVECTOR( PropText, "POSTPIVOT=", PostPivot ) && ObjectClass->IsChildOf(AActor::StaticClass) )
		((AActor*)Object)->Location += PostPivot;

	// Prepare brush.
	if( ImportedBrush && ObjectClass->IsChildOf(ABrush::StaticClass) )
	{
		guard(PrepBrush);
		check(GIsEditor);
		ABrush* Actor = (ABrush*)Object;
		if( Actor->bStatic )
		{
			// Prepare static brush.
			Actor->SetFlags       ( RF_NotForClient | RF_NotForServer );
			Actor->Brush->SetFlags( RF_NotForClient | RF_NotForServer );
		}
		else
		{
			// Prepare moving brush.
			GEditor->csgPrepMovingBrush( Actor );
		}
		unguard;
	}
	Mark.Pop();
	return Data;
	unguard;
}

/*---------------------------------------------------------------------------------------
	The End.
---------------------------------------------------------------------------------------*/
