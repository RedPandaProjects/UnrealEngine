/*=============================================================================
	UnViewport.cpp: Generic Unreal viewport code
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	URenderDevice.
-----------------------------------------------------------------------------*/

void URenderDevice::InternalClassInitializer( UClass* Class )
{
	guard(URenderDevice::InternalClassInitializer);
	if( appStricmp( Class->GetName(), "RenderDevice" )==0 )
	{
		new(Class,"VolumetricLighting",RF_Public)UBoolProperty(CPP_PROPERTY(VolumetricLighting), "Client", CPF_Config );
		new(Class,"ShinySurfaces",     RF_Public)UBoolProperty(CPP_PROPERTY(ShinySurfaces     ), "Client", CPF_Config );
		new(Class,"Coronas",           RF_Public)UBoolProperty(CPP_PROPERTY(Coronas           ), "Client", CPF_Config );
		new(Class,"HighDetailActors",  RF_Public)UBoolProperty(CPP_PROPERTY(HighDetailActors  ), "Client", CPF_Config );
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	UViewport object implementation.
-----------------------------------------------------------------------------*/

void UViewport::Serialize( FArchive& Ar )
{
	guard(UViewport::Serialize);
	UPlayer::Serialize(Ar);
	Ar << Console << MiscRes << Canvas << RenDev << Input;
	unguard;
}
IMPLEMENT_CLASS(UViewport);

/*-----------------------------------------------------------------------------
	Dragging.
-----------------------------------------------------------------------------*/

//
// Set mouse dragging.
// The mouse is dragging when and only when one or more button is held down.
//
UBOOL UViewport::SetDrag( UBOOL NewDrag )
{
	guard(UViewport::SetDrag);
	UBOOL Result = Dragging;
	Dragging = NewDrag;
	if( Dragging && !Result )
	{
		// First hit.
		Client->Engine->MouseDelta( this, MOUSE_FirstHit, 0, 0 );
	}
	else if( Result && !Dragging )
	{
		// Last release.
		Client->Engine->MouseDelta( this, MOUSE_LastRelease, 0, 0 );
	}
	return Result;
	unguard;
}

/*-----------------------------------------------------------------------------
	Scene frames.
-----------------------------------------------------------------------------*/

//
// Precompute rendering info.
//
void FSceneNode::ComputeRenderCoords( FVector& Location, FRotator& Rotation )
{
	guard(UViewport::ComputeRenderCoords);
	check(Viewport->Actor);
	Coords =
	(	Viewport->Actor->RendMap==REN_OrthXY ? FCoords(FVector(0,0,0),FVector(+1,0,0),FVector(0,+1,0),FVector(0,0,+1))
	:	Viewport->Actor->RendMap==REN_OrthXZ ? FCoords(FVector(0,0,0),FVector(+1,0,0),FVector(0,0,-1),FVector(0,+1,0))
	:	Viewport->Actor->RendMap==REN_OrthYZ ? FCoords(FVector(0,0,0),FVector(0,+1,0),FVector(0,0,-1),FVector(+1,0,0))
	:								           GMath.ViewCoords / Rotation) / Location;
	Uncoords = Coords.Transpose();
	ComputeRenderSize();
	unguard;
}

//
// Precompute sizing info.
//
void FSceneNode::ComputeRenderSize()
{
	guard(FSceneNode::ComputeRenderSize);

	// Precomputes.
	INT Angle	= (0.5 * 65536.0 / 360.0) * Viewport->Actor->FovAngle;
	FX 			= (FLOAT)X;
	FY 			= (FLOAT)Y;
	FX1 		= 65536.0 * (FX + 1);
	FY1 		= FY + 1;
	FX2			= FX * 0.5;
	FY2			= FY * 0.5;	
	FX15		= (FX+1.0001) * 0.5;
	FY15		= (FY+1.0001) * 0.5;	
	Proj		= FVector( 0.5-FX2, 0.5-FY2, FX / (2.0*GMath.SinTab(Angle) / GMath.CosTab(Angle)) );
	RProj		= FVector( 1/Proj.X, 1/Proj.Y, 1/Proj.Z );
	Zoom 		= Viewport->Actor->OrthoZoom / (FX * 15.0);
	RZoom		= 1.0/Zoom;
	PrjXM		= (0  - FX2)*(-RProj.Z);
	PrjXP		= (FX - FX2)*(+RProj.Z);
	PrjYM		= (0  - FY2)*(-RProj.Z);
	PrjYP		= (FY - FY2)*(+RProj.Z);

	// Precompute side info.
	FLOAT TempSigns[2]={-1.0,+1.0};
	for( int i=0; i<2; i++ )
	{
		for( int j=0; j<2; j++ )
		{
			ViewSides[i*2+j] = FVector(TempSigns[i] * FX2, TempSigns[j] * FY2, Proj.Z).UnsafeNormal().TransformVectorBy(Uncoords);
		}
		ViewPlanes[i] = FPlane
		(
			Coords.Origin,
			FVector(0,TempSigns[i] / FY2,1.0/Proj.Z).UnsafeNormal().TransformVectorBy(Uncoords)
		);
		ViewPlanes[i+2] = FPlane
		(
			Coords.Origin,
			FVector(TempSigns[i] / FX2,0,1.0/Proj.Z).UnsafeNormal().TransformVectorBy(Uncoords)
		);
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Custom viewport creation and destruction.
-----------------------------------------------------------------------------*/

//
// UViewport constructor.  Creates the viewport object but doesn't open its window.
//
UViewport::UViewport( ULevel* InLevel, UClient* InClient )
:	UPlayer( InLevel   )
,	Client ( InClient  )
{
	guard(UViewport::UViewport);
	check(InLevel);
	check(InClient);

	// Update viewport array.
	Client->Viewports.AddItem( this );

	// Create canvas.
	UClass* CanvasClass = GObj.LoadClass( UCanvas::StaticClass, NULL, "ini:Engine.Engine.Canvas", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
	Canvas = CastChecked<UCanvas>(GObj.ConstructObject( CanvasClass ));
	Canvas->Init( this );

	// Create input system.
	UClass* InputClass = GObj.LoadClass( UInput::StaticClass, NULL, "ini:Engine.Engine.Input", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
	Input = (UInput*)GObj.ConstructObject( InputClass );

	// Find an available viewport actor or spawn a new one.
	if( GIsEditor )
		InLevel->SpawnViewActor( this );

	// Create console.
	if( !GIsEditor )
	{
		UClass* ConsoleClass = GObj.LoadClass( UConsole::StaticClass, NULL, "ini:Engine.Engine.Console", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
		Console = ConstructClassObject<UConsole>( ConsoleClass );
		Console->_Init( this );
	}

	// Set initial time.
	LastUpdateTime = appSeconds();

	unguard;
}

//
// Close a viewport.
//warning: Lots of interrelated stuff is happening here between the viewport code,
// object code, platform-specific viewport manager code, and Windows.
//
void UViewport::Destroy()
{
	guard(UViewport::Destroy);

	// Temporary for editor!!
	if( appStricmp(GetName(),"Standard3V")==0 && Client->Engine->Audio )
		Client->Engine->Audio->SetViewport( NULL );

	// Close the viewport window.
	guard(CloseWindow);
	CloseWindow();
	unguard;

	// Delete the input subsystem.
	guard(ExitInput);
	delete Input;
	unguard;

	// Delete the console.
	guard(DeleteConsole);
	if( Console )
		delete Console;
	unguard;

	// Delete the canvas.
	guard(DeleteCanvas);
	delete Canvas;
	unguard;

	// Shut down rendering.
	guard(DeleteRenDev);
	if( RenDev )
	{
		RenDev->Exit();
		delete RenDev;
	}
	unguard;

	// Remove from viewport list.
	Client->Viewports.RemoveItem( this );

	UPlayer::Destroy();
	unguardobj;
}

/*---------------------------------------------------------------------------------------
	Viewport information functions.
---------------------------------------------------------------------------------------*/

//
// Is this camrea a wireframe view?
//
UBOOL UViewport::IsWire()
{
	guard(UViewport::IsWire);
	return
	Actor &&
	(	Actor->XLevel->Model->Nodes->Num()==0
	||	Actor->RendMap==REN_OrthXY
	||	Actor->RendMap==REN_OrthXZ
	||	Actor->RendMap==REN_OrthYZ
	||	Actor->RendMap==REN_Wire );

	unguard;
}

/*-----------------------------------------------------------------------------
	Viewport locking & unlocking.
-----------------------------------------------------------------------------*/

//
// Lock the viewport for rendering.
//
UBOOL UViewport::Lock( FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData, INT* HitSize )
{
	guard(UViewport::Lock);
	check(RenDev);

	// Set info.
	CurrentTime = appSeconds();
	HitTesting = HitData!=NULL;
	RenderFlags = RenderLockFlags;
	ExtraPolyFlags = ((Actor->RendMap==REN_PolyCuts) || (Actor->RendMap==REN_Zones)) ? PF_NoMerge : 0;
	FrameCount++;

	// Lock rendering device.
	RenDev->Lock( FlashScale, FlashFog, ScreenClear, RenderLockFlags, HitData, HitSize );

	// Successfully locked it.
	return 1;
	unguard;
}

//
// Unlock the viewport.
//
void UViewport::Unlock( UBOOL Blit )
{
	guard(UViewport::Unlock);
	check(Actor);
	check(RenDev);
	check(HitSizes.Num()==0);

	// Unlock rendering device.
	RenDev->Unlock( Blit );

	// Update time.
	if( Blit )
		LastUpdateTime = CurrentTime;

	unguard;
}

/*-----------------------------------------------------------------------------
	Holding and unholding.
-----------------------------------------------------------------------------*/

//
// Put the viewport on hold, preventing it from being resized.
//
void UViewport::Hold()
{
	guard(UViewport::Hold);
	OnHold=1;
	unguard;
}

//
// Unhold the viewport, and perform any latent resizing.
//
void UViewport::Unhold()
{
	guard(UViewport::Unhold);
	if( OnHold )
	{
		OnHold = 0;
		UpdateWindow();
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

//
// UViewport command line.
//
UBOOL UViewport::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(UViewport::Exec);
	check(Actor);
	if( Input && Input->Exec(Cmd,Out) )
	{
		return 1;
	}
	else if( RenDev && RenDev->Exec(Cmd,Out) )
	{
		return 1;
	}
	else if( ParseCommand(&Cmd,"SHOWALL") )
	{
		for( INT i=0; i<Actor->XLevel->Num(); i++ )
			if( Actor->XLevel->Element(i) )
				Actor->XLevel->Element(i)->bHidden = 0;
		return 1;
	}
	else if( ParseCommand(&Cmd,"REPORT") )
	{
		FStringOut Str;
		FString UrlStr;
		Actor->XLevel->URL.String(UrlStr);
		Str.Log( "Report:\r\n" );
		Str.Logf( "   Version: " __DATE__ " " __TIME__ "\r\n" );
		Str.Logf( "   Player class: %s\r\n", Actor->GetClassName() );
		Str.Logf( "   URL: %s\r\n", *UrlStr );
		Str.Logf( "   Location: %i %i %i\r\n", (INT)Actor->Location.X, (INT)Actor->Location.Y, (INT)Actor->Location.Z );
		if( Actor->Level->Game==NULL )
		{
			Str.Logf( "   Network client\r\n" );
		}
		else
		{
			Str.Logf( "   Game class: %s\r\n", Actor->Level->Game->GetClassName() );
			Str.Logf( "   Difficulty: %i\r\n", Actor->Level->Game->Difficulty );
		}
		ClipboardCopy( *Str );
		return 1;
	}
	else if( ParseCommand(&Cmd,"SHOT") )
	{
		// Screenshot.
		char File[32];
		for( INT i=0; i<256; i++ )
		{
			appSprintf( File, "Shot%04i.bmp", i );
			if( appFSize(File) < 0 )
				break;
		}
		if( appFSize(File)<0 )
		{
			FMemMark Mark(GMem);
			FColor* Buf = new(GMem,SizeX*SizeY)FColor;
			RenDev->ReadPixels( Buf );
			FILE* F = appFopen( File, "w+b" );
			if( F )
			{
				// Types.
				#pragma pack (push,1)
				struct BITMAPFILEHEADER
				{
					_WORD   bfType; 
					DWORD   bfSize;
					_WORD   bfReserved1; 
					_WORD   bfReserved2;
					DWORD   bfOffBits;
				} FH; 
				struct BITMAPINFOHEADER
				{
					DWORD  biSize; 
					INT    biWidth;
					INT    biHeight;
					_WORD  biPlanes;
					_WORD  biBitCount;
					DWORD  biCompression;
					DWORD  biSizeImage;
					INT    biXPelsPerMeter; 
					INT    biYPelsPerMeter;
					DWORD  biClrUsed;
					DWORD  biClrImportant; 
				} IH;
				#pragma pack (pop)

				// File header.
				FH.bfType		= 'B' + 256*'M';
				FH.bfSize		= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3 * SizeX * SizeY;
				FH.bfReserved1	= 0;
				FH.bfReserved2	= 0;
				FH.bfOffBits	= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
				appFwrite( &FH, sizeof(FH), 1, F );

				// Info header.
				IH.biSize			= sizeof(BITMAPINFOHEADER);
				IH.biWidth			= SizeX;
				IH.biHeight			= SizeY;
				IH.biPlanes			= 1;
				IH.biBitCount		= 24;
				IH.biCompression	= 0; //BI_RGB
				IH.biSizeImage		= SizeX * SizeY * 3;
				IH.biXPelsPerMeter	= 0;
				IH.biYPelsPerMeter	= 0;
				IH.biClrUsed		= 0;
				IH.biClrImportant	= 0;
				appFwrite( &IH, sizeof(IH), 1, F );

				// Colors.
				for( INT i=SizeY-1; i>=0; i-- )
					for( INT j=0; j<SizeX; j++ )
						appFwrite( &Buf[i*SizeX+j], 3, 1, F );

				// Success.
				appFclose( F );
			}
			Mark.Pop();
		}
		return 1;
	}
	else if( ParseCommand(&Cmd,"SHOWACTORS") )
	{
		Actor->ShowFlags |= SHOW_Actors;
		return 1;
	}
	else if( ParseCommand(&Cmd,"HIDEACTORS") )
	{
		Actor->ShowFlags &= ~SHOW_Actors;
		return 1;
	}
	else if( ParseCommand(&Cmd,"RMODE") )
	{
		INT Mode = appAtoi(Cmd);
		if( Mode>REN_None && Mode<REN_MAX )
			Actor->RendMap = Mode;
		return 1;
	}
	else if( ParseCommand(&Cmd,"EXEC") )
	{
		char Filename[64];
		if( ParseToken( Cmd, Filename, ARRAY_COUNT(Filename), 0 ) )
			ExecMacro( Filename, Out );
		return 1;
	}
	else if( Actor->XLevel->Exec(Cmd,Out) )
	{
		return 1;
	}
	else if( Client->Engine->Exec(Cmd,Out) )
	{
		return 1;
	}
	else if( Console && Console->ScriptConsoleExec(Cmd,Out) )
	{
		return 1;
	}
	else if( Actor->ScriptConsoleExec(Cmd,Out) )
	{
		return 1;
	}
	else return 0;
	unguard;
}

//
// Execute a macro on this viewport.
//
void UViewport::ExecMacro( const char* Filename, FOutputDevice* Out )
{
	guard(UViewport::ExecMacro);

	// Create text buffer and prevent garbage collection.
	UTextBuffer* Text = ImportObjectFromFile<UTextBuffer>( GObj.GetTransientPackage(), NAME_None, Filename, GSystem );
	GObj.AddToRoot( Text );
	if( Text )
	{
		debugf( "Execing %s", Filename );
		char Temp[256];
		const char* Data = *Text->Text;
		while( ParseLine( &Data, Temp, ARRAY_COUNT(Temp) ) )
			Exec( Temp, Out );
		GObj.RemoveFromRoot( Text );
		delete Text;
	}
	else Out->Logf( NAME_ExecWarning, LocalizeError("FileNotFound","Core"), Filename );

	unguard;
}

/*-----------------------------------------------------------------------------
	UViewport FOutputDevice interface.
-----------------------------------------------------------------------------*/

//
// Output a message on the viewport's console.
//
void UViewport::WriteBinary( const void* Data, INT Length, EName MsgType )
{
	guard(UViewport::WriteBinary);

	// Pass to console.
	if( Console )
		Console->WriteBinary( Data, Length, MsgType );

	unguard;
}

/*-----------------------------------------------------------------------------
	Input.
-----------------------------------------------------------------------------*/

//
// Read input from the viewport.
//
void UViewport::ReadInput( FLOAT DeltaSeconds )
{
	guard(UViewport::ReadInput);
	check(Input);

	// Update input.
	UpdateInput( 0 );

	// Get input from input system.
	Input->ReadInput( DeltaSeconds, GSystem );

	unguard;
}

/*-----------------------------------------------------------------------------
	Viewport hit testing.
-----------------------------------------------------------------------------*/

//
// Viewport hit-test pushing.
//
void UViewport::PushHit( const HHitProxy& Hit, INT Size )
{
	guard(UViewport::PushHit);

	Hit.Size = Size;
	HitSizes.AddItem(Size);
	RenDev->PushHit( (BYTE*)&Hit, Size );

	unguard;
}

//
// Pop the most recently pushed hit.
//
void UViewport::PopHit( UBOOL bForce )
{
	guard(UViewport::PopHit);
	debug(RenDev);
	debug(HitTesting);
	check(HitSizes.Num());

	RenDev->PopHit( HitSizes(HitSizes.Num()-1), bForce );
	HitSizes.Remove(HitSizes.Num()-1);

	unguard;
}

//
// Execute all hits in the hit buffer.
//
void UViewport::ExecuteHits( const FHitCause& Cause, BYTE* HitData, INT HitSize )
{
	guard(UViewport::ExecuteHits);

	// String together the hit stack.
	HHitProxy* TopHit=NULL;
	while( HitSize>0 )
	{
		HHitProxy* ThisHit = (HHitProxy*)HitData;
		HitData           += ThisHit->Size;
		HitSize           -= ThisHit->Size;
		ThisHit->Parent    = TopHit;
		TopHit             = ThisHit;
	}
	check(HitSize==0);

	// Call the innermost hit.
	if( TopHit )
		TopHit->Click( Cause );

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
