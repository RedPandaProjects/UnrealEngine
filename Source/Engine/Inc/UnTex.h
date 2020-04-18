/*=============================================================================
	UnTex.h: Unreal texture related classes.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Constants.
-----------------------------------------------------------------------------*/

enum{NUM_PAL_COLORS=256};	// Number of colors in a standard palette.

/*-----------------------------------------------------------------------------
	UPalette.
-----------------------------------------------------------------------------*/

//
// A truecolor value.
//
class ENGINE_API FColor
{
public:
	// Variables.
	union
	{
		struct
		{
#if __INTEL__
			BYTE R,G,B,A;
#else
			BYTE A,B,G,R;
#endif
		};
		struct
		{
			CHAR NormalU,NormalV;
		};
		DWORD D;
		BYTE Component[4];
	};

	// Constants.
	enum
	{
		EHiColor565_R = 0xf800,
		EHiColor565_G = 0x07e0,
		EHiColor565_B = 0x001f,

		EHiColor555_R = 0x7c00,
		EHiColor555_G = 0x03e0,
		EHiColor555_B = 0x001f,

		ETrueColor_R  = 0x00ff0000,
		ETrueColor_G  = 0x0000ff00,
		ETrueColor_B  = 0x000000ff,
	};

	// Constructors.
	FColor() {}
	FColor( BYTE InR, BYTE InG, BYTE InB )
	:	R(InR), G(InG), B(InB) {}
	FColor( BYTE InR, BYTE InG, BYTE InB, BYTE InA )
	:	R(InR), G(InG), B(InB), A(InA) {}
	FColor( const FPlane& P )
	:	R(Clamp(appFloor(P.R*256),0,255))
	,	G(Clamp(appFloor(P.G*256),0,255))
	,	B(Clamp(appFloor(P.B*256),0,255))
	,	A(Clamp(appFloor(P.W*256),0,255))
	{}

	// Serializer.
	friend FArchive& operator<< (FArchive &Ar, FColor &Color )
	{
		return Ar << Color.R << Color.G << Color.B << Color.A;
	}

	// Operators.
	UBOOL operator==( const FColor &C ) const
	{
		return R==C.R && G==C.G && B==C.B;
	}
	UBOOL operator!=( const FColor &C ) const
	{
		return R!=C.R || G!=C.G || B!=C.B;
	}
	INT Brightness() const
	{
		return (2*(int)R + 3*(int)G + 1*(int)B)>>3;
	}
	FLOAT FBrightness() const
	{
		return (2.0*R + 3.0*G + 1.0*B)/(6.0*256.0);
	}
	DWORD TrueColor() const
	{
		return ((D&0xff)<<16) + (D&0xff00) + ((D&0xff0000)>>16);
	}
	_WORD HiColor565() const
	{
		return ((D&0xf8) << 8) + ((D&0xfC00) >> 5) + ((D&0xf80000) >> 19);
	}
	_WORD HiColor555() const
	{
		return ((D&0xf8) << 7) + ((D&0xf800) >> 6) + ((D&0xf80000) >> 19);
	}
	FVector Plane() const
	{
		return FPlane(R/255.f,G/255.f,B/255.f,A/255.0);
	}
	FColor Brighten( INT Amount )
	{
		return FColor( Plane() * (1.0 - Amount/24.0) );
	}
};

extern ENGINE_API FPlane FGetHSV( BYTE H, BYTE S, BYTE V );

enum EBestMatchRange
{
	RANGE_All           = 0,
	RANGE_AllButZero    = 1,
	RANGE_AllButWindows = 2,
};

//
// A palette object.  Holds NUM_PAL_COLORS unique FColor values, 
// forming a 256-color palette which can be referenced by textures.
//
class ENGINE_API UPalette : public UObject
{
	DECLARE_CLASS(UPalette,UObject,CLASS_SafeReplace)

	// Variables.
	TArray<FColor> Colors;

	// Constructors. 

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UPalette interface.
	BYTE BestMatch( FColor Color, EBestMatchRange Range );
	UPalette* ReplaceWithExisting();
	void Smooth();
	void FixPalette();
	void Flush();
};

/*-----------------------------------------------------------------------------
	UTexture and FTextureInfo.
-----------------------------------------------------------------------------*/

// Maximum texture dimension.
enum {MAX_TEXTURE_SIZE=1024};

// Flags for normal textures.
enum ETextureFlags
{
	// General info about the texture.
	TF_Realtime         = 0x00000008,   // Texture data (not animation) changes in realtime.
	TF_Parametric       = 0x00000010,   // Texture is parametric so data need not be saved.
	TF_RealtimeChanged  = 0x00000020,   // Realtime texture has changed since last lock.
	TF_RealtimePalette  = 0x00000040,	// Realtime palette.
};

//
// Information about one of the texture's mipmaps.
//
struct ENGINE_API FMipmap
{
	BYTE*			DataPtr;		// Pointer to data, valid only when locked.
	INT				USize,  VSize;	// Power of two tile dimensions.
	BYTE			UBits,  VBits;	// Power of two tile bits.
	TArray<BYTE>	DataArray;		// Data.
	FMipmap()
	{}
	FMipmap( BYTE InUBits, BYTE InVBits )
	:	DataPtr		(0)
	,	DataArray	()
	,	USize		(1<<InUBits)
	,	VSize		(1<<InVBits)
	,	UBits		(InUBits)
	,	VBits		(InVBits)
	{
		DataArray.Add( USize * VSize );
	}
	friend FArchive& operator<<( FArchive& Ar, FMipmap& M )
	{
		guard(FMipmap<<);
		return Ar << M.DataArray << M.USize << M.VSize << M.UBits << M.VBits;
		if( Ar.IsLoading() )
			M.DataPtr = NULL;
		unguard;
	}
};

//
// Texture clearing flags.
//
enum ETextureClear
{
	TCLEAR_Temporal	= 1,	// Clear temporal texture effects.
	TCLEAR_Bitmap   = 2,    // Clear the immediate bitmap.
};

//
// A low-level bitmap.
//
class ENGINE_API UBitmap : public UObject
{
	DECLARE_ABSTRACT_CLASS(UBitmap,UObject,0)

	// General bitmap information.
	BYTE		Format;				// ETextureFormat.
	UPalette*	Palette;			// Palette if 8-bit palettized.
	BYTE		UBits, VBits;		// # of bits in USize, i.e. 8 for 256.
	INT			USize, VSize;		// Size, must be power of 2.
	INT			UClamp, VClamp;		// Clamped width, must be <= size.
	FColor		MipZero;			// Overall average color of texture.
	FColor		MaxColor;			// Maximum color for normalization.
	DOUBLE		LastUpdateTime;		// Last time texture was locked for rendering.

	// Static.
	static class UClient* Client;

	// Constructor.
	UBitmap();

	// UBitmap interface.
	virtual void GetInfo( FTextureInfo& TextureInfo, DOUBLE Time )=0;
	virtual INT GetNumMips()=0;
	virtual FMipmap* GetMip( INT i )=0;
};

//
// A complex material texture.
//
class ENGINE_API UTexture : public UBitmap
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UTexture,UBitmap,CLASS_SafeReplace)

	// Subtextures.
	UTexture*	BumpMap;			// Bump map to illuminate this texture with.
	UTexture*	DetailTexture;		// Detail texture to apply.
	UTexture*	MacroTexture;		// Macrotexture to apply, not currently used.

	// Surface properties.
	FLOAT		Diffuse;			// Diffuse lighting coefficient (0.0-1.0).
	FLOAT		Specular;			// Specular lighting coefficient (0.0-1.0).
	FLOAT		Alpha;				// Reflectivity (0.0-0.1).
	FLOAT       Scale;              // Scaling relative to parent, 1.0=normal.
	FLOAT		Friction;			// Surface friction coefficient, 1.0=none, 0.95=some.
	FLOAT		MipMult;			// Mipmap multiplier.

	// Sounds.
	USound*		FootstepSound;		// Footstep sound.
	USound*		HitSound;			// Sound when the texture is hit with a projectile.

	// Flags.
	DWORD		PolyFlags;			// Polygon flags to be applied to Bsp polys with texture (See PF_*).
	DWORD		TextureFlags;		// Texture flags, see TF_*.

	// Animation related.
	UTexture*	AnimNext;			// Next texture in looped animation sequence.
	UTexture*	AnimCur;			// Current animation frame.
	BYTE		PrimeCount;			// Priming total for algorithmic textures.
	BYTE		PrimeCurrent;		// Priming current for algorithmic textures.
	FLOAT		MinFrameRate;		// Minimum animation rate in fps.
	FLOAT		MaxFrameRate;		// Maximum animation rate in fps.
	FLOAT		Accumulator;		// Frame accumulator.

	// Table of mipmaps.
	TArray<FMipmap> Mips;			// Mipmaps.

	// Constructor.
	UTexture();

	// UObject interface.
	void Serialize( FArchive& Ar );
	const char* Import( const char* Buffer, const char* BufferEnd, const char* FileType );
	void Export( FOutputDevice& Out, const char* FileType, INT Indent );
	void PostLoad();

	// UBitmap interface.
	DWORD GetColorsIndex() {return Palette->GetIndex();}
	FColor* GetColors()    {return Palette ? &Palette->Colors(0) : NULL;}
	INT GetNumMips()       {return Mips.Num();}
	FMipmap* GetMip(INT i) {return &Mips(i);}
	void GetInfo( FTextureInfo& TextureInfo, DOUBLE Time );

	// UTexture interface.
	virtual void Clear( DWORD ClearFlags );
	virtual void Init( INT InUSize, INT InVSize );
	virtual void Tick( FLOAT DeltaSeconds );
	virtual void ConstantTimeTick();
	virtual void MousePosition( DWORD Buttons, FLOAT X, FLOAT Y ) {}
	virtual void Click( DWORD Buttons, FLOAT X, FLOAT Y ) {}

	// UTexture functions.
	void Update( DOUBLE Time );
	void BuildRemapIndex( UBOOL Masked );
	void CreateMips( UBOOL FullMips, UBOOL Downsample );
	void CreateColorRange();

	// UTexture accessors.
	UTexture* Get( DOUBLE Time )
	{
		Update( Time );
		return AnimCur ? AnimCur : this;
	}
};

//
// Texture formats.
//
enum ETextureFormat
{
	TEXF_P8			= 0,
	TEXF_RGB32		= 1,
	TEXF_RGB64		= 2,
	TEXF_MAX		= 3,
};

//
// Return the number of bytes per texel of a 
// specified texture format.
//
ENGINE_API inline int GColorBytes( ETextureFormat F )
{
	static int ColorBytes[TEXF_MAX] = {1,4};
	return ColorBytes[F];
}

//
// Information about a locked texture. Used for ease of rendering.
//
enum {MAX_MIPS=12};
struct FTextureInfo
{
	QWORD				CacheID;		// Unique cache ID.
	QWORD				PaletteCacheID;	// Unique cache ID of palette.
	FVector				Pan;			// Panning value relative to texture planes.
	FColor*				MaxColor;		// Maximum color in texture and all its mipmaps.
	ETextureFormat		Format;			// Texture format.
	FLOAT				UScale;			// U Scaling.
	FLOAT				VScale;			// V Scaling.
	INT					USize;			// Base U size.
	INT					VSize;			// Base V size.
	INT					UClamp;			// U clamping value, or 0 if none.
	INT					VClamp;			// V clamping value, or 0 if none.
	INT					NumMips;		// Number of mipmaps.
	FColor*				Palette;		// Palette colors.
	DWORD				TextureFlags;	// From ETextureFlags.
	FMipmap*			Mips[MAX_MIPS];	// Array of NumMips of mipmaps.
};

/*-----------------------------------------------------------------------------
	UFont.
-----------------------------------------------------------------------------*/

//
// Information about one font character which resides in a texture.
//
class ENGINE_API FFontCharacter
{
public:
	// Variables.
	int	StartU, StartV;
	int	USize, VSize;

	// Serializer.
	friend FArchive& operator<< (FArchive &Ar, FFontCharacter &Ch )
	{
		guard(FFontCharacter<<);
		return Ar << Ch.StartU << Ch.StartV << Ch.USize << Ch.VSize;
		unguard;
	}
};

//
// A font object, containing information about a set of characters.
// The character bitmaps are stored in the contained textures, while
// the font database only contains the coordinates of the individual
// characters.
//
class ENGINE_API UFont : public UTexture
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UFont,UTexture,0)

	// Constants.
	enum {NUM_FONT_CHARS=256};

	// Variables.
	TArray<FFontCharacter> Characters;

	// Constructors.
	UFont();

	// UObject interface.
	void Serialize( FArchive& Ar );
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
