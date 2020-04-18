/*===========================================================================
	DrawPoly.cpp: Actor polygon drawing functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
	    * Rewritten & MMXized by Erik de Neve

		- Notes:
		* All vertex 'Point' usage is now non-destructive.
		* "Unlit" property is handled at a higher level.
		* There's still some bad input detection needed: when PointX->ScreenY is an invalid floating 
		  point number; ->IntY is negative.
		
		- Todo:
		* Pentium nonmmx needs speedups BADLY - at least, create special *fast* unlit-poly drawing support.	
		* Mipmap is kludgy- only checks DU/dx and DV/dx, but DU/dy and DV/dy might be very different..
		* Some kind of BURST-polygon drawer would be cool -> then lots of things can be assumed 
		  instead of checked every poly.
		* Make sure Fog/ Flashscale brightness settings correspond with DrawPolyV.
		* More NonMMX optimization needed ! Interleave and use the alignment-trick-ftoi 
		  stuff.
		* Small triangles that don't really need rasterization and delta's (<2x2 or <3*3
		  make up about 10-20% of all; make special code with less setup overhead, will
		  hugely benefit far-away creatures...
		* Idea => There is a certain texture size below which a triangle really only 
		  displays 1 or at most 4 different texels in a certain mipmap -> when at that 
		  point, try to forego any specific texture mapping setup and span-lighting
		  setup/calculations ?? - unless detecting that would give too much overhead...

		* Mipfactor is nonconfigurable for polys, use member var DetailBias as in DrawSurf ??
			
=============================================================================*/

#include "SoftDrvPrivate.h"
#include <stdlib.h> //for _rotl and _rotr

#include <float.h> // for _finite

#define NUDE 0   // For research purposes only.

#pragma warning( disable : 4799 ) /* no EMMS instruction */


/*------------------------------------------------------------------------------
	Polygon setup structures.
------------------------------------------------------------------------------*/

static FTransTexture* Point0;
static FTransTexture* Point1;
static FTransTexture* Point2;

static struct FTexSetup
{
	FLOAT U, V,  R, G, B, XAdjust;
	DWORD X, EndX;
} Setup[MaximumYScreenSize];//max y res !!


static FMMX* MMXColors;
static int SavedEBP,SavedESP;

static FMMX TriDeltaLight;
static FMMX TriDeltaFog;

static FLOAT* FloatPalBase;


// Global flash fog , nonmmx only.

struct TriVertexType
{
	FVector Fog,Light; 
	INT YFloor;
	INT Reserved;
};

static TriVertexType TriVertex[3];


// Pentium per-span setup structure 

static struct FPolyCSetup
{
	FMMX 		UV, DUV;
	FRainbowPtr Ptr;
	BYTE*       TexBase;
	//FLOAT*		PalBase;
	FLOAT		R, G, B, DR, DG, DB, FlashR, FlashG, FlashB;
	INT         X0, X1;
	DWORD		MaskUV;
	DWORD       UBits;
} GPolyC;



// MMX per-edge setup structure

struct FMMXEdgeSetup 
{
	FMMX		UV,DUV;
	FMMX        LightRGB,DeltaLightRGB;
	FMMX        FogRGB,  DeltaFogRGB;
	INT         StartX,DeltaX;
	INT         StartY,EndY;
	DWORD       SideIndicator;
};


// MMX per-span setup structure - 32 bytes wide.

struct FMMXPolyCSetup
{
	FMMX		UV,LightRGB,FogRGB;
	INT         StartX,EndX;
}MMXSetup[MaximumYScreenSize];


/*------------------------------------------------------------------------------
	Gouraud span rendering.
------------------------------------------------------------------------------*/

static UBOOL GDontMaskThePolygon=0;
static UBOOL GTranslucent=0;
static UBOOL GModulated=0;
static UBOOL GMasked=0; 


static void PentiumPolyC15Normal()
{
	_WORD* Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	_WORD* End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;

	do
	{
		DWORD R,G,B;
		DWORD UV    = ((DWORD)(GPolyC.UV.Q >> 32)) & GPolyC.MaskUV;
		DWORD I     = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

			*(FLOAT*)&R	= FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G = FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B	= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
			*Dest       = (R & 0x7c00) + (G & 0x03e0) + B;
	
		GPolyC.UV.Q += GPolyC.DUV.Q;
		GPolyC.R	+= GPolyC.DR;
		GPolyC.G	+= GPolyC.DG;
		GPolyC.B	+= GPolyC.DB;
	} while( ++Dest < End );
}


static void PentiumPolyC15Translucent()
{
	_WORD* Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	_WORD* End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;
	do
	{
		DWORD R,G,B;
		DWORD UV    = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;
		DWORD I     = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];
		if( I )
		{
			DWORD SampleDest = (*Dest & (0xFFFF - 0x08420 ));
			*(FLOAT*)&R	= FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G = FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B	= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
			DWORD AddColor   =  (R & 0x7800) +  (G & 0x03C0) + B ;
			
			// Slow but correct overflow checking:
			DWORD OvResult = ( SampleDest + AddColor);
			DWORD OvMask = OvResult & 0x08420;
			if (OvMask) 
			{
				OvResult |= (OvMask - (OvMask >> 5)); //saturate
			}
			*Dest = OvResult;
			
			//  Overflow checking of whatever's added done together with overflow 
			//  checking of LIGHT ?? -> might be easy just to have an overflow table???
			//  -> of R*R, G*G etc -> screenflash could be done by updating those tables..
		}

		GPolyC.UV.Q += GPolyC.DUV.Q;
		GPolyC.R	+= GPolyC.DR;
		GPolyC.G	+= GPolyC.DG;
		GPolyC.B	+= GPolyC.DB;

	} while( ++Dest < End );
}


static void PentiumPolyC15Masked()
{
	_WORD* Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	_WORD* End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;

	do
	{
		DWORD R,G,B;
		DWORD UV    = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;
		DWORD I     = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];
		if( I )
		{	
			*(FLOAT*)&R	= FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G = FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B	= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
			*Dest       = (R & 0x7C00) + (G & 0x03e0) + B;
		}

		GPolyC.UV.Q  += GPolyC.DUV.Q;
		GPolyC.R   += GPolyC.DR;
		GPolyC.G   += GPolyC.DG;
		GPolyC.B   += GPolyC.DB;
	} while( ++Dest < End );
}


static void PentiumPolyC15Modulated()
{
	guardSlow(PentiumPolyC15Modulated);

	_WORD *Dest, *End;

	Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;

	do
	{
		DWORD UV = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;			
		DWORD I  = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

		DWORD R = Min( appRound ( (FLOAT)( (*Dest ) & 0x7C00 ) * FloatPalBase[I*4+0] * (1.0f/(1<<(4+10)))  ), 0x7C00);
		DWORD G = Min( appRound ( (FLOAT)( (*Dest ) & 0x03e0 ) * FloatPalBase[I*4+1] * (1.0f/(1<<(4+ 5)))  ), 0x03e0);
		DWORD B = Min( appRound ( (FLOAT)( (*Dest ) & 0x001f ) * FloatPalBase[I*4+2] * (1.0f/(1<<(4+ 0)))  ), 0x001f);
		*Dest = ( (R & 0x7C00) + (G & 0x3E0) + (B & 0x001F) );
		
		GPolyC.UV.Q += GPolyC.DUV.Q;

	} while( ++Dest < End );
	unguardSlow;
}


static void PentiumPolyC16Normal()
{
	guardSlow(PentiumPolyC16Normal);

	_WORD* Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	_WORD* End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;
	do
	{
		DWORD R,G,B;
		DWORD UV     = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;	
		DWORD I      = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

		*(FLOAT*)&R	 = FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
		*(FLOAT*)&G  = FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
		*(FLOAT*)&B	 = FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
		*Dest        = (R&0xF800) + (G&0x07E0) + B;
		
		GPolyC.UV.Q  += GPolyC.DUV.Q;
		GPolyC.R	 += GPolyC.DR;
		GPolyC.G	 += GPolyC.DG;
		GPolyC.B	 += GPolyC.DB;
	} while( ++Dest < End );

	unguardSlow;
}


static void PentiumPolyC16Modulated()
{
	guardSlow(PentiumPolyC16Modulated);

	_WORD *Dest, *End;

	Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;

	do
	{
		DWORD UV = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;			
		DWORD I  = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

		DWORD R = Min( appRound ( (FLOAT)( (*Dest ) & 0xF800 ) * FloatPalBase[I*4+0] * (1.0f/(1<<(4+11)))  ), 0xF800);
		DWORD G = Min( appRound ( (FLOAT)( (*Dest ) & 0x07C0 ) * FloatPalBase[I*4+1] * (1.0f/(1<<(5+ 5)))  ), 0x07C0);
		DWORD B = Min( appRound ( (FLOAT)( (*Dest ) & 0x001F ) * FloatPalBase[I*4+2] * (1.0f/(1<<(4+ 0)))  ), 0x001f);
		*Dest = ( (R & 0xF800) + (G & 0x7E0) + (B & 0x001F) );

		GPolyC.UV.Q += GPolyC.DUV.Q;

	} while( ++Dest < End );
	unguardSlow;
}



static void PentiumPolyC16Translucent()
{
	guardSlow(PentiumPolyC16Translucent);

	_WORD* Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	_WORD* End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;
	do
	{
		DWORD R,G,B;
		DWORD UV    = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;
		DWORD I     = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

		if( I )
		{
			DWORD SampleDest = (*Dest & (0x0001FFFF - 0x010820 ));
			*(FLOAT*)&R	= FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G = FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B	= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
			DWORD AddColor   =  (R & 0xF000) +  (G & 0x07C0) + B ;
			
			// Slow but correct overflow checking:
			DWORD OvResult = ( SampleDest + AddColor);
			DWORD OvMask = OvResult & 0x10820;
			if (OvMask) 
			{
				OvResult |= (OvMask - (OvMask >> 5)); //saturate
			}
			*Dest = OvResult;
			
			//  Overflow checking of whatever's added done together with overflow 
			//  checking of LIGHT ?? -> might be easy just to have an overflow table???
			//  -> of R*R, G*G etc -> screenflash could be done by updating those tables..
		}

		GPolyC.UV.Q += GPolyC.DUV.Q;
		GPolyC.R	+= GPolyC.DR;
		GPolyC.G	+= GPolyC.DG;
		GPolyC.B	+= GPolyC.DB;

	} while( ++Dest < End );

	unguardSlow;
}


static void PentiumPolyC16Masked()
{
	guardSlow(PentiumPolyC16Masked);

	_WORD* Dest = GPolyC.Ptr.PtrWORD + GPolyC.X0;
	_WORD* End  = GPolyC.Ptr.PtrWORD + GPolyC.X1;

	do
	{
		DWORD R,G,B;
		DWORD UV    = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;
		DWORD I     = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];
		if( I )
		{	
			*(FLOAT*)&R	= FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G = FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B	= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
			*Dest       = (R & 0xF800) + (G & 0x07E0) + B;
		}
		GPolyC.UV.Q  += GPolyC.DUV.Q;
		GPolyC.R   += GPolyC.DR;
		GPolyC.G   += GPolyC.DG;
		GPolyC.B   += GPolyC.DB;
	} while( ++Dest < End );

	unguardSlow;
}




static void PentiumPolyC32Normal()
{
	guardSlow(PentiumPolyC32Normal);

	static DWORD R,G,B,*Dest, *End;

	Dest = GPolyC.Ptr.PtrDWORD + GPolyC.X0;
	End  = GPolyC.Ptr.PtrDWORD + GPolyC.X1;


#if NUDE
	 // Nude cheat:

	 //static FLOAT TestTexelR = 180.0f;
	 //static FLOAT TestTexelG = 180.0f;
	 //static FLOAT TestTexelB = 180.0f;

	 static FLOAT TestTexelR = 255.0f; //220,180,150
	 static FLOAT TestTexelG = 255.0f;
	 static FLOAT TestTexelB = 255.0f;

	 //static FLOAT Test0TexelR = 0.0f;
	 //static FLOAT Test0TexelG = 0.0f;
	 //static FLOAT Test0TexelB = 0.0f;

	 static FLOAT Test0TexelR = 255.0f;
	 static FLOAT Test0TexelG = 255.0f;
	 static FLOAT Test0TexelB = 255.0f;

#endif


#if (0) // ASM //#debug
	__asm
	{
		mov [SavedEBP], ebp

		mov ebx, DWORD PTR [GPolyC.UV+0]
		mov esi, DWORD PTR [GPolyC.UV+4]
		
		mov edx, [FloatPalBase]
		mov ebp, [GPolyC.TexBase]

		// load light start gouraud values	
		fld [GPolyC.R] //  st:     R
		fld [GPolyC.G] //  st:   G R
		fld [GPolyC.B] //  st: B G R

		// lead-in
		mov eax, [GPolyC.MaskUV]
		mov ecx, [GPolyC.UBits]
		and eax, esi

		rol eax, cl		
		xor ecx,ecx

		add ebx, DWORD PTR [GPolyC.DUV+0]
		adc esi, DWORD PTR [GPolyC.DUV+4]

		mov		cl, [ebp + eax] // get texel
		mov		edi, [Dest]
		add		ecx,ecx

#if NUDE
		fld DWORD PTR Test0TexelR
		fmul st,st(3)
		fld DWORD PTR Test0TexelG
		fmul st,st(3)
		fld DWORD PTR Test0TexelB
		fmul st,st(3)
#else
		fld DWORD PTR [edx + ecx*8 + 0] // st: RR       B G R
		fmul st,st(3)
		fld DWORD PTR [edx + ecx*8 + 4] // st: GG RR    B G R
		fmul st,st(3)
		fld DWORD PTR [edx + ecx*8 + 8] // st: BB GG RR B G R
		fmul st,st(3)
#endif

		           // st: BB GG RR  B G R
		fxch st(2) // st: RR GG BB  B G R
		fadd dword ptr [GPolyC.FlashR]
		fxch st(1) // st: GG RR BB  B G R
		fadd dword ptr [GPolyC.FlashG]
		fxch st(2) // st: BB RR GG  B G R
		fadd dword ptr [GPolyC.FlashB]
		fxch st(1)

		fstp [R] // st: BB GG B G R
		add edi,4
		fstp [B] // st: GG B G R
		fstp [G] // st: B G R

		cmp edi, [End]
		jge OverTexLoop
		// skip-if-only-one pixel...


	// Inner loop
	TexLoop:
		
		xor  eax,eax	
		mov  al, BYTE PTR [R]
		mov  cl, BYTE PTR [G]
		shl  eax,8
		add  eax,ecx
		mov  cl, BYTE PTR [B]
		shl  eax,8
		add  eax,ecx
		mov  dword ptr [edi-4], eax

		mov eax, [GPolyC.MaskUV]
		mov ecx, [GPolyC.UBits]
		and eax, esi

		rol eax, cl
		xor ecx, ecx
		
		add ebx, DWORD PTR [GPolyC.DUV+0]
		adc esi, DWORD PTR [GPolyC.DUV+4]

		mov cl, [ebp + eax] // Get texel.
		add ecx,ecx
		// Add gouraud light delta.
		// b g r
		fxch st(1)

		fadd DWORD PTR [GPolyC.DG] // g b r
		fxch st(1) 
		fadd DWORD PTR [GPolyC.DB] // b g r
		fxch st(2)
		fadd DWORD PTR [GPolyC.DR] // r g b
		fxch st(2)                  
		// b g r

#if NUDE
		fld DWORD PTR TestTexelR
		fmul st,st(3)
		fld DWORD PTR TestTexelG
		fmul st,st(3)
		fld DWORD PTR TestTexelB
		fmul st,st(3)		
#else
		fld DWORD PTR [edx + ecx*8 + 0] // st: RR         B G R
		fmul st,st(3)
		fld DWORD PTR [edx + ecx*8 + 4] // st: GG RR      B G R
		fmul st,st(3)
		fld DWORD PTR [edx + ecx*8 + 8] // st: BB GG RR   B G R
		fmul st,st(3)
#endif

		           // st: BB GG RR  B G R
		fxch st(2) // st: RR GG BB  B G R
		fadd dword ptr [GPolyC.FlashR]
		fxch st(1) // st: GG RR BB  B G R
		fadd dword ptr [GPolyC.FlashG]
		fxch st(2) // st: BB RR GG  B G R
		fadd dword ptr [GPolyC.FlashB]
		fxch st(1)

		fstp [R] // st:    BB GG B G R
		fstp [B] // st:       GG B G R
		fstp [G] // st:          B G R

		add edi,4
		cmp edi,[End]
		jl  TexLoop

	OverTexLoop:

		///////////////
		// Lead-out.
		///////////////

		xor eax,eax
		
		mov  al, BYTE PTR [R]
		mov  cl, BYTE PTR [G]
		shl  eax,8
		add  eax,ecx
		mov  cl, BYTE PTR [B]
		shl  eax,8
		add  eax,ecx
		mov  dword ptr [edi-4], eax		

		mov ebp, [SavedEBP]
		fcomp
		fcompp

	}
#else
		do
		{
			DWORD UV	= (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;
			DWORD I     = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

			*(FLOAT*)&R			= FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G			= FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B			= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;

			*Dest = ((R&0xff)<<16)  + ((G&0xff)<<8) + (B&0xff);
			
			GPolyC.UV.Q        += GPolyC.DUV.Q;
			GPolyC.R           += GPolyC.DR;
			GPolyC.G           += GPolyC.DG;
			GPolyC.B           += GPolyC.DB;

		} while( ++Dest < End );
#endif

	unguardSlow;
}





static void PentiumPolyC32Masked()
{
	guardSlow(PentiumPolyC32Masked);
	static DWORD *Dest, *End, R, G, B;

	Dest = GPolyC.Ptr.PtrDWORD + GPolyC.X0;
	End  = GPolyC.Ptr.PtrDWORD + GPolyC.X1;

	do
	{
		DWORD UV    = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;
		DWORD I     = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

		if (I)
		{
			*(FLOAT*)&R			= FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G			= FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B			= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
			*Dest = ((R&0xff)<<16) + ((G&0xff)<<8) + (B&0xff);
		}

		GPolyC.UV.Q          += GPolyC.DUV.Q;
		GPolyC.R           += GPolyC.DR;
		GPolyC.G           += GPolyC.DG;
		GPolyC.B           += GPolyC.DB;

	} while( ++Dest < End );
	unguardSlow;
}



static void PentiumPolyC32Translucent()
{
	guardSlow(PentiumPolyC32Translucent);
	DWORD *Dest, *End, R, G, B;

	Dest = GPolyC.Ptr.PtrDWORD + GPolyC.X0;
	End  = GPolyC.Ptr.PtrDWORD + GPolyC.X1;

	do
	{
		DWORD UV = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;			
		DWORD I  = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];
		if (I)
		{
			*(FLOAT*)&R = FloatPalBase[I*4+0] * GPolyC.R + GPolyC.FlashR;
			*(FLOAT*)&G	= FloatPalBase[I*4+1] * GPolyC.G + GPolyC.FlashG;
			*(FLOAT*)&B	= FloatPalBase[I*4+2] * GPolyC.B + GPolyC.FlashB;
			DWORD D = (*Dest & 0x00fefefe) + ( ((R&0xfe)<<16) + ((G&0xfe)<<8) + (B&0xfe) );
			DWORD M = D;
			if( (M = (D & 0x1010100))!=0 ) // Fast overflow detection.
				D |= (M-(M>>7));
			*Dest = D;					
		}

		GPolyC.UV.Q += GPolyC.DUV.Q;
		GPolyC.R  += GPolyC.DR;
		GPolyC.G  += GPolyC.DG;
		GPolyC.B  += GPolyC.DB;

	} while( ++Dest < End );
	unguardSlow;
}



//#debug optimize me ! (and all the other non-mmx fpu modulation/translucency stuff...)
static void PentiumPolyC32Modulated()
{
	guardSlow(PentiumPolyC32Modulated);
	DWORD *Dest, *End;

	Dest = GPolyC.Ptr.PtrDWORD + GPolyC.X0;
	End  = GPolyC.Ptr.PtrDWORD + GPolyC.X1;

	do
	{
		DWORD UV = (GPolyC.UV.Q >> 32) & GPolyC.MaskUV;			
		DWORD I  = GPolyC.TexBase[ _rotl (UV, GPolyC.UBits) ];

		DWORD R = Min(appRound ((FLOAT)( (*Dest ) & 0xff0000 ) * FloatPalBase[I*4+0]* (1.0/128.0)),255<<16);
		DWORD G = Min(appRound ((FLOAT)( (*Dest ) & 0x00ff00 ) * FloatPalBase[I*4+1]* (1.0/128.0)),255<< 8);
		DWORD B = Min(appRound ((FLOAT)( (*Dest ) & 0x0000ff ) * FloatPalBase[I*4+2]* (1.0/128.0)),255<< 0);
		*Dest = ( (R & 0xff0000) + (G & 0xff00) + B);
		
		GPolyC.UV.Q += GPolyC.DUV.Q;

	} while( ++Dest < End );
	unguardSlow;
}



/*--------------------------------------------
  MMX-specific triangle-only render functions
--------------------------------------------*/

 void USoftwareRenderDevice::InnerGouraudMMX32(DWORD PolyFlags, INT MinY, INT MaxY, FSceneNode* Frame,FMipmap* Mip,FSpanBuffer* SpanBuffer)
{

#if ASM

	static  BYTE*	TexBase;
	static  FMMX*	PalBase;
	static  FMMX	MMXCoMask,MMXCoShift; 
	static  FRainbowPtr  Screen;

	Screen.PtrBYTE  = Frame->Screen(0,MinY); 
	TexBase			= Mip->DataPtr;

	MMXCoMask.DL	= 0x3FFFFFFF >> (30- (Mip->UBits + Mip->VBits ));
	MMXCoShift.DL	= 32 - Mip->UBits;  // Shift right this amount. #debug needs full 64 bits cleared ?
	MMXCoShift.DH   = 0;

	FMMXPolyCSetup* Set  = MMXSetup + MinY;
	FSpan** Index   = SpanBuffer->Index + MinY - SpanBuffer->StartY;

	if ( !(PolyFlags & (PF_Masked|PF_Translucent|PF_Modulated)) )
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// Prestepping part.
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						// Prestep the MMX vars by: (Span->Start - SpanX0) guaranteed > 0.
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB
							movq	mm2,[eax]FMMXPolyCSetup.FogRGB					

							PrestepOn1:
								paddd   mm7,mm6
								paddw   mm4,mm5
								paddw   mm2,mm3
								dec		esi
							jnz PrestepOn1

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
							movq	[eax]FMMXPolyCSetup.FogRGB,mm2
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
					__asm{

						// #debug make 1/odd  pixel a special case, all else can be unrolled.
						// Interleaved stuff == coordinate calculation.
						// jmp OldRender 

						mov     ecx,[Set]
						mov     esi,[SpanX0]
						mov     edx,[SpanX1]
						mov     [SavedEBP],ebp
						dec     edx   //#debug


						movq	mm7,[ecx]FMMXPolyCSetup.UV

						mov     ebx,[TexBase]			
						mov     edi,[Screen]			
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
							xor			eax,eax			   //  explicitly, for PII sake.
							mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
							//movq        mm1,mm7          //  new UV
							movq		mm0,mm4            //  Copy lightvalue, should be 15-bits...
							pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							paddw       mm0, mm2           //  Add fogging.
							psraw       mm0,5              //  14-bit result to 9 bits  5???     
							packuswb    mm0,mm0            //  Pack words to bytes.
							mov			ebp,[SavedEBP]     //
							movd		[edi+esi*4], mm0   //  Store to screen. 
						jmp EndRender                      //
						/////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne:	// lead-in partly above

							xor         eax,eax
							movq        mm1,mm7           // new UV

							movq		mm0,mm4           //  Copy lightvalue, should be 15-bits...
							mov         al, byte ptr [ebx+ecx]

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
							paddw       mm0, mm2			//  Add fogging.

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

						// Lead-out, write last pixel:     
						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						paddw       mm0, mm2           //  Add fogging.
						psraw       mm0,5              //  14-bit result to 9 bits  5???     
						packuswb    mm0,mm0            //  Pack words to bytes.
						mov			ebp,[SavedEBP]     //
						movd		[edi+esi*4], mm0   //  Store to screen. 
						EndRender:
					}
				}
			}
		}
	}

	else if (  PolyFlags & PF_Translucent)
	{
	for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB

							PrestepOn3:
								paddd   mm7,mm6
								paddw   mm4,mm5
								dec esi
							jnz PrestepOn3

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
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
						mov     edi,[Screen]			
						mov     ebp,[MMXColors]				

						movq	mm4,[ecx]FMMXPolyCSetup.LightRGB
						movq    mm1,mm7
						movq    mm3,[MMXCoShift]
						psllq   mm1,12  

						mov			eax,edx
						paddd       mm7,mm6
						punpckhdq   mm1,mm1				// Copy high word to low word.

						sub			eax,esi				// eax == number of pixels to draw.
						psrlq       mm1,mm3	//       

						pand        mm1,[MMXCoMask]		//
						test		eax,eax

						movd		ecx,mm1					
						jnz			MoreThanOne3


						// Singular pixel....
							xor			eax,eax			   //  explicitly, for PII sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							//movq        mm1,mm7          //  new UV
							movq		mm0,mm4            //  Copy lightvalue, should be 15-bits...
							test        eax,eax 
							jz          EndRender3         // bail out 

							pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							psraw       mm0,5              //  14-bit result to 9 bits  5???     
							packuswb    mm0,mm0            //  Pack words to bytes.

							movd        mm1,[edi+esi*4]
							paddusb     mm0,mm1
							movd		[edi+esi*4], mm0   //  Store to screen. 
						jmp EndRender3                     //
						/////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne3:	// lead-in partly above

							xor         eax,eax
							movq        mm1,mm7           // new UV

							movq		mm0,mm4           // Copy lightvalue, should be 15-bits...
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender83:	  // Loop using current light & light delta's. Getting the texel coordinate				
							psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
							test        eax,eax

							punpckhdq   mm1, mm1  //
							jz          SkipMaskedPixel3

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							paddd       mm7, mm6 		     //  PMULHW has severe caching penalty.
					
							xor         eax, eax			 //  
							add esi,1						 //  
							
							psrlq       mm1,mm3				 //
							cmp			esi, edx             //  cmp  X, PX;  

							pand        mm1,[MMXCoMask]      //
							psraw       mm0,5                //  13-bit result to 8 bits  

							movd        mm2,[edi+esi*4-4]    //  load screen pixel
							packuswb    mm0,mm0              //  Pack words to bytes.

							movd        ecx,mm1              //
							paddusb     mm0,mm2              //  Actual transparency add.

							paddw		mm4, mm5			 //  + mm5, signed 16-bit R,G,B LightDeltas 
							movq        mm1,mm7              //

							movd		[edi+esi*4-4], mm0   //  Store to screen. 
							movq        mm0,mm4              //

							mov         al,byte ptr [ebx+ecx]//
							jb			ReloopInnerRender83  //


						LeadOut3: // write last pixel:     
						test        eax,eax
						jz          EndRender3 

						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						psraw       mm0,5				//  14-bit result to 9 bits  5???     
						packuswb    mm0,mm0				//  Pack words to bytes.

						movd        mm1,[edi+esi*4]
						paddusb     mm0,mm1
						movd		[edi+esi*4], mm0	//	Store to screen. 
						jmp			EndRender3

						align 16
						SkipMaskedPixel3:
							//punpckhdq   mm1, mm1			//
							paddw		mm4, mm5			// + mm5, signed 16-bit R,G,B LightDeltas 
							add			esi,1				//  inc 		esi                 //  ++X

							psrlq       mm1, mm3			//
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.

							pand        mm1,[MMXCoMask]     //
							movq        mm0,mm4				//

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							cmp			esi, edx				//  cmp  X, PX;
							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender83		//
						jmp LeadOut3

						align 16						
						EndRender3:
						mov			ebp,[SavedEBP]     //
					}
				}
			}
		}
	}
	else if (  PolyFlags & PF_Modulated )
	{
		// Modulation fixed around level 128, no lighting needed.

		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							//movq	mm4,[eax]FMMXPolyCSetup.LightRGB

							PrestepOn4:
								paddd   mm7,mm6
								//paddw   mm4,mm5
								dec esi
							jnz PrestepOn4

							movq	[eax]FMMXPolyCSetup.UV,mm7
							//movq	[eax]FMMXPolyCSetup.LightRGB,mm4
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
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
						mov     edi,[Screen]			
						mov     ebp,[MMXColors]				

						//movq	mm4,[ecx]FMMXPolyCSetup.LightRGB
						movq    mm1,mm7
						movq    mm3,[MMXCoShift]
						psllq   mm1,12  

						mov			eax,edx
						paddd       mm7,mm6
						punpckhdq   mm1,mm1				// Copy high word to low word.

						sub			eax,esi				// eax == number of pixels to draw.
						psrlq       mm1,mm3	//       

						pand        mm1,[MMXCoMask]		//
						test		eax,eax

						movd		ecx,mm1					
						jnz			MoreThanOne4

						// Singular pixel....-> just SKIP ?!?! #debug
							xor			eax,eax			   // explicitly, for PII sake.
							mov         al, byte ptr [ebx+ecx]  //  Get the texel - ebx = Texture map base. 
							//movq		mm0,mm4            //  Copy lightvalue, should be 15-bits...
							test        eax,eax
							movq		mm0, qword ptr [ebp+eax*8] // edx = colors palette base  eax= texelcolor index.
							jz          EndRender4 

								psubw       mm1, mm1
								punpcklbw   mm1, [edi+esi*4] // get screen in 16-bit
								psrlw       mm1,1	 // make 15-bit , mult with palette's '15 bit', 2^14=neutral..
								pmulhw      mm0, mm1 // 
								psrlw       mm0,5  // 
								packuswb    mm0, mm0
								movd		[edi+esi*4], mm0  //  Store to screen. 

						jmp EndRender4                     //
						/////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne4:	// lead-in partly above

							xor         eax, eax
							movq        mm1,mm7           // new UV

							//movq		mm0, mm4           //  Copy lightvalue, should be 15-bits...
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender84:		  // Loop using current light & light delta's. Getting the texel coordinate				
							psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....

							//test        eax,eax
							//jz          SkipMaskedPixel4

							//paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 

							movq		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
					
							add esi,1						//  inc 		esi                 //  ++X
							punpckhdq   mm1, mm1			//

							xor         eax, eax			//  						
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1, mm3	//
							psubw       mm2, mm2

							punpcklbw   mm2, [edi+esi*4-4]  // get screen into 16-bits
							pand        mm1,[MMXCoMask]     //#pairing error

							movd        ecx, mm1				//
							psrlw       mm2,1					// 16-bits unpacked screen data to 15 bits.

							pmulhw      mm0,mm2					// Actual modulation step., 15bits * 14/13 bit value = 11 bit...
							movq        mm1, mm7				//

							mov         al, byte ptr [ebx+ecx]  // Cache misses fall in pmulhw latency...
							psrlw       mm0,5					// adjust 
							packuswb    mm0, mm0				// pack to unsigned bytes

							movd		[edi+esi*4-4], mm0		//  Store to screen. 
							//movq        mm0, mm4				//

							jb			ReloopInnerRender84     //

						//LeadOut4:							
						// Write last pixel
								test        eax,eax
								movq		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
						jz          EndRender4 
								psubw       mm1, mm1
								punpcklbw   mm1, [edi+esi*4]
								psrlw       mm1,1	 // 
								pmulhw      mm0, mm1 //
								psrlw       mm0,5	 //
								packuswb    mm0, mm0
								movd		[edi+esi*4], mm0  //  Store to screen. 

						//jmp		EndRender4

						////////////////////////////////////////

						/*
						align 16
						SkipMaskedPixel4:
							
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,mm3	//

							pand        mm1,[MMXCoMask]     //

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							//movq        mm0,mm4					//
							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender84		//

						jmp LeadOut4
						*/

						EndRender4:
						mov			ebp,[SavedEBP]     //

					}
				}
			}
		}
	}
	else if (  PolyFlags & (PF_Masked))
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB
							movq	mm2,[eax]FMMXPolyCSetup.FogRGB					

							PrestepOn2:
								paddd   mm7,mm6
								paddw   mm4,mm5
								paddw   mm2,mm3
								dec esi
							jnz PrestepOn2

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
							movq	[eax]FMMXPolyCSetup.FogRGB,mm2
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
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
						mov     edi,[Screen]			
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
						jnz			MoreThanOne2

						// Singular pixel....
							xor			eax,eax			   //  explicitly, for PII sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							movq		mm0,mm4            //  Copy lightvalue, should be 15-bits...
							test        eax,eax 
							jz          EndRender2         // bail out 

							pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							paddw       mm0, mm2           //  Add fogging.
							psraw       mm0,5              //  14-bit result to 9 bits  5???     
							packuswb    mm0,mm0            //  Pack words to bytes.
							movd		[edi+esi*4], mm0   //  Store to screen. 
						jmp EndRender2                      //
						/////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne2:	// lead-in partly above

							xor         eax,eax
							movq        mm1,mm7           // new UV

							movq		mm0,mm4           //  Copy lightvalue, should be 15-bits.
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender82:		  // Loop using current light & light delta's. Getting the texel coordinate				
							psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
							test        eax,eax

							paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 
							jz          SkipMaskedPixel2

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							paddd       mm7, mm6 		     //  PMULHW has severe caching penalty.
					
							xor         eax, eax			 //  
							add esi,1						 //  inc 		esi                 //  ++X
							
							punpckhdq   mm1, mm1			 //
							cmp			esi, edx             //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	 //
							paddw       mm0, mm2			 //  Add fogging.

							pand        mm1,[MMXCoMask]      //
							psraw       mm0,5                //  14-bit result to 9 bits  5???     						

							paddw       mm2,mm3              //  fog += fogdelta
							packuswb    mm0,mm0              //  Pack words to bytes.

							movd        ecx,mm1              //
							movq        mm1,mm7              //

							movd		[edi+esi*4-4], mm0   //  Store to screen. 
							movq        mm0,mm4              //

							mov         al,byte ptr [ebx+ecx]//

							jb			ReloopInnerRender82  //


						LeadOut2: // write last pixel:     
						test        eax,eax
						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						jz          EndRender2 
						paddw       mm0, mm2           //  Add fogging.
						psraw       mm0,5              //  14-bit result to 9 bits  5???     
						packuswb    mm0,mm0            //  Pack words to bytes.
						movd		[edi+esi*4], mm0   //  Store to screen. 
						jmp			EndRender2


						align 16
						SkipMaskedPixel2:
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	//

							pand        mm1,[MMXCoMask]     //
							paddw       mm2,mm3             //  fog += fogdelta

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							movq        mm0,mm4					//
							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender82		//

						jmp LeadOut2

						EndRender2:
						mov			ebp,[SavedEBP]     //

					}
				}
			}
		}
	}

	__asm emms;

#endif

} // InnerGouraud32




 void USoftwareRenderDevice::InnerGouraudMMX15(DWORD PolyFlags, INT MinY, INT MaxY, FSceneNode* Frame,FMipmap* Mip,FSpanBuffer* SpanBuffer)
{
	static  BYTE*	TexBase;
	static  FMMX*	PalBase;
	static  FMMX	MMXCoMask,MMXCoShift; 
	static  FRainbowPtr  Screen;

	static  FMMX    MMX15REDBLUE;	//masks out all but the 5MSBits of red and blue
	static  FMMX    MMX15GREEN;		//masks out all but the 5MSBits of green
	static  FMMX    MMX15MULFACT;	//multiply each word by 2**13, 2**3, 2**13, 2**3 and add results

	//#debug just change these globals to suit 15 or 16 bit conversion
	MMX15REDBLUE.Q	= 0x00f800f800f800f8;
	MMX15GREEN.Q	= 0x0000f8000000f800;  
	MMX15MULFACT.Q	= 0x2000000820000008;
	#define MMX15RGBSHIFT 6

	Screen.PtrBYTE  = Frame->Screen(0,MinY); 
	TexBase			= Mip->DataPtr;

	MMXCoMask.DL	= 0x3FFFFFFF >> (30- (Mip->UBits + Mip->VBits ));
	MMXCoShift.DL	= 32 - Mip->UBits;  // Shift right this amount. #debug needs full 64 bits cleared ?
	MMXCoShift.DH   = 0;

	FMMXPolyCSetup* Set = MMXSetup + MinY;
	FSpan** Index		= SpanBuffer->Index + MinY - SpanBuffer->StartY;

	if ( !(PolyFlags & (PF_Masked|PF_Translucent|PF_Modulated)) )
	{
	// Normal drawing:

		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// Prestepping part.
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						// Prestep the MMX vars by: (Span->Start - SpanX0) guaranteed > 0.
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB
							movq	mm2,[eax]FMMXPolyCSetup.FogRGB					

							PrestepOn:
								paddd   mm7,mm6
								paddw   mm4,mm5
								paddw   mm2,mm3
								dec		esi
							jnz PrestepOn

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
							movq	[eax]FMMXPolyCSetup.FogRGB,mm2
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
					__asm{

						// #debug make 1/odd  pixel a special case, all else can be unrolled.
						// Interleaved stuff == coordinate calculation.
						// jmp OldRender 

						mov     ecx,[Set]
						mov     esi,[SpanX0]
						mov     edx,[SpanX1]
						mov     [SavedEBP],ebp
						dec     edx   //#debug

						movq	mm7,[ecx]FMMXPolyCSetup.UV

						mov     ebx,[TexBase]			
						mov     edi,[Screen]			
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
							paddw       mm0, mm2           //  Add fogging.
							psraw       mm0,5              //  14-bit result to 9 bits  5???     
							packuswb    mm0,mm0            //  Pack words to bytes.

							// 32-to-15 bit conversion, single pixel...(intel appnote #553)
							movq        mm1,mm0					// copy result
							pand		mm0, [MMX15REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX15MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX15GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX15RGBSHIFT					// shift to final position
							movd        eax,mm0                 //
							mov			word ptr [edi+esi*2],ax // Store 15 bit value to screen...
							mov			ebp,[SavedEBP]			//
						jmp EndRender							//
						//////////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne:	//        lead-in partly above

							xor         eax,eax
							movq        mm1,mm7           // new UV

							movq		mm0,mm4           //  Copy lightvalue, should be 15-bits...
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender8:		  // Loop using current light & light delta's. Getting the texel coordinate				
							psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
							paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
					
							xor         eax, eax			//  
							add esi,1						//  inc 		esi                 //  ++X
							
							punpckhdq   mm1, mm1			//  

							psrlq       mm1,[MMXCoShift]	//
							paddw       mm0, mm2			//  Add fogging.

							pand        mm1,[MMXCoMask]     //
							psraw       mm0,5               //  14-bit result to 9 bits  5???     						

							paddw       mm2,mm3             //  fog += fogdelta
							packuswb    mm0,mm0             //  Pack words to bytes.

							// 32-to-15 bit conversion.

							movd        ecx,mm1             
							//////////////////////
							movq        mm1,mm0					// copy result
							pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX15GREEN]		// Mask out all but the 5MSBits of green
							por			mm0, mm1				// Combine the red, green, and blue bits
							psrld		mm0, MMX15RGBSHIFT					// Shift to final position
							movd        eax,mm0
							mov			word ptr [edi+esi*2-2], ax // Store 15 bit value to screen...
							xor         eax,eax					//

							movq        mm1,mm7					//
							movq        mm0,mm4					//

							cmp			esi, edx				//  cmp  X, PX;  
							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender8		//

						// Lead-out, write last pixel:     
						pmulhw		mm0, qword ptr [ebp+eax*8]	// edx = Colors palette base  eax= Texelcolor index.
						paddw       mm0, mm2					//  Add fogging.
						psraw       mm0,5						//  14-bit result to 9 bits  5???     
						packuswb    mm0,mm0						//  Pack words to bytes.

						// 32-to-15 bit conversion.
						movq        mm1,mm0					// Copy result.
						pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
						pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
						pand		mm1, [MMX15GREEN]		// Mask out all but the 5MSBits of green.

						por			mm0, mm1				// Combine the red, green, and blue bits.
						psrld		mm0, MMX15RGBSHIFT					// Shift to final position.
						movd        eax,mm0
						mov			word ptr [edi+esi*2], ax    //  Store 15 bit value to screen...

						//movd		[edi+esi*2], mm0   //  Store to screen. 
						mov			ebp,[SavedEBP]     //

						EndRender:
					}
				}
			}
		}
	}
	else if (  PolyFlags & PF_Translucent )
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB

							PrestepOn2:
								paddd   mm7,mm6
								paddw   mm4,mm5
								dec esi
							jnz PrestepOn2

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}

				////////////////////////////////////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
				__asm{

						// #debug make 1/odd pixel a special case, all else can be unrolled.
						// Interleaved stuff == coordinate calculation.
						// jmp OldRender 

						mov     [SavedESP],esp
						mov     ecx,[Set]
						mov     esi,[SpanX0]
						mov     edx,[SpanX1]
						mov     [SavedEBP],ebp
						dec     edx						//#debug

						movq	mm7,[ecx]FMMXPolyCSetup.UV

						mov     ebx,[TexBase]			
						mov     edi,[Screen]			
						mov     ebp,[MMXColors]				

						movq	mm4,[ecx]FMMXPolyCSetup.LightRGB
						movq    mm1,mm7
						psllq   mm1,12  

						mov			eax,edx				//
						paddd       mm7,mm6				//
						punpckhdq   mm1,mm1				// Copy high word to low word.

						sub			eax,esi				// eax == number of pixels to draw.
						psrlq       mm1,[MMXCoShift]	//       

						pand        mm1,[MMXCoMask]		//
						test		eax,eax				//

						movd		ecx,mm1				//	
						jnz			MoreThanOne2		//

						// Singular pixel....
							xor			eax,eax			//  explicitly, for PIIs sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							movq		mm0,mm4         //  Copy lightvalue, should be 15-bits...
							test        eax,eax         //
							jz          EndRender2      // bail out 
                                                        //
							//////////////////////////////
							xor         esp,esp         // zero it for pII sake
							mov         sp,[edi+esi*2]	// #debug interleave this all to above!

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							movd        ecx,mm1			//

							pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,11			//
							por         mm3,mm2			//

							psllq       mm2,11			//
							por         mm3,mm2			//

							psllw       mm3,11			// to 16bit 
							psrlw       mm3,3           // avoid sign + scaledown

							paddw       mm0,mm3			// Add signed saturated MMX color result.
							psraw       mm0,5			// 14-bit doublebright scaled result down to 9 bits....

							packuswb    mm0, mm0				//
							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							pand		mm3, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX15RGBSHIFT					// Shift to final position.
							movd        esp,mm0					//
							mov         [edi+esi*2],sp			//
							//////////////////////////////////////
						jmp EndRender2          

						////////////////
						align 16	  // #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne2: // lead-in partly above
							xor         eax,eax             //
							movq        mm1,mm7				// New UV.

							movq		mm0,mm4				// Copy lightvalue, should be 15-bits.
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender82:				// Loop using current light & light delta's. Getting the texel.
							test        eax,eax             //
							jz          SkipMaskedPixel2    //

							xor         esp,esp				// zero it for pII sake
							paddw		mm4, mm5			// + mm5, signed 16-bit R,G,B LightDeltas 

							mov         sp,[edi+esi*2]  // #debug interleave this all to above!
							paddd       mm7, mm6 		// PMULHW has severe caching penalty.

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							psllq		mm1, 12			// coordinate psllq mm1,12  is SLOW for some weird reason.

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,11			//					
							xor         eax, eax		// 

							por         mm3,mm2			//
							psllq       mm2,11			//
							
							por         mm3,mm2			//
							add			esi,1			// inc	esi     //  ++X

							punpckhdq   mm1, mm1		//
							psllw       mm3,11			// to 16bit 

							psrlq       mm1,[MMXCoShift]//
							psrlw       mm3,3           // avoid sign + scaledown

							pand        mm1,[MMXCoMask] //
							paddw       mm0,mm3			// Add signed saturated MMX color result.

							cmp			esi, edx             // cmp X, PX;  
							psraw       mm0,5				 // 14-bit doublebright scaled result down to 9 bits....
															 
							//////////////////////////////////////
							movd        ecx,mm1					//					
							packuswb    mm0, mm0				//

							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							movq        mm1,mm7					//
							pand		mm3, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX15RGBSHIFT					// Shift to final position.
							mov         al,byte ptr [ebx+ecx]	//
							movd        esp,mm0					//
							movq        mm0,mm4					//
							mov         [edi+esi*2-2],sp		//
							//////////////////////////////////////																														
							jb			ReloopInnerRender82		


						LeadOut2:								// write last pixel:     
						test        eax,eax						//
						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						jz          EndRender2 
						//////////////////////////////////////////
						xor         esp,esp         // zero it for pII sake
						mov         sp,[edi+esi*2]  // #debug interleave this all to above!

						movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
						movd        ecx,mm1			//

						movq        mm3,mm2			// Low one filled 
						psllq       mm2,11			//
						por         mm3,mm2			//
						psllq       mm2,11			//
						por         mm3,mm2			//
						psllw       mm3,11			// to 16bit 
						psrlw       mm3,3           // avoid sign + scaledown

						paddw       mm0,mm3			// Add signed saturated MMX color result.
						psraw       mm0,5			// 14-bit doublebright scaled result down to 9 bits....

						packuswb    mm0, mm0				//
						movq        mm3, mm0				// Copy result.
						pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
						pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
						pand		mm3, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
						por			mm0, mm3				// Combine the red, green, and blue bits.
						psrld		mm0, MMX15RGBSHIFT					// Shift to final position.
						movd        esp,mm0					//
						mov         [edi+esi*2],sp		//
						//////////////////////////////////////
						jmp				EndRender2


						align 16
						SkipMaskedPixel2:
							psllq		mm1, 12				// coordinate psllq mm1,12  is SLOW for some weird reason.
							paddw		mm4, mm5	

							paddd       mm7,mm6
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	//
							movq        mm0,mm4					//
							pand        mm1,[MMXCoMask]     //

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender82		//
						jmp LeadOut2

						EndRender2:
						mov			ebp,[SavedEBP]     //
						mov         esp,[SavedESP]     //

					}
				}
			}
		}
	}
	else if (  PolyFlags & PF_Modulated )
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						// Prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							//movq	mm4,[eax]FMMXPolyCSetup.LightRGB

							PrestepOnMod:
								paddd   mm7,mm6
								//paddw   mm4,mm5
								dec esi
							jnz PrestepOnMod

							movq	[eax]FMMXPolyCSetup.UV,mm7
							//movq	[eax]FMMXPolyCSetup.LightRGB,mm4
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}

				////////////////////////////////////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
				__asm{

						// #debug make 1/odd pixel a special case, all else can be unrolled.
						// Interleaved stuff == coordinate calculation.
						// jmp OldRender 

						mov     [SavedESP],esp
						mov     ecx,[Set]
						mov     esi,[SpanX0]
						mov     edx,[SpanX1]
						mov     [SavedEBP],ebp
						dec     edx						//#debug

						movq	mm7,[ecx]FMMXPolyCSetup.UV

						mov     ebx,[TexBase]			
						mov     edi,[Screen]			
						mov     ebp,[MMXColors]				

						//movq	mm4,[ecx]FMMXPolyCSetup.LightRGB
						movq    mm1,mm7
						psllq   mm1,12  

						mov			eax,edx				//
						paddd       mm7,mm6				//
						punpckhdq   mm1,mm1				// Copy high word to low word.

						sub			eax,esi				// eax == number of pixels to draw.
						psrlq       mm1,[MMXCoShift]	//       

						pand        mm1,[MMXCoMask]		//
						test		eax,eax				//

						movd		ecx,mm1				//	
						jnz			MoreThanOneMod		//

						// Singular pixel....
							xor			eax,eax			//  explicitly, for PIIs sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							//movq		mm0,mm4         //  Copy lightvalue, should be 15-bits...
							test        eax,eax         //
							jz          EndRenderMod      // bail out 
                                                        //
							//////////////////////////////
							xor         esp,esp         // zero it for pII sake
							mov         sp,[edi+esi*2]	// #debug interleave this all to above!

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							movd        ecx,mm1			//

							movq		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,11			//
							por         mm3,mm2			//

							psllq       mm2,11			//
							por         mm3,mm2			//

							psllw       mm3,11			// to 16bit 
							psrlw       mm3,3           // avoid sign + scaledown

							pmulhw      mm0,mm3			// MODULATE signed saturated MMX color result.
							psrlw       mm0,3           //

							packuswb    mm0, mm0				//
							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							pand		mm3, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX15RGBSHIFT					// Shift to final position.
							movd        esp,mm0					//
							mov         [edi+esi*2],sp			//
							//////////////////////////////////////
						jmp EndRenderMod          

						////////////////
						align 16	  // #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOneMod: // lead-in partly above
							xor         eax,eax             //
							movq        mm1,mm7				// New UV.

							//movq		mm0,mm4				// Copy lightvalue, should be 15-bits.
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender8Mod:				// Loop using current light & light delta's. Getting the texel.
							//test        eax,eax             //
							//jz          SkipMaskedPixelMod  //

							xor         esp,esp				// zero it for pII sake
							//paddw		mm4, mm5			// + mm5, signed 16-bit R,G,B LightDeltas 

							mov         sp,[edi+esi*2]  // #debug interleave this all to above!
							paddd       mm7, mm6 		// PMULHW has severe caching penalty.

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							psllq		mm1, 12			// coordinate psllq mm1,12  is SLOW for some weird reason.

							movq		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,11			//					
							xor         eax, eax		// 

							por         mm3,mm2			//
							psllq       mm2,11			//
							
							por         mm3,mm2			//
							add			esi,1			// inc	esi     //  ++X

							punpckhdq   mm1, mm1		//
							psllw       mm3,11			// to 16bit 

							psrlq       mm1,[MMXCoShift]//
							psrlw       mm3,3           // avoid sign + scaledown

							pand        mm1,[MMXCoMask] //

							pmulhw      mm0,mm3			// MODULATE signed saturated MMX color result.
							psrlw       mm0,3			// psraw       mm0,5 #debug

							cmp			esi, edx        // cmp X, PX;  
															 
							//////////////////////////////////////
							movd        ecx, mm1				//					
							packuswb    mm0, mm0				//
							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							movq        mm1, mm7				//
							pand		mm3, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX15RGBSHIFT					// Shift to final position.
							mov         al,byte ptr [ebx+ecx]	//
							movd        esp, mm0				//
							movq        mm0, mm4				//
							mov         [edi+esi*2-2], sp		//
							//////////////////////////////////////																														
							jb			ReloopInnerRender8Mod


						//LeadOutMod:								// write last pixel:     
						test        eax,eax						//
						movq		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						jz          EndRenderMod
						//////////////////////////////////////////
						xor         esp,esp         // zero it for pII sake
						mov         sp,[edi+esi*2]  // #debug interleave this all to above!

						movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
						movd        ecx,mm1			//

						movq        mm3,mm2			// Low one filled 
						psllq       mm2,11			//
						por         mm3,mm2			//
						psllq       mm2,11			//
						por         mm3,mm2			//
						psllw       mm3,11			// to 16bit 
						psrlw       mm3,3           // avoid sign + scaledown

						pmulhw      mm0,mm3			// MODULATE signed saturated MMX color result.
						psrlw       mm0,3       

						packuswb    mm0, mm0				//
						movq        mm3, mm0				// Copy result.
						pand		mm0, [MMX15REDBLUE]		// Mask out all but the 5MSBits of red and blue.
						pmaddwd		mm0, [MMX15MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
						pand		mm3, [MMX15GREEN]		// Mask out all but the 5MSBits of green.
						por			mm0, mm3				// Combine the red, green, and blue bits.
						psrld		mm0, MMX15RGBSHIFT					// Shift to final position.
						movd        esp,mm0					//
						mov         [edi+esi*2],sp		//
						//////////////////////////////////////
						//jmp				EndRenderMod


						/*
						align 16
						SkipMaskedPixelMod:
							psllq		mm1, 12				// coordinate psllq mm1,12  is SLOW for some weird reason.
							//paddw		mm4, mm5	

							paddd       mm7,mm6
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	//
							//movq        mm0,mm4					/
							pand        mm1,[MMXCoMask]     //

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender8Mod		//
						jmp LeadOutMod
						*/

						EndRenderMod:
						mov			ebp,[SavedEBP]     //
						mov         esp,[SavedESP]     //

					}
				}
			}
		}
	}
	else if (  PolyFlags & PF_Masked )
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB
							movq	mm2,[eax]FMMXPolyCSetup.FogRGB					

							PrestepOnMASK:
								paddd   mm7,mm6
								paddw   mm4,mm5
								paddw   mm2,mm3
								dec esi
							jnz PrestepOnMASK

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
							movq	[eax]FMMXPolyCSetup.FogRGB,mm2
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
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
						mov     edi,[Screen]			
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
						jnz			MoreThanOneMASK

						// Singular pixel....
							xor			eax,eax			   //  explicitly, for PII sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							movq		mm0,mm4            //  Copy lightvalue, should be 15-bits...
							test        eax,eax 
							jz          EndRenderMASK        // bail out 

							pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							paddw       mm0, mm2           //  Add fogging.
							psraw       mm0,5              //  14-bit result to 9 bits  5???     
							packuswb    mm0,mm0            //  Pack words to bytes.
							// 32-to-15 bit conversion, single pixel...(intel appnote #553)
							movq        mm1,mm0					// copy result
							pand		mm0, [MMX15REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX15MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX15GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX15RGBSHIFT					// shift to final position
							movd        eax,mm0                 //
							mov			word ptr [edi+esi*2],ax //  Store 15 bit value to screen...
							//movd		[edi+esi*2], mm0   //  Store to screen. 

						jmp EndRenderMASK                      //
						/////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOneMASK:	// lead-in partly above

							xor         eax,eax
							movq        mm1,mm7           // new UV

							movq		mm0,mm4           //  Copy lightvalue, should be 15-bits...
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender8MASK:		  // Loop using current light & light delta's. Getting the texel coordinate				
							psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
							test        eax,eax

							paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 
							jz          SkipMaskedPixelMASK

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							paddd       mm7, mm6 		     //  PMULHW has severe caching penalty.
					
							xor         eax, eax			 //  
							add esi,1						 //  inc 		esi                 //  ++X
							
							punpckhdq   mm1, mm1			 //

							psrlq       mm1,[MMXCoShift]	 //
							paddw       mm0, mm2			 //  Add fogging.

							pand        mm1,[MMXCoMask]      //
							psraw       mm0,5                //  14-bit result to 9 bits  5???     						

							paddw       mm2,mm3              //  fog += fogdelta
							packuswb    mm0,mm0              //  Pack words to bytes.

							movd        ecx,mm1              //

							//////////////////////
							movq        mm1,mm0 // copy result
							pand		mm0, [MMX15REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX15MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX15GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX15RGBSHIFT					// shift to final position
							movd        eax,mm0
							mov			word ptr [edi+esi*2-2], ax    //  Store 15 bit value to screen...
							xor         eax,eax
							//movd		[edi+esi*2-2], mm0   //  Store to screen. 

							cmp			esi, edx             //  cmp  X, PX;  

							movq        mm1,mm7              //
							movq        mm0,mm4              //

							mov         al,byte ptr [ebx+ecx]//
							jb			ReloopInnerRender8MASK  //


						LeadOutMASK: // write last pixel:     
						test        eax,eax
						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						jz          EndRenderMASK
						paddw       mm0, mm2           //  Add fogging.
						psraw       mm0,5              //  14-bit result to 9 bits  5???     
						packuswb    mm0,mm0            //  Pack words to bytes.
						// 32-to-15 bit conversion.
							movq        mm1,mm0 // copy result
							pand		mm0, [MMX15REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX15MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX15GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX15RGBSHIFT					// shift to final position
							movd        eax,mm0
							mov			word ptr [edi+esi*2], ax    //  Store 15 bit value to screen...
							//movd		[edi+esi*2], mm0   //  Store to screen. 
						jmp			EndRenderMASK


						align 16
						SkipMaskedPixelMASK:
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	//
							movq        mm0,mm4				//

							pand        mm1,[MMXCoMask]     //
							paddw       mm2,mm3             //  fog += fogdelta

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender8MASK	//

						jmp LeadOutMASK

						EndRenderMASK:
						mov			ebp,[SavedEBP]     //

					}
				}
			}
		}
	}

	#if ASM
	__asm emms;
	#endif

} // InnerGouraud15



 void USoftwareRenderDevice::InnerGouraudMMX16(DWORD PolyFlags, INT MinY, INT MaxY, FSceneNode* Frame,FMipmap* Mip,FSpanBuffer* SpanBuffer)
{
	static  BYTE*	TexBase;
	static  FMMX*	PalBase;
	static  FMMX	MMXCoMask,MMXCoShift; 
	static  FRainbowPtr  Screen;

	static  FMMX    MMX16REDBLUE;	//masks out all but the 5MSBits of red and blue
	static  FMMX    MMX16GREEN;		//masks out all but the 5MSBits of green
	static  FMMX    MMX16MULFACT;	//multiply each word by 2**13, 2**3, 2**13, 2**3 and add results

	//#debug just change these globals to suit 15 or 16 bit conversion
	MMX16REDBLUE.Q	= 0x00f800f800f800f8;
	MMX16GREEN.Q	= 0x0000fC000000fC00;  
	MMX16MULFACT.Q	= 0x2000000420000004;
	#define MMX16RGBSHIFT 5
	

	Screen.PtrBYTE  = Frame->Screen(0,MinY); 
	TexBase			= Mip->DataPtr;

	MMXCoMask.DL	= 0x3FFFFFFF >> (30- (Mip->UBits + Mip->VBits ));
	MMXCoShift.DL	= 32 - Mip->UBits;  // Shift right this amount. #debug needs full 64 bits cleared ?
	MMXCoShift.DH   = 0;

	FMMXPolyCSetup* Set = MMXSetup + MinY;
	FSpan** Index		= SpanBuffer->Index + MinY - SpanBuffer->StartY;

	if ( !(PolyFlags & (PF_Masked|PF_Translucent|PF_Modulated)) )
	{
	// Normal drawing:

		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// Prestepping part.
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						// Prestep the MMX vars by: (Span->Start - SpanX0) guaranteed > 0.
						__asm
						{
							mov edi,[Span]
							mov eax,[Set]
							mov ebx,[SpanX0]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB
							movq	mm2,[eax]FMMXPolyCSetup.FogRGB					

							PrestepOn:
								paddd   mm7,mm6
								paddw   mm4,mm5
								paddw   mm2,mm3
								dec		esi
							jnz PrestepOn

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
							movq	[eax]FMMXPolyCSetup.FogRGB,mm2
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
					__asm{

						// #debug make 1/odd  pixel a special case, all else can be unrolled.
						// Interleaved stuff == coordinate calculation.
						// jmp OldRender 

						mov     ecx,[Set]
						mov     esi,[SpanX0]
						mov     edx,[SpanX1]
						mov     [SavedEBP],ebp
						dec     edx   //#debug

						movq	mm7,[ecx]FMMXPolyCSetup.UV

						mov     ebx,[TexBase]			
						mov     edi,[Screen]			
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
							paddw       mm0, mm2           //  Add fogging.
							psraw       mm0,5              //  14-bit result to 9 bits  5???     
							packuswb    mm0,mm0            //  Pack words to bytes.

							// 32-to-15 bit conversion, single pixel...(intel appnote #553)
							movq        mm1,mm0					// copy result
							pand		mm0, [MMX16REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX16MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX16GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX16RGBSHIFT      // shift to final position
							movd        eax,mm0                 //
							mov			word ptr [edi+esi*2],ax // Store 15 bit value to screen...
							mov			ebp,[SavedEBP]			//
						jmp EndRender							//
						//////////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne:	//        lead-in partly above

							xor         eax,eax
							movq        mm1,mm7           // new UV

							movq		mm0,mm4           //  Copy lightvalue, should be 15-bits...
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender8:		  // Loop using current light & light delta's. Getting the texel coordinate				
							psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
							paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
					
							xor         eax, eax			//  
							add esi,1						//  inc 		esi                 //  ++X
							
							punpckhdq   mm1, mm1			//  

							psrlq       mm1,[MMXCoShift]	//
							paddw       mm0, mm2			//  Add fogging.

							pand        mm1,[MMXCoMask]     //
							psraw       mm0,5               //  14-bit result to 9 bits  5???     						

							paddw       mm2,mm3             //  fog += fogdelta
							packuswb    mm0,mm0             //  Pack words to bytes.

							// 32-to-15 bit conversion.

							movd        ecx,mm1             
							//////////////////////
							movq        mm1,mm0					// copy result
							pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX16GREEN]		// Mask out all but the 5MSBits of green
							por			mm0, mm1				// Combine the red, green, and blue bits
							psrld		mm0, MMX16RGBSHIFT
							movd        eax,mm0
							mov			word ptr [edi+esi*2-2], ax // Store 15 bit value to screen...
							xor         eax,eax					//

							movq        mm1,mm7					//
							movq        mm0,mm4					//

							cmp			esi, edx				//  cmp  X, PX;  
							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender8		//

						// Lead-out, write last pixel:     
						pmulhw		mm0, qword ptr [ebp+eax*8]	// edx = Colors palette base  eax= Texelcolor index.
						paddw       mm0, mm2					//  Add fogging.
						psraw       mm0,5						//  14-bit result to 9 bits  5???     
						packuswb    mm0,mm0						//  Pack words to bytes.

						// 32-to-15 bit conversion.
						movq        mm1,mm0					// Copy result.
						pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
						pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
						pand		mm1, [MMX16GREEN]		// Mask out all but the 5MSBits of green.

						por			mm0, mm1				// Combine the red, green, and blue bits.
						psrld		mm0, MMX16RGBSHIFT
						movd        eax,mm0
						mov			word ptr [edi+esi*2], ax    //  Store 15 bit value to screen...

						//movd		[edi+esi*2], mm0   //  Store to screen. 
						mov			ebp,[SavedEBP]     //

						EndRender:
					}
				}
			}
		}
	}
	else if (  PolyFlags & PF_Translucent )
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB

							PrestepOn2:
								paddd   mm7,mm6
								paddw   mm4,mm5
								dec esi
							jnz PrestepOn2

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}

				////////////////////////////////////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
				__asm{

						// #debug make 1/odd pixel a special case, all else can be unrolled.
						// Interleaved stuff == coordinate calculation.
						// jmp OldRender 

						mov     [SavedESP],esp
						mov     ecx,[Set]
						mov     esi,[SpanX0]
						mov     edx,[SpanX1]
						mov     [SavedEBP],ebp
						dec     edx						//#debug

						movq	mm7,[ecx]FMMXPolyCSetup.UV

						mov     ebx,[TexBase]			
						mov     edi,[Screen]			
						mov     ebp,[MMXColors]				

						movq	mm4,[ecx]FMMXPolyCSetup.LightRGB
						movq    mm1,mm7
						psllq   mm1,12  

						mov			eax,edx				//
						paddd       mm7,mm6				//
						punpckhdq   mm1,mm1				// Copy high word to low word.

						sub			eax,esi				// eax == number of pixels to draw.
						psrlq       mm1,[MMXCoShift]	//       

						pand        mm1,[MMXCoMask]		//
						test		eax,eax				//

						movd		ecx,mm1				//	
						jnz			MoreThanOne2		//

						// Singular pixel....
							xor			eax,eax			//  explicitly, for PIIs sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							movq		mm0,mm4         //  Copy lightvalue, should be 15-bits...
							test        eax,eax         //
							jz          EndRender2      // bail out 
                                                        //
							//////////////////////////////
							xor         esp,esp         // zero it for pII sake
							mov         sp,[edi+esi*2]	// #debug interleave this all to above!

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							movd        ecx,mm1			//

							pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,10			//
							por         mm3,mm2			//

							psllq       mm2,11			//
							por         mm3,mm2			//

							psllw       mm3,11  		// to 16bit 
							psrlw       mm3,3           // avoid sign + scaledown

							paddw       mm0,mm3			// Add signed saturated MMX color result.
							psraw       mm0,5			// 14-bit doublebright scaled result down to 9 bits....

							packuswb    mm0, mm0				//
							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							pand		mm3, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX16RGBSHIFT
							movd        esp,mm0					//
							mov         [edi+esi*2],sp			//
							//////////////////////////////////////
						jmp EndRender2          

						////////////////
						align 16	  // #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOne2: // lead-in partly above
							xor         eax,eax             //
							movq        mm1,mm7				// New UV.

							movq		mm0,mm4				// Copy lightvalue, should be 15-bits.
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender82:				// Loop using current light & light delta's. Getting the texel.
							test        eax,eax             //
							jz          SkipMaskedPixel2    //

							xor         esp,esp				// zero it for pII sake
							paddw		mm4, mm5			// + mm5, signed 16-bit R,G,B LightDeltas 

							mov         sp,[edi+esi*2]  // #debug interleave this all to above!
							paddd       mm7, mm6 		// PMULHW has severe caching penalty.

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							psllq		mm1, 12			// coordinate psllq mm1,12  is SLOW for some weird reason.

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,10			//					
							xor         eax, eax		// 

							por         mm3,mm2			//
							psllq       mm2,11			//
							
							por         mm3,mm2			//
							add			esi,1			// inc	esi     //  ++X

							punpckhdq   mm1, mm1		//
							psllw       mm3,11			// to 16bit 

							psrlq       mm1,[MMXCoShift]//
							psrlw       mm3,3           // avoid sign + scaledown

							pand        mm1,[MMXCoMask] //
							paddw       mm0,mm3			// Add signed saturated MMX color result.

							cmp			esi, edx             // cmp X, PX;  
							psraw       mm0,5				 // 14-bit doublebright scaled result down to 9 bits....
															 
							//////////////////////////////////////
							movd        ecx,mm1					//					
							packuswb    mm0, mm0				//

							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							movq        mm1,mm7					//
							pand		mm3, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX16RGBSHIFT
							mov         al,byte ptr [ebx+ecx]	//
							movd        esp,mm0					//
							movq        mm0,mm4					//
							mov         [edi+esi*2-2],sp		//
							//////////////////////////////////////																														
							jb			ReloopInnerRender82		


						LeadOut2:								// write last pixel:     
						test        eax,eax						//
						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						jz          EndRender2 
						//////////////////////////////////////////
						xor         esp,esp         // zero it for pII sake
						mov         sp,[edi+esi*2]  // #debug interleave this all to above!

						movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
						movd        ecx,mm1			//

						movq        mm3,mm2			// Low one filled 
						psllq       mm2,10			//
						por         mm3,mm2			//
						psllq       mm2,11			//
						por         mm3,mm2			//
						psllw       mm3,11			// to 16bit 
						psrlw       mm3,3           // avoid sign + scaledown

						paddw       mm0,mm3			// Add signed saturated MMX color result.
						psraw       mm0,5			// 14-bit doublebright scaled result down to 9 bits....

						packuswb    mm0, mm0				//
						movq        mm3, mm0				// Copy result.
						pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
						pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
						pand		mm3, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
						por			mm0, mm3				// Combine the red, green, and blue bits.
						psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
						movd        esp,mm0					//
						mov         [edi+esi*2],sp		//
						//////////////////////////////////////
						jmp				EndRender2


						align 16
						SkipMaskedPixel2:
							psllq		mm1, 12				// coordinate psllq mm1,12  is SLOW for some weird reason.
							paddw		mm4, mm5	

							paddd       mm7,mm6
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	//
							movq        mm0,mm4					//
							pand        mm1,[MMXCoMask]     //

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender82		//
						jmp LeadOut2

						EndRender2:
						mov			ebp,[SavedEBP]     //
						mov         esp,[SavedESP]     //

					}
				}
			}
		}
	}
	else if (  PolyFlags & PF_Modulated )
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							//movq	mm4,[eax]FMMXPolyCSetup.LightRGB

							PrestepOnMod:
								paddd   mm7,mm6
								//paddw   mm4,mm5
								dec esi
							jnz PrestepOnMod

							movq	[eax]FMMXPolyCSetup.UV,mm7
							//movq	[eax]FMMXPolyCSetup.LightRGB,mm4
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}

				////////////////////////////////////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
				__asm{

						// #debug make 1/odd pixel a special case, all else can be unrolled.
						// Interleaved stuff == coordinate calculation.
						// jmp OldRender 

						mov     [SavedESP],esp
						mov     ecx,[Set]
						mov     esi,[SpanX0]
						mov     edx,[SpanX1]
						mov     [SavedEBP],ebp
						dec     edx						//#debug

						movq	mm7,[ecx]FMMXPolyCSetup.UV

						mov     ebx,[TexBase]			
						mov     edi,[Screen]			
						mov     ebp,[MMXColors]				

						//movq	mm4,[ecx]FMMXPolyCSetup.LightRGB
						movq    mm1,mm7
						psllq   mm1,12  

						mov			eax,edx				//
						paddd       mm7,mm6				//
						punpckhdq   mm1,mm1				// Copy high word to low word.

						sub			eax,esi				// eax == number of pixels to draw.
						psrlq       mm1,[MMXCoShift]	//       

						pand        mm1,[MMXCoMask]		//
						test		eax,eax				//

						movd		ecx,mm1				//	
						jnz			MoreThanOneMod		//

						// Singular pixel....
							xor			eax,eax			//  explicitly, for PIIs sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							//movq		mm0,mm4         //  Copy lightvalue, should be 15-bits...
							test        eax,eax         //
							jz          EndRenderMod    // bail out 
                                                        //
							//////////////////////////////
							xor         esp,esp         // zero it for pII sake
							mov         sp,[edi+esi*2]	// #debug interleave this all to above!

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							movd        ecx,mm1			//

							movq		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,10			//
							por         mm3,mm2			//

							psllq       mm2,11			//
							por         mm3,mm2			//

							psllw       mm3,11			// to 16bit 
							psrlw       mm3,3           // avoid sign + scaledown

							pmulhw      mm0,mm3			// MODULATE signed saturated MMX color result.
							psrlw       mm0,3

							packuswb    mm0, mm0				//
							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							pand		mm3, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
							movd        esp,mm0					//
							mov         [edi+esi*2],sp			//
							//////////////////////////////////////
						jmp EndRenderMod          

						////////////////
						align 16	  // #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOneMod: // lead-in partly above
							xor         eax,eax             //
							movq        mm1,mm7				// New UV.

							//movq		mm0,mm4				// Copy lightvalue, should be 15-bits.
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender8Mod:				// Loop using current light & light delta's. Getting the texel.
							//test        eax,eax             //
							//jz          SkipMaskedPixelMod  //

							xor         esp,esp				// zero it for pII sake
							//paddw		mm4, mm5			// + mm5, signed 16-bit R,G,B LightDeltas 

							mov         sp,[edi+esi*2]  // #debug interleave this all to above!
							paddd       mm7, mm6 		// PMULHW has severe caching penalty.

							movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
							psllq		mm1, 12			// coordinate psllq mm1,12  is SLOW for some weird reason.

							movq		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							movq        mm3,mm2			// Low one filled 

							psllq       mm2,10			//					
							xor         eax, eax		// 

							por         mm3,mm2			//
							psllq       mm2,11			//
							
							por         mm3,mm2			//
							add			esi,1			// inc	esi     //  ++X

							punpckhdq   mm1, mm1		//
							psllw       mm3,11			// to 16bit 

							psrlq       mm1,[MMXCoShift]//
							psrlw       mm3,3           // avoid sign + scaledown

							pand        mm1,[MMXCoMask] //

							pmulhw      mm0,mm3			// MODULATE signed saturated MMX color result.
							psrlw       mm0,3

							cmp			esi, edx        // cmp X, PX;  
															 
							//////////////////////////////////////
							movd        ecx, mm1				//					
							packuswb    mm0, mm0				//
							movq        mm3, mm0				// Copy result.
							pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
							pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
							movq        mm1, mm7				//
							pand		mm3, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
							por			mm0, mm3				// Combine the red, green, and blue bits.
							psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
							mov         al,byte ptr [ebx+ecx]	//
							movd        esp, mm0				//
							movq        mm0, mm4				//
							mov         [edi+esi*2-2], sp		//
							//////////////////////////////////////																														
							jb			ReloopInnerRender8Mod		


						//LeadOutMod:								// write last pixel:     
						test        eax,eax						//
						movq		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						jz          EndRenderMod
						//////////////////////////////////////////
						xor         esp,esp         // zero it for pII sake
						mov         sp,[edi+esi*2]  // #debug interleave this all to above!

						movd        mm2,esp     	// use mm2 and mm1 to unpack 15-bit to 15,15,15 FMMX 
						movd        ecx,mm1			//

						movq        mm3,mm2			// Low one filled 
						psllq       mm2,10			//
						por         mm3,mm2			//
						psllq       mm2,11			//
						por         mm3,mm2			//
						psllw       mm3,11			// to 16bit 
						psrlw       mm3,3           // avoid sign + scaledown

						pmulhw      mm0,mm3			// MODULATE signed saturated MMX color result.
						psrlw       mm0,3           //

						packuswb    mm0, mm0				//
						movq        mm3, mm0				// Copy result.
						pand		mm0, [MMX16REDBLUE]		// Mask out all but the 5MSBits of red and blue.
						pmaddwd		mm0, [MMX16MULFACT]		// Multiply each word by 2**13, 2**3, 2**13, 2**3 and add results.
						pand		mm3, [MMX16GREEN]		// Mask out all but the 5MSBits of green.
						por			mm0, mm3				// Combine the red, green, and blue bits.
						psrld		mm0, MMX16RGBSHIFT		// Shift to final position.
						movd        esp,mm0					//
						mov         [edi+esi*2],sp			//
						//////////////////////////////////////
						//jmp				EndRenderMod


						/*
						align 16
						SkipMaskedPixelMod:
							psllq		mm1, 12				// coordinate psllq mm1,12  is SLOW for some weird reason.
							//paddw		mm4, mm5			//

							paddd       mm7,mm6
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	//
							//movq        mm0,mm4				//
							pand        mm1,[MMXCoMask]     //

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender8Mod	//
						jmp LeadOutMod
						*/

						EndRenderMod:
						mov			ebp,[SavedEBP]     //
						mov         esp,[SavedESP]     //

					}
				}
			}
		}
	}
	else if (  PolyFlags & PF_Masked )
	{
		for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT SpanX0 = Set->StartX;  
				INT SpanX1 = Set->EndX;
				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				// prestepping part
				if (SpanX0 < Span->Start)
				{ 
					if (Span->Start < SpanX1) // ANY to draw let alone prestep ?
					{
						//prestep the MMX vars by: (Span->Start - SpanX0) guaranteed >0
						__asm
						{
							mov edi,[Span]
							mov ebx,[SpanX0]
							mov eax,[Set]
							mov esi,[edi]FSpan.Start

							movq	mm7,[eax]FMMXPolyCSetup.UV
							sub		esi,ebx
							movq	mm4,[eax]FMMXPolyCSetup.LightRGB
							movq	mm2,[eax]FMMXPolyCSetup.FogRGB					

							PrestepOnMASK:
								paddd   mm7,mm6
								paddw   mm4,mm5
								paddw   mm2,mm3
								dec esi
							jnz PrestepOnMASK

							movq	[eax]FMMXPolyCSetup.UV,mm7
							movq	[eax]FMMXPolyCSetup.LightRGB,mm4
							movq	[eax]FMMXPolyCSetup.FogRGB,mm2
						}
						SpanX0 =  Span->Start; // new start
					}
					else continue; 
				}
				/////////////////////////

				if( SpanX1 > SpanX0 ) // any to draw ?
				{
					//Draw from X0 to X1
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
						mov     edi,[Screen]			
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
						jnz			MoreThanOneMASK

						// Singular pixel....
							xor			eax,eax			   //  explicitly, for PII sake.
							mov         al, byte ptr [ebx+ecx] //  Get the texel - ebx = Texture map base. 
							movq		mm0,mm4            //  Copy lightvalue, should be 15-bits...
							test        eax,eax 
							jz          EndRenderMASK        // bail out 

							pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
							paddw       mm0, mm2           //  Add fogging.
							psraw       mm0,5              //  14-bit result to 9 bits  5???     
							packuswb    mm0,mm0            //  Pack words to bytes.
							// 32-to-15 bit conversion, single pixel...(intel appnote #553)
							movq        mm1,mm0					// copy result
							pand		mm0, [MMX16REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX16MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX16GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX16RGBSHIFT		// shift to final position
							movd        eax,mm0                 //
							mov			word ptr [edi+esi*2],ax //  Store 15 bit value to screen...
							//movd		[edi+esi*2], mm0   //  Store to screen. 

						jmp EndRenderMASK                      //
						/////////////////////////////////////

						align 16		// #debug fudge this so the Loop will start on paragraph boundary
						MoreThanOneMASK:	// lead-in partly above

							xor         eax,eax
							movq        mm1,mm7           // new UV

							movq		mm0,mm4           //  Copy lightvalue, should be 15-bits...
							mov         al, byte ptr [ebx+ecx]

						ReloopInnerRender8MASK:		  // Loop using current light & light delta's. Getting the texel coordinate				
							psllq		mm1, 12   // psllq mm1,12  is SLOW for some weird reason.....
							test        eax,eax

							paddw		mm4, mm5  // + mm5, signed 16-bit R,G,B LightDeltas 
							jz          SkipMaskedPixelMASK

							pmulhw		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
							paddd       mm7, mm6 		     //  PMULHW has severe caching penalty.
					
							xor         eax, eax			 //  
							add esi,1						 //  inc 		esi                 //  ++X
							
							punpckhdq   mm1, mm1			 //

							psrlq       mm1,[MMXCoShift]	 //
							paddw       mm0, mm2			 //  Add fogging.

							pand        mm1,[MMXCoMask]      //
							psraw       mm0,5                //  14-bit result to 9 bits  5???     						

							paddw       mm2,mm3              //  fog += fogdelta
							packuswb    mm0,mm0              //  Pack words to bytes.

							movd        ecx,mm1              //

							//////////////////////
							movq        mm1,mm0 // copy result
							pand		mm0, [MMX16REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX16MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX16GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX16RGBSHIFT		// shift to final position
							movd        eax,mm0
							mov			word ptr [edi+esi*2-2], ax    //  Store 15 bit value to screen...
							xor         eax,eax
							//movd		[edi+esi*2-2], mm0   //  Store to screen. 

							cmp			esi, edx             //  cmp  X, PX;  

							movq        mm1,mm7              //
							movq        mm0,mm4              //

							mov         al,byte ptr [ebx+ecx]//
							jb			ReloopInnerRender8MASK  //


						LeadOutMASK: // write last pixel:     
						test        eax,eax
						pmulhw		mm0, qword ptr [ebp+eax*8]  // edx = colors palette base  eax= texelcolor index.
						jz          EndRenderMASK
						paddw       mm0, mm2           //  Add fogging.
						psraw       mm0,5              //  14-bit result to 9 bits  5???     
						packuswb    mm0,mm0            //  Pack words to bytes.
						// 32-to-15 bit conversion.
							movq        mm1,mm0 // copy result
							pand		mm0, [MMX16REDBLUE]		// mask out all but the 5MSBits of red and blue
							pmaddwd		mm0, [MMX16MULFACT]		// multiply each word by 2**13, 2**3, 2**13, 2**3 and add results
							pand		mm1, [MMX16GREEN]		// mask out all but the 5MSBits of green
							por			mm0, mm1				// combine the red, green, and blue bits
							psrld		mm0, MMX16RGBSHIFT   	// shift to final position
							movd        eax,mm0
							mov			word ptr [edi+esi*2], ax    //  Store 15 bit value to screen...
							//movd		[edi+esi*2], mm0   //  Store to screen. 
						jmp			EndRenderMASK


						align 16
						SkipMaskedPixelMASK:
							paddd       mm7, mm6 		    //  PMULHW has severe caching penalty.
							add esi,1						//  inc 		esi                 //  ++X

							punpckhdq   mm1, mm1			//
							cmp			esi, edx            //  cmp  X, PX;  

							psrlq       mm1,[MMXCoShift]	//
							movq        mm0,mm4				//

							pand        mm1,[MMXCoMask]     //
							paddw       mm2,mm3             //  fog += fogdelta

							movd        ecx,mm1             //
							movq        mm1,mm7             //

							mov         al,byte ptr [ebx+ecx]	//
							jb			ReloopInnerRender8MASK	//

						jmp LeadOutMASK

						EndRenderMASK:
						mov			ebp,[SavedEBP]     //

					}
				}
			}
		}
	}

	#if ASM
	__asm emms;
	#endif

} // InnerGouraud15




//
// Pack 3 floating point numbers vect123 to a single MMXRegister as 15 = 8:7 bit...
// fixed point signed numbers.... Cannot use actual MMX since FPU instructions are close.
//

static FLOAT Magic15 = (3 << (22-14));
static FLOAT Magic14 = (3 << (22-13));

#define MMX15Load(MMXRegister, VectR, VectG, VectB)\
{\
	FLOAT TempR,TempG,TempB;\
	TempR = ((FLOAT)VectR + Magic15);\
	TempG = ((FLOAT)VectG + Magic15);\
	TempB = ((FLOAT)VectB + Magic15);\
	MMXRegister.DH =   ( *(DWORD*)&TempR );\
	MMXRegister.DL =  (( *(DWORD*)&TempB ) & 0xFFFF) + (( *(DWORD*)&TempG ) << 16);\
}

#define MMX14Load(MMXRegister, VectR, VectG, VectB)\
{\
	FLOAT TempR,TempG,TempB;\
	TempR = ((FLOAT)VectR + Magic14);\
	TempG = ((FLOAT)VectG + Magic14);\
	TempB = ((FLOAT)VectB + Magic14);\
	MMXRegister.DH =   ( *(DWORD*)&TempR );\
	MMXRegister.DL =  (( *(DWORD*)&TempB ) & 0xFFFF) + (( *(DWORD*)&TempG ) << 16);\
}

// Fully MACROed version is slightly faster than simple  function.

void USoftwareRenderDevice::MMXFlashTriangle
(										
	FSceneNode* Frame,
	FTextureInfo& Texture,
	DWORD PolyFlags,
	FSpanBuffer* SpanBuffer
)
{
	guardSlow(USoftwareRenderDevice::MMXFlashTriangle);


	// #optimize me ! (C++ generates bad asm code)
	// Guard against indefinite floats which got translated to Min_Int...!

	if ( ( (DWORD)Point0->IntY   == 0x80000000 ) ||
		 ( (DWORD)Point1->IntY   == 0x80000000 ) ||
		 ( (DWORD)Point2->IntY   == 0x80000000 ) )
	{  
		debugf(NAME_Log,"Indefinite ScreenY triangle coordinate error.");
		return;
	}


	/*
	#if DO_SLOW_GUARD
	if (! _finite( Point0->U) ||
		! _finite( Point0->V) ||		
		! _finite( Point1->U) ||
		! _finite( Point1->V) ||
		! _finite( Point2->U) ||
		! _finite( Point2->V) ) 
		appErrorf(" Non-finite texture coordinate input for MMXFlashTriangle.");
	#endif
	*/


	
	INT	MinY, MaxY, MinX, MaxX;
	MinX = MaxX = appFloor( Point0->ScreenX );
	MinY = MaxY = TriVertex[0].YFloor = Point0->IntY; //appFloor( Point0->ScreenY );

	INT	PX1 = appFloor( Point1->ScreenX );
	INT PY1 = TriVertex[1].YFloor = Point1->IntY; //appFloor( Point1->ScreenY );
	MinX  = Min(MinX,PX1);
	MaxX  = Max(MaxX,PX1);
	MinY  = Min(MinY,PY1);
	MaxY  = Max(MaxY,PY1);

    INT PX2 = appFloor( Point2->ScreenX );
    INT PY2 = TriVertex[2].YFloor = Point2->IntY; //appFloor( Point2->ScreenY );
	MinX  = Min(MinX,PX2);
	MaxX  = Max(MaxX,PX2);
	MinY  = Min(MinY,PY2);
	MaxY  = Max(MaxY,PY2);

	// Clip vertical range to spanbuffer. This *will* make MinY > MaxY in some cases.
	MinY = Max( MinY,SpanBuffer->StartY);
	MaxY = Min( MaxY,SpanBuffer->EndY);

	INT SizeX = MaxX-MinX;
	INT SizeY = MaxY-MinY;
	if ( (SizeX<1) || (SizeY<1) ) return;

	
	/*
	static INT Triangles = 0;
	static INT TriReject = 0;
	static INT TriSmall  = 0;
	if (((MaxY-MinY)<3) && ((MaxX-MinX)<3)) TriSmall++;
	Triangles++;
	if ( !( MinY < MaxY ) || !(MinX < MaxX) ) TriReject++;
	if (!(Triangles & 4095)) debugf(NAME_Log," TriAll, Reject, Small: %i %i %i ",Triangles,TriReject,TriSmall);
	*/
	
	FLOAT D12y = Point1->ScreenY - Point2->ScreenY;
	FLOAT D02y = Point0->ScreenY - Point2->ScreenY;

	FLOAT TriDivisor = ( (Point0->ScreenX - Point2->ScreenX)*D12y - (Point1->ScreenX - Point2->ScreenX)*D02y);
	DWORD TriExponent;
	*(FLOAT*)&TriExponent = TriDivisor;

	FLOAT DivTriDelta     = 1.0f / TriDivisor;
	// #debug -  Probably NOT essential to fatal error prevention as long as destination 
	// and source are always clipped & wrapped within bounds...
	// Refuse near-zero divisor.
	// Falls inside FDIV though, so mostly OK.

	if ( (TriExponent & 0x7FFFFFFF) < ( 16 << 23) )  // 16 appears acceptable low bound...
	{   
		//debugf(NAME_Log,"Tridivisor underflow. ");  
		return; 
	}

	FLOAT SD12y = D12y * DivTriDelta;
	FLOAT SD02y = D02y * DivTriDelta;

	// Compute the texture mapping delta's.
	FLOAT dUdX =  (Point0->U - Point2->U)*SD12y - (Point1->U - Point2->U)*SD02y;
	FLOAT dVdX =  (Point0->V - Point2->V)*SD12y - (Point1->V - Point2->V)*SD02y;

	// Max exponent (dUdX ,dVdX) determines mipmap to use.
	// scaling bias by adding a number to ExpU and ExpV.
	DWORD ExpU,ExpV;
	*(FLOAT*)&ExpU = (FLOAT)dUdX;
	*(FLOAT*)&ExpV = (FLOAT)dVdX;

	// Fill the 4 different precalculated presteps, [ 0/4, 1/4, 2/4, 3/4 ]
	// The two (arbitrary) +0x600000 offsets brings the mips closer since it carries the 
	// mantissa over into the exponent. Scales from 0x000000 to max 0x7f0000. 

	INT MipFactor = Max(( (ExpU&0x7FFFFFFF) + 0x600000 ) >>23, ( (ExpV & 0x7FFFFFFF) + 0x600000 ) >>23);
	INT iMip = Clamp( (MipFactor-127), 0, Texture.NumMips-1 );

	// Scale the Fog and Light colors at the vertices. Scale and Fog values range 0-255 for MMX.
	// Into TriVertex points just so we don't have to modify any vertices.
	// "saturation" per vertex, per channel... (hope that doesn't skew colors relatively?)

	// #debug  Todo: optimize me !

	guardSlow(Poly vertex light scaling);

	if (PolyFlags & PF_RenderFog)		
	{
		
		TriVertex[0].Fog.R = ( Point0->Fog.R * GFloatScale.R + GFloatFog.R ); 
		TriVertex[0].Fog.G = ( Point0->Fog.G * GFloatScale.G + GFloatFog.G ); 
		TriVertex[0].Fog.B = ( Point0->Fog.B * GFloatScale.B + GFloatFog.B ); 

		TriVertex[1].Fog.R = ( Point1->Fog.R * GFloatScale.R + GFloatFog.R ); 
		TriVertex[1].Fog.G = ( Point1->Fog.G * GFloatScale.G + GFloatFog.G ); 
		TriVertex[1].Fog.B = ( Point1->Fog.B * GFloatScale.B + GFloatFog.B ); 

		TriVertex[2].Fog.R = ( Point2->Fog.R * GFloatScale.R + GFloatFog.R ); 
		TriVertex[2].Fog.G = ( Point2->Fog.G * GFloatScale.G + GFloatFog.G ); 
		TriVertex[2].Fog.B = ( Point2->Fog.B * GFloatScale.B + GFloatFog.B ); 
		
		//
		// Attenuate the lightscale by the fog. 
		// #debug rearrange, and put clamping back in ????? / mult by 1-Trivertex fog instead ????
		// #debug  1.0f - PointX->Fog.R/G/B.... -> 1.f - Fog.A...
		//
		// Point0->Fog.A = Point0->Fog.G*2;
		// Point1->Fog.A = Point1->Fog.G*2;
		// Point2->Fog.A = Point2->Fog.G*2;
		// #debug
		//

		//FLOAT FogScale0 = 1.0f-Point0->Fog.A;

		TriVertex[0].Light.R =Point0->Light.R * (1.f-Point0->Fog.R) * GFloatScale.R;
		TriVertex[0].Light.G =Point0->Light.G * (1.f-Point0->Fog.G) * GFloatScale.G;
		TriVertex[0].Light.B =Point0->Light.B * (1.f-Point0->Fog.B) * GFloatScale.B;

		//FLOAT FogScale1 = 1.0f-Point1->Fog.A;

		TriVertex[1].Light.R =Point1->Light.R * (1.f-Point1->Fog.R) * GFloatScale.R;
		TriVertex[1].Light.G =Point1->Light.G * (1.f-Point1->Fog.G) * GFloatScale.G;
		TriVertex[1].Light.B =Point1->Light.B * (1.f-Point1->Fog.B) * GFloatScale.B;

		//FLOAT FogScale2 = 1.0f-Point2->Fog.A;

		TriVertex[2].Light.R =Point2->Light.R * (1.f-Point2->Fog.R) * GFloatScale.R;
		TriVertex[2].Light.G =Point2->Light.G * (1.f-Point2->Fog.G) * GFloatScale.G;
		TriVertex[2].Light.B =Point2->Light.B * (1.f-Point2->Fog.B) * GFloatScale.B;

	}
	else // Use the uniform screenflash-fog instead:
	{
		TriVertex[0].Fog.R = GFloatFog.R; 
		TriVertex[0].Fog.G = GFloatFog.G; 
		TriVertex[0].Fog.B = GFloatFog.B; 

		TriVertex[1].Fog.R = GFloatFog.R; 
		TriVertex[1].Fog.G = GFloatFog.G; 
		TriVertex[1].Fog.B = GFloatFog.B; 

		TriVertex[2].Fog.R = GFloatFog.R; 
		TriVertex[2].Fog.G = GFloatFog.G; 
		TriVertex[2].Fog.B = GFloatFog.B; 

		
		TriVertex[0].Light.R = MinPositiveFloat( Point0->Light.R * GFloatScale.R , GFloatRange.R );
		TriVertex[0].Light.G = MinPositiveFloat( Point0->Light.G * GFloatScale.G , GFloatRange.G );
		TriVertex[0].Light.B = MinPositiveFloat( Point0->Light.B * GFloatScale.B , GFloatRange.B );
			
		TriVertex[1].Light.R = MinPositiveFloat( Point1->Light.R * GFloatScale.R , GFloatRange.R );
		TriVertex[1].Light.G = MinPositiveFloat( Point1->Light.G * GFloatScale.G , GFloatRange.G );
		TriVertex[1].Light.B = MinPositiveFloat( Point1->Light.B * GFloatScale.B , GFloatRange.B );

		TriVertex[2].Light.R = MinPositiveFloat( Point2->Light.R * GFloatScale.R , GFloatRange.R );
		TriVertex[2].Light.G = MinPositiveFloat( Point2->Light.G * GFloatScale.G , GFloatRange.G );
		TriVertex[2].Light.B = MinPositiveFloat( Point2->Light.B * GFloatScale.B , GFloatRange.B );
	}

	    
	FLOAT  dRdX  = (TriVertex[0].Light.R - TriVertex[2].Light.R)*SD12y - (TriVertex[1].Light.R - TriVertex[2].Light.R)*SD02y;
	FLOAT  dGdX  = (TriVertex[0].Light.G - TriVertex[2].Light.G)*SD12y - (TriVertex[1].Light.G - TriVertex[2].Light.G)*SD02y;
	FLOAT  dBdX  = (TriVertex[0].Light.B - TriVertex[2].Light.B)*SD12y - (TriVertex[1].Light.B - TriVertex[2].Light.B)*SD02y;
	MMX15Load(TriDeltaLight,dRdX,dGdX,dBdX);

	FLOAT  dRFdX = (TriVertex[0].Fog.R - TriVertex[2].Fog.R)*SD12y - (TriVertex[1].Fog.R - TriVertex[2].Fog.R)*SD02y;
	FLOAT  dGFdX = (TriVertex[0].Fog.G - TriVertex[2].Fog.G)*SD12y - (TriVertex[1].Fog.G - TriVertex[2].Fog.G)*SD02y;
	FLOAT  dBFdX = (TriVertex[0].Fog.B - TriVertex[2].Fog.B)*SD12y - (TriVertex[1].Fog.B - TriVertex[2].Fog.B)*SD02y;
	MMX14Load(TriDeltaFog,dRFdX,dGFdX,dBFdX);

	// Setup the edges, 3 different edges to rasterize.
	// Vertices always in same clockwise manner.
	// All right edges need is their X coordinate.

	unguardSlow;

	
	FMMXEdgeSetup	  MMXEdge[3]; 
	FMMXEdgeSetup*    EdgeSetupPtr;
	FMipmap* Mip	= Texture.Mips[iMip];
	INT UBits       = Mip->UBits;

	//INT TexURotate  = 32 + 4 - Mip->UBits;  // 16:16 -> 12:Vshift:fractional.
	//INT TexVRotate  = 32 + 4;    //16:16 -> 12:20.... for V, 12 :Ubits :20-Ubits for U.


	FLOAT MipVScale,MipUScale;

	// Try to avoid extra divides...
	if ( (Texture.VScale != 1.f) || (Texture.UScale != 1.f) )
	{
		FLOAT InvTexUScale,InvTexVScale;

		// Worth it to special-casing this! (Unreal low-res texture mode..)
		if ((Texture.VScale == 2.f) && (Texture.UScale == 2.f))
		{
			InvTexUScale = 0.5;
			InvTexVScale = 0.5;
		}
		else
		{
			InvTexUScale = 1.f/Texture.UScale;
			InvTexVScale = 1.f/Texture.VScale;
		}

		MipVScale = InvTexUScale * (  (65536*16) >> iMip );   //#debug rotate left: need 12:20 alignment.
		MipUScale = InvTexVScale * (1 << ( (32-12) - Mip->UBits - iMip) );
	}
	else // no special inherent texture scaling 
	{
		MipVScale = (  (65536*16) >> iMip );   //#debug rotate left: need 12:20 alignment.
		MipUScale = (1 << ( (32-12) - Mip->UBits - iMip) );
	}

	// * (1.f/ Texture.UScale);

guardSlow(FlashTriangle EdgeSetup);

	dUdX *= MipUScale; 
	dVdX *= MipVScale; 

	EdgeSetupPtr    = &MMXEdge[0];        

	FTransTexture *PA, *PB, *PC;
	TriVertexType *TA, *TB, *TC;

	TA = &TriVertex[0];
	TB = &TriVertex[1];
	TC = &TriVertex[2];
	PA = Point0;
	PB = Point1;
	PC = Point2;

	// Fill a per-edge setup structure.
	do {	

		if ( TB->YFloor == TA->YFloor )
		{
			EdgeSetupPtr->SideIndicator = 0;
			EdgeSetupPtr++;
		}
		
		if ( TB->YFloor < TA->YFloor )
		{
			FLOAT RDY     = 1.0f / ( PA->ScreenY - PB->ScreenY );
			EdgeSetupPtr->SideIndicator = 1;   // Left side
			EdgeSetupPtr->StartY = TB->YFloor;
			EdgeSetupPtr->EndY   = TA->YFloor;

			FLOAT DX      = RDY * ( PA->ScreenX - PB->ScreenX );
			FLOAT YAdj    = (FLOAT)TB->YFloor - (FLOAT)PB->ScreenY;
			FLOAT X       = PB->ScreenX + YAdj * DX;

			EdgeSetupPtr->StartX = appRound(  X * (FLOAT)0x40000); // special 14:18 bit fixed point
			EdgeSetupPtr->DeltaX = appRound( DX * (FLOAT)0x40000); // 
			
			FLOAT DU      = MipUScale * RDY * ( PA->U - PB->U );
			FLOAT DV      = MipVScale * RDY * ( PA->V - PB->V );
			FLOAT U       = MipUScale * PB->U + YAdj * DU;
			FLOAT V       = MipVScale * PB->V + YAdj * DV;

			// us a FP factor whenever that can take the place of rotation -> rotation by
			// CL is always expensive...
			EdgeSetupPtr->UV.DL  = appRound(V);
			EdgeSetupPtr->UV.DH  = appRound(U);
			EdgeSetupPtr->DUV.DL = appRound(DV);
			EdgeSetupPtr->DUV.DH = appRound(DU);

			FLOAT DR      = RDY * ( TA->Light.R - TB->Light.R );
			FLOAT DG      = RDY * ( TA->Light.G - TB->Light.G );
			FLOAT DB      = RDY * ( TA->Light.B - TB->Light.B );
			MMX15Load(EdgeSetupPtr->DeltaLightRGB, DR, DG, DB);

			FLOAT R       = TB->Light.R + YAdj * DR;
			FLOAT G       = TB->Light.G + YAdj * DG;
			FLOAT B       = TB->Light.B + YAdj * DB;
			MMX15Load(EdgeSetupPtr->LightRGB, R, G, B);

			FLOAT DFR     = RDY * (TA->Fog.R - TB->Fog.R);
			FLOAT DFB     = RDY * (TA->Fog.B - TB->Fog.B);
			FLOAT DFG     = RDY * (TA->Fog.G - TB->Fog.G);
			MMX14Load(EdgeSetupPtr->DeltaFogRGB,DFR,DFG,DFB);

			FLOAT FR	   = TB->Fog.R + YAdj * DFR;
			FLOAT FG	   = TB->Fog.G + YAdj * DFG;
			FLOAT FB	   = TB->Fog.B + YAdj * DFB;
			MMX14Load(EdgeSetupPtr->FogRGB,FR,FG,FB);


			// #debug  separate fogged TRIANGLE drawer routine for 
			// volumetric fog ? Normal fog always needed ->screenflashes.
			EdgeSetupPtr++;
		}

		// Right part only needs X.

		if ( TB->YFloor > TA->YFloor )
		{
			EdgeSetupPtr->SideIndicator = 2;	// right side

			FLOAT RDY     =  1.0f / ( PB->ScreenY - PA->ScreenY );
			FLOAT DX      =  RDY * ( PB->ScreenX - PA->ScreenX );

			FLOAT YAdj    =  TA->YFloor  -  PA->ScreenY;
			FLOAT X       =  PA->ScreenX  +  YAdj * DX; 

			EdgeSetupPtr->StartY = TA->YFloor;
			EdgeSetupPtr->EndY   = TB->YFloor;

			EdgeSetupPtr->StartX = appRound(  X * (FLOAT)0x40000 );
			EdgeSetupPtr->DeltaX = appRound( DX * (FLOAT)0x40000 );

			EdgeSetupPtr++;
		}

		TA = TB;
		TB = TC;
		TC = &TriVertex[0];

		PA = PB;
		PB = PC;
		PC = Point0;

	} while (PA != Point0);


unguardSlow;



	// Create the global pre-stepping delta's. 

	static FMMX TriDeltaUV[5]; // TriDeltaUV[0] is the regular one, 1,2,3,4  give further precalculated presteps 3/4, 2/4, 1/4, 0/4	

	guardSlow(FlashTriangle Rasterization);

	TriDeltaUV[0].DL    = appRound(dVdX); // #debug need to remain signed values
	TriDeltaUV[0].DH    = appRound(dUdX);

	// End of FPU code, MMX starts.
	__asm
	{
		MOVQ	mm0,[TriDeltaUV+0*8];
		MOVQ	mm1,mm0
		PSRAD	mm0,1
		PSUBD   mm2,mm2
		PSRAD	mm1,2
		MOVQ	[TriDeltaUV+2*8],mm0
		PADDD   mm0,mm1
		MOVQ	[TriDeltaUV+3*8],mm1
		MOVQ    [TriDeltaUV+1*8],mm0
		MOVQ    [TriDeltaUV+4*8],mm2
	}	
	
	FMMXEdgeSetup  *FA,*FB,*FC;
	FA = &MMXEdge[0];
	FB = &MMXEdge[1];
	FC = &MMXEdge[2];

	do {	

		if (FA->SideIndicator == 1) 
		{	


			__asm
			{
				mov  esi, [FA]  
				mov  edi, offset MMXSetup // MMXSetup structure index, destination 

				mov  ecx, [esi]FMMXEdgeSetup.EndY
				mov  eax, [esi]FMMXEdgeSetup.StartY

				movq mm0, [esi]FMMXEdgeSetup.UV
				movq mm1, [esi]FMMXEdgeSetup.DUV
				movq mm2, [esi]FMMXEdgeSetup.LightRGB
				movq mm3, [esi]FMMXEdgeSetup.DeltaLightRGB
				movq mm4, [esi]FMMXEdgeSetup.FogRGB
				movq mm5, [esi]FMMXEdgeSetup.DeltaFogRGB

				sub  ecx,eax
				mov  ebx, [esi]FMMXEdgeSetup.StartX

				shl  eax,5     //
				mov  edx, [esi]FMMXEdgeSetup.DeltaX

				add  ebx,edx   // X + delta X lead-in
	 			add  edi,eax   // starty * SetupSize 


			LeftEdgeLoop:
				// Using starts/delta's all in MMX registers,
				// create ECX steps and put them in [edi]FMMXPolyCSetup.UV, LightRGB, FogRGB, StartX, EndX

				paddw	mm2,mm3					// Light + delta ligh/delta edge.
				mov     eax,ebx					// 

				sar     eax,18                 
				mov     esi,ebx 

				paddw	mm4,mm5					// Fog   + delta fog/delta edge.				
				mov		[edi]FMMXPolyCSetup.StartX, eax // ACCESS violation during play: <- was an invalid (NaN) Input problem.

				paddd   mm0,mm1							// UV + duv/delta edge
				shr     esi,(18-2)						// 

				movq	[edi]FMMXPolyCSetup.LightRGB, mm2	
				movq    mm6,mm0							// Copy current 'UV'

				and     esi,0x3							// Mask to get the 2 fractional bits.
				add		ebx,edx							// X + delta X
														// Prestep UV by using the 2 highest fractional bits of eax.			
				paddd   mm6, TriDeltaUV[8+esi*8];		// Lookup gives appropriate prestepping,   3/4 2/4 1/4 0/4....
				movq	[edi]FMMXPolyCSetup.FogRGB, mm4 // Fog store.
				
				// Store UV
				dec ecx
				movq	[edi]FMMXPolyCSetup.UV, mm6		//  

				lea      edi,[edi+32]	  // add edi,32 == sizeof(FMMXPolyCSetup);
				jnz LeftEdgeLoop 
			}
		}
		else if (FA->SideIndicator == 2)// Right part only needs EndXes.
		{			


			__asm
			{	
				mov		esi, [FA]  
				mov		edi, offset MMXSetup // MMXSetup structure index, destination.

				mov		ecx, [esi]FMMXEdgeSetup.EndY
				mov		eax, [esi]FMMXEdgeSetup.StartY
				sub		ecx, eax

				mov		ebx, [esi]FMMXEdgeSetup.StartX
				mov		edx, [esi]FMMXEdgeSetup.DeltaX

				shl		eax, 5    // *32
				add		ebx, edx // X + deltaX

	 			add		edi, eax  // start at MMXPolyCSetup[edi]
				mov     eax, ebx

			RightEdgeLoop:
				sar     eax, 18
				add     ebx, edx

				mov		[edi]FMMXPolyCSetup.EndX, eax 
				add		edi, 32  // sizeof(FMMXPolyCSetup); 

				dec		ecx  
				mov     eax, ebx

				jnz		RightEdgeLoop
			}
		}

		FA = FB;
		FB = FC;
		FC = &MMXEdge[0];

	} while (FA != &MMXEdge[0]);

	unguardSlow;


	//
	// Draw and clip to spanbuffer.
	//


	/* 
	Logic:

	for( INT Y=MinY; Y<MaxY; Y++, Set++, Screen.PtrBYTE+= GByteStride )
		{
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
		
			SpanX0 = Set->StartX;  //#debug
			SpanX1 = Set->EndX;
		
			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 >= Span->Start )  // nonclipped start.
			{	
				if( SpanX1 > SpanX0 ) // any to draw ?
				{
				draw nonclipped
				}
			}
			else
			{
				if (SpanX1 > Span->Start) // any ?
				{
				draw clipped
				}
			}
		}
	*/


	// Load constant MMX delta's.
	__asm 
	{
		movq    mm6, [TriDeltaUV]	  // #debug keep these in registers during poly !
		movq    mm5, [TriDeltaLight]  //
		movq    mm3, [TriDeltaFog]
	}

	// Render, depending on bit depth.
	if (Viewport->ColorBytes==2) 
	{
		if (Viewport->Caps & CC_RGB565) 
		InnerGouraudMMX16(PolyFlags, MinY, MaxY, Frame, Mip, SpanBuffer); //#debug
		else
		InnerGouraudMMX15(PolyFlags, MinY, MaxY, Frame, Mip, SpanBuffer);
	}
	else
	{
		InnerGouraudMMX32(PolyFlags, MinY, MaxY, Frame, Mip, SpanBuffer);
	}


	unguardSlow;	// MMXFlashTriangle;
}



void USoftwareRenderDevice::PentiumFlashTriangle
(	
	FSceneNode* Frame,
	FTextureInfo& Texture, 
	DWORD PolyFlags, 
	FSpanBuffer* SpanBuffer 
)
{
/*	
    4 divides per triangle.

	Calculating texture & gouraud gradients for a triangle:
     
                     p0
                     /\ 
                    /  \ 
                   /    \ 
                  /______\ 
                p1        p2

	float d = (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
	float tridiv = 1.0/d;
	float dudx = ((p0.u - p2.u) * (p1.y - p2.y) - (p1.u - p2.u) * (p0.y - p2.y)) * tridiv;
	float dpdx = ((p0.p - p2.p) * (p1.y - p2.y) - (p1.p - p2.p) * (p0.y - p2.y)) * tridiv;

*/

	guardSlow(USoftwareRenderDevice::PentiumFlashTriangle);

	// #optimize me ! (C++ generates bad asm code here)
	// Guard against indefinite floats which got translated to Min_Int...!

	if ( ( (DWORD)Point0->IntY   == 0x80000000 ) ||
		 ( (DWORD)Point1->IntY   == 0x80000000 ) ||
		 ( (DWORD)Point2->IntY   == 0x80000000 ) )
	{  
		//debugf("Indefinite ScreenY triangle coordinates");
		return;
	}

#if DO_SLOW_GUARD
	/*
	if (! _finite( Point0->U) ||
		! _finite( Point0->V) ||		
		! _finite( Point1->U) ||
		! _finite( Point1->V) ||
		! _finite( Point2->U) ||
		! _finite( Point2->V) ) 
		appErrorf(" Non-finite texture coordinate input for PentiumFlashTriangle.");
	*/
#endif

	INT	MinY, MaxY, MinX, MaxX;
	MinX = MaxX = appFloor( Point0->ScreenX );
	MinY = MaxY = TriVertex[0].YFloor = Point0->IntY; // appFloor( Point0->ScreenY );

	INT	PX1 = appFloor( Point1->ScreenX );
	INT PY1 = TriVertex[1].YFloor = Point1->IntY;     // appFloor( Point1->ScreenY );
	MinX  = Min(MinX,PX1);
	MaxX  = Max(MaxX,PX1);
	MinY  = Min(MinY,PY1);
	MaxY  = Max(MaxY,PY1);

    INT PX2 = appFloor( Point2->ScreenX );
    INT PY2 = TriVertex[2].YFloor = Point2->IntY;     // appFloor( Point2->ScreenY );
	MinX  = Min(MinX,PX2);
	MaxX  = Max(MaxX,PX2);
	MinY  = Min(MinY,PY2);
	MaxY  = Max(MaxY,PY2);

	// Clip vertical range to spanbuffer. This *will* make MinY > MaxY in some cases.
	MinY = Max( MinY,SpanBuffer->StartY);
	MaxY = Min( MaxY,SpanBuffer->EndY);

	INT SizeX = MaxX-MinX;
	INT SizeY = MaxY-MinY;
	if ( (SizeX<1) || (SizeY<1) ) return;


	FLOAT D12y = Point1->ScreenY - Point2->ScreenY;
	FLOAT D02y = Point0->ScreenY - Point2->ScreenY;

	FLOAT TriDivisor = ( (Point0->ScreenX - Point2->ScreenX)*D12y - (Point1->ScreenX - Point2->ScreenX)*D02y);
	DWORD TriExponent;
	*(FLOAT*)&TriExponent = TriDivisor;
	
	FLOAT DivTriDelta = 1.0f / TriDivisor;
	// Refuse near-zero divisor.
	if ( (TriExponent & 0x7FFFFFFF) < ( 16 << 23) )  // 16 = lowest acceptable exponent ?
	{   
		//debugf(NAME_Log," Tridivisor underflow. "); 
		return; 
	}


	FLOAT SD12y = D12y * DivTriDelta;
	FLOAT SD02y = D02y * DivTriDelta;

	// Compute the texture mapping delta's.
	FLOAT dUdX =  (Point0->U - Point2->U)*SD12y - (Point1->U - Point2->U)*SD02y;
	FLOAT dVdX =  (Point0->V - Point2->V)*SD12y - (Point1->V - Point2->V)*SD02y;
	
	// Max exponent (dUdX ,dVdX) determines mipmap to use.
	// scaling bias by adding a number to ExpU and ExpV.
	DWORD ExpU,ExpV;
	*(FLOAT*)&ExpU = (FLOAT)dUdX;
	*(FLOAT*)&ExpV = (FLOAT)dVdX;

	// The (arbitrary) +0x200000 offset brings the mips closer since it carries
	// the mantissa over in to the exponent. Scales from 0x000000 to max 0x7f0000.

	INT MipFactor = Max(( (ExpU&0x7FFFFFFF) + 0x000000 ) >> 23, ( (ExpV & 0x7FFFFFFF) + 0x000000 ) >>23);
	INT iMip = Clamp( (MipFactor-127), 0, Texture.NumMips-1 );

	//See if Texture.Uscale and Vscale are 1.f so they can be ignored...

	FLOAT MipUScale,MipVScale;

	// Try to avoid extra divides...
	if ( (Texture.VScale != 1.f) || (Texture.UScale != 1.f) )
	{
		FLOAT InvTexUScale,InvTexVScale;

		// Worth it to special-casing this! (Unreal low-res texture mode..)
		if ((Texture.VScale == 2.f) && (Texture.UScale == 2.f))
		{
			InvTexUScale = 0.5;
			InvTexVScale = 0.5;
		}
		else
		{
			InvTexUScale = 1.f/Texture.UScale;
			InvTexVScale = 1.f/Texture.VScale;
		}

		MipUScale = InvTexUScale * (65536 >> iMip );
		MipVScale = InvTexVScale * (65536 >> iMip );
	}
	else // no special inherent texture scaling 
	{
		MipUScale = (65536 >> iMip ); 
		MipVScale = MipUScale;
	}

	dUdX *= MipUScale;
	dVdX *= MipVScale;

	TriVertex[0].Light.R = MinPositiveFloat( Point0->Light.R * GFloatScale.R , GFloatRange.R );
	TriVertex[0].Light.G = MinPositiveFloat( Point0->Light.G * GFloatScale.G , GFloatRange.G );
	TriVertex[0].Light.B = MinPositiveFloat( Point0->Light.B * GFloatScale.B , GFloatRange.B );
		
	TriVertex[1].Light.R = MinPositiveFloat( Point1->Light.R * GFloatScale.R , GFloatRange.R );
	TriVertex[1].Light.G = MinPositiveFloat( Point1->Light.G * GFloatScale.G , GFloatRange.G );
	TriVertex[1].Light.B = MinPositiveFloat( Point1->Light.B * GFloatScale.B , GFloatRange.B );

	TriVertex[2].Light.R = MinPositiveFloat( Point2->Light.R * GFloatScale.R , GFloatRange.R );
	TriVertex[2].Light.G = MinPositiveFloat( Point2->Light.G * GFloatScale.G , GFloatRange.G );
	TriVertex[2].Light.B = MinPositiveFloat( Point2->Light.B * GFloatScale.B , GFloatRange.B );
   
	FLOAT  dRdX = (TriVertex[0].Light.R - TriVertex[2].Light.R)*SD12y - (TriVertex[1].Light.R - TriVertex[2].Light.R)*SD02y;
	FLOAT  dGdX = (TriVertex[0].Light.G - TriVertex[2].Light.G)*SD12y - (TriVertex[1].Light.G - TriVertex[2].Light.G)*SD02y;
	FLOAT  dBdX = (TriVertex[0].Light.B - TriVertex[2].Light.B)*SD12y - (TriVertex[1].Light.B - TriVertex[2].Light.B)*SD02y;


	// 3 different edges to rasterize.
	// Vertices always in same clockwise manner.
	// All right edges need is their X coordinate.

	// #debug rasterization is very  non-interleaved by the compiler?

	// Setup the edges. 
	FTransTexture *PA, *PB, *PC;
	TriVertexType *TA, *TB, *TC;

	TA = &TriVertex[0];
	TB = &TriVertex[1];
	TC = &TriVertex[2];

	PA = Point0;
	PB = Point1;
	PC = Point2;

	do {	
		if( TB->YFloor != TA->YFloor )
		{
			if ( TB->YFloor < TA->YFloor )
			{
				FTexSetup* Set =  Setup + TB->YFloor;

				FLOAT RDY     = 1.0f / ( PA->ScreenY - PB->ScreenY );
				FLOAT DX      =  RDY * ( PA->ScreenX - PB->ScreenX );

				FLOAT YAdj    = TB->YFloor - PB->ScreenY;
				FLOAT X       = PB->ScreenX  + YAdj * DX;

				FLOAT DU      = MipUScale * RDY * (PA->U - PB->U);
				FLOAT DV      = MipVScale * RDY * (PA->V - PB->V);

				// Y-direction prestep
				FLOAT U       = MipUScale * PB->U + YAdj * DU;
				FLOAT V       = MipVScale * PB->V + YAdj * DV;

				FLOAT DR      = RDY * (TA->Light.R - TB->Light.R);
				FLOAT DB      = RDY * (TA->Light.B - TB->Light.B);
				FLOAT DG      = RDY * (TA->Light.G - TB->Light.G);

				FLOAT R       = TB->Light.R + YAdj * DR; // prestepping required here to prevent overflow.
				FLOAT G       = TB->Light.G + YAdj * DG; 
				FLOAT B       = TB->Light.B + YAdj * DB;

				DWORD Count = TA->YFloor - TB->YFloor; // Guaranteed positive.
				do
				{
					//FLOAT ThisX = (X+=DX);
					//Set->X = appFloor(ThisX);

					Set->X = appFloor(X += DX);
					Set->R = R+=DR;
					Set->G = G+=DG;
					Set->B = B+=DB;

					// BUG SCREENY delivered was #ind / NAN - not a number !
					// one case: from SubSurface->SubSurface...
					// and Point0->ScreenY was -1.#IND00e+000
					FLOAT AdjX = Set->X + 1 - X;  
					// Prestep U and V

					Set->U = ((U+=DU) + dUdX*AdjX);  // + (3<<24); // prestepping 
					Set->V = ((V+=DV) + dVdX*AdjX);  // + (3<<24);

					Set++;
				} while( --Count !=0 );

			}
			else // Right part only needs X.
			{
				FTexSetup* Set =  Setup + TA->YFloor;

				FLOAT RDY     =  1.0f / ( PB->ScreenY - PA->ScreenY );
				FLOAT DX      =  RDY * ( PB->ScreenX - PA->ScreenX );
				FLOAT YAdj    =  TA->YFloor - PA->ScreenY;
				FLOAT X       =  PA->ScreenX  + YAdj * DX;

				// Y-direction prestep.
				INT Count = TB->YFloor - TA->YFloor;
				do
				{	
					Set->EndX = appFloor( X+=DX ); 
					Set++;
				} while( --Count != 0 );
			}
		}

		TA = TB;
		TB = TC;
		TC = &TriVertex[0];

		PA = PB;
		PB = PC;
		PC = Point0;

	} while (PA != Point0);


	// Setup.
	// INT     iMip    = 0;
	FMipmap* Mip	= Texture.Mips[iMip];
	GPolyC.TexBase	= Mip->DataPtr;
	GPolyC.UBits	= Mip->UBits;
	DWORD DHMask    = ( Mip->VSize-1 );
	GPolyC.MaskUV	= ( Mip->VSize-1 )  +  ((0xffff0000) << (16-(DWORD)Mip->UBits));
	
	DWORD DU,DV;
	*(INT*)&DU = appRound(dUdX);
	*(INT*)&DV = appRound(dVdX);

	// GPolyC.DUV	  = ((QWORD)(DV & 0xffffff) << 18) + ((QWORD)DU << (50-Mip->UBits));
	DWORD DVRotated = _rotl(DV, 16);
	GPolyC.DUV.DL   = DVRotated;
	GPolyC.DUV.DH   = ( ( DU << (16 - Mip->UBits)) & ~DHMask) + (DVRotated & DHMask);

	GPolyC.Ptr      = Frame->Screen(0,MinY);
	FTexSetup* Set  = Setup + MinY;

	GDontMaskThePolygon = !(PolyFlags & (PF_Masked|PF_Translucent|PF_Modulated));
	GTranslucent		=   PolyFlags & PF_Translucent;
	GModulated			=   PolyFlags & PF_Modulated;

	// Figure out drawing function.
	void(*Draw)() = NULL;

	if( (Viewport->ColorBytes==2) && (Viewport->Caps & CC_RGB565) )
	{
		if ( ! (PolyFlags &(PF_Masked|PF_Translucent|PF_Modulated)) )
			Draw = PentiumPolyC16Normal;
		else if ( PolyFlags & PF_Translucent)
			Draw = PentiumPolyC16Translucent;
		else if ( PolyFlags & PF_Modulated)
			Draw = PentiumPolyC16Modulated;
		else if ( PolyFlags & PF_Masked)
			Draw = PentiumPolyC16Masked;

		if (PolyFlags&(PF_Translucent|PF_Modulated) )
		{
			GPolyC.FlashR	= (3<<22);  // Built-in alignment forcing factor.
			GPolyC.FlashG	= (3<<22);  // No fog offset for translucency.
			GPolyC.FlashB	= (3<<22);  //
		}
		else
		{
			GPolyC.FlashR	= (3<<22) + (GFloatFog.R* 32.0f)*(1<<11); // >> 3); // Built-in alignment forcing factor + fog offset.
			GPolyC.FlashG	= (3<<22) + (GFloatFog.G* 64.0f)*(1<< 5); // >> 2); //
			GPolyC.FlashB	= (3<<22) + (GFloatFog.B* 32.0f);		  // >> 3); //
		}
	}
	else if ( Viewport->ColorBytes==2 )
	{
		if ( ! (PolyFlags &(PF_Masked|PF_Translucent|PF_Modulated)) )
			Draw = PentiumPolyC15Normal;
		else if ( PolyFlags & PF_Translucent)
			Draw = PentiumPolyC15Translucent;
		else if ( PolyFlags & PF_Modulated)
			Draw = PentiumPolyC15Modulated;
		else if ( PolyFlags & PF_Masked)
			Draw = PentiumPolyC15Masked;

		if (PolyFlags&(PF_Translucent|PF_Modulated) )
		{
 			GPolyC.FlashR	= (3<<22);  // Built-in alignment forcing factor.
			GPolyC.FlashG	= (3<<22);  // No fog offset for translucency.
			GPolyC.FlashB	= (3<<22);  // 
		}
		else
		{
			GPolyC.FlashR	= (FLOAT)(3<<22) + (GFloatFog.R* 32.0f)*(1<<10); // >> 3);  // Built-in alignment forcing factor + fog offset.
			GPolyC.FlashG	= (FLOAT)(3<<22) + (GFloatFog.G* 32.0f)*(1<<5);  // >> 3);  // 
			GPolyC.FlashB	= (FLOAT)(3<<22) + (GFloatFog.B* 32.0f);         // >> 3);  //
		}
	}
	else if( Viewport->ColorBytes==4 )
	{
		if ( ! (PolyFlags &(PF_Masked|PF_Translucent|PF_Modulated)) )
			Draw = PentiumPolyC32Normal;
		else if ( PolyFlags & PF_Translucent)
			Draw = PentiumPolyC32Translucent;
		else if ( PolyFlags & PF_Modulated)
			Draw = PentiumPolyC32Modulated;
		else if ( PolyFlags & PF_Masked)
			Draw = PentiumPolyC32Masked;

		if (PolyFlags&(PF_Translucent|PF_Modulated) )
		{
			GPolyC.FlashR	= (3<<22);  // Built-in alignment forcing factor.
			GPolyC.FlashG	= (3<<22);  // No fog offset for translucency.
			GPolyC.FlashB	= (3<<22);  //
		}
		else
		{
			GPolyC.FlashR	= (FLOAT)(3<<22) + GFloatFog.R * 256.0f;  // Built-in alignment forcing factor + fog offset.
			GPolyC.FlashG	= (FLOAT)(3<<22) + GFloatFog.G * 256.0f;  //
			GPolyC.FlashB	= (FLOAT)(3<<22) + GFloatFog.B * 256.0f;  //

		}
	}

	// Draw.

	GPolyC.DR  = dRdX;
	GPolyC.DG  = dGdX;
	GPolyC.DB  = dBdX;

	// Always clip to spanbuffer. Usually, no clipping required.
	FSpan** Index   = SpanBuffer->Index + MinY - SpanBuffer->StartY;

	DWORD UShift = 16-Mip->UBits;

	for( INT Y=MinY; Y<MaxY; Y++,Set++, GPolyC.Ptr.PtrBYTE+= GByteStride )
	{
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{
			// Set up the start coords.
			GPolyC.X0 = Set->X;
			GPolyC.X1 = Set->EndX;
			
			if( GPolyC.X1 > Span->End )
				GPolyC.X1 = Span->End;
			
			if( GPolyC.X0 >= Span->Start )
			{	
				// Default case: draw span, original start position.
				if( GPolyC.X1 > GPolyC.X0 )
				{
					GPolyC.R		= Set->R;
					GPolyC.G		= Set->G;
					GPolyC.B		= Set->B;
					DWORD U			= appRound( Set->U );			
					DWORD V			= appRound( Set->V );
					DWORD VRotated  = _rotl(V, 16);
					GPolyC.UV.DL    = VRotated; 
					GPolyC.UV.DH    = (( U << UShift) & ~DHMask) + (VRotated & DHMask); 
					Draw();									
				}
			}			
			else
			{
				GPolyC.X0 = Span->Start;
				// Draw span with adjusted start position.
				if( GPolyC.X1 > GPolyC.X0 )
				{					
				// Always integer prestep..
					FLOAT XStep		= GPolyC.X0 - Set->X;  
					GPolyC.R		= Set->R  + XStep*dRdX;
					GPolyC.G		= Set->G  + XStep*dGdX;
					GPolyC.B		= Set->B  + XStep*dBdX;
					DWORD U			= appRound( Set->U + XStep*dUdX );			
					DWORD V			= appRound( Set->V + XStep*dVdX );
					DWORD VRotated  = _rotl(V, 16);
					GPolyC.UV.DL    = VRotated; 
					GPolyC.UV.DH    = (( U << UShift) & ~DHMask) + (VRotated & DHMask); 
					Draw();
				}
			}			
		}
	}

	unguardSlow;
}


/*-----------------------------------------------------------------------------
	Polygon drawing routine.
-----------------------------------------------------------------------------*/

void USoftwareRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Texture, FTransTexture** Pts, INT NumPts, DWORD PolyFlags, FSpanBuffer* SpanBuffer )

{

	guardSlow(USoftwareRenderDevice::DrawGouraudPolygon);

	// Screen Y size not supported if bigger than our setup buffer size:
	if( Viewport->SizeY > MaximumYScreenSize ) return;

	//#debug 
	/*
	if (PolyFlags & PF_Translucent) 
	{
		PolyFlags &= ~PF_Translucent;
		PolyFlags |=  PF_Masked;
	}
	*/

	guardSlow(SpanBufferPrepare);
	// Ensure we have a valid spanbuffer
	if ( !SpanBuffer ) 
	{
		static INT SavedX=0, SavedY=0;
		static FSpanBuffer TempSpanBuffer;
		static FSpan *SpanIndex[MaximumYScreenSize], DefaultSpan;
		if( SavedX!=Viewport->SizeX || SavedY!=Viewport->SizeY )
		{
			SavedX = Viewport->SizeX;
			SavedY = Viewport->SizeY;
			TempSpanBuffer.Index  = SpanIndex;
			TempSpanBuffer.StartY = 0;
			TempSpanBuffer.EndY   = Viewport->SizeY;
			DefaultSpan.Start = 0;
			DefaultSpan.End   = Viewport->SizeX;
			DefaultSpan.Next  = 0;
			for( INT i=0; i<Viewport->SizeY; i++ ) SpanIndex[i]  = &DefaultSpan;
		}
		SpanBuffer = &TempSpanBuffer;		
	}
	unguardSlow;



	//
	// Extra check to test validity of spanbuffer input.
	//

#if DO_SLOW_GUARD
	{
		// Walk over the whole spanbuffer and check the sizes against the viewport.
		//
		// Other things that *MIGHT* go wrong:
		// - triangle vertices are outside the screenbounds, and read outside the span->index
		//   when looking into the span for clipping ? (a maxY/MinY problem)
		// - some floating-point dependency ?????   
		//

		/*
		INT MinY = SpanBuffer->StartY;
		INT MaxY = SpanBuffer->EndY;

		FSpan** Index   = SpanBuffer->Index;

		if (MinY < 0) appErrorf(" SpanStartY negative!");
		if (MaxY > Viewport->SizeY) appErrorf(" SpanEndY bigger than ViewportSizeY!");

		for( INT Y=MinY; Y<MaxY; Y++ )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT X0 = Span->Start;
				INT X1 = Span->End;	

				if ( (X0 <0) || ( X1 > Viewport->SizeX))
					appErrorf( "Span out of bounds. Ypos: %i XStart: %i XEnd: %i",Y, Span->Start,Span->End);
			}
		}
		*/
	}
#endif



	// Use cached float/MMX palette or create it.
	FCacheItem* Item; 

	guardSlow(PreparePalette);

	if ( GIsMMX ) 
	{
		guardSlow(USoftwareRenderDevice::SetupMMXPalette);

		QWORD CacheID = (Texture.PaletteCacheID&~(QWORD)255) | CID_PolyMMXPalette;
		// Viewport ByteID: not needed here, MMX palette can stay same for different bit depths.

		MMXColors = (FMMX*)GCache.Get( CacheID, Item );
		if( !MMXColors )
		{
			// Debugf(NAME_Log,"NEW PALETTE");
			MMXColors = (FMMX*)GCache.Create( CacheID, Item, NUM_PAL_COLORS * sizeof(FMMX) );
			// MMXColors = (FMMX*)GCache.Create( CacheID, Item, NUM_PAL_COLORS * sizeof(DWORD) );
		
			for( int i=0; i<NUM_PAL_COLORS; i++ )
			{   
				// Promote 8-bit palette to MMX packed signed words 15:15:15:15 format.
				MMXColors[i].R = (INT)Texture.Palette[i].B << 7;
				MMXColors[i].G = (INT)Texture.Palette[i].G << 7;
				MMXColors[i].B = (INT)Texture.Palette[i].R << 7;
				MMXColors[i].A = 0;
				// #debug do this faster - all at once ???
				// *(((DWORD*)&MMXColors[0])+i) = ( ( *(DWORD*)&Texture.Palette[i]) >>1 ) & 0x7F7F7F7F;
			}			
		}
		unguardSlow;

	}
	else  // Non-MMX palettes
	{
		// Cached palette.
		QWORD CacheID = (Texture.PaletteCacheID&~(QWORD)255) + CID_PolyPalette + 65536 * Viewport->ByteID();
		// Viewport ByteID here ensures different color depths' palettes get different cache id's.

		FloatPalBase = (FLOAT*)GCache.Get(CacheID,Item);

		if( !FloatPalBase )
		{
			FloatPalBase = (FLOAT*)GCache.Create(CacheID,Item,NUM_PAL_COLORS * 4 * sizeof(FLOAT));
			FColor* Colors  = Texture.Palette;

				
			if( Viewport->ColorBytes==2 && (Viewport->Caps & CC_RGB565) )
			{
				for( int i=0; i<NUM_PAL_COLORS; i++ )
				{
					FloatPalBase[i*4+0] = Colors[i].R * (FLOAT)(0x10000 / 256.0);
					FloatPalBase[i*4+1] = Colors[i].G * (FLOAT)(0x00800 / 256.0);
					FloatPalBase[i*4+2] = Colors[i].B * (FLOAT)(0x00020 / 256.0);
				}
			}
			else if( Viewport->ColorBytes==2 )
			{
				for( int i=0; i<NUM_PAL_COLORS; i++ )
				{
					FloatPalBase[i*4+0] = Colors[i].R * (FLOAT)(0x08000 / 256.0);
					FloatPalBase[i*4+1] = Colors[i].G * (FLOAT)(0x00400 / 256.0);
					FloatPalBase[i*4+2] = Colors[i].B * (FLOAT)(0x00020 / 256.0);
				}
			}
			else if( Viewport->ColorBytes==4 )
			{
				for( int i=0; i<NUM_PAL_COLORS; i++ )
				{
					FloatPalBase[i*4+0] = Colors[i].R;
					FloatPalBase[i*4+1] = Colors[i].G;
					FloatPalBase[i*4+2] = Colors[i].B;
				}
			}
		}
	}

	unguardSlow; //preparepalette


	// Split all polygons into triangles so all delta's can be constant.


	guardSlow(CallPolyDrawing);
	
	typedef void (USoftwareRenderDevice::*DrawTrianglePtr)(FSceneNode* Frame,FTextureInfo& Texture, DWORD PolyFlags, FSpanBuffer* SpanBuffer);

	DrawTrianglePtr TriangleFunc;

	if (GIsMMX)
		TriangleFunc =&USoftwareRenderDevice::MMXFlashTriangle;
		else
		TriangleFunc = &USoftwareRenderDevice::PentiumFlashTriangle;

	Point0 = Pts[0];
	Point1 = Pts[1]; 
	Point2 = Pts[2]; 

	(this->*TriangleFunc)(
		Frame,
		Texture, 
		PolyFlags, 
		SpanBuffer
		);	


	if (NumPts > 3) 
	{
		Point1 = Pts[2];
		Point2 = Pts[3];

		(this->*TriangleFunc)(
			Frame,
			Texture, 
			PolyFlags, 
			SpanBuffer
			);	

		if (NumPts > 4) 
		{
			Point1 = Pts[3];
			Point2 = Pts[4];			

			(this->*TriangleFunc)(
				Frame,
				Texture, 
				PolyFlags, 
				SpanBuffer
				);	

			if (NumPts > 5) 
			{
				Point1 = Pts[4];
				Point2 = Pts[5];

				(this->*TriangleFunc)(
					Frame,
					Texture, 
					PolyFlags, 
					SpanBuffer
					);	

				if (NumPts == 7) 
				{
					Point1 = Pts[5];
					Point2 = Pts[6];

					(this->*TriangleFunc)(
						Frame,
						Texture, 
						PolyFlags, 
						SpanBuffer
						);	
				}
			}
		}
	}
	
	unguardSlow;
	
	Item->Unlock();
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	The end.
-----------------------------------------------------------------------------*/
