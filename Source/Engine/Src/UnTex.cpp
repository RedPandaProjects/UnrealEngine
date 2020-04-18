/*=============================================================================
	UnTex.cpp: Unreal texture loading/saving/processing functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	UBitmap.
-----------------------------------------------------------------------------*/

UBitmap::UBitmap()
{
	guard(UBitmap::UBitmap);

	Format			= TEXF_P8;
	Palette			= NULL;
	UBits			= 0;
	VBits			= 0;
	USize			= 0;
	VSize			= 0;
	MipZero			= FColor(64,128,64,0);
	MaxColor		= FColor(255,255,255,255);
	LastUpdateTime  = appSeconds();

	unguard;
}
UClient* UBitmap::Client=NULL;
IMPLEMENT_CLASS(UBitmap);

/*-----------------------------------------------------------------------------
	FColor functions.
-----------------------------------------------------------------------------*/

//
// Convert byte hue-saturation-brightness to floating point red-green-blue.
//
FPlane ENGINE_API FGetHSV( BYTE H, BYTE S, BYTE V )
{
	FLOAT Brightness = V * 1.4 / 255.0;
	Brightness *= 0.70/(0.01 + appSqrt(Brightness));
	Brightness  = Clamp(Brightness,(FLOAT)0.0,(FLOAT)1.0);
	FVector Hue = (H<86) ? FVector((85-H)/85.f,(H-0)/85.f,0) : (H<171) ? FVector(0,(170-H)/85.f,(H-85)/85.f) : FVector((H-170)/85.f,0,(255-H)/84.f);
	return FPlane( (Hue + S/255.0 * (FVector(1,1,1) - Hue)) * Brightness, 0 );
}

/*-----------------------------------------------------------------------------
	Texture locking and unlocking.
-----------------------------------------------------------------------------*/

//
// Update texture.
//
void UTexture::Update( DOUBLE CurrentTime )
{
	guard(UTexture::Update);

	if( CurrentTime != LastUpdateTime )
	{
		if( TextureFlags & TF_Realtime )
			TextureFlags |= TF_RealtimeChanged;
		Tick( CurrentTime - LastUpdateTime);
		LastUpdateTime = CurrentTime;
	}

	unguard;
}

//
// Lock a texture for rendering.
//
void UTexture::GetInfo( FTextureInfo& TextureInfo, DOUBLE CurrentTime )
{
	guard(UTexture::GetInfo);

	// Update the texture if time has passed.
	if( CurrentTime != 0.0 )
		Update( CurrentTime );

	// Set locked texture info.
	TextureInfo.TextureFlags	= TextureFlags;
	TextureInfo.Pan				= FVector( 0, 0, 0 );
	TextureInfo.MaxColor		= &MaxColor;
	TextureInfo.Format			= TEXF_P8;
	TextureInfo.UScale          = Scale;
	TextureInfo.VScale          = Scale;
	TextureInfo.CacheID			= MakeCacheID( CID_RenderTexture, this );
	TextureInfo.PaletteCacheID	= MakeCacheID( CID_RenderPalette, Palette );
	TextureInfo.USize			= USize;
	TextureInfo.VSize			= VSize;
	TextureInfo.UClamp			= USize;
	TextureInfo.VClamp			= VSize;
	TextureInfo.NumMips			= GetNumMips();
	TextureInfo.Palette			= GetColors();
	for( INT i=0; i<Mips.Num(); i++ )
	{
		Mips(i).DataPtr     = &Mips(i).DataArray(0);
		TextureInfo.Mips[i] = &Mips(i);
	}

	// Reset the texture flags.
	TextureFlags &= ~TF_RealtimeChanged;

	// Success.
	unguardobj;
}

/*---------------------------------------------------------------------------------------
	Texture animation.
---------------------------------------------------------------------------------------*/

//
// Continuous time update.
//
// To provide your own continuous-time update, override this and do not call this
// base class function in your child class.
//
void UTexture::Tick( FLOAT DeltaSeconds )
{
	guard(UTexture::Tick);

	// Prime the texture.
	while( PrimeCurrent < PrimeCount )
	{
		PrimeCurrent++;
		ConstantTimeTick();
	}

	// Update.
	if( MaxFrameRate == 0.0 )
	{
		// Constant update.
		ConstantTimeTick();
	}
	else
	{
		// Catch up.
		FLOAT MinTime  = 1.f/Clamp(MaxFrameRate,0.01f,100.0f);
		FLOAT MaxTime  = 1.f/Clamp(MinFrameRate,0.01f,100.0f);
		Accumulator   += DeltaSeconds;
		if( Accumulator < MinTime )
		{
			// Skip update.
		}
		else if( Accumulator < MaxTime )
		{
			// Normal update.
			ConstantTimeTick();
			Accumulator = 0;
		}
		else
		{
			ConstantTimeTick();
			Accumulator -= MaxTime;
			if( Accumulator > MaxTime )
				Accumulator = MaxTime;
		}
	}
	unguardobj;
}

//
// Discrete time update.
//
// To provide your own discrete-time update, override this and do not call this
// base class function in your child class.
//
void UTexture::ConstantTimeTick()
{
	guard(UTexture::ConstantTimeTick);

	// Simple cyclic animation.
	if( !AnimCur ) AnimCur = this;
	AnimCur = AnimCur->AnimNext;
	if( !AnimCur ) AnimCur = this;

	unguardobj;
}

/*---------------------------------------------------------------------------------------
	UTexture object implementation.
---------------------------------------------------------------------------------------*/

//
// PCX file header.
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
    _WORD  bfType;          // Always "BM".
    DWORD bfSize;          // Size of file in bytes.
    _WORD  bfReserved1;     // Ignored.
    _WORD  bfReserved2;     // Ignored.
    DWORD bfOffBits;       // Offset of bitmap in file.
};
#pragma pack(pop)

#pragma pack(push,1)
struct FBitmapInfoHeader
{
    DWORD biSize;          // Size of header in bytes.
    DWORD biWidth;         // Width of bitmap in pixels.
    DWORD biHeight;        // Height of bitmap in pixels.
    _WORD  biPlanes;        // Number of bit planes (always 1).
    _WORD  biBitCount;      // Number of bits per pixel.
    DWORD biCompression;   // Type of compression (ingored).
    DWORD biSizeImage;     // Size of pixel array (usually 0).
    DWORD biXPelsPerMeter; // Ignored.
    DWORD biYPelsPerMeter; // Ignored.
    DWORD biClrUsed;       // Number of colors (usually 0).
    DWORD biClrImportant;  // Important colors (usually 0).
};
#pragma pack(pop)

UTexture::UTexture()
{
	guard(UTexture::UTexture);

	BumpMap			= NULL;
	DetailTexture	= NULL;
	MacroTexture	= NULL;
	AnimNext		= NULL;
	AnimCur			= NULL;
	Diffuse			= 1.0;
	Specular		= 1.0;
	Alpha           = 0.0;
	Scale			= 1.0;
	Friction		= 1.0;
	MipMult			= 1.0;
	FootstepSound	= NULL;
	HitSound		= NULL;
	PolyFlags		= 0;
	TextureFlags	= 0;
	PrimeCount		= 0;
	PrimeCurrent	= 0;
	MinFrameRate	= 0;
	MaxFrameRate	= 0;
	Accumulator		= 0;

	unguardobj;
}
void UTexture::Serialize( FArchive& Ar )
{
	guard(UTexture::Serialize);
	UObject::Serialize( Ar );
	if( (Ar.IsSaving() || Ar.IsLoading()) && (TextureFlags & TF_Parametric) )
		for( INT i=0; i<Mips.Num(); i++ )
			Mips(i).DataArray.Empty();
	Ar << Mips;
	if( (Ar.IsSaving() || Ar.IsLoading()) && (TextureFlags & TF_Parametric) )
		for( INT i=0; i<Mips.Num(); i++ )
			Mips(i).DataArray.AddZeroed( Mips(i).USize * Mips(i).VSize );
	if( Ar.Ver() <= 38 )//oldver
	{
		UClamp = USize;
		VClamp = VSize;
	}
	if
	(	Ar.IsLoading()
	&&	!GIsEditor
	&&	Client 
	&&	Client->LowDetailTextures
	&&	Mips.Num()>1 )
	{
		Mips.Remove( 0 );
		Scale *= 2;
		USize = Mips(0).USize;
		VSize = Mips(0).VSize;
		UBits = FLogTwo(USize);
		VBits = FLogTwo(VSize);
	}
	unguard;
}
void UTexture::Export( FOutputDevice& Out, const char* FileType, int Indent )
{
	guard(UTexture::Export);

	// Set all PCX file header properties.
	FPCXFileHeader PCX;
	appMemset( &PCX, 0, sizeof(PCX) );
	PCX.Manufacturer	= 10;
	PCX.Version			= 05;
	PCX.Encoding		= 1;
	PCX.BitsPerPixel	= 8;
	PCX.XMin			= 0;
	PCX.YMin			= 0;
	PCX.XMax			= USize-1;
	PCX.YMax			= VSize-1;
	PCX.hdpi			= USize;
	PCX.vdpi			= VSize;
	PCX.NumPlanes		= 1;
	PCX.BytesPerLine	= USize;
	PCX.PaletteType		= 0;
	PCX.HScreenSize		= 0;
	PCX.VScreenSize		= 0;
	Out.WriteBinary( &PCX, sizeof(PCX) );

	// Special PCX RLE code.
	BYTE RLE=0xc1;

	// Copy all RLE bytes.
	BYTE* ScreenPtr = &Mips(0).DataArray(0);
	for( int i=0; i<USize*VSize; i++ )
	{
		BYTE Color = *ScreenPtr++;
		if( (Color&0xc0)!=0xc0 )
		{
			// No run length required.
			Out.WriteBinary( &Color, 1 );
		}
		else
		{
			// Run length = 1.
			Out.WriteBinary( &RLE, 1 );
			Out.WriteBinary( &Color, 1 );
		}
	}

	// Write PCX trailer then palette.
	BYTE Extra = 12;
	Out.WriteBinary( &Extra, 1 );
	FColor* Colors = GetColors();
	for( i=0; i<NUM_PAL_COLORS; i++ )
	{
		Out.WriteBinary( &Colors[i].R, 1 );
		Out.WriteBinary( &Colors[i].G, 1 );
		Out.WriteBinary( &Colors[i].B, 1 );
	}
	unguardobj;
}
void UTexture::PostLoad()
{
	guard(UTexture::PostLoad);
	UObject::PostLoad();

	// Handle post editing.
	if( !Palette )
	{
		// Make sure the palette is valid.
		Palette = new(GetParent())UPalette;
		for( int i=0; i<256; i++ )
			new(Palette->Colors)FColor(i,i,i,0);
	}
	UClamp = Clamp(UClamp,0,USize);
	VClamp = Clamp(VClamp,0,VSize);

	// Init animation.
	Accumulator = 0;
	LastUpdateTime = appSeconds();

	unguardobj;
}
IMPLEMENT_CLASS(UTexture);

/*---------------------------------------------------------------------------------------
	UTexture mipmap generation.
---------------------------------------------------------------------------------------*/

#define DO_RGB(x) x(R); x(G); x(B); /* Macro for doing the x thing to each RGB component */

//
// Initialize the texture for a given resolution, single mipmap.
//
void UTexture::Init( INT InUSize, INT InVSize )
{
	guard(UTexture::Init);
	check(!(USize & (USize-1)));
	check(!(VSize & (VSize-1)));

	// Allocate space.
	USize = UClamp = InUSize;
	VSize = VClamp = InVSize;
    UBits = FLogTwo(USize);
    VBits = FLogTwo(VSize);

	// Allocate first mipmap.
	Mips.Empty();
	new(Mips)FMipmap(UBits,VBits);
	for( int i=0; i<USize*VSize; i++ )
		Mips(0).DataArray(i) = 0;

	unguardobj;
}


//
// Generate all mipmaps for a texture.  Call this after setting the texture's palette.
// Erik changed: converted to simpler 2x2 box filter with 24-bit color intermediates.
//

void UTexture::CreateMips( UBOOL FullMips, UBOOL Downsample )
{
	guard(UTexture::CreateMips);

	check(Palette!=NULL);
	FColor* Colors = GetColors(); 

	// Create average color.
	FPlane C(0,0,0,0);
	for( int i=0; i<Mips(0).DataArray.Num(); i++ )
		C += Colors[Mips(0).DataArray(i)].Plane();
	MipZero = FColor(C/Mips(0).DataArray.Num());

	// Empty any mipmaps.
	if( Mips.Num() > 1 )
		Mips.Remove( 1, Mips.Num()-1 );

	// Allocate mipmaps.
	if( FullMips )
	{
		while( UBits-Mips.Num()>=0 || VBits-Mips.Num()>=0 )
		{
			INT Num = Mips.Num();
			new(Mips)FMipmap( Max(UBits-Num,0), Max(VBits-Num,0) );
		}
	}

	if( FullMips && Downsample )
	{
		// Build each mip from the next-larger mip.

		FColor  *TrueSource = NULL;
		FColor  *TrueDest   = NULL;		

		for( INT MipLevel=1; MipLevel<Mips.Num(); MipLevel++ )
		{
			FMipmap&  Src        = Mips(MipLevel-1);
			FMipmap&  Dest       = Mips(MipLevel  );
			INT       ThisUTile  = Src.USize;
			INT       ThisVTile  = Src.VSize;

			// Cascade down the mip sequence with truecolor source and destination textures.			
			TrueSource = TrueDest; // Last destination is current source..
			TrueDest = new FColor[Src.USize * Src.VSize];
		
			if( !(PolyFlags & PF_Masked) )
			{
				// Source coordinate masking important for degenerate mipmap sizes.
				DWORD MaskU = (ThisUTile-1);
				DWORD MaskV = (ThisVTile-1);

				INT UD =   1 & MaskU;
				INT VD =  (1 & MaskV)*ThisUTile;

				// Non-masked mipmap.
				for( INT V=0; V<Dest.VSize; V++ )
				{
					for( INT U=0; U<Dest.USize; U++)
					{
						// Get 4 pixels from a one-higher-level mipmap.
						INT TexCoord = U*2 + V*2*ThisUTile;

						FVector C(0,0,0);

						if (TrueSource)
						{	
							C += TrueSource[ TexCoord +  0 +  0 ].Plane();
							C += TrueSource[ TexCoord + UD +  0 ].Plane();
							C += TrueSource[ TexCoord +  0 + VD ].Plane();
							C += TrueSource[ TexCoord + UD + VD ].Plane();
						}
						else
						{
							C += Colors[ Src.DataArray( TexCoord +  0 +  0 ) ].Plane();
							C += Colors[ Src.DataArray( TexCoord + UD +  0 ) ].Plane();
							C += Colors[ Src.DataArray( TexCoord +  0 + VD ) ].Plane(); 
							C += Colors[ Src.DataArray( TexCoord + UD + VD ) ].Plane();
						}

						FColor MipTexel;
						TrueDest[V*Dest.USize+U] = MipTexel = FColor( C/4.0f );
						Dest.DataArray(V*Dest.USize+U) = Palette->BestMatch( MipTexel , RANGE_All );
					}
				}
			}
			else
			// Masked mipmap.
			{

				DWORD MaskU = (ThisUTile-1);
				DWORD MaskV = (ThisVTile-1);

				for( INT V=0; V<Dest.VSize; V++ )
				{
					for( INT U=0; U<Dest.USize; U++) 
					{
						INT F = 0;
						BYTE B;
						FPlane C(0,0,0,0);

						INT TexCoord = V*2*ThisUTile + U*2;

						for (INT I=0;I<2;I++)
						{
							for (INT J=0;J<2;J++)
							{
								B = Src.DataArray(TexCoord + (I&MaskU) + (J&MaskV)*ThisUTile);
								if (B)
								{
									F++;
									if (TrueSource)
										C += TrueSource[TexCoord + (I&MaskU) + (J&MaskV)*ThisUTile].Plane();
									else
										C += Colors[B].Plane();
								}
							}
						}						

						// 1 masked texel or less becomes a non-masked texel.
						if (F >= 2)
						{
							FColor MipTexel = TrueDest[V*Dest.USize+U] = FColor( C / F );
							Dest.DataArray(V*Dest.USize+U) = Palette->BestMatch( MipTexel, RANGE_AllButZero );
						}
						else
						{
							TrueDest[V*Dest.USize+U] = FColor(0,0,0);
							Dest.DataArray(V*Dest.USize+U) = 0;
						}
					}
				}
			}

		if (TrueSource) delete TrueSource; 

		} // Per miplevel.

		if (TrueDest) delete TrueDest;
	}
	unguardobj;
}


/*

void UTexture::CreateOldMips( UBOOL FullMips, UBOOL Downsample )
{
	guard(UTexture::CreateMips);
	check(Palette!=NULL);
	FColor* Colors = GetColors();

	// Tables.
	static const FLOAT BoxC[4][4] =
	{
		{ 4, 7, 4, 0},
		{ 7,21, 7, 0},
		{ 4, 7, 4, 0},
		{ 0, 0, 0, 0}
	};
	const int BoxCSize=3;
	const FLOAT BoxSum=64;

	// Create average color.
	FPlane C(0,0,0,0);
	for( int i=0; i<Mips(0).DataArray.Num(); i++ )
		C += Colors[Mips(0).DataArray(i)].Plane();
	MipZero = FColor(C/Mips(0).DataArray.Num());

	// Empty any mipmaps.
	if( Mips.Num() > 1 )
		Mips.Remove( 1, Mips.Num()-1 );

	// Allocate mipmaps.
	if( FullMips )
	{
		while( UBits-Mips.Num()>=0 || VBits-Mips.Num()>=0 )
		{
			INT Num = Mips.Num();
			new(Mips)FMipmap( Max(UBits-Num,0), Max(VBits-Num,0) );
		}
	}

	if( FullMips && Downsample )
	{
		// Build each mip from the next-larger mip.
		for( INT MipLevel=1; MipLevel<Mips.Num(); MipLevel++ )
		{
			FMipmap&  Src        = Mips(MipLevel-1);
			FMipmap&  Dest       = Mips(MipLevel  );
			INT       ThisUTile  = Src.USize;
			INT       ThisVTile  = Src.VSize;
			DWORD     UAnd	     = Src.USize-1;
			DWORD     VAnd	     = Src.VSize-1;
			if( !(PolyFlags & PF_Masked) )
			{
				// Non-masked mipmap.
				for( INT V=0; V<Dest.VSize; V++ )
				{
					for( INT U=0; U<Dest.USize; U++)
					{
						FVector C(0,0,0);
						for( int X=0; X<BoxCSize; X++ )
							for( int Y=0; Y<BoxCSize; Y++ )
								C += Colors[Src.DataArray(((V*2+Y-1)&VAnd)*ThisUTile + ((U*2+X-1)&UAnd))].Plane() * BoxC[X][Y];
						Dest.DataArray(V*Dest.USize+U) = Palette->BestMatch( FColor(C/BoxSum), RANGE_All );
					}
				}
			}
			else
			{
				// Masked mipmap.
				for( INT V=0; V<Dest.VSize; V++ )
				{
					for( INT U=0; U<Dest.USize; U++) 
					{
						FLOAT F = 0;
						FPlane C(0,0,0,0);
						for( int X=0; X<BoxCSize; X++)
						{
							for( int Y=0; Y<BoxCSize; Y++ )
							{
								BYTE B = Src.DataArray(((V*2+Y-1)&VAnd)*ThisUTile + ((U*2+X-1)&UAnd));
								if( B )
								{
									F += BoxC[X][Y];
									C += BoxC[X][Y] * Colors[B].Plane();
								}
							}
						}
						Dest.DataArray(V*Dest.USize+U) = F>=BoxSum/2 ? Palette->BestMatch(FColor(C/F),RANGE_AllButZero) : 0;
					}
				}
			}
		}
	}
	unguardobj;
}

*/





//
// Set the texture's MaxColor and MinColor so that the texture can be normalized
// when converting to lower color resolutions like RGB 5-5-5 for hardware
// rendering.
//
void UTexture::CreateColorRange()
{
	guard(UTexture::SetMaxColor);
	if( Palette )
	{
		MaxColor = FColor(0,0,0,0);
		FColor* Colors = GetColors();
		for( int i=0; i<Mips.Num(); i++ )
		{
			for( int j=0; j<Mips(i).DataArray.Num(); j++ )
			{
				FColor& Color = Colors[Mips(i).DataArray(j)];
				MaxColor.R    = ::Max(MaxColor.R, Color.R);
				MaxColor.G    = ::Max(MaxColor.G, Color.G);
				MaxColor.B    = ::Max(MaxColor.B, Color.B);
				MaxColor.A    = ::Max(MaxColor.A, Color.A);
			}
		}
	}
	else
	{
		MaxColor = FColor(255,255,255,255);
	}
	unguardobj;
}

/*---------------------------------------------------------------------------------------
	UTexture functions.
---------------------------------------------------------------------------------------*/

//
// Clear the texture.
//
void UTexture::Clear( DWORD ClearFlags )
{
	guard(UTexture::Clear);
	if( ClearFlags & TCLEAR_Bitmap )
		for( int i=0; i<Mips.Num(); i++ )	
			for( int j=0; j<Mips(i).DataArray.Num(); j++ )
				Mips(i).DataArray(j)=0;
	unguardobj;
}

/*---------------------------------------------------------------------------------------
	UPalette implementation.
---------------------------------------------------------------------------------------*/

void UPalette::Serialize( FArchive& Ar )
{
	guard(UPalette::Serialize);
	UObject::Serialize( Ar );
	Ar << Colors;
	unguard;
}
IMPLEMENT_CLASS(UPalette);

/*-----------------------------------------------------------------------------
	UPalette general functions.
-----------------------------------------------------------------------------*/

//
// Flush all caches associated with the palette.
//
void UPalette::Flush()
{
	guard(UPalette::Flush);

	GCache.Flush( MakeCacheID(CID_LightingTable,GetIndex(),0,NULL), MakeCacheID(CID_MAX,~0,0,0) );
	GCache.Flush( MakeCacheID(CID_ColorDepthPalette,0,0,this), MakeCacheID(CID_MAX,0,0,(UObject*)~0) );

	unguard;
}

//
// Adjust a regular (imported) palette.
//
void UPalette::FixPalette()
{
	guard(UPalette::FixPalette);

	FColor TempColors[256];
	for( int i=0; i<256; i++ )
		TempColors[i] = Colors(0);

	for( int iColor=0; iColor<8; iColor++ )
	{
		int iStart = (iColor==0) ? 1 : 32*iColor;
		for( int iShade=0; iShade<28; iShade++ )
			TempColors[16 + iColor + (iShade<<3)] = Colors(iStart + iShade);

	}
	for( i=0; i<256; i++ )
	{
		Colors(i) = TempColors[i];
		Colors(i).A = i+0x10;
	}
	Colors(0).A=0;

	unguardobj;
}

//
// Find closest palette color matching a given RGB value.
//
BYTE UPalette::BestMatch( FColor Color, EBestMatchRange Range )
{
	guard(UPalette::BestMatch);

	INT First,Last;
	if( Range == RANGE_AllButZero )
	{
		First = 1;
		Last  = 256;
	}
	else if( Range == RANGE_AllButWindows )
	{
		First = 10;
		Last  = 246;
	}
	else
	{
		First = 0;
		Last  = 256;
	}

	int BestDelta = MAXINT;
	int BestUnscaledDelta = MAXINT;

	int BestColor = First;

	int TexRed   = Color.R;
	int TexBlue  = Color.B;
	int TexGreen = Color.G;

	for( int TestColor=First; TestColor<Last; TestColor++ )
	{
		FColor* ColorPtr = &Colors(TestColor);

		INT GreenDelta = Square(ColorPtr->G - TexGreen);

		// By comparing unscaled green, saves about 4 out of every 5 full comparisons.
		if (GreenDelta < BestUnscaledDelta) // Same as comparing 8*GreenDelta with BestDelta.
		{
			INT Delta = 
			(
				8 * GreenDelta                     +
				4 * Square(ColorPtr->R - TexRed  ) +
				1 * Square(ColorPtr->B - TexBlue )
			);

			if( Delta < BestDelta ) 
			{
				BestColor = TestColor;
				BestDelta = Delta;
				BestUnscaledDelta = (Delta + 7) >> 3; 
			}
		}
	}

	/*
	// The straight algorithm:
	for( int TestColor=First; TestColor<Last; TestColor++ )
	{
		FColor* ColorPtr = &Colors(TestColor);

		int Delta =
		(
			3*3*Square((int)ColorPtr->G - (int)Color.G) +
			2*2*Square((int)ColorPtr->R - (int)Color.R) +
			1*1*Square((int)ColorPtr->B - (int)Color.B)
		);
		if( Delta < BestDelta )
		{
			BestColor = TestColor;
			BestDelta = Delta;
		}
	}
	*/

	return BestColor;
	unguardobj;
}

//
// Smooth out a ramp palette by averaging adjacent colors.
//
void UPalette::Smooth()
{
	guard(UPalette::Smooth);

	FColor *C1=&Colors(0), *C2=&Colors(1);

	for( int i=1; i<NUM_PAL_COLORS; i++ )
	{
		C2->R = ((int)C1->R + (int)C2->R)>>1;
		C2->G = ((int)C1->G + (int)C2->G)>>1;
		C2->B = ((int)C1->B + (int)C2->B)>>1;
		C1++; C2++;
	}
	unguardobj;
}

//
// Sees if this palette is a duplicate of an existing palette.
// If it is, deletes this palette and returns the existing one.
// If not, returns this palette.
//
UPalette* UPalette::ReplaceWithExisting()
{
	guard(UPalette::ReplaceWithExisting);
	for( TObjectIterator<UPalette>It; It; ++It )
	{
		if( *It!=this && It->GetParent()==GetParent() )
		{
			FColor* C1 = &Colors(0);
			FColor* C2 = &It->Colors(0);
			for( int i=0; i<NUM_PAL_COLORS; i++ )
				if( *C1++ != *C2++ ) break;
			if( i == NUM_PAL_COLORS )
			{
				debugf( NAME_Log, "Replaced palette %s with %s", GetName(), It->GetName() );
				delete this;
				return *It;
			}
		}
	}

	return this;
	unguardobj;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
