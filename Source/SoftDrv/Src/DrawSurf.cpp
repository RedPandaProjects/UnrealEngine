/*=============================================================================

	DrawSurf.cpp: World surface drawing functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
	    * Optimized & MMXized by Erik de Neve
	
    Work-in-progress todo's:

	 - Check unlit constants against 3dfx.
     - NONMMX MODULATED ! 

     - If there's any indication a certain span/POLY? does NOT have the first mipmap in 
	   view, try to NOT call the fuzzing version of the texture mapper....
     - Convert all single-innerloop MMX hicolor routines (modulated, translucent) to the same
	   efficient interleaving scheme as the MMX 32 translucency...
	 - Check unlit modulation in poly & surf...... (weaponfx is mostly drawtile modulation)
	 - Check if our current elaborate mipmap-level multiplier setup can be optimized
	 - Tweak mipmap criterium - be less sensitive to tilt, and more to distance ?
	 - NonMMX pentium LIGHT setup still lots to optimize.
	 - 15/16bit MMX masking and translucency are very messily interleaved...
	 - Simple (cached-palette in target format) FAST UNLIT support (LOW END MACHINES !)

	 - Clean up the whole ugly decision-tree mess. Modularize !!!
	    
=============================================================================*/

#include "SoftDrvPrivate.h"
#include <stdlib.h>          //for _rotl and _rotr
#include <math.h>            //for  fabs()

/*----------------------------------------------------------------------------
	Stubs.
----------------------------------------------------------------------------*/

#pragma pack(8)                    /*  !!  */ 
#pragma warning( disable : 4035 )  /*  Implied return value in [eax] */
#pragma warning( disable : 4799 )  /*  no EMMS instruction           */
#pragma warning( disable : 4505 )  /*  "Unreferenced local function removed"  */
   
#define FixedMult   (65536.f)		// Texture coordinate fixed point multiplier. ( & Lightmap...)
#define FixedBits   (16)			// Texture coordinate fixed point integer bits. ( & Lightmap...)


#define PERSPECTIVEACCURACY 1.58 // 1.4 Max allowed perspective deviation (1.5?) scale: in pixels for screensize 400. 

#define MAXPERSPSUBDIVSIZE 256    // MUST BE 8-aligned......

//
// #debugging stuff:
//

#define INDICATOR    0		//  Perspective correction boundary test
#define MIPINDICATOR 0		//  MIPmap switching indicators...
#define ALPHAFOG     0		//  Alpha fogging

//#pragma inline_depth(0)	//  DEBUGGING purposes only: prevent any functions from inline expansion. 

#define  DOALIGN16  align 16
//#define  DOALIGN16 

// Aligning inline assembler sometimes screws up the compiler causing grave 
// errors in the code, like jumps into oblivion !


/*----------------------------------------------------------------------------
	Types.
----------------------------------------------------------------------------*/

struct FMipSetup
{
	FMMX			Mask;
	FMMX            RMask;
	FMMX			Mult;
	FMMX            HybridShift;
	FMMX            HybridMask;
	DWORD			USize;
	FRainbowPtr		Data;
	struct FTexSetup* (*Func)( struct FTexSetup* Setup );
	INT             TexVRotate; 
	INT             TexURotate;
	INT             TexVScale; 
	INT             TexUScale;
	INT				LVShift;	
	INT				HUShift; 
};

struct FLightMipSetup
{
	FMMX			Mask;
	FMMX			RMask;
	FMMX			Mult;
	DWORD			USize;
	FRainbowPtr		Data;
	FRainbowPtr     FogData;
	INT				LVShift;
	INT				LVShift3;
	INT				HUShift; 
	//FLOAT           UScale;
	//FLOAT           VScale;
	FLOAT			VScale8;
	FLOAT			VScale;
	FLOAT			UScale8;
	FLOAT			UScale;
	struct FTexSetup* (*Func)( struct FTexSetup* Setup );
};

// Per-mip variables.
static FMipSetup		Mips[12];
static FLightMipSetup	LightMip;

/*

	MMX texture coordinate setup:  U, V in a special fixed point format in doublewords:

		Coordinate.DH =  12:20 -> 12 = empty, 20 = left-aligned U coordinate integer bits, filled out with the fractional bits.
		Coordinate.DL =  32:0  -> the V coordinate, with the 'decimal' point at 12.20 so that when shifted left by 
		                          12, we shift all the V integer bits into the high dword.

	Setting up U and V (and DU,DV) -> with 'FixedBits' fixed bits: 

		TexURotate  = 32 + (FixedBits - 12) - Src.UBits - iMip; // 16:16 -> 12:Vshift:fractional
		TexVRotate  = 32 + (FixedBits - 12) - iMip;

		Tex.DH = _rotl(U, Mip->TexURotate); // Rotate right by  12,  left by (FixedBits - UBits) to left-align
		Tex.DL = _rotl(V, Mip->TexVRotate); // right-align on lowest of 12 highest bits -> 

		HybridShift.Q  = 32-Src.UBits;		// shift right this amount 
		shift right to align ubits to right again.

		HybridMask.DL  = 0x3FFFFFFF >> (30- (Src.UBits + Src.VBits ));   
		Masks out the resulting VBITS:UBITS texel coordinate.

    To obtain the texel index:
	After a shift left by 12 bits, the upper dwords looks like [U-integer,U-fractional,V-integer]
	Then the high dword is copied to the low one, the whole thing shifted right by (32-Ubits) and masked

	movq        mm1,mm7            // copy 
	paddd		mm7,mm6			   // add delta
	psllq       mm1,12             //  
	punpckhdq   mm1,mm1            // copy high word to low word 
	psrlq       mm1,[MMXCoShift]   // 
	pand        mm1,[MMXCoMask]    // 
	movd        ecx,mm1            // 

   Texture coordinates could probably be delivered in a 12:20 format during setup, 
   but lightmaps are done with a fixed 16:16 scheme, important for access to the 
   fractional bits for bilinear interpolation.
			
*/

struct FTexSetup
{
	INT  	X;
	void*   CoSetup;
	FMMX	Tex, Lit;

	//void InitMipMMX( FMipSetup* Mip )
	//{
	//	CoSetup = Mip;
	//}
	
	void InitTexPentium( INT InX, FMipSetup* Mip, DWORD U, DWORD V )
	{
    	X	   = InX;
		DWORD VRotated = _rotl(V, Mip->LVShift);
		Tex.DL  = VRotated;
		Tex.DH  = ((U << Mip->HUShift) & Mip->RMask.DH) + ( VRotated & Mip->Mask.DH);				
	}

	void InitLightPentium( DWORD U, DWORD V )
	{
		DWORD VRotated = _rotl(V, LightMip.LVShift);
		Lit.DL = VRotated;
		Lit.DH = ((U<<LightMip.HUShift)& LightMip.RMask.DH) + ( VRotated & LightMip.Mask.DH );
	}

	void InitLightPentiumDelta( DWORD U, DWORD V )
	{
		DWORD VRotated = _rotl(V, LightMip.LVShift);
		Lit.DL = VRotated;
		Lit.DH = ((U<<LightMip.HUShift)&  LightMip.RMask.DH) + ( VRotated & LightMip.Mask.DH );
	}

	/*
	void InitTexMMX( INT InX, FMipSetup* Mip, FLOAT UF, FLOAT VF )
	{   
		X	   =  InX;
		Tex.DL =  appRound( VF * Mip->TexVScale );
		Tex.DH =  appRound( UF * Mip->TexUScale );
	}
	*/

	void InitTexMMX( INT InX, FMipSetup* Mip, DWORD U, DWORD V)
	{   
		X	   =  InX;
		Tex.DL = _rotl(V, Mip->TexVRotate);
		Tex.DH = _rotl(U, Mip->TexURotate);
	}

	void InitLightMMX( FLOAT UF, FLOAT VF )
	{
		Lit.DL = appRound( UF * LightMip.UScale ); 
		Lit.DH = appRound( VF * LightMip.VScale ); 
	}

	void InitLightMMXDelta( FLOAT UF, FLOAT VF )
	{
		Lit.DL = appRound( UF * LightMip.UScale8); 
		Lit.DH = appRound( VF * LightMip.VScale8); 
	}

};


typedef FTexSetup* (*FTexturePassFunction)( FTexSetup* Setup );

union FTexSetupUnion
{
	BYTE*                   PtrBYTE;
	void*					PtrVOID;
	FTexSetup*				PtrFTexSetup;
	//FTexSetupMMX*           PtrFTexSetupMMX;
	INT*					PtrINT;
	DWORD*					PtrDWORD;
	FMMX*                   PtrFMMX;
	FTexturePassFunction*	PtrFTexturePassFunction;
};


/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// DEBUGGING defines
#define NUMSIZE FLOAT   // Default variable size in setup FP calculations.
#define FinalShift  4 
#define TRANS 0
#define TESTCLAMP 0

// Defines.
#define UNLITLEVEL 	 0x3F  // Unlit light level. 0.5 * 127  #debug is this right ?
#define PALSHADES_R  64+4  // All + 4 for dithering elbowspace.
#define PALSHADES_G  64+4
#define PALSHADES_B  64+4
#define LIGHTSHADES 128
#define SHADE_G  ( 0 )
#define SHADE_R  ( PALSHADES_G  )
#define SHADE_B  ( PALSHADES_G  +  PALSHADES_R )
#define SAFEHEAPLIMIT 0x18000
#define SETUPHEAPSIZE 0x20000

#define MMX16RGBSHIFT 5
#define MMX15RGBSHIFT 6

// Permanent variables.

static INT KernelDU[4]={  2, 14,  6, 10 };
static INT KernelDV[4]={ 14,  6, 10,  2 };
static INT SubScaleTable[258];

// Tables & MMX Constants

static BYTE			TexSetupHeap[SETUPHEAPSIZE];
static FLOAT		DivTable[256+2];
static DWORD        Colors[NUM_PAL_COLORS];
static FMMX         MMXFlashOffset13, MMXFlashCompress;
static FMMX         MMXFlashDither0, MMXFlashDither1;
static FMMX         MMXTempFogDelta; 
//static FMMX         MMXUnLit;
							
static DWORD		SavedCaps;
static DWORD        SavedMMXState;
static FMipSetup*	MipTbl[512];

// Internal texture-mapper storage

static FMMX         PrevLitCoordinate;
static DWORD        UnlitPentiumValue;
static FMMX         UnlitMMXValue;
static FMMX         MMXLit;
static FMMX         MMXLitDelta;
static FMMX         MMXCoShift;
static FMMX         MMXCoMask;
static FMMX         MMXFogDelta;
static FMMX         MMXCoorDelta;
static FMMX         MMX14Bits;
static FMMX         KnightA0,KnightA1, KnightB0,KnightB1;
static DWORD        NextXStore;
static DWORD        NX;
static DWORD        PX;

static QWORD		MMX15REDBLUE = 0x00f800f800f800f8;	// masks out all but the 5MSBits of red and blue
static QWORD		MMX15GREEN   = 0x0000f8000000f800;  // masks out all but the 5MSBits of green
static QWORD		MMX15MULFACT = 0x2000000820000008;	// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results

static QWORD		MMX16REDBLUE = 0x00f800f800f800f8;	// masks out all but the 5MSBits of red and blue
static QWORD		MMX16GREEN   = 0x0000fC000000fC00;  // masks out all but the 5MSBits of green
static QWORD		MMX16MULFACT = 0x2000000420000004;	// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results

/*
static QWORD		MMX15DitherEvenA = 0x0606060600000000;   //
static QWORD		MMX15DitherOddA  = 0x0000000006060606;   //
static QWORD		MMX15DitherEvenB = 0x0202020204040404;   //
static QWORD		MMX15DitherOddB  = 0x0404040402020202;   //
*/

static QWORD		MMX15DitherEvenA = 0x0506060700000000;   //
static QWORD		MMX15DitherOddA  = 0x0000000006050607;   //
static QWORD		MMX15DitherEvenB = 0x0102030204030405;   //
static QWORD		MMX15DitherOddB  = 0x0403040501010203;   //

static QWORD		MMX15CleanRed      = 0x000000FF000000FF; //
static QWORD		MMX15CleanGreenRed = 0x0000FFFF0000FFFF; //

static QWORD		MMX16CleanRed      = 0x000000FF000000FF; //
static QWORD		MMX16CleanGreenRed = 0x0000FFFF0000FFFF; //

static QWORD		MMX16DitherEvenA = 0x0606030600000000;   //
static QWORD		MMX16DitherOddA  = 0x0000000006060306;   //
static QWORD		MMX16DitherEvenB = 0x0202020204040204;   //
static QWORD		MMX16DitherOddB  = 0x0404020402020102;   //


static FTexSetupUnion SetupWalker;        //  Point Setup at start of setupheap.
static FTexSetupUnion PhotonSetupWalker;  //  Point into lightvalue-setup array. Used in ASM for temp storage only.
static FMMX         MMXDeltaAdjust[16];   //  Contains 1/N in FMMX "0:15, 0:15, 0:15, 0:15" fixed point format

static FMMX*        MMXColors;
static DWORD		LightUBits;

// Per-span blitting variables.
static FRainbowPtr	ScreenDest;
static DWORD		SavedEBP,SavedESP;
static INT          Sub;
static DWORD        TexSetup; 

// Sampled light values for a span.
static FMMX         Photon[ MaximumXScreenSize * 2 ];  //!!crashes at resolutions above 1600


// NonMMX stuff.
static BYTE         Shade[ LIGHTSHADES * 256 ];


/*-----------------------------------------------------------------------------
	Initialization.
-----------------------------------------------------------------------------*/

void USoftwareRenderDevice::InitMMXFlashes( FLOAT Brightness, INT ColorBytes, DWORD Caps)
{
	guardSlow(USoftwareRenderDevice::InitMMXFlashes);
	//  FlashScale: screen intensity scaling: 0=none, 128=normal brightness, 255=2X overbright.	
	//  Scaled for: 15 bit output, 7 bit input.
	//  (0.5 + Brightness) is the reference scaling factor;  won't match 3dfx since Brightness 
	//  on 3DFX cards just affects the gamma correction....

	// Scale: 1.0 =  0x3FFF = 14 bits, overbright = 1.0<x<1.5 (approx) = max. 0x7FFF.    
	
	DOUBLE Scale = (0.50f + Brightness)*(double)(0x3FFF)/(double)(0x80);  // (1 << (14 - 7) ) -1 ); 
	
	// Note: Blue and Red swapped to conform to MMX/32-bit mode color 'endian'.

	DWORD FR = appRound( ( Scale * FlashScale.B ));
	DWORD FG = appRound( ( Scale * FlashScale.G ));
	DWORD FB = appRound( ( Scale * FlashScale.R ));

	MMXFlashCompress.R = Clamp( FR ,(DWORD)0x0, (DWORD)0x7FFF);
	MMXFlashCompress.G = Clamp( FG ,(DWORD)0x0, (DWORD)0x7FFF);
	MMXFlashCompress.B = Clamp( FB ,(DWORD)0x0, (DWORD)0x7FFF);
	MMXFlashCompress.A = 0;

	// FlashFog   = screen fogging: 0=none, 255=fullly saturated fog.

	// 8-bit values -> 13-bit values.
	MMXFlashOffset13.R = ((DWORD)FlashFog.B << 4);
	MMXFlashOffset13.G = ((DWORD)FlashFog.G << 4);
	MMXFlashOffset13.B = ((DWORD)FlashFog.R << 4);
	MMXFlashOffset13.A = 0;

	// Unlit light level.	Affected by flashcompress also.
	FLOAT UnlitLight = (Brightness + 0.5f) * (FLOAT)(UNLITLEVEL << 7) / (FLOAT)(0x80);

	UnlitMMXValue.R = Clamp( appRound(UnlitLight * FlashScale.R) ,0,0x7FFF);
	UnlitMMXValue.G = Clamp( appRound(UnlitLight * FlashScale.G) ,0,0x7FFF);
	UnlitMMXValue.B = Clamp( appRound(UnlitLight * FlashScale.B) ,0,0x7FFF);
	UnlitMMXValue.A = 0;

	unguardSlow;
}






//
// Initialize the color tables.
// FlashFog   = screen fogging: 0=none, 255=fullly saturated fog.
// FlashScale = screen scaling: 0=none, 128=normal brightness, 255=2X overbright.
// Brightness: = 0 .. 1.0,  currently 0.39 by default.
// Texture colors: 0..31 for each channel in current implementation.
//

void USoftwareRenderDevice::InitColorTables( FLOAT Brightness, INT ColorBytes, DWORD Caps)
{
	guardSlow(USoftwareRenderDevice::InitColorTables);
	//
	// Optimized to facilitate the non-mmx software screenflashes.
	//
	//  Explicit formula:
	//  LUTShade = Fog  +  ( TextureShade * LightShade * (0.3 + 0.7*GRender->Brightness) * 18.0 * (FlashScale/128.0) ) / 256.0
	//
 	//  At default brightness (0.39) about 1/4 of ALL values (4400 out of 16384) are 
	//  saturated for each channel. With brightness at full 1.0, about 50% of all are saturated.
	//
	//  Todo:  - Knowing saturation ceiling in advance for every shade is much faster..
	//         - Consistency between 32,15 and 16 bit could be better
	//         - Pentiums like read-before-write.
	//         - 32-bit doesn't need dither, so could easily be extended to 84 shades instead of 64.
	//
	
	FLOAT Scale = (0.50f + Brightness) * 8.0 / (128.0 * 128.0);

	DWORD UnlitValue = Clamp( appRound( (FLOAT)UNLITLEVEL * (0.50f + Brightness) ),0,127);
	UnlitPentiumValue = UnlitValue + (UnlitValue << 8) + (UnlitValue <<16) + (UnlitValue << 24);

	int FogR  = appRound( FlashFog.R * (double)( 1<<20 ) );  // 1<<20  
	int FogG  = appRound( FlashFog.G * (double)( 1<<20 ) );
	int FogB  = appRound( FlashFog.B * (double)( 1<<20 ) );

	int D_ScaleR = appRound( Scale * FlashScale.R * (double)(1 << 20));
	int D_ScaleG = appRound( Scale * FlashScale.G * (double)(1 << 20));
	int D_ScaleB = appRound( Scale * FlashScale.B * (double)(1 << 20));

	int ScaleR = 0;
	int ScaleG = 0;
	int ScaleB = 0;

	for( int Light=0; Light< LIGHTSHADES; Light++ )
	{
		int HighLight = Light * 256;

		int ValR = FogR;
		int ValG = FogG;
		int ValB = FogB;

		if( ColorBytes == 4 )
		{
			for( int Tex=0; Tex<(64+4); Tex++ ) 
			{
				Shade[SHADE_R + Tex + HighLight] = Min( ( ValR += ScaleR)>>20 ,0xff );
				Shade[SHADE_G + Tex + HighLight] = Min( ( ValG += ScaleG)>>20 ,0xff );
				Shade[SHADE_B + Tex + HighLight] = Min( ( ValB += ScaleB)>>20 ,0xff );
			}
		}
		else if( Caps & CC_RGB565 )
		{
			for( int Tex=0; Tex<(64+4); Tex++ ) 
			{
				Shade[SHADE_R + Tex + HighLight] = Min( ( ValR += ScaleR)>>(20+3 ),0x1f ) << 3;
				Shade[SHADE_G + Tex + HighLight] = Min( ( ValG += ScaleG)>>(20+2 ),0x3f );
				Shade[SHADE_B + Tex + HighLight] = Min( ( ValB += ScaleB)>>(20+3 ),0x1f );
			}
		}
		else
		{
			for( int Tex=0; Tex<(64+4); Tex++ ) 
			{
				Shade[SHADE_R + Tex + HighLight] = Min( ( ValR += ScaleR)>>(20+3 ),0x1f ) << 2;
				Shade[SHADE_G + Tex + HighLight] = Min( ( ValG += ScaleG)>>(20+3 ),0x1f );
				Shade[SHADE_B + Tex + HighLight] = Min( ( ValB += ScaleB)>>(20+3 ),0x1f );
			}
		}

		ScaleR += D_ScaleR;
		ScaleG += D_ScaleG;
		ScaleB += D_ScaleB;

	}
	unguardSlow;
}



//
//	LightOSTable oversampling precalculation.
//

static QWORD LightOSTable[12][4];
static void SetupOverSampling()
{
	guardSlow(SetupOverSampling);
	QWORD Tmp[4];
	for (int ubits = 0; ubits < 12; ubits++)
	{
		for( int j,i=0; i<4; i++ )
			Tmp[i] = ((QWORD)KernelDV[i] << (32-4)) + ((QWORD)KernelDU[i] << (64-4-ubits));

		for( i=0,j=3; i<4; j=i++ )
			LightOSTable[ubits][i] = Tmp[i] - Tmp[j];	
	}
	unguardSlow;
}


static inline void InitBilinearKernel(INT UBits)
{
	//
	// 'Bilinear dither' approximation trick:  activate according to  !PF_NoSmooth
	//
	//  Biases texture coordinates according to a 2x2 kernel resulting in fuzzing of up-close pixels.
	//
	//  ->  make sure to disable this for non-up-close pixels because it results in major extra noise...?
	//  -> could be disabled according to some MIN Z distance combined with mipmult ???
	//
	//	Shifts all to be measured in 8th of a pixel, eg -3/8, +1/8  ...
	//		    

	//	Coordinate.DH =  12:20 -> 12 = empty, 20 = left-aligned U coordinate integer bits, filled out with the fractional bits.
	//	Coordinate.DL =  32:0  -> the V coordinate, with the 'decimal' point at 12.20 so that when shifted left by 
	//	                          12, we shift all the V integer bits into the high dword.

	//  Setting up U and V (and DU,DV) -> with 'FixedBits' fixed bits: 
	//
	//		TexURotate  = 32 + (FixedBits - 12) - Src.UBits - iMip; // 16:16 -> 12:Vshift:fractional
	//		TexVRotate  = 32 + (FixedBits - 12) - iMip;
	//
	//		Tex.DH = _rotl(U, Mip->TexURotate); // Rotate right by  12,  left by (FixedBits - UBits) to left-align
	//		Tex.DL = _rotl(V, Mip->TexVRotate); // right-align on lowest of 12 highest bits -> 
	//

	// Fuzzing over mipmaps seems OK: for V, any distortion diminishes proportionally; for U, it diminishes more
	// since it goes to lower bits with every next mip;

	//  Displacements measured as 1/16th of a texel.
	INT FuzzFracBits = 4;
	//  Dither scaler for U and V directions:
	INT UDitherShift = 20 - UBits - FuzzFracBits;
	INT VDitherShift = 20 - FuzzFracBits; // 

	//  A B         -> Our dithering: separate U,V displacements for each pixel in each 2x2 kernel. 
	//  C D          
	//                 

	// A good one: -1-2 , -2 0 , 0 +1 , +1 -1,  (* 3 or *4 for equal-sized subsquares...)
	// One that completely dissolves into equal sized squares 
	enum{ 
		A_U =  -4,
		A_V =  -8 ,
		
		B_U =  -8 ,
		B_V =   0,

		C_U =   0,
		C_V =  +4,

		D_U =  +4,
		D_V =  -4,
	};

	// Even lines.
	KnightA0.IH		= A_U << UDitherShift;
	KnightA1.IH		= B_U << UDitherShift;
	KnightA0.IL		= A_V << VDitherShift;
	KnightA1.IL		= B_V << VDitherShift;

	// Odd lines.
	KnightB0.IH		= C_U << UDitherShift;
	KnightB1.IH		= D_U << UDitherShift;
	KnightB0.IL		= C_V << VDitherShift;
	KnightB1.IL		= D_V << VDitherShift;

}


static inline void EraseBilinearKernel()
{
	// Even lines.
	KnightA0.IH	    = 0;
	KnightA1.IH		= 0;
	KnightA0.IL		= 0;
	KnightA1.IL		= 0;
	// Odd lines.
	KnightB0.IH		= 0;
	KnightB1.IH		= 0;
	KnightB0.IL		= 0;
	KnightB1.IL		= 0;
}

//
// Init software rendering globals. Called on screen size/bit depth changes also.
//

void USoftwareRenderDevice::InitDrawSurf()
{
	guardSlow(InitSoft);

	// MMX constants.
	MMX14Bits.Q = 0x3F803F803F803F80;   // Filled 14 bits for 127-X negation purposes


	// Divide table.
	DivTable[0]=0.0;
	for(INT i=1; i<ARRAY_COUNT(DivTable); i++ )
		DivTable[i] = FixedMult/i;  // 65536/i.....

	// Mipmap depth lookup table.
	for( i=0; i<512; i++ )
		MipTbl[i] = &Mips[Clamp( i - 0x85, 0, (INT)ARRAY_COUNT(Mips)-1 )];

	for (i = 0; i<16; i++)
	{ 
		DWORD Factor;
		if (i<2) Factor = 0.0; 
 			else Factor = appRound( (double)0x7FFF * (double)1.0/((double)i - 1.0) ); // -1.0 [e]

		MMXDeltaAdjust[i].R  =  Factor;
		MMXDeltaAdjust[i].G  =  Factor;
		MMXDeltaAdjust[i].B  =  Factor;
		MMXDeltaAdjust[i].A  =  Factor;
	}

	SetupOverSampling();

	// Perspective error should differ in a more complex way with differing
	// resolutions, ie. pixel deviations are a bit more forgiveable in low res (test!)

		//
		//  Subdivision LUT formula: based on midpoint abberation (amount of pixels the midpoint is off in 
		//  perfect perspective-corrected mapping vs. linear mapping)
		//
		//	Eyespace to screenpixel coordinate trafo:  D = assumed eye-screen distance in eyespace units.
		//
		//  XScreen =  D * Xeye/Zeye   
		//  usually units prepared so that D unnecessary, so that
		//   Xeye = XScreen*Zeye
		//
		//	Midpoint abberation: we have X0screen,X1screen and Z0eye,Z1eye -> now we can get 
		//  real  vs  interpolated midpoint position with:
		//
		//  (X0screen*Z0eye + X1Screen*Z1eye) / (Z1eye+Z2eye)   -   ((X1screen+X0screen)/2)
		//  
		//  which simpified to:   Error = 'C' =   (( X0 - X1 )*(Z0 - Z1)) / ( 2*( Z0 + Z1))
		//	
		//			=  DX*DZ/ 2*(Z0+Z1)    -> with Z0 calibrated:
		//        
		//          =  X1= X0+t*(X1-X0),  Z1 = Z0+t*(Z1-Z0): then Error formula becomes:
		//
		//          =  t*t*(DX)*(DZ) / 2* (Z0 + Z0 + t*(DZ))  -> Z0 being normalized to 1:
		//
		//			=  t*t * DX * DZ /  2 + 2tDZ   =   0.5  *  (t^2 * DX*DZ) / (1 + tDZ)
		// 
		//  (0.5 factor to be omitted from now on:) 
		//
		//  t = the scaling of the delta's;  so this formula for 'normalized' DZ, gives us
		//  the Error as function of t, the [0-1] scaling of our span.
		//
		//  The only thing determining the error is really DZ:DX ->  t can scale our Subdivision
		//  to any size once we have this DZ:DX 
		//  
		//  We really want t as function of a constant threshold error. 
		//	al3o, we really want t to represent SCREEN pixels. SO:   scale DX to ==1, and DZ'  to DZ/DX
		//
		//  This gives us  Error = t*t*(DZ/DX) / (1 + t(DZ/DX)) ,  R = DZ/DX the new 'calibrated' DZ....
		//
		//  ->  Error = c = ( t * t * R ) / ( 1 + tR )  
		//  
		//  DZ is normalized Z size:  ( (oldZFurthest - oldZClosest) / oldZClosest ) =  ( oldZF/oldZC - 1 )
		//  DX is screen span size in pixels.
		//
		//  Formula derived by inverting the midpoint abberation function:
		//
		//		sqrt( cR(cR+4))+cR)
		//	t =	-------------------
		//				2R
		//
		// Which gives us our desired Subdivision size for a maximum midpoint error of 'c', for R = DZ/DX.
		//
		// -> Decided not to vary Subdivision along spans because the variation in Subdivision size doesn't decrease
		//    significantly along a span even at very sharp viewing angles, partly because of the snapping on 8-pixel
		//    boundaries....
		//
		// Rather than using a table for this (which turns out to be too coarse at one end always) I now
	    // use the same function for 1/R  (simply DX/DZ) which gives us a desired Subdivision size of:
		//
		//	t =  0.5*( sqrt (c (c + 4R)) + c ) )   (& ~ 7)
	    //  
		// Ranges from: DZ/DX = 0 (large Subdivs) to DZ/DX = 1 ( small Subdivs, 45% slope, probably needs
		// maximum Subdiv with any reasonable C threshold.
		//

	unguardSlow;
}




/*-----------------------------------------------------------------------------
	Lighting pass.
-----------------------------------------------------------------------------*/

//
// Unlit pass.
//

static FTexSetup* NoLightPass( FTexSetup* Setup )
{
	guardSlow(NoLightPass);

	while( (Setup++)->X );
	return Setup;

	unguardSlow;
}


//
// 4X oversampled lighting pass.
//
static QWORD LightOS[4]={0,0,0,0};

static inline DWORD LightPentium( FMMX Lit )
{

#if !ASM 

	FMMX A,B,C,D;
	A.Q = Lit.Q;
	B.Q = A.Q + LightOS[1];
	C.Q = B.Q + LightOS[2];
	D.Q = C.Q + LightOS[3];

	DWORD L1
	=	LightMip.Data.PtrDWORD
			[	((A.DH                 ) >> (32-LightUBits))
			+	((A.DH&LightMip.Mask.DL) << (   LightUBits)) ]
	+	LightMip.Data.PtrDWORD
			[	((B.DH                 ) >> (32-LightUBits))
			+	((B.DH&LightMip.Mask.DL) << (   LightUBits)) ];
	DWORD L2
	=	LightMip.Data.PtrDWORD
			[	((C.DH                 ) >> (32-LightUBits))
			+	((C.DH&LightMip.Mask.DL) << (   LightUBits)) ]
	+	LightMip.Data.PtrDWORD
			[	((D.DH                 ) >> (32-LightUBits))
			+	((D.DH&LightMip.Mask.DL) << (   LightUBits)) ];

	return  (((L1&0xfefefe)+(L2&0xfefefe))/4) & 0x7f7f7f;

#else
	__asm
	{
		mov [SavedESP], esp
		mov [SavedEBP], ebp

		mov ebx, [Lit.DL]
		mov esi, [Lit.DH]
		mov edi, [LightMip.Mask.DL]
		mov ebp, [LightMip.Data]
		mov ecx, dword ptr [LightUBits]

		mov edx, edi
		and edx, esi
		rol edx, cl

		add ebx, [DWORD PTR LightOS +  8]
		adc esi, [DWORD PTR LightOS + 12]

		mov eax, [ebp + edx*4]

		mov edx, edi
		and edx, esi
		rol edx, cl
		
		add ebx, [DWORD PTR LightOS+16]
		adc esi, [DWORD PTR LightOS+20]

		mov esp, [ebp + edx*4]

		mov edx, edi
		and edx, esi
		lea eax, [eax + esp + 0x010101]
		rol edx, cl

		add ebx, [DWORD PTR LightOS+24]
		adc esi, [DWORD PTR LightOS+28]

		mov esp, [ebp + edx*4]

		mov edx, edi
		and edx, esi
		rol edx, cl

		add ebx, [DWORD PTR LightOS+0]
		adc esi, [DWORD PTR LightOS+4]
		
		mov edx, [ebp + edx*4]

		and eax, 0xfefefe 
		lea esp, [esp + edx + 0x010101]
		and esp, 0xfefefe 

		add eax, esp
		mov esp, [SavedESP]
		shr eax, 2
		mov ebp, [SavedEBP]
		and eax,0x7f7f7f
	}
#endif
}


//
//#debug Optimize!!
//

static FTexSetup* LightPassPentium( FTexSetup* Setup )
{
	FMMX Lit        = Setup->Lit;
	DWORD RGB       = LightPentium( Lit );
	INT X           = Setup->X;

	// Todo:
	// Small spans optimization: just choose some middle value and get ONE coordinate for those.
	// Faraway lightmap optimization: don't oversample !
	//

	while( (++Setup)->X )
	{
		FMMX LitD = Setup->Lit;
		INT  NX   = X + Setup->X;

		// Smooth light with a quick-and-dirty trick in batches of 16 and 8

		#if  !ASM
		while( X+8 <= NX )
		{
			Lit.Q                          += LitD.Q;
			Photon[X+0].DL = Photon[X+1].DL = RGB;
			DWORD NextRGB                   = LightPentium( Lit ); 
			DWORD D12                       = ((RGB + NextRGB) & 0xfefefe) >> 1;
			Photon[X+4].DL = Photon[X+5].DL =   D12;
			Photon[X+2].DL = Photon[X+3].DL = ((D12 +     RGB) & 0xfefefe) >> 1;
			Photon[X+6].DL = Photon[X+7].DL = ((D12 + NextRGB) & 0xfefefe) >> 1;
			RGB                             = NextRGB;
			X += 8;
		}
		#else
		while ( X+8 <= NX )
		{
			Lit.Q	+= LitD.Q;
			DWORD NextRGB  = LightPentium (Lit);

			// Smooth out RGB and NextRGB over 8 Photon light locations; when
			// both are the same (close-up !) just duplicate them all across though...
			__asm
			{
				// Writing 8x8 bytes = 2 cache lines, warming not required since
				// the Photon array will reside in the cache (usually).
				mov esi, offset Photon
				mov edi, X
				mov eax, RGB
				mov ebx, NextRGB
				mov ecx, eax

				//Smoothout:
				add  ecx,ebx                // RGB+NextRGB

				mov [esi+edi*8 + 0*8], eax  // Photon 0
				and  ecx,0x00fefefe         // mask out low bits

				mov [esi+edi*8 + 1*8], eax  // Photon 0
				shr  ecx,1

				add  ebx,ecx				//
				add  eax,ecx                //

				mov  [esi+edi*8+ 4*8],ecx   //
				and  eax,0x00fefefe         //  

				mov  [esi+edi*8+ 5*8],ecx   //				
				shr  eax,1				    //

				mov  [esi+edi*8+ 2*8],eax	//
				and  ebx, 0x00fefefe		//

				mov  [esi+edi*8+ 3*8],eax	//	
				shr  ebx,1					//

				mov  [esi+edi*8+ 6*8],ebx   //
				mov  [esi+edi*8+ 7*8],ebx   //	
				
				//EndSmooth:
			}
			RGB		= NextRGB;
			X += 8;				
		}
		#endif

		// Any remaining: smooth them out in batches of 4; but just duplicate if
		// we are at the last subdivision ????
		
		if (Setup->X) // is this NOT end of a span  ?
		{
			if( X < NX )
			{
				LitD.SQ >>= 1;
				do
				{
					Lit.Q						   += LitD.Q;
					Photon[X+0].DL = Photon[X+1].DL = RGB;
					DWORD NextRGB					= LightPentium( Lit ); 
					Photon[X+2].DL = Photon[X+3].DL = ((RGB + NextRGB) & 0xfefefe) >> 1;
					RGB								= NextRGB;
					X += 4;
				} while( X < NX );
			}
		}
		else // end of a span - smear out our light, don't do another sampling.
		{
			if( X < NX )
			{
				do
				{
					Photon[X+0].DL = Photon[X+1].DL = Photon[X+2].DL = Photon[X+3].DL = RGB;					
					X += 4;
				} while( X < NX );
			}
		}

	}
	return Setup;
}



/*-----------------------------------------------------------------------------
	Texture mapping pass.
-----------------------------------------------------------------------------*/

#if ASM /* Assembler texture mapping */

//
// Expands to tmap routines for every combination of mipmap and u-size;
// writes result to local array which later gets merged with the
// lightvalues and written to the line on the screen.
//

#define TEXTUREPASS_MACRO(ubits,imip) \
	static FTexSetup* TexturePass##ubits##_##imip( FTexSetup* Setup ) \
	{ \
		/* eax = texture offset     */ \
		/* ebx = vl                 */ \
		/* ecx = texel              */ \
		/* edx = texture base       */ \
		/* esi = vh | ul | uh       */ \
		/* edi = x                  */ \
		/* esp = dvh | dul | duh    */ \
		/* ebp = dvl                */ \
		__asm mov eax, [Setup] \
		\
		__asm mov [SavedESP], esp \
		__asm mov [SavedEBP], ebp \
		\
		__asm mov edi, [eax]FTexSetup.X \
		__asm mov ebx, [eax]FTexSetup.Tex.DL \
		\
		__asm mov esi, [eax]FTexSetup.Tex.DH \
		__asm add eax, SIZE FTexSetup \
		\
		__asm mov [TexSetup], eax \
		__asm mov ecx, [eax]FTexSetup.X \
		\
		__asm Loop1: \
			__asm mov ebp, [eax]FTexSetup.Tex.DL \
			__asm mov esp, [eax]FTexSetup.Tex.DH \
			\
			__asm mov eax, [Mips.Mask.DL + imip*SIZE FMipSetup] \
			__asm add ecx, edi \
			\
			__asm mov edx, [DWORD PTR Mips.Data + imip*SIZE FMipSetup] \
			__asm and eax, esi \
			\
			__asm rol eax, ubits \
			__asm mov [NX], ecx \
			\
			__asm Loop2: \
				__asm xor ecx,ecx \
				__asm add ebx, ebp \
				\
				__asm adc esi, esp \
				__asm mov cl, [eax + edx] \
				\
				__asm mov eax, [Mips.Mask.DL + imip*SIZE FMipSetup] \
				__asm inc edi \
				\
				__asm mov ecx, [Colors + ecx*4] \
				__asm and eax, esi \
				\
				__asm rol eax, ubits \
				__asm mov [DWORD PTR Photon - 4 + edi*8], ecx \
				\
				__asm mov  ecx, [NX] \
				\
				__asm cmp  edi, ecx \
				__asm jl   Loop2 \
			__asm mov eax, [TexSetup] \
			\
			__asm add eax, SIZE FTexSetup \
			\
			__asm mov [TexSetup], eax \
			\
			__asm mov ecx, [eax]FTexSetup.X \
			\
			__asm cmp ecx, 0 \
			__asm jnz Loop1 \
		__asm mov esp, [SavedESP] \
		__asm mov ebp, [SavedEBP] \
	}
#else /* Normal texture mapping */
#define TEXTUREPASS_MACRO(ubits,imip) \
	static FTexSetup* TexturePass##ubits##_##imip( FTexSetup* Setup ) \
	{ \
		FMMX Tex = Setup->Tex; \
		INT  X   = Setup->X; \
		while( (++Setup)->X ) \
		{ \
			INT NX    = X+Setup->X; \
			FMMX DTex = Setup->Tex; \
			do \
			{ \
				Photon[X].DH \
				=	Colors \
				[	Mips[imip].Data.PtrBYTE \
					[	((Tex.DH&Mips[imip].Mask.DL)>>(32-ubits)) \
					+	((Tex.DH&Mips[imip].Mask.DL)<<(ubits   )) ] ]; \
				Tex.Q += DTex.Q; \
			} while( ++X < NX ); \
			/* Photon[X-1].DH = Colors[255]; indicator ?*/\
			X = NX; \
		} \
		return Setup; \
	}
#endif

/* Stupid Compiler Tricks 101 */
#define TEXTUREPASS_MIP(imip) \
TEXTUREPASS_MACRO(0,imip)  TEXTUREPASS_MACRO(1,imip)  TEXTUREPASS_MACRO(2, imip) TEXTUREPASS_MACRO(3, imip) \
TEXTUREPASS_MACRO(4,imip)  TEXTUREPASS_MACRO(5,imip)  TEXTUREPASS_MACRO(6, imip) TEXTUREPASS_MACRO(7, imip) \
TEXTUREPASS_MACRO(8,imip)  TEXTUREPASS_MACRO(9,imip)  TEXTUREPASS_MACRO(10,imip) TEXTUREPASS_MACRO(11,imip)

TEXTUREPASS_MIP(0 ) TEXTUREPASS_MIP(1 ) TEXTUREPASS_MIP(2 ) TEXTUREPASS_MIP(3 ) 
TEXTUREPASS_MIP(4 ) TEXTUREPASS_MIP(5 ) TEXTUREPASS_MIP(6 ) TEXTUREPASS_MIP(7 ) 
TEXTUREPASS_MIP(8 ) TEXTUREPASS_MIP(9 ) TEXTUREPASS_MIP(10) TEXTUREPASS_MIP(11) 

#define TEXTUREPASS_TBL_MIP(imip) \
{ \
	TexturePass0_##imip,  TexturePass1_##imip,  TexturePass2_##imip,  TexturePass3_##imip,  \
	TexturePass4_##imip,  TexturePass5_##imip,  TexturePass6_##imip,  TexturePass7_##imip,  \
	TexturePass8_##imip,  TexturePass9_##imip,  TexturePass10_##imip, TexturePass11_##imip, \
}
static FTexturePassFunction TexturePassFunctions[16][16] =
{
	TEXTUREPASS_TBL_MIP(0 ), TEXTUREPASS_TBL_MIP(1 ), TEXTUREPASS_TBL_MIP(2 ), TEXTUREPASS_TBL_MIP(3 ), 
	TEXTUREPASS_TBL_MIP(4 ), TEXTUREPASS_TBL_MIP(5 ), TEXTUREPASS_TBL_MIP(6 ), TEXTUREPASS_TBL_MIP(7 ), 
	TEXTUREPASS_TBL_MIP(8 ), TEXTUREPASS_TBL_MIP(9 ), TEXTUREPASS_TBL_MIP(10), TEXTUREPASS_TBL_MIP(11), 
};



/*-----------------------------------------------------------------------------
	Blitting pass.
-----------------------------------------------------------------------------*/
//
// Merge the texture and lighting together to form a resulting color
// and write it to the screen.
// 2 y variations
//

#if !ASM

static void MergePass16( INT Y, INT X, INT InnerX )
{
#if 0 /* Non-dithered unaligned C++ */
	do
	{

	ScreenDest.PtrWORD[X+0] =
		Shade[SHADE_B  + Photon[X+0].SB2 + Photon[X+0].SB1*256]       +
		Shade[SHADE_G  + Photon[X+0].SG2 + Photon[X+0].SG1*256] *  32 +
   	    Shade[SHADE_R  + Photon[X+0].SR2 + Photon[X+0].SR1*256] * 256 ;
	}
	while( ++X < InnerX );

#else /* Dithered 2-aligned C++ */

	enum {DITHERSTEP  = 1}; //

	if( Y & 1 )
	{
		if( X & 1 )
		{
    	    ScreenDest.PtrWORD[X] =
    		    Shade[ 3*DITHERSTEP   + SHADE_B  + Photon[X].SB2 + Photon[X].SB1*256]       +
        		Shade[ 0*DITHERSTEP   + SHADE_G  + Photon[X].SG2 + Photon[X].SG1*256] *  32 +
           	    Shade[ 2*DITHERSTEP   + SHADE_R  + Photon[X].SR2 + Photon[X].SR1*256] * 256 ;
 
			X++;
		}
		while( X+1 < InnerX )
		{
			DWORD LR = Photon[X].SR1*256;
			DWORD LG = Photon[X].SG1*256;
			DWORD LB = Photon[X].SB1*256;

    	    ScreenDest.PtrWORD[X] =
    		    Shade[ 1*DITHERSTEP + SHADE_B  + Photon[X].SB2 + LB]       +
        		Shade[ 2*DITHERSTEP + SHADE_G  + Photon[X].SG2 + LG] *  32 +
           	    Shade[ 0*DITHERSTEP + SHADE_R  + Photon[X].SR2 + LR] * 256 ;

    	    ScreenDest.PtrWORD[X+1] =
    		    Shade[ 3*DITHERSTEP + SHADE_B  + Photon[X].SB2 + LB]       +
        		Shade[ 0*DITHERSTEP + SHADE_G  + Photon[X].SG2 + LG] *  32 +
           	    Shade[ 2*DITHERSTEP + SHADE_R  + Photon[X].SR2 + LR] * 256 ;

			X+=2;
		}
		if( X < InnerX )
		{
    	    ScreenDest.PtrWORD[X] =
    		    Shade[ 1*DITHERSTEP + SHADE_B  + Photon[X].SB2 + Photon[X].SB1*256]       +
        		Shade[ 2*DITHERSTEP + SHADE_G  + Photon[X].SG2 + Photon[X].SG1*256] *  32 +
           	    Shade[ 0*DITHERSTEP + SHADE_R  + Photon[X].SR2 + Photon[X].SR1*256] * 256 ;
		}
	}
	else
	{
	    // dither:  010  201 010  201

		if( X & 1 )
		{
    	    ScreenDest.PtrWORD[X] =
    		    Shade[ 0*DITHERSTEP + SHADE_B  + Photon[X].SB2 + Photon[X].SB1*256]       +
        		Shade[ 1*DITHERSTEP + SHADE_G  + Photon[X].SG2 + Photon[X].SG1*256] *  32 +
           	    Shade[ 0*DITHERSTEP + SHADE_R  + Photon[X].SR2 + Photon[X].SR1*256] * 256 ;

			X++;
		}

		while( X+1 < InnerX )
		{
			DWORD LR              = Photon[X].SR1*256;
			DWORD LG              = Photon[X].SG1*256;
			DWORD LB              = Photon[X].SB1*256;

    	    ScreenDest.PtrWORD[X] =
    		    Shade[ 2*DITHERSTEP + SHADE_B  + Photon[X].SB2 + LB]       +
        		Shade[ 0*DITHERSTEP + SHADE_G  + Photon[X].SG2 + LG] *  32 +
           	    Shade[ 1*DITHERSTEP + SHADE_R  + Photon[X].SR2 + LR] * 256 ;

    	    ScreenDest.PtrWORD[X+1] =
    		    Shade[ 0*DITHERSTEP + SHADE_B  + Photon[X].SB2 + LB]       +
        		Shade[ 1*DITHERSTEP + SHADE_G  + Photon[X].SG2 + LG] *  32 +
           	    Shade[ 0*DITHERSTEP + SHADE_R  + Photon[X].SR2 + LR] * 256 ;

			X+=2;
		}

		if( X < InnerX )
		{
    	    ScreenDest.PtrWORD[X] =
    		    Shade[ 2*DITHERSTEP + SHADE_B  + Photon[X].SB2 + Photon[X].SB1*256]       +
        		Shade[ 0*DITHERSTEP + SHADE_G  + Photon[X].SG2 + Photon[X].SG1*256] *  32 +
           	    Shade[ 1*DITHERSTEP + SHADE_R  + Photon[X].SR2 + Photon[X].SR1*256] * 256 ;
		}
	}
#endif
}

#endif  //!ASM



static void MergePass32Translucent( INT Y, INT X, INT InnerX )
{
	//
	// #optimize to do 2 or more at a time !
	//
	do 
	{
		if ( Photon[X+0].SB2 != 255)   // masked ??
		{
			DWORD ScreenDWord = ScreenDest.PtrDWORD[X];
			DWORD ColorDWord  = ( Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256]       ) +
								( Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256] << 8  ) +
								( Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256] << 16 );

			DWORD ColorSum = ScreenDWord + ColorDWord;
			DWORD ColorCarries = ( ( ScreenDWord ^ ColorDWord ) ^ ColorSum ) & 0x1010100;		
			ScreenDest.PtrDWORD[X] =  ColorSum | ( ColorCarries - (ColorCarries >> 8) );

			// Straight algorithm:
			// ScreenDest.PtrBYTE[X*4+0] = Min( ScreenDest.PtrBYTE[X*4+0] + Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256], 255);
			// ScreenDest.PtrBYTE[X*4+1] = Min( ScreenDest.PtrBYTE[X*4+1] + Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256], 255);
			// ScreenDest.PtrBYTE[X*4+2] = Min( ScreenDest.PtrBYTE[X*4+2] + Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256], 255);
		}
	}
	while( ++X < InnerX );
}



static void MergePass32Masked( INT Y, INT X, INT InnerX )
{
	do 
	{
		if ( Photon[X+0].SB2 != 255)   // masked ??
		{
			ScreenDest.PtrBYTE[X*4+0] = Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256];
			ScreenDest.PtrBYTE[X*4+1] = Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256];
			ScreenDest.PtrBYTE[X*4+2] = Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256];
		}
	}
	while( ++X < InnerX );
}


static void MergePass32Modulated( INT Y, INT X, INT InnerX )
{
	// 'lit modulation'
	do 
	{
		ScreenDest.PtrBYTE[X*4+0] = Min( ScreenDest.PtrBYTE[X*4+0] * Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256] >> 5, 255);
		ScreenDest.PtrBYTE[X*4+1] = Min( ScreenDest.PtrBYTE[X*4+1] * Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256] >> 5, 255);
		ScreenDest.PtrBYTE[X*4+2] = Min( ScreenDest.PtrBYTE[X*4+2] * Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256] >> 5, 255);
	}
	while( ++X < InnerX );
}

static void MergePass32ModulatedUnLit( INT Y, INT X, INT InnerX )
{
	do 
	{
		ScreenDest.PtrBYTE[X*4+0] = Min( (ScreenDest.PtrBYTE[X*4+0] * Photon[X+0].SB2) >> 5  , 255);
		ScreenDest.PtrBYTE[X*4+1] = Min( (ScreenDest.PtrBYTE[X*4+1] * Photon[X+0].SG2) >> 5  , 255);
		ScreenDest.PtrBYTE[X*4+2] = Min( (ScreenDest.PtrBYTE[X*4+2] * Photon[X+0].SR2) >> 5  , 255);
	}
	while( ++X < InnerX );
}




static void MergePass15Translucent( INT Y, INT X, INT InnerX )
{
	do
	{

		if (Photon[X+0].SB2 != 255) // masked ??
		{
			DWORD DestPix =
			Shade[SHADE_B  + Photon[X+0].SB2 + Photon[X+0].SB1*256]       +
			Shade[SHADE_G  + Photon[X+0].SG2 + Photon[X+0].SG1*256] *  32 +
   			Shade[SHADE_R  + Photon[X+0].SR2 + Photon[X+0].SR1*256] * 256 ;

			// Add 16-bit to 16-bit with saturation...
			_WORD ScreenPix = ScreenDest.PtrWORD[X+0];
			DWORD SumAdd = DestPix + ScreenPix;   // Sum with carries		
			DWORD Mask = ((DestPix ^ ScreenPix) ^ SumAdd) & 0x08420; // Diff carryless sums with normal sum
				  SumAdd |=  Mask - (Mask >> 5);   // Expand overflow carries
			
			ScreenDest.PtrWORD[X+0] = (_WORD) SumAdd;
		}
	}
	while( ++X < InnerX );
};


// Lit, modulated....
static void MergePass15Modulated( INT Y, INT X, INT InnerX )
{
	do
	{
		DWORD ScreenPix = ScreenDest.PtrWORD[X+0];

		ScreenDest.PtrWORD[X] =
		(  Min (  ( Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256] * (ScreenPix & 0x001f) ) >> ( 0+5), (DWORD) 31)       )+
		(  Min (  ( Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256] * (ScreenPix & 0x03E0) ) >> ( 5+5), (DWORD) 31) <<  5 )+
		(  Min (  ( Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256] * (ScreenPix & 0x7C00) ) >> (10+5+2), (DWORD) 31) << 10 );
	}
	while( ++X < InnerX );
};




/*
static void MergePass32ModulatedUnLit( INT Y, INT X, INT InnerX )
{
	do 
	{
		ScreenDest.PtrBYTE[X*4+0] = Min( (ScreenDest.PtrBYTE[X*4+0] * Photon[X+0].SB2) >> 5  , 255);
		ScreenDest.PtrBYTE[X*4+1] = Min( (ScreenDest.PtrBYTE[X*4+1] * Photon[X+0].SG2) >> 5  , 255);
		ScreenDest.PtrBYTE[X*4+2] = Min( (ScreenDest.PtrBYTE[X*4+2] * Photon[X+0].SR2) >> 5  , 255);
	}
	while( ++X < InnerX );
}
*/


static void MergePass15ModulatedUnlit( INT Y, INT X, INT InnerX )
{
	do
	{
		DWORD ScreenPix = ScreenDest.PtrWORD[X+0];

		ScreenDest.PtrWORD[X] =
		(  Min (  (Photon[X].SB2 * (ScreenPix & 0x001f) ) >> ( 0+5), (DWORD) 31)       )+
		(  Min (  (Photon[X].SG2 * (ScreenPix & 0x03E0) ) >> ( 5+5), (DWORD) 31) <<  5 )+
		(  Min (  (Photon[X].SR2 * (ScreenPix & 0x7C00) ) >> (10+5), (DWORD) 31) << 10 );
	}
	while( ++X < InnerX );
};


static void MergePass16ModulatedUnlit( INT Y, INT X, INT InnerX )
{
	do
	{
		DWORD ScreenPix = ScreenDest.PtrWORD[X];

		ScreenDest.PtrWORD[X] =
		( Min (  ( Photon[X].SB2 * (ScreenPix & 0x001f) ) >> ( 0+5), (DWORD) 31)       )+
		( Min (  ( Photon[X].SG2 * (ScreenPix & 0x07E0) ) >> ( 5+5), (DWORD) 63) <<  5 )+
		( Min (  ( Photon[X].SR2 * (ScreenPix & 0xF800) ) >> (11+5+2), (DWORD) 31) << 11 );
	}
	while( ++X < InnerX );
};



static void MergePass16Translucent( INT Y, INT X, INT InnerX )
{
	do
	{
		if (Photon[X+0].SB2 != 255) // masked ??
		{
			DWORD DestPix =
			Shade[SHADE_B  + Photon[X+0].SB2 + Photon[X+0].SB1*256]       +
			Shade[SHADE_G  + Photon[X+0].SG2 + Photon[X+0].SG1*256] *  32 +
   			Shade[SHADE_R  + Photon[X+0].SR2 + Photon[X+0].SR1*256] * 256 ;

			// Add 16-bit to 16-bit with saturation...
			_WORD ScreenPix = ScreenDest.PtrWORD[X+0];
			DWORD SumAdd = DestPix + ScreenPix;   // Sum with carries	
			DWORD Mask = ((DestPix ^ ScreenPix) ^ SumAdd) & 0x10820; // Diff carryless sums with normal sum
				  SumAdd |=  Mask - (Mask >> 5);   // Expand overflow carries
			
			ScreenDest.PtrWORD[X+0] = (_WORD) SumAdd;
		}
	}
	while( ++X < InnerX );
};


static void MergePass16Modulated( INT Y, INT X, INT InnerX )
{
	do
	{
		DWORD ScreenPix = ScreenDest.PtrWORD[X+0];

		ScreenDest.PtrWORD[X] =
		(  Min (  ( Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256] * (ScreenPix & 0x001f) ) >> ( 0+5), (DWORD) 31)       )+
		(  Min (  ( Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256] * (ScreenPix & 0x07E0) ) >> ( 5+5), (DWORD) 63)   <<  5 )+
		(  Min (  ( Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256] * (ScreenPix & 0xF800) ) >> (11+5+2), (DWORD) 31) << 11 );
	}
	while( ++X < InnerX );
};


#if !ASM

static void MergePass1516Masked( INT Y, INT X, INT InnerX )
{
	do
	{
		if (Photon[X+0].SB2 != 255) // masked ??
		{
			ScreenDest.PtrWORD[X+0] =
			Shade[SHADE_B  + Photon[X+0].SB2 + Photon[X+0].SB1*256]       +
			Shade[SHADE_G  + Photon[X+0].SG2 + Photon[X+0].SG1*256] *  32 +
   			Shade[SHADE_R  + Photon[X+0].SR2 + Photon[X+0].SR1*256] * 256 ;
		}
	}
	while( ++X < InnerX );
};




static void MergePass1516( INT Y, INT X, INT InnerX )
{
	do
	{
		ScreenDest.PtrWORD[X+0] =
		( Shade[SHADE_B  + Photon[X+0].SB2 + Photon[X+0].SB1*256]       ) +
		( Shade[SHADE_G  + Photon[X+0].SG2 + Photon[X+0].SG1*256] <<  5 ) +
  		( Shade[SHADE_R  + Photon[X+0].SR2 + Photon[X+0].SR1*256] <<  8 ) ;
	}
	while( ++X < InnerX );
};


static void MergePass1516Stippled( INT Y, INT X, INT InnerX )
{
	X += (Y ^ X) & 1;

	if (X<InnerX)
	{
		do // 32-bit color - no dithering required.
		{		
			if (Photon[X+0].SB2 != 255) // masked ??
			{
				ScreenDest.PtrWORD[X+0] =
				( Shade[SHADE_B  + Photon[X+0].SB2 + Photon[X+0].SB1*256]      ) +
				( Shade[SHADE_G  + Photon[X+0].SG2 + Photon[X+0].SG1*256] << 5 ) +
   				( Shade[SHADE_R  + Photon[X+0].SR2 + Photon[X+0].SR1*256] << 8 ) ;
			}
		}
		while( (X +=2) < InnerX );
	}
};
#endif



#if ASM
//
// Merge the texture and lighting together to form a resulting color
// and write it to the screen.
// Works in both 16-bit and 15 bit mode provided the tables were set up right.
//
// Ditherin' numbers:
//  1 & 2 are for odd/even X on even Y
//  3 & 4 are for odd/even X on odd  Y
//  1a 2a
//  1b 2b
//
// -> dithering in lightspace would seem the most useful - but dither values
//    are nontrivial since the lightrange isn't exactly like   [0.0 .. 1.0] * texturecolor
//    but more like a clamped  [0.0 .. 2.5]  *  texturecolor.
//

enum { // orig: 120 302 201  010    = RGB RGB RGB RGB dithers
	DITHERSTEP  = 1, // 

    D_R1a = 1*DITHERSTEP, // 1 3 2 0
    D_R2a = 3*DITHERSTEP,
    D_R1b = 2*DITHERSTEP,
    D_R2b = 0*DITHERSTEP,

    D_G1a = 2*DITHERSTEP, // 2 0 0 1 
    D_G2a = 0*DITHERSTEP,
    D_G1b = 0*DITHERSTEP,
    D_G2b = 1*DITHERSTEP,

    D_B1a = 0*DITHERSTEP, // 0 2 1 0
    D_B2a = 2*DITHERSTEP, 
    D_B1b = 1*DITHERSTEP,
    D_B2b = 0*DITHERSTEP,
};


static void MergePass1516( INT Y, INT X, INT InnerX )
{

	if (Y & 1)
    __asm
	{
		mov     esi, X
		mov		edi, InnerX
		//..........................
		mov     eax,ScreenDest.PtrBYTE
		mov     [SavedEBP],ebp
		mov     [SavedESP],esp
		mov		ebp,eax
		xor     eax,eax
		xor     ebx,ebx
		xor     ecx,ecx
		xor     edx,edx
		dec     edi

		// if ( X & 1 )
		test    esi,1
		jz      GoFirstEven

	//InnerTexelFirst: //  First byte..
		// Texel:

		mov     ah,Photon[esi*8].SG1
		mov     bl,Photon[esi*8].SR2

		mov     ch,Photon[esi*8].SB1
		mov     al,Photon[esi*8].SG2

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2

		inc     esi
		mov     dl, byte ptr Shade[SHADE_G+eax+D_G1a]  // green!

		mov     al, byte ptr Shade[SHADE_B+ecx+D_B1a]  //
		shl     edx,5                                  // shift green into position.

		mov     ah, byte ptr Shade[SHADE_R+ebx+D_R1a]  // blu
        add		eax,edx                                // add green..

		xor     edx,edx                                // rstore fuhur next time.
		mov     word ptr [ebp+esi*2-2],ax              //
		//...............................

	GoFirstEven:   //now:  while ( X+1 < InnerX )  ==  X < (InnerX-1) == esi < edi
		cmp    esi,edi
		jae    StartLastOdd
		//...............................

	InnerTexel_A: //  Dispatches dithered doublewords.

		mov     ah,Photon[esi*8].SG1
		mov     bl,Photon[esi*8].SR2

		mov     ch,Photon[esi*8].SB1
		mov     al,Photon[esi*8].SG2

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2

		add     esi,2
		mov     dl, byte ptr Shade[SHADE_G+eax +D_G2a] // green!
 
		shl     edx,5                                  // Shift green into position
		mov     al, byte ptr Shade[SHADE_B+ecx +D_B2a] // Raw blue

		mov     ah, byte ptr Shade[SHADE_R+ebx +D_R2a] // preshifted (!) red
		mov     esp,edx                                // 

		//...............................
        or		esp,eax                                // add green..
		xor     edx,edx                                // rstore fur next time.

		mov     ah,Photon[esi*8-8].SG1
		mov     bl,Photon[esi*8-8].SR2

		mov     ch,Photon[esi*8-8].SB1   
		mov     al,Photon[esi*8-8].SG2

		mov     bh,Photon[esi*8-8].SR1
		mov     cl,Photon[esi*8-8].SB2   

		shl     esp,16
		mov     dl, byte ptr Shade[SHADE_G+eax +D_G1a] // green!
 
		shl     edx,5                                  // Shift green into position
		mov     al, byte ptr Shade[SHADE_B+ecx +D_B1a] // Raw blue

		mov     ah, byte ptr Shade[SHADE_R+ebx +D_R1a] // preshifted (!) red
		or      esp,edx                                //  insert 

        or 		esp,eax                                // add green..
		xor     edx,edx                                // rstore fur next time.

		rol     esp,16 //?? NP ?? still worth it possibly.
		cmp     esi,edi

		mov     dword ptr [ebp+esi*2-4],esp
		jb		InnerTexel_A

		//...............................
	StartLastOdd:
		cmp		esi,edi
		ja      SkipLastOdd

	//TexelLastOdd: //  Dispatch dithered doublewords.
		mov     ah,Photon[esi*8].SG1
		mov     bl,Photon[esi*8].SR2

		mov     ch,Photon[esi*8].SB1   
		mov     al,Photon[esi*8].SG2   

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2   

		nop                                             
		mov     dl, byte ptr Shade[SHADE_G+eax +D_G2a]  // Green

		shl     edx,5                                   // Shift green into position.
		mov     al, byte ptr Shade[SHADE_B+ecx +D_B2a]  //

		nop                                             //
		mov     ah, byte ptr Shade[SHADE_R+ebx +D_R2a]  //

		nop
        add		eax,edx                                 // add green

		mov     word ptr [ebp+esi*2],ax                 //
		//...............................
	SkipLastOdd:

		mov		esp,[SavedESP]
		mov     ebp,[SavedEBP]
	}

	else // Y is even

__asm
	{
		mov     esi, X
		mov		edi, InnerX
		//..........................
		mov     eax,ScreenDest.PtrBYTE
		xor     edx,edx
		mov     [SavedEBP],ebp
		xor     ebx,ebx
		xor     ecx,ecx
		mov     [SavedESP],esp
		mov		ebp,eax
		xor     eax,eax
		dec     edi

		// if ( X & 1 )
		test    esi,1
		jz      bGoFirstEven

	//InnerTexelFirst: //  First byte..
		// Texel:

		mov     ah,Photon[esi*8].SG1
		mov     bl,Photon[esi*8].SR2

		mov     ch,Photon[esi*8].SB1   
		mov     al,Photon[esi*8].SG2

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2   

		inc     esi
		mov     dl, byte ptr Shade[SHADE_G+eax+D_G1b]  // green!

		mov     al, byte ptr Shade[SHADE_B+ecx+D_B1b]  //
		shl     edx,5                                  // shift green into position.

		mov     ah, byte ptr Shade[SHADE_R+ebx+D_R1b]  // blu
        add		eax,edx                                // add green..

		xor     edx,edx                                // rstore for next time.
		mov     word ptr [ebp+esi*2-2],ax              //  

		//...............................

	bGoFirstEven:   //now:  while ( X+1 < InnerX )  ==  X < (InnerX-1) == esi < edi
		cmp    esi,edi
		jae    bStartLastOdd

		//...............................

	bInnerTexel_A: //  Dispatch dithered doublewords.
		// Texel:

		mov     ah,Photon[esi*8].SG1
		mov     bl,Photon[esi*8].SR2

		mov     ch,Photon[esi*8].SB1
		mov     al,Photon[esi*8].SG2

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2

		add     esi,2
		mov     dl, byte ptr Shade[SHADE_G+eax +D_G2b] // green!

		shl     edx,5                                  // Shift green into position
		mov     al, byte ptr Shade[SHADE_B+ecx +D_B2b] // Raw blue

		mov     ah, byte ptr Shade[SHADE_R+ebx +D_R2b] // preshifted (!) red
		mov     esp,edx                                //

		//...............................

        or		esp,eax                                // add green..
		xor     edx,edx                                // rstore fur next time.

		mov     ah,Photon[esi*8-8].SG1
		mov     bl,Photon[esi*8-8].SR2

		mov     ch,Photon[esi*8-8].SB1
		mov     al,Photon[esi*8-8].SG2

		mov     bh,Photon[esi*8-8].SR1
		mov     cl,Photon[esi*8-8].SB2

		shl     esp,16
		mov     dl, byte ptr Shade[SHADE_G+eax +D_G1b] // green!

		shl     edx,5                                  // Shift green into position
		mov     al, byte ptr Shade[SHADE_B+ecx +D_B1b] // Raw blue

		mov     ah, byte ptr Shade[SHADE_R+ebx +D_R1b] // preshifted (!) red
		or      esp,edx                                //  insert

        or 		esp,eax                                // add green..
		xor     edx,edx                                // rstore fur next time.

		rol     esp,16 //?? NP ?? still worth it possibly.
		cmp     esi,edi

		mov     dword ptr [ebp+esi*2-4],esp
		jb		bInnerTexel_A

		//...............................

	bStartLastOdd:
		cmp		esi,edi
		ja      bSkipLastOdd

	//InnerTexelLastOdd: //  Dispatch dithered doublewords.
		// Texel:
		mov     ah,Photon[esi*8].SG1
		mov     bl,Photon[esi*8].SR2

		mov     ch,Photon[esi*8].SB1
		mov     al,Photon[esi*8].SG2

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2

		nop                                             
		mov     dl, byte ptr Shade[SHADE_G+eax +D_G2b]  // Green

		shl     edx,5                                   // Shift green into position.
		mov     al, byte ptr Shade[SHADE_B+ecx +D_B2b]  //

		nop                                             //
		mov     ah, byte ptr Shade[SHADE_R+ebx +D_R2b]  //

		nop                                             //
        add		eax,edx                                 // Add green..

		mov     word ptr [ebp+esi*2],ax                 //

		//...............................

	bSkipLastOdd:

		mov		esp,[SavedESP]
		mov     ebp,[SavedEBP]
	}
}



//
// Stippled merge pass: instead of dithering, just skip pixels in a checkerboard-pattern.
//

static void MergePass1516Stippled( INT Y, INT X, INT InnerX )
{

    __asm
	{
		mov     esi, X
		mov		edi, InnerX // inclusive.
		mov     ebx, Y
		//..........................
		mov     eax,ScreenDest.PtrBYTE
		mov     [SavedEBP],ebp
		mov     [SavedESP],esp
		mov		ebp,eax
		xor     eax,eax
		xor     ebx,esi
		xor     ecx,ecx
		xor     edx,edx
		dec     edi

		shr    ebx,1
		// if ( (X ^ Y) & 1 ) -> checkerboard stippling 
		adc    esi,0

		xor    ebx,ebx
		cmp    esi,edi
		jb     InnerTexel_A
		jmp    EndStipple1516
		

		//////////////////
		DOALIGN16
	SkipSetter:
		add		esi,2
		xor		eax,eax

		cmp		esi,edi
		ja		EndStipple1516

		mov     bl,Photon[esi*8].SR2
		mov     ah,Photon[esi*8].SG1

		cmp     bl,0xff
		je      SkipSetter
		jmp     NoSkip

		//...............................
		DOALIGN16

	InnerTexel_A: //  Dispatches dithered doublewords.
		mov     bl,Photon[esi*8].SR2
		mov     ah,Photon[esi*8].SG1

		cmp     bl,0xff
		je      SkipSetter

	NoSkip:
		mov     ch,Photon[esi*8].SB1
		mov     al,Photon[esi*8].SG2

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2 

		mov     dl, byte ptr Shade[SHADE_G+eax ] // Green!
 		add     esi,2

		shl     edx,5                            // Shift green into position
		mov     al, byte ptr Shade[SHADE_B+ecx ] // Raw blue

		mov     ah, byte ptr Shade[SHADE_R+ebx ] // Preshifted (!) red
		xor     ebx,ebx

		add     edx,eax                          
		mov     word ptr [ebp+esi*2-4],dx

		xor     edx,edx
		xor     eax,eax

		cmp     esi,edi
		jbe		InnerTexel_A

		//...............................
	EndStipple1516:

		mov		esp,[SavedESP]
		mov     ebp,[SavedEBP]
	}
}



static void MergePass1516Masked( INT Y, INT X, INT InnerX )
{
    __asm
	{
		mov     esi, X
		mov		edi, InnerX // inclusive.
		mov     ebx, Y
		//..........................
		mov     eax,ScreenDest.PtrBYTE
		mov     [SavedEBP],ebp
		mov     [SavedESP],esp

		mov		ebp,eax
		xor     eax,eax
		xor     ebx,esi
		xor     ecx,ecx
		xor     edx,edx
		dec     edi

		xor    ebx,ebx
		cmp    esi,edi
		jb     InnerTexel_A
		jmp    EndMask1516
		

		//////////////////
		DOALIGN16
	SkipSetter:
		inc     esi
		xor		eax,eax

		cmp		esi,edi
		ja		EndMask1516

		mov     bl,Photon[esi*8].SR2
		mov     ah,Photon[esi*8].SG1

		cmp     bl,0xff
		je      SkipSetter
		jmp     NoSkip

		//...............................
		DOALIGN16

	InnerTexel_A: //  Dispatches dithered doublewords.
		mov     bl,Photon[esi*8].SR2
		mov     ah,Photon[esi*8].SG1

		cmp     bl,0xff
		je      SkipSetter

	NoSkip:
		mov     ch,Photon[esi*8].SB1
		mov     al,Photon[esi*8].SG2

		mov     bh,Photon[esi*8].SR1
		mov     cl,Photon[esi*8].SB2 

		mov     dl, byte ptr Shade[SHADE_G+eax ] // Green!
 		inc		esi

		shl     edx,5                            // Shift green into position
		mov     al, byte ptr Shade[SHADE_B+ecx ] // Raw blue

		mov     ah, byte ptr Shade[SHADE_R+ebx ] // Preshifted (!) red
		xor     ebx,ebx

		add     edx,eax                          
		mov     word ptr [ebp+esi*2-2],dx // ACCESS violation ! ESI being ONE !!.... was ebp+esi*2-4 !!!

		xor     edx,edx
		xor     eax,eax

		cmp     esi,edi
		jbe		InnerTexel_A

		//...............................
	EndMask1516:

		mov		esp,[SavedESP]
		mov     ebp,[SavedEBP]
	}
};

#endif




static void MergePass32Stippled( INT Y, INT X, INT InnerX )
{
#if !ASM

	X += (Y ^ X) & 1;

	if (X<InnerX)
	{
		do // 32-bit color - no dithering required.
		{
			if (Photon[X+0].SB2 != 255)   // masked ??
			{
				ScreenDest.PtrBYTE[X*4+0] = Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256];
				ScreenDest.PtrBYTE[X*4+1] = Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256];
				ScreenDest.PtrBYTE[X*4+2] = Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256];
			}
		}
		while( (X +=2) < InnerX );
	}
	return;

#else

	// Warning: *preliminary* non-MMX optimized code for PPRo/PII

	static DWORD SavedInnerX;

	__asm
	{
		mov		edi, InnerX
		mov     esi, X
		mov     ebx, Y

		xor     ebx,esi
		shr     ebx,1
		adc     esi,0    // checkerboard offset....

		mov     [SavedInnerX],edi
		//..........................
		mov     eax,ScreenDest.PtrBYTE
		mov     [SavedEBP],ebp
		mov     [SavedESP],esp
		mov		ebp,eax
		jmp     Texel24Loop


		DOALIGN16
	Skip24Loop:
		movzx   eax,Photon[esi*8].SR2
		movzx   ebx,Photon[esi*8].SR1  

		shl     ebx,8
		cmp     al,0xff
		jne     Do24Pixel
	Skip24Pixel:
		add     esi,2
		cmp     esi,[SavedInnerX]
		//.......................
		jb		Skip24Loop
		jmp     Texel24End


		DOALIGN16
	Texel24Loop:
        //.........................
		movzx   eax,Photon[esi*8].SR2

		movzx   ebx,Photon[esi*8].SR1  
		shl     ebx,8

		cmp     al,0xff
		je      Skip24Pixel
	Do24Pixel:

		movzx   ecx,Photon[esi*8].SG1
		shl     ecx,8

		movzx   edx,Photon[esi*8].SB1
		shl     edx,8

		add     eax,ebx

		movzx   edi,Photon[esi*8].SG2
		add     edi,ecx

		movzx   esp,Photon[esi*8].SB2
		add     esi,2
		add     esp,edx

		movzx   ecx,Shade[SHADE_R + eax ]
		movzx   edx,Shade[SHADE_G + edi ]
		shl     ecx,8
		movzx   ebx,Shade[SHADE_B + esp ]
		add     ecx,edx
		shl     ecx,8
		add     ecx,ebx

		cmp     esi,[SavedInnerX]

		mov     [ebp+esi*4-4-4],ecx
		//.......................
		jb      Texel24Loop

	Texel24End:

		mov     ebp,[SavedEBP]
		mov     esp,[SavedESP]	
	}
#endif
}






static void MergePass32( INT Y, INT X, INT InnerX )
{

#if !ASM

	do // 32-bit color - no dithering required.
	{
		ScreenDest.PtrBYTE[X*4+0] = Shade[SHADE_B + Photon[X+0].SB2 + Photon[X+0].SB1*256];
		ScreenDest.PtrBYTE[X*4+1] = Shade[SHADE_G + Photon[X+0].SG2 + Photon[X+0].SG1*256];
		ScreenDest.PtrBYTE[X*4+2] = Shade[SHADE_R + Photon[X+0].SR2 + Photon[X+0].SR1*256];
	}
	while( ++X < InnerX );
	return;

#else

	// Warning: *preliminary* non-MMX optimized code for PPRo/PII 

	static DWORD SavedInnerX;

	__asm
	{
		mov		edi, InnerX
		mov     esi, X
		mov     [SavedInnerX],edi
		//..........................
		mov     eax,ScreenDest.PtrBYTE
		mov     [SavedEBP],ebp
		mov     [SavedESP],esp
		mov		ebp,eax

		DOALIGN16
	Texel24Loop:
        //.........................
		movzx   ebx,Photon[esi*8].SR1  
		shl     ebx,8

		movzx   ecx,Photon[esi*8].SG1
		shl     ecx,8

		movzx   edx,Photon[esi*8].SB1
		shl     edx,8

		movzx   eax,Photon[esi*8].SR2
		add     eax,ebx

		movzx   edi,Photon[esi*8].SG2
		add     edi,ecx

		movzx   esp,Photon[esi*8].SB2
		add     esi,1
		add     esp,edx

		movzx   ecx,Shade[SHADE_R + eax ]
		movzx   edx,Shade[SHADE_G + edi ]
		shl     ecx,8
		movzx   ebx,Shade[SHADE_B + esp ]
		add     ecx,edx
		shl     ecx,8
		add     ecx,ebx

		cmp     esi,[SavedInnerX]

		mov     [ebp+esi*4-4],ecx
		//.......................
		jb      Texel24Loop

		//Texel24End:

		mov     ebp,[SavedEBP]
		mov     esp,[SavedESP]	
	}
#endif
}

static void MergeNone( INT Y, INT X, INT InnerX )
{}






//
// All MMX Code : Rendering, Lighting, for all (Most) cases.
//

#if !ASM

static void MMX15Render8() {};
static void MMX15Render8A() {};
static void MMX15Render8B() {};
static void MMX15MaskedRender8() {};
static void MMX15ModulatedRender8() {};
static void MMX15TranslucentRender8() {};
static void MMX15StippledRender8() {};

static void MMX16Render8() {};
static void MMX16Render8A() {};
static void MMX16Render8B() {};
static void MMX16MaskedRender8() {};
static void MMX16ModulatedRender8() {};
static void MMX16TranslucentRender8() {};
static void MMX16StippledRender8() {};

static void MMX32Render8() {};
static void MMX32Render8A() {};
static void MMX32Render8B() {};
static void MMX32MaskedRender8() {};
static void MMX32ModulatedRender8() {};
static void MMX32TranslucentRender8() {};
static void MMX32StippledRender8() {};

static void MMX32FogRender8() {};
static void MMX32FogRender8A() {};
static void MMX32FogRender8B() {};
static void MMX32FogMaskedRender8() {};
static void MMX32FogModulatedRender8() {};

static void MMXLight8() {};
static void MMXFogLight8() {};
static void MMXUnlitLight8() {};
static void MMXScaledFogLight8() {};


#else

static void MMXUnlitLight8() {};

//static void MMX16FogRender8() {};
//static void MMX16Render8() {};
//static void MMX16Render8A() {};
//static void MMX16Render8B() {};
//static void MMX16MaskedRender8() {};
//static void MMX16ModulatedRender8() {};
//static void MMX16TranslucentRender8() {};
//static void MMX16StippledRender8() {};



//
// Unoptimized basic 32-bit MMX rendering included for reference.
//

static void MMX32Render8()  // Basic plain lit-texturemapper, no unrolling, no dithering.
{
	guardSlow(MMX32Render8);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	movq    mm3,[MMXFlashOffset13]			//

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////

			mov			ecx,[eax]FTexSetup.X         // get delta

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

			DOALIGN16 //#debug ?

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta

	        movq        mm5,[ebp+8]	        // next light value;    movq  mm3,[ebp +16 + 8]   = next fog value
			add         ebp,8               //
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // blit how many pixels:  min (8, NextX-X)
			jae         BiteOffEight		//
					
			pmulhw		mm5,MMXDeltaAdjust[eax*8]; // faster to shift right for default of 8 pixels....
			xor			eax,eax             //
			paddw       mm5,mm5             // #debug optim make it work WITHOUT paddw OR move this....


			//#debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
	        //
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.

			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    // 
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		
			paddw		mm0, mm3            //  ad FlashOffset
											//
			//paddw		mm0, mm2            //  Add 14-bit fog. 
			//paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   // Tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
			// EndofInnerLight8:

			// Indicator: blacken last pixel
#if INDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0 //  Store to screen.
#endif

			mov         ecx,[NextXStore]    //  Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  //  X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0   //  Store to screen.
#endif

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     //  Skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             //  New mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8					  //  This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    //  Greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X	  //  First X....
			jmp			ReloopNewSpan8            //  -2, more spans on this line.


	DOALIGN16
	BiteOffEight:
		lea         esp,[esi+8]			
		psraw		mm5,3
		xor			eax,eax
		jmp ReloopInnerRender8

	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8  // skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}
	unguardSlow;

}// MMX32Render8



//
// Unoptimized basic 15-bit MMX rendering included for reference.
//

static void MMX15Render8()  // Basic plain lit-texturemapper, no unrolling, no dithering.
{
	guardSlow(MMX15Render8);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//

	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////

			mov			ecx,[eax]FTexSetup.X         // get delta

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16 

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // Blit how many pixels:  min (8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			xor         eax,eax
			paddw       mm5,mm5
			
	ReloopInnerRender8:  // Loop using current light & light delta's. 		
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc         esi

			paddw		mm0, mm3            //  ad FlashOffset

			//paddw		mm2, [Fog]          //  Add 14-bit fog. 
			//paddw		[Fog],[FogDelta]    //  Add FogDelta.

			psraw       mm0,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!

			movq        mm2, mm0				// copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0
			mov 		word ptr [edi+esi*2 - 1*2], ax	// Store 15 bit value to screen...

			xor         eax,eax					// !! This is no good for PII, must be XOR -> clashes with jb...
												//
			cmp			esi,esp					//	   cmp  X, PX;  always runs 8 times.		
			jb			ReloopInnerRender8		//
												//
	//////////////////////////////////////////////
		   // Tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0

			//EndInnerRender8:
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
																			
			//EndofInnerTex8:

			// Indicator: blacken last pixel
#if INDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*2 - 1*2], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*2 - 1*2], mm0    //  store to screen.
#endif

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	DOALIGN16
	BiteOffEight:
		lea         esp,[esi+8]			
		psraw		mm5,3
		xor			eax,eax
		jmp ReloopInnerRender8


	DOALIGN16
	NegativeStart:		//  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	//  skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15Render8()





/*
static void MMX15FogRender8()  // Basic plain lit-texturemapper, no unrolling, no dithering.
{
	guardSlow(MMX15FogRender8);

	//#debug just change these globals to suit 15 or 16 bit conversion

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx
											//
	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
											//
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////

			mov			ecx,[eax]FTexSetup.X         // get delta

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // Save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm2,[ebp+8]         // Start fog value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

			DOALIGN16 //#debug ?

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta

	        movq        mm5,[ebp+16]	    // Next light value;    
			movq		mm3,[ebp+16 + 8 ]	// Next fog value.

			add         ebp,16              // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm3,mm2             // ,, fog delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // Blit how many pixels:  min (8, NextX-X)
			jae         BiteOffEight		//
					
			pmulhw		mm3,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels.
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels.
			xor			eax,eax             //
			paddw       mm3,mm3             // #debug Optim make it work WITHOUT paddw OR move this...
			paddw       mm5,mm5             // #debug Optim make it work WITHOUT paddw OR move this...
			movq        [MMXTempFogDelta],mm3 //


			//#debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 

			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    // 
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		

			paddw		mm0, mm2             //  Add 14-bit fog. 
			paddw		mm2,[MMXTempFogDelta]  //  mm3               //  Add FogDelta.
											 //
			psraw       mm0,FinalShift       //  14-bit result to 9 bits   
			packuswb    mm0,mm0              //  Pack words to bytes

			// Pack to 15-bits
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			movq        mm3, mm0				// copy result
			pand		mm0, [MMX15REDBLUE]	// mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]	// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm3, [MMX15GREEN]	// mask out all but the 5MSBits of green
			por			mm0, mm3				// combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// shift to final position
			movd        eax,mm0                 //
			mov			word ptr [edi+esi*2-1*2],ax // Store 15 bit value to screen...
			mov			eax,0					// !! 		

			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   // Tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
			// EndofInnerLight8:

			// #debug blacken last pixel
#if INDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*2 - 1*2 ], mm0  //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*2 - 1*2 ], mm0    //  store to screen.
#endif

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16					// This is for the extra light/fog values at the end of EVERY span .
			 
			cmp         ecx,-3                  // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	DOALIGN16
	BiteOffEight:
		lea         esp,[esi+8]			
		psraw		mm3,3
		psraw		mm5,3
		xor			eax,eax
		movq        [MMXTempFogDelta],mm3 //
		jmp ReloopInnerRender8

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}
	unguardSlow;

}// MMX15FogRender8()
*/






//
// Unoptimized basic 15-bit MMX rendering template suitable for 2x2 colordither/oversampling kernels.
//
static void MMX15Render8Cool()  // Basic plain lit-texturemapper, no unrolling
{
	guardSlow(MMX15Render8Cool);

	//static DWORD FakeESP;

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			//mov         ecx,[NX]
			//mov         [FakeESP],ecx

			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	FromBiteOffEight:						//
											//
	//////////////////////////////////////////
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			//sub		[FakeESP],1			//
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	// InnerRender_1:						//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3            //  ad FlashOffset

			//paddw		mm2, [Fog]          //  Add 14-bit fog. 
			//paddw		[Fog],[FogDelta]    //  Add FogDelta.

			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX15DitherOddA]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			cmp         esi,esp                   //
			//cmp         esi,[FakeESP]			  //
			ja          SkipInnerRender0_         //
			je          InnerRender0_			  //
												  //
	////////////////////////////////////////////////

	DOALIGN16 
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//			
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd       mm1,[KnightA0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.

			//paddw		mm2, [Fog]          //  Add 14-bit fog. 
			//paddw		[Fog],[FogDelta]    //  Add FogDelta. 

			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

	//////////////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				//  ad FlashOffset.

			// paddw		mm2, [Fog]          //  Add 14-bit fog. 
			// paddw		[Fog],[FogDelta]    //  Add FogDelta.

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0,[MMX15DitherEvenA]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
												//
			add         esi,2                   //
			cmp			esi,esp					//
			//cmp		esi,FakeESP				// Cmp  X, PX;  always runs 8 times.
			jb			ReloopInnerRender01		//
			// lead-out //

			//////////////////////////////////////
			ja			SkipInnerRender0_		//

	//////////////////////////////////////////////

	InnerRender0_:
			movq        mm1,mm7     		//
			xor         eax,eax				//
			paddd       mm1,[KnightA0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				// .
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  ad FlashOffset.

			// paddw		mm2, [Fog]          //  Add 14-bit fog. 
			// paddw		[Fog],[FogDelta]    //  Add FogDelta.

			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	DOALIGN16
	BiteOffEight:
		lea         esp,[esi+8]			
		//lea       ecx,[esi+8]
		//mov       [FakeESP],ecx

		psraw		mm5,3
		jmp			FromBiteOffEight //#optim put unrolled version here !

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15Render8Cool()







static void MMX15Render8A()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15Render8A);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
			lea			ebx,[ebx]
			lea         esi,[esi]
	//////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			//FromBiteOffEight:				//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !....
			test	esi,1					//
			paddw       mm5,mm5				//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
											//
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX15DitherOddA]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //
												  //
			// lead-in coordinate calculation.... //
			// moved up !
			movq        mm1,mm7				//
			paddd       mm1,[KnightA0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
			//////////////////////////////////
			movq        mm1, mm7				//
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			paddd       mm1,[KnightA1]		//
			movq		mm0, mm4			//  Copy lightvalue.
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12				//  

			paddd		mm7,mm6				//  texture delta
			punpckhdq   mm1,mm1				//  copy high word to low word 

			psrlq       mm1,[MMXCoShift]	// 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			pand        mm1,[MMXCoMask]		// 
			paddw		mm0, mm3            //  Ad FlashOffset.

			movd        ecx,mm1				// 
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////////
			xor			eax,eax					//
			movq		mm2, mm4				//  Copy lightvalue.

			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			movq        mm1, mm7				//

			paddd       mm1, [KnightA0]			//
			paddd		mm7, mm6				//
			
			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1, 12					//  

			paddw		mm4, mm5				// + mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				// Ad FlashOffset.

			psraw       mm2,FinalShift			// 14-bit result to 9 bits   
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			paddusb     mm0, [MMX15DitherEvenA] //  
			punpckhdq   mm1, mm1				// Copy high word to low word.

			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			psrlq       mm1, [MMXCoShift]		// 

			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green

			pand        mm1, [MMXCoMask]		// 
			por			mm0, mm2				// Combine the red, green, and blue bits

			xor			eax, eax				//
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movd        ecx, mm1				//

			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			//////////////////////////////////////												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//

			//lead-out //

			psubd		mm7,mm6					// undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3				//  ad FlashOffset.
			psraw       mm0,FinalShift			//  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				  // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenA]   //		
			movq        mm2, mm0				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.

			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green.
			por			mm0, mm2				  // Combine the red, green, and blue bits.

			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
										    //
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15Render8A()



static void MMX15Render8B()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15Render8B);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
			lea			ebx,[ebx]
			lea         esi,[esi]
	//////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			//FromBiteOffEight:				//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !....
			test	esi,1					//
			paddw       mm5,mm5				//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
											//
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightB1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX15DitherOddB]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //
												  //
			// lead-in coordinate calculation.... //
			// moved up !
			movq        mm1,mm7				//
			paddd       mm1,[KnightB0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
			//////////////////////////////////
			movq        mm1, mm7				//
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			paddd       mm1,[KnightB1]		//
			movq		mm0, mm4			//  Copy lightvalue.
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12				//  

			paddd		mm7,mm6				//  texture delta
			punpckhdq   mm1,mm1				//  copy high word to low word 

			psrlq       mm1,[MMXCoShift]	// 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			pand        mm1,[MMXCoMask]		// 
			paddw		mm0, mm3            //  Ad FlashOffset.

			movd        ecx,mm1				// 
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////////
			xor			eax,eax					//
			movq		mm2, mm4				//  Copy lightvalue.

			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			movq        mm1, mm7				//

			paddd       mm1, [KnightB0]			//
			paddd		mm7, mm6				//
			
			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1, 12					//  

			paddw		mm4, mm5				// + mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				// Ad FlashOffset.

			psraw       mm2,FinalShift			// 14-bit result to 9 bits   
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			paddusb     mm0, [MMX15DitherEvenB] //  
			punpckhdq   mm1, mm1				// Copy high word to low word.

			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			psrlq       mm1, [MMXCoShift]		// 

			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green

			pand        mm1, [MMXCoMask]		// 
			por			mm0, mm2				// Combine the red, green, and blue bits

			xor			eax, eax				//
			psrld		mm0, MMX15RGBSHIFT 		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movd        ecx, mm1				//

			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			//////////////////////////////////////												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//

			//lead-out //

			psubd		mm7,mm6					// undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3				//  ad FlashOffset.
			psraw       mm0,FinalShift			//  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				  // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenB]   //		
			movq        mm2, mm0				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.

			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green.
			por			mm0, mm2				  // Combine the red, green, and blue bits.

			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
										    //
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15Render8B()





static void MMX15FogRender8CoolA()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15FogRender8CoolA);


	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]         // start fog value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
	        movq        mm2,[ebp+16+8]	        // Next fog   value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2				//
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerFogRender	//
											//
	//////////////////////////////////////////
	InnerFogRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FOG
			paddw       mm3,[MMXTempFogDelta]     // update fog

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX15DitherOddA]	  // 
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerFogRender:					  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerFogRender0_      //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd       mm1,[KnightA0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerFogRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerFogRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenA]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
												//
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd       mm1, [KnightA0]			//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerFogRender01		//
			//lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerFogRender0_		//
	//////////////////////////////////////////////

	InnerFogRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerFogRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
										//
		psraw	mm2,3					//
		psraw	mm5,3					//
		movq    [MMXTempFogDelta],mm2		//
										//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerFogRender	//
		nop								//
		jmp		InnerFogRender_1        //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15FogRender8CoolA()



static void MMX15FogRender8CoolB()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15FogRender8CoolB);


	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]         // start fog value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
	        movq        mm2,[ebp+16+8]	        // Next fog   value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2				//
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerFogRender	//
											//
	//////////////////////////////////////////
	InnerFogRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightB1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FOG
			paddw       mm3,[MMXTempFogDelta]     // update fog

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX15DitherOddB]	  // 
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerFogRender:					  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerFogRender0_      //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd       mm1,[KnightB0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerFogRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerFogRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd       mm1,[KnightB1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenB]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
												//
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd       mm1, [KnightB0]			//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerFogRender01		//
			//lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerFogRender0_		//
	//////////////////////////////////////////////

	InnerFogRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerFogRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
										//
		psraw	mm2,3					//
		psraw	mm5,3					//
		movq    [MMXTempFogDelta],mm2		//
										//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerFogRender	//
		nop								//
		jmp		InnerFogRender_1        //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15FogRender8CoolB()






static void MMX15MaskedRender8A()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15MaskedRender8A);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX15DitherOddA]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			movq        mm1,mm7				//
			paddd		mm7,mm6				//

			psllq       mm1,12				//  
			test		eax,eax

			punpckhdq   mm1,mm1				// copy high word to low word 
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			xor         eax,eax

			psrlq       mm1,[MMXCoShift]	// 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			pand        mm1,[MMXCoMask]		// 
			paddw		mm0, mm3            //  Ad FlashOffset.

			psraw       mm0,FinalShift      //  14-bit result to 9 bits.
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 

			movq        mm1, mm7				//
			paddd		mm7, mm6				//

			xor			eax, eax				//
			psllq       mm1, 12					//  

			punpckhdq   mm1, mm1				// Copy high word to low word.
			paddw		mm2, mm3				//  ad FlashOffset.

			psrlq       mm1, [MMXCoShift]		// 
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			pand        mm1, [MMXCoMask]		// 
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!

			paddusb     mm0, [MMX15DitherEvenA]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue

			movd        ecx, mm1				//
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Next coordinate calculation....
			
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			//movq        mm1,mm7				//
			xor			eax,eax				//
			//paddd		mm7,mm6				//
			//psllq       mm1,12				//  
			//punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX15DitherOddA]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.
			movd        eax,mm0					  //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15MaskedRender8A()







static void MMX15MaskedRender8B()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15MaskedRender8B);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX15DitherOddB]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			test		eax,eax
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				//  ad FlashOffset.

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenB]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX15DitherOddB]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.
			movd        eax,mm0			  	    //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15MaskedRender8B()







static void MMX15FogMaskedRender8A()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15FogMaskedRender8A);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]			// Fog start value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			movq        mm2,[ebp+16+8]      // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16              // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3             // fog delta calculation
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2  
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset

			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX15DitherOddA]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT	      // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			test		eax,eax
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenA]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		psraw	mm2,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		movq    [MMXTempFogDelta],mm2
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX15DitherOddA]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.
			movd        eax,mm0					  //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15FogMaskedRender8A()

static void MMX15FogMaskedRender8B()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX15FogMaskedRender8B);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]			// Fog start value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			movq        mm2,[ebp+16+8]      // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16              // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3             // fog delta calculation
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2  
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset

			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX15DitherOddB]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			test		eax,eax
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX15DitherEvenB]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		psraw	mm2,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		movq    [MMXTempFogDelta],mm2
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX15DitherOddB]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX15DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.
			movd        eax,mm0			  	    //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX15FogMaskedRender8B()




static void MMX15StippledRender8()
{
	guardSlow(MMX15StippledRender8);

	// checkerboard: defined by bit 0 of X  xor bit 0 of Y !
	
	__asm {

	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....

	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

	//
	//	Prestep ?
	//
	//	test     ebx,ebx		// Parity
	//	jnz      NoPrestep		//
	//	add      esi,1
	//	NoPrestep:
	//

	DOALIGN16
	nop
	nop
	nop

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP = [0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // movq    MMXCoMask,  mm4              
	movq    mm3,[MMXFlashOffset13]			//

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]			// movq        mm2,[ebp+8] get inital fog value...
			paddd       mm6,mm6             // double the tex delta.
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 

			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // Copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,mm2             //    
			movd        ecx,mm1             //  

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // Next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // New minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // Need different delta scaling

			psraw       mm5,2			    // 3   but double delta for stippling 
			lea			esp,[esi+8]         // just do those last 1-7 ones...

	ReloopInnerStippledRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.

			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! Compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //   

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			punpckhdq   mm1, mm1            //  Copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			//  proceed through texture 

			paddw       mm0, mm3            //
			pand        mm1,mm2             //  

			psraw       mm0, FinalShift     //  14-bit result to 9 bits.
			add 		esi,2               //  ++X

			movd        ecx,mm1             //  
			packuswb    mm0, mm0            //  Pack words to bytes

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			// convert mm0 to 15-bits.....
			movq        mm1, mm0				  // Copy.
			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm1, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm1				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			//
			movd        eax,mm0
			mov  		[edi+esi*2 - 2*2], ax    //  Store to screen. // unavoidable delay, Upipe..
			//
			jb			ReloopInnerStippledRender8  //   Unpairable, Upipe

	EndLoopInnerStippledRender8: 
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd       mm7,mm6             // undo preliminary delta-add from inner loop

			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                   // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8           // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8					// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                  // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	DOALIGN16

	SkipMaskedPixel: // don't draw, update light & coordinates using delta's...
			psllq       mm1, 12             //   
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			punpckhdq   mm1, mm1            //  Copy high word to low word 
			paddd       mm7, mm6			//

			psrlq       mm1,[MMXCoShift]    //  

			add 		esi,2               //  ++X
			pand        mm1,mm2             //  

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             // 
			jb			ReloopInnerStippledRender8  //

			nop
			jmp			EndLoopInnerStippledRender8 //

						
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	//	Same as above, but most probably the last stretch of a span.
	//
	///////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler

			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			psllw       mm5,2						// *2, *2 to double delta for stippling 
			jmp			ReLoopInnerStippledRender8       

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}
// MMX15StippledRender8




static void MMX15FogStippledRender8()
{
	guardSlow(MMX15FogStippledRender8);

	// checkerboard: defined by bit 0 of X  xor bit 0 of Y !
	
	__asm {

	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....

	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

	//
	//	Prestep ?
	//
	//	test     ebx,ebx		// Parity
	//	jnz      NoPrestep		//
	//	add      esi,1
	//	NoPrestep:
	//

	DOALIGN16
	nop
	nop
	nop

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP = [0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // movq    MMXCoMask,  mm4              
	movq    MMXCoMask,  mm2                 //

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]			// movq        mm2,[ebp+8] get inital light value...
			movq		mm2,[ebp+8]			// movq        mm2,[ebp+8] get inital fog value...

			paddd       mm6,mm6             // double the tex delta.
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 

			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // Copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             //  

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	    // Next light value
			movq        mm3,[ebp +16+8]	    // Next light value
			add         ebp,16				//

			psubsw      mm5,mm4			    // New minus current = light delta
			psubsw      mm3,mm2			    // New minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // Need different delta scaling

			psraw       mm5,2			    // 3 but double delta for stippling 
			psraw       mm3,2			    // 3 but double delta for stippling 
			lea			esp,[esi+8]         // just do those last 1-7 ones...

	ReloopInnerFogStippledRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.

			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! Compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //   

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			punpckhdq   mm1, mm1            //  Copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			//  proceed through texture 

			paddw       mm0, mm2            // Fog...            
			pand        mm1,[MMXCoMask]     //  
			paddw       mm2, mm3

			psraw       mm0, FinalShift     //  14-bit result to 9 bits.
			add 		esi,2               //  ++X

			movd        ecx,mm1             //  
			packuswb    mm0, mm0            //  Pack words to bytes

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			// convert mm0 to 15-bits.....
			movq        mm1, mm0				  // Copy.
			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm1, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm1				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			//
			movd        eax,mm0
			mov  		[edi+esi*2 - 2*2], ax     //  Store to screen. // unavoidable delay, Upipe..
			//
			jb			ReloopInnerFogStippledRender8  //   Unpairable, Upipe

	EndLoopInnerFogStippledRender8:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd       mm7,mm6             // undo preliminary delta-add from inner loop

			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                   // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8           // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16					// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                  // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	DOALIGN16

	SkipMaskedPixel: // don't draw, update light & coordinates using delta's...
			psllq       mm1, 12             //   
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			punpckhdq   mm1, mm1            //  Copy high word to low word 
			paddd       mm7, mm6			//

			psrlq       mm1,[MMXCoShift]    //  
			paddw       mm2,mm3

			add 		esi,2               //  ++X
			pand        mm1,[MMXCoMask]     //  

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             // 
			jb			ReloopInnerFogStippledRender8  //

			nop
			jmp			EndLoopInnerFogStippledRender8 //

						
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	//	Same as above, but most probably the last stretch of a span.
	//
	///////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // Delta scaler
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			psllw       mm3,2                       // *2 *2 to double delta for stippling
			psllw       mm5,2						// *2, *2 to double delta for stippling 
			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			jmp			ReLoopInnerFogStippledRender8       

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}
// MMX15FogStippledRender8




//
// Full hicolor translucency (always slow...) but with inner loop of 32-bit aligned writes AND reads..
// 

static void MMX15TranslucentRender8()  
{
	guardSlow(MMX15TranslucentRender8);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm3 to unpack 15-bit to 15,15,15 FMMX 

			movq        mm3,mm2			// Low one filled ...

			psllq       mm2,11			//
			por         mm3,mm2			//

			psllq       mm2,11			//
			por         mm3,mm2			// all 3 colors filled ...
			
			//////////////////////////////
			xor         eax,eax

			movq		mm2, mm4				//  Copy lightvalue
			mov          al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 

			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.

			psllw       mm3,11			// to 16bit 
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 
			psrlw       mm3,4           // avoid sign + scaledown

			paddw		mm2, mm3				//  ad FlashOffset + screen
			psraw       mm2, FinalShift			//  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16  
	ReloopInnerRender01:					// Loop using current light & light delta's. 		

			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			///////////// GET screen value...& unpack the coordinate ...
			//  unpack 2 16-bit values into 2 24-bit values in mm1 !
			// making 16:16:16:16 -bit is faster though:

			movd		mm2,[edi+esi*2] // get 2x source word
			movq        mm1,mm7				//

			// unpack to high and low dword and unpack coordinate...
			punpcklwd   mm2,mm2    //  0,0 : 16a,16b ->  16a 16a , 16b 16b -> want: 0888 0888 format !
			xor			eax,eax				//

			pslld       mm2,3	   //
			paddd		mm7,mm6				//

			psllq       mm1,12				//  
			movq        mm3,mm2    //  low 8 OK: Red

			pand        mm3,[MMX15CleanRed]
			psrad       mm2,3 + 5  //  erase red

			punpckhdq   mm1,mm1				// copy high word to low word 
			pslld       mm2,8 + 3  //  low 8:0 = Green

			psrlq       mm1,[MMXCoShift]	// 
			por         mm3,mm2

			pand        mm1,[MMXCoMask]		// 
			psrad       mm2,8 + 3 + 5	// Erase Green

			pand        mm3,[MMX15CleanGreenRed]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			movd        ecx,mm1				// 
			pslld       mm2,8 + 8 + 3	// 

			por         mm3,mm2
			////////////////////////
			//////////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

			movq		mm2, mm4				//  Copy lightvalue.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			movq        mm1, mm7				//
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			// interleaved with coordinate calculation...

			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddd		mm7, mm6				//

			paddusb     mm0, mm3                // SCREEN value as unpacked...
			psllq       mm1, 12					//  

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue

			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			punpckhdq   mm1, mm1				// Copy high word to low word.

			psrlq       mm1, [MMXCoShift]		// 
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits

			pand        mm1, [MMXCoMask]		// 
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			xor			eax, eax				//

			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			//lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:

			///////////////////////////////
			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm3 to unpack 15-bit to 15,15,15 FMMX 
			movq        mm3,mm2			// Low one filled ...

			psllq       mm2,11			//
			por         mm3,mm2			//

			psllq       mm2,11			//
			por         mm3,mm2			// all 3 colors filled ...
			
			//////////////////////////////
			xor         eax, eax

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al,  byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			psllw       mm3, 11				// to 16bit 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			psrlw       mm3, 4				// avoid sign + scaledown

			paddw		mm0, mm3            //  ad FlashOffset.
			psraw       mm0, FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX15RGBSHIFT		// Shift to final position.

			movd        eax, mm0				  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi, 1					  //

			paddd       mm7, mm6				  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX15TranslucentRender8()





//
// Full hicolor MODULATION (always slow...) 
// 

static void MMX15ModulatedRender8()  
{
	guardSlow(MMX15ModulatedRender8);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
	//////////////////////////////////////////
	DOALIGN16

	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			///////////////////////////////
			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 

			movq        mm1,mm2			// Low one filled ...

			psllq       mm2,11			//
			por         mm1,mm2			//

			psllq       mm2,11			//
			por         mm1,mm2			// all 3 colors filled ...

			psllw       mm1,11			// to 16bit 
			xor         eax,eax
			psrlw       mm1,1           // avoid sign
			//////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov          al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 

			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, mm1				//  Modulate  #debug1  scaling !
			psraw       mm2, FinalShift - 1     //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack result
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
			xor			eax,eax					  //
			cmp         esi,esp                   //
			jb          InnerRender_1             //

	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX15ModulatedRender8()



//
// Full hicolor FogModulATION (always slow...) 
// 

static void MMX15FogModulatedRender8()  
{
	guardSlow(MMX15FogModulatedRender8);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			movq        mm2,[ebp+16+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16                // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3				// New minus current = fog delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2				//
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 
											//
	DOALIGN16
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			///////////////////////////////
			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 

			movq        mm1,mm2			// Low one filled ...

			psllq       mm2,11			//
			por         mm1,mm2			//

			psllq       mm2,11			//
			por         mm1,mm2			// all 3 colors filled ...

			psllw       mm1,11			// to 16bit 
			xor         eax,eax
			psrlw       mm1,1           // avoid sign
			//////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov          al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 

			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, mm1				//  FogModulate  #debug1  scaling !

			movq        mm0, mm3
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0, FinalShift

			psraw       mm2, FinalShift - 1     //  14-bit result to 9 bits   

			paddw       mm2,mm0 // final-shifted fog value...

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack result
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX15REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX15MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX15GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX15RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
			cmp         esi,esp                   //
			jb          InnerRender_1             //

	// SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm2,3					//
		psraw	mm5,3					//
		movq    [MMXTempFogDelta],mm2   //
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX15FogModulatedRender8()



/*
       ALL 16 -bit MMX rendering routines: note: identical to 15-bit, change 'MMX16' to 'MMX16'
*/


static void MMX16Render8A()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16Render8A);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
			lea			ebx,[ebx]
			lea         esi,[esi]
	//////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			//FromBiteOffEight:				//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !....
			test	esi,1					//
			paddw       mm5,mm5				//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
											//
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX16DitherOddA]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //
												  //
			// lead-in coordinate calculation.... //
			// moved up !
			movq        mm1,mm7				//
			paddd       mm1,[KnightA0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
			//////////////////////////////////
			movq        mm1, mm7				//
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			paddd       mm1,[KnightA1]		//
			movq		mm0, mm4			//  Copy lightvalue.
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12				//  

			paddd		mm7,mm6				//  texture delta
			punpckhdq   mm1,mm1				//  copy high word to low word 

			psrlq       mm1,[MMXCoShift]	// 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			pand        mm1,[MMXCoMask]		// 
			paddw		mm0, mm3            //  Ad FlashOffset.

			movd        ecx,mm1				// 
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.


			//////////////////////////////////////
			xor			eax,eax					//
			movq		mm2, mm4				//  Copy lightvalue.

			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			movq        mm1, mm7				//

			paddd       mm1, [KnightA0]			//
			paddd		mm7, mm6				//
			
			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1, 12					//  

			paddw		mm4, mm5				// + mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				// Ad FlashOffset.

			psraw       mm2,FinalShift			// 14-bit result to 9 bits   
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			paddusb     mm0, [MMX16DitherEvenA] //  
			punpckhdq   mm1, mm1				// Copy high word to low word.

			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			psrlq       mm1, [MMXCoShift]		// 

			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green

			pand        mm1, [MMXCoMask]		// 
			por			mm0, mm2				// Combine the red, green, and blue bits

			xor			eax, eax				//
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//

			movd        ecx, mm1				//

			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			//////////////////////////////////////												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//

			//lead-out //

			psubd		mm7,mm6					// undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3				//  ad FlashOffset.
			psraw       mm0,FinalShift			//  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				  // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenA]   //		
			movq        mm2, mm0				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.

			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green.
			por			mm0, mm2				  // Combine the red, green, and blue bits.

			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
										    //
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16Render8A()



static void MMX16Render8B()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16Render8B);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
			lea			ebx,[ebx]
			lea         esi,[esi]
	//////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			//FromBiteOffEight:				//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !....
			test	esi,1					//
			paddw       mm5,mm5				//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
											//
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightB1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX16DitherOddB]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //
												  //
			// lead-in coordinate calculation.... //
			// moved up !
			movq        mm1,mm7				//
			paddd       mm1,[KnightB0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
			//////////////////////////////////
			movq        mm1, mm7				//
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			paddd       mm1,[KnightB1]		//
			movq		mm0, mm4			//  Copy lightvalue.
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12				//  

			paddd		mm7,mm6				//  texture delta
			punpckhdq   mm1,mm1				//  copy high word to low word 

			psrlq       mm1,[MMXCoShift]	// 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			pand        mm1,[MMXCoMask]		// 
			paddw		mm0, mm3            //  Ad FlashOffset.

			movd        ecx,mm1				// 
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////////
			xor			eax,eax					//
			movq		mm2, mm4				//  Copy lightvalue.

			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			movq        mm1, mm7				//

			paddd       mm1, [KnightB0]			//
			paddd		mm7, mm6				//
			
			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1, 12					//  

			paddw		mm4, mm5				// + mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				// Ad FlashOffset.

			psraw       mm2,FinalShift			// 14-bit result to 9 bits   
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			paddusb     mm0, [MMX16DitherEvenB] //  
			punpckhdq   mm1, mm1				// Copy high word to low word.

			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			psrlq       mm1, [MMXCoShift]		// 

			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green

			pand        mm1, [MMXCoMask]		// 
			por			mm0, mm2				// Combine the red, green, and blue bits

			xor			eax, eax				//
			psrld		mm0, MMX16RGBSHIFT 		// Shift to final position.

			//packssdw    mm0,mm0				// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//

			movd        ecx, mm1				//

			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			//////////////////////////////////////												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//

			//lead-out //

			psubd		mm7,mm6					// undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3				//  ad FlashOffset.
			psraw       mm0,FinalShift			//  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				  // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenB]   //		
			movq        mm2, mm0				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.

			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green.
			por			mm0, mm2				  // Combine the red, green, and blue bits.

			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
										    //
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16Render8B()





static void MMX16FogRender8CoolA()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16FogRender8CoolA);


	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]         // start fog value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
	        movq        mm2,[ebp+16+8]	        // Next fog   value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2				//
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerFogRender	//
											//
	//////////////////////////////////////////
	InnerFogRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FOG
			paddw       mm3,[MMXTempFogDelta]     // update fog

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX16DitherOddA]	  // 
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerFogRender:					  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerFogRender0_      //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd       mm1,[KnightA0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerFogRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerFogRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd       mm1,[KnightA1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenA]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
												//
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd       mm1, [KnightA0]			//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerFogRender01		//
			//lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerFogRender0_		//
	//////////////////////////////////////////////

	InnerFogRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerFogRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
										//
		psraw	mm2,3					//
		psraw	mm5,3					//
		movq    [MMXTempFogDelta],mm2		//
										//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerFogRender	//
		nop								//
		jmp		InnerFogRender_1        //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16FogRender8CoolA()



static void MMX16FogRender8CoolB()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16FogRender8CoolB);


	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]         // start fog value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
	        movq        mm2,[ebp+16+8]	        // Next fog   value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2				//
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerFogRender	//
											//
	//////////////////////////////////////////
	InnerFogRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd       mm1,[KnightB1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FOG
			paddw       mm3,[MMXTempFogDelta]     // update fog

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// Pack to 15-bits:	
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
	
			paddusb     mm2, [MMX16DitherOddB]	  // 
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerFogRender:					  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerFogRender0_      //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd       mm1,[KnightB0]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//

			je          InnerFogRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16 
	ReloopInnerFogRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd       mm1,[KnightB1]		//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 

			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenB]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//

												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
												//
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd       mm1, [KnightB0]			//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerFogRender01		//
			//lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerFogRender0_		//
	//////////////////////////////////////////////

	InnerFogRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi,1					  //

			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerFogRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
										//
		psraw	mm2,3					//
		psraw	mm5,3					//
		movq    [MMXTempFogDelta],mm2		//
										//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerFogRender	//
		nop								//
		jmp		InnerFogRender_1        //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16FogRender8CoolB()






static void MMX16MaskedRender8A()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16MaskedRender8A);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX16DitherOddA]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			movq        mm1,mm7				//
			paddd		mm7,mm6				//

			psllq       mm1,12				//  
			test		eax,eax

			punpckhdq   mm1,mm1				// copy high word to low word 
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			xor         eax,eax

			psrlq       mm1,[MMXCoShift]	// 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			pand        mm1,[MMXCoMask]		// 
			paddw		mm0, mm3            //  Ad FlashOffset.

			psraw       mm0,FinalShift      //  14-bit result to 9 bits.
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 

			movq        mm1, mm7				//
			paddd		mm7, mm6				//

			xor			eax, eax				//
			psllq       mm1, 12					//  

			punpckhdq   mm1, mm1				// Copy high word to low word.
			paddw		mm2, mm3				//  ad FlashOffset.

			psrlq       mm1, [MMXCoShift]		// 
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			pand        mm1, [MMXCoMask]		// 
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!

			paddusb     mm0, [MMX16DitherEvenA]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue

			movd        ecx, mm1				//
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Next coordinate calculation....
			
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			//movq        mm1,mm7				//
			xor			eax,eax				//
			//paddd		mm7,mm6				//
			//psllq       mm1,12				//  
			//punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX16DitherOddA]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
			movd        eax,mm0					  //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16MaskedRender8A()







static void MMX16MaskedRender8B()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16MaskedRender8B);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX16DitherOddB]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			test		eax,eax
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				//  ad FlashOffset.

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenB]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX16DitherOddB]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
			movd        eax,mm0			  	    //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16MaskedRender8B()







static void MMX16FogMaskedRender8A()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16FogMaskedRender8A);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]			// Fog start value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			movq        mm2,[ebp+16+8]      // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16              // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3             // fog delta calculation
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2  
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset

			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX16DitherOddA]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT	      // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			test		eax,eax
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenA]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		psraw	mm2,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		movq    [MMXTempFogDelta],mm2
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX16DitherOddA]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenA] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
			movd        eax,mm0					  //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16FogMaskedRender8A()

static void MMX16FogMaskedRender8B()  // Cool plain-lit texturemapper. Inner cycle aligned, for dithering purp'ses.
{
	guardSlow(MMX16FogMaskedRender8B);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]			// Fog start value
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			movq        mm2,[ebp+16+8]      // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16              // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3             // fog delta calculation
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2  
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 //
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
						
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test        eax,eax

			jz          SkipMasked_1		
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			paddw		mm2, mm3            //  ad FlashOffset

			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 1 pixel....
	
			paddusb     mm2, [MMX16DitherOddB]	  //
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												 
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...

			SkipMasked_1:
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////
											//
	DOALIGN16								//
	ReloopInnerRender01:					// Loop using current light & light delta's. 		
											//						 
			//////////////////////////////////
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		
			
			test		eax,eax
			jz          SkipMasked01a		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			paddw		mm0, mm3            //  Ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			test        eax,eax
			jz			SkipMasked01b

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm2, mm3				//  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]

			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddusb     mm0, [MMX16DitherEvenB]  //

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0					// Pack  words XX AA XX BB  into lower  dword   AA BB
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//
												//
			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			
			// Coordinate calculation....
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:
			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas.
			test        eax,eax
			jz          SkipMasked0_

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm0, mm3            //  ad FlashOffset.
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0,FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				//
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			
			SkipMasked0_:
			add         esi,1					  //
			paddd       mm7,mm6					  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		psraw	mm2,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		movq    [MMXTempFogDelta],mm2
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////


	DOALIGN16
	// Masking: cannot jump into KERNEL for pixel-0-masked, since no double stuff to do !!!
	SkipMasked01a: // create coordinate for 2nd pixel, see if masked also/ if NOT, handle here too:

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			//////////////////////////////////
			movq        mm1,mm7				//
			xor			eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 		

			test        eax,eax
			jz			SkipMasked01BafterAMASKED
			////////////////////// DO that pixel (_1) here now...

			movq		mm2, mm4				//  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm2, mm3            //  ad FlashOffset
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm2,FinalShift      //  14-bit result to 9 bits   
			// Pack to 15-bits:		
			packuswb    mm2, mm2				  // Pack results, 1 pixel....	
			paddusb     mm2, [MMX16DitherOddB]	  //
			movq        mm0, mm2				  // Copy.
			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //												 
			mov 		word ptr [edi+esi*2 + 2], ax  // Store 15 bit value to screen...

			//////////////////////////////////////
	SkipMasked01BafterAMASKED: 
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			jmp         FromSkipMasked01B

	//////////////////////////////////////////////////////////////////////////////////////////////

	DOALIGN16
	SkipMasked01B: // get here after A was nonmasked: PLOT A !-> in mm0...
			paddw       mm4,mm5

			packuswb    mm0, mm0				// Pack results, 1 pixel....
			paddusb     mm0, [MMX16DitherEvenB] //		
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
			movd        eax,mm0			  	    //

			mov 		word ptr [edi+esi*2 + 0], ax  // Store 15 bit value to screen.

			//jmp         FromSkipMasked01B

		FromSkipMasked01b:
			// Coordinate calculation.... DO it here because above will be terribly interleaved with pixel-WRITE..
			movq        mm1, mm7				//
			xor			eax, eax				//
			paddd		mm7, mm6				//
			psllq       mm1, 12					//  
			punpckhdq   mm1, mm1				// Copy high word to low word.
			psrlq       mm1, [MMXCoShift]		// 
			pand        mm1, [MMXCoMask]		// 
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			// lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
			jmp 		InnerRender0_			//


	//////////////////////////////////////////////////////////////////////////

	DOALIGN16
	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16FogMaskedRender8B()




static void MMX16StippledRender8()
{
	guardSlow(MMX16StippledRender8);

	// checkerboard: defined by bit 0 of X  xor bit 0 of Y !
	
	__asm {

	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....

	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

	//
	//	Prestep ?
	//
	//	test     ebx,ebx		// Parity
	//	jnz      NoPrestep		//
	//	add      esi,1
	//	NoPrestep:
	//

	DOALIGN16
	nop
	nop
	nop

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP = [0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // movq    MMXCoMask,  mm4              
	movq    mm3,[MMXFlashOffset13]			//

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]			// movq        mm2,[ebp+8] get inital fog value...
			paddd       mm6,mm6             // double the tex delta.
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 

			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // Copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,mm2             //    
			movd        ecx,mm1             //  

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // Next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // New minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // Need different delta scaling

			psraw       mm5,2			    // 3   but double delta for stippling 
			lea			esp,[esi+8]         // just do those last 1-7 ones...

	ReloopInnerStippledRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.

			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! Compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //   

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			punpckhdq   mm1, mm1            //  Copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			//  proceed through texture 

			paddw       mm0, mm3            //
			pand        mm1,mm2             //  

			psraw       mm0, FinalShift     //  14-bit result to 9 bits.
			add 		esi,2               //  ++X

			movd        ecx,mm1             //  
			packuswb    mm0, mm0            //  Pack words to bytes

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			// convert mm0 to 15-bits.....
			movq        mm1, mm0				  // Copy.
			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm1, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm1				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			//
			movd        eax,mm0
			mov  		[edi+esi*2 - 2*2], ax    //  Store to screen. // unavoidable delay, Upipe..
			//
			jb			ReloopInnerStippledRender8  //   Unpairable, Upipe

	EndLoopInnerStippledRender8: 
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd       mm7,mm6             // undo preliminary delta-add from inner loop

			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                   // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8           // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8					// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                  // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	DOALIGN16

	SkipMaskedPixel: // don't draw, update light & coordinates using delta's...
			psllq       mm1, 12             //   
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			punpckhdq   mm1, mm1            //  Copy high word to low word 
			paddd       mm7, mm6			//

			psrlq       mm1,[MMXCoShift]    //  

			add 		esi,2               //  ++X
			pand        mm1,mm2             //  

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             // 
			jb			ReloopInnerStippledRender8  //

			nop
			jmp			EndLoopInnerStippledRender8 //

						
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	//	Same as above, but most probably the last stretch of a span.
	//
	///////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler

			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			psllw       mm5,2						// *2, *2 to double delta for stippling 
			jmp			ReLoopInnerStippledRender8       

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}
// MMX16StippledRender8




static void MMX16FogStippledRender8()
{
	guardSlow(MMX16FogStippledRender8);

	// checkerboard: defined by bit 0 of X  xor bit 0 of Y !
	
	__asm {

	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....

	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

	//
	//	Prestep ?
	//
	//	test     ebx,ebx		// Parity
	//	jnz      NoPrestep		//
	//	add      esi,1
	//	NoPrestep:
	//

	DOALIGN16
	nop
	nop
	nop

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP = [0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // movq    MMXCoMask,  mm4              
	movq    MMXCoMask,  mm2                 //

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]			// movq        mm2,[ebp+8] get inital light value...
			movq		mm2,[ebp+8]			// movq        mm2,[ebp+8] get inital fog value...

			paddd       mm6,mm6             // double the tex delta.
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 

			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // Copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             //  

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	    // Next light value
			movq        mm3,[ebp +16+8]	    // Next light value
			add         ebp,16				//

			psubsw      mm5,mm4			    // New minus current = light delta
			psubsw      mm3,mm2			    // New minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // Need different delta scaling

			psraw       mm5,2			    // 3 but double delta for stippling 
			psraw       mm3,2			    // 3 but double delta for stippling 
			lea			esp,[esi+8]         // just do those last 1-7 ones...

	ReloopInnerFogStippledRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.

			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! Compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //   

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			punpckhdq   mm1, mm1            //  Copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			//  proceed through texture 

			paddw       mm0, mm2            // Fog...            
			pand        mm1,[MMXCoMask]     //  
			paddw       mm2, mm3

			psraw       mm0, FinalShift     //  14-bit result to 9 bits.
			add 		esi,2               //  ++X

			movd        ecx,mm1             //  
			packuswb    mm0, mm0            //  Pack words to bytes

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			// convert mm0 to 15-bits.....
			movq        mm1, mm0				  // Copy.
			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm1, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm1				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			//
			movd        eax,mm0
			mov  		[edi+esi*2 - 2*2], ax     //  Store to screen. // unavoidable delay, Upipe..
			//
			jb			ReloopInnerFogStippledRender8  //   Unpairable, Upipe

	EndLoopInnerFogStippledRender8:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd       mm7,mm6             // undo preliminary delta-add from inner loop

			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                   // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8           // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16					// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                  // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	DOALIGN16

	SkipMaskedPixel: // don't draw, update light & coordinates using delta's...
			psllq       mm1, 12             //   
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			punpckhdq   mm1, mm1            //  Copy high word to low word 
			paddd       mm7, mm6			//

			psrlq       mm1,[MMXCoShift]    //  
			paddw       mm2,mm3

			add 		esi,2               //  ++X
			pand        mm1,[MMXCoMask]     //  

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             // 
			jb			ReloopInnerFogStippledRender8  //

			nop
			jmp			EndLoopInnerFogStippledRender8 //

						
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	//	Same as above, but most probably the last stretch of a span.
	//
	///////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // Delta scaler
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			psllw       mm3,2                       // *2 *2 to double delta for stippling
			psllw       mm5,2						// *2, *2 to double delta for stippling 
			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			jmp			ReLoopInnerFogStippledRender8       

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}
// MMX16FogStippledRender8





//
// Full hicolor translucency (always slow...) but with inner loop of 32-bit aligned writes AND reads..
// 
static void MMX16TranslucentRender8()  
{
	guardSlow(MMX16TranslucentRender8);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
			sub		esp,1					// Draw [esi.. esp] inclusive interval !
			test	esi,1					//
			jz		EvenStartInnerRender	//
											//
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm3 to unpack 15-bit to 15,15,15 FMMX 

			movq        mm3,mm2			// Low one filled ...

			psllq       mm2,10			//
			por         mm3,mm2			//

			psllq       mm2,11			//
			por         mm3,mm2			// all 3 colors filled ...
			
			//////////////////////////////
			xor         eax,eax

			movq		mm2, mm4				//  Copy lightvalue
			mov          al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 

			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.

			psllw       mm3,11			// to 16bit 
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 
			psrlw       mm3,4           // avoid sign + scaledown

			paddw		mm2, mm3				//  ad  screen
			psraw       mm2, FinalShift			//  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack results, 2 pixels at once !!!
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
												  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
	EvenStartInnerRender:						  //
			xor			eax,eax					  //
			cmp         esi,esp                   //
			ja          SkipInnerRender0_         //

			// lead-in coordinate calculation....
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// Copy high word to low word.
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				//
											//
			je          InnerRender0_		//
											//
	//////////////////////////////////////////

	DOALIGN16  
	ReloopInnerRender01:					// Loop using current light & light delta's. 		

			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base.		

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.

			///////////// GET screen value...& unpack the coordinate ...
			//  unpack 2 16-bit values into 2 24-bit values in mm1 !
			// making 16:16:16:16 -bit is faster though:

			movd		mm2,[edi+esi*2] // get 2x source word
			movq        mm1,mm7				//

			// unpack to high and low dword and unpack coordinate...
			punpcklwd   mm2,mm2    //  0,0 : 16a,16b ->  16a 16a , 16b 16b -> want: 0888 0888 format !
			xor			eax,eax	   //

			pslld       mm2,3	   //
			paddd		mm7,mm6	   //

			psllq       mm1,12	   //  
			movq        mm3,mm2    //  low 8 OK: Red

			pand        mm3,[MMX16CleanRed]
			psrad       mm2,8      //  erase red

			punpckhdq   mm1,mm1	   // copy high word to low word 
			pslld       mm2,8 + 2  //  low 8:0 = Green

			psrlq       mm1,[MMXCoShift]	
			por         mm3,mm2

			pand        mm1,[MMXCoMask]	// 
			psrad       mm2,16      	// Erase Green

			pand        mm3,[MMX16CleanGreenRed]
			psraw       mm0,FinalShift  //  14-bit result to 9 bits.

			movd        ecx,mm1			// 
			pslld       mm2,16 + 3	    // Shift Blue in right position.

			por         mm3,mm2         //
			//////////////////////////////////////
			//////////////////////////////////////
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

			movq		mm2, mm4				//  Copy lightvalue.
			paddw		mm4, mm5				//	+ mm5, signed 16-bit R,G,B LightDeltas 
			
			pmulhw		mm2, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			movq        mm1, mm7				//
			psraw       mm2,FinalShift			//  14-bit result to 9 bits   

			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
			// interleaved with coordinate calculation...

			packuswb    mm0, mm2                // Pack results, 2 pixels at once !!!
			paddd		mm7, mm6				//

			paddusb     mm0, mm3                // SCREEN value as unpacked...
			psllq       mm1, 12					//  

			movq        mm2, mm0				// copy.
			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue

			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			punpckhdq   mm1, mm1				// Copy high word to low word.

			psrlq       mm1, [MMXCoShift]		// 
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits

			pand        mm1, [MMXCoMask]		// 
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			//packssdw    mm0,mm0				// Pack  words XX AA XX BB  into lower  dword   AA B
			movq        mm2,mm0
			psrlq       mm0,16
			por         mm0,mm2
			//

			xor			eax, eax				//

			movd 		[edi+esi*2], mm0		// Store 15 bit value to screen...
			movd        ecx, mm1				//
			//////////////////////////////////////
												//
			add         esi,2                   //
			cmp			esi,esp					//
			jb			ReloopInnerRender01		//
			//lead-out //

			psubd		mm7,mm6 // undo last coordinate increment  ONLY if skip done...
			//////////////////////////////////////
			ja			SkipInnerRender0_		//
	//////////////////////////////////////////////

	InnerRender0_:

			///////////////////////////////
			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm3 to unpack 15-bit to 15,15,15 FMMX 
			movq        mm3,mm2			// Low one filled ...

			psllq       mm2,10			//
			por         mm3,mm2			//

			psllq       mm2,11			//
			por         mm3,mm2			// all 3 colors filled ...
			
			//////////////////////////////
			xor         eax, eax

			movq		mm0, mm4				//  Copy lightvalue.
			mov         al,  byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.

			psllw       mm3, 11				// to 16bit 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			psrlw       mm3, 4				// avoid sign + scaledown

			paddw		mm0, mm3            //  Add adjusted screenval
			psraw       mm0, FinalShift      //  14-bit result to 9 bits.

			// Pack to 15-bits:				
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!
			movq        mm2, mm0				// Copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
			por			mm0, mm2				// Combine the red, green, and blue bits.
			psrld		mm0, MMX16RGBSHIFT		// Shift to final position.

			movd        eax, mm0				  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen.
			add         esi, 1					  //

			paddd       mm7, mm6				  // un-undo last coordinate decrement...

	SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		sub		esp,1					// Draw [esi.. esp] inclusive interval !
		test	esi,1					//
		jz		EvenStartInnerRender	//
		nop								//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX16TranslucentRender8()






//
// Full hicolor MODULATION (always slow...) 
// 

static void MMX16ModulatedRender8()  
{
	guardSlow(MMX16ModulatedRender8);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			paddw       mm5,mm5				//
											//
	//FromBiteOffEight:						//
											//
	//////////////////////////////////////////
	DOALIGN16

	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			///////////////////////////////
			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 

			movq        mm1,mm2			// Low one filled ...

			psllq       mm2,10			//
			por         mm1,mm2			//

			psllq       mm2,11			//
			por         mm1,mm2			// all 3 colors filled ...

			psllw       mm1,11			// to 16bit 
			xor         eax,eax
			psrlw       mm1,1           // avoid sign
			//////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov          al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 

			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, mm1				//  Modulate  #debug1  scaling !
			psraw       mm2, FinalShift - 1     //  14-bit result to 9 bits   

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack result
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
			xor			eax,eax					  //
			cmp         esi,esp                   //
			jb          InnerRender_1             //

	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,8				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm5,3					//
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX16ModulatedRender8()



//
// Full hicolor FogModulATION (always slow...) 
// 

static void MMX16FogModulatedRender8()  
{
	guardSlow(MMX16FogModulatedRender8);

	//static DWORD FakeESP;
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6		TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5		LightDelta
	// mm3		FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
													 //
			mov			ecx,[eax]FTexSetup.X         // get delta
													 //
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex		 	 //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			movq        mm3,[ebp+8]
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16								
	////////////////////////////////////////////////////

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+16]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			movq        mm2,[ebp+16+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,16                // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			psubsw      mm2,mm3				// New minus current = fog delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]           
			cmp         eax,8               // Blit how many pixels:	min(8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			pmulhw		mm2,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...

			paddw       mm2,mm2				//
			paddw       mm5,mm5				//
			movq        [MMXTempFogDelta],mm2 
											//
	DOALIGN16
	//////////////////////////////////////////
	InnerRender_1:							//
			movq        mm1,mm7				//
			xor         eax,eax				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			///////////////////////////////
			// GET a screen value (16-bit)-> 151515 !
			mov         ax,[edi+esi*2]	// #debug interleave this all to above!

			movd        mm2,eax     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 

			movq        mm1,mm2			// Low one filled ...

			psllq       mm2,11			//
			por         mm1,mm2			//

			psllq       mm2,11			//
			por         mm1,mm2			// all 3 colors filled ...

			psllw       mm1,11			// to 16bit 
			xor         eax,eax
			psrlw       mm1,1           // avoid sign
			//////////////////////////////

			movq		mm2, mm4				//  Copy lightvalue
			mov          al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 

			pmulhw		mm2, qword ptr [edx+eax*8] //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5				//  + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm2, mm1				//  FogModulate  #debug1  scaling !

			movq        mm0, mm3
			paddw       mm3,[MMXTempFogDelta]
			psraw       mm0, FinalShift

			psraw       mm2, FinalShift - 1     //  14-bit result to 9 bits   

			paddw       mm2,mm0 // final-shifted fog value...

			// Pack to 15-bits:		
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm2, mm2				  // Pack result
			movq        mm0, mm2				  // Copy.

			pand		mm0, [MMX16REDBLUE]		  // Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		  // Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		  // Mask out all but the 5MSBits of green
			por			mm0, mm2				  // Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT		  // Shift to final position.
			movd        eax,mm0					  //
			mov 		word ptr [edi+esi*2], ax  // Store 15 bit value to screen...
			add         esi,1					  // 
												  //
	////////////////////////////////////////////////
												  //
			cmp         esi,esp                   //
			jb          InnerRender_1             //

	// SkipInnerRender0_:
	//////////////////////////////////////////////
			mov         eax,[NX]				//  Inner loop pixel counter (always 8 here though) #debug 
			nop									//
			cmp         esi, eax				//
			jb          ReloopInnerTex8			//
												//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.
											//
	//////////////////////////////////////////
	// X<0 -> new mipmap if last X >0		//
			add         eax,8               // Skip 2 dwords and point to next setup structure.
	        cmp         ecx,-1				//
			je          ReloopNewMip8       // New mip, always valid 2 structs after this.

	//////////////////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span.
			add			eax,8				//
			// [[E]] -> only when there really is one at end of span/line/whatever.
			add         ebp,16				// This is for the one extra at END of EVERY span.
											//
			cmp         ecx,-3              // Greater == -2, lower = -4  eq = -3.
			jle         EndOfLine                 
			// StartNewSpan8: Only happens for structures obscured by stuff in front. 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, More spans on this line.

	//////////////////////////////////////
	DOALIGN16							//
	BiteOffEight:						//
		lea     esp,[esi+8]				//
		psraw	mm2,3					//
		psraw	mm5,3					//
		movq    [MMXTempFogDelta],mm2   //
		jmp		InnerRender_1           //
	//////////////////////////////////////

	DOALIGN16

	NegativeStart:		// ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	// skip tag + skip dummy end-LIT.

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax // Update SetupWalker, reflecting the setup data that was executed.

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX16FogModulatedRender8()








//  
// MMX 16 bit rendering routine : unoptimized nondithered, for reference.
//

static void MMX16Render8()  // Basic plain lit-texturemapper, no unrolling, no dithering.
{
	guardSlow(MMX16Render8);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:							//
 ReloopNewMip8:								// New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         //
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //
											//
	movq    mm3,[MMXFlashOffset13]			//

	//////////////////////////////////////////
	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////

			mov			ecx,[eax]FTexSetup.X         // get delta

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx			 //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			movq		mm4,[ebp]           // Current (start) light value.
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	DOALIGN16 

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp+8]	        // Next light value;    movq  mm3,[ebp +16 + 8] = next fog value
			add         ebp,8               // 
			
			psubsw      mm5,mm4				// New minus current = light delta
			sub         eax,esi             // eax = NextX - X

			mov			esp,[NX]            //
			cmp         eax,8               // Blit how many pixels:  min (8, NextX-X)
			jae         BiteOffEight		// 
					
			// 1 to 7 pixels to do; DIGEST an odd one first if needed, then come back in main 2x loop if any left...
			pmulhw		mm5,MMXDeltaAdjust[eax*8] // Faster to shift right for default of 8 pixels...
			xor         eax,eax
			paddw       mm5,mm5
			
	ReloopInnerRender8:  // Loop using current light & light delta's. 		
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7				//
			paddd		mm7,mm6				//
			psllq       mm1,12				//  
			punpckhdq   mm1,mm1				// copy high word to low word 
			psrlq       mm1,[MMXCoShift]	// 
			pand        mm1,[MMXCoMask]		// 
			movd        ecx,mm1				// 
			//////////////////////////////////

			movq		mm0, mm4				//  Copy lightvalue
			mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc         esi

			paddw		mm0, mm3            //  ad FlashOffset

			//paddw		mm2, [Fog]          //  Add 14-bit fog. 
			//paddw		[Fog],[FogDelta]    //  Add FogDelta.

			psraw       mm0,FinalShift      //  14-bit result to 9 bits   

			// Pack to 15-bits:				&&&&
			// 32-to-15 bit conversion, single pixel...(intel appnote #553)			
												
			packuswb    mm0, mm0				// Pack results, 2 pixels at once !!!


			movq        mm2, mm0				// copy.

			pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
			pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
			pand		mm2, [MMX16GREEN]		// Mask out all but the 5MSBits of green
			por			mm0, mm2				// Combine the red, green, and blue bits
			psrld		mm0, MMX16RGBSHIFT	    // Shift to final position.

			movd        eax,mm0
			mov 		word ptr [edi+esi*2 - 1*2], ax	// Store 15 bit value to screen...

			xor         eax,eax					// !! This is no good for PII, must be XOR -> clashes with jb...
												//
			cmp			esi,esp					//	   cmp  X, PX;  always runs 8 times.		
			jb			ReloopInnerRender8		//
												//
	//////////////////////////////////////////////
		   // Tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0

			//EndInnerRender8:
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
																			
			//EndofInnerTex8:

			// Indicator: blacken last pixel
#if INDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*2 - 1*2], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*2 - 1*2], mm0    //  store to screen.
#endif

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	DOALIGN16
	BiteOffEight:
		lea         esp,[esi+8]			
		psraw		mm5,3
		xor			eax,eax
		jmp ReloopInnerRender8


	DOALIGN16
	NegativeStart:		//  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8	//  skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMX16Render8()



//  
// MMX 32 bit rendering routines.
//

static void MMX32Render8Unrolled()
{
	guardSlow(MMXRender8Unrolled);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	movq    mm3,[MMXFlashOffset13]   

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...


			//psubd       mm0,mm0
			//movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

	// get initial light & fog values.
			movq		mm4,[ebp]                    // current (start) light value.
			//movq        mm2,[ebp+8]
			mov         eax,[NX]		      // End of current Subdiv #debug get from other register above?

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	      // next light value  movq  mm3,[ebp +16 + 8]   = next fog value
			add         ebp,8                 //

			
			psubsw       mm5,mm4			  // New minus current = light delta
			sub         eax,esi             

			cmp         eax,8				  // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 
			cmp         eax,4         
			jnb         Full4loop             // 4-pixel span, usually part of a 4-7 pixel endspan.

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 
			mov			esp,[NX]              // Just do those last 1-7 ones...
			paddw      mm5,mm5                // #debug optim make it work WITHOUT paddw OR move this....
			xor			eax,eax               //

			// #debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
	        //
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		
			paddw      mm0, mm3             // ad FlashOffset

			//paddw		mm0, mm2            //  Add 14-bit fog. 
			//paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   // #debug tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:

#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	///////////////////////////////
	//
	// Prelim optim: perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	jmp Full8Loop
	DOALIGN16

		Full8Loop:
			psraw       mm5,3		       // * 1/8  
 
			movq        mm2,mm7            // 
			paddd		mm7,mm6			   // 

			psllq       mm2,12             //  
			xor         eax,eax            // 
			punpckhdq   mm2,mm2            // copy high word to low word 
			psrlq       mm2,[MMXCoShift]   // 
			pand        mm2,[MMXCoMask]    //    
			movd        ecx,mm2            // 

			/////////////

			 movq        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			 movq        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2             // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]    // 
			paddw		mm4, mm5             // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]     //    
			paddw       mm1, mm3             //    

			psraw       mm1,FinalShift       // 14-bit result to 9 bits   
			packuswb    mm1,mm1              // Pack words to bytes.

			movd        ecx,mm2              // 
			movd		[edi+esi*4+1*4], mm1 // Store to screen. 	

			/////////////

			 movq        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue.
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			 movq        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue.

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits.
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+3*4], mm1 // Store to screen. 	

			/////////////

			 movq        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			 movq        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+4*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+5*4], mm1 // Store to screen. 	

            ////////////

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //

			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .

			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            //    

			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   

			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.

			///// End of unrolled inner loop
			movq		mm1, mm4            //  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movd		[edi+esi*4+6*4], mm0 // Store to screen. 	

			pmulhw		mm1, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm1, mm3            //    
			psraw       mm1, FinalShift      //  14-bit result to 9 bits   
			packuswb    mm1,mm1             //  Pack words to bytes
			movd		[edi+esi*4+7*4], mm1    //  store to screen. 

			add         esi,8
			//////////////////////////////////
			mov         eax,[NX]            //  
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
			jmp         EndOfInnerLight8    //
			//////////////////////////////////


   			//// 7 Times unrolled interleaved inner loop 
			/* basic block: //#debug use mm2 -> not for mem but EXTRA thrd...
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 			
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.

			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			*/

	/////////////////////////////////////////
	jmp Full4Loop
	DOALIGN16
		Full4Loop:						   //

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 

			movq        mm2,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm2,12             //  
			xor         eax,eax            //
			punpckhdq   mm2,mm2            // copy high word to low word 
			psrlq       mm2,[MMXCoShift]   // 
			pand        mm2,[MMXCoMask]    //    
			movd        ecx,mm2            // 

			paddw       mm5,mm5                    // 

			/////////////

			 movq        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			 movq        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            // ad FlashOffset

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			 //#debug
//			psubd        mm0,mm0
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            // ad FlashOffset

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+1*4], mm1 // Store to screen. 	

            ////////////

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //

			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .

			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            // ad FlashOffset

			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   

			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.


			// End of unrolled inner loop

			movq		mm1, mm4            //  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			pmulhw		mm1, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm1, mm3            // ad FlashOffset
			psraw       mm1, FinalShift      //  14-bit result to 9 bits   
			packuswb    mm1,mm1             //  Pack words to bytes
			movd		[edi+esi*4+3*4], mm1    //  store to screen. 

			add         esi,4
			//////////////////////////////////
			mov         eax,[NX]            //  
			cmp         esi, eax            //
			jnb         EndOfInnerLight8    //

			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			jmp ReloopInnerRender8          // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.

	//////////////////////////////////////////
		
	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8  // skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMXRender8Unrolled()





//  
// MMX 32 bit rendering routines.
//
// Warning: this routine banks on the current situationt that in a 1-7 pixel stretch 
// it is IMPLIED that we have at least an end of a span here, if not an end-of-line.
//

static void MMX32Render8UnrolledFaster()
{
	guardSlow(MMXRender8UnrolledFaster);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 align 16	// #FLAKY compiler behavior if you put in any more jumps to aligned addresses / multiple 'align16's...
    nop
	nop
	nop

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	movq    mm3,[MMXFlashOffset13]   

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			movq        mm1,mm7            // start interleaved first-coordinate calculation

			//paddd		mm7,mm6			   //
			add         ecx,esi						 // deltaX + X
			psllq       mm1,12             //  

			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			punpckhdq   mm1,mm1            // 

			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...

			psrlq       mm1,[MMXCoShift]   // 

			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			pand        mm1,[MMXCoMask]    //    
		 	// get initial light & fog values.
			movq		mm4,[ebp]                    // current (start) light value.//movq        mm2,[ebp+8]
			mov         eax,[NX]		      // End of current Subdiv #debug get from other register above?

			movd        ecx,mm1            // 
			/////////////////////////////////

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	      // next light value  movq  mm3,[ebp +16 + 8]   = next fog value
			add         ebp,8                 //
			
			psubsw       mm5,mm4			  // New minus current = light delta
			sub         eax,esi             

			cmp         eax,8				  // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 
			paddd       mm7,mm6

			mov			esp,[NX]              // Just do those last 1-7 ones...
			paddw       mm5,mm5                // #debug optim make it work WITHOUT paddw OR move this....

	ReloopInnerRender8:  // Loop using current light & light delta's. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			
			xor         eax,eax
			movq		mm0, mm4            //  Copy lightvalue

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq        mm1,mm7            //
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			inc 		esi                 //  ++X
			paddd		mm7,mm6			   //

			psllq       mm1,12             //  
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		

			paddw       mm0, mm3            // ad FlashOffset
			punpckhdq   mm1,mm1            // copy high word to low word 

			psrlq       mm1,[MMXCoShift]   // 
			//paddw		mm0, mm2            //  Add 14-bit fog. 
			//paddw		mm2, mm3            //  Add FogDelta.
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   

			pand        mm1,[MMXCoMask]     //    
			packuswb    mm0,mm0             //  Pack words to bytes

			movd        ecx,mm1             // 
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			//////////////////////////////////
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
											//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			add         eax,16              // skip 2 dwords and point to next setup structure
			add         ebp,8				// This is for the one extra at END of EVERY span .
			cmp         ecx,-3              // greater == -2, lower = -4  eq = -3
			jle         EndOfLine           //      

			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	///////////////////////////////
	//
	// Prelim optim: perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	//DOALIGN16

		nop
		nop
		nop
		nop
		nop
		nop

		nop
		nop
		nop

		Full8Loop: // ecx already prepared.
			psraw       mm5,3		       // * 1/8  
			paddd       mm7,mm6

			xor         eax,eax            // 
			/////////////
			 movq        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			 movq        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2             // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]    // 
			paddw		mm4, mm5             // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]     //    
			paddw       mm1, mm3             //    

			psraw       mm1,FinalShift       // 14-bit result to 9 bits   
			packuswb    mm1,mm1              // Pack words to bytes.

			movd        ecx,mm2              // 
			movd		[edi+esi*4+1*4], mm1 // Store to screen. 	

			/////////////

			 movq        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue.
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			 movq        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue.

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits.
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+3*4], mm1 // Store to screen. 	

			/////////////

			 movq        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			 movq        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+4*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+5*4], mm1 // Store to screen. 	

            ////////////

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //

			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .

			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            //    

			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   

			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.

			///// End of unrolled inner loop
			

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq        mm2,mm7            //

			movd		[edi+esi*4+6*4], mm0 // Store to screen. 	
			//paddd		mm7,mm6			   //
			movq		mm1, mm4            //  Copy lightvalue

			pmulhw		mm1, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm2,12             //  

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			punpckhdq   mm2, mm2            // 

			psrlq       mm2,[MMXCoShift]   // 
			paddw       mm1, mm3            //    

			pand        mm2,[MMXCoMask]    //    
			psraw       mm1, FinalShift      //  14-bit result to 9 bits   

			packuswb    mm1, mm1             //  Pack words to bytes		
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 

			movd        ecx,mm2            //

			movd		[edi+esi*4+7*4], mm1    //  store to screen. 
			add         esi,8

	//////////////////////////////////////////
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		// EndofInnerLight8:

		#if INDICATOR
		  	pcmpeqd      mm1,mm1
	        movd		[edi+esi*4 +7*4 - 8*4], mm1    //  store to screen.
		#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			//psubd       mm7,mm6
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

			//
   			// 7 Times unrolled interleaved inner loop 
			//

			/* basic block: //#debug use mm2 -> not for mem but EXTRA thrd...
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 			
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.

			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			*/

	///////////////////////////////////////////
	//		
	nop
	nop
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8  // skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMXRender8UnrolledFaster()






//  
// RAW unrolled non-paired MMX 32 bit rendering routine.
//

static void MMX32Render8RawNew()
{
	guardSlow(MMXRender8RawNew);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	movq    mm2,[MMXFlashOffset13]   //

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			// Get initial light & fog values.
			movq		mm4,[ebp]                    // current (start) light value.
			mov         eax,[NX]		      // End of current Subdiv #debug get from other register above?

			/////////////////////////////////
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            //
			/////////////////////////////////

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	      // next light value  movq  mm3,[ebp + 16 + 8]   = next fog value
			add         ebp,8                //
		
			psubsw      mm5,mm4			      // New minus current = light delta
			sub         eax,esi             

			cmp         eax,8				  // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 
			mov			esp,[NX]              // Just do those last 1-7 ones...
			paddw       mm5,mm5               // #debug optim make it work WITHOUT paddw OR move this....
			xor			eax,eax               // 

			// #debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			
			/////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
											//
			jb			ReloopInnerRender8  //
											//
			//////////////////////////////////
			//
			// done a 1-7 pixel stretch, now we're done for this SPAN at least
			// Warning: this banks on the current situationt that in a 1-7 pixel stretch 
			// it is IMPLIED that we have at least an end of a span here, if not an end-of-line.
			//
		
			mov         eax,[SetupWalker]   //
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			add         eax,8 //+ 8           // skip 2 dwords and point to next setup structure
			add         ebp,8				// This is for the one extra at END of EVERY span .
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 

			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	//
	// Perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	DOALIGN16

		Full8Loop:

			psraw		mm5,3  //* 1/8
			xor			eax,eax

			//////////////////////////////////
			// 8 x this !					//
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X

			paddw		mm0, mm2            //  Add 14-bit fog. 
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			// add         esi,8  &&&&&&&&&&&&&&

			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //	
			
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd		mm7,mm6			    // Undo extra coordinate delta add
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8 + 8	// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	//////////////////////////////////////////////////////////////////

	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8  // skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMXRender8RawNew()



 
//  
// RAW unrolled non-paired MMX 32 bit rendering routine.
//

static void MMX32Render8RawNewFog()
{
	guardSlow(MMXRender8RawNewFog);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	// movq    mm3,[MMXFlashOffset13]    &&&&

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			// Get initial light & fog values.
			movq		mm4,[ebp]                    // current (start) light value.
			movq        mm2,[ebp+8]                 // current (start) fog value 
 
			mov         eax,[NX]		      // End of current Subdiv #debug get from other register above?

			/////////////////////////////////
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            //
			/////////////////////////////////

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	      // next light value  movq  mm3,[ebp + 16 + 8]   = next fog value
			movq        mm3,[ebp +16 +8]	  // next light value  movq  mm3,[ebp + 16 + 8]   = next fog value
			add         ebp,16                //
		
			psubsw      mm5,mm4			      // New minus current = light delta
			psubsw      mm3,mm2			      // New minus current = fog   delta
			sub         eax,esi             

			cmp         eax,8				  // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 
			pmulhw		mm3,MMXDeltaAdjust[eax*8]; 
			mov			esp,[NX]              // Just do those last 1-7 ones...
			paddw       mm5,mm5               // #debug optim make it work WITHOUT paddw OR move this....
			paddw       mm3,mm3               // #debug optim make it work WITHOUT paddw OR move this....
			xor			eax,eax               // 

			// #debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			
			/////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
											//
			jb			ReloopInnerRender8  //
											//
			//////////////////////////////////
			//
			// done a 1-7 pixel stretch, now we're done for this SPAN at least
			// Warning: this banks on the current situationt that in a 1-7 pixel stretch 
			// it is IMPLIED that we have at least an end of a span here, if not an end-of-line.
			//
		
			mov         eax,[SetupWalker]   //
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			add         eax,8 + 8           // skip 2 dwords and point to next setup structure
			add         ebp,16				// This is for the one extra at END of EVERY span .
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 

			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	//
	// Perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	DOALIGN16

		Full8Loop:

			psraw		mm5,3  //* 1/8
			psraw		mm3,3  //* 1/8

			xor			eax,eax

			//////////////////////////////////
			// 8 x this !					//
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw       mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			//////////////////////////////////
			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			//paddw      mm0, mm3             // ad FlashOffset

			paddw		mm0, mm2            //  Add 14-bit fog. 
			paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

			movq        mm1,mm7             //
			paddd		mm7,mm6			    //
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             // 
			//////////////////////////////////

			// add         esi,8  &&&&&&&&&&&&&&

			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //	
			
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd		mm7,mm6			    // Undo extra coordinate delta add
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	//////////////////////////////////////////////////////////////////

	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8  // skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMXRender8RawNewFog()


 

static void MMX32Render8A()
{
	guardSlow(MMXRender8A);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	movq    mm3,[MMXFlashOffset13]			//

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

			//psubd     mm0,mm0
			//movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

	// get initial light & fog values.
			movq		mm4,[ebp]           // Current (start) light value.
			//movq        mm2,[ebp+8]
			mov         eax,[NX]		    // End of current Subdiv #debug get from other register above?

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // next light value  movq  mm3,[ebp +16 + 8]   = next fog value
			add         ebp,8               //
			
			psubsw       mm5,mm4			// New minus current = light delta
			sub         eax,esi             

			cmp         eax,8               // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop			//
			cmp         eax,4				//
			jnb         Full4loop           // 4-pixel span, usually part of a 4-7 pixel endspan.

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 
			mov			esp,[NX]            // Just do those last 1-7 ones...
			paddw      mm5,mm5              // #debug optim make it work WITHOUT paddw OR move this....
			xor			eax,eax             //

			//#debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
	        //
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    // 
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		
			paddw       mm0, mm3            //  ad FlashOffset

			//paddw		mm0, mm2            //  Add 14-bit fog. 
			//paddw		mm2, mm3            //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0   //  store to screen.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   // #debug tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:

			//#debug blacken last pixel in a lightsample-run...
#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	///////////////////////////////
	//
	// Prelim optim: perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////

	jmp Full8Loop
	DOALIGN16

		Full8Loop:
			movq        mm2,[KnightA0]     //
			psraw       mm5,3		       // * 1/8  
			paddd       mm2,mm7            // 
			paddd		mm7,mm6			   // 

			psllq       mm2,12             // .
			xor         eax,eax            // 
			punpckhdq   mm2,mm2            // Copy high word to low word.
			psrlq       mm2,[MMXCoShift]   // 
			pand        mm2,[MMXCoMask]    // 
			movd        ecx,mm2            // 

			/////////////
			movq        mm1,[KnightA1]      //

			paddd        mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // 

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			movq        mm2,[KnightA0]		//
			paddd        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    // 
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12              // 

			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			 //

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    // 
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+1*4], mm1 // Store to screen. 	

			/////////////

			movq        mm1,[KnightA1]
			 paddd       mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			movq        mm2,[KnightA0]
			paddd        mm2,mm7            //

			 pand        mm1,[MMXCoMask]    // 
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+3*4], mm1 // Store to screen. 	

			/////////////

			movq        mm1,[KnightA1]
			 paddd       mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			movq        mm2,[KnightA0]
			 paddd       mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+4*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+5*4], mm1 // Store to screen. 	

            ////////////

			movq        mm1,[KnightA1]
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 paddd       mm1,mm7            //

			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .

			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            //    

			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   

			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.

			///// End of unrolled inner loop
			movq		mm1, mm4            //  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movd		[edi+esi*4+6*4], mm0 // Store to screen. 	

			pmulhw		mm1, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm1, mm3            //    
			psraw       mm1, FinalShift      //  14-bit result to 9 bits   
			packuswb    mm1,mm1             //  Pack words to bytes
			movd		[edi+esi*4+7*4], mm1    //  store to screen. 

			add         esi,8
			//////////////////////////////////
			mov         eax,[NX]            //  
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //


			jmp         EndOfInnerLight8    //
			//////////////////////////////////


   			//// 7 Times unrolled interleaved inner loop 
			/* basic block: // #debug use mm2 -> not for mem but EXTRA thrd...
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 			
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.

			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			*/

	/////////////////////////////////////////
	jmp Full4Loop
	DOALIGN16
		Full4Loop:						   //

			movq        mm2,[KnightA0]
			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 

			paddd       mm2,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm2,12             //  
			xor         eax,eax            //
			punpckhdq   mm2,mm2            // copy high word to low word 
			psrlq       mm2,[MMXCoShift]   // 
			pand        mm2,[MMXCoMask]    //    
			movd        ecx,mm2            // 

			movq        mm1,[KnightA1]
			paddw       mm5,mm5            // 

			/////////////////////////////////

			 paddd       mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // 

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			movq        mm2,[KnightA0]

			 psrlq       mm1,[MMXCoShift]   // 
			 paddd       mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            // ad FlashOffset

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			//#debug
			//psubd        mm0,mm0

			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            // ad FlashOffset

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+1*4], mm1 // Store to screen. 	

            ////////////
			movq        mm1,[KnightA1]

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 paddd      mm1,mm7                //

			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .

			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            // ad FlashOffset

			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   

			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.


			// End of unrolled inner loop

			movq		mm1, mm4            //  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			pmulhw		mm1, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm1, mm3            // ad FlashOffset
			psraw       mm1, FinalShift      //  14-bit result to 9 bits   
			packuswb    mm1,mm1             //  Pack words to bytes
			movd		[edi+esi*4+3*4], mm1    //  store to screen. 

			add         esi,4
			//////////////////////////////////
			mov         eax,[NX]            //  
			cmp         esi, eax            //
			jnb         EndOfInnerLight8    //



			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			jmp ReloopInnerRender8          // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.

	//////////////////////////////////////////

	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8  // skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMXRender



static void MMX32Render8B()
{
	guardSlow(MMXRender8B);
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 //
	movq    MMXCoMask,  mm4                 //

	movq    mm3,[MMXFlashOffset13]   

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...


			//psubd       mm0,mm0
			//movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.

	// get initial light & fog values.
			movq		mm4,[ebp]                    // current (start) light value.
			//movq        mm2,[ebp+8]
			mov         eax,[NX]		      // End of current Subdiv #debug get from other register above?

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	      // next light value  movq  mm3,[ebp +16 + 8]   = next fog value
			add         ebp,8                 //

			
			psubsw       mm5,mm4			    // New minus current = light delta
			sub         eax,esi             

			cmp         eax,8               // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 
			cmp         eax,4         
			jnb         Full4loop           // 4-pixel span, usually part of a 4-7 pixel endspan.

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 
			mov			esp,[NX]            // Just do those last 1-7 ones...
			paddw      mm5,mm5             // #debug optim make it work WITHOUT paddw OR move this....
			xor			eax,eax             //

			// #debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
	        //
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		
			paddw      mm0, mm3               // ad FlashOffset

			//paddw		mm0, mm2           //  Add 14-bit fog. 
			//paddw		mm2, mm3           //  Add FogDelta.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   // #debug tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:

#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	///////////////////////////////
	//
	// Prelim optim: perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	jmp Full8Loop
	DOALIGN16

		Full8Loop:
			psraw       mm5,3		       // * 1/8  
 
			movq        mm2,[KnightB0]      
			paddd       mm2,mm7            // 
			paddd		mm7,mm6			   // 

			psllq       mm2,12             //  
			xor         eax,eax            // 
			punpckhdq   mm2,mm2            // copy high word to low word 
			psrlq       mm2,[MMXCoShift]   // 
			pand        mm2,[MMXCoMask]    //    
			movd        ecx,mm2            // 

			/////////////

			movq        mm1,[KnightB1]
			 paddd       mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			movq        mm2,[KnightB0]
			 paddd       mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+1*4], mm1 // Store to screen. 	

			/////////////

			movq        mm1,[KnightB1]
			 paddd       mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			movq        mm2,[KnightB0]
			 paddd       mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+3*4], mm1 // Store to screen. 	

			/////////////

			movq        mm1,[KnightB1]
			 paddd       mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			movq        mm2,[KnightB0]
			 paddd       mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            //    

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			movd		[edi+esi*4+4*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            //    

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+5*4], mm1 // Store to screen. 	

            ////////////

			movq        mm1,[KnightB1]
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 paddd       mm1,mm7            //

			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .

			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            //    

			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   

			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.

			///// End of unrolled inner loop
			movq		mm1, mm4            //  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movd		[edi+esi*4+6*4], mm0 // Store to screen. 	

			pmulhw		mm1, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm1, mm3            //    
			psraw       mm1, FinalShift      //  14-bit result to 9 bits   
			packuswb    mm1,mm1             //  Pack words to bytes
			movd		[edi+esi*4+7*4], mm1    //  store to screen. 

			add         esi,8
			//////////////////////////////////
			mov         eax,[NX]            //  
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
			jmp         EndOfInnerLight8    //
			//////////////////////////////////


   			//// 7 Times unrolled interleaved inner loop 
			/* basic block: //#debug use mm2 -> not for mem but EXTRA thrd...
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 			
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.

			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			*/

	/////////////////////////////////////////
	jmp Full4Loop
	DOALIGN16
		Full4Loop:						   //

			movq        mm2,[KnightB0]
			pmulhw		mm5,MMXDeltaAdjust[eax*8]; 

			paddd       mm2,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm2,12             //  
			xor         eax,eax            //
			punpckhdq   mm2,mm2            // copy high word to low word 
			psrlq       mm2,[MMXCoShift]   // 
			pand        mm2,[MMXCoMask]    //    
			movd        ecx,mm2            // 

			movq        mm1,[KnightB1]
			paddw       mm5,mm5            // 

			/////////////////////////////////

			 paddd       mm1,mm7            //
			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 

			 paddd		 mm7,mm6			//
			 psllq       mm1,12             // 

			movq		mm0, mm4            // Copy lightvalue
			 punpckhdq   mm1,mm1            // Copy high word to low word.

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			movq        mm2,[KnightB0]

			 psrlq       mm1,[MMXCoShift]   // 
			 paddd       mm2,mm7            //

			 pand        mm1,[MMXCoMask]    //    
			paddw       mm0, mm3            // ad FlashOffset

			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 

			packuswb    mm0,mm0             // Pack words to bytes.
			movq		mm1, mm4            // Copy lightvalue

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 psllq       mm2,12             // .

			//#debug
			//psubd        mm0,mm0

			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	
			 paddd		 mm7,mm6			//

			pmulhw		mm1, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 punpckhdq   mm2,mm2            // Copy high word to low word.

			 psrlq       mm2,[MMXCoShift]   // 
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 pand        mm2,[MMXCoMask]    //    
			paddw       mm1, mm3            // ad FlashOffset

			psraw       mm1,FinalShift      // 14-bit result to 9 bits   
			packuswb    mm1,mm1             // Pack words to bytes.

			movd        ecx,mm2            // 
			movd		[edi+esi*4+1*4], mm1 // Store to screen. 	

            ////////////
			movq        mm1,[KnightB1]

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 paddd      mm1,mm7                //

			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue

			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .

			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.

			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm3            // ad FlashOffset

			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   

			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.


			// End of unrolled inner loop

			movq		mm1, mm4            //  Copy lightvalue
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			pmulhw		mm1, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm1, mm3            // ad FlashOffset
			psraw       mm1, FinalShift      //  14-bit result to 9 bits   
			packuswb    mm1,mm1             //  Pack words to bytes
			movd		[edi+esi*4+3*4], mm1    //  store to screen. 

			add         esi,4
			//////////////////////////////////
			mov         eax,[NX]            //  
			cmp         esi, eax            //
			jnb         EndOfInnerLight8    //



			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			jmp ReloopInnerRender8          // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.


	//////////////////////////////////////////
		
	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8  // skip tag + skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMXRender



/* well-crafted PolYC rolled loop:
			ReloopInnerRender8:		  // Loop using current light & light delta's. Getting the texel coordinate				

			psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
			paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 

			pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
			paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
	
			xor         eax, eax			//  
			add esi,1						//  Inc 		esi                 //  ++X
			
			punpckhdq   mm1, mm1			//
			cmp			esi, edx            //  Cmp  X, PX;  

			psrlq       mm1,[MMXCoShift]	//
			paddw       mm0, mm2			//  Ad Fogging

			pand        mm1,[MMXCoMask]     //
			psraw       mm0,5               //  14-bit result to 9 bits  5???     						

			paddw       mm2,mm3             //  Fog += fogdelta
			packuswb    mm0,mm0             //  Pack words to bytes.

			movd        ecx,mm1             //
			movq        mm1,mm7             //

			movd		[edi+esi*4-4], mm0  //  Store to screen. 
			movq        mm0,mm4             //

			mov         al,byte ptr [ebx+ecx] //
			jb			ReloopInnerRender8    //
*/


/* 

  Nice rolled loop -> no really want 1 loop for generality ???? 7.5 + 2 -cycle = old fully-unrolled, this is 9+0-cycle !!
  //
  // 10 cycles with the fog-add. Unrolled stuff is 9.5 cycles theory, but turned out to be faster though ???
  // 
  // Also, this here is harder to dither...
  //

*/


//#debug has pixel artifacts at Subdivisions.
static void MMX32NewRender8()
{
	guardSlow(MMX32NewRender8);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm2,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm3,[ecx]FMipSetup.HybridMask   //
	//movq    MMXCoShift, mm3               // 
	//movq    MMXCoMask,  mm4               //

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]			// movq        mm2,[ebp+8] get inital fog value...
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?


			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,mm2             //    
			movd        ecx,mm1             //  
			movq        mm1,mm7
			movq        mm0,mm4		
			
	ReloopInnerTex8:                        // calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // new minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // need different delta scaling

			psraw       mm5,3			    // 3 
			xor			eax,eax				// 

			lea			esp,[esi+8]         // just do those last 1-7 ones...
			mov         al,byte ptr [ebx+ecx]
		
	ReloopInnerRender8:	
			psllq		mm1, 12				// 
			paddw		mm4, mm5			//

			pmulhw		mm0, qword ptr [edx+eax*8] // edx = colors palette base  eax= texelcolor index.
			paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
	
			punpckhdq   mm1, mm1			//
			add			esi,1				//
		
			psrlq       mm1,mm2				//	[MMXCoShift]
			xor         eax,eax				//

			pand        mm1,mm3				//	[MMXCoMask] 
			//paddw       mm0,Fog				

			//paddw       Fog,[Fogdelta]      // or NOP....
			psraw       mm0,5               //  

			packuswb    mm0,mm0             //  Pack words to bytes.
			cmp			esi, esp            //  Cmp  X, PX;  

			movd        ecx,mm1             // 
			movq        mm1,mm7             //

			movd		[edi+esi*4-4], mm0  //  Store to screen.
			movq        mm0,mm4				//

			mov         al,byte ptr [ebx+ecx] //  Next texel.
			jb			ReloopInnerRender8    //

			//////////////////////////////////
			//
			//	#debug tag - end of light lerp
			//	mov          dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			psubw       mm4,mm5				//
			jb          ReloopInnerTex8     //
											//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
		    psubd       mm7,mm6             // undo preliminary delta-add from inner loop
			mov         eax,[SetupWalker]   //

			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	//////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			mov			esp,[NX]				    // Just do those last 1-7 ones...
			xor			eax,eax
			paddw       mm5,mm5
			mov         al,byte ptr [ebx+ecx]
			jmp			ReLoopInnerRender8       

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]

	}
	unguardSlow;
}// MMXNewRender







static void MMX32FogRender8() //
{
	guardSlow(MMX32FogRender8);
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm4                 //

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light & fog values.
			movq		mm4,[ebp]                    // Start light value.
			movq        mm2,[ebp+8]					 // Start   fog value.
			mov         eax,[NX]					 // End of current Subdiv #debug get from other register above?

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	      // next light value
			movq        mm3,[ebp +16 + 8]     // next fog value
			add         ebp,16                //

			psubsw       mm5,mm4			    // New minus current = light delta
			psubsw       mm3,mm2             // ,, ,, fog delta
			sub         eax,esi             

			cmp         eax,8               // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 
			cmp         eax,4         
			jnb         Full4loop           // 4-pixel span, usually part of a 4-7 pixel endspan.

			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // [[e]]
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // [[e]]
			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			paddw       mm5,mm5             // #debug optim make it work WITHOUT paddw OR move this....
			paddw       mm3,mm3             // #debug optim make it work WITHOUT paddw OR move this....

			// #debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
			
			// Alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		

			paddw		mm0, mm2             //  Add 14-bit fog. 
			psraw       mm0, FinalShift      //  14-bit result to 9 bits   
			paddw		mm2, mm3             //  Add FogDelta.
			packuswb    mm0, mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   //#debug tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:


#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif


	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.
	////////////////////////////////



	///////////////////////////////
	//
	// Prelim optim: perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	jmp Full8Loop
	DOALIGN16
		Full8Loop:

			psraw       mm5,3			    // * 1/8   
			psraw       mm3,3			    // * 1/8   

			movq        mm1,mm7            //
			xor         eax,eax            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 

			//
			// 7 Times unrolled interleaved inner loop 
			//

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+1*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+3*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+4*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3             //
			movd		[edi+esi*4+5*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7             //
			 paddd		 mm7,mm6			 //
			movq		mm0, mm4             // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12              // .
			 punpckhdq   mm1,mm1             // Copy high word to low word.
			paddw		mm4, mm5             // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]    // 
			paddw       mm0, mm2             // ad FlashOffset
			 pand        mm1,[MMXCoMask]     //    
			psraw       mm0,FinalShift       // 14-bit result to 9 bits   
			 movd        ecx,mm1             // 
			packuswb    mm0,mm0              // Pack words to bytes.
			paddw       mm2,mm3			     //
			movd		[edi+esi*4+6*4], mm0 // Store to screen. 	

			// End of unrolled inner loop

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq		mm0, mm4			   //  Copy lightvalue
			paddw		mm4, mm5               //  + mm5, signed 16-bit R,G,B LightDeltas 
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm0, mm2               // ad FlashOffset
			psraw       mm0,FinalShift		   //  14-bit result to 9 bits   
			packuswb    mm0,mm0				   //  Pack words to bytes
			paddw       mm2,mm3                //
			movd		[edi+esi*4+7*4], mm0   //  store to screen. 

			add         esi,8
			///////////////////////////////////
			mov         eax,[NX]             //  
			nop                              //
			cmp         esi, eax             //
			jb          ReloopInnerTex8      //
			jmp         EndOfInnerLight8     //
    ///////////////////////////////////////////






	///////////////////////////////////////////
	jmp Full4Loop
	DOALIGN16
		Full4Loop:

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; // 
			pmulhw		mm3,MMXDeltaAdjust[eax*8]; // 

			movq        mm1,mm7            //
			xor         eax,eax            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 

			paddw		mm3,mm3
			paddw		mm5,mm5

			//
			// 3 Times unrolled interleaved inner loop 
			//

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+1*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			// End of unrolled inner loop

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq		mm0, mm4			   //  Copy lightvalue
			paddw		mm4, mm5               //  + mm5, signed 16-bit R,G,B LightDeltas 
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm0, mm2               // ad FlashOffset
			psraw       mm0,FinalShift		   //  14-bit result to 9 bits   
			packuswb    mm0,mm0				   //  Pack words to bytes
			paddw       mm2,mm3                //
			movd		[edi+esi*4+3*4], mm0   //  store to screen. 

			add         esi,4
			//////////////////////////////////
			mov         eax,[NX]            //  
			cmp         esi, eax            //
			jnb         EndOfInnerLight8    //

			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			jmp ReloopInnerRender8          // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.

   ///////////////////////////////////////////

	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8   // skip tag  +skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX32FogRender





static void MMX32FogRender8A() // 
{
	guardSlow(MMX32FogRender8A);
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm4                 //

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light & fog values.
			movq		mm4,[ebp]                    // Start light value.
			movq        mm2,[ebp+8]					 // Start   fog value.
			mov         eax,[NX]					 // End of current Subdiv #debug get from other register above?

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	      // next light value
			movq        mm3,[ebp +16 + 8]     // next fog value
			add         ebp,16                //

			psubsw       mm5,mm4			    // New minus current = light delta
			psubsw       mm3,mm2             // ,, ,, fog delta
			sub         eax,esi             

			cmp         eax,8               // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 
			cmp         eax,4         
			jnb         Full4loop           // 4-pixel span, usually part of a 4-7 pixel endspan.

			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // [[e]]
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // [[e]]
			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			paddw       mm5,mm5             // #debug optim make it work WITHOUT paddw OR move this....
			paddw       mm3,mm3             // #debug optim make it work WITHOUT paddw OR move this....

			// #debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
			
			// Alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		

			paddw		mm0, mm2             //  Add 14-bit fog. 
			psraw       mm0, FinalShift      //  14-bit result to 9 bits   
			paddw		mm2, mm3             //  Add FogDelta.
			packuswb    mm0, mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   //#debug tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:


#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif


	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.
	////////////////////////////////



	///////////////////////////////
	//
	// Prelim optim: perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	jmp Full8Loop
	DOALIGN16
		Full8Loop:

			psraw       mm5,3			   // * 1/8   
			psraw       mm3,3			   // * 1/8   

			movq        mm1,mm7            //
			xor         eax,eax            //
			paddd       mm1,[KnightA1]     //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 

			//
			// 7 Times unrolled interleaved inner loop 
			//

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+1*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+3*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+4*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3             //
			movd		[edi+esi*4+5*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7             //
			 paddd		 mm7,mm6			 //
			 paddd       mm1,[KnightA0]      //
			movq		mm0, mm4             // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12              // .
			 punpckhdq   mm1,mm1             // Copy high word to low word.
			paddw		mm4, mm5             // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]    // 
			paddw       mm0, mm2             // ad FlashOffset
			 pand        mm1,[MMXCoMask]     //    
			psraw       mm0,FinalShift       // 14-bit result to 9 bits   
			 movd        ecx,mm1             // 
			packuswb    mm0,mm0              // Pack words to bytes.
			paddw       mm2,mm3			     //
			movd		[edi+esi*4+6*4], mm0 // Store to screen. 	

			// End of unrolled inner loop

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq		mm0, mm4			   //  Copy lightvalue
			paddw		mm4, mm5               //  + mm5, signed 16-bit R,G,B LightDeltas 
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm0, mm2               // ad FlashOffset
			psraw       mm0,FinalShift		   //  14-bit result to 9 bits   
			packuswb    mm0,mm0				   //  Pack words to bytes
			paddw       mm2,mm3                //
			movd		[edi+esi*4+7*4], mm0   //  store to screen. 

			add         esi,8
			///////////////////////////////////
			mov         eax,[NX]             //  
			nop                              //
			cmp         esi, eax             //
			jb          ReloopInnerTex8      //
			jmp         EndOfInnerLight8     //
    ///////////////////////////////////////////






	///////////////////////////////////////////
	jmp Full4Loop
	DOALIGN16
		Full4Loop:

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; // 
			pmulhw		mm3,MMXDeltaAdjust[eax*8]; // 

			movq        mm1,mm7            //
			xor         eax,eax            //
			paddd       mm1,[KnightA1]     //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 

			paddw		mm3,mm3
			paddw		mm5,mm5

			//
			// 3 Times unrolled interleaved inner loop 
			//

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+1*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightA0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			// End of unrolled inner loop

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq		mm0, mm4			   //  Copy lightvalue
			paddw		mm4, mm5               //  + mm5, signed 16-bit R,G,B LightDeltas 
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm0, mm2               // ad FlashOffset
			psraw       mm0,FinalShift		   //  14-bit result to 9 bits   
			packuswb    mm0,mm0				   //  Pack words to bytes
			paddw       mm2,mm3                //
			movd		[edi+esi*4+3*4], mm0   //  store to screen. 

			add         esi,4
			//////////////////////////////////
			mov         eax,[NX]            //  
			cmp         esi, eax            //
			jnb         EndOfInnerLight8    //

			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			jmp ReloopInnerRender8          // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.

   ///////////////////////////////////////////

	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8   // skip tag  +skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX32FogRender8A


static void MMX32FogRender8B() // 
{
	guardSlow(MMX32FogRender8B);
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm4                 //

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light & fog values.
			movq		mm4,[ebp]                    // Start light value.
			movq        mm2,[ebp+8]					 // Start   fog value.
			mov         eax,[NX]					 // End of current Subdiv #debug get from other register above?

	ReloopInnerTex8:                          // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	      // next light value
			movq        mm3,[ebp +16 + 8]     // next fog value
			add         ebp,16                //

			psubsw       mm5,mm4			    // New minus current = light delta
			psubsw       mm3,mm2             // ,, ,, fog delta
			sub         eax,esi             

			cmp         eax,8               // full 8-pixel span with it's own delta-TEX etc
			jnb         Full8loop 
			cmp         eax,4         
			jnb         Full4loop           // 4-pixel span, usually part of a 4-7 pixel endspan.

			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // [[e]]
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // [[e]]
			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			paddw       mm5,mm5             // #debug optim make it work WITHOUT paddw OR move this....
			paddw       mm3,mm3             // #debug optim make it work WITHOUT paddw OR move this....

			// #debug - pays off to have all remaining sizes as special cases (3,2,1,0 pixels)
	ReloopInnerRender8:  // Loop using current light & light delta's. 
			
			// Alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.		

			paddw		mm0, mm2             //  Add 14-bit fog. 
			psraw       mm0, FinalShift      //  14-bit result to 9 bits   
			paddw		mm2, mm3             //  Add FogDelta.
			packuswb    mm0, mm0             //  Pack words to bytes
			movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
		   //#debug tag - end of light lerp
		   // mov         dword ptr [edi+esi*4-4],0
			
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:


#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif


	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.
	////////////////////////////////



	///////////////////////////////
	//
	// Prelim optim: perform full unrolled (nonfogged) 8x loop
	//
	////////////////////////////////
	jmp Full8Loop
	DOALIGN16
		Full8Loop:

			psraw       mm5,3			   // * 1/8   
			psraw       mm3,3			   // * 1/8   

			movq        mm1,mm7            //
			xor         eax,eax            //
			paddd       mm1,[KnightB1]     //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 

			//
			// 7 Times unrolled interleaved inner loop 
			//

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+1*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+3*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+4*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3             //
			movd		[edi+esi*4+5*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7             //
			 paddd		 mm7,mm6			 //
			 paddd       mm1,[KnightB0]      //
			movq		mm0, mm4             // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12              // .
			 punpckhdq   mm1,mm1             // Copy high word to low word.
			paddw		mm4, mm5             // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]    // 
			paddw       mm0, mm2             // ad FlashOffset
			 pand        mm1,[MMXCoMask]     //    
			psraw       mm0,FinalShift       // 14-bit result to 9 bits   
			 movd        ecx,mm1             // 
			packuswb    mm0,mm0              // Pack words to bytes.
			paddw       mm2,mm3			     //
			movd		[edi+esi*4+6*4], mm0 // Store to screen. 	

			// End of unrolled inner loop

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq		mm0, mm4			   //  Copy lightvalue
			paddw		mm4, mm5               //  + mm5, signed 16-bit R,G,B LightDeltas 
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm0, mm2               // ad FlashOffset
			psraw       mm0,FinalShift		   //  14-bit result to 9 bits   
			packuswb    mm0,mm0				   //  Pack words to bytes
			paddw       mm2,mm3                //
			movd		[edi+esi*4+7*4], mm0   //  store to screen. 

			add         esi,8
			///////////////////////////////////
			mov         eax,[NX]             //  
			nop                              //
			cmp         esi, eax             //
			jb          ReloopInnerTex8      //
			jmp         EndOfInnerLight8     //
    ///////////////////////////////////////////






	///////////////////////////////////////////
	jmp Full4Loop
	DOALIGN16
		Full4Loop:

			pmulhw		mm5,MMXDeltaAdjust[eax*8]; // 
			pmulhw		mm3,MMXDeltaAdjust[eax*8]; // 

			movq        mm1,mm7            //
			xor         eax,eax            //
			paddd       mm1,[KnightB1]     //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 

			paddw		mm3,mm3
			paddw		mm5,mm5

			//
			// 3 Times unrolled interleaved inner loop 
			//

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+0*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB1]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+1*4], mm0 // Store to screen. 	

			mov         al, byte ptr [ebx+ecx] // Get the texel - ebx = Texture map base. 
			 movq        mm1,mm7            //
			 paddd		 mm7,mm6			//
			 paddd       mm1,[KnightB0]     //
			movq		mm0, mm4            // Copy lightvalue
			pmulhw		mm0, qword ptr [edx+eax*8]  // edx = colors palette base  eax= texelcolor index.
			 psllq       mm1,12             // .
			 punpckhdq   mm1,mm1            // Copy high word to low word.
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas.
			 psrlq       mm1,[MMXCoShift]   // 
			paddw       mm0, mm2            // ad FlashOffset
			 pand        mm1,[MMXCoMask]    //    
			psraw       mm0,FinalShift      // 14-bit result to 9 bits   
			 movd        ecx,mm1            // 
			packuswb    mm0,mm0             // Pack words to bytes.
			paddw       mm2,mm3
			movd		[edi+esi*4+2*4], mm0 // Store to screen. 	

			// End of unrolled inner loop

			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			movq		mm0, mm4			   //  Copy lightvalue
			paddw		mm4, mm5               //  + mm5, signed 16-bit R,G,B LightDeltas 
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			paddw       mm0, mm2               // ad FlashOffset
			psraw       mm0,FinalShift		   //  14-bit result to 9 bits   
			packuswb    mm0,mm0				   //  Pack words to bytes
			paddw       mm2,mm3                //
			movd		[edi+esi*4+3*4], mm0   //  store to screen. 

			add         esi,4
			//////////////////////////////////
			mov         eax,[NX]            //  
			cmp         esi, eax            //
			jnb         EndOfInnerLight8    //

			mov			esp,[NX]            // Just do those last 1-7 ones...
			xor			eax,eax             //
			jmp ReloopInnerRender8          // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.

   ///////////////////////////////////////////

	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8   // skip tag  +skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;
}// MMX32FogRender8B









/*

//
// Irregular Subdivision: using our ultracool ROLLED loop.
// Disadvantage: more discontinuities ??? currently stuff's interpolated from points along the edges, much better 
// thought to do some Subdivision thing that cuts at SCREEN positions, would give much less popping.
// - as long as they set in at fixed distances, you'll have much less popping..
// 

static void NewMMXFogRender8() // SUPER-GENERAL, non-unrolled renderer.
{
	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]                 
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	//-> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm4                 //

	//
	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	//
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta  
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace
	//
	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light & fog values.
			movq		mm4,[ebp]                    // current (start) light value.
			movq        mm2,[ebp+8]                  // current fog value
			mov         eax,[NX]				// End of current Subdiv #debug get from other register above?

	ReloopInnerTex8:							// Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]			// next light value
			movq        mm3,[ebp +16 + 8]		// next fog value
			add         ebp,16					//

			psubsw      mm5,mm4					// New minus current = light delta
			psubsw      mm3,mm2					// ,, ,, fog delta
			sub         eax,esi					// number of pixels to interpolate over.

			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // [[e]]
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // [[e]]
			mov			esp,[NX]				// Draw all.
			xor			eax,eax					//
			paddw       mm5,mm5					// #debug optim make it work WITHOUT paddw OR move this.
			paddw       mm3,mm3					// 



			ReloopInnerRender8:					// Loop using current light & light delta's. Getting the texel coordinate				
				psllq		mm1, 12				// psllq mm1,12  is SLOW for some weird reason.....
				paddw		mm4, mm5			// + mm5, signed 16-bit R,G,B LightDeltas 

				pmulhw		mm0, qword ptr [ebp+eax*8] //!!  // edx = colors palette base  eax= texelcolor index.
				paddd       mm7, mm6 		    
		
				xor         eax, eax			//  !! No matter what, first set of instr after PMULHW is EXPENSIVE... (why???)
				add esi,1						//  inc 		esi                 //  ++X
				
				punpckhdq   mm1, mm1			//
				cmp			esi, esp            //  cmp  X, PX;  

				psrlq       mm1,[MMXCoShift]	//
				paddw       mm0, mm2			//  ad Fogging

				pand        mm1,[MMXCoMask]     //
				psraw       mm0,5               //  14-bit result to 9 bits  5???  						

				paddw       mm2,mm3             //  fog += fogdelta
				packuswb    mm0,mm0             //  Pack words to bytes.

				movd        ecx,mm1             //
				movq        mm1,mm7             //

				movd		[edi+esi*4-4], mm0  //  Store to screen. //Access violation...
				movq        mm0,mm4             //

				mov         al,byte ptr [ebx+ecx]  // Get texel.
				jb			ReloopInnerRender8     //

			SingleLeadOut:


			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:

			mov         ecx,[NextXStore]    //  Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  //  X>0, perspective correction.


	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	////////////////////////////////

	DOALIGN16
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8   // skip tag  +skip dummy end-LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}
}// MMXFogRender

*/


/*
				__asm{

					//#debug make 1/odd  pixel a special case, all else can be unrolled.

					// Interleaved stuff == coordinate calculation.
					//jmp OldRender 

					mov     ecx,[Set]
					mov     esi,[SpanX0]
					mov     edx,[SpanX1]
					mov     [SavedEBP],ebp
					dec     edx   //#debug

					movq	mm7,[ecx]FMMXPolyCSetup.UV

					mov     ebx,[TexBase]			
					mov     edi,[ScreenDest]			
					mov     ebp,[MMXColors]				

					movq	mm4,[ecx]FMMXPolyCSetup.LightRGB
					movq    mm1,mm7
					movq	mm2,[ecx]FMMXPolyCSetup.FogRGB					
					psllq   mm1,12  

					mov			eax,edx
					paddd       mm7,mm6
					punpckhdq   mm1,mm1				// Copy high word to low word.

					sub			eax,esi				// eax == number of pixels to draw.
					psrlq       mm1,[MMXCoShift]	//       

					pand        mm1,[MMXCoMask]		//
					test		eax,eax

					movd		ecx,mm1					
					jnz			MoreThanOne


					// Singular pixel....
						xor			eax,eax			   // explicitly, for PII sake.
						mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
						//movq        mm1,mm7          // new UV
						movq		mm0,mm4            //  Copy lightvalue, should be 15-bits...
						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						paddw       mm0, mm2           //  ad Fogging
						psraw       mm0,5              //  14-bit result to 9 bits  5???  
						packuswb    mm0,mm0            //  Pack words to bytes.
						mov			ebp,[SavedEBP]     //
						movd		[edi+esi*4], mm0   //  Store to screen. //Access violation...
					jmp EndRender                      //
					/////////////////////////////////////

					DOALIGN16		// #debug fudge this so the Loop will start on paragraph boundary
					MoreThanOne:	// lead-in partly above

						xor         eax,eax
						movq        mm1,mm7           // new UV

						movq		mm0,mm4           //  Copy lightvalue, should be 15-bits...
						mov         al, byte ptr [ebx+ecx]

					ReloopInnerRender8:		  // Loop using current light & light delta's. Getting the texel coordinate				
						psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
						paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 

						pmulhw		mm0, qword ptr [ebp+eax*8] //!!  // edx = colors palette base  eax= texelcolor index.
						paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
				
						xor         eax, eax			//  !! No matter what, first set of instr after PMULHW is EXPENSIVE... (why???)
						add esi,1						//  inc 		esi                 //  ++X
						
						punpckhdq   mm1, mm1			//
						cmp			esi, edx            //  cmp  X, PX;  

						psrlq       mm1,[MMXCoShift]	//
						paddw       mm0, mm2			//  ad Fogging

						pand        mm1,[MMXCoMask]     //
						psraw       mm0,5               //  14-bit result to 9 bits  5???  						

						paddw       mm2,mm3             //  fog += fogdelta
						packuswb    mm0,mm0             //  Pack words to bytes.
 
						movd        ecx,mm1               //
						movq        mm1,mm7               //

						movd		[edi+esi*4-4], mm0    //  Store to screen. //Access violation...
						movq        mm0,mm4               //

						mov         al,byte ptr [ebx+ecx] //

						jb			ReloopInnerRender8    //

				    // Lead-out, write last pixel:     
					pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
					paddw       mm0, mm2           //  ad Fogging
					psraw       mm0,5              //  14-bit result to 9 bits  5???  
					packuswb    mm0,mm0            //  Pack words to bytes.
					mov			ebp,[SavedEBP]     //
					movd		[edi+esi*4], mm0   //  Store to screen. 
					EndRender:
				}
*/



/*
static void MMX32OldMaskedRender8()
{
	guardSlow(MMXMaskedRender8);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

	DOALIGN16

 ReloopMaskedNewSpan8:
 ReloopMaskedNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm4                 //

	movq    mm3,[MMXFlashOffset13]			//

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopMaskedPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light & fog values.
			movq		mm4,[ebp]
			//movq        mm2,[ebp+8]
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

	ReloopMaskedInnerTex8:                        // calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // new minus current = light delta
			sub         eax,esi
			cmp         eax,8
			jb          Sub8LightStretch    // need different delta scaling

			psraw       mm5,3			    // * 1/8  
			lea			esp,[esi+8]         // just do those last 1-7 ones...
			xor			eax,eax

	
	ReloopMaskedInnerRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test		eax,eax				//
			jz          SkipMaskedPixel

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
		
			paddw       mm0, mm3            // ad FlashOffset
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4], mm0    //  store to screen.

			SkipMaskedPixel:				//
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.
			jb			ReloopMaskedInnerRender8  //
											//
	//////////////////////////////////////////
											
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopMaskedInnerTex8     //
											//									
		EndofInnerLight8:


#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif


			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopMaskedPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif


	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopMaskedNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopMaskedNewSpan8                  // -2, more spans on this line.

    ////////////////////// 

	DOALIGN16
	Sub8LightStretch:
			test		eax,eax                  // Ever taken ???
			jz			EndofInnerLight8         // 0 = end of subdvv...
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			mov			esp,[NX]				 // Just do those last 1-7 ones...
			xor			eax,eax
			paddw       mm5,mm5
			jmp			ReloopMaskedInnerRender8       // Don't adjust for now....

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		                

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]

	}
	unguardSlow;
}// MMXOldMaskedRender

*/



static void MMX32MaskedRender8()
{
	guardSlow(MMXMaskedRender8);
	
	__asm {
	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopMaskedNewSpan8:
 ReloopMaskedNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP =[0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm4,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    mm3,[MMXFlashOffset13]			//
	movq    MMXCoShift, mm4                 // movq    MMXCoMask,  mm4              

	///////////////////////////////////////////////////
	mov			ecx,[eax]FTexSetup.X         // get delta

	DOALIGN16 

	nop
	nop

	ReloopMaskedPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// Get initial light value
			movq		mm4,[ebp]			// movq        mm2,[ebp+8] get inital fog value...
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,mm2             //    
			movd        ecx,mm1             //  

	ReloopMaskedInnerTex8:                        // calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // New minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // need different delta scaling

			psraw       mm5,3			    // 3 

			lea			esp,[esi+8]         // just do those last 1-7 ones...
			xor			eax,eax				// 

	ReloopMaskedInnerRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.		
			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! Compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //   

			paddw       mm0, mm3            // ad FlashOffset
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			psraw       mm0, FinalShift     //  14-bit result to 9 bits   
			paddd		mm7, mm6			//  

			packuswb    mm0, mm0            //  Pack words to bytes				
			punpckhdq   mm1,mm1             //  Copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  

			movd		[edi+esi*4], mm0    //  Store to screen.
			pand        mm1,mm2             //  

			inc 		esi                 //  ++X
			movd        ecx,mm1             //  #pairing error...

			cmp			esi,esp             //  Cmp  X, PX;  always runs 8 times.
			jb			ReloopMaskedInnerRender8  //  

	EndLoopMaskedInnerRender8:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax				 //
			jb          ReloopMaskedInnerTex8    //

#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0  //  store to screen.
#endif
											//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
		    psubd       mm7,mm6             // undo preliminary delta-add from inner loop
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopMaskedPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0   //  Store to screen.
#endif



	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopMaskedNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopMaskedNewSpan8                  // -2, more spans on this line.


	////////////////////////
	// DOALIGN16		#debug
	// jne SkipMaskedPixel
	//
	nop
	nop

	SkipMaskedPixel: // don't draw, update light & coordinates.
			psllq       mm1,12              //   

			punpckhdq   mm1,mm1             //  Copy high word to low word 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			// 				

			pand        mm1,mm2             //  
			inc 		esi                 //  ++X

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             //  #pairing error...
			jb			ReloopMaskedInnerRender8  //

			nop
			jmp			EndLoopMaskedInnerRender8 //


							
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	// Same as above, but most probably the last stretch of a span.
	//
	//////////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler

			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			paddw       mm5,mm5
			jmp			ReloopMaskedInnerRender8       


	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}// MMXMaskedRender









static void MMX32FogMaskedRender8()
{
	guardSlow(MMX32FogMaskedRender8);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon   // Light/Fog presampled values.
	mov		edi,[ScreenDest]
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm4                 //

	// EBX = texture map base
	// ECX = scratchspace
	// EDX = palette base
	//
	// esp = PX = end of light lerping stretch
	// esi = X
	// edi = display
	// ebp = outer loop: points into PHOTON array for light (and sometimes fog)..
	// 
	// mm7  Tex     // Texel coordinates
	//
	// mm2  Fog
	// mm4  Light 
	//
	// ........delta's etc (subject to change as multiple strands may be needed for unrolling..)
	// mm6  TexDelta    =>! May well want to use this from memory instead so you have more MMX regs avaibable for unrolling.
	// mm5  LightDelta
	// mm3  FogDelta
	// 
	// mm1,mm0  workspace

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light & fog values.
			movq		mm4,[ebp]
			movq        mm2,[ebp+8]

			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

	ReloopInnerTex8:                        // calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	    // next light value
			movq        mm3,[ebp +16 + 8]   // next fog value
			add         ebp,16              //

			psubsw       mm5,mm4			    // new minus current = light delta
			psubsw       mm3,mm2			    // new minus current = fog delta

			sub         eax,esi
			cmp         eax,8
			jb          Sub8LightStretch

			psraw       mm5,3			    // * 1/8   #debug not applicable here really
			psraw       mm3,3			    // * 1/8   #debug not applicable here really 

			lea			esp,[esi+8]         // just do those last 1-7 ones...
			xor			eax,eax


	ReloopInnerRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.

			//#debug: completely superfluous. Will make the superoptimized one for 8, one for 4, 3, 2, and 1 !!!!
			// The one for 4 also functions automatically as our '4-sample' one, and 3,2,1 all use simple DUP-scemes,
			// in fact they just don't sum delta's !!!
		
			movq        mm1,mm7            //
			paddd		mm7,mm6			   //
			psllq       mm1,12             //  
			punpckhdq   mm1,mm1            // copy high word to low word 
			psrlq       mm1,[MMXCoShift]   // 
			pand        mm1,[MMXCoMask]    //    
			movd        ecx,mm1            // 
			/////////////////////////////////

			movq		mm0, mm4            //  Copy lightvalue
			mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			test		eax,eax
			jz          SkipMaskedPixel
			
			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
		
			paddw		mm0, mm2            //  Add 14-bit fog.
											//
			psraw       mm0,FinalShift      //  14-bit result to 9 bits   
			packuswb    mm0,mm0             //  Pack words to bytes
			movd		[edi+esi*4], mm0    //  store to screen.

		SkipMaskedPixel:
			paddw		mm2, mm3            //  Add FogDelta.
			inc 		esi                 //  ++X
			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.
			jb			ReloopInnerRender8  //
											//
	//////////////////////////////////////////
	//
	// #debug tag - end of light lerp
	//  mov         dword ptr [edi+esi*4-4],0
	//
	//		
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //
											//									
		EndofInnerLight8:

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8               // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8       // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3              // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

    ////////////////////// 
	/*
	DOALIGN16
		Sub8LightStretch:
			test    eax,eax
			jz      EndofInnerLight8         // 0 = end of subdvv...
			//sub     eax,esi
			mov     esp,[NX]                 // just do those last 1-7 ones...
			xor     eax,eax
			jmp     ReloopInnerRender8       // don't adjust for now....
	*/

	DOALIGN16
		Sub8LightStretch:
			test		eax,eax                  // Ever taken ???
			jz			EndofInnerLight8         // 0 = end of subdvv...
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // Delta scaler
			mov			esp,[NX]				 // Just do those last 1-7 ones...
			xor			eax,eax
			paddw		mm5,mm5
			paddw		mm3,mm3
			jmp			ReloopInnerRender8       // Don't adjust for now....

	///////////////////
	DOALIGN16         //
	NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8 + 8   // skip tag + dummy LIT 

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax 

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]
	}

	unguardSlow;

}// MMXFogMaskedRender






static void MMX32TranslucentRender8()
{
	guardSlow(MMX32TranslucentRender8);
	
	__asm {
	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP =[0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // movq    MMXCoMask,  mm4              

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]					 // movq        mm2,[ebp+8] get inital fog value...
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,mm2             //    
			movd        ecx,mm1             //  

	ReloopInnerTex8:                        // calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // New minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // need different delta scaling

			psraw       mm5,3			    // 3 
			lea			esp,[esi+8]         // just do those last 1-7 ones...

	ReloopInnerRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.		
			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //  

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			psraw       mm0, FinalShift     //  14-bit result to 9 bits   

			paddd		mm7, mm6			// 
			packuswb    mm0, mm0            //  Pack words to bytes
					
			movd        mm3,[edi+esi*4]     //  GET screen..EXPENSIVE OPERATION !! (Try doing 2 (aligned) pixels at a time???)
			punpckhdq   mm1,mm1             //  copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  
			paddusb     mm0,mm3             //  ADD screen into it.. //#optimize !

			movd		[edi+esi*4], mm0    //  Store to screen.
			pand        mm1,mm2             //  

			inc 		esi                 //  ++X
			movd        ecx,mm1             //  #pairing error...

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.
			jb			ReloopInnerRender8  //  

	EndLoopInnerRender8:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif
											//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
		    psubd       mm7,mm6             // undo preliminary delta-add from inner loop
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif



	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	////////////////////////
	// DOALIGN16		#debug
	// jne SkipMaskedPixel
	//
	nop
	nop
	nop

	SkipMaskedPixel: // don't draw, update light & coordinates.
			psllq       mm1,12              //   

			punpckhdq   mm1,mm1             //  Copy high word to low word 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			// 				

			pand        mm1,mm2             //  
			inc 		esi                 //  ++X

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             //  #pairing error...
			jb			ReloopInnerRender8  //

			nop
			jmp			EndLoopInnerRender8 //


							
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	// Same as above, but most probably the last stretch of a span.
	//
	//////////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler

			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			paddw       mm5,mm5
			jmp			ReLoopInnerRender8       


	

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               

	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}// MMXTranslucentRender




//
// STIPPLED rendering.
//
// Number of pixels per subtask:  (X + Parity) >> 1;
// Parity can stay the same across all subdivisions because they're even numbers.
// if Parity, prestep Texture+light by single delta, prestep DEST by 1, then double the 2 delta's;
// if no parity, just double the 2 delta's.
// 
//

static void MMX32StippledRender8()
{
	guardSlow(MMXStippledRender8);

	// checkerboard: defined by bit 0 of X  xor bit 0 of Y !
	
	__asm {

	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....

	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

	//
	//	Prestep ?
	//
	//	test     ebx,ebx		// Parity
	//	jnz      NoPrestep		//
	//	add      esi,1
	//	NoPrestep:
	//

	DOALIGN16
	nop
	nop
	nop

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP = [0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // movq    MMXCoMask,  mm4              
	movq    mm3,[MMXFlashOffset13]			//

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]					 // movq        mm2,[ebp+8] get inital fog value...
			paddd       mm6,mm6             // double the tex delta.
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 

			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // Copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,mm2             //    
			movd        ecx,mm1             //  

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // Next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // New minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // Need different delta scaling

			psraw       mm5,2			    // 3   but double delta for stippling 
			lea			esp,[esi+8]         // just do those last 1-7 ones...

	ReloopInnerStippledRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! Compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //   

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			punpckhdq   mm1, mm1            //  Copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			//  proceed through texture 

			paddw       mm0, mm3            
			pand        mm1,mm2             //  

			psraw       mm0, FinalShift     //  14-bit result to 9 bits.
			add 		esi,2               //  ++X

			movd        ecx,mm1             //  
			packuswb    mm0, mm0            //  Pack words to bytes

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd		[edi+esi*4 - 2*4], mm0    //  Store to screen. // unavoidable delay, Upipe..
			jb			ReloopInnerStippledRender8  //   Unpairable, Upipe

	EndLoopInnerStippledRender8:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd       mm7,mm6             // undo preliminary delta-add from inner loop

			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                   // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8           // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8					// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                  // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	nop
	nop
	nop

	SkipMaskedPixel: // don't draw, update light & coordinates using delta's...
			psllq       mm1, 12             //   
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			punpckhdq   mm1, mm1            //  Copy high word to low word 
			paddd       mm7, mm6			//

			psrlq       mm1,[MMXCoShift]    //  

			add 		esi,2               //  ++X
			pand        mm1,mm2             //  

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             // 
			jb			ReloopInnerStippledRender8  //

			nop
			jmp			EndLoopInnerStippledRender8 //

						
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	//	Same as above, but most probably the last stretch of a span.
	//
	//////////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler

			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			psllw       mm5,2						// *2, *2 to double delta for stippling 
			jmp			ReLoopInnerStippledRender8       

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}// MMX32StippledRender





static void MMX32FogStippledRender8()
{
	guardSlow(MMX32FogStippledRender8);

	// Checkerboard: defined by bit 0 of X  xor bit 0 of Y !
	
	__asm {

	mov		eax,[SetupWalker]

	mov		[SavedEBP], ebp
	mov		[SavedESP], esp
								
	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]				//
	mov		esi,[eax]FTexSetup.X			// First X....

	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

	//
	//	Prestep ?
	//
	//	test     ebx,ebx		// Parity
	//	jnz      NoPrestep		//
	//	add      esi,1
	//	NoPrestep:
	//

	DOALIGN16
	nop
	nop
	nop

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP = [0], ACCESS VIOLATION: EAX == 0.....
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm2              

	movq    mm3,[MMXFlashOffset13]			//

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...   // ACCESS error.... eax = 0...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             // ERROR ???
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]					 // light
			movq		mm2,[ebp+8]					 // fog

			paddd       mm6,mm6             // double the tex delta.
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above.
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 

			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // Copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             //  

	ReloopInnerTex8:                        // Calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	    // Next light value
			movq        mm3,[ebp +16+8]	    // Next fog value
			add         ebp,16				//

			psubsw      mm5,mm4			    // New minus current = light delta
			psubsw      mm3,mm2			    // New minus current = fog delta

			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // Need different delta scaling

			psraw       mm5,2			    // 3   but double delta for stippling 
			psraw       mm3,2			    // 3   but double delta for stippling 
			lea			esp,[esi+8]         // just do those last 1-7 ones...

	ReloopInnerStippledRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12. 
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq		mm0, mm4            //  Copy lightvalue 
			xor         eax, eax            //

			movq        mm1, mm7            // 
			mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			test		eax,eax				// #debug bailout creates the hangup ?? #compiler error
			jz			SkipMaskedPixel		// ! Compiled to WRONG address !!

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psllq       mm1,12              //   

			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas.
			punpckhdq   mm1, mm1            //  Copy high word to low word.

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			//  proceed through texture 

			paddw       mm0, mm2            // fog add
			paddw       mm2, mm3            // fog += delta
			pand        mm1,[MMXCoMask]     //  

			psraw       mm0, FinalShift     //  14-bit result to 9 bits.
			add 		esi,2               //  ++X

			movd        ecx,mm1             //  
			packuswb    mm0, mm0            //  Pack words to bytes

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd		[edi+esi*4 - 2*4], mm0    //  Store to screen. // unavoidable delay, Upipe..
			jb			ReloopInnerStippledRender8  //   Unpairable, Upipe

	EndLoopInnerStippledRender8:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
			psubd       mm7,mm6             // undo preliminary delta-add from inner loop

			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

	///////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                   // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8           // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16					// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                  // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.

	///////////////////////////////
	nop
	nop
	nop

	SkipMaskedPixel: // don't draw, update light & coordinates using delta's...
			psllq       mm1, 12             //   
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			punpckhdq   mm1, mm1            //  Copy high word to low word 
			paddd       mm7, mm6			//

			paddw       mm2,mm3

			psrlq       mm1,[MMXCoShift]    //  

			add 		esi,2               //  ++X
			pand        mm1,[MMXCoMask]     //  

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.

			movd        ecx,mm1             // 
			jb			ReloopInnerStippledRender8  //

			nop
			jmp			EndLoopInnerStippledRender8 //

						
    //////////////////////  X = -1,-2, or -3...
	//
	// A  X=-1: CoSetup ?: new mipmap         (always after another non-empty setup structure)
	// B  X=-2  Cosetup ?: end of span but more spans this line. (==NEVER the only tag on a line..)
	// C  X=-3, CoSetup ?: end of line but more lines after this line  (perhaps only empty lines though!)
	// D  X=-4, CoSetup ?: end of poly.
	//
	//	Same as above, but most probably the last stretch of a span.
	//
	//////////////////////////

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			pmulhw		mm3,MMXDeltaAdjust[eax*8];  // Delta scaler

			mov			esp,[NX]				    // Just do those last 1-7 ones...	
			xor			eax,eax
			psllw       mm5,2						// *2, *2 to double delta for stippling 
			psllw       mm3,2						// *2, *2 to double delta for stippling 
			jmp			ReLoopInnerStippledRender8       

	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	//Emergency:
	mov		ebp, [SavedEBP]
	mov		esp, [SavedESP]

	}
	unguardSlow;
}// MMX32FogStippledRender








static void MMX32ModulatedRender8()
{
	guardSlow(MMXModulatedRender8);

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm3,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm2,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm3                 // 
	movq    MMXCoMask,  mm2					//

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ +8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]					 // movq        mm2,[ebp+8] get inital fog value...
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             //  
			
	ReloopInnerTex8:                        // calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +8]	    // next light value
			add         ebp,8				//

			psubsw      mm5,mm4			    // new minus current = light delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // need different delta scaling

			psraw       mm5,3			    // 3 
			xor         eax,eax
			lea			esp,[esi+8]         // just do those last 1-7 ones...
			mov         al, byte ptr [ebx+ecx]

	ReloopInnerRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12.
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq		mm0, mm4            //  Copy lightvalue 
			movq        mm1, mm7            // 

			//xor         eax, eax          //
			//mov         al,  byte ptr [ebx+ecx]	//  Get the texel - ebx = Texture map base. 

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psubd       mm3,mm3

			punpcklbw   mm3,[edi+esi*4]		// Get screen  -> cache delay fills pmulhw latency?
			psllq       mm1,12				//  
				
			psrlw       mm3,1				// Screen data from 16 bits to 15 bits.
			paddd		mm7, mm6			// 

			pmulhw      mm0,mm3				// 15 bits * ??? modulated values...
			punpckhdq   mm1,mm1             // copy high word to low word 

			psrlq       mm1,[MMXCoShift]    //  
			paddw		mm4, mm5            // + mm5, signed 16-bit R,G,B LightDeltas 

			pand        mm1,[MMXCoMask]     // 
			psrlw       mm0,FinalShift -1   // Adjustment 

			packuswb    mm0,mm0			    // 
			movd        ecx,mm1             //  

			movd		[edi+esi*4],mm0		// 
			inc 		esi                 // ++X

			xor         eax,eax				//  
			cmp			esi,esp             // cmp  X, PX;  always runs 8 times.

			mov         al, byte ptr [ebx+ecx]  // Texel for next one..
			jb			ReloopInnerRender8		// 

	//EndLoopInnerRender:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif
											//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
		    psubd       mm7,mm6             // undo preliminary delta-add from inner loop
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif


	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,8				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	///////////////////
	/*
	je SkipMaskedPixel
	DOALIGN16
	nop 

	SkipMaskedPixel: // don't draw, update light & coordinates.
			psllq       mm1,12              //  

			punpckhdq   mm1,mm1             //  copy high word to low word 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			// 				

			pand        mm1,[MMXCoMask]     //  
			inc 		esi                 //  ++X

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.
			movd        ecx,mm1             //  #pairing error...
			jb			ReloopInnerRender8  //

			nop
			jmp			EndLoopInnerRender  //
	*/


    ////////////////////// X = -1,-2, or -3...
	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			mov			esp,[NX]				    // Just do those last 1-7 ones...
			xor			eax,eax
			paddw       mm5,mm5
			mov         al, byte ptr [ebx+ecx]
			jmp			ReLoopInnerRender8       


	//////////////////////
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]

	}
	unguardSlow;
}// MMXModulatedRender






static void MMX32FogModulatedRender8()
{
	guardSlow(MMX32FogModulatedRender8);

	FMMX  MMX32FogDelta;

	__asm {
	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp
	mov		[SavedESP], esp

	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		edi,[ScreenDest]					//
	mov		esi,[eax]FTexSetup.X			// First X....
	mov     edx,[MMXColors]					//
	test	esi,esi							//
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	mov     ecx,[eax]FTexSetup.CoSetup		// MIP 
	movq	mm7,[eax]FTexSetup.Tex          // TEX
	add     eax, 1* (3*8)                   // Advance FTexSetup by one setup structure.
	// -> esi==X still at right place, no need to reload..
 
	mov		ebx,[ecx]FMipSetup.Data         // 
	movq    mm2,[ecx]FMipSetup.HybridShift  // Slow- no pairing opportunities for mem-accesing MMX instructions.
	movq    mm4,[ecx]FMipSetup.HybridMask   //
	movq    MMXCoShift, mm2                 // 
	movq    MMXCoMask,  mm4					//

	///////////////////////////////////////////////////
			mov			ecx,[eax]FTexSetup.X         // get delta
	ReloopPerspective8:								 //
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Tex       // new Tex delta
			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1 * (3*8)               // Step over the delta structure just used
			mov         [NextXStore],ecx             //
			mov			[SetupWalker],eax			 // save next X - whether delta, tag, or X...

	// get initial light value
			movq		mm4,[ebp]	        // movq        mm2,[ebp+8] get inital fog value...
			movq		mm3,[ebp +8]		// movq        mm2,[ebp+8] get inital fog value...
			mov         eax,[NX]		    // end of current Subdiv #debug get from other register above?

			// New coordinate calc lead-in #debug interleave with above
			movq        mm1,mm7             // 
			paddd		mm7,mm6			    // 
			psllq       mm1,12              //  
			punpckhdq   mm1,mm1             // copy high word to low word 
			psrlq       mm1,[MMXCoShift]    // 
			pand        mm1,[MMXCoMask]     //    
			movd        ecx,mm1             //  
			
	ReloopInnerTex8:                        // calculate new light lerps: FogDelta and LightDelta
	        movq        mm5,[ebp +16]	    // next light value
			movq        mm2,[ebp +16 +8]    // next light value
			add         ebp,16				//

			psubsw      mm5,mm4			    // new minus current = light delta
			psubsw      mm2,mm3			    // new minus current = fog   delta
			sub         eax,esi				//
			cmp         eax,8				//
			jbe         Sub8LightStretch    // Need different delta scaling

			psraw       mm2,3			    // 
			psraw       mm5,3			    // 
			movq        [MMX32FogDelta],mm2 //

			xor			eax,eax				// 
			lea			esp,[esi+8]         // Just do those last 1-7 ones...
			mov         al, byte ptr [ebx+ecx]

	ReloopInnerRender8:  // Loop using current light & light delta's. Getting the texel coordinate - only 6 instructions.
	
			// New hybrid MMX coordinate method. Disadvantage: always less V bits available 20 = 32-12.
			// alternative with pmaddw is also 7 instructions, more accurate, but has latency problems with the multiply.
			movq		mm0, mm4				//  Copy lightvalue 
			movq        mm1, mm7				// 

			// Modulated skipping ONLY FOR UNLIT ones !
			// test		eax,eax					//
			// jz		SkipMaskedPixel			//  

			pmulhw		mm0, qword ptr [edx+eax*8]  //  edx = colors palette base  eax= texelcolor index.
			psubd       mm2,mm2

			punpcklbw   mm2,[edi+esi*4] // get screen  -> cache delay fills pmulhw latency?
			psllq       mm1,12          //  
				
			psrlw       mm2,1     // screen data from 16 bits to 15 bits.
			paddd		mm7, mm6			 // 

			pmulhw      mm0,mm2   // 15 bits * ??? modulated values...
			punpckhdq   mm1,mm1              //  copy high word to low word 

			psrlq       mm1, [MMXCoShift]    //  

			paddw		mm4, mm5             //  + mm5, signed 16-bit R,G,B LightDeltas 

			movq        mm2, mm3
			psraw       mm2, FinalShift      // #optimize

			paddw       mm3, [MMX32FogDelta] //
			psrlw       mm0, FinalShift -1   // Adjustment 

			pand        mm1,[MMXCoMask]     //  
			paddw       mm0, mm2             // ADJUSTED final-shifted fog...

			movd        ecx,mm1             //  #pairing error...
			packuswb    mm0,mm0				// 

			inc 		esi                 //  ++X
			xor         eax,eax

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.
			mov         al, byte ptr [ebx+ecx]

			movd		[edi+esi*4-4],mm0
			jb			ReloopInnerRender8  //  

	//EndLoopInnerRender:
			//////////////////////////////////
			//
			// #debug tag - end of light lerp
			// mov         dword ptr [edi+esi*4-4],0
			//		
			//////////////////////////////////
											//
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			nop                             //
			cmp         esi, eax            //
			jb          ReloopInnerTex8     //

#if INDICATOR
		  	pcmpeqd      mm0,mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif
											//
			mov         ecx,[NextXStore]    // Stored, to avoid an AGI here...
		    psubd       mm7,mm6             // undo preliminary delta-add from inner loop
			mov         eax,[SetupWalker]   //
			test		ecx,ecx			    // 
			jns			ReloopPerspective8  // X>0, perspective correction.

#if MIPINDICATOR
		  	pcmpeqd      mm0, mm0
	        movd		[edi+esi*4 - 1*4 ], mm0    //  store to screen.
#endif


	///////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8                     // skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8             // new mip, always valid 2 structs after this...
	///////////////////////////////

			// Skip over the extra LIT coordinate at the end of each span
			add			eax,8 
			// [[E]] -> only when there really is one at end of span/line/whatever
			add         ebp,16				// This is for the one extra at END of EVERY span .
			 
			cmp         ecx,-3                    // greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 
			//StartNewSpan8: Only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X			// First X....
			jmp			ReloopNewSpan8                  // -2, more spans on this line.


	/*
	///////////////////
	je SkipMaskedPixel
	//
	// DOALIGN16
	//
	nop 

	SkipMaskedPixel: // don't draw, update light & coordinates.
			psllq       mm1,12              //  

			punpckhdq   mm1,mm1             //  copy high word to low word 
			paddw		mm4, mm5            //  + mm5, signed 16-bit R,G,B LightDeltas 
			paddw		mm3,[MMX32FogDelta] //  + mm5, signed 16-bit R,G,B LightDeltas 

			psrlq       mm1,[MMXCoShift]    //  
			paddd		mm7, mm6			// 				

			pand        mm1,[MMXCoMask]     //  
			inc 		esi                 //  ++X

			cmp			esi,esp             //  cmp  X, PX;  always runs 8 times.
			movd        ecx,mm1             //  #pairing error...
			jb			ReloopInnerRender8  //

			nop
			jmp			EndLoopInnerRender  //
	*/

    ////////////////////// X = -1,-2, or -3...

	DOALIGN16
	Sub8LightStretch:
			pmulhw		mm5,MMXDeltaAdjust[eax*8];  // Delta scaler
			pmulhw		mm2,MMXDeltaAdjust[eax*8];  // Delta scaler
			mov			esp,[NX]				    // Just do those last 1-7 ones...
			xor			eax,eax
			paddw       mm2,mm2
			paddw       mm5,mm5
			movq        [MMX32FogDelta],mm2
			mov         al, byte ptr [ebx+ecx]
			jmp			ReLoopInnerRender8

	//////////////////////
						//
	NegativeStart:	    //  ESI has X < 0, if -3 end of line, if -4 end of poly.
	add		eax, 8+8    //  skip tag skip dummy lit
		               
	DOALIGN16
	EndOfLine: 
	mov		[SetupWalker],eax

	mov		esp, [SavedESP]
	mov		ebp, [SavedEBP]

	}
	unguardSlow;

}// MMX32FogModulatedRender








static void MMXLight8()
{
	guardSlow(MMXLight8);

__asm {

	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp

	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		esi,[eax]FTexSetup.X			// First X....
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	movq	mm3,[eax]FTexSetup.Lit           // Lightmap  coordinates.
	//movq    [MMXLit],mm7
	add     eax, 1* (3*8)                    // Advance FTexSetup by one setup structure.
	///////////////////////////////////////////
	mov		ecx,[eax]FTexSetup.X             // get delta X

	

	///////////////////////////////////////////////////
	ReloopPerspective8:                              //
	// + ECX = new X    , now get Delta-Tex          //
			movq        mm6,[eax]FTexSetup.Lit       // new Lit delta
			movq		mm0,mm3					 // dup current lit position to obtain integer bits

			add         ecx,esi						 // deltaX + X
			movq		mm1,mm3					 // dup current lit position to obtain fractional bits

			movq        [MMXLitDelta],mm6            //
			psrld		mm0, 16      // Shift right logical the 2 dwords: get integers into low words of the 2 dwords.

			pand		mm0, [LightMip.Mask]  // Mask out: 2 dwords:  (VSize-1):(USize-1)
			psrlw		mm1,1				  // make 15-bit fractional coordinates (shift 0 into bit 15)

			packssdw mm0, mm0	    // Pack doublewords to words -> low 32 bits are now (Vcoord:Ucoord)
			mov         dword ptr [NX],ecx           // NX = X + Xdelta

			pmaddwd  mm0, [LightMip.Mult]  // Mul by (SrcMip.USize<<16) : 1 (2 different multiplications) and sum
		 								   // (Vcoord * Usize) + (Ucoord * 1) -> make up lowest 32 bits !
			paddd    mm3, mm6		       // MMXLitDelta  //[MMXLitDelta]     // Lit + LitDelta

			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1*(3*8)                 // Step over the delta structure just used

			mov         [NextXStore],ecx             //
			mov			[PhotonSetupWalker],eax		 // save next X - whether delta, tag, or X...
	///////////////////////////////////////////////////

	////// Lead-in...
			
			movd     ebx, mm0       // Our index into the lightmap.
			movq     mm2,mm1        // 15-bit fractional lightmap index.

			// Unpacking...
			punpcklwd mm1,mm1     // 00:00:xx:16  to  xx:xx:16:16 
			mov       edi, [LightMip.USize] // #debug keep throughout routine
			punpckldq mm1,mm1     // 00:00:16:16  to  16:16:16:16 - quad expansion of fractional U coordinate 
			mov       edx, [LightMip.Data]  //
			punpckhwd mm2,mm2     // xx:16:00:00  to  xx:xx:16:16 
			punpckldq mm2,mm2     // xx:xx:16:16 to 16:16:16:16 -  fractional V coordinate 
			lea       ebx, [ebx*4 + edx]    //  Full pointer into lightmap.

	/////////////////////////////////////
	ReloopInnerLight8:                 // calculate new light lerps: FogDelta and LightDelta		
			// call		SampleLight; 
			// SampleLight explicitly, for unrolling purposes.
					
			// Now MM1 and MM2 have the fractional coordinates,
			// Load integer indices for the lightmap

			pxor      mm4,mm4	  //  zeroes needed when unpacking bytes to words...
			pxor      mm5,mm5				

			punpcklbw mm4, [ebx+0]			//  A - low 4 bytes of source dword into 4 words  
			pxor      mm6,mm6               

			punpcklbw mm5, [ebx+4]			//  B
			pxor      mm7,mm7

			punpcklbw mm6, [ebx+edi*4]		//  C
			psubsw    mm5, mm4				//	B - A (signed)

			punpcklbw mm7, [ebx+edi*4+4]	//  D
			//  Calculate bilinear interpolated value  
			psubsw		mm7, mm6   // D - C (signed)

			pmulhw		mm5, mm1		// 
				movq		mm0,mm3     // dup current lit position to obtain integer bits

			pmulhw		mm7, mm1		//
				movq		mm1,mm3     // dup current lit position to obtain fractional bits

				psrld		mm0, 16     // Shift right logical the 2 dwords: get integers into low words of the 2 dwords.
				psrlw		mm1, 1      // make 15-bit fractional coordinates (shift 0 into bit 15)

				pand		mm0, [LightMip.Mask]  // Mask out: 2 dwords:  (VSize-1):(USize-1)
			psllw	    mm5, 1     // shift left: to get into proper alignment for adding mm4

			psllw	    mm7, 1     // shift left: to get into proper alignment for adding mm6			
			paddw		mm5, mm4   // I = X(B-A) + A; -> always unsigned;      I = XB + (A-XA)

				packssdw mm0, mm0	  // Pack doublewords to words -> low 32 bits are now (Vcoord:Ucoord)
			paddw		mm7, mm6   // J = X(D-C) + C; -> COULD be a signed number... ?!   

				pmaddwd  mm0, [LightMip.Mult]  // Mul by (SrcMip.USize<<16) : 1 (2 different multiplications) and sum
			psubsw		mm7, mm5   // J - I

				paddd    mm3,[MMXLitDelta]     // Lit + LitDelta
			pmulhw		mm7, mm2   // Y (J-I) 

				movq     mm2,mm1      // 15-bit fractional lightmap index.
			psrlw		mm5, 1     // pshift right: proper alignment for adding to mm7...

				movd     ebx, mm0     // Our index into the lightmap.
				punpcklwd mm1,mm1     // 00:00:xx:16  to  xx:xx:16:16 

 			paddw		mm7, mm5   // Y (J-I) + I     //#debug latency.... 
				punpckldq mm1,mm1     // 00:00:16:16  to  16:16:16:16 - quad expansion of fractional U coordinate 

			pmulhw		mm7,MMXFlashCompress  // 14-bit times 15-bit double-bright == 13 bit, 12 bit reference level.
				punpckhwd mm2,mm2     // xx:16:00:00  to  xx:xx:16:16 

				punpckldq mm2,mm2     // xx:xx:16:16  to  16:16:16:16 - fractional V coordinate 
				lea       ebx, [ebx*4 + edx]    //  Full pointer into lightmap.

			add         esi,8               //  X += LightSub, THOUGH we may actually have traversed LESS here

			psllw		mm7,2                 // 14-bit (reference-level) light output. 
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 

			movq		[ebp], mm7         // Store. 

			add			ebp,8              // advance light setup ptr....
			//////////////////////////////////
			cmp         esi, eax            //  8 full more to do ?
			jb          ReloopInnerLight8   //  only do it when it still FITS in there...
											//  #debug take advantage of the fact that
											//  LookNextSubdiv will never have persp.correction after non-8 lightsub.....

			psubd      mm3,[MMXLitDelta]    // undo last interleaved coordinate delta addition

		//LookNextSubdiv:							//      
			mov         ecx,[NextXStore]			// Stored, to avoid an AGI here...
			mov         eax,[PhotonSetupWalker]		//
			test		ecx,ecx						// 
			jns			ReloopPerspective8			// X>0, perspective correction.
	//////////////////////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8						// skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8				// new mip, always valid 2 structs after this...

			// #debug [e] not always needed though -depends on how many remaining pixels ;-)
			movq        mm3,[eax]//[eax]      // one, over the 2dword tag..
			add         eax,8
			mov         [PhotonSetupWalker],eax
			call		SampleLight	// add          ebp,8   //affects eax!!!!!!!
			mov         eax,[PhotonSetupWalker]		//#debug use another FREE var here ?!!!?!!

			// DUP light at end of a complete span
			// movq        [ebp],mm7
			// add			ebp,8

	///////////////////////////////
			cmp         ecx,-3						// greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 

	//// StartNewSpan8: // only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X		// First X....
			jmp			ReloopNewSpan8              // -2, more spans on this line.
			
    //////////////////////  X = -1,-2, or -3...

	// BILINEAR SAMPLING /////////////////////////////////////
	DOALIGN16
	SampleLight:
			movq     mm0,mm3      // dup current lit position to obtain integer bits
			movq     mm1,mm3      // dup current lit position to obtain fractional bits

			paddd    mm3,[MMXLitDelta]    // Lit + LitDelta
			psrld    mm0, 16      // Shift right logical the 2 dwords: get integers into low words of the 2 dwords.

			pand     mm0, [LightMip.Mask]  // Mask out: 2 dwords:  (VSize-1):(USize-1)
			psrlw    mm1,1        // make 15-bit fractional coordinates (shift 0 into bit 15)

			packssdw mm0, mm0	  // Pack doublewords to words -> low 32 bits are now (Vcoord:Ucoord)
			movq     mm2,mm1       // 15-bit fractional lightmap index

			pmaddwd  mm0, [LightMip.Mult]  // Mul by (SrcMip.USize<<16) : 1 (2 different multiplications) and sum
 		                                   // (Vcoord * Usize) + (Ucoord * 1) -> make up lowest 32 bits !
			// Unpacking...
			punpcklwd mm1,mm1     // 00:00:xx:16  to  xx:xx:16:16 

			mov       edi, [LightMip.USize] // #debug keep throughout routine
			punpckldq mm1,mm1     // 00:00:16:16  to  16:16:16:16 - quad expansion of fractional U coordinate 

			mov       edx, [LightMip.Data]  //
			punpckhwd mm2,mm2     // xx:16:00:00  to  xx:xx:16:16 

			movd     ebx, mm0      // Our index into the lightmap.
			punpckldq mm2,mm2    // xx:xx:16:16 to 16:16:16:16 -  fractional V coordinate 

			lea       ebx, [ebx*4 + edx]    //  Full pointer into lightmap.

			// Now MM1 and MM2 have the fractional coordinates,
			// Load integer indices for the lightmap, then 

			pxor      mm4,mm4				//  zeroes needed when unpacking bytes to words...
			pxor      mm5,mm5				//
			punpcklbw mm4, [ebx+0]			//  A - low 4 bytes of source dword into 4 words  
			pxor      mm6,mm6               
			punpcklbw mm5, [ebx+4]			//  B
			pxor      mm7,mm7
			punpcklbw mm6, [ebx+edi*4]		//  C
			psubsw    mm5, mm4				//	B - A (signed)
			punpcklbw mm7, [ebx+edi*4+4]	//  D

			//
			//  Calculate bilinear interpolated value  
			//
			//		A   B        X = eax       Y = ebx
			// 
			//		C   D
			//

			// psubsw     mm5, mm4   // B - A (signed) ^
			psubsw		mm7, mm6   // D - C (signed)
			pmulhw		mm5, mm1   // 
			pmulhw		mm7, mm1   //

			psllw	    mm5, 1    // shift left: to get into proper alignment for adding mm4
			psllw	    mm7, 1    // shift left: to get into proper alignment for adding mm6

			paddw		mm5, mm4   // I = X(B-A) + A; -> always unsigned;      I = XB + (A-XA)
			paddw		mm7, mm6   // J = X(D-C) + C; -> COULD be a signed number... ?!   

			psubsw		mm7, mm5   // J - I

			pmulhw		mm7, mm2   //   Y (J-I) 
			psrlw		mm5, 1     //   pshift right: proper alignment for adding to mm7...
 			paddw		mm7, mm5   //   Y (J-I) + I     //#debug latency.... 

			//14-bits output...
			//
			// RGBA result in 16:16:16:16 format.
			//

			pmulhw		mm7,MMXFlashCompress  // 14-bit times 15-bit double-bright == 13 bit, 12 bit reference level.
			psllw		mm7,2                 // 14-bit (reference-level) light output. 

			////////////////////////////////
			// FOR NON-fogged light, this is our 14-bit output. Add FlashOffset in final renderer !

			movq		[ebp], mm7         // Store. 
			add			ebp,8              // advance light setup ptr....
			retn
			///////////////////////////////////////////////////////////



		///////////////////
		DOALIGN16
		NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
		//add		eax, 8   // skip tag
		EndOfLine: 
		//mov		[PhotonSetupWalker],eax // Unneeded... Texture mapper will advance the SetupWalker ptr.

		mov		ebp, [SavedEBP]
	}

	unguardSlow;
}




/*   Non'rolled' light: 

static void MMXLight8()
{
	//	
	// PERFECT light: The last LIT coordinate delta will jump out of the real span-end whenever the span is not
	// a multiple of eight.
	//
	// Creating a perfect coordinate involves shifting right the light coordinate to do /8, then jump into
	// a routine which adds 1/2/3/4/5/6/7 versions of it....
	// Workaround now: just sample at 4 (ie use half the delta) whenever the spot is at 4/5/6/7, sample at 0
	// for 0/1/2/3.
	// -> 'miss' for the sampling is then never worse than 3 pixels either direction;
	// this should be acceptable especially because inherent in sampling is that we sample only every 8 pixels, between
	// which might be wildly different values anyway...
	//

__asm {

	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp

	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		esi,[eax]FTexSetup.X			// First X....
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	movq	mm3,[eax]FTexSetup.Lit           // Lightmap  coordinates.
	//movq    [MMXLit],mm7
	add     eax, 1* (3*8)                    // Advance FTexSetup by one setup structure.
 
	///////////////////////////////////////////
	mov		ecx,[eax]FTexSetup.X             // get delta X

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Lit       // new Lit delta
			movq        [MMXLitDelta],mm6

			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1*(3*8)                 // Step over the delta structure just used
			mov         [NextXStore],ecx
			mov			[PhotonSetupWalker],eax		 // save next X - whether delta, tag, or X...

	/////////////////////////////////////////
	ReloopInnerLight8:                     // calculate new light lerps: FogDelta and LightDelta		
			/// BILINEAR SAMPLING
			call		SampleLight;
			// (1-Fog)*Light*MMXFlashCompress => Photon LIGHT
			// Fog*MMXFlashCompress  + MMXFlashOffset => Photon FOG
			//
			// In texture renderer: Light*Texel+Fog ; Light and Fog obtained by lerping.
			// + !! Special cases for NON-fog, highly optimized etc.
			//
			// WrapupSample:
			//////////////////////////////////
											//
			add         esi,8               //  X += LightSub, THOUGH we may actually have traversed LESS here
			mov         eax,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			cmp         esi, eax            //  8 full more to do ?
			jb          ReloopInnerLight8   //  only do it when it still FITS in there...
											//  #debug take advantage of the fact that
											//  LookNextSubdiv will never have persp.correction after non-8 lightsub.....
		//LookNextSubdiv:							//      
			mov         ecx,[NextXStore]			// Stored, to avoid an AGI here...
			mov         eax,[PhotonSetupWalker]		//
			test		ecx,ecx						// 
			jns			ReloopPerspective8			// X>0, perspective correction.
	//////////////////////////////////////////////////
	// X<0 -> new mipmap if last X >0
			add         eax,8						// skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8				// new mip, always valid 2 structs after this...

			// #debug [e] not always needed though -depends on how many remaining pixels ;-)
			movq        mm3,[eax]//[eax]      // one, over the 2dword tag..
			add         eax,8
			mov         [PhotonSetupWalker],eax
			call		SampleLight	// add          ebp,8   //affects eax!!!!!!!
			mov         eax,[PhotonSetupWalker]		//#debug use another FREE var here ?!!!?!!

			// DUP light at end of a complete span
			// movq        [ebp],mm7
			// add			ebp,8

	///////////////////////////////
			cmp         ecx,-3						// greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 

	//// StartNewSpan8: // only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X		// First X....
			jmp			ReloopNewSpan8              // -2, more spans on this line.
			
    ////////////////////// but X = -1,-2, or -3...

	// BILINEAR SAMPLING /////////////////////////////////////
	DOALIGN16
	SampleLight:
			movq     mm0,mm3      // dup current lit position to obtain integer bits
			movq     mm1,mm3      // dup current lit position to obtain fractional bits
			psrld    mm0, 16      // Shift right logical the 2 dwords: get integers into low words of the 2 dwords.
			pand     mm0, [LightMip.Mask]  // Mask out: 2 dwords:  (VSize-1):(USize-1)
			psrlw    mm1,1        // make 15-bit fractional coordinates (shift 0 into bit 15)

			packssdw mm0, mm0	  // Pack doublewords to words -> low 32 bits are now (Vcoord:Ucoord)
			pmaddwd  mm0, [LightMip.Mult]  // Mul by (SrcMip.USize<<16) : 1 (2 different multiplications) and sum
 		                                   // (Vcoord * Usize) + (Ucoord * 1) -> make up lowest 32 bits !
			paddd    mm3,[MMXLitDelta]    // Lit + LitDelta
			movd     ebx, mm0      // Our index into the lightmap.
			movq     mm2,mm1       // 15-bit fractional lightmap index

			// Unpacking...
			punpcklwd mm1,mm1     // 00:00:xx:16  to  xx:xx:16:16 

			mov       edi, [LightMip.USize] // #debug keep throughout routine
			punpckldq mm1,mm1     // 00:00:16:16  to  16:16:16:16 - quad expansion of fractional U coordinate 

			mov       edx, [LightMip.Data]  //
			punpckhwd mm2,mm2     // xx:16:00:00  to  xx:xx:16:16 

			punpckldq mm2,mm2    // xx:xx:16:16 to 16:16:16:16 -  fractional V coordinate 
			lea       ebx, [ebx*4 + edx]    //  Full pointer into lightmap.

			// Now MM1 and MM2 have the fractional coordinates,
			// Load integer indices for the lightmap, then 

			pxor      mm4,mm4				//  zeroes needed when unpacking bytes to words...
			pxor      mm5,mm5				//
			punpcklbw mm4, [ebx+0]			//  A - low 4 bytes of source dword into 4 words  
			pxor      mm6,mm6               
			punpcklbw mm5, [ebx+4]			//  B
			pxor      mm7,mm7
			punpcklbw mm6, [ebx+edi*4]		//  C
			psubsw    mm5, mm4				//	B - A (signed)
			punpcklbw mm7, [ebx+edi*4+4]	//  D

			//
			//  Calculate bilinear interpolated value  
			//
			//		A   B        X = eax       Y = ebx
			// 
			//		C   D
			//

			//psubsw     mm5, mm4   // B - A (signed)
			psubsw		mm7, mm6   // D - C (signed)
			pmulhw		mm5, mm1   // 
			pmulhw		mm7, mm1   //

			psllw	    mm5, 1    // shift left: to get into proper alignment for adding mm4
			psllw	    mm7, 1    // shift left: to get into proper alignment for adding mm6

			paddw		mm5, mm4   // I = X(B-A) + A; -> always unsigned;      I = XB + (A-XA)
			paddw		mm7, mm6   // J = X(D-C) + C; -> COULD be a signed number... ?!   

			psubsw		mm7, mm5   // J - I

			pmulhw		mm7, mm2   //   Y (J-I) 
			psrlw		mm5, 1     //   pshift right: proper alignment for adding to mm7...
 			paddw		mm7, mm5   //   Y (J-I) + I     //#debug latency.... 

			//14-bits output...
			//
			// RGBA result in 16:16:16:16 format.
			//

			pmulhw		mm7,MMXFlashCompress  // 14-bit times 15-bit double-bright == 13 bit, 12 bit reference level.
			psllw		mm7,2                 // 14-bit (reference-level) light output. 

			////////////////////////////////
			// FOR NON-fogged light, this is our 14-bit output. Add FlashOffset in final renderer !

			movq		[ebp], mm7         // Store. 
			add			ebp,8              // advance light setup ptr....
			retn
			///////////////////////////////////////////////////////////



		///////////////////
		DOALIGN16
		NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
		//add		eax, 8   // skip tag
		EndOfLine: 
		//mov		[PhotonSetupWalker],eax // Unneeded... Texture mapper will advance the SetupWalker ptr.

		mov		ebp, [SavedEBP]
	}
}

*/



static void MMXFogLight8()
{
	//	
	// PERFECT light: The last LIT coordinate delta will jump out of the real span-end whenever the span is not
	// a multiple of eight.
	//
	// Creating a perfect coordinate involves shifting right the light coordinate to do /8, then jump into
	// a routine which adds 1/2/3/4/5/6/7 versions of it....
	// Workaround now: just sample at 4 (ie use half the delta) whenever the spot is at 4/5/6/7, sample at 0
	// for 0/1/2/3.
	// -> 'miss' for the sampling is then never worse than 3 pixels either direction;
	// this should be acceptable especially because inherent in sampling is that we sample only every 8 pixels, between
	// which might be wildly different values anyway...
	//
	// #debug make 'naked' function so we can ReTn from  anywhere...

	guardSlow(MMXFogLight8);

__asm {

	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp

	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		esi,[eax]FTexSetup.X			// First X....
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	movq	mm7,[eax]FTexSetup.Lit           // Lightmap  coordinates.
	movq    [MMXLit],mm7
	add     eax, 1* (3*8)                    // Advance FTexSetup by one setup structure.
 
	///////////////////////////////////////////
	mov		ecx,[eax]FTexSetup.X             // get delta X

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Lit       // new Lit delta
			movq        [MMXLitDelta],mm6

			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1*(3*8)                 // Step over the delta structure just used
			mov			[PhotonSetupWalker],eax		 // save next X - whether delta, tag, or X...

	ReloopInnerLight8:                     // calculate new light lerps: FogDelta and LightDelta		
	/// BILINEAR SAMPLING /////////////////////////////////////
			movq		mm7,[MMXLit]
			//////////////////////
			call		SampleFogLight ;// uses: EAX EBX EDI EDX    = ECX=free !
			/////////////////////// reserved ESI=X  EBP=photonsetupwalker 
			add         esi,8               //  X += LightSub, THOUGH we may actually have traversed LESS here
			mov         ebx,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			cmp         esi, ebx            //  8 full more to do ?
			jb          ReloopInnerLight8   //  only do it when it still FITS in there...
											// 
											//  #debug take advantage of the fact that
											//  LookNextSubdiv will never have persp.correction after non-8 lightsub.....
			//LookNextSubdiv:						//
			mov         eax,[PhotonSetupWalker]		//
			test		ecx,ecx						// 
			jns			ReloopPerspective8			// X>0, perspective correction.
													//
	//////////////////////////////////////////////////
	//
	// X<0 -> new mipmap if last X >0
	//
			add         eax,8						// skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8				// new mip, always valid 2 structs after this...


	// #debug [e] not always needed though -depends on how many remaining pixels ;-)
			movq         mm7,[eax]  // one, over the 2dword tag..
			add          eax,8          //
			mov          [PhotonSetupWalker],eax
			call		 SampleFogLight	// add          ebp,8
			mov          eax,[PhotonSetupWalker]

	///////////////////////////////
			cmp         ecx,-3						// greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 

	//// StartNewSpan8: // only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X		// next X....
			jmp			ReloopNewSpan8              // -2, more spans on this line.

			//#optimize - should fall through on default ????
			
    //////////////////////  X = -1,-2, or -3...
	jmp ExtraSample
	DOALIGN16
	ExtraSample:
			// minus HALF the coordinate's delta (ie, step back..) wil be useful here.
			movq		mm6,[MMXLitDelta]
			movq		mm7,[MMXLit]
			psrad		mm6,1
			psubd		mm7,mm6						// Sample using minus-half-delta...
			call		SampleFogLight

			mov         eax,[PhotonSetupWalker]
			cmp         ecx,-3						// greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 

			mov			esi,[eax+8]FTexSetup.X		// First X....
			add         eax,8                       // Sux having to reload !!!!
			jmp			ReloopNewSpan8              // -2, more spans on this line.
													//
		//////////////////////////////////////////////
		DOALIGN16
		SampleFogLight: //#optimize !!!!!!!!!!! #debug

			movq     mm1,mm7      // dup current lit position to obtain fractional bits
			movq     mm0,mm7      // dup current lit position to obtain integer bits
			psrld    mm0, 16      // Shift right logical the 2 dwords: get integers into low words of the 2 dwords.
			pand     mm0, [LightMip.Mask]  // Mask out: 2 dwords:  (VSize-1):(USize-1)
			psrlw    mm1,1        // make 15-bit fractional coordinates (shift 0 into bit 15)

			packssdw mm0, mm0	  // Pack doublewords to words -> low 32 bits are now (Vcoord:Ucoord)
			pmaddwd  mm0, [LightMip.Mult]  //  Mul by (SrcMip.USize<<16) : 1 (2 different multiplications) and sum
 		                                   //  (Vcoord * Usize) + (Ucoord * 1) -> make up lowest 32 bits !
			paddd    mm7,[MMXLitDelta]     //  Lit + LitDelta
			movd     eax, mm0      // Our index into the lightmap.
			movq     mm2,mm1       // 15-bit fractional lightmap index
			movq     [MMXLit],mm7  // save...

			// Unpacking...
			punpcklwd mm1,mm1     // 00:00:xx:16  to  xx:xx:16:16 

			mov       edi, [LightMip.USize] // 
			punpckldq mm1,mm1     // 00:00:16:16  to  16:16:16:16 - quad expansion of fractional U coordinate 

			mov       edx, [LightMip.Data]  //
			punpckhwd mm2,mm2     // xx:16:00:00  to  xx:xx:16:16 

			punpckldq mm2,mm2    // xx:xx:16:16 to 16:16:16:16 -  fractional V coordinate 
			lea       ebx, [eax*4 + edx]    //  Full pointer into lightmap. 

			// Now MM1 and MM2 have the fractional coordinates,
			// Load integer indices for the lightmap, then 
												 	    
			pxor      mm4,mm4              //  zeroes needed when unpacking bytes to words...
			pxor      mm5,mm5              //

			punpcklbw mm4, [ebx+0]	       //  A - low 4 bytes of source dword into 4 words  
			pxor      mm6,mm6               

			punpcklbw mm5, [ebx+4]	       //  B
			pxor      mm7,mm7

			punpcklbw mm6, [ebx+edi*4]     //  C
			psubsw     mm5, mm4   // B - A (signed)

			punpcklbw mm7, [ebx+edi*4+4]   //  D

			//
			//  Calculate bilinear interpolated value  
			//
			//		A   B        X = eax       Y = ebx
			// 
			//		C   D
			//

			// psubsw     mm5, mm4   // B - A (signed)
			
			pmulhw    mm5, mm1   // 
			psubsw     mm7, mm6   // D - C (signed)

			pmulhw    mm7, mm1   //
			psllw	   mm5, 1    // shift left: to get into proper alignment for adding mm4

			psllw	   mm7, 1    // shift left: to get into proper alignment for adding mm6

			paddw     mm5, mm4   // I = X(B-A) + A; -> COULD be a signed number... ?!
			paddw     mm7, mm6   // J = X(D-C) + C; -> COULD be a signed number... ?!

			psubsw     mm7, mm5   // J - I

			pmulhw    mm7, mm2   //  Y (J-I) // U pipe only 
			psrlw     mm5, 1      //  pshift right: proper alignment for adding to mm7...
 			paddw     mm7, mm5   //  Y (J-I) + I     //#debug latency....

			// 14-bit result...
			//
			// RGBA result in 16:16:16:16 format.
			//

			///////////////////////// FOG
			mov       edx, [LightMip.FogData]  //
			lea       ebx, [eax*4 + edx]       //  Full pointer into lightmap.

			// Now MM1 and MM2 have the fractional coordinates,
			// Load integer indices for the lightmap, then 
												 	    
			pxor      mm4,mm4              //  zeroes needed when unpacking bytes to words...
			pxor      mm5,mm5              //
			punpcklbw mm4, [ebx+0]	       //  A - low 4 bytes of source dword into 4 words  
			pxor      mm6,mm6               
			punpcklbw mm5, [ebx+4]	       //  B
			pxor      mm3,mm3
			punpcklbw mm6, [ebx+edi*4]     //  C
			punpcklbw mm3, [ebx+edi*4+4]   //  D

			//
			//  Calculate bilinear interpolated value  
			//
			//		A   B        X = eax       Y = ebx
			// 
			//		C   D
			//
			
			psubsw     mm5, mm4   // B - A (signed)

			pmulhw    mm5, mm1   // 
			psubsw     mm3, mm6   // D - C (signed)

			pmulhw    mm3, mm1   //
			psllw	   mm5, 1    // shift left: to get into proper alignment for adding mm4

			psllw	   mm3, 1    // shift left: to get into proper alignment for adding mm6

			paddw     mm5, mm4   // I = X(B-A) + A; -> COULD be a signed number... ?!
			paddw     mm3, mm6   // J = X(D-C) + C; -> COULD be a signed number... ?!

			psubsw     mm3, mm5   // J - I

			pmulhw    mm3, mm2   //  Y (J-I) // U pipe only 
			psrlw     mm5, 1      //  pshift right: proper alignment for adding to mm3...

			pmulhw    mm7, MMXFlashCompress  // FlashCompress: 15-bit doublebright 15 bits, but "1" actually 14...
 
 			paddw     mm3, mm5   //  Y (J-I) + I     //#debug latency....

			///////////////////////// FOG 14-bit in MM3 // LIGHT 14-bit in MM7
			// (1-Fog)*Light*MMXFlashCompress => Photon LIGHT
			// Fog*MMXFlashCompress  + MMXFlashOffset => Photon FOG
			
#if ALPHAFOG
			//
			//
			// #DEBUG #RGBA-specific fogging with special darkening component:
			// Darken background according to '1-A' as opposed to 
			// expand highest word to 4 words: 
			//
			movq        mm4, [MMX14Bits]       // 
			psubsw      mm4, mm3               // 1-fog = fogcompress		 

			psrlq		mm4,16+16+16   // use only highest 16 bits 
			punpcklwd	mm4,mm4       // 00:00:xx:16  to  xx:xx:16:16 
			punpckldq	mm4,mm4       // 00:00:16:16  to  16:16:16:16 - quad expansion of fractional U coordinate 

#else
			movq       mm4, [MMX14Bits]       // 
			psubsw     mm4, mm3               // 1-fog = fogcompress		 
#endif
		 
			pmulhw    mm3, MMXFlashCompress  // Fogoffset * FlashCompress   = 14*14 = 12 bits 
			 
			//psrlw     mm3,1                // keep at 12-bits level //#DEBUG NO !!!
			pmulhw    mm7, mm4               // light * flashcompress * (1-fogmap)... 14*14*14 = 10 bits left.
			paddw     mm3, MMXFlashOffset13  // + 13-bit FlashOffset
			psllw     mm7,4                  // make 14 bits again.(actually full 15 bit)
		
			//////////////////// 
			movq      [ebp+8], mm3        // Store Fog   - 13-bit scale.
			movq      [ebp+0], mm7        // Store Light - 14-bit scale.

			add       ebp,16    
			ret							  //
   		////////////////////////////////////

		DOALIGN16
		///////////////////
		NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
		//add		eax, 8   // skip tag
		EndOfLine: 
		//mov		[PhotonSetupWalker],eax // Unneeded... Texture mapper will advance the SetupWalker ptr.


		mov		ebp, [SavedEBP]
	}
	unguardSlow;
}



static void MMXScaledFogLight8()
{

 // SCALED fog light - output ONLY the lightmap which has been compressed, do not
 // output any fog offset; this is for translucent/modulated textures.
	guardSlow(MMXScaledFogLight8);

__asm {

	mov		eax,[SetupWalker]
	mov		[SavedEBP], ebp

	mov		ebp,offset Photon				// Light/Fog presampled values.
	mov		esi,[eax]FTexSetup.X			// First X....
	test	esi,esi
	js		NegativeStart   				// Negative tag at beginning == always end of line.

 ReloopNewSpan8:
 ReloopNewMip8: // New Mip's X-value already in esi,  MIP ptr == ecx

	movq	mm7,[eax]FTexSetup.Lit           // Lightmap  coordinates.
	movq    [MMXLit],mm7
	add     eax, 1* (3*8)                    // Advance FTexSetup by one setup structure.
 
	///////////////////////////////////////////
	mov		ecx,[eax]FTexSetup.X             // get delta X

	ReloopPerspective8:
	// + ECX = new X    , now get Delta-Tex
			movq        mm6,[eax]FTexSetup.Lit       // new Lit delta
			movq        [MMXLitDelta],mm6			

			add         ecx,esi						 // deltaX + X
			mov         dword ptr [NX],ecx           // NX = X + Xdelta
			mov         ecx,[eax+ 8*3]FTexSetup.X    // next X...
			add			eax, 1*(3*8)                 // Step over the delta structure just used
			mov			[PhotonSetupWalker],eax		 // save next X - whether delta, tag, or X...

 
	//
	// Do {..} while (8 or more to do) else fixup.
	//
	ReloopInnerLight8:                     // calculate new light lerps: FogDelta and LightDelta		
	/// BILINEAR SAMPLING /////////////////////////////////////
			movq     mm7,[MMXLit]
			//////////////////////
			call  SampleFogLight ;// uses: EAX EBX EDI EDX    = ECX=free !
			/////////////////////// reserved ESI=X  EBP=photonsetupwalker 
			add         esi,8               //  X += LightSub, THOUGH we may actually have traversed LESS here
			mov         ebx,[NX]            //  Inner loop pixel counter (always 8 here though) #debug 
			cmp         esi, ebx            //  8 full more to do ?
			jb          ReloopInnerLight8   //  only do it when it still FITS in there...
											// 
											//  #debug take advantage of the fact that
											//  LookNextSubdiv will never have persp.correction after non-8 lightsub.....
											//
											//
		// LookNextSubdiv:						
													//
			mov         eax,[PhotonSetupWalker]		//
			test		ecx,ecx						// 
			jns			ReloopPerspective8			// X>0, perspective correction.
													//
	//////////////////////////////////////////////////
	//
	// X<0 -> new mipmap if last X >0
	//
			add         eax,8						// skip 2 dwords and point to next setup structure
	        cmp         ecx,-1
			je          ReloopNewMip8				// new mip, always valid 2 structs after this...

			movq         mm7,[eax]		// One, over the 2dword tag..
			add          eax,8          //
			mov          [PhotonSetupWalker],eax
			call		 SampleFogLight	// add	ebp,8
			mov          eax,[PhotonSetupWalker]
			
	///////////////////////////////
			cmp         ecx,-3						// greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 

	//// StartNewSpan8: // only happens for structures obscured by stuff in front 
			mov			esi,[eax]FTexSetup.X		// next X....
			jmp			ReloopNewSpan8              // -2, more spans on this line.

			//#optimize - should fall through on default ????
			
    ////////////////////// Last CoSetup = 0 but X = -1,-2, or -3...
	jmp ExtraSample
	DOALIGN16
	ExtraSample:
			// minus HALF the coordinate's delta (ie, step back..) wil be useful here.
			movq		mm6,[MMXLitDelta]
			movq		mm7,[MMXLit]
			psrad		mm6,1
			psubd		mm7,mm6  //Sample using minus-half-delta...
			call		SampleFogLight

			mov         eax,[PhotonSetupWalker]
			cmp         ecx,-3						// greater == -2, lower = -4  eq = -3
			jle         EndOfLine                 

			mov			esi,[eax+8]FTexSetup.X		// First X....
			add         eax,8                       // Sux having to reload !!!!
			jmp			ReloopNewSpan8              // -2, more spans on this line.
			

		////////////////////////////////////////////////////////////////
		DOALIGN16
		SampleFogLight: //#optimize !!!!!!!!!!! #debug

			movq     mm1,mm7      // dup current lit position to obtain fractional bits
			movq     mm0,mm7      // dup current lit position to obtain integer bits
			psrld    mm0, 16      // Shift right logical the 2 dwords: get integers into low words of the 2 dwords.
			pand     mm0, [LightMip.Mask]  // Mask out: 2 dwords:  (VSize-1):(USize-1)
			psrlw    mm1,1        // make 15-bit fractional coordinates (shift 0 into bit 15)
			packssdw mm0, mm0	  // Pack doublewords to words -> low 32 bits are now (Vcoord:Ucoord)
			pmaddwd  mm0, [LightMip.Mult]  // Mul by (SrcMip.USize<<16) : 1 (2 different multiplications) and sum
 		                                   // (Vcoord * Usize) + (Ucoord * 1) -> make up lowest 32 bits !
			paddd    mm7,[MMXLitDelta]    // Lit + LitDelta
			movd     eax, mm0      // Our index into the lightmap.
			movq     mm2,mm1       // 15-bit fractional lightmap index
			movq     [MMXLit],mm7  // save...

			// Unpacking...
			punpcklwd mm1,mm1     // 00:00:xx:16  to  xx:xx:16:16 

			mov       edi, [LightMip.USize] // 
			punpckldq mm1,mm1     // 00:00:16:16  to  16:16:16:16 - quad expansion of fractional U coordinate 

			mov       edx, [LightMip.Data]  //
			punpckhwd mm2,mm2     // xx:16:00:00  to  xx:xx:16:16 

			punpckldq mm2,mm2    // xx:xx:16:16 to 16:16:16:16 -  fractional V coordinate 
			lea       ebx, [eax*4 + edx]    //  Full pointer into lightmap. 

			// Now MM1 and MM2 have the fractional coordinates,
			// Load integer indices for the lightmap, then 
												 	    
			pxor      mm4,mm4              //  zeroes needed when unpacking bytes to words...
			pxor      mm5,mm5              //
			punpcklbw mm4, [ebx+0]	       //  A - low 4 bytes of source dword into 4 words  
			pxor      mm6,mm6               
			punpcklbw mm5, [ebx+4]	       //  B
			pxor      mm7,mm7
			punpcklbw mm6, [ebx+edi*4]     //  C
			psubsw    mm5, mm4   // B - A (signed)
			punpcklbw mm7, [ebx+edi*4+4]   //  D

			//
			//  Calculate bilinear interpolated value  
			//
			//		A   B        X = eax       Y = ebx
			// 
			//		C   D
			//

			 //psubsw     mm5, mm4   // B - A (signed)

			 pmulhw    mm5, mm1   // 
			 psubsw     mm7, mm6   // D - C (signed)

			 pmulhw    mm7, mm1   //
			 psllw	   mm5, 1    // shift left: to get into proper alignment for adding mm4

			 psllw	   mm7, 1    // shift left: to get into proper alignment for adding mm6

			 paddw     mm5, mm4   // I = X(B-A) + A; -> COULD be a signed number... ?!
			 paddw     mm7, mm6   // J = X(D-C) + C; -> COULD be a signed number... ?!

			 psubsw     mm7, mm5   // J - I

			 pmulhw    mm7, mm2   //  Y (J-I) // U pipe only 
			 psrlw     mm5, 1      //  pshift right: proper alignment for adding to mm7...
 			 paddw     mm7, mm5   //  Y (J-I) + I     //#debug latency....

			 // 14-bit result...
			 //
			 // RGBA result in 16:16:16:16 format.
			 //

			///////////////////////// FOG
			mov       edx, [LightMip.FogData]  //
			lea       ebx, [eax*4 + edx]       //  Full pointer into lightmap.

			// Now MM1 and MM2 have the fractional coordinates,
			// Load integer indices for the lightmap, then 
												 	    
			pxor      mm4,mm4              //  zeroes needed when unpacking bytes to words...
			pxor      mm5,mm5              //
			punpcklbw mm4, [ebx+0]	       //  A - low 4 bytes of source dword into 4 words  
			pxor      mm6,mm6               
			punpcklbw mm5, [ebx+4]	       //  B
			pxor      mm3,mm3
			punpcklbw mm6, [ebx+edi*4]     //  C
			psubsw    mm5, mm4   // B - A (signed)
			punpcklbw mm3, [ebx+edi*4+4]   //  D

			//
			//  Calculate bilinear interpolated value  
			//
			//		A   B        X = eax       Y = ebx
			// 
			//		C   D
			//




			 //psubsw     mm5, mm4   // B - A (signed)

			 pmulhw    mm5, mm1    // 
			 psubsw     mm3, mm6   // D - C (signed)

			 pmulhw    mm3, mm1   //
			 psllw	   mm5, 1     // shift left: to get into proper alignment for adding mm4

			 psllw	   mm3, 1     // shift left: to get into proper alignment for adding mm6

			 paddw     mm5, mm4   // I = X(B-A) + A; -> COULD be a signed number... ?!
			 paddw     mm3, mm6   // J = X(D-C) + C; -> COULD be a signed number... ?!

			 psubsw     mm3, mm5   // J - I

			 pmulhw    mm3, mm2   //  Y (J-I) // U pipe only 
			 psrlw     mm5, 1      //  pshift right: proper alignment for adding to mm3...
 			 paddw     mm3, mm5   //  Y (J-I) + I     //#debug latency....

			///////////////////////// FOG 14-bit in MM3 // LIGHT 14-bit in MM7
			// (1-Fog)*Light*MMXFlashCompress => Photon LIGHT
			// Fog*MMXFlashCompress  + MMXFlashOffset => Photon FOG

			 movq      mm4, [MMX14Bits]       // 

			 psubsw    mm4, mm3               // 1-fog = fogcompress &&&&&&&

			 pmulhw    mm7, MMXFlashCompress  // FlashCompress: 15-bit doublebright 15 bits, but "1" actually 14...
			 pmulhw    mm7, mm4               // light * flashcompress * (1-fogmap)... 14*14*14 = 10 bits left.
			 psllw     mm7,4                  // make 14 bits again.(actually full 15 bit)

			//////////////////// 
			 movq      [ebp], mm7			  // Store Light - 14-bit scale.
			 add       ebp,8

			 ret
   		/////////////////////////////////////////////////////////////////

		DOALIGN16
		///////////////////
		NegativeStart:	 // ESI has X < 0, if -3 end of line, if -4 end of poly.
		//add		eax, 8   // skip tag
		EndOfLine: 
		//mov		[PhotonSetupWalker],eax // Unneeded... Texture mapper will advance the SetupWalker ptr.
		mov		ebp, [SavedEBP]
	}
	unguardSlow;
}


#endif // all MMX code.

static inline void MakeQuadPalette(DWORD* QuadPalette, FRainbowPtr OrigPal)
{
	//
	// create a quad-palette, with 6-byte colors....
	//
	for( int i=0; i<NUM_PAL_COLORS; i++ )
	{
		QuadPalette[i] =
		 	(((DWORD)OrigPal.PtrBYTE[i*4+0] >> 2) <<  0)  //   2  0
		+	(((DWORD)OrigPal.PtrBYTE[i*4+1] >> 2) <<  8)  //   2  8     
		+	(((DWORD)OrigPal.PtrBYTE[i*4+2] >> 2) << 16); //   2 16
	}
}

/*-----------------------------------------------------------------------------
	World polygon drawing.
-----------------------------------------------------------------------------*/

static inline FMipSetup* CalcMip( FLOAT Z )
{
	static DWORD Temp;
	*(FLOAT*)&Temp = Z;
	return MipTbl[Temp>>23];
}


inline UBOOL DifferenceIsSmall(const FLOAT P1, const FLOAT P2 )
{ 
	FLOAT Temp = P1 - P2;

	// Hex constants for 32-bit floats:
	//			-   return ( ( (*(DWORD*)&Temp) & ~0x80000000 ) <= 0x38d1b717  ); //0.0001
	//			-
	//			-
	return ( ( (*(DWORD*)&Temp) & ~0x80000000 ) <= 0x38d1b717  ); //0.0001
}




inline void USoftwareRenderDevice::RenderSurfSpansMMX32(FRainbowPtr& DestPtr, FSurfaceInfo& Surface,INT TaskStartY, INT TaskEndY)
{
	guardSlow(USoftwareRenderDevice::RenderSurfSpansMMX32);

	if (!(Surface.LightMap)) //All unlit cases  todo: also unlit fogged cases...
	{
		if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
			guardSlow(UnlitStippled32);
				MMXUnlitLight8();
				MMX32StippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			unguardSlow;
			}
		}
		if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX32TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX32ModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}						
		}
		else if (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX32MaskedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWNORMAL)// normal Unlit surface
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
			guardSlow(UnlitNormal32);
				MMXUnlitLight8();
				if (YR & 1) MMX32Render8A(); else MMX32Render8B(); 
				DestPtr.PtrBYTE += GByteStride;			
			unguardSlow;
			}
		}
	}
	else if (Surface.FogMap) // All fog cases 
	{
		if (RenderMode == DRAWNORMAL)// normal fog surface
		{
			if (TextureSmooth)
			{
				for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
				{
					MMXFogLight8();
					if (YR & 1) MMX32FogRender8A(); else MMX32FogRender8B(); //MMX32FogRender8(); 
					DestPtr.PtrBYTE += GByteStride;
				}
			}
			else
			{
				for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
				{
					MMXFogLight8();
					//MMX32Render8RawNewFog(); //&&&&&&&&&&&&&&&&&&&&
					MMX32FogRender8();  //MMX32Render8RawNewFog();  //MMX32FogRender8();
					DestPtr.PtrBYTE += GByteStride;
				}
			}
		}
		else if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXScaledFogLight8();
				MMX32TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				MMX32FogStippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8(); 
				MMX32FogModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}												
		}
		else if (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				MMX32FogMaskedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
	}

	else // Lightmap but no fogmap

	{
		if (RenderMode == DRAWNORMAL) // normal surface
		{
			if (TextureSmooth)
			{
				for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
				{
					MMXLight8();							
					if (YR & 1) MMX32Render8A(); else MMX32Render8B(); 
					DestPtr.PtrBYTE += GByteStride;
				}
			}
			else
			{
				for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
				{
					MMXLight8();							
					MMX32Render8UnrolledFaster(); 
					DestPtr.PtrBYTE += GByteStride;
				}
			}
		}
		else if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX32TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
			guardSlow(LitStippled32);
				MMXLight8();
				MMX32StippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			unguardSlow;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX32ModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}												
		}
		else if (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX32MaskedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}		
	}
	unguardSlow;
}


inline void USoftwareRenderDevice::RenderSurfSpansMMX15(FRainbowPtr& DestPtr, FSurfaceInfo& Surface,INT TaskStartY, INT TaskEndY)
{
	guardSlow(USoftwareRenderDevice::RenderSurfSpansMMX15);

	if (!(Surface.LightMap)) //All unlit cases  todo: also unlit fogged cases...
	{
		if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX15StippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX15TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX15ModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}						
		}
		else if (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				if (YR & 1) MMX15MaskedRender8A(); else MMX15MaskedRender8B(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWNORMAL)// normal Unlit surface
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				if (YR & 1) MMX15Render8A();  else MMX15Render8B(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
	}
	else
	if (Surface.FogMap) // All fog cases 
	{
		if (RenderMode == DRAWNORMAL)// normal fog surface
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				if (YR & 1) MMX15FogRender8CoolA();  else MMX15FogRender8CoolB(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXScaledFogLight8();
				MMX15TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				MMX15FogStippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8(); 
				MMX15FogModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}												
		}
		else if  (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				if (YR & 1) MMX15FogMaskedRender8A(); else MMX15FogMaskedRender8B();
				DestPtr.PtrBYTE += GByteStride;
			}
		}
	}
	else // Lightmap but no fogmap
	{
		if (RenderMode == DRAWNORMAL) // normal surface
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();							
				if (YR & 1) MMX15Render8A(); else MMX15Render8B(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX15TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX15StippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX15ModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}												
		}
		else if (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				if (YR & 1) MMX15MaskedRender8A();  else MMX15MaskedRender8B();
				DestPtr.PtrBYTE += GByteStride;
			}
		}
	}
	unguardSlow;
}

inline void USoftwareRenderDevice::RenderSurfSpansMMX16(FRainbowPtr& DestPtr, FSurfaceInfo& Surface,INT TaskStartY, INT TaskEndY)
{
	guardSlow(USoftwareRenderDevice::RenderSurfSpansMMX16);

	if (!(Surface.LightMap)) //All unlit cases  todo: also unlit fogged cases...
	{
		if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX16StippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX16TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				MMX16ModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}						
		}
		else if (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				if (YR & 1) MMX16MaskedRender8A(); else MMX16MaskedRender8B(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWNORMAL)// normal Unlit surface
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXUnlitLight8();
				if (YR & 1) MMX16Render8A();  else MMX16Render8B(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
	}
	else
	if (Surface.FogMap) // All fog cases 
	{
		if (RenderMode == DRAWNORMAL)// normal fog surface
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				if (YR & 1) MMX16FogRender8CoolA();  else MMX16FogRender8CoolB(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXScaledFogLight8();
				MMX16TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				MMX16FogStippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8(); 
				MMX16FogModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}												
		}
		else if  (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXFogLight8();
				if (YR & 1) MMX16FogMaskedRender8A(); else MMX16FogMaskedRender8B();
				DestPtr.PtrBYTE += GByteStride;
			}
		}		
	}
	else // Lightmap but no fogmap
	{
		if (RenderMode == DRAWNORMAL) // normal surface
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();							
				if (YR & 1) MMX16Render8A(); else MMX16Render8B(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWTRANSLUCENT)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX16TranslucentRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWSTIPPLED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX16StippledRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}
		}
		else if (RenderMode == DRAWMODULATED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				MMX16ModulatedRender8(); 
				DestPtr.PtrBYTE += GByteStride;
			}												
		}
		else if (RenderMode == DRAWMASKED)
		{
			for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
			{
				MMXLight8();
				if (YR & 1) MMX16MaskedRender8A();  else MMX16MaskedRender8B();
				DestPtr.PtrBYTE += GByteStride;
			}
		}
	}
	unguardSlow;
}


void USoftwareRenderDevice::DrawComplexSurface( FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet )
//void USoftwareRenderDevice::DrawPolyV( FSceneFrame* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet )
{
	guardSlow(USoftwareRenderDevice::DrawComplexSurface);
	FMemMark Mark(GMem);


	void (*MergePass)( INT Y, INT X, INT InnerX ) = NULL;

	//#debug simplify flag-detection!

	INT IsModulated   = (Surface.PolyFlags & PF_Modulated);
	INT IsTranslucent = (Surface.PolyFlags & PF_Translucent);
	INT IsMasked      = (Surface.PolyFlags & PF_Masked);
	INT IsStippled    = (IsTranslucent) && (FastTranslucency);

	INT RefreshSetupFlag = 0;

	// Warning: order is important; these options aren't simply mutually exclusive !

	
	RenderMode = DRAWNORMAL;
	if (IsMasked)		RenderMode = DRAWMASKED;
	if (IsModulated)	RenderMode = DRAWMODULATED;
	if (IsTranslucent)	RenderMode = DRAWTRANSLUCENT;
	if (IsStippled)		RenderMode = DRAWSTIPPLED;
	


	if( Surface.PolyFlags & PF_FlatShaded )
	{
		// Draw flatshaded.
		FRainbowPtr Line = Frame->Screen(0, Facet.Span->StartY);
		if( Viewport->ColorBytes==2 )
		{
			_WORD HiColor = (Viewport->Caps & CC_RGB565) ? Surface.FlatColor.HiColor565() : Surface.FlatColor.HiColor555();
			for( int i=Facet.Span->StartY; i<Facet.Span->EndY; i++ )
			{
				for( FSpan* Span = Facet.Span->Index[i-Facet.Span->StartY]; Span; Span=Span->Next )
					for( int j=Span->Start; j<Span->End; j++ )
						Line.PtrWORD[j] = HiColor;
				Line.PtrBYTE += GByteStride;
			}
		}
		else if( Viewport->ColorBytes==4 )
		{
			DWORD TrueColor = Surface.FlatColor.TrueColor();
			for( int i=Facet.Span->StartY; i<Facet.Span->EndY; i++ )
			{
				for( FSpan* Span = Facet.Span->Index[i-Facet.Span->StartY]; Span; Span=Span->Next )
					for( int j=Span->Start; j<Span->End; j++ )
						Line.PtrDWORD[j] = TrueColor;
				Line.PtrBYTE += GByteStride;
			}
		}
	}
	else
	{	
		//
		// Draw texture mapped surface.
		//

		static int SavedColorBytes;
		static FLOAT PError, PError05, PErrorSqr;
		static int MaxSub;

		// Check only ONCE each new frame, for framesize/depth specific setup tasks.
		static INT OldFrameLocks = -1;
		if (FrameLocksCounter != OldFrameLocks) 
		{
			OldFrameLocks = FrameLocksCounter;
			static FLOAT	SavedBrightness=-1.0;
			static FVector	SavedScale, SavedFog;
			static DWORD	SavedCaps,SavedRenderMode;

			TextureSmooth = ( (LowResTextureSmooth && (Frame->X<=400)) ||  (HighResTextureSmooth && (Frame->X>400)) );

			if
			(	Viewport->Client->Brightness != SavedBrightness
			||	Viewport->ColorBytes	     != SavedColorBytes
			||	SavedScale				     != FlashScale		// Float compares....
			||	SavedFog				     != FlashFog
			||	(Viewport->Caps&CC_RGB565)   != (SavedCaps&CC_RGB565) 
			||  SavedRenderMode              != RenderMode )
			{
				// Create pre-scaled error constants for faster per-span Subdiv size calculation.
				RefreshSetupFlag = 1;
				PerspError = PERSPECTIVEACCURACY * 400.f / (FLOAT)Frame->X;  // accuracy 1.5 is acceptable ?
				PError     = PerspError;
				PError05   = PerspError * 0.5;
				PErrorSqr  = PerspError * PerspError * 0.25;

				// Set maximum perspective subdivision size. MUST BE 8-aligned !!!!
				MaxSub = (Min( Frame->X >> 1, MAXPERSPSUBDIVSIZE) & ~7); 

				if (GIsMMX)
				{
					InitMMXFlashes ( Viewport->Client->Brightness, Viewport->ColorBytes, Viewport->Caps);
				}
				else
				{
					InitColorTables( Viewport->Client->Brightness, Viewport->ColorBytes, Viewport->Caps);
				}

				SavedFog		= FlashFog;
				SavedScale		= FlashScale;
				SavedColorBytes = Viewport->ColorBytes;
				SavedBrightness = Viewport->Client->Brightness;
				SavedCaps       = Viewport->Caps;
				SavedRenderMode = RenderMode;
			}
		}

		FLOAT    DivSub=0;

		// Palette SetupWalker.
		// Inefficient: copies 1K of data per unique palette rendered.

		FCacheItem* Item;
		if ( GIsMMX ) 
		{
			QWORD CacheID = MakeCacheID((ECacheIDBase)CID_SurfMMXPalette,Surface.Texture->PaletteCacheID);
			MMXColors = (FMMX*)GCache.Get( CacheID, Item );
			if( !MMXColors )
			{
				MMXColors = (FMMX*)GCache.Create( CacheID, Item, NUM_PAL_COLORS * sizeof(FMMX) );
				for( int i=0; i<NUM_PAL_COLORS; i++ )
				{   
					// Promote 8-bit palette to MMX packed signed words 15:15:15:15 format.
					MMXColors[i].R = (INT)Surface.Texture->Palette[i].B << 7;
					MMXColors[i].G = (INT)Surface.Texture->Palette[i].G << 7;
					MMXColors[i].B = (INT)Surface.Texture->Palette[i].R << 7;
					// MMXColors[i].A = 0;
				}
			}
		}
		else // No MMX selected
		{

			QWORD CacheID = MakeCacheID((ECacheIDBase)CID_SurfPalette,Surface.Texture->PaletteCacheID); //!! TriPalette+66..

			if (IsTranslucent)
			{								
				if	( Viewport->ColorBytes==2 )	
				{
					if (IsStippled)
					{
						MergePass = MergePass1516Stippled;
					}
					else
					{
						if (Viewport->Caps&CC_RGB565) MergePass = MergePass16Translucent;
						          				else  MergePass = MergePass15Translucent;
					}
				}
				else if	( Viewport->ColorBytes==4 )	
				{
					if (IsStippled) MergePass = MergePass32Stippled;
					else			MergePass = MergePass32Translucent;
				}
			}
			else if (IsMasked)
			{				
				if	( Viewport->ColorBytes==2 )	MergePass = MergePass1516Masked;
				else if	( Viewport->ColorBytes==4 )	MergePass = MergePass32Masked;
			}
			else if (IsModulated) 
			{
				if	( Viewport->ColorBytes==2 )
				{
					if (Surface.LightMap)
					{
						if (Viewport->Caps&CC_RGB565) MergePass = MergePass16Modulated;
											     else MergePass = MergePass15Modulated;
					}
					else
					{
							if (Viewport->Caps&CC_RGB565) MergePass = MergePass16ModulatedUnlit;
													 else MergePass = MergePass15ModulatedUnlit;
					}
				}
				else if	( Viewport->ColorBytes==4 )	
				{
					if (Surface.LightMap) MergePass = MergePass32Modulated;
					                 else MergePass = MergePass32ModulatedUnLit;
				}
			}
			else  // Normal..
			{
				if	( Viewport->ColorBytes==2 )	MergePass = MergePass1516;
				else if	( Viewport->ColorBytes==4 )	MergePass = MergePass32;
			}
				
			DWORD* ColorsPtr = (DWORD*)GCache.Get( CacheID, Item );

			if( !ColorsPtr )
			{
				ColorsPtr = (DWORD*)GCache.Create( CacheID, Item, NUM_PAL_COLORS * sizeof(DWORD) );
				MakeQuadPalette( ColorsPtr, Surface.Texture->Palette );  // Palette setup core routine.
			}

			static QWORD LastPaletteCacheID=0;
			if ( CacheID != LastPaletteCacheID )
			{
				LastPaletteCacheID = CacheID;
				for( int i=0; i<NUM_PAL_COLORS; i++ )
					Colors[i] = ColorsPtr[i];				
			}
			else Colors[0] = ColorsPtr[0]; // Ensure no masking indicator for textures that are masked AND nonmasked

			if (IsMasked || IsTranslucent) Colors[0] = 0xFFFFFFFF; // Masking indicator for masked/translucent/modulated.

		}


		//
		// If TEXTURE differs, only update the DATA pointers... if texture and SIZE differ,
		// update everything.
		//

		static QWORD TextureDataID = 0;
		static DWORD LastTextureID = 0; 
		
		// Create an ID that uniquely specifies certain texture dimensions including number of mipmaps.
		FMipmap* FirstMip = Surface.Texture->Mips[0];
		DWORD ThisTextureID = (FirstMip->UBits) + ((FirstMip->VBits)<<8) + ((Surface.Texture->NumMips)<<16);

		// Number of mips not really an issue ? ie just don't 'mip' when such a texture
		// is present, and assume full (up to degenerate) mipmaps in all other cases.....

		/*
		static DWORD AllMipz = 0;
		static DWORD NewMipz = 0;
		static DWORD DataMipz = 0;
		if (!(AllMipz & 1023)) debugf(NAME_Log," MIPZ  All %i New %i Data %i ",AllMipz,NewMipz,DataMipz); 
		*/

		if ( (ThisTextureID != LastTextureID) || RefreshSetupFlag ) // Same as last setup ?
		{
			LastTextureID = ThisTextureID;
			// Initialize Texture MIPS data setup


			if (GIsMMX)
			{		
				for( int i=0; i<ARRAY_COUNT(Mips); i++ )
				{
					INT iMip            = Min(i,Surface.Texture->NumMips-1);
					FMipmap& Src		= *Surface.Texture->Mips[iMip];
					Mips[i].Data		= Src.DataPtr;
					Mips[i].TexURotate  = 32 - iMip + (FixedBits - 12) - Src.UBits;	// 16:16 -> 12:Vshift:fractional
					Mips[i].TexVRotate  = 32 - iMip + (FixedBits - 12);				// TexU/V rotate the data to the LEFT.
					Mips[i].HybridShift.Q  = 32-Src.UBits; // shift right this amount
					Mips[i].HybridMask.DL  = 0x3FFFFFFF >> (30- (Src.UBits + Src.VBits ));
				}
			}
			else 
			// Pentium nonmmx mip setup
			{
				for( int i=0; i<ARRAY_COUNT(Mips); i++ )
				{					
					INT iMip            = Min(i,Surface.Texture->NumMips-1);
					FMipmap& Src		= *Surface.Texture->Mips[iMip];
					Mips[i].Data		= Src.DataPtr;
					Mips[i].Mask.DH		= Src.VSize-1;
					Mips[i].RMask.DH	= ~Mips[i].Mask.DH;
					Mips[i].Mask.DL	    = (Src.VSize-1) | (0xffff0000 << ( FixedBits-Src.UBits));
					Mips[i].LVShift		= FixedBits - iMip;  
					Mips[i].HUShift		= FixedBits - Src.UBits - iMip;
					Mips[i].Func		= TexturePassFunctions[iMip][Src.UBits];
				}
			}

			TextureDataID = Surface.Texture->CacheID; // mark DATA pointers as being OK too...
		}
		else // Texture size doesn't differ, but still, maybe TEXTURE DATA ptr does, so then update ONLY those...
		{
			if (TextureDataID != Surface.Texture->CacheID)
			{
				TextureDataID = Surface.Texture->CacheID;
				for( int i=0; i<ARRAY_COUNT(Mips); i++ )
				{
					INT iMip            = Min(i,Surface.Texture->NumMips-1);
					FMipmap& Src		= *Surface.Texture->Mips[iMip];
					Mips[i].Data		= Src.DataPtr;
				}
			}
		}

		
		FLOAT  InvTexUScale,InvTexVScale;
		FLOAT  UScaleFixedMult,VScaleFixedMult;

		if ((Surface.Texture->UScale != 1.f) || (Surface.Texture->VScale != 1.f))
		{
			// Worth it special casing - usually 2.f and 2.f
			if ((Surface.Texture->UScale == 2.f) && (Surface.Texture->VScale ==2.f))
			// !! This texture scaling usually just 1/2 = 0.5...
			{
				InvTexUScale = 0.5f;
				InvTexVScale = 0.5f;
			}
			else
			{
				InvTexUScale = 1.f/ Surface.Texture->UScale;
				InvTexVScale = 1.f/ Surface.Texture->VScale;
			}
		}
		else
		{
			InvTexUScale = 1.f;
			InvTexVScale = 1.f;
		}
		

		UScaleFixedMult = FixedMult * InvTexUScale;
		VScaleFixedMult = FixedMult * InvTexVScale;	

		//
		// Light SetupWalker.
		//

		static INT LastWasUnlit = 0;

		INT LightUF=0.f, LightVF=0.f;

		// BYTE LightShift=0, LightShift3=0;
		// FOG: dimensions and coordinates are those of the lightmap.
		// There's no fog whenever Surface.FogMap = NULL

		LightMip.FogData.PtrVOID = NULL;
		if( Surface.FogMap)
			LightMip.FogData.PtrVOID = Surface.FogMap->Mips[0]->DataPtr;		
		    
		LightMip.Data.PtrVOID = NULL;
		if( Surface.LightMap )
		{   
			LastWasUnlit = 0;

			LightUF	= UScaleFixedMult * ( Surface.LightMap->Pan.X - Surface.Texture->Pan.X );
			LightVF	= VScaleFixedMult * ( Surface.LightMap->Pan.Y - Surface.Texture->Pan.Y ); 

			FMipmap& SrcMip			= *Surface.LightMap->Mips[0];
			DWORD* LightMap			= (DWORD*)Surface.LightMap->Mips[0]->DataPtr;
			LightMip.Data.PtrVOID	= SrcMip.DataPtr;		

			// Initialize LIGHT Mip setup for MMX
			if (GIsMMX)
			{
				LightMip.Mask.DL		= ((SrcMip.USize)-1);    
				LightMip.Mask.DH		= ((SrcMip.VSize)-1);

				LightMip.UScale 	= Surface.Texture->UScale / ( Surface.LightMap->UScale );  // Divide ONCE per surface - fast enough..
				LightMip.UScale8	= 8.f * LightMip.UScale;	  // the per-8-pixels scaling.

				LightMip.VScale 	= Surface.Texture->VScale / ( Surface.LightMap->VScale );  // Divide ONCE per surface - fast enough..
				LightMip.VScale8	= 8.f * LightMip.VScale;	  // the per-8-pixels scaling.

				LightMip.Mult.Q		= 1 + ( (SrcMip.USize) << FixedBits);
				LightMip.USize      = SrcMip.USize;
			}
			else
			// Initialize LIGHT Mip setup for Pentium
			{

				PrevLitCoordinate.Q      = 0xFFFFFFFFFFFFFFFF;
				LightUBits				= SrcMip.UBits;

				LightMip.Mask.DL		= (SrcMip.VSize-1) | (0xffff0000 << (FixedBits-SrcMip.UBits));
				LightMip.Mask.DH		= (SrcMip.VSize*8)-1;
				LightMip.RMask.DH		= ~LightMip.Mask.DH;

				LightMip.LVShift		= FixedBits + 3;
				LightMip.HUShift		= FixedBits + 3 - SrcMip.UBits;

				LightMip.UScale         = Surface.Texture->UScale / (Surface.LightMap->UScale );  
				LightMip.VScale         = Surface.Texture->VScale / (Surface.LightMap->VScale );  

				LightMip.UScale8		 = (1.f/8.f) * LightMip.UScale;
				LightMip.VScale8		 = (1.f/8.f) * LightMip.VScale;

				LightMip.Func           = LightPassPentium;
				
				// Copy specific lightmap oversampling deltas.
				LightOS[0] = LightOSTable[SrcMip.UBits][0];
				LightOS[1] = LightOSTable[SrcMip.UBits][1];
				LightOS[2] = LightOSTable[SrcMip.UBits][2];
				LightOS[3] = LightOSTable[SrcMip.UBits][3];

				//	Lightmap fudge needed because of the oversampling kernel asymmetries
				//	relative to the MMX code, also taking into account the 0th oversampling-offset.
				//
				//  LightU -= (INT)(DU[0]-2) * (65536/16) << LightShift;  
				//  LightV -= (INT)(DV[0]-2) * (65536/16) << LightShift;  

				// -2 -4 -8 ?? -> adjust for lightmap misalignment....
				LightUF -= (FLOAT)(KernelDU[0]-2) * (UScaleFixedMult/16.f) * Surface.LightMap->UScale;  
				LightVF -= (FLOAT)(KernelDV[0]-2) * (VScaleFixedMult/16.f) * Surface.LightMap->VScale;  

				//	LightUF -= appRound( ((FLOAT)(KernelDU[0]-2)) * 4096.f * Surface.LightMap->UScale );
				//	LightVF -= appRound( ((FLOAT)(KernelDV[0]-2)) * 4096.f * Surface.LightMap->VScale );
				//	LightUF -= (INT) ( ((INT)DU[0])-2.f) * (65536.f/16.f) * Surface.LightMap->UScale;  
				//	LightVF -= (INT) ( ((INT)DV[0])-2.f) * (65536.f/16.f) * Surface.LightMap->VScale;  
			}
		}
		else
		{
			// 
			if ( !LastWasUnlit || IsModulated || RefreshSetupFlag) 
			{
				LastWasUnlit = 1;

				if (GIsMMX)
				{
					QWORD UnlitQ;

					if (IsModulated) // Modulation needs a fixed  unlit value....
					{
						UnlitQ = 0x3F003f003f003f00;
						LastWasUnlit = 0;
					}
					else
					{
						UnlitQ = UnlitMMXValue.Q;
					}

					for( int X=0; X<Frame->X; X++ )
					{
						Photon[X].Q = UnlitQ;
					}
				}
				else
				{
					for( int X=0; X<Frame->X; X++ )
					{
						Photon[X].DL = UnlitPentiumValue;
					}
					LightMip.Func = NoLightPass;
				}
			}
		}



		// Magic number setup.
		FVector TexBase = FVector(0,0,0).TransformPointBy( Facet.MapCoords ) - Surface.Texture->Pan;
		FCoords Inc = FCoords
		(
			FVector( Frame->Proj.X, Frame->Proj.Y + Facet.Span->StartY, Frame->Proj.Z ),
			Facet.MapCoords.XAxis,
			Facet.MapCoords.YAxis,
			Facet.MapCoords.ZAxis / -TexBase.Z
		).Transpose();

		// Initial minimum subdivision setup (must be a multiple of 8).
		// Sub = Frame->X<400 ? 8 : Frame->X<800 ? 16 : 24; // 8 for <400, 16 for <800, 24 for all over...
		INT MainSub = Frame->X<800 ? 16 : 24; 


		// #debug make faster if it gives noticeable overhead.
		// Mipmap criterium factor setup. 

		// Todo: re-evaluate !!!!! Also: HOW is this routine aware of 
		// texel size vs pixel size ?!?!?

		// Viewport->Client->MipFactor now member var DetailBias !

		// Todo: this one only reacts to Surface.Texture->UScale; !
		FLOAT MipMult = 0.75f * 80.f * (1.0f + DetailBias) * FastSqrt (
		  (1.f + Square( 120.f / (Facet.MapCoords.Origin | Facet.MapCoords.ZAxis)))
		 *(Facet.MapCoords.XAxis.SizeSquared() + Facet.MapCoords.YAxis.SizeSquared())
		) * Viewport->Actor->FovAngle * (FLOAT)(1.f/90.f) * InvTexUScale;    

		// Push mipmaps further away when in low resolutions...
		if( Frame->X<=400 ) MipMult *= 0.67; // initial suggested value 0.6....

		// Old mipmap calculation 
		/*
		FLOAT MipMult = 0.6 * Viewport->Client->MipFactor * 80.0 * FastSqrt
		(	(1 + Square( 120.0 / (Facet.MapCoords.Origin | Facet.MapCoords.ZAxis)))
		*	(Facet.MapCoords.XAxis.SizeSquared() + Facet.MapCoords.YAxis.SizeSquared())
		) * Viewport->Actor->FovAngle / 90.0;

		if( Frame->X<=400 ) MipMult *= 0.6;
		*/

		//
		// Initialize bilinear-ish texel fuzzing: Only for first mip;
		// 

		static INT Bilinearset = -1;

		if ( TextureSmooth && (RenderMode == DRAWNORMAL) && (!(Surface.PolyFlags & PF_NoSmooth)))
		{
			InitBilinearKernel( Surface.Texture->Mips[0]->UBits );
			Bilinearset = 1;
		}
		else
		if (Bilinearset == 1)
		{
			EraseBilinearKernel();
			Bilinearset = 0;
		}

		///FLOAT FixedMult = FixedMult / Surface.Texture->UScale; // Account for scaling textures.

		// Fix TexBase.
		TexBase.X *= UScaleFixedMult;
		TexBase.Y *= VScaleFixedMult;

		//
		// Setup all spans.
		// Only huge polys need multiple setup/render cycles.
		// Setup and Rendering: from Draw->Span.StartY up to Draw->Span.EndY
		//
		
		INT Y,TaskStartY,TaskEndY;
		INT Sub,CoherentSubdiv,SharedSubdivs;
		SharedSubdivs = 0;
		CoherentSubdiv = 0;

		Y = Facet.Span->StartY;

		while( Y < Facet.Span->EndY) // Multiple only for _really_ big surfaces.
		{
			TaskStartY = Y;
			SetupWalker.PtrBYTE = &TexSetupHeap[0];

			// 2 versions for setting up single line - regular Pentium and MMX.
		
			if (GIsMMX)
			{
				do // setup multiple lines
				{
					FSpan* Span = Facet.Span->Index[Y-Facet.Span->StartY];
					
					// Stippling checkerboard alignment support.
					if (IsStippled && Span)
					{					
						UBOOL DoProcess = false;
						do
						{
							DoProcess = false;
							if ((Span->Start ^ Y) & 1)
							{
								if ( (Span->Start+1) < Span->End)									
								{
									Span->Start++;
								}
								else
								{									
									Span = Span->Next;
									if (Span) DoProcess = true;
								}
							}							
						}while (DoProcess);				
					}

								
					if (! Span)
					{

						*SetupWalker.PtrINT = -3; // End of line.
						SetupWalker.PtrINT +=  2; //
						SetupWalker.PtrINT +=  2; // fill out... 
					}
					else 
					do  // proceed thru spans
					{

						// Start coords SetupWalker.
						INT EndX     = Span->End;
						INT X		 = Span->Start;

						INT XSize    = EndX-X;
						NUMSIZE XF1  = X;

						#if DO_SLOW_GUARD
						if (XSize == 0) appErrorf("0-size span detected."); //#debug
						#endif

						NUMSIZE TexZ  = Inc.XAxis.Z * XF1 - Inc.Origin.Z;
						NUMSIZE TexX  = Inc.XAxis.X * XF1 - Inc.Origin.X;
						NUMSIZE TexY  = Inc.XAxis.Y * XF1 - Inc.Origin.Y;
						
						NUMSIZE FZ   = 1.0/TexZ;
						NUMSIZE FX   = FZ *TexX;
						NUMSIZE FY   = FZ *TexY;
						
						// Starting texture coordinates
						FLOAT UF = FX * UScaleFixedMult + TexBase.X;
						FLOAT VF = FY * VScaleFixedMult + TexBase.Y;

						INT U = appRound(UF);
						INT V = appRound(VF);

						// End coords SetupWalker.
						NUMSIZE XF2   = EndX;
						FLOAT   FNX   = XF2-1.0;
						NUMSIZE TexZ2 = Inc.XAxis.Z * FNX - Inc.Origin.Z;

						NUMSIZE HZ    = 1.0/TexZ2;
						NUMSIZE HX    = HZ *(Inc.XAxis.X * FNX - Inc.Origin.X);
						NUMSIZE HY    = HZ *(Inc.XAxis.Y * FNX - Inc.Origin.Y);

						FLOAT FXSize = (FLOAT)(XF2 - XF1);			

						// Start first setup structure 
						FMipSetup* Mip = CalcMip(MipMult*FZ);
						SetupWalker.PtrFTexSetup->CoSetup = Mip;  // convey MIP index here..
						SetupWalker.PtrFTexSetup->InitTexMMX( X, Mip, U, V );
											
						if ( Surface.LightMap ) 
							SetupWalker.PtrFTexSetup->InitLightMMX( (UF - LightUF), (VF - LightVF) );  // LIGHTPANNING
						SetupWalker.PtrFTexSetup++;

						//
						// Per-span Subdivision criterium:
						// if( (X+Sub+4) < EndX ) // Size of span less than current Subdiv size ?
						//
						// SubdivCoherence counter maintains sharing of determined Subdivision
						// over a number of lines. Usually worth it.
						//
						// A proper Subdivision CoherentSubdiv has been computed if SharedSubdivs > 0,
						// then sharedSubdiv just counts down. (fixed size ???)
						//

						if (SharedSubdivs > 0)
						{
							Sub = CoherentSubdiv;
							SharedSubdivs--;							
						}
						else
						{													
							Sub = MainSub;
							if ( Sub < XSize ) // Size of span less than current Subdiv size ?						
							{  
								FLOAT dXdZ;							

								// #debug checks for div-by-0 anywhere ?
								if ( DifferenceIsSmall(TexZ,TexZ2) ) // zero Z range  (tolerance..) &&&&&&
								// if ( fabs(TexZ-TexZ2) < 0.001 )
								{
									Sub = MaxSub; // just do as much as possible.
								}
								else
								{								
									// dZ/dX, using a 'normalized'  Z-range scaled by  1/ClosestZ 
									if (TexZ>TexZ2)
									{
										dXdZ =  (FXSize * TexZ2) / (TexZ - TexZ2); //dXdZ =  (FXSize ) / (TexZ - TexZ2);
									}
									else
									{
										dXdZ =  (FXSize * TexZ) / (TexZ2 - TexZ);  //dXdZ =  (FXSize ) / (TexZ2 - TexZ);
									}
									//default:  FSub = 0.5* ( FastSqrt( PerspError * (PerspError + 4*dXdZ)) + PerspError );

									//#define FUDGE 1.f  //&&&&&&&&&&&&&&

									//#debug optimize constants...							
									FLOAT FSub =  FastSqrt( PErrorSqr + PError*dXdZ) + PError05;
									Sub    =  appRound( FSub ) & ~(DWORD)7;

									if (Sub < 16) Sub = 16;
									else
									{
										/*
								 		if (Sub > MaxSub) Sub = MaxSub; 
										else
										{
											SharedSubdivs = 16; // Reserve this Subdivision for the next 8/16 or so.
											CoherentSubdiv = Sub;	
										}
										*/
										if (Sub > MaxSub) 
										{
											Sub = MaxSub;
										}
										else
										{
											SharedSubdivs = 16; // Reserve this Subdivision for the next 8/16 or so.&&&
											// Reserve it for a proportional sized-area !!
											CoherentSubdiv = Sub;	
										}
									}
								}
							}	
						}


						FLOAT DivEnd;
						// Process interior Subdivided spans.
						// while( (X+Sub+4) < EndX ) 
						if ( Sub < XSize ) // ANY Subdivision to be done ?
						{
							DivSub  =  DivTable[Sub];
							
							FVector TexInc  =  Inc.XAxis * Sub;  //! * (XF2-XF1-1.4)/(XF2-XF1); //#debug small-span fudge needed ?

							do
							{						
								NUMSIZE GZ =  1.f /(TexZ+=TexInc.Z);
								NUMSIZE GX =  GZ * (TexX+=TexInc.X);
								NUMSIZE GY =  GZ * (TexY+=TexInc.Y);

								FLOAT DUF  = ( GX - FX ) * DivSub * InvTexUScale;
								FLOAT DVF  = ( GY - FY ) * DivSub * InvTexVScale;
								INT DU	   = appRound(DUF);		// 16:16 texture format -> convert to !!!
								INT DV	   = appRound(DVF);

								FMipSetup* NewMip = CalcMip(MipMult*GZ);
								
								/*
								static inline FMipSetup* CalcMip( FLOAT Z )
								{
									static DWORD Temp;
									*(FLOAT*)&Temp = Z;
									return MipTbl[Temp>>23];
								}
								*/

								SetupWalker.PtrFTexSetup->InitTexMMX( Sub, Mip, DU, DV );

								if( Surface.LightMap ) 
									SetupWalker.PtrFTexSetup->InitLightMMXDelta( DUF, DVF );

								SetupWalker.PtrFTexSetup++;

								FX  = GX;
								FY  = GY;
								X  += Sub;

								if ( NewMip != Mip ) 
								{
									*SetupWalker.PtrINT = -1; 
									SetupWalker.PtrINT +=  2; //advance by 2 dwords.

									Mip = NewMip;
									FLOAT UF = FX*UScaleFixedMult + TexBase.X;
									FLOAT VF = FY*VScaleFixedMult + TexBase.Y;
									INT U = appRound(UF);
									INT V = appRound(VF);

									SetupWalker.PtrFTexSetup->CoSetup = Mip;
									SetupWalker.PtrFTexSetup->InitTexMMX( X, Mip, U, V );
									if( Surface.LightMap )
										SetupWalker.PtrFTexSetup->InitLightMMX( (UF - LightUF), (VF - LightVF) );

									SetupWalker.PtrFTexSetup++;
								}
							}	while( (X+Sub) < EndX );

							INT XLastSize = EndX-X;
							DivEnd = DivTable[XLastSize];
						}
						else // no Subdivision needed, set up for entire span.
						{
							DivEnd = DivTable[XSize];
						}

						// check( X < EndX ); //#debug 

						// Process last Subdivided span.
						FLOAT DUF		= (HX - FX) * DivEnd * InvTexUScale;
						FLOAT DVF		= (HY - FY) * DivEnd * InvTexVScale;
						INT DU			= appRound( DUF );
						INT DV			= appRound( DVF );

						SetupWalker.PtrFTexSetup->InitTexMMX( EndX-X, Mip, DU, DV );

						if( Surface.LightMap )
							SetupWalker.PtrFTexSetup->InitLightMMXDelta( DUF, DVF );

						SetupWalker.PtrFTexSetup++;

						*SetupWalker.PtrINT = -2;  // says end of span but more to come
						SetupWalker.PtrINT +=  2;

						// Exact end-of-span lightmap coordinates value == HX HY 
						*SetupWalker.PtrDWORD++ = appRound( (( HX * UScaleFixedMult + TexBase.X ) - LightUF ) * LightMip.UScale );
						*SetupWalker.PtrDWORD++ = appRound( (( HY * VScaleFixedMult + TexBase.Y ) - LightVF ) * LightMip.VScale ); 

						// END value plugged-in. 

						// Proceed thru spans.
       					Span = Span->Next;
						
						// Stipple alignment for next span...
						if ((Span) && (IsStippled))
						{
							UBOOL DoProcess = false;
							do
							{
								DoProcess = false;
								if ((Span->Start ^ Y) & 1)
								{
									if ( (Span->Start+1) < Span->End)
									{
										Span->Start++;
									}
									else
									{									
										Span = Span->Next;
										if (Span) DoProcess = true;
									}
								}							
							}while (DoProcess);
						}
						
					} while (Span);  // end of spans on one line (possibly no spans at all)

					*(SetupWalker.PtrINT - 2 -2 ) = -3;  // X= -3, says end of line

	  				Inc.Origin -= Inc.YAxis;			 // move on-screen coordinates down..
    				Y++;

    			} while ( ( Y < Facet.Span->EndY ) && (  SetupWalker.PtrVOID < &TexSetupHeap[SAFEHEAPLIMIT] ) );

				// *(SetupWalker.PtrINT - 2 -2) = -4; // X= -4, says end of whole polygon setup stuff. ??

				TaskEndY = Y;

				//
				// Render all setup.
				//
	
				SetupWalker.PtrBYTE = &TexSetupHeap[0];
				ScreenDest.PtrBYTE = Frame->Screen(0,TaskStartY);

				//
				// MMX actual drawing:
				//

				if	(ColorMode == MMX32)
				{
					RenderSurfSpansMMX32( ScreenDest,Surface,TaskStartY,TaskEndY );
				}
				else if (ColorMode == MMX15)
				{
					RenderSurfSpansMMX15( ScreenDest,Surface,TaskStartY,TaskEndY );
				}
				else if (ColorMode == MMX16)
				{
					RenderSurfSpansMMX16( ScreenDest,Surface,TaskStartY,TaskEndY );
				}

				// else if (ColorMode == MMXReserved)

				#if ASM
				__asm emms;
				#endif
			}
			
			else 
			//
			// Slightly different setup for Pentium non-MMX.
			//
			{
				do  // setup multiple lines
				{
					FSpan* Span = Facet.Span->Index[Y-Facet.Span->StartY];
					while (Span)
					{
						// Start coords SetupWalker.
						INT EndX     = Span->End;
						INT X		 = Span->Start;

						INT XSize    = EndX-X;
						NUMSIZE XF1  = X;

						#if DO_SLOW_GUARD
						if (XSize == 0) appErrorf("0-size span detected."); //#debug
						#endif

						NUMSIZE TexZ  = Inc.XAxis.Z * XF1 - Inc.Origin.Z;
						NUMSIZE TexX  = Inc.XAxis.X * XF1 - Inc.Origin.X;
						NUMSIZE TexY  = Inc.XAxis.Y * XF1 - Inc.Origin.Y;
						
						NUMSIZE FZ   = 1.0/TexZ;
						NUMSIZE FX   = FZ *TexX;
						NUMSIZE FY   = FZ *TexY;
						
						// Starting texture coordinates
						FLOAT UF = FX*UScaleFixedMult + TexBase.X;
						FLOAT VF = FY*VScaleFixedMult + TexBase.Y;

						INT U = appRound(UF);
						INT V = appRound(VF);

						// End coords SetupWalker.
						NUMSIZE XF2   = EndX;
						FLOAT   FNX   = XF2-1.0;
						NUMSIZE TexZ2 = Inc.XAxis.Z * FNX - Inc.Origin.Z;

						NUMSIZE HZ    = 1.0/TexZ2;
						NUMSIZE HX    = HZ *(Inc.XAxis.X * FNX - Inc.Origin.X);
						NUMSIZE HY    = HZ *(Inc.XAxis.Y * FNX - Inc.Origin.Y);

						FLOAT FXSize = (FLOAT)(XF2 - XF1);			

						// Start first setup structure 
						FMipSetup* Mip = CalcMip(MipMult*FZ);
						*SetupWalker.PtrFTexturePassFunction++ = Mip->Func;
						SetupWalker.PtrFTexSetup->InitTexPentium( X, Mip, U, V );
											
						if( Surface.LightMap ) 
							SetupWalker.PtrFTexSetup->InitLightPentium( appRound((UF - LightUF)*LightMip.UScale8), appRound((VF - LightVF)*LightMip.VScale8) ); 
						SetupWalker.PtrFTexSetup++;

						//
						// Per-span Subdivision criterium:
						// if( (X+Sub+4) < EndX ) // Size of span less than current Subdiv size ?
						//
						// SubdivCoherence counter maintains sharing of determined Subdivision
						// over a number of lines. Usually worth it.
						//
						// A proper Subdivision CoherentSubdiv has been computed if SharedSubdivs > 0,
						// then sharedSubdiv just counts down. (fixed size ???)
						//

						if (SharedSubdivs > 0)
						{
							Sub = CoherentSubdiv;
							SharedSubdivs--;							
						}
						else
						{													
							Sub = MainSub;
							if ( Sub < XSize ) // Size of span less than current Subdiv size ?						
							{  
								FLOAT dXdZ;							

								// #debug checks for div-by-0 anywhere ?
								if ( DifferenceIsSmall(TexZ,TexZ2) ) // zero Z range  (tolerance..) &&&&&
								// if ( fabs(TexZ-TexZ2) < 0.001 )
								{
									Sub = MaxSub; // just do as much as possible.
								}
								else
								{								
									// dZ/dX, using a 'normalized'  Z-range scaled by  1/ClosestZ 
									if (TexZ>TexZ2)
									{
										dXdZ =  (FXSize * TexZ2) / (TexZ - TexZ2); //dXdZ =  (FXSize ) / (TexZ - TexZ2);
									}
									else
									{
										dXdZ =  (FXSize * TexZ) / (TexZ2 - TexZ);  //dXdZ =  (FXSize ) / (TexZ2 - TexZ);
									}
									//default:  FSub = 0.5* ( FastSqrt( PerspError * (PerspError + 4*dXdZ)) + PerspError );

									//#debug optimize constants...							
									FLOAT FSub =  FastSqrt( PErrorSqr + PError*dXdZ) + PError05;
									Sub    =  appRound( FSub ) & ~(DWORD)7;

									if (Sub < 16) Sub = 16;
									else
									{
								 		if (Sub > MaxSub) 
										{
											Sub = MaxSub;
										}
										else
										{
											SharedSubdivs = 16; // Reserve this Subdivision for the next 8/16 or so.
											CoherentSubdiv = Sub;	
										}
									}
								}
							}	
						}



						FLOAT DivEnd;
						// Process interior Subdivided spans.
						// while( (X+Sub+4) < EndX ) 
						if ( Sub < XSize ) // ANY Subdivision to be done ?
						{
							DivSub  =  DivTable[Sub];

							FVector TexInc  =  Inc.XAxis * Sub;  //! * (XF2-XF1-1.4)/(XF2-XF1); //#debug small-span fudge needed ?

							do
							{						
								NUMSIZE GZ =  1.f /(TexZ+=TexInc.Z);
								NUMSIZE GX =  GZ * (TexX+=TexInc.X);
								NUMSIZE GY =  GZ * (TexY+=TexInc.Y);

								FLOAT DUF  = ( GX - FX ) * DivSub * InvTexUScale;
								FLOAT DVF  = ( GY - FY ) * DivSub * InvTexVScale;
								INT DU	   = appRound(DUF);		// 16:16 texture format -> convert to !!!
								INT DV	   = appRound(DVF);

								FMipSetup* NewMip = CalcMip(MipMult*GZ);
								
								/*
								static inline FMipSetup* CalcMip( FLOAT Z )
								{
									static DWORD Temp;
									*(FLOAT*)&Temp = Z;
									return MipTbl[Temp>>23];
								}
								*/

								SetupWalker.PtrFTexSetup->InitTexPentium( Sub, Mip, DU, DV );

								if( Surface.LightMap ) 
									SetupWalker.PtrFTexSetup->InitLightPentiumDelta( appRound(DUF * LightMip.UScale) , appRound(DVF * LightMip.VScale) );

								SetupWalker.PtrFTexSetup++;

								FX  = GX;
								FY  = GY;
								X  += Sub;

								if ( NewMip != Mip ) 
								{
									*SetupWalker.PtrINT++ = 0;

									Mip = NewMip;
									FLOAT UF = FX * UScaleFixedMult + TexBase.X;
									FLOAT VF = FY * VScaleFixedMult + TexBase.Y;
									INT U = appRound(UF);
									INT V = appRound(VF);

									*SetupWalker.PtrFTexturePassFunction++ = Mip->Func;
									SetupWalker.PtrFTexSetup->InitTexPentium( X, Mip, U, V );
									if( Surface.LightMap )
										SetupWalker.PtrFTexSetup->InitLightPentium( appRound((UF - LightUF)*LightMip.UScale8), appRound((VF - LightVF)*LightMip.VScale8) );

									SetupWalker.PtrFTexSetup++;
								}
							}	while( (X+Sub) < EndX );

							INT XLastSize = EndX-X;

							DivEnd = DivTable[XLastSize];
						}
						else // no Subdivision needed, set up for entire span.
						{
							DivEnd = DivTable[XSize];
						}

						// check( X < EndX ); //#debug 

						// Process last Subdivided span.
						FLOAT DUF		= (HX - FX) * DivEnd * InvTexUScale;
						FLOAT DVF		= (HY - FY) * DivEnd * InvTexVScale;
						INT DU			= appRound( DUF );
						INT DV			= appRound( DVF );

						SetupWalker.PtrFTexSetup->InitTexPentium( EndX-X, Mip, DU, DV );

						if( Surface.LightMap )
							SetupWalker.PtrFTexSetup->InitLightPentiumDelta( (DUF*LightMip.UScale), (DVF*LightMip.VScale) );

						SetupWalker.PtrFTexSetup++;
						*SetupWalker.PtrINT++ = 0;
						*SetupWalker.PtrFTexturePassFunction++ = NULL;

						// proceed thru spans
       					Span = Span->Next;										
       				}

   					// End of spans on one line (and possibly no spans at all).
    				Inc.Origin -= Inc.YAxis; // Move to next screen line.

    				Y++;
				
				} while ( (Y<Facet.Span->EndY) && ( SetupWalker.PtrVOID < &TexSetupHeap[SAFEHEAPLIMIT] ) );

				TaskEndY = Y;

				//
				// Render all setup.
				//

				SetupWalker.PtrBYTE = &TexSetupHeap[0];
				ScreenDest.PtrBYTE = Frame->Screen(0,TaskStartY);
				
				for( INT YR = TaskStartY;  YR < TaskEndY; YR++ )
				{
					for( FSpan* Span=Facet.Span->Index[YR-Facet.Span->StartY]; Span; Span=Span->Next )
					{
						FTexturePassFunction Func;
						while( (Func=*SetupWalker.PtrFTexturePassFunction++) != NULL )
						{
							LightMip.Func( SetupWalker.PtrFTexSetup );
							SetupWalker.PtrFTexSetup = Func( SetupWalker.PtrFTexSetup );
							SetupWalker.PtrINT++;
						}
						MergePass( YR, Span->Start, Span->End );
					}
					ScreenDest.PtrBYTE += GByteStride;
				}
			}// nonmmx Pentium 
			
		// End of single- or multi-chunk setup loop 
		}	// While (Y<Draw->Span.EndY) 

		// Cleanup.
		Item->Unlock();
	}



	//
	//	Draw selection markings on a surface.
	//

	if( GIsEditor && (Surface.PolyFlags & PF_Selected) )
	{
		FColor SelectColor(0,127,255);
		INT     Y     = (Facet.Span->StartY+1)&~1;
		FSpan** Index = Facet.Span->Index;
		if( Viewport->ColorBytes==2 )
		{
			_WORD HiColor = (Viewport->Caps & CC_RGB565) ? SelectColor.HiColor565() : SelectColor.HiColor555();
			_WORD* Line    = (_WORD*)Viewport->_Screen(0,Y);
			while( Y < Facet.Span->EndY )
			{
				FSpan* Span = *Index;
				while( Span )
				{
					INT XOfs      = (Y & 2) * 2;
					INT X         = (((int)Span->Start + XOfs + 7) & ~7) - XOfs;
					_WORD* Screen = Line+X;
					while( X < Span->End )
					{
						*Screen = HiColor;
						Screen += 8;
						X      += 8;
					}
					Span = Span->Next;
				}
				Line  += Viewport->Stride*2;
				Y     += 2;
				Index += 2;
			}
		}
		else
		{
			DWORD TrueColor = SelectColor.TrueColor();
			DWORD* Line = (DWORD*)Viewport->_Screen(0,Y);
			while( Y < Facet.Span->EndY )
			{
				FSpan* Span = *Index;
				while( Span )
				{
					INT    XOfs   = (Y & 2) * 2;
					INT    X      = (((int)Span->Start + XOfs + 7) & ~7) - XOfs;
					DWORD* Screen = Line+X;
					while( X < Span->End )
					{
						*Screen = TrueColor;
						Screen += 8;
						X      += 8;
					}
					Span = Span->Next;
				}
				Line  += Viewport->Stride * 2;
				Y     += 2;
				Index += 2;
			}
		}
	}

	Mark.Pop();
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
