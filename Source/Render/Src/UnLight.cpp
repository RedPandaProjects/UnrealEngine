/*=============================================================================
	UnLight.cpp: Unreal global lighting subsystem implementation.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Description:
	Computes all point lighting information and builds surface light meshes 
	based on light actors and shadow maps.

Definitions:
	attenuation:
		The amount by which light diminishes as it travells from a point source
		outward through space.  Physically correct attenuation is propertional to
		1/(distance*distance), but for speed, Unreal uses a lookup table 
		approximation where all light ceases after a light's predefined radius.
	diffuse lighting:
		Viewpoint-invariant lighting on a surface that is the result of a light's
		brightness and a surface texture's diffuse lighting coefficient.
	dynamic light:
		A light that does not move, but has special effects.
	illumination map:
		A 2D array of floating point or MMX Red-Green-Blue-Unused values which 
		represent the illumination that a light applies to a surface. An illumination
		map is the result of combining a light's spatial effects, attenuation,
		incidence factors, and shadow map.
	incidence:
		The angle at which a ray of light hits a point on a surface. Resulting brightness
		is directly proportional to incidence.
    light:
		Any actor whose LightType member has a value other than LT_None.
	meshel:
		A mesh element; a single point in the rectangular NxM mesh containing lighting or
		shadowing values.
	moving light:
		A light that moves. Moving lights do not cast shadows.
	radiosity:
		The process of determining the surface lighting resulting from 
		propagation of light through an environment, accounting for interreflection
		as well as direct light propagation. Radiosity is a computationally
		expensive preprocessing step but generates physically correct lighting.
	raytracing:
		The process of tracing rays through a level between lights and map lattice points
		to precalculate shadow maps, which are later filtered to provide smoothing. 
		Raytracing generates cool looking though physically unrealistic lighting.
	resultant map:
		The final 2D array of floating point or MMX values which represent the total
		illumination resulting from all of the lights (and hence illumination maps) 
		which apply to a surface.
	shadow map:
		A 2D array of floating point values which represent the amount of shadow
		occlusion between a light and a map lattice point, from 0.0 (fully occluded)
		to 1.0 (fully visible).
	shadow hypervolume:
		The six-dimensional hypervolume of space which is not affected by a volume
		lightsource.
	shadow volume:
		The volume of space which is not affected by a point lightsource. The inverse of light
		volume.
	shadow z-buffer:
		A 2D z-buffer representing a perspective projection depth view of the world from a
		lightsource. Often used in dynamic shadowing computations.
	spatial lighting effect:
		A lighting effect that is a function of a location in space, usually relative to
		a light's location.
	specular lighting:
		Viewpoint-varient lighting on a shiny surface that is the result of a
		light's brightness and a surface texture's specular lighting
		coefficient.
	static illumination map:
		An illumination map that represents the total of all static light illumination
		maps that apply to a surface. Static illumination maps do not change in time
		and thus they can be cached.
	static light:
		A light that is constantly on, does not move, and has no special effects.
	surface map:
		Any 2D map that applies to a surface, such as a shadow map or illumination
		map.  Surface maps are always aligned to the surface's U and V texture
		coordinates and are bilinear filtered across the extent of the surface.
	volumetric lighting:
		Lighting that is visible as a result of light interacting with a volume in
		space due to an interacting media such as fog. Volumetric lighting is view
		variant and cannot be associated with a particular surface.

Design notes:
 *	Uses a multi-tiered system for generating the resultant map for a surface,
	where all known constant intermediate and resulting meshes that may be needed 
	in the future are cached, and all known variable intermediate and resulting
	meshes are allocated temporarily.

Notes:
	No radiosity.
	No dynamic shadows.
	No shadow hypervolumes.
	No shadow volumes.
	No shadow z-buffers.

Revision history:
    9-23-96, Tim: Rewritten from the ground up.
=============================================================================*/

#include "RenderPrivate.h"
#include <math.h>

#define SHADOW_SMOOTHING 1 /* Smooth shadows (should be 1) */
#define ZERO_FLOAT_LIGHT (FLOAT)((3<<22) + 0x10)

/*------------------------------------------------------------------------------------
	Approximate math implementation.
------------------------------------------------------------------------------------*/

FLOAT SqrtManTbl[2<<APPROX_MAN_BITS];
FLOAT DivSqrtManTbl[1<<APPROX_MAN_BITS],DivManTbl[1<<APPROX_MAN_BITS];
FLOAT DivSqrtExpTbl[1<<APPROX_EXP_BITS],DivExpTbl[1<<APPROX_EXP_BITS];

static INT SavedESP,SavedEBP; 

/*------------------------------------------------------------------------------------
	Subsystem definition
------------------------------------------------------------------------------------*/

//
// Lighting manager definition.
//
class FLightManager : public FLightManagerBase
{
public:
	// FLightManagerBase functions.
	void Init();
	void Exit();
	DWORD SetupForActor( FSceneNode* Frame, AActor* Actor, FVolActorLink* LeafLights, FActorLink* Volumetrics );
	void SetupForSurf( FSceneNode* Frame, FCoords& FacetCoords, FBspDrawList* Draw, FTextureInfo*& LightMap, FTextureInfo*& FogMap, FTextureInfo* BumpMap, UBOOL Merged );
	void FinishSurf();
	void FinishActor();
	FPlane Light( FTransSample& Point, DWORD PolyFlags );
	FPlane Fog( FTransSample& Point, DWORD PolyFlags );
	FPlane AMD3DLight( FTransSample& Point, DWORD PolyFlags );
	FPlane AMD3DFog( FTransSample& Point, DWORD PolyFlags );

	// Constants.
	class FLightInfo;
	enum {MAX_LIGHTS=256};

	// Function pointer types.
	typedef void (*LIGHT_SPATIAL_FUNC)( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	typedef DWORD FILTER_TAB[4];

	// Information about one special lighting effect.
	struct FLocalEffectEntry
	{
		LIGHT_SPATIAL_FUNC	SpatialFxFunc;		// Function to perform spatial lighting
		INT					IsSpatialDynamic;	// Indicates whether light spatiality changes over time.
		INT					IsMergeDynamic;		// Indicates whether merge function changes over time.
	};
	enum ELightKind
	{
		ALO_StaticLight		= 0,	// Actor is a non-moving, non-changing lightsource
		ALO_DynamicLight	= 1,	// Actor is a non-moving, changing lightsource
		ALO_MovingLight		= 2,	// Actor is a moving, changing lightsource
		ALO_NotLight		= 3,	// Not a surface light (probably volumetric only).
	};

	// Information about a lightsource.
	class FLightInfo
	{
	public:
		// For all lights.
		AActor*		Actor;					// All actor drawing info.
		ELightKind	Opt;					// Light type.
		FVector		Location;				// Transformed screenspace location of light.
		FLOAT		Radius;					// Maximum effective radius.
		FLOAT		RRadius;				// 1.0 / Radius.
		FLOAT		RRadiusMult;			// 16383.0 / (Radius * Radius).
		FLOAT		Brightness;				// Center brightness at this instance, 1.0=max, 0.0=none.
		FLOAT		Diffuse;				// BaseNormalDelta * RRadius.
		BYTE*		IlluminationMap;		// Temporary illumination map pointer.
		BYTE*		ShadowBits;				// Temporary shadow map.
		UBOOL		IsVolumetric;			// Whether it's volumetric.

		// Clipping region.
		INT MinU, MaxU, MinV, MaxV;

		// For volumetric lights.
		FLOAT		VolRadius;				// Volumetric radius.
		FLOAT		VolRadiusSquared;		// VolRadius*VolRadius.
		FLOAT		VolBrightness;			// Volumetric lighting brightness.
		FLOAT		LocationSizeSquared;	// Location.SizeSqurated().
		FLOAT		RVolRadius;				// 1/Volumetric radius.
		FLOAT		RVolRadiusSquared;		// 1/VolRadius*VolRadius.
		UBOOL       VolInside;               // Viewpoint is inside the sphere.

		// Information about the lighting effect.
		FLocalEffectEntry Effect;

		// Coloring.
		FPlane		FloatColor;				// Incident lighting color.
		FPlane		VolumetricColor;		// Volumetric lighting color.
		FColor*		Palette;				// Brightness scaler.
		FColor*     VolPalette;             // Volumetric color scaler.
		// Functions.
		void ComputeFromActor( FTextureInfo& Map, FSceneNode* Frame, UBOOL Surface );
	};

	// Spatial lighting functions.
	static void spatial_None		( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_SearchLight	( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_SlowWave	( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_FastWave	( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_CloudCast	( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Shock		( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Disco		( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Interference( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Cylinder	( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Rotor		( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Spotlight	( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_NonIncidence( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Shell       ( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );
	static void spatial_Test		( FTextureInfo& Tex, FLightInfo* Info, BYTE* Src, BYTE* Dest );

	// FLightManager functions.
	static void Merge( FTextureInfo& Tex, BYTE LightEffect, INT Key, FLightInfo* Light, DWORD* Stream, DWORD* Dest );
	static FLOAT Volumetric( FLightInfo* Info, FVector& Vertex );
	void ShadowMapGen( FTextureInfo& Tex, BYTE* SrcBits, BYTE* Dest1 );
	UBOOL AddLight( AActor* Actor, AActor* Other );

	// Variables.
	static FCoords			*MapCoords, MapUncoords;
	static FVector			VertexBase, VertexDU, VertexDV;
	static FSceneNode*		Frame;
	static ULevel*			Level;
	static FMemMark			Mark;
	static INT				ShadowMaskU, ShadowMaskSpace, ShadowSkip;
	static INT				StaticLights, DynamicLights, MovingLights, StaticLightingChanged;
	static FTextureInfo		LightMap, FogMap;
	static FMipmap			LightMip, FogMip;
	static FLightInfo*		LastLight;
	static FLightInfo*const FinalLight;
	static FLightInfo		FirstLight[MAX_LIGHTS];
	static ALevelInfo*		LevelInfo;
	static AZoneInfo*		Zone;
	static FPlane			AmbientVector;
	static FLOAT			Diffuse;
	static INT              TemporaryTablesBuilt;
	static FLOAT			BackdropBrightness;
	static FLOAT            LightSqrt[4096];
	static FILTER_TAB		FilterTab[128];
	static BYTE				ByteMuck[0x4000];
	static AActor*			Actor;
	static const FLocalEffectEntry Effects[LE_MAX];

	// Memory cache info.
	enum {MAX_UNLOCKED_ITEMS=256};
	static FCacheItem* ItemsToUnlock[MAX_UNLOCKED_ITEMS];
	static FCacheItem** TopItemToUnlock;
};

FCoords*						FLightManager::MapCoords;
FCoords							FLightManager::MapUncoords;
FVector							FLightManager::VertexBase;
FVector							FLightManager::VertexDU;
FVector							FLightManager::VertexDV;
FSceneNode*						FLightManager::Frame;
ULevel*							FLightManager::Level;
FMemMark						FLightManager::Mark;
INT								FLightManager::ShadowMaskU;
INT								FLightManager::ShadowMaskSpace;
INT								FLightManager::ShadowSkip;
INT								FLightManager::StaticLights;
INT								FLightManager::DynamicLights;
INT								FLightManager::MovingLights;
INT								FLightManager::StaticLightingChanged;
FTextureInfo					FLightManager::LightMap;
FTextureInfo					FLightManager::FogMap;
FMipmap							FLightManager::LightMip;
FMipmap							FLightManager::FogMip;
FLightManager::FLightInfo*		FLightManager::LastLight;
FLightManager::FLightInfo*const	FLightManager::FinalLight = &FirstLight[MAX_LIGHTS];
FLightManager::FLightInfo		FLightManager::FirstLight[MAX_LIGHTS];
FLightManager::FILTER_TAB		FLightManager::FilterTab[128];
ALevelInfo*						FLightManager::LevelInfo;
AZoneInfo*						FLightManager::Zone;
FPlane							FLightManager::AmbientVector;
FLOAT							FLightManager::Diffuse;
BYTE							FLightManager::ByteMuck[0x4000];
AActor*							FLightManager::Actor;
INT								FLightManager::TemporaryTablesBuilt;
FLOAT							FLightManager::BackdropBrightness;
FLOAT							FLightManager::LightSqrt[4096];
FCacheItem*						FLightManager::ItemsToUnlock[MAX_UNLOCKED_ITEMS];
FCacheItem**					FLightManager::TopItemToUnlock;

const FLightManager::FLocalEffectEntry FLightManager::Effects[LE_MAX] =
{
// LE_ tag			Spatial func			SpacDyn MergeDyn
// ----------------	-----------------------	------- --------
{/* None         */	spatial_None,			0,		0        },
{/* TorchWaver   */	spatial_None,			0,		1        },
{/* FireWaver    */	spatial_None,			0,		1        },
{/* WateryShimmer*/	spatial_None,			0,		1        },
{/* Searchlight  */	spatial_SearchLight,	1,		0        },
{/* SlowWave     */	spatial_SlowWave,		1,		0        },
{/* FastWave     */	spatial_FastWave,		1,		0        },
{/* CloudCast    */	spatial_CloudCast,		1,		0        },
{/* StaticSpot   */	spatial_Spotlight,		0,		0        },
{/* Shock        */	spatial_Shock,			1,		0        },
{/* Disco        */	spatial_Disco,			1,		0        },
{/* Warp         */	spatial_None,			0,		0        },
{/* Spotlight    */	spatial_Spotlight,		0,		0        },
{/* NonIncidence */	spatial_NonIncidence,	0,		0        },
{/* Shell        */	spatial_Shell,			0,		0        },
{/* Satellite    */	spatial_None,			0,		0        },
{/* Interference */	spatial_Interference,	1,		0        },
{/* Cylinder     */	spatial_Cylinder,		0,		0        },
{/* Rotor        */	spatial_Rotor,			1,		0        },
{/* Unused		 */	spatial_None,			0,		0        },
};

/*------------------------------------------------------------------------------------
	Init & Exit.
------------------------------------------------------------------------------------*/

//
// Set up the tables required for fast square root computation.
//
static void SetupTable( FLOAT* ManTbl, FLOAT* ExpTbl, FLOAT Power )
{
	union {FLOAT F; DWORD D;} Temp;

	Temp.F = 1.0;
	for( DWORD i=0; i<(1<<APPROX_EXP_BITS); i++ )
	{
		Temp.D = (Temp.D & 0x007fffff ) + (i << (32-APPROX_EXP_BITS));
		ExpTbl[ i ] = appPow( Abs(Temp.F), Power );
		if( appIsNan(ExpTbl[ i ]) )
			ExpTbl[ i ]=0.0;
		//debugf("exp [%f] %i = %f",Power,i,ExpTbl[i]);
	}

	Temp.F = 1.0;
	for( i=0; i<(1<<APPROX_MAN_BITS); i++ )
	{
		Temp.D = (Temp.D & 0xff800000 ) + (i << (32-APPROX_EXP_BITS-APPROX_MAN_BITS));
		ManTbl[ i ] = appPow( Abs(Temp.F), Power );
		if( appIsNan(ManTbl[ i ]) )
			ManTbl[ i ]=0.0;
		//debugf("man [%f] %i = %f",i,Power,ManTbl[i]);
	}
}

//
// Initialize the global lighting subsystem.
//
void FLightManager::Init()
{
	guard(FLightManager::Init);

	// Mutual occlusion blending.
	for( INT i=0; i<128; i++ )
		for( int j=0; j<128; j++ )
			ByteMuck[i*128+j] = i + j - (i*j/127); 

	// Filtering table.
	INT FilterWeight[8][8] = 
	{
		{ 0,24,40,24,0,0,0,0},
		{ 0,40,64,40,0,0,0,0},
		{ 0,24,40,24,0,0,0,0},
		{ 0, 0, 0, 0,0,0,0,0},
		{ 0, 0, 0, 0,0,0,0,0},
		{ 0, 0, 0, 0,0,0,0,0},
		{ 0, 0, 0, 0,0,0,0,0},
		{ 0, 0, 0, 0,0,0,0,0}
	};

	// Setup square root tables.
	for( DWORD D=0; D< (1<< APPROX_MAN_BITS ); D++ )
	{
		union {FLOAT F; DWORD D;} Temp;
		Temp.F = 1.0;
		Temp.D = (Temp.D & 0xff800000 ) + (D << (23 - APPROX_MAN_BITS));
		Temp.F = appSqrt(Temp.F);
		Temp.D = (Temp.D - ( 64 << 23 ) );   // exponent bias re-adjust
		SqrtManTbl[ D ] = (FLOAT)(Temp.F * appSqrt(2.0)); // for odd exponents
		SqrtManTbl[ D + (1 << APPROX_MAN_BITS) ] =  (FLOAT) (Temp.F * 2.0);
	}
	SetupTable(DivSqrtManTbl,DivSqrtExpTbl,-0.5);
	SetupTable(DivManTbl,    DivExpTbl,    -1.0);

	
	// Init square roots.
	for( i=0; i<ARRAY_COUNT(LightSqrt); i++ )
	{
		FLOAT S = appSqrt((FLOAT)(i+1) * (1.0/ARRAY_COUNT(LightSqrt)));

		// This function gives a more luminous, specular look to the lighting.
		FLOAT Temp = (2*S*S*S-3*S*S+1); // Or 1.0-S.

		// This function makes surfaces look more matte.
		//FLOAT Temp = (1.0-S);
		LightSqrt[i] = Temp/S;
	}

	// Generate filter lookup table
	int FilterSum=0;
	for( i=0; i<8; i++ )
		for( int j=0; j<8; j++ )
			FilterSum += FilterWeight[i][j];

	// Iterate through all filter table indices 0x00-0x3f.
	for( i=0; i<128; i++ )
	{
		// Iterate through all vertical filter weights 0-3.
		for( int j=0; j<4; j++ )
		{
			// Handle all four packed values.
			FilterTab[i][j] = 0;
			for( INT Pack=0; Pack<4; Pack++ )
			{
				// Accumulate filter weights in FilterTab[i][j] according to which bits are set in i.
				INT Acc = 0;
				for( INT Bit=0; Bit<8; Bit++ )
					if( i & (1<<(Pack + Bit)) )
						Acc += FilterWeight[j][Bit];

				// Add to sum.
				DWORD Result = (Acc * 255) / FilterSum;
				check(Result>=0 && Result<=255);
				FilterTab[i][j] += (Result << (Pack*8));
			}
		}
	}

	// Cache items.
	TopItemToUnlock = &ItemsToUnlock[0];

	// Success.
	debugf( NAME_Init, "Lighting subsystem initialized" );
	unguard;
}

//
// Shut down the global lighting system.
//
void FLightManager::Exit()
{
	guard(FLightManager::Exit);

	debugf( NAME_Exit, "Lighting subsystem shut down" );
	unguard;
}

/*------------------------------------------------------------------------------------
	Intermediate map generation code
------------------------------------------------------------------------------------*/

//
// Generate the shadow map for one lightsource that applies to a Bsp surface.
//
// Input: 1-bit-deep occlusion bitmask.
//
// Output: Floating point representation of the fraction of occlusion
// between each point and the lightsource, and range from 0.0 (fully occluded) to
// 1.0 (fully unoccluded).  These values refer only to occlusion and say nothing
// about resultant lighting (brightness, incidence, etc) which IlluminationMapGen handles.
//
void FLightManager::ShadowMapGen( FTextureInfo& Tex, BYTE* SrcBits, BYTE* Dest1 )
{
	guardSlow(FLightManager::ShadowMapGen);

	// If no source, fill it.
	if( !SrcBits )
	{
		appMemset( Dest1, 127, ShadowMaskSpace*8 );
		return;
	}
	debug(((INT)Dest1 & 3)==0);

	// Generate smooth shadow map by convolving the shadow bitmask with a smoothing filter.
	INT Size4 = (ShadowMaskU*8)/4;
	appMemset( Dest1, 0, ShadowMaskSpace*8 );
	DWORD* Dests[3] = { (DWORD*)Dest1, (DWORD*)Dest1, (DWORD*)Dest1 + Size4 };
	for( INT V=0; V<Tex.VClamp; V++ )
	{
		// Get initial bits, with low bit shifted in.
		BYTE* Src = SrcBits;

		// Offset of shadow map relative to convolution filter left edge.
		DWORD D = (DWORD)*Src++ << (8+2);
		if( D & 0x400 ) D |= 0x300;

		// Filter everything.
		for( INT U=0; U<ShadowMaskU; U++ )
		{
			D = (D >> 8) | (((DWORD)*Src++) << (8+2));

			FILTER_TAB& Tab1 = FilterTab[D & 0x7f];
			*Dests[0]++     += Tab1[0];
			*Dests[1]++     += Tab1[1];
			*Dests[2]++     += Tab1[2];

			FILTER_TAB& Tab2 = FilterTab[(D>>4) & 0x7f];
			*Dests[0]++     += Tab2[0];
			*Dests[1]++     += Tab2[1];
			*Dests[2]++     += Tab2[2];
		}
		SrcBits += ShadowMaskU;
		if( V == 0            ) Dests[0] -= Size4;
		if( V == Tex.VClamp-2 ) Dests[2] -= Size4;
	}
	unguardSlow;
}

/*------------------------------------------------------------------------------------
	Vertex lighting and fogging.
------------------------------------------------------------------------------------*/

#ifndef NOAMD3D

#pragma warning( disable : 4799 )
#pragma pack( 8 )

#define MAKE_DWORD(Val) *((DWORD *)&Val)
#define MAKE_FP(Val) *((float *)&Val)

static _inline void DoFemms(void)
{
	_asm femms;
}

static _inline UBOOL CheckFloat(FLOAT A,FLOAT B)
{
	_asm femms;
	FLOAT Diff=A-B;
	if (A!=0)
		Diff=Diff/A;
	if (Diff<0)
		Diff=-Diff;
	UBOOL RetVal=(Diff>0.05);
	_asm femms;
	return RetVal;
}

static _inline UBOOL CheckVector(FVector& V1,FVector& V2)
{
	return (CheckFloat(V1.X,V2.X) ||
			CheckFloat(V1.Y,V2.Y) ||
			CheckFloat(V1.Z,V2.Z));
}

//
// K6 3D optmized version of:
//		FLOAT PointSquared(Vert.Point.SizeSquared());
//
static _inline void DoSetPointSquared(FTransSample& Vert,FLOAT& PointSquared)
{
#if 1
	_asm
	{	
		mov		eax,Vert
		mov		edx,PointSquared
		movq	mm0,[eax]FOutVector.Point.X			// mm0=Y|X, mm1=0|Z
		movd	mm1,[eax]FOutVector.Point.Z		
		pfmul	(m0,m0)								// mm0=Y^2|X^2, mm1=0|Z^2
		pfmul	(m1,m1)
		pfacc	(m0,m0)								// mm0=?|X^2+Y^2, mm1=?|X^2+Y^2+Z^2=P^2
		pfadd	(m1,m0)
		movd	[edx],mm1							// save P^2

		femms
	}

	FLOAT TestP2=Vert.Point.X*Vert.Point.X + Vert.Point.Y*Vert.Point.Y + Vert.Point.Z*Vert.Point.Z;
	if (CheckFloat(PointSquared,TestP2))
		debugf("POINTSQUARED WRONG: %f should be %f",PointSquared,TestP2);

#else
	PointSquared = Vert.Point.X*Vert.Point.X + Vert.Point.Y*Vert.Point.Y + Vert.Point.Z*Vert.Point.Z;
#endif
}

//
// K6 3D optimized version of the inside of the loop of FLightManager::Light
//
static _inline void DoLightingInnerLoop(FTransSample& Vert, 
										FVector& Location,
										FLOAT& PointSquared,
										FLOAT& RRadius,
										FPlane& FloatColor,
										FPlane& DestColor)
{
#if 1
	static float Two=2.0f;
	static float One=1.0f;
	static float OnePointFive=1.5f;
	static float Six=6.0f;
	FVector	LightVector;
	FLOAT	LightSquared;

	_asm
	{
		femms
		mov		eax,Location						// eax=ptr to Location
		mov		ecx,Vert							// ecx=ptr to Vert
		movq	mm0,[eax]FVector.X					// mm0=Yll|Xll, mm1=Yn|Xn
		movq	mm1,[ecx]FTransSample.Normal+0/*X*/
		movq	mm2,[ecx]FOutVector.Point.X			// mm2=Yp|Xp, mm1=Ynll|Xnll
		pfmul	(m1,m0)
		movd	mm3,[eax]FVector.Z					// mm3=0|Zll, mm2=Ylv|Xlv
		pfsubr	(m2,m0)
		movq	mm4,[ecx]FTransSample.Normal+8/*Z*/	// mm4=Wn|Zn, mm1=?|Ynll+Ynll
		pfacc	(m1,m1)
		movq	mm5,mm4								// mm5=Wn|Zn, mm4=0|Znll
		pfmul	(m4,m3)
		movd	mm6,[ecx]FOutVector.Point.Z			// mm6=0|Zp, mm5=0|Wn
		psrlq	mm5,32
		movd	mm7,Two								// mm7=0|2.0, mm1=?|Xnll+Ynll+Znll
		pfadd	(m1,m4)
		movq	LightVector.X,mm2					// save Ylv|Xlv, mm6=0|Zlv
		pfsubr	(m6,m3)
		pfmul	(m2,m2)								// mm2=Ylv^2|Xlv^2, mm1=?|Ynll+Ynll+Znll-Wn
		pfsub	(m1,m5)
		movd	LightVector.Z,mm6					// save Zlv, mm6=0|Zlv^2
		pfmul	(m6,m6)
		pfacc	(m2,m2)								// mm2=?|Ylv^2+Xlv^2, mm5=Yn|Xn
		movq	mm5,[ecx]FTransSample.Normal+0/*X*/
		pfmul	(m1,m7)								// mm1=?|2(Ynll+Ynll+Znll-Wn)=?|S, mm4=0|Zn
		movd	mm4,[ecx]FTransSample.Normal+8/*Z*/
		pfadd	(m2,m6)								// mm2=?|Ylv^2+Xlv^2+Zlv^2=L^2, mm4=0|ZnS
		pfmul	(m4,m1)
		punpckldq mm1,mm1							// mm1=S|S, mm7=Ylv|Xlv
		movq	mm7,LightVector.X
		movd	LightSquared,mm2					// added - save L^2, edx=ptr to PointSquared
		mov		edx,PointSquared
		pfmul	(m1,m5)								// mm1=YnS|XnS, mm2=1/LS|1/LS
		pfrsqrt	(m2,m2)
		pfmul	(m5,m7)								// mm5=Ynlv|Xnlv, mm7=0|Zp
		movd	mm7,[ecx]FOutVector.Point.Z
		pfsub	(m3,m4)								// mm3=0|Zll-ZnS=0|Zm, mm4=Yp|Xp
		movq	mm4,[ecx]FOutVector.Point.X
		pfsub	(m0,m1)								// mm0=Yll-YnS|Xll-XnS=Ym|Xm, mm6=0|Zn
		movd	mm6,[ecx]FTransSample.Normal+8/*Z*/
		pfmul	(m3,m7)								// mm3=0|Zmp, mm7=0|Zlv
		movd	mm7,LightVector.Z
		pfmul	(m0,m4)								// mm0=Ymp|Xmp, mm4=0|P^2
		movd	mm4,[edx]
		pfacc	(m5,m5)								// mm5=?|Xnlv+Ynlv,	mm6=0|Znlv
		pfmul	(m6,m7)
		pfacc	(m0,m0)								// mm0=?|Xm^2+Ym^2, mm1=0|1.0
		movd	mm1,One
		pfadd	(m5,m6)								// mm5=?|Xnlv+Ynlv+Znlv=NVL^2, mm7=0|1.5
		movd	mm7,OnePointFive
		pfadd	(m0,m3)								// mm0=?|Xm^2+Ym^2+Zm^2=?|M^2, mm5=0|NLV^2/LS
		pfmul	(m5,m2)
		pfrcp	(m2,m2)								// mm2=LS|LS, mm0=?|M^2-P^2=?|SP
		pfsub	(m0,m4)
		movd	mm3,LightSquared					// mm3=0|L^2, mm1=0|1.0+NLV^2/LS
		pfadd	(m1,m5)
		pxor	mm6,mm6								// mm6=0|0, mm4=0|(PL)^2
		mov		edx,RRadius							// added - edx=ptr to RRadius
		pfmul	(m4,m3)
		pfcmpge	(m6,m0)								// mm6=?|0>=SP, mm1=?|(1.0+NLV^2/LS)^2
		pfmul	(m1,m1)
		pfrcp	(m4,m4)								// mm4=1/(PL)^2|1/(PL)^2, mm3=0|RRadius
		movd	mm3,[edx]
		pfmul	(m0,m0)								// mm0=?|SP^2, mm5=0|6.0
		movd	mm5,Six
		pfsub	(m1,m7)								// mm1=?|(1.0+NLV^2/LS)^2-1.5=?|G,
		pandn	mm6,mm4										//	mm6=?|Masked(1/(PL)^2)
		pfmul	(m5,m0)								// mm5=?|6*SP^2, mm4=0|0
		pxor	mm4,mm4
		mov		edx,FloatColor						// added - edx=ptr to FloatColor
		pfcmpgt	(m4,m1)								// mm4=?|0>G, mm7=0|1.0
		movd	mm7,One
		pfmul	(m5,m6)								// mm5=?|6*SP^2*Masked(1/(PL)^2)=0|SpecG
		movq	mm0,[edx]FVector.R							// mm0=Gl|Rl
		pfmul	(m3,m2)								// mm3=0|RRadius*LS, mm4=mG
		pandn	mm4,mm1
		mov		eax,DestColor						// added - edx=ptr to DestColor
		pfadd	(m5,m4)								// mm5=0|SpecG+mG, mm2=0|Bl
		movd	mm2,[edx]FVector.B
		pfsub	(m7,m3)								// mm7=?|1-RRadius*LS, mm1=G|R
		movq	mm1,[eax]FVector.R
		pxor	mm6,mm6								// mm6=0|0, mm4=0|B
		movd	mm4,[eax]FVector.B
		pfmul	(m7,m5)								// mm7=?|1-RRadius*LS*(SpecG+mG)=G',
		pfcmpge (m6,m7)										// mm6=?|inv(Mask)
		pandn	mm6,mm7								// mm6=?|mG', mm2=0|Bl'
		pfmul	(m2,m6)
		punpckldq mm6,mm6							// mm6=mG'|mG', mm6=Gl'|Rl'
		pfmul	(m6,m0)
		pfadd	(m4,m2)								// mm4=0|B+Bl'=0|B', mm6=G+Gl'|R+Rl'=G'|R'
		pfadd	(m6,m1)
		movd	[eax]FVector.B,mm4					// save B', save G'|R'
		movq	[eax]FVector.R,mm6

		femms
	}
#else
	// Diffuse lighting.
	FVector LightVector;
	LightVector.X = Location.X - Vert.Point.X;
	LightVector.Y = Location.Y - Vert.Point.Y;
	LightVector.Z = Location.Z - Vert.Point.Z;
	FLOAT   LightSquared = LightVector.X*LightVector.X + LightVector.Y*LightVector.Y + LightVector.Z*LightVector.Z;
	FLOAT   LightSize    = SqrtApprox( LightSquared );
	FLOAT   G            = Square(1.0 + (LightVector.X*Vert.Normal.X + LightVector.Y*Vert.Normal.Y + LightVector.Z*Vert.Normal.Z) / LightSize) - 1.5;
	if ( G < 0.0)
		G = 0.0;

	// Specular lighting.
	FVector Mirrored;
	FLOAT Scale=2.0 * (Vert.Normal.X*Location.X + 
					   Vert.Normal.Y*Location.Y +
					   Vert.Normal.Z*Location.Z -
					   Vert.Normal.W);
	Mirrored.X=Location.X - Vert.Normal.X * Scale;
	Mirrored.Y=Location.Y - Vert.Normal.Y * Scale;
	Mirrored.Z=Location.Z - Vert.Normal.Z * Scale;
	
	FLOAT Specular = (Mirrored.X*Vert.Point.X + Mirrored.Y*Vert.Point.Y + Mirrored.Z*Vert.Point.Z) - PointSquared;
	if ( Specular > 0.0 )
		G += 6.0 * Square(Specular)/(LightSquared*PointSquared);

	// Radial falloff.
	G *= 1.0 - LightSize * RRadius;

	// Update result color.
	if( G > 0.0 )
	{
		DestColor.R += FloatColor.R * G;
		DestColor.G += FloatColor.G * G;
		DestColor.B += FloatColor.B * G;
	}
#endif
}

//
// K6 3D optimized version of:
//		Color.R = Min( Diffuse * Color.R + AmbientVector.R, 1.f );
//		Color.G = Min( Diffuse * Color.G + AmbientVector.G, 1.f );
//		Color.B = Min( Diffuse * Color.B + AmbientVector.B, 1.f );
//
static _inline void DoAddAmbientLight(FPlane& Color,FLOAT& Diffuse,FPlane& AmbientVector)
{
#if 1
	// These are used to access static variables with class scope. Damn compiler.
	static union
	{
		float f[2];
		double d;
	} One={1.0,1.0};
	_asm
	{
		mov		ecx,Diffuse				// ecx=ptr to Diffuse
		mov		edx,Color				// edx=ptr to Color
		mov		eax,AmbientVector		// eax=ptr to AmbientVector
		movd	mm0,[ecx]				// mm0=0|D, mm1=G|R
		movq	mm1,[edx]FVector.R
		punpckldq mm0,mm0				// mm0=D|D, mm2=0|B
		movd	mm2,[edx]FVector.B
		pfmul	(m1,m0)					// mm1=GD|RD, mm3=Ga|Ra
		movq	mm3,[eax]FVector.R
		pfmul	(m2,m0)					// mm2=0|BD, mm4=0|Ba
		movd	mm4,[eax]FVector.B
		pfadd	(m3,m1)					// mm3=GD+Ga|RD+Ra=G'|R', mm5=1.0|1.0
		movq	mm5,One
		pfadd	(m4,m2)					// mm4=0|BD+Ba=0|B', mm6=0|1.0
		movd	mm6,One
		pfcmpge	(m5,m3)					// mm5=invGmask|invRmask, mm7=1.0|1.0
		movq	mm7,One
		pfcmpge	(m6,m4)					// mm6=?|invBmask, mm2=0|1.0
		movd	mm2,One
		pand	mm3,mm5					// mm3=masked(G'|R'), mm5=masked(1.0|1.0)
		pandn	mm5,mm7
		pand	mm4,mm6					// mm4=masked(?|B'), mm6=masked(?|1.0)
		pandn	mm6,mm2
		por		mm5,mm3					// mm7=G"|R", mm4=?|B"
		por		mm6,mm4
		movq	[edx]FVector.R,mm5		// save G"|R", save B"
		movd	[edx]FVector.B,mm6
	}
#else
	Color.R = Diffuse * Color.R + AmbientVector.R;
	Color.G = Diffuse * Color.G + AmbientVector.G;
	Color.B = Diffuse * Color.B + AmbientVector.B;
	if (Color.R > 1.0f) Color.R = 1.0f;
	if (Color.G > 1.0f) Color.G = 1.0f;
	if (Color.B > 1.0f) Color.B = 1.0f;
#endif
}

// 
// K6 3D optimized version of:
//		Color = Color*0.5 + FVector(0.5,0.5,0.5);
//
static _inline void DoEditorAdjust(FPlane& Color)
{
#if 1
	static union
	{
		float f[2];
		double d;
	} PointFive={0.5,0.5};
	_asm
	{
		mov		eax,Color
		movq	mm0,PointFive		// mm0=0.5|0.5, mm1=G|R
		movq	mm1,[eax]FVector.R
		movd	mm2,[eax]FVector.B	// mm2=0|B, mm1=0.5G|0.5R
		pfmul	(m1,m0)
		pfmul	(m2,m0)				// mm2=0|0.5B, mm1=0.5G+0.5|0.5R+0.5=G'|R'
		pfadd	(m1,m0)
		pfadd	(m2,m0)				// mm2=0.5|0.5B+0.5=0.5|B', save G'|R'
		movq	[eax]FVector.R,mm1
		movd	[eax]FVector.B,mm2	// save B'
	}
#else
	Color.R = Color.R*0.5 + 0.5;
	Color.G = Color.G*0.5 + 0.5;
	Color.B = Color.B*0.5 + 0.5;
#endif
}

__inline FPlane FLightManager::AMD3DLight( FTransSample& Vert, DWORD PolyFlags )
{
	guard(FLightManager::AMD3DLight);
	STAT(clock(GStat.MeshLightTime));

	DoFemms();

	FPlane Color(0,0,0,0);
	if( !(PolyFlags & PF_Unlit) )
	{
		// Lit.
		STAT(GStat.MeshVertLightCount += LastLight-FirstLight);
//		FLOAT PointSquared(Vert.Point.SizeSquared());
		FLOAT PointSquared;
		DoSetPointSquared(Vert,PointSquared);

		for( FLightInfo* Light=FirstLight; Light<LastLight; Light++ )
		{
			if( Light->Opt != ALO_NotLight )
			{
//				// Diffuse lighting.
//				FVector LightVector  = Light->Location - Vert.Point;
//				FLOAT   LightSquared = LightVector.SizeSquared();
//				FLOAT   LightSize    = SqrtApprox( LightSquared );
//				FLOAT   G            = Square(1.0 + (LightVector | Vert.Normal) / LightSize) - 1.5;
//				if ( G < 0.0)
//					G = 0.0;
//
//				// Specular lighting.
//				FLOAT Specular = (Light->Location.MirrorByPlane(Vert.Normal) | Vert.Point) - PointSquared;
//				if ( Specular > 0.0 )
//					G += 6.0 * Square(Specular)/(LightSquared*PointSquared);
//
//				// Radial falloff.
//				G *= 1.0 - LightSize * Light->RRadius;
//
//				// Update result color.
//				if( G > 0.0 )
//					Color += Light->FloatColor * G;
				DoLightingInnerLoop(Vert,Light->Location,PointSquared,
									Light->RRadius,Light->FloatColor,Color);
			}
		}
	}
	else Color = FPlane(0.5,0.5,0.5,0);

	// Add ambient light.
//	Color.R = Min( Diffuse * Color.R + AmbientVector.R, 1.f );
//	Color.G = Min( Diffuse * Color.G + AmbientVector.G, 1.f );
//	Color.B = Min( Diffuse * Color.B + AmbientVector.B, 1.f );
	DoAddAmbientLight(Color,Diffuse,AmbientVector);

	// Editor highlighting.
	if( (PolyFlags & PF_Selected) && GIsEditor )
//		Color = Color*0.5 + FVector(0.5,0.5,0.5);
		DoEditorAdjust(Color);

	DoFemms();

	STAT(unclock(GStat.MeshLightTime));
	return Color;
	unguard;
}

__inline FPlane FLightManager::AMD3DFog(FTransSample& Vert, DWORD PolyFlags )
{
	guard(FLightManager::AMD3DFog);

	if( PolyFlags & PF_RenderFog )
	{
		STAT(clock(GStat.MeshLightTime));

		DoFemms();

		FPlane Fog(0,0,0,0);

		// First fog light is copied, all next lights are merged.
		// Look for the first volumetric light.

		// Todo: zero/bailout test probably also useful here. Try higher level
		// optims (bounding spheres etc) first !

		for( FLightInfo* LightInfo=FirstLight; LightInfo<LastLight; LightInfo++ )
		{
			if( LightInfo->IsVolumetric ) 
			{
				FPlane LocalFog;
				FLOAT VolumeValue = 2.0f * Volumetric( LightInfo, Vert.Point);
				LocalFog.R = LightInfo->VolumetricColor.R * VolumeValue;
				LocalFog.G = LightInfo->VolumetricColor.G * VolumeValue;
				LocalFog.B = LightInfo->VolumetricColor.B * VolumeValue;
				// LocalFog.A = LightInfo->VolumetricColor.A * VolumeValue;

				Fog.R = MinPositiveFloat( LocalFog.R, 1.0f );
				Fog.G = MinPositiveFloat( LocalFog.G, 1.0f );
				Fog.B = MinPositiveFloat( LocalFog.B, 1.0f );
				Fog.A = 0.f; // Fog.A = MinPositiveFloat( LocalFog.A, 1.0f );
				break; 
			}		
		}

		// Continue merging.
		// Really should be sorted & Z order dependent !
		while( ++LightInfo < LastLight)
		{
			if( LightInfo->IsVolumetric ) 
			{
				// Todo: Alpha-merging r		
				// Todo: many Volumetric==0 - values go through these calculations unnecessarily.
				// todo: MinPositiveFloat scaling shouldn't be necessary ??

				FPlane LocalFog;
				FLOAT VolumeValue = 2.0f * Volumetric( LightInfo, Vert.Point);
				if (*(DWORD*)&VolumeValue != 0)	// quick zero test 
				{
					LocalFog.R = LightInfo->VolumetricColor.R * VolumeValue;
					LocalFog.G = LightInfo->VolumetricColor.G * VolumeValue;
					LocalFog.B = LightInfo->VolumetricColor.B * VolumeValue;
					// LocalFog.A = LightInfo->VolumetricColor.A * VolumeValue;

					LocalFog.R = MinPositiveFloat( LocalFog.R, 1.0f );
					LocalFog.G = MinPositiveFloat( LocalFog.G, 1.0f );
					LocalFog.B = MinPositiveFloat( LocalFog.B, 1.0f );
					// LocalFog.A = MinPositiveFloat( LocalFog.A, 1.0f );

					// Fog = Fog + LocalFog - (LocalFog*Fog); 
					Fog.R = Fog.R + LocalFog.R - ( LocalFog.R * Fog.R ); 
					Fog.G = Fog.G + LocalFog.G - ( LocalFog.G * Fog.G );
					Fog.B = Fog.B + LocalFog.B - ( LocalFog.B * Fog.B );
					Fog.A = 0.f; // Fog.A + LocalFog.A - ( LocalFog.A * Fog.A );
				}
			}
		}

		DoFemms();
		
		STAT(unclock(GStat.MeshLightTime));
		return Fog; 
	}
	else return FPlane(0,0,0,0);
	unguard;
}

#pragma warning( default : 4799 )
#pragma pack()
#endif

FPlane FLightManager::Light( FTransSample& Vert, DWORD PolyFlags )
{
#ifndef NOAMD3D
	if (GIsK63D)
		return(AMD3DLight(Vert,PolyFlags));
#endif

	guard(FLightManager::Light);
	STAT(clock(GStat.MeshLightTime));

	FPlane Color(0,0,0,0);
	if( !(PolyFlags & PF_Unlit) )
	{
		// Lit.
		STAT(GStat.MeshVertLightCount += LastLight-FirstLight);
		FLOAT PointSquared(Vert.Point.SizeSquared());
		for( FLightInfo* Light=FirstLight; Light<LastLight; Light++ )
		{
			if( Light->Opt != ALO_NotLight )
			{
				// Diffuse lighting.
				FVector LightVector  = Light->Location - Vert.Point;
				FLOAT   LightSquared = LightVector.SizeSquared();
				FLOAT   LightSize    = SqrtApprox( LightSquared );
				FLOAT   G            = Square(1.0 + (LightVector | Vert.Normal) / LightSize) - 1.5;
				if ( G < 0.0)
					G = 0.0;

				// Specular lighting.
				FLOAT Specular = (Light->Location.MirrorByPlane(Vert.Normal) | Vert.Point) - PointSquared;
				if ( Specular > 0.0 )
					G += 6.0 * Square(Specular)/(LightSquared*PointSquared);

				// Radial falloff.
				G *= 1.0 - LightSize * Light->RRadius;

				// Update result color.
				if( G > 0.0 )
					Color += Light->FloatColor * G;
			}
		}
	}
	else Color = FPlane(0.5,0.5,0.5,0);

	// Add ambient light.
	Color.R = Min( Diffuse * Color.R + AmbientVector.R, 1.f );
	Color.G = Min( Diffuse * Color.G + AmbientVector.G, 1.f );
	Color.B = Min( Diffuse * Color.B + AmbientVector.B, 1.f );

	// Editor highlighting.
	if( (PolyFlags & PF_Selected) && GIsEditor )
		Color = Color*0.5 + FVector(0.5,0.5,0.5);

	STAT(unclock(GStat.MeshLightTime));
	return Color;
	unguard;
}



/*

FPlane FLightManager::Fog( FTransSample& Vert, DWORD PolyFlags )
{
	guard(FLightManager::Fog);
	if( PolyFlags & PF_RenderFog )
	{
		STAT(clock(GStat.MeshLightTime));
		FPlane Fog(0,0,0,0);
		for( FLightInfo* LightInfo=FirstLight; LightInfo<LastLight; LightInfo++ )
		{
			if( LightInfo->IsVolumetric )
			{
				FVector LocalFog = LightInfo->FloatColor * Volumetric( LightInfo, Vert.Point )/256.0;
				Fog = Fog + LocalFog - Fog*LocalFog;
			}
		}
		STAT(unclock(GStat.MeshLightTime));
		return Fog*(FVector(2,2,2)-Fog);
	}
	else return FPlane(0,0,0,0);
	unguard;
}
*/


inline FPlane FLightManager::Fog(FTransSample& Vert, DWORD PolyFlags )
{
#ifndef NOAMD3D
	if (GIsK63D)
		return(AMD3DFog(Vert,PolyFlags));
#endif

	guard(FLightManager::Fog);
	if( PolyFlags & PF_RenderFog )
	{
		STAT(clock(GStat.MeshLightTime));
		FPlane Fog(0,0,0,0);

		// First fog light is copied, all next lights are merged.
		// Look for the first volumetric light.

		// Todo: zero/bailout test probably also useful here. Try higher level
		// optims (bounding spheres etc) first !

		for( FLightInfo* LightInfo=FirstLight; LightInfo<LastLight; LightInfo++ )
		{
			if( LightInfo->IsVolumetric ) 
			{
				FPlane LocalFog;
				FLOAT VolumeValue = 2.0f * Volumetric( LightInfo, Vert.Point);
				LocalFog.R = LightInfo->VolumetricColor.R * VolumeValue;
				LocalFog.G = LightInfo->VolumetricColor.G * VolumeValue;
				LocalFog.B = LightInfo->VolumetricColor.B * VolumeValue;
				// LocalFog.A = LightInfo->VolumetricColor.A * VolumeValue;

				Fog.R = MinPositiveFloat( LocalFog.R, 1.0f );
				Fog.G = MinPositiveFloat( LocalFog.G, 1.0f );
				Fog.B = MinPositiveFloat( LocalFog.B, 1.0f );				
				Fog.A = 0.f; // Fog.A = MinPositiveFloat( LocalFog.A, 1.0f );
				break; 
			}		
		}

		// Continue merging.
		// Really should be sorted & Z order dependent !
		while( ++LightInfo < LastLight)
		{
			if( LightInfo->IsVolumetric ) 
			{
				// Todo: Alpha-merging r		
				// Todo: many Volumetric==0 - values go through these calculations unnecessarily.
				// todo: MinPositiveFloat scaling shouldn't be necessary ??

				FPlane LocalFog;
				FLOAT VolumeValue = 2.0f * Volumetric( LightInfo, Vert.Point);
				if (*(DWORD*)&VolumeValue != 0)	// quick zero test 
				{
					LocalFog.R = LightInfo->VolumetricColor.R * VolumeValue;
					LocalFog.G = LightInfo->VolumetricColor.G * VolumeValue;
					LocalFog.B = LightInfo->VolumetricColor.B * VolumeValue;
					// LocalFog.A = LightInfo->VolumetricColor.A * VolumeValue;

					LocalFog.R = MinPositiveFloat( LocalFog.R, 1.0f );
					LocalFog.G = MinPositiveFloat( LocalFog.G, 1.0f );
					LocalFog.B = MinPositiveFloat( LocalFog.B, 1.0f );
					// LocalFog.A = MinPositiveFloat( LocalFog.A, 1.0f );

					// Fog = Fog + LocalFog - (LocalFog*Fog); 
					Fog.R = Fog.R + LocalFog.R - ( LocalFog.R * Fog.R ); 
					Fog.G = Fog.G + LocalFog.G - ( LocalFog.G * Fog.G );
					Fog.B = Fog.B + LocalFog.B - ( LocalFog.B * Fog.B );
					Fog.A = 0.f; // Fog.A + LocalFog.A - ( LocalFog.A * Fog.A );
				}
			}
		}
		
		STAT(unclock(GStat.MeshLightTime));
		return Fog; 
	}
	else return FPlane(0,0,0,0);
	unguard;
}


/*------------------------------------------------------------------------------------
	Light merging.
------------------------------------------------------------------------------------*/

void FLightManager::Merge( FTextureInfo& Tex, BYTE Effect, INT Key, FLightInfo* Info, DWORD* Stream, DWORD* Dest )
{
	guardSlow(FLightManager::Merge);

	static INT Count; 
	FColor* Palette;
    INT Skip;

	// Merge the two streams of light.
	BYTE* Src = Info->IlluminationMap;
	Palette   = Info->Palette;
	Skip      = Info->MinU;
	Count     = Info->MaxU - Info->MinU;

	if( Count<=0 ) return;

	UBOOL FXDetect = ( (Effect==LE_TorchWaver) || (Effect==LE_FireWaver) || (Effect==LE_WateryShimmer) );

	Src    += Info->MinV * Tex.UClamp;
	Dest   += Info->MinV * Tex.USize;
	Stream += Info->MinV * Tex.USize;


	for( INT i=Info->MinV; i<Info->MaxV; i++ )
	{
		BYTE* NewSrc = Src;

		// Execute merge-time effects.
		if( FXDetect )
		{
			BYTE Temp[1024];
			NewSrc = Temp;
			if( Effect==LE_TorchWaver )
			{
				for( INT i=Info->MinU; i<Info->MaxU; i++ )
					Temp[i] = appFloor((FLOAT)Src[i] * (0.95 + 0.05 * GRandoms->RandomBase(Key++)));
			}
			else if( Effect==LE_FireWaver )
			{
				for( INT i=Info->MinU; i<Info->MaxU; i++ )
					Temp[i] = appFloor((FLOAT)Src[i] * (0.80 + 0.20 * GRandoms->RandomBase(Key++)));
			}
			else if( Effect==LE_WateryShimmer )
			{
				for( INT i=Info->MinU; i<Info->MaxU; i++ )
					Temp[i] = appFloor((FLOAT)Src[i] * (0.60 + 0.40 * GRandoms->Random(Key++)));
			}
		}


		// Scale and merge the lighting.
	#if ASM 
		__asm
		{
			// esi = Stream
			// edi = Dest
			// eax = Temp
			// ebx = Palette
			// ecx = Loop counter
			// edx = Temp
			// ebp = Light element
			// esp = NewSrc

			// Setup.
			mov		[SavedESP], esp
			mov		[SavedEBP], ebp
			mov		edx, [Skip]
			mov		esi, [Stream]
			mov		edi, [Dest]
			mov		ebx, [Palette]
			mov		esp, [NewSrc]
			xor		ecx, ecx
			mov     ebp, [Count]
			lea     esi, [esi+edx*4]
			lea     edi, [edi+edx*4]
			lea     esp, [esp+edx]

			// lead-in
			xor		edx, edx
			dec     ebp   // Compensate for leadin/out plus test for 1.
			mov     dl, [esp+ecx]
			jz      LeadOut 
			
			align 16
			// Get scaled light element - 6 cycles
			LightLoop:
			mov		eax, [esi+ecx*4] // Get stream
			mov		edx, [ebx+edx*4] // Get color from palette
			add		eax, edx         // Add stream and color		
			xor     edx, edx
			test	eax, 0x80808080  // Check for saturation
			jnz		LightSaturate    // Fix up after saturation
			mov		dl,[esp+ecx+1]
			mov		[edi+ecx*4], eax // Store result
			inc		ecx
			cmp		ecx, ebp
			jb		LightLoop

			// lead-out
			LeadOut:
			mov		eax, [esi+ecx*4] // Get stream
			mov		edx, [ebx+edx*4] // Get color from palette
			add		eax, edx         // Add stream and color		
			test	eax, 0x80808080  // Check for saturation
			jnz		LastLightSaturate    // Fix up after saturation
			mov		[edi+ecx*4], eax     // Store result
			jmp		LightOut

			align 16
			// Handle saturation - about 9 cycles
			LightSaturate:
			mov		edx,eax
			and		edx,0x80808080
			mov		ebp,edx
			shr		edx,7
			and		eax,0x7F7F7F7F	// mask out all overflowed bits
			sub		ebp,edx			// Creates 7f in each overflowing channel
			xor     edx,edx
			or		eax,ebp			// Set saturated channels
			mov     ebp,[Count]     // reload end indicator
			mov		[edi+ecx*4],eax // Store result
			mov		dl,[esp+ecx+1]
			dec     ebp             // compensate for leadin/out
			inc		ecx
			cmp		ecx, ebp
			jb		LightLoop
			jmp		LeadOut


			align 16
			LastLightSaturate:
			mov		edx,eax
			and		edx,0x80808080
			mov		ebp,edx
			shr     edx,7
			and     eax,0x7f7f7f7f
			sub     ebp,edx
			or      eax,ebp
			mov     [edi+ecx*4],eax

			///
			align 16
			LightOut:
			mov ebp, [SavedEBP]
			mov esp, [SavedESP]
		}		

	#else

		for( INT j=Info->MinU; j<Info->MaxU; j++ )
		{
			Dest[j] = Stream[j] + Palette[NewSrc[j]].D;
			if( Dest[j] & 0x80808080 )
			{
				// Handle saturation.
				DWORD SatMask = Dest[j] & 0x80808080;
				SatMask -= (SatMask >>7);
				Dest[j] = (Dest[j] & 0x7f7f7f7f) | SatMask;
			}
		}

	#endif

		Src    += Tex.UClamp;
		Stream += Tex.USize;
		Dest   += Tex.USize;
	}

	unguardSlow;
}

/*------------------------------------------------------------------------------------
	Spatial effect functions.
------------------------------------------------------------------------------------*/

//
// Convenience macros that give you access to the following parameters easily:
// Info			= FLightInfo pointer
// Vertex		= This point in space
// Location		= Location of light in space
// RRadiusMult	= Inverse radius multiplier
//
#define SPATIAL_PRE \
	STAT(GStat.MeshPtsGen+=Map.UClamp*Map.VClamp); \
	STAT(GStat.MeshesGen++); \
	/* Compute values for stepping through mesh points */ \
	FVector Vertex1 = VertexBase + VertexDV*Info->MinV + VertexDU*Info->MinU; \
	Src  += (ShadowMaskU*8)*Info->MinV + Info->MinU; \
	Dest += Map.UClamp*Info->MinV + Info->MinU; \
	INT USkip = Map.UClamp - (Info->MaxU - Info->MinU); \
	for( INT VCounter=Info->MinV; VCounter<Info->MaxV; VCounter++,Src+=USkip+ShadowSkip,Dest+=USkip ) {

#define SPATIAL_POST \
		Vertex1 += VertexDV; }

#define SPATIAL_BEGIN \
	SPATIAL_PRE \
	FVector Vertex      = Vertex1; \
	FVector Location    = Info->Actor->Location; \
	FLOAT	RRadiusMult = Info->RRadiusMult; \
	FLOAT   Diffuse     = Info->Diffuse; \
	for( INT UCounter=Info->MinU; UCounter<Info->MaxU; UCounter++,Vertex+=VertexDU,Src++,Dest++ ) { \
		if( *Src ) { \
			DWORD SqrtOfs = appRound( FDistSquared(Vertex,Location) * RRadiusMult ); \
			if( SqrtOfs<4096 ) {

#define SPATIAL_BEGIN1 \
	SPATIAL_PRE \
	FVector Vertex = Vertex1 - Info->Actor->Location; \
	FLOAT	RRadiusMult = Info->RRadiusMult; \
	FLOAT   Diffuse     = Info->Diffuse; \
	for( INT UCounter=Info->MinU; UCounter<Info->MaxU; UCounter++,Vertex+=VertexDU,Src++,Dest++ ) { \
		if( *Src ) { \
			DWORD SqrtOfs = appRound( Vertex.SizeSquared() * RRadiusMult ); \
			if( SqrtOfs<4096 ) {

#define SPATIAL_END } else *Dest=0; } else *Dest=0; } SPATIAL_POST

// No effects.
void FLightManager::spatial_None( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_None);
	STAT(GStat.MeshPtsGen+=Map.UClamp*Map.VClamp);
	STAT(GStat.MeshesGen++);

	// Variables.
	static FVector Vertex;
	static FLOAT   Scale, Diffuse;
	static INT     Dist, DistU, DistV, DistUU, DistVV, DistUV;
	static INT     Interp00, Interp10, Interp20, Interp01, Interp11, Interp02;
	static DWORD   Inner0, Inner1;
	static INT     Hecker;

	// Compute values for stepping through mesh points.
#if 0
	FVector Vertex1 = VertexBase - Info->Actor->Location;
	Scale           = Info->RRadiusMult;
	Diffuse         = Info->Diffuse;
	for( INT V=0; V<Map.VClamp; V++,Vertex1+=VertexDV )
	{
		FVector Vertex = Vertex1;
		for( INT U=0; U<Map.UClamp; U++,Vertex+=VertexDU,Src++,Dest++ )
		{
			if( *Src )
			{
				DWORD SqrtOfs = appRound( Vertex.SizeSquared() * Scale );
				if( SqrtOfs<4096 )
				{
					*(FLOAT*)&Hecker = *Src * Diffuse * LightSqrt[SqrtOfs] + (2<<22);
					*Dest = Hecker;
				}
				else *Dest = 0;
			}
			else *Dest = 0;
		}
	}
#else
	Vertex   = VertexBase - Info->Actor->Location + VertexDU*Info->MinU + VertexDV*Info->MinV;
	Diffuse  = Info->Diffuse;
	Scale    = Info->RRadiusMult * 4096.0;
	Dist     = appRound((Vertex   | Vertex  ) * Scale);
	DistU    = appRound((Vertex   | VertexDU) * Scale);
	DistV    = appRound((Vertex   | VertexDV) * Scale);
	DistUU   = appRound((VertexDU | VertexDU) * Scale);
	DistVV   = appRound((VertexDV | VertexDV) * Scale);
	DistUV   = appRound((VertexDU | VertexDV) * Scale);

	Interp00 = Dist;
	Interp10 = 2 * DistV + DistVV;
	Interp20 = 2 * DistVV;
	Interp01 = 2 * DistU + DistUU;
	Interp11 = 2 * DistUV;
	Interp02 = 2 * DistUU;

	Src  += (ShadowMaskU*8) * Info->MinV + Info->MinU;
	Dest += Map.UClamp * Info->MinV + Info->MinU;
	INT USkip = Map.UClamp - (Info->MaxU-Info->MinU);
	for( INT VCounter=Info->MinV; VCounter<Info->MaxV; VCounter++ )
	{
		// Forward difference the square of the distance between the points.
		Inner0 = Interp00;
		Inner1 = Interp01;
		for( INT U=Info->MinU; U<Info->MaxU; U++ )
		{
			if( *Src!=0 && Inner0<4096*4096 ) 
			{
				*(FLOAT*)&Hecker = *Src * Diffuse * LightSqrt[Inner0>>12] + (2<<22);
				*Dest = Hecker;
			}
			else *Dest = 0;
			Src++;
			Dest++;
			Inner0 += Inner1;
			Inner1 += Interp02;
		}
		Interp00 += Interp10;
		Interp10 += Interp20;
		Interp01 += Interp11;
		Src  += USkip+ShadowSkip;
		Dest += USkip;
	}
#endif
	unguardSlow;
}

inline FLOAT FLightManager::Volumetric( FLightInfo* Info, FVector &Vertex )
{
	// Optimize: there's 2 sqrtapproxes and 2 divides -   too many?
	//
    // d  = (square of) distance of shortest line from viewer-to-surface-line to light location.
	// c1 = distance along viewer-to-surface line to surface, relative to nearest point to light location.
	// c2 = distance along viewer-to-surface line to viewer, relative to nearest point to light location.
	// F  = fog line-integral.
	//
	// FLOAT VertexSize = SqrtApprox(Vertex.SizeSquared());	// Distance eye-to-vertex.
	//
	static int FogRejectionMethod = 0;

	FLOAT c1, c2, d, F, h, c0, S, S2; 
	
	S  = ( Info->Location | Vertex ); // 3 fmuls 2 fadds 
	
	if (! Info->VolInside )
	{
		// No fog if negative dotproduct Light*Vertex AND we're not inside the sphere.
		if (IsNegativeFloat(S)) return 0.f;
	}

	// Determine distance to the vertex.
	FLOAT VertexSizeSquared = Vertex.SizeSquared(); // 3 fmuls
	S2  = S*S; 

	// Whenever the stricter 'd'-check rejected a vertex, we'll try to revert to this one next time.
	// 2 fmuls.
	if ( FogRejectionMethod )
		// equivalent to    if( Info->VolRadiusSquared < d )
		if ( (VertexSizeSquared * Info->VolRadiusSquared) < (VertexSizeSquared * Info->LocationSizeSquared - S2) )	
		{
			return 0.0f;
		}

	// d>=0, sqrt(fabs(d))= Distance of shortest line from viewer-to-surface-line to light location.
	d  = Info->LocationSizeSquared - S2/VertexSizeSquared;   

	// Viewer-to-surface line does not intersect light's sphere.
	if( Info->VolRadiusSquared < d )	// Out distance squared < d 
	{
		FogRejectionMethod = 1; // primed to do cheaper radius checks.
		return 0.0f;
	}

	FogRejectionMethod = 0; //radius checks failed, probably next time also.

	// ERIK+: Maybe it's worth having a lookup table which simultaneously
	// returns sqrt(x) and 1.0/sqrt(x), to avoid the S/VertexSize divide. -Tim

	FLOAT VertexSize = SqrtApprox(VertexSizeSquared);	
	c0 = S/VertexSize;		// c2 <= Info.Location.Size().

	// Compute c1 and c2 from line clipped to interior of sphere.
	h  = SqrtApprox( Info->VolRadiusSquared - d );

	int FullSphere = 0;

	c1 = c0 - VertexSize;   

	if (c0 > h)
	{ 
		c2 = h;
		if (c1 < -h)
		{
			c1 = -h;        // special-case: c1==-h, c2==h, only one integral needed.
			FullSphere = 1;
		}
	}
	else
	{
		c2 = c0;
		if (c1 < -h)
		{
			c1 = -h;
		}	
	}
	
	if (c1 >= c2) return 0.0f; // point totally outside sphere

	FLOAT D2 = d  * Info->RVolRadiusSquared; // real distance from center -> normalized.
	c2 = c2 * Info->RVolRadius;	// scaled relative to volume radius.

	if (FullSphere) // c1== -c2
	{
		FLOAT I = (3-3*D2);
		F =  2.0f * Info->VolBrightness * ( c2*(I - c2*c2) );

		// F = 2.0f * Bri * ( c2 * Rad  ( (3-3*d*rad*rad)  - c2 c2 rad rad ) );
	}
	else
	{
		c1 = c1  * Info->RVolRadius;  
		FLOAT I = (3-3*D2);
		F =  Info->VolBrightness *( (c2*(I - c2*c2 ) ) - ( c1*(I - c1*c1 ) ) );
	}

	if (F < 0.f) return 0.f; //#debug superfluous check, with the new lighting code ?!!
	return MinPositiveFloat( F, 1.f );       	
}

// Yawing searchlight effect.
void FLightManager::spatial_SearchLight( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_SearchLight);
	FLOAT Offset
	=	(2.0 * PI)
	+	(Info->Actor->LightPhase * (8.0 * PI / 256.0))
	+	(Info->Actor->LightPeriod ? 35.0 * LevelInfo->TimeSeconds / Info->Actor->LightPeriod : 0);
	SPATIAL_BEGIN1
		FLOAT Angle = appFmod( Offset + 4.0 * appAtan2( Vertex.X, Vertex .Y), 8.*PI );
		if( Angle<PI || Angle>PI*3.0 )
		{
			*Dest = 0.0;
		}
		else
		{
			FLOAT Scale = 0.5 + 0.5 * GMath.CosFloat(Angle);
			FLOAT D     = 0.00006 * (Square(Vertex.X) + Square(Vertex.Y));
			if( D < 1.0 )
				Scale *= D;
			*Dest = appFloor(*Src * Scale * Diffuse * LightSqrt[SqrtOfs]);
		}
	SPATIAL_END
	unguardSlow;
}

// Yawing rotor effect.
void FLightManager::spatial_Rotor( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Rotor);
	SPATIAL_BEGIN1
		FLOAT Angle = 6.0 * appAtan2(Vertex.X,Vertex.Y);
		FLOAT Scale = 0.5 + 0.5 * GMath.CosFloat(Angle + LevelInfo->TimeSeconds*3.5);
		FLOAT D     = 0.0001 * (Square(Vertex.X) + Square(Vertex.Y));
		if (D<1.0) Scale = 1.0 - D + Scale * D;
		*Dest		= appFloor(*Src * Scale * Diffuse * LightSqrt[SqrtOfs]);
	SPATIAL_END
	unguardSlow;
}

// Slow radial waves.
void FLightManager::spatial_SlowWave( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_SlowWave);
	SPATIAL_BEGIN1
		FLOAT Scale	= 0.7 + 0.3 * GMath.SinTab(((int)SqrtApprox(Vertex.SizeSquared()) - LevelInfo->TimeSeconds*35.0) * 1024.0);
		*Dest		= appFloor(*Src * Scale * Diffuse * LightSqrt[SqrtOfs]);
	SPATIAL_END
	unguardSlow;
}

// Fast radial waves.
void FLightManager::spatial_FastWave( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_FastWave);
	SPATIAL_BEGIN1
		FLOAT Scale	= 0.7 + 0.3 * GMath.SinTab((((int)SqrtApprox(Vertex.SizeSquared())>>2) - LevelInfo->TimeSeconds*35.0) * 2048.0);
		*Dest		= appFloor(*Src * Scale * Diffuse * LightSqrt[SqrtOfs]);
	SPATIAL_END
	unguardSlow;
}

// Scrolling clouds.
void FLightManager::spatial_CloudCast( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_CloudCast);
	//if( !LevelInfo->CloudcastTexture )
	{
		spatial_None( Map, Info, Src, Dest );
		return;
	}
	/*
	BYTE*	Data	= LevelInfo->CloudcastTexture->GetMip(0)->DataPtr;
	BYTE	VShift	= LevelInfo->CloudcastTexture->UBits;
	int		UMask	= LevelInfo->CloudcastTexture->USize-1;
	int		VMask	= LevelInfo->CloudcastTexture->VSize-1;
	int		UPan	= 256.0 * 40.0 * LevelInfo->TimeSeconds;
	int		VPan	= 256.0 * 40.0 * LevelInfo->TimeSeconds;

	// optimize: Convert to assembly and optimize reasonably well. This routine is often used
	// for large outdoors areas.
	SPATIAL_PRE
		FVector Vertex = Vertex1;
		for( int i=0; i<Map.UClamp; i++,Vertex+=VertexDU,Src++,Dest++ )
		{
			*Dest = 0.0;
			if( *Src != 0.0 )
			{
				DWORD SqrtOfs = appRound( FDistSquared(Vertex,Info->Actor->Location) * Info->RRadiusMult );
				if( SqrtOfs<4096 )
				{
					int		FixU	= appRound((Vertex.X+Vertex.Z) * (256.0 / 12.0)) + UPan;
					int		FixV	= appRound((Vertex.Y+Vertex.Z) * (256.0 / 12.0)) + VPan;
					int		U0		= (FixU >> 8) & UMask;
					int		U1		= (U0    + 1) & UMask;
					int		V0		= (FixV >> 8) & VMask;
					int		V1		= (V0    + 1) & VMask;

					FLOAT	Alpha1	= FixU & 255;
					FLOAT	Beta1	= FixV & 255;
					FLOAT	Alpha2	= 256.0 - Alpha1;
					FLOAT	Beta2	= 256.0 - Beta1;

					*Dest = Clamp(appFloor(*Src * 3.0 * LightSqrt[SqrtOfs] * Info->Diffuse *
					(
						Data[U0 + (V0<<VShift)] * Alpha2 * Beta2 +
						Data[U1 + (V0<<VShift)] * Alpha1 * Beta2 +
						Data[U0 + (V1<<VShift)] * Alpha2 * Beta1 +
						Data[U1 + (V1<<VShift)] * Alpha1 * Beta1
					) / (256.0 * 256.0 * 256.0)), 0, 255 );
				}
			}
		}
	SPATIAL_POST*/
	unguardSlow;
}

// Shock wave.
void FLightManager::spatial_Shock( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Shock);
	SPATIAL_BEGIN1
		int Dist = 8.0 * SqrtApprox(Vertex.SizeSquared());
		FLOAT Brightness  = 0.9 + 0.1 * GMath.SinTab(((Dist<<1) - (LevelInfo->TimeSeconds * 4000.0))*16.0);
		Brightness       *= 0.9 + 0.1 * GMath.CosTab(((Dist   ) + (LevelInfo->TimeSeconds * 4000.0))*16.0);
		Brightness       *= 0.9 + 0.1 * GMath.SinTab(((Dist>>1) - (LevelInfo->TimeSeconds * 4000.0))*16.0);
		*Dest = appFloor(*Src * Diffuse * LightSqrt[SqrtOfs] * Brightness);
	SPATIAL_END
	unguardSlow;
}

// Disco ball.
void FLightManager::spatial_Disco( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Disco);
	SPATIAL_BEGIN1
		FLOAT Yaw	= 11.0 * appAtan2(Vertex.X,Vertex.Y);
		FLOAT Pitch = 11.0 * appAtan2(SqrtApprox(Square(Vertex.X)+Square(Vertex.Y)),Vertex.Z);

		FLOAT Scale1 = 0.50 + 0.50 * GMath.CosFloat(Yaw   + LevelInfo->TimeSeconds*5.0);
		FLOAT Scale2 = 0.50 + 0.50 * GMath.CosFloat(Pitch + LevelInfo->TimeSeconds*5.0);

		FLOAT Scale  = Scale1 + Scale2 - Scale1 * Scale2;

		FLOAT D = 0.00005 * (Square(Vertex.X) + Square(Vertex.Y));
		if (D<1.0) Scale *= D;

		*Dest = appFloor(*Src * (1.0-Scale) * Diffuse * LightSqrt[SqrtOfs]);
	SPATIAL_END
	unguardSlow;
}

// Cylinder lightsource.
void FLightManager::spatial_Cylinder( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Cylinder);
	SPATIAL_PRE
		FVector Vertex = Vertex1 - Info->Actor->Location;
		for( INT i=Info->MinU; i<Info->MaxU; i++,Vertex+=VertexDU,Src++,Dest++ )
			*Dest = Max(0,appFloor(*Src * (1.0 - ( Square(Vertex.X) + Square(Vertex.Y) ) * Square(Info->RRadius)) ));
	SPATIAL_POST
	unguardSlow;
}

// Interference pattern.
void FLightManager::spatial_Interference( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Interference);
	SPATIAL_BEGIN1
		FLOAT Pitch = 11.0 * appAtan2(SqrtApprox(Square(Vertex.X)+Square(Vertex.Y)),Vertex.Z);
		FLOAT Scale = 0.50 + 0.50 * GMath.CosFloat(Pitch + LevelInfo->TimeSeconds*5.0);
		*Dest = appFloor(*Src * Scale * Diffuse * LightSqrt[SqrtOfs]);
	SPATIAL_END
	unguardSlow;
}

// Spotlight lighting.
void FLightManager::spatial_Spotlight( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Spotlight);
	FVector View      = Info->Actor->GetViewRotation().Vector();
	FLOAT   Sine      = 1.0 - Info->Actor->LightCone / 256.0;
	FLOAT   RSine     = 1.0 / (1.0 - Sine);
	FLOAT   SineRSine = Sine * RSine;
	FLOAT   SineSq    = Sine * Sine;
	SPATIAL_BEGIN1
		FLOAT SizeSq = Vertex | Vertex;
		FLOAT VDotV  = Vertex | View;
		if( VDotV > 0.0 && Square(VDotV) > SineSq * SizeSq )
		{
			FLOAT Dot = Square( VDotV * RSine * DivSqrtApprox(SizeSq) - SineRSine );
			*Dest = appFloor(Dot * *Src * Diffuse * LightSqrt[SqrtOfs]);
		}
		else *Dest = 0;
	SPATIAL_END
	unguardSlow;
}

// Spatial routine for testing.
void FLightManager::spatial_Test( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Test);
	SPATIAL_BEGIN1
		*Dest = 0;
	SPATIAL_END
	unguardSlow;
}

// Amorphous lighting.
void FLightManager::spatial_NonIncidence( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_NonIncidence);
	SPATIAL_BEGIN1
		*Dest = appFloor( *Src * (1.02 - SqrtApprox(Vertex.SizeSquared()) * Info->RRadius) );
	SPATIAL_END
	unguardSlow;
}

// Shell lighting.
void FLightManager::spatial_Shell( FTextureInfo& Map, FLightInfo* Info, BYTE* Src, BYTE* Dest )
{
	guardSlow(FLightManager::spatial_Shell);
	SPATIAL_BEGIN1
		FLOAT Dist = SqrtApprox(Vertex.SizeSquared()) * Info->RRadius;
		if( Dist >= 1.0 || Dist <= 0.8 )
			*Dest = 0;
		else
			*Dest = appFloor( *Src * (1.0 - 10.0*Abs(Dist-0.9)) );
	SPATIAL_END
	unguardSlow;
}

/*------------------------------------------------------------------------------------
	Global light effects.
------------------------------------------------------------------------------------*/

// No global lighting
static void global_None( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_None);
	Brightness=0.0;
	unguardSlow;
}

// Steady global lighting
static void global_Steady( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_Steady);
	unguardSlow;
}

// Global light pulsing effect
static void global_Pulse( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_Pulse);
	Brightness *= 0.6 + 0.39 * GMath.SinTab
	(
		(Owner->Level->TimeSeconds * 35.0 * 65536.0) / Max((int)Owner->LightPeriod,1) + (Owner->LightPhase << 8)
	);
	unguardSlow;
}

// Global blinking effect
static void global_Blink( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_Blink);
	if( (int)((Owner->Level->TimeSeconds * 35.0 * 65536.0)/(Owner->LightPeriod+1) + (Owner->LightPhase << 8)) & 1 )
		Brightness = 0.0;
	unguardSlow;
}

// Global flicker effect
static void global_Flicker( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_Flicker);

	FLOAT Random = GRandoms->RandomBase((int)Owner);
	if( Random < 0.5 )	Brightness = 0.0;
	else				Brightness *= Random;
	unguardSlow;
}

// Strobe light.
static void global_Strobe( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_Strobe);
	static float LastUpdateTime=0; static int Toggle=0;
	if( LastUpdateTime != Owner->Level->TimeSeconds )
	{
		LastUpdateTime = Owner->Level->TimeSeconds;
		Toggle ^= 1;
	}
	if( Toggle ) Brightness = 0.0;
	unguardSlow;
}

// Simulated light emmanating from the backdrop.
static void global_BackdropLight( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_BackdropLight);
	unguardSlow;
}

// Global subtle light pulsing effect
static void global_SubtlePulse( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_Pulse);
	Brightness *= 0.9 + 0.09 * GMath.SinTab
	(
		(Owner->Level->TimeSeconds * 35.0 * 65536.0) / Max((int)Owner->LightPeriod,1) + (Owner->LightPhase << 8)
	);
	unguardSlow;
}

// Use texture palette with LifeSpan to indicate index.
static void global_TexturePaletteOnce( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_TexturePaletteOnce);
	if( Owner->Skin && Owner->Skin->Palette )
	{
		FColor C = Owner->Skin->Palette->Colors(appFloor(255.0 * Owner->LifeFraction()));
		Color = FVector( C.R, C.G, C.B ).SafeNormal();
		Brightness *= C.FBrightness() * 2.8;
	}
	unguardSlow;
}

// Use texture palette, looping over and over.
static void global_TexturePaletteLoop( AActor* Owner, FLOAT& Brightness, FVector& Color )
{
	guardSlow(global_TexturePaletteLoop);
	if( Owner->Skin && Owner->Skin->Palette )
	{
		FLOAT Time        = Owner->Level->TimeSeconds * 35 / Max((int)Owner->LightPeriod,1) + Owner->LightPhase;
		FColor C          = Owner->Skin->Palette->Colors(((int)(Time*256) & 255) % 255);
		Color             = FVector( C.R, C.G, C.B ).UnsafeNormal();
		Brightness       *= C.FBrightness() * 2.8;
	}
	unguardSlow;
}

// Table of global lighting functions.
typedef void (*LIGHT_TYPE_FUNC)( AActor* Owner, FLOAT& Brightness, FVector& Color );
static const LIGHT_TYPE_FUNC GLightTypeFuncs[LT_MAX] =
{
	global_None,
	global_Steady,
	global_Pulse,
	global_Blink,
	global_Flicker,
	global_Strobe,
	global_BackdropLight,
	global_SubtlePulse,
	global_TexturePaletteOnce,
	global_TexturePaletteLoop
};

// Compute global lighting for an actor.
void URender::GlobalLighting( UBOOL Realtime, AActor* Owner, FLOAT& Brightness, FPlane& Color )
{
	guard(URender::GlobalLighting);

	// Figure out global dynamic lighting effect.
	ELightType Type = (ELightType)Owner->LightType;
	if( !Realtime )
		Type = LT_Steady;

	// Coloring.
	Color = FGetHSV( Owner->LightHue, Owner->LightSaturation, 255 );

	// Only compute global lighting effect once per actor per frame, so that
	// lights with random functions produce consistent lighting on all surfaces they hit!!
	if( Type<LT_MAX )
		GLightTypeFuncs[Type]( Owner, Brightness, Color );
	Brightness = Clamp( Brightness, 0.f, 1.f );

	unguard;
}

/*------------------------------------------------------------------------------------
	Implementation of FLightInfo class
------------------------------------------------------------------------------------*/

//
// Compute lighting information based on an actor lightsource.
//
void FLightManager::FLightInfo::ComputeFromActor( FTextureInfo& Map, FSceneNode* Frame, UBOOL Surface )
{
	guard(FLightManager::FLightInfo::ComputeFromActor);

	// Compute coords.
	if( MapCoords && !TemporaryTablesBuilt )
	{
		TemporaryTablesBuilt = 1;
		MapUncoords = MapCoords->Inverse().Transpose();
		VertexBase  = MapCoords->Origin + MapUncoords.XAxis*Map.Pan.X + MapUncoords.YAxis*Map.Pan.Y;
		VertexDU    = MapUncoords.XAxis * Map.UScale;
		VertexDV    = MapUncoords.YAxis * Map.VScale;
	}

	// Setupstuff.
	Radius			= Actor->WorldLightRadius();
	RRadius			= 1.0/Max((FLOAT)1.0,Radius);
	RRadiusMult		= 4093.0 * RRadius * RRadius;
	Location		= Actor->Location.TransformPointBy( Frame->Coords );
	Brightness      = Actor->LightBrightness/255.f;
	Effect          = Effects[(Actor->LightEffect<LE_MAX) ? Actor->LightEffect : 0];
	GRender->GlobalLighting( (Frame->Viewport->Actor->ShowFlags&SHOW_PlayerCtrl)!=0, Actor, Brightness, FloatColor );

	// Other precomputed info.
	if( MapCoords )
		Diffuse = Abs(((Actor->Location-MapCoords->Origin) | MapCoords->ZAxis) * RRadius);

	// Adjust color.
	FloatColor *= Brightness * Actor->Level->Brightness;

	// Surface setup.
	if( Surface )
	{
		// Cache the scaler palette.
		clock(GStat.ExtraTime);
		QWORD CacheID = MakeCacheID( CID_LightPalette, Actor );
		FVector* Color = (FVector*)GCache.Get(CacheID,*TopItemToUnlock++);
		if( !Color || *Color!=FloatColor || Actor->bLightChanged )
		{
			// Create or replace the palette.
			if( !Color )
				Color = (FVector*)GCache.Create(CacheID,TopItemToUnlock[-1],sizeof(FVector)+256*sizeof(FColor));
			*Color = FloatColor;
			Palette = (FColor*)(Color+1);
			INT FixR = 0; INT FixDR=appFloor(FloatColor.R*65536.0);
			INT FixG = 0; INT FixDG=appFloor(FloatColor.G*65536.0);
			INT FixB = 0; INT FixDB=appFloor(FloatColor.B*65536.0);
			for( INT i=0; i<256; i++ )
			{
				Palette[i].B = Min(Unfix(FixR),127); FixR+=FixDR;
				Palette[i].G = Min(Unfix(FixG),127); FixG+=FixDG;
				Palette[i].R = Min(Unfix(FixB),127); FixB+=FixDB;
			}
		}
		Palette = (FColor*)(Color+1);
		unclock(GStat.ExtraTime);

		// Compute clipping region.
		FLOAT   PlaneDot    = (Actor->Location - MapCoords->Origin) | MapCoords->ZAxis;
		FLOAT   Radius      = Actor->WorldLightRadius();
		FLOAT   PlaneRadius = SqrtApprox( Max( Radius*Radius*1.05 - PlaneDot*PlaneDot, 0.0 ) );
		FVector Center      = Actor->Location - MapCoords->ZAxis*PlaneDot;
		FLOAT   CenterU     = ((Center - MapCoords->Origin) | MapCoords->XAxis) - LightMap.Pan.X;
		FLOAT   CenterV     = ((Center - MapCoords->Origin) | MapCoords->YAxis) - LightMap.Pan.Y;
		FLOAT   RadiusU     = PlaneRadius * SqrtApprox(MapCoords->XAxis.SizeSquared());
		FLOAT   RadiusV     = PlaneRadius * SqrtApprox(MapCoords->YAxis.SizeSquared());

		// Save clipping region.
		MinU = Max( appRound( (CenterU - RadiusU)/LightMap.UScale), 0 );
		MinV = Max( appRound( (CenterV - RadiusV)/LightMap.VScale), 0 );
		MaxU = Min( appRound( (CenterU + RadiusU)/LightMap.UScale), LightMap.UClamp );//!!
		MaxV = Min( appRound( (CenterV + RadiusV)/LightMap.VScale), LightMap.VClamp );
	}

	// Init volumetric lighting.
	if( IsVolumetric )
	{		
		VolumetricColor   = FloatColor;   
		//VolumetricColor.A = (FLOAT)Actor->VolumeFog * (1.f/255.f);

		// Cache the volumetric color scaler palette
		clock(GStat.ExtraTime);
		QWORD CacheID = MakeCacheID( CID_Extra2, Actor );
		FPlane* Color = (FPlane*)GCache.Get(CacheID,*TopItemToUnlock++);
		if( !Color || *Color!=VolumetricColor || Actor->bLightChanged )
		{
			// Create or replace the palette.
			if( !Color )
				Color = (FPlane*)GCache.Create(CacheID,TopItemToUnlock[-1],sizeof(FPlane)+256*sizeof(FColor));
			*Color = VolumetricColor;
			VolPalette = (FColor*)(Color+1);
			INT FixR = 0; INT FixDR=appFloor(VolumetricColor.R*65536.0);
			INT FixG = 0; INT FixDG=appFloor(VolumetricColor.G*65536.0);
			INT FixB = 0; INT FixDB=appFloor(VolumetricColor.B*65536.0);
			//INT FixA = 0; INT FixDA=appFloor(VolumetricColor.A*65536.0);
			for( INT i=0; i<256; i++ )
			{
				VolPalette[i].B = Min(Unfix(FixR),127); FixR+=FixDR;
				VolPalette[i].G = Min(Unfix(FixG),127); FixG+=FixDG;
				VolPalette[i].R = Min(Unfix(FixB),127); FixB+=FixDB;
				//VolPalette[i].A = Min(Unfix(FixA),127); FixA+=FixDA;
			}
		}
		VolPalette = (FColor*)(Color+1);
		unclock(GStat.ExtraTime);

		VolRadius			= Actor->WorldVolumetricRadius();
		VolRadiusSquared	= VolRadius * VolRadius;
		RVolRadius          = 1.0f/VolRadius;
		RVolRadiusSquared   = RVolRadius * RVolRadius;
		LocationSizeSquared = Location.SizeSquared();
		VolBrightness		= Brightness * Actor->VolumeBrightness / 64.0f;
		VolInside           = (LocationSizeSquared < VolRadiusSquared);
	}
	unguard;
}

/*------------------------------------------------------------------------------------
	Implementation of FLightList class
------------------------------------------------------------------------------------*/

//
// Compute fast light list for a surface.
//
void FLightManager::SetupForSurf
(
	FSceneNode*		InFrame,
	FCoords&		InMapCoords,
	FBspDrawList*	Draw,
	FTextureInfo*&	OutLightMap,
	FTextureInfo*&	OutFogMap,
	FTextureInfo*	BumpMap,
	UBOOL			Merged
)
{
	guard(FLightManager::SetupForSurf);
	STAT(clock(GStat.IllumTime));
	INT Key=0;

#if 0
	// To regenerate all lighting every frame, uncomment this.
	guard(FlushAll);
	GCache.Flush(MakeCacheID(CID_StaticMap      ,0,0),MakeCacheID(CID_MAX,0,0,NULL));
	GCache.Flush(MakeCacheID(CID_DynamicMap     ,0,0),MakeCacheID(CID_MAX,0,0,NULL));
	GCache.Flush(MakeCacheID(CID_ShadowMap      ,0,0),MakeCacheID(CID_MAX,0,0,NULL));
	GCache.Flush(MakeCacheID(CID_IlluminationMap,0,0),MakeCacheID(CID_MAX,0,0,NULL));
	unguard;
#endif

	// Init mip pointer.
	LightMap.TextureFlags	= 0;
	LightMap.NumMips		= 1;
	LightMap.Mips[0]		= &LightMip;
	LightMap.Format			= TEXF_RGB32;
	LightMap.Palette		= NULL;
	LightMap.Mips[0]->DataPtr = NULL;

	// Fog.
	FogMap.TextureFlags		= 0;
	FogMap.NumMips			= 1;
	FogMap.Mips[0]			= &FogMip;
	FogMap.Format			= TEXF_RGB32;
	FogMap.Palette			= NULL;
	FogMap.Mips[0]->DataPtr = NULL;

	// Set up variables.
	Mark                    = FMemMark(GMem);
	Frame					= InFrame;
	Level					= Frame->Level;
	INT iLightMap	        = Level->Model->Surfs->Element(Draw->iSurf).iLightMap;
	LevelInfo				= Level->GetLevelInfo();
	Zone					= NULL;
	TemporaryTablesBuilt	= 0;	
	AMover* Mover           = (Draw->iSurf < Level->Model->Surfs->Num()) ? NULL : (AMover*)Level->Model->Surfs->Element(Draw->iSurf).Actor;
	UModel* Model			= Mover ? Mover->Brush : Level->Model;
	FLightMapIndex* Index	= &Model->LightMap(iLightMap);
	Zone					= Draw->Zone;
	BYTE ZoneID				= Zone ? Zone->Region.ZoneNumber : 255;
	LastLight				= FirstLight;
	StaticLights			= 0;
	DynamicLights			= 0;
	MovingLights			= 0;
	StaticLightingChanged	= 0;
	MapCoords				= &InMapCoords;
	ShadowMaskU				= (Index->UClamp+7) >> 3;
	ShadowMaskSpace			= ShadowMaskU * Index->VClamp;
	ShadowSkip              = ShadowMaskU*8 - Index->UClamp;
	OutLightMap             = &LightMap;

	/*if( !Mover || !Mover->bDynamicLightMover )
	{
		guard(SetupNormalSurface);
		Mover = NULL;

		// Static lights.
		BYTE* ShadowBase = &Model->LightBits(Index->DataOffset);
		if( Index->iLightActors != INDEX_NONE )
			for( INT i=0; Model->Lights(i+Index->iLightActors); i++,ShadowBase+=ShadowMaskSpace )
				if( AddLight( Mover, Model->Lights(i+Index->iLightActors) ) )
					LastLight[-1].ShadowBits = ShadowBase;

		// Dynamic lights.
		for( FActorLink* Link=Draw->SurfLights; Link; Link=Link->Next )
			AddLight( Mover, Link->Actor );

		unguard;
	}*/
	if( !Mover || !Mover->bDynamicLightMover )
	{
		guard(SetupNormalSurface);
		Mover = NULL;

		// Static lights.
		BYTE* ShadowBase = &Model->LightBits(Index->DataOffset);
		if( Index->iLightActors != INDEX_NONE )
			for( INT i=0; Model->Lights(i+Index->iLightActors); i++,ShadowBase+=ShadowMaskSpace )
				if( AddLight( Mover, Model->Lights(i+Index->iLightActors) ) )
					LastLight[-1].ShadowBits = ShadowBase;

		// Dynamic lights.
		for( FActorLink* Link=Draw->SurfLights; Link; Link=Link->Next )
			AddLight( Mover, Link->Actor );

		unguard;
	}
	else if( Mover->Region.iLeaf!=INDEX_NONE && Level->Model->Leaves.Num() )
	{
		guard(SetupMoverSurface);

		// Static mover lights.
		FLeaf& Leaf = Level->Model->Leaves(Mover->Region.iLeaf);
		if( Leaf.iPermeating!=INDEX_NONE )
			for( INT i=Leaf.iPermeating; Level->Model->Lights(i); i++ )
				if( ((Level->Model->Lights(i)->Location - MapCoords->Origin)|MapCoords->ZAxis) > 0.0 && Level->Model->Lights(i)->bSpecialLit==Mover->bSpecialLit )
					AddLight( NULL, Level->Model->Lights(i) );

		// Dynamic mover lights.
		for( FActorLink* Link=Draw->SurfLights; Link; Link=Link->Next )
			if( Link->Actor->bSpecialLit==Mover->bSpecialLit )
				AddLight( Mover, Link->Actor );

		// Volumetric mover lights.
		for( FActorLink* Volumetrics=Draw->Volumetrics; Volumetrics && LastLight<FinalLight; Volumetrics=Volumetrics->Next )
		{
			for( FLightInfo* Find=FirstLight; Find<LastLight && Find->Actor!=Volumetrics->Actor; Find++ );
			Find->IsVolumetric = 1;
			if( Find==LastLight )
			{
				// New volumetric light.
				LastLight->Actor = Volumetrics->Actor;
				LastLight->Opt   = ALO_NotLight;
				LastLight++;
			}
		}
		unguard;
	}

#if 0
	// Omnidirectional bumpmap lighting.
	if( BumpMap )
	{
		guard(SetupBumpMap);

		// Prevent caching of palettes.
		GCache.Flush( CID_TriPalette, 0xff );
		BumpMap->MaxColor = FColor(255,255,255,255);
		BumpMap->TextureFlags |= TF_RealtimePalette;

		// Get transformed bump map normals.
		QWORD CacheID = MakeCacheID( CID_BumpNormals, Draw->iSurf, 0, 0, Model );
		FCacheItem* Item;
		FVector* BumpNormals = (FVector*)GCache.Get( CacheID, Item );
		if( !BumpNormals )
		{
			FCoords Uncoords = MapCoords->Inverse();
			BumpNormals = (FVector*)GCache.Create( CacheID, Item, 256 * sizeof(FVector) );
			FColor* Colors = BumpMap->Palette;
			for( INT i=0; i<256; i++ )
				BumpNormals[i] = FVector
				( 
					(128-Colors[i].B)/128.0,
					(128-Colors[i].R)/128.0,
					(Colors[i].G-128)/128.0
				).TransformVectorBy( Uncoords );
		}

		// Compute resulting colors.
		FVector C[256];
		for( INT i=0; i<256; i++ )
			C[i] = FVector(0,0,0);
		for( FLightInfo* Light=FirstLight; Light<LastLight; Light++ )
		{
			if( Light->Actor->LightEffect == LE_OmniBumpMap )
			{
				Light->ComputeFromActor( LightMap, Frame, 1 );
				FVector LightDir = Light->Actor->Rotation.Vector();
				for( i=0; i<256; i++ )
				{
					FLOAT Dot = BumpNormals[i] | LightDir;
					Dot = 0.5*Max(Dot,0.f);
					C[i] += Dot * Light->FloatColor;
				}
			}
		}
		BumpMap->Palette = New<FColor>(GMem,256);
		for( i=0; i<256; i++ )
			BumpMap->Palette[i] = FColor(C[i]);
		Item->Unlock();
		unguard;
	}
#endif

	// Volumetric lights.
	if( Zone && Zone->bFogZone && Draw->Volumetrics )
	{
		guard(SetupVolumetrics);
		OutFogMap = &FogMap;
		for( FActorLink* Link=Draw->Volumetrics; Link && LastLight<FinalLight; Link=Link->Next )
		{
			// See if volumetric light is already on the list as a regular light.
			for( FLightInfo* Find=FirstLight; Find<LastLight && Find->Actor!=Link->Actor; Find++ );
			Find->IsVolumetric = 1;
			if( Find==LastLight )
			{
				// New volumetric light.
				LastLight->Actor     = Link->Actor;
				LastLight->Opt       = ALO_NotLight;
				LastLight++;
			}
		}

		// Setup FogMip and FogMap.
		FogMip.UBits			= FLogTwo(Index->UClamp);
		FogMip.VBits			= FLogTwo(Index->VClamp);
		FogMip.USize			= 1 << FogMip.UBits;
		FogMip.VSize			= 1 << FogMip.VBits;
		FogMap.Pan				= Index->Pan;
		FogMap.UScale			= Index->UScale;
		FogMap.VScale			= Index->VScale;
		FogMap.UClamp			= Index->UClamp;
		FogMap.VClamp			= Index->VClamp;
		FogMap.USize			= FogMip.USize;
		FogMap.VSize			= FogMip.VSize;
		FogMap.TextureFlags		= TF_RealtimeChanged;
		FogMap.CacheID			= MakeCacheID( CID_RenderFogMap, iLightMap, ZoneID, Model );

		// Setup the volumetrics.
		FogMip.DataPtr = New<BYTE>(GMem,FogMap.USize*FogMap.VSize*sizeof(DWORD)+sizeof(FColor));
		FogMap.MaxColor = (FColor*)FogMip.DataPtr; FogMip.DataPtr += sizeof(FColor);
		*FogMap.MaxColor = FColor(255,255,255,255);
		unguard;
		
		// Merge the volumetrics.
		guard(MergeVolumetrics);
		UBOOL VirginFog = true;
		for( FLightInfo* Info=FirstLight; Info<LastLight; Info++ )
		{
			if( Info->IsVolumetric )
			{
				// Compute the volumetric.
				Info->ComputeFromActor( FogMap, Frame, 1 );

				FVector	Vertex1  = VertexBase.TransformPointBy ( Frame->Coords );
				FVector VertDU   = VertexDU  .TransformVectorBy( Frame->Coords );
				FVector VertDV   = VertexDV  .TransformVectorBy( Frame->Coords );

				if (VirginFog)
				{					
					// first-time fog calculation, no merging required.
					FColor* Dest = (FColor*)FogMip.DataPtr;
					for( int i=0; i<FogMap.VClamp; i++ )
					{
						FVector Vertex = Vertex1;
						for( int j=0; j<FogMap.UClamp; j++ )
						{
							DWORD Light = appRound( Volumetric( Info, Vertex ) * 255.0f );
							Vertex+=VertDU;
							Dest[j] = Info->VolPalette[Light];
						}
						Vertex1 += VertDV;
						Dest += FogMap.USize; 
					}
					VirginFog = false;
				}
				else
				{
					// Merge in more fog. 
					// Todo: MMX-optimize ? & nonMMX assembly-optimize ( unpack to 15:15:15:15... )
					// -> Optimize for recurrence, AND most merging will STILL be with one component
					// Completely black // or saturated.... - worth a few mispredicted jumps !!!!!
					// Todo: after alpha-aware merging is in, also check for original value being
					// zero as a quick bailout.
					
					FColor* Dest = (FColor*)FogMip.DataPtr;

					for( int i=0; i<FogMap.VClamp; i++ )
					{
						FVector Vertex = Vertex1;
						for( int j=0; j<FogMap.UClamp; j++ )
						{
							FLOAT FogAdd = Volumetric( Info, Vertex );
							Vertex +=VertDU;
						    if (*(DWORD*)&FogAdd != 0) // bailout if 0...
							{
								DWORD Light = appRound( FogAdd  * 255.0f );
								Dest[j].R = ByteMuck[Dest[j].R+128*(INT)Info->VolPalette[Light].R]; 
								Dest[j].G = ByteMuck[Dest[j].G+128*(INT)Info->VolPalette[Light].G]; 
								Dest[j].B = ByteMuck[Dest[j].B+128*(INT)Info->VolPalette[Light].B]; 
								Dest[j].A = 0; //ByteMuck[Dest[j].A+128*(INT)Info->VolPalette[Light].A];
							}
						}
						Vertex1 += VertDV;
						Dest += FogMap.USize;
					}

				}
			}
		}
		unguard;
	}

	// Set up LightMip and LightMap.
	guard(SetupLightMap);
	LightMip.UBits			= FLogTwo(Index->UClamp);
	LightMip.VBits			= FLogTwo(Index->VClamp);
	LightMip.USize			= 1 << LightMip.UBits;
	LightMip.VSize			= 1 << LightMip.VBits;
	LightMap.Pan			= Index->Pan;
	LightMap.UScale			= Index->UScale;
	LightMap.VScale			= Index->VScale;
	LightMap.UClamp			= Index->UClamp;
	LightMap.VClamp			= Index->VClamp;
	LightMap.USize			= LightMip.USize;
	LightMap.VSize			= LightMip.VSize;
	LightMap.TextureFlags	= 0;
	LightMap.CacheID		= MakeCacheID( CID_StaticMap, iLightMap, ZoneID, Model );
	unguard;

	// Handle static lighting.
	DWORD* Stream = (DWORD*)GCache.Get(LightMap.CacheID,*TopItemToUnlock++);
	struct FMoverStamp{ INT iLeaf; FVector Location; FRotator Rotation; };
	if( Mover && Stream )
	{
		StaticLightingChanged |= ((FMoverStamp*)Stream)->iLeaf    != Mover->Region.iLeaf;
		StaticLightingChanged |= ((FMoverStamp*)Stream)->Location != Mover->Location;
		StaticLightingChanged |= ((FMoverStamp*)Stream)->Rotation != Mover->Rotation;
	}
	if( !Stream || StaticLightingChanged )
	{
		// Setup caching.
		guard(StaticLighting);
		StaticLightingChanged=1;
		if( !Stream )
			Stream = (DWORD*)GCache.Create( LightMap.CacheID, TopItemToUnlock[-1], (LightMap.USize*LightMap.VClamp) * sizeof(DWORD) + sizeof(FColor) + sizeof(FMoverStamp), DEFAULT_ALIGNMENT, LightMap.USize*(LightMap.VSize-LightMap.VClamp) );
		if( Mover )
		{
			((FMoverStamp*)Stream)->iLeaf    = Mover->Region.iLeaf;
			((FMoverStamp*)Stream)->Location = Mover->Location;
			((FMoverStamp*)Stream)->Rotation = Mover->Rotation;
		}
		Stream += sizeof(FMoverStamp)/sizeof(DWORD);
		LightMap.MaxColor = (FColor*)Stream++;
		*LightMap.MaxColor = FColor(255,255,255,255);

		// Generate ambient color.
		AmbientVector = FGetHSV( Zone->AmbientHue, Zone->AmbientSaturation, Zone->AmbientBrightness );
		FColor AmbientColor( AmbientVector*0.25 );
		Exchange( AmbientColor.R, AmbientColor.B );
		DWORD* Temp = Stream;
		for( INT i=0; i<LightMap.VClamp; i++ )
		{
			for( INT j=0; j<LightMap.UClamp; j++ )
				Temp[j] = AmbientColor.D;
			Temp += LightMap.USize;
		}

		// Add in all static lights.
		FMemMark Mark(GMem);
		for( FLightInfo* Info = FirstLight; Info < LastLight; Info++ )
		{
			if( Info->Opt == ALO_StaticLight )
			{
				// Static lighting.
				Info->ComputeFromActor( LightMap, Frame, 1 );
				BYTE* ShadowMap = New<BYTE>(GMem,ShadowMaskSpace*8);
				ShadowMapGen( LightMap, Info->ShadowBits, ShadowMap );
				Info->IlluminationMap = New<BYTE>(GMem,LightMap.UClamp*LightMap.VClamp);
				Info->Effect.SpatialFxFunc( LightMap, Info, ShadowMap, Info->IlluminationMap );
				Merge( LightMap, Info->Actor->LightEffect, 0, Info, Stream, Stream );
				Mark.Pop();
			}
		}
		unguard;
	}
	else
	{
		Stream += sizeof(FMoverStamp)/sizeof(DWORD);
		LightMap.MaxColor = (FColor*)Stream++;
	}

	// Merge in the dynamic lights.
	if( DynamicLights || MovingLights )
	{
		guard(DynamicLight);
		DWORD* Static = Stream;
		LightMap.CacheID = MakeCacheID( CID_DynamicMap, iLightMap, ZoneID, Model );
		if( Merged )
		{
			// Allocate in temporary memory.
			Stream = New<DWORD>(GMem,LightMap.USize*LightMap.VSize+1);
			LightMap.MaxColor = (FColor*)Stream++;
		}
		else
		{
			// Cache it.
			Stream = (DWORD*)GCache.Get( LightMap.CacheID, *TopItemToUnlock++ );
			if( !Stream || *(DOUBLE*)Stream!=Frame->Viewport->CurrentTime )
			{
				if( !Stream )
					Stream = (DWORD*)GCache.Create( LightMap.CacheID, TopItemToUnlock[-1], (LightMap.USize*LightMap.VClamp + 3) * sizeof(DWORD), DEFAULT_ALIGNMENT, LightMap.USize*(LightMap.VSize-LightMap.VClamp) );
				*(DOUBLE*)Stream = Frame->Viewport->CurrentTime;
				Stream += 2;
				LightMap.MaxColor = (FColor*)Stream++;
			}
			else
			{
				*(DOUBLE*)Stream = Frame->Viewport->CurrentTime;
				Stream += 2;
				LightMap.MaxColor = (FColor*)Stream++;
				goto SkipDynamicLight;
			}
		}
		*LightMap.MaxColor = FColor(255,255,255,255);

		// Copy the static lighting.
		DWORD *TmpStatic=Static, *TmpStream=Stream;
		for( INT i=0; i<LightMap.VClamp; i++ )
		{
			appMemcpy( TmpStream, TmpStatic, LightMap.UClamp*sizeof(DWORD) );
			TmpStatic += LightMap.USize;
			TmpStream += LightMap.USize;
		}

		// Merge in the dynamic lights.
		FMemMark Mark(GMem);
		LightMap.TextureFlags |= TF_RealtimeChanged;
		for( FLightInfo* Info=FirstLight; Info<LastLight; Info++ )
		{
			if( Info->Opt==ALO_DynamicLight || Info->Opt==ALO_MovingLight )
			{	
				// Set up.
				BYTE* ShadowMap;
				Info->ComputeFromActor( LightMap, Frame, 1 );
				if( Info->Opt==ALO_MovingLight )
				{
					// Build a temporary shadow map and fill it with max.
					ShadowMap = New<BYTE>(GMem,ShadowMaskSpace*8);
					ShadowMapGen( LightMap, NULL, ShadowMap );

					// Build a temporary illumination map.
					Info->IlluminationMap = New<BYTE>(GMem,LightMap.UClamp*LightMap.VClamp);
					Info->Effect.SpatialFxFunc( LightMap, Info, ShadowMap, Info->IlluminationMap );
				}
				else if( Info->Effect.IsSpatialDynamic )
				{
					// This light has spatial effects, so we must cache its shadow map since
					// we will be generating its illumination map per frame.
					//note: we use iSurf because only (iLightMap,Actor,Mover) is unique.
					QWORD CacheID = MakeCacheID( CID_ShadowMap, Draw->iSurf/*iLightMap*/, 0, Info->Actor );
					ShadowMap = (BYTE *)GCache.Get( CacheID, *TopItemToUnlock++ );
					if( !ShadowMap  )
					{
						// Create and generate its shadow map.
						ShadowMap = (BYTE *)GCache.Create( CacheID, TopItemToUnlock[-1], ShadowMaskSpace*8 );
						ShadowMapGen( LightMap, Info->ShadowBits, ShadowMap );
					}

					// Build a temporary illumination map:
					Info->IlluminationMap = New<BYTE>(GMem,LightMap.UClamp*LightMap.VClamp);
					Info->Effect.SpatialFxFunc( LightMap, Info, ShadowMap, Info->IlluminationMap );
				}
				else
				{
					// No spatial lighting. We use a cached illumination map generated from a temporary
					// shadow map. See if the illumination map is already cached:
					//note: we use iSurf because only (iLightMap,Actor,Mover) is unique.
					QWORD CacheID = MakeCacheID( CID_IlluminationMap, Draw->iSurf/*iLightMap*/, 0, Info->Actor );
					Info->IlluminationMap = (BYTE *)GCache.Get( CacheID, *TopItemToUnlock++ );
					if( !Info->IlluminationMap || Info->Actor->bLightChanged )
					{
						// Build a temporary shadow map.
						ShadowMap = New<BYTE>(GMem,ShadowMaskSpace*8);
						ShadowMapGen( LightMap, Info->ShadowBits, ShadowMap );

						// Build and cache an illumination map
						if( !Info->IlluminationMap )
							Info->IlluminationMap = (BYTE *)GCache.Create( CacheID, TopItemToUnlock[-1], (LightMap.UClamp*(LightMap.VClamp+1)+1) * sizeof(BYTE) );
						Info->Effect.SpatialFxFunc( LightMap, Info, ShadowMap, Info->IlluminationMap );
					}
				}

				// Merge the illumination map in.
				Merge( LightMap, Info->Actor->LightEffect, Key, Info, Stream, Stream );
				Mark.Pop();
			}
		}
		unguard;
	}
	SkipDynamicLight:;

	// Set pointers.
	LightMip.DataPtr = (BYTE*)Stream;

	STAT(unclock(GStat.IllumTime));
	unguard;
}

//
// Finish surface lighting.
//
void FLightManager::FinishSurf()
{
	guard(FLightManager::FinishSurf);

	// Release working memory.
	Mark.Pop();

	// Unlock any locked cache items.
	while( TopItemToUnlock > &ItemsToUnlock[0] )
		(*--TopItemToUnlock)->Unlock();

	// Update stats.
	STAT(GStat.Lightage += LightMap.UClamp * LightMap.VClamp);
	STAT(GStat.LightMem += LightMap.UClamp * LightMap.VClamp * sizeof(FLOAT));

	unguard;
}

/*------------------------------------------------------------------------------------
	Light setup.
------------------------------------------------------------------------------------*/

//
// Add a light to the list.
//
UBOOL FLightManager::AddLight( AActor* Actor, AActor* Light )
{
	guardSlow(FLightManager::AddLight);

	// Reject.
	if
	(	LastLight>=FinalLight
	||	Light->LightType==LT_None
	||	Light->LightBrightness==0
	||	Light==Actor )
		return 0;

	// Set actor and optimization flags.
	if( Actor )
	{
		// Distance-reject will reject lights that partially hit the mesh
		// because it doesn't take the collision size into account.
		LastLight->Opt = ALO_MovingLight;
		MovingLights++;
	}
	else if( Light->LightEffect == LE_OmniBumpMap )
	{
		LastLight->Opt = ALO_NotLight;
	}
	else if( Actor || Light->bDynamicLight || !(Light->bStatic || Light->bNoDelete) )
	{
		LastLight->Opt = ALO_MovingLight;
		MovingLights++;
	}
	else if
	(	Light->bStatic 
	&&	Light->LightType==LT_Steady
	&&	!Effects[Light->LightEffect].IsSpatialDynamic
	&&	!Effects[Light->LightEffect].IsMergeDynamic )
	{
		LastLight->Opt = ALO_StaticLight;
		StaticLights++;
	}
	else
	{
		LastLight->Opt = ALO_DynamicLight;
		DynamicLights++;
	}

	// Info.
	LastLight->Actor        = Light;
	LastLight->IsVolumetric = 0;
	LastLight->ShadowBits   = NULL;
	if( Light->bLightChanged )
		StaticLightingChanged = 1;
	LastLight++;
	return 1;
	unguardSlow;
}

/*------------------------------------------------------------------------------------
	Actor lighting.
------------------------------------------------------------------------------------*/

//
// Compute fast light list for an actor.
// Returns 1 if the actor should be lit, 0 if it should be fake-lit.
//
enum {MaxActorLights=16};
enum {DesiredActorLights=6};
static AActor* Consider[120];
static INT NumConsider=0, ConsiderTag=0;
inline void AddConsider( AActor* Actor, AActor* Light, BYTE Factor )
{
	if( Light->ExtraTag!=ConsiderTag )
	{
		// Note that distance reject neglects actor bounding sphere.
		FLOAT DistSquared = FDistSquared(Actor->Location,Light->Location);
		FLOAT Radius      = Light->WorldLightRadius();
		if( Light->bSpecialLit==Actor->bSpecialLit && DistSquared<Square(Radius) )
		{
			Light->LightingTag = (1.0-SqrtApprox(DistSquared)/Radius) * Light->LightBrightness * 1024;
			Light->ExtraTag    = ConsiderTag;
			Light->SpecialTag  = Factor;
			Consider[NumConsider++] = Light;
		}
	}
}
inline INT Compare( const class AActor* A1, const class AActor* A2 )
{
	return 1 - 2*(A1->LightingTag>A2->LightingTag);
}
DWORD FLightManager::SetupForActor( FSceneNode* InFrame, AActor* InActor, FVolActorLink* LeafLights, FActorLink* Volumetrics )
{
	guard(FLightManager::SetupForActor);
	DWORD Result=0;

	// Init per actor variables.
	Mark      = FMemMark(GMem);
	Frame     = InFrame;
	Level	  = Frame->Level;
	LevelInfo = Level->GetLevelInfo();
	Actor     = InActor;
	Zone	  = Actor->Region.Zone ? Actor->Region.Zone : LevelInfo;
	MapCoords = NULL;
	LastLight = FirstLight;
	Diffuse   = Actor->ScaleGlow;

	// If not zoned, try now.
	if( Actor->Region.ZoneNumber==0 )
		Level->SetActorZone( Actor, 1, 0 );

	// Ambient lighting.
	FLOAT C       = Actor->AmbientGlow!=255 ? Actor->AmbientGlow/255.0 : 0.25+0.2*sin(8*Frame->Viewport->CurrentTime);
	AmbientVector = FVector(C,C,C) + FGetHSV( Zone->AmbientHue, Zone->AmbientSaturation, Zone->AmbientBrightness );

	// Reject if the proper data structures aren't in place.
	if
	(	Actor->Region.iLeaf!=INDEX_NONE
	&&	Level->Model->Leaves.Num()
	&&	Frame->Viewport->Actor->RendMap==REN_DynLight 
	&&	!Frame->Viewport->Client->NoLighting )
	{
		// Get actor light cache.
		INT Delta      = Clamp((INT)((Frame->Viewport->CurrentTime-Frame->Viewport->LastUpdateTime)*768),0,255);
		INT FrameCount = Frame->Viewport->FrameCount;
		INT* Num       = NULL;
		struct FInfo
		{
			AActor* Actor;
			BYTE Factor;
			INT Index;
		} *Senders;
		UBOOL FirstSeeActor = 0;
		QWORD CacheID  = MakeCacheID( CID_ActorLightCache, Actor );
		Senders        = (FInfo*)GCache.Get( CacheID, *TopItemToUnlock++ );
		if( Senders )
		{
			// Look up actors from cache.
			guardSlow(CacheLook);
			Num = (INT*)Senders++;
			for( INT i=0,NewNum=0; i<*Num; i++ )
			{
				if( NewNum!=i )
					Senders[NewNum] = Senders[i];
				if( GObj.GetIndexedObject(Senders[i].Index)==Senders[i].Actor && !Senders[i].Actor->bDeleteMe )
					NewNum++;
			}
			*Num = NewNum;
			unguardSlow;
		}
		else
		{
			// Create cache item.
			guardSlow(CacheCreate);
			FirstSeeActor = 1;
			Senders  = (FInfo*)GCache.Create( CacheID, TopItemToUnlock[-1], (MaxActorLights+1) * sizeof(FInfo) );
			Num      = (INT*)Senders++;
			*Num     = 0;
			unguardSlow;
		}

		// Make list of all lights to consider.
		NumConsider=0;
		ConsiderTag++;

		// Cached lights.
		guardSlow(ConsiderLights);
		for( INT i=0; i<*Num; i++ )
			if( Senders[i].Actor )
				AddConsider( Actor, Senders[i].Actor, Senders[i].Factor );
		unguardSlow;

		// Static leaf lights.
		guardSlow(StaticLeafLights);
		FLeaf& Leaf = Level->Model->Leaves(Actor->Region.iLeaf);
		if( Leaf.iPermeating!=INDEX_NONE )
			for( INT i=Leaf.iPermeating; Level->Model->Lights(i) && NumConsider<ARRAY_COUNT(Consider); i++ )
				AddConsider( Actor, Level->Model->Lights(i), 0 );
		unguardSlow;

		// Dynamic leaf lights.
		guardSlow(DynamicLeafLights);
		for( FVolActorLink* Link=LeafLights; Link && NumConsider<ARRAY_COUNT(Consider); Link=Link->Next )
			AddConsider( Actor, Link->Actor, 255 );
		unguardSlow;

		// Sort considered lights by priority.
		appSort( Consider, NumConsider );

		// Amortized trace visibility.
		INT NumRealVisible=0, Threshold=-1;
		guardSlow(TraceVis);
		for( INT i=0; i<NumConsider; i++ )
		{
			// Determine effective visibility.
			UBOOL IsVisible;
			if( Consider[i]->LightingTag<Threshold || NumRealVisible>=DesiredActorLights )
			{
				IsVisible = 0;
			}
			else if( !Consider[i]->bStatic && Consider[i]->bMovable )
			{
				IsVisible = 1;
			}
			else if( FirstSeeActor || ((FrameCount ^ Consider[i]->GetIndex())&7)==0 )
			{
				FCheckResult Hit(0);
				IsVisible = Level->Model->LineCheck( Hit, NULL, Consider[i]->Location, Actor->Location, FVector(0,0,0), 0 );
				if( IsVisible && FirstSeeActor )
					Consider[i]->SpecialTag = 255;
			}
			else
			{
				IsVisible = (Consider[i]->SpecialTag&1);
			}

			// Handle light.
			if( IsVisible )
			{
				// This light is considered visible.
				NumRealVisible++;
				if( Threshold==-1 )
					Threshold = Consider[i]->LightingTag/8;
				AddLight( Actor, Consider[i] );
				Consider[i]->SpecialTag |= 1;

				// Update factor.
				Consider[i]->SpecialTag = Min( (INT)Consider[i]->SpecialTag+Delta, 255 ) | 1;
			}
			else
			{
				// This light is not considered visible.
				Consider[i]->SpecialTag = Max( (INT)Consider[i]->SpecialTag-Delta, 0 ) & ~1;
				if( Consider[i]->SpecialTag>0 )
					AddLight( Actor, Consider[i] );
			}
		}
		unguardSlow;

		// Recache all accepted lights.
		guardSlow(Recache);
		*Num=0;
		for( INT i=0; i<NumConsider && *Num<MaxActorLights; i++ )
		{
			if( Consider[i]->SpecialTag > 1 )
			{
				Senders[*Num].Actor  = Consider[i];
				Senders[*Num].Index  = Consider[i]->GetIndex();
				Senders[*Num].Factor = Consider[i]->SpecialTag;
				++*Num;
			}
		}
		unguardSlow;

		// Volumetric occluding lights.
		guardSlow(Vtrics);
		for( Volumetrics; Volumetrics && LastLight<FinalLight; Volumetrics=Volumetrics->Next )
		{
			Result |= PF_RenderFog;
			for( FLightInfo* Find=FirstLight; Find<LastLight && Find->Actor!=Volumetrics->Actor; Find++ );
			Find->IsVolumetric = 1;
			if( Find==LastLight )
			{
				// New volumetric light.
				LastLight->Actor = Volumetrics->Actor;
				LastLight->Opt   = ALO_NotLight;
				STAT(GStat.MeshVtricCount++);
				LastLight++;
			}
		}
		unguardSlow;

		// Set up the lights.
		guardSlow(SetupLights);
		for( FLightInfo* Light=FirstLight; Light<LastLight; Light++ )
		{
			Light->ComputeFromActor( LightMap, Frame, 0 );
			Light->FloatColor *= Light->Actor->SpecialTag/255.0;
		}
		unguardSlow;
	}
	STAT(GStat.MeshLightCount+=(LastLight-FirstLight));
	return Result;
	unguard;
}

//
// Finish actor lighting.
//
void FLightManager::FinishActor()
{
	guard(FLightManager::FinishActor);

	// Release working memory.
	Mark.Pop();

	// Unlock any locked cache items.
	while( TopItemToUnlock > &ItemsToUnlock[0])
		(*--TopItemToUnlock)->Unlock();

	unguard;
}

/*------------------------------------------------------------------------------------
	Light subsystem instantiation
------------------------------------------------------------------------------------*/

static FLightManager GLightManagerInstance;
FLightManagerBase* GLightManager = &GLightManagerInstance;

/*------------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------------*/
