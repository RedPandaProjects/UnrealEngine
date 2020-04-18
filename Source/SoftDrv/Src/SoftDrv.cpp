/*=============================================================================
	SoftDrv.cpp: Unreal software rendering driver.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#include "SoftDrvPrivate.h"

/*-----------------------------------------------------------------------------
	Global implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_PACKAGE(SoftDrv);
IMPLEMENT_CLASS(USoftwareRenderDevice);

/*-----------------------------------------------------------------------------
	Stubs.
-----------------------------------------------------------------------------*/

void USoftwareRenderDevice::InternalClassInitializer( UClass* Class )
{
	guardSlow(USoftwareRenderDevice::InternalClassInitializer);
	if( appStricmp( Class->GetName(), "SoftwareRenderDevice" )==0 )
	{
		new(Class,"HighResTextureSmooth",RF_Public)UBoolProperty (CPP_PROPERTY(HighResTextureSmooth), "Options", CPF_Config );
		new(Class,"LowResTextureSmooth", RF_Public)UBoolProperty (CPP_PROPERTY(LowResTextureSmooth ), "Options", CPF_Config );
		new(Class,"FastTranslucency",    RF_Public)UBoolProperty (CPP_PROPERTY(FastTranslucency    ), "Options", CPF_Config );
		new(Class,"DetailBias",			 RF_Public)UFloatProperty(CPP_PROPERTY(DetailBias          ), "Options", CPF_Config );
		//new(Class,"RAM32Mode",		 RF_Public)UBoolProperty (CPP_PROPERTY(RAM32Mode           ), "Options", CPF_Config );
	}
	unguardSlow;
}


UBOOL USoftwareRenderDevice::Init( UViewport* InViewport )
{
	guardSlow(USoftwareRenderDevice::Init);

	// Variables.
	Viewport			= InViewport;
	FrameLocksCounter	= 0;
	SurfPalBuilds = 0;
	SetupFastSqrt();
	
	// Driver flags.
	SpanBased			= 1;
	FrameBuffered		= 1;
	SupportsFogMaps		= GIsMMX;
	SupportsDistanceFog	= 0;

	//InitPowerTables();
	InitDrawSurf();

	/*
	// Log the initialization statistics:
	INT ColorBits = Viewport->ColorBytes * 8;
	if ((ColorBits == 16) && !(Viewport->Caps & CC_RGB565)) ColorBits--;
	debugf("Software rendering initialized, Bit depth: %i  XSize %i YSize %i ",ColorBits,Viewport->SizeX,Viewport->SizeY);
	*/

	return 1;
	unguardSlow;
}

void USoftwareRenderDevice::Exit()
{
	guardSlow(USoftwareRenderDevice::Exit);


	unguardSlow;
}

void USoftwareRenderDevice::Flush()
{
	guardSlow(USoftwareRenderDevice::Flush);
	unguardSlow;
}

UBOOL USoftwareRenderDevice::Exec( const char* Cmd, FOutputDevice* Out )
{
	guardSlow(USoftwareRenderDevice::Exec);
	return 0;
	unguardSlow;
}

// Lock - called at the start of every frame.

void USoftwareRenderDevice::Lock( FPlane InFlashScale, FPlane InFlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* InHitData, INT* InHitSize )
{
	guardSlow(USoftwareRenderDevice::Lock);	
	check(Viewport);
	check(Viewport->ScreenPointer);

	FrameLocksCounter++; // Software frame counter. 

	GByteStride = Viewport->Stride * Viewport->ColorBytes ;


	// CPU and bit depth detection.
	if (GIsMMX)
	{
		if (Viewport->ColorBytes==4) ColorMode = MMX32;
		else ColorMode = (Viewport->Caps & CC_RGB565) ? MMX16 : MMX15;
	}
	else
	{
		if (Viewport->ColorBytes==4) ColorMode = Pentium32;
		else ColorMode = (Viewport->Caps & CC_RGB565) ? Pentium16 : Pentium15;
	}

	FlashScale = InFlashScale * 255.0f;
	FlashFog   = InFlashFog   * 255.0f;

	HitCount   = 0;
	HitData    = InHitData;
	HitSize    = InHitSize;

	guardSlow(Cleanings);

	// Colordepth-specific actions
	if( Viewport->ColorBytes==2 && (Viewport->Caps & CC_RGB565) )
	{
		// various color scalers for DrawPolyC
		GMaxColor.R = (31*8)/256.0f;
		GMaxColor.G = (63*4)/256.0f;
		GMaxColor.B = (31*8)/256.0f;

		// Clear the screen.
		if( RenderLockFlags & LOCKR_ClearScreen )
		{
			_WORD ColorWord = FColor(ScreenClear).HiColor565(), *Dest=(_WORD*)Viewport->ScreenPointer;
			ClearScreenFast16(Dest,ColorWord);
		}
	}
	else if( Viewport->ColorBytes==2 )
	{
		GMaxColor.R = (31*8)/256.0f;
		GMaxColor.G = (31*8)/256.0f;
		GMaxColor.B = (31*8)/256.0f;

		// Clear the screen.
		if( RenderLockFlags & LOCKR_ClearScreen )
		{
			_WORD ColorWord = FColor(ScreenClear).HiColor555(), *Dest=(_WORD*)Viewport->ScreenPointer;
			ClearScreenFast16(Dest,ColorWord);
		}
	}
	else  if( Viewport->ColorBytes==4 )
	{
		GMaxColor.R = 1.0f;
		GMaxColor.G = 1.0f;
		GMaxColor.B = 1.0f;

		// Clear the screen.
		if( RenderLockFlags & LOCKR_ClearScreen )
		{
			DWORD ColorDWord = FColor(ScreenClear).TrueColor(), *Dest=(DWORD*)Viewport->ScreenPointer;
			ClearScreenFast32(Dest,ColorDWord);
		}
	}

	unguardSlow; //cleanings

	// FlashScale 128.0 = no flash.
	GMasterScale = ( 0.5 + Viewport->Client->Brightness ); // global brightness scaler
	FLOAT BrightScale = (1.0f/128.0f)*GMasterScale;
	
	// #debug scale by GMaxColor rather than clip by it...
	GFloatFog.R =  MinPositiveFloat( InFlashFog.R , GMaxColor.R ); 
	GFloatFog.G =  MinPositiveFloat( InFlashFog.G , GMaxColor.G ); 
	GFloatFog.B =  MinPositiveFloat( InFlashFog.B , GMaxColor.B );

	GFloatScale.R = BrightScale * FlashScale.R; 
	GFloatScale.G = BrightScale * FlashScale.G;
	GFloatScale.B = BrightScale * FlashScale.B;

	// Ensure global light and fog factors will always combine without overflow, regardless
	// of bit depth.

	// GFloatRange is maximum color a light can have when only Global fog is taken into account.
	GFloatRange.R = GMaxColor.R - GFloatFog.R;
	GFloatRange.G = GMaxColor.G - GFloatFog.G;
	GFloatRange.B = GMaxColor.B - GFloatFog.B;

	unguardSlow;
}

// Unlock - called at end of frame.

void USoftwareRenderDevice::Unlock( UBOOL Blit )
{
	guardSlow(USoftwareRenderDevice::Unlock);

	check(HitStack.Num()==0);
	if( HitSize )
		*HitSize = HitCount;
	unguardSlow;
}


void USoftwareRenderDevice::GetStats( char* Result )
{
	guardSlow(USoftwareRenderDevice::GetStats);
	appSprintf( Result, "No stats available" );
	unguardSlow;
}



FLOAT FastSqrtTbl[2 << FASTAPPROX_MAN_BITS];

void SetupFastSqrt()
{
	// Setup square root tables.
	for( DWORD D=0; D< (1 << FASTAPPROX_MAN_BITS ); D++ )
	{
		union {FLOAT F; DWORD D;} Temp;
		Temp.F = 1.0;
		Temp.D = (Temp.D & 0xff800000 ) + (D << (23 - FASTAPPROX_MAN_BITS));
		Temp.F = appSqrt(Temp.F);
		Temp.D = (Temp.D - ( 64 << 23 ) );   // exponent bias re-adjust
		FastSqrtTbl[ D ] = (FLOAT)(Temp.F * appSqrt(2.0)); // for odd exponents
		FastSqrtTbl[ D + (1 << FASTAPPROX_MAN_BITS) ] =  (FLOAT) (Temp.F * 2.0);
	}
}




/*-----------------------------------------------------------------------------
	The end.
-----------------------------------------------------------------------------*/
