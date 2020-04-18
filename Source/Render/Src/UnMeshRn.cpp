/*=============================================================================
	UnMeshRn.cpp: Unreal mesh rendering.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "RenderPrivate.h"

/*------------------------------------------------------------------------------
	Globals.
------------------------------------------------------------------------------*/

UBOOL               HasSpecialCoords;
FCoords             SpecialCoords;
static FLOAT        UScale, VScale;
static UTexture*    Textures[16];
static FTextureInfo TextureInfo[16];
static FTextureInfo EnvironmentInfo;
static FVector      GUnlitColor;

/*------------------------------------------------------------------------------
	Environment mapping.
------------------------------------------------------------------------------*/

static void EnviroMap( FSceneNode* Frame, FTransTexture& P )
{
	FVector T = P.Point.UnsafeNormal().MirrorByVector( P.Normal ).TransformVectorBy( Frame->Uncoords );
	P.U = (T.X+1.0) * 0.5 * 256.0 * UScale;
	P.V = (T.Y+1.0) * 0.5 * 256.0 * VScale;
}

/*--------------------------------------------------------------------------
	Clippers.
--------------------------------------------------------------------------*/

static FLOAT Dot[32];
static inline INT Clip( FSceneNode* Frame, FTransTexture** Dest, FTransTexture** Src, INT SrcNum )
{
	INT DestNum=0;
	for( INT i=0,j=SrcNum-1; i<SrcNum; j=i++ )
	{
		if( Dot[j]>=0.0 )
		{
			Dest[DestNum++] = Src[j];
		}
		if( Dot[j]*Dot[i]<0.0 )
		{
			FTransTexture* T = Dest[DestNum] = New<FTransTexture>(GMem);
			*T = FTransTexture( *Src[j] + (*Src[i]-*Src[j]) * (Dot[j]/(Dot[j]-Dot[i])) );
			T->Project( Frame );
			DestNum++;
		}
	}
	return DestNum;
}

/*------------------------------------------------------------------------------
	Subsurface rendering.
------------------------------------------------------------------------------*/

// Triangle subdivision table.
static const int CutTable[8][4][3] =
{
	{{0,1,2},{9,9,9},{9,9,9},{9,9,9}},
	{{0,3,2},{2,3,1},{9,9,9},{9,9,9}},
	{{0,1,4},{4,2,0},{9,9,9},{9,9,9}},
	{{0,3,2},{2,3,4},{4,3,1},{9,9,9}},
	{{0,1,5},{5,1,2},{9,9,9},{9,9,9}},
	{{0,3,5},{5,3,1},{1,2,5},{9,9,9}},
	{{0,1,4},{4,2,5},{5,0,4},{9,9,9}},
	{{0,3,5},{3,1,4},{5,4,2},{3,4,5}}
};

void RenderSubsurface
(
	FSceneNode*		Frame,
	FTextureInfo&	Texture,
	FSpanBuffer*	Span,
	FTransTexture**	Pts,
	DWORD			PolyFlags,
	INT				SubCount
)
{
	guard(RenderSubsurface);

	// Handle effects.z
	if( PolyFlags & (PF_Environment | PF_Unlit) )
	{
		// Environment mapping.
		if( PolyFlags & PF_Environment )
			for( INT i=0; i<3; i++ )
				EnviroMap( Frame, *Pts[i] );

		// Handle unlit.
		if( PolyFlags & PF_Unlit )
			for( int j=0; j<3; j++ )
				Pts[j]->Light = GUnlitColor;
	}

	// Handle subdivision.
	if( SubCount<3 && !(PolyFlags & PF_Flat) )
	{
		// Compute side distances.
		INT CutSide[3], Cuts=0;
		FLOAT Alpha[3];
		STAT(clock(GStat.MeshSubTime));
		for( INT i=0,j=2; i<3; j=i++ )
		{
			FLOAT Dist   = FDistSquared(Pts[j]->Point,Pts[i]->Point);
			FLOAT Curvy  = (Pts[j]->Normal ^ Pts[i]->Normal).SizeSquared();
			FLOAT Thresh = 50.0 * Frame->FX * SqrtApprox(Dist * Curvy) / Max(1.f, Pts[j]->Point.Z + Pts[i]->Point.Z);
			Alpha[j]     = Min( Thresh / Square(32.f) - 1.f, 1.f );
			CutSide[j]   = Alpha[j]>0.0;
			Cuts        += (CutSide[j]<<j);
		}
		STAT(unclock(GStat.MeshSubTime));

		// See if it should be subdivided.
		if( Cuts )
		{
			STAT(clock(GStat.MeshSubTime));
			FTransTexture Tmp[3];
			Pts[3]=Tmp+0;
			Pts[4]=Tmp+1;
			Pts[5]=Tmp+2;
			for( INT i=0,j=2; i<3; j=i++ ) if( CutSide[j] )
			{
				// Compute midpoint.
				FTransTexture& MidPt = *Pts[j+3];
				MidPt = (*Pts[j]+*Pts[i])*0.5;

				// Compute midpoint normal.
				MidPt.Normal = Pts[j]->Normal + Pts[i]->Normal;
				MidPt.Normal *= DivSqrtApprox( MidPt.Normal.SizeSquared() );

				// Enviro map it.
				if( PolyFlags & PF_Environment )
				{
					FLOAT U=MidPt.U, V=MidPt.V;
					EnviroMap( Frame, MidPt );
					MidPt.U = U + (MidPt.U - U)*Alpha[j];
					MidPt.V = V + (MidPt.V - V)*Alpha[j];
				}

				// Shade the midpoint.
				MidPt.Light += (GLightManager->Light( MidPt, PolyFlags ) - MidPt.Light) * Alpha[j];

				// Curve the midpoint.
				(FVector&)MidPt
				+=	0.15
				*	Alpha[j]
				*	(FVector&)MidPt.Normal
				*	SqrtApprox
					(
						(Pts[j]->Point  - Pts[i]->Point ).SizeSquared()
					*	(Pts[i]->Normal ^ Pts[j]->Normal).SizeSquared()
					);

				// Outcode and optionally transform midpoint.
				MidPt.ComputeOutcode( Frame );
				MidPt.Project( Frame );
			}
			FTransTexture* NewPts[6];
			for( i=0; i<4 && CutTable[Cuts][i][0]!=9; i++ )
			{
				for( INT j=0; j<3; j++ )
					NewPts[j] = Pts[CutTable[Cuts][i][j]];
				RenderSubsurface( Frame, Texture, Span, NewPts, PolyFlags, SubCount+1 );
			}
			STAT(unclock(GStat.MeshSubTime));
			return;
		}
	}

	// If outcoded, skip it.
	if( Pts[0]->Flags & Pts[1]->Flags & Pts[2]->Flags )
		return;

	// Backface reject it.
	if( (PolyFlags & PF_TwoSided) && FTriple(Pts[0]->Point,Pts[1]->Point,Pts[2]->Point) <= 0.0 )
	{
		if( !(PolyFlags & PF_TwoSided) )
			return;
		Exchange( Pts[2], Pts[0] );
	}

	// Clip it.
	INT NumPts=3;
	BYTE AllCodes = Pts[0]->Flags | Pts[1]->Flags | Pts[2]->Flags;
	if( AllCodes )
	{
		if( AllCodes & FVF_OutXMin )
		{
			static FTransTexture* LocalPts[8];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = Frame->PrjXM * Pts[i]->Point.Z + Pts[i]->Point.X;
			NumPts = Clip( Frame, LocalPts, Pts, NumPts );
			if( NumPts==0 ) return;
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutXMax )
		{
			static FTransTexture* LocalPts[8];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = Frame->PrjXP * Pts[i]->Point.Z - Pts[i]->Point.X;
			NumPts = Clip( Frame, LocalPts, Pts, NumPts );
			if( NumPts==0 ) return;
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutYMin )
		{
			static FTransTexture* LocalPts[8];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = Frame->PrjYM * Pts[i]->Point.Z + Pts[i]->Point.Y;
			NumPts = Clip( Frame, LocalPts, Pts, NumPts );
			if( NumPts==0 ) return;
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutYMax )
		{
			static FTransTexture* LocalPts[8];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = Frame->PrjYP * Pts[i]->Point.Z - Pts[i]->Point.Y;
			NumPts = Clip( Frame, LocalPts, Pts, NumPts );
			if( NumPts==0 ) return;
			Pts = LocalPts;
		}
		if( Frame->NearClip.W != 0.0 )
		{
			UBOOL Clipped=0;
			for( INT i=0; i<NumPts; i++ )
			{
				Dot[i] = Frame->NearClip.PlaneDot(Pts[i]->Point);
				Clipped |= (Dot[i]<0.0);
			}
			if( Clipped )
			{
				static FTransTexture* LocalPts[8];
				NumPts = Clip( Frame, LocalPts, Pts, NumPts );
				if( NumPts==0 ) return;
				Pts = LocalPts;
			}
		}
	}

	for( INT i=0; i<NumPts; i++ )
	{
		//Pts[i]->ScreenX = Clamp(Pts[i]->ScreenX,0.f,Frame->FX);
		//Pts[i]->ScreenY = Clamp(Pts[i]->ScreenY,0.f,Frame->FY);
		ClipFloatFromZero(Pts[i]->ScreenX, Frame->FX);
		ClipFloatFromZero(Pts[i]->ScreenY, Frame->FY);
	}

	// Render it.
	STAT(clock(GStat.MeshTmapTime));
	Frame->Viewport->RenDev->DrawGouraudPolygon( Frame, Texture, Pts, NumPts, PolyFlags, Span );
	STAT(unclock(GStat.MeshTmapTime));
	STAT(GStat.MeshSubCount++);

	unguard;
}

/*------------------------------------------------------------------------------
	High level mesh rendering.
------------------------------------------------------------------------------*/

//
// Structure used by DrawMesh for sorting triangles.
//
struct FMeshTriSort
{
	FMeshTri* Tri;
	INT Key;
};
INT Compare( const FMeshTriSort& A, const FMeshTriSort& B )
{
	return B.Key - A.Key;
}
INT Compare( const FTransform* A, const FTransform* B )
{
	return appRound(B->Point.Z - A->Point.Z);
}

#ifndef NOAMD3D
#pragma pack( 8 )
#pragma warning( disable : 4799 )

#define MAKE_DWORD(Val) *((DWORD *)&Val)
#define MAKE_FP(Val) *((float *)&Val)

static _inline void DoFemms(void)
{
	_asm femms
}

static _inline void DoBreak(void)
{
	_asm int 3
}

static _inline UBOOL CheckFloat(FLOAT A,FLOAT B)
{
	_asm femms;
	UBOOL RetVal=(Max(A-B,B-A)>0.001);
	_asm femms;
	return RetVal;
}

// Unions containing QWORDS are used here so the compiler will align them properly.
static union
{
	DWORD d[2];
	double q;
} Negator={0x80000000,0x80000000},PlusMask={FVF_OutXMax,FVF_OutYMax},MinusMask={FVF_OutXMin,FVF_OutYMin};

//
// K6 3D Optimized version of the ComputeOutcode loop of URender::DrawMesh
// 
static _inline BYTE DoComputeOutcodeLoop(FSceneNode* Frame,FTransTexture* Samples,INT FrameVerts)
{
#if 1
	// Unions containing QWORDS are used here so the compiler will align them properly.
	static float fMinusOne=-1.0f;

	BYTE Outcode;

	_asm
	{
		// Get ready for loop
		mov		eax,FVF_OutReject
		mov		ebx,Samples
		mov		ecx,FrameVerts
		mov		edx,Frame
		cmp		ecx,0
		mov		edi,fMinusOne
		jz		Done

		// Loop through vertices, doing clip testing.
TopLoop:
		movd	mm0,[ebx]FOutVector.Point.Z		// mm0=0|Z, mm1=PrjYP|PrjXP
		movq	mm1,[edx]FSceneNode.PrjXP
		punpckldq mm0,mm0						// mm0=Z|Z, mm2=PrjYM|PrjXM
		movq	mm2,[edx]FSceneNode.PrjXM
		pfmul	(m1,m0)							// mm1=Zyp|Zxp, mm3=Y|X
		movq	mm3,[ebx]FOutVector.Point.X
		pfmul	(m2,m0)							// mm2=Zym|Zxm, mm4=Negator
		movq	mm4,Negator
		pfcmpge	(m1,m3)							// mm1=inv(+val), mm5=+mask
		movq	mm5,PlusMask
		pxor	mm4,mm2							// mm4=-Zym|-Zxm, mm6=-mask
		movq	mm6,MinusMask
		pfcmpgt	(m4,m3)							// mm4=inv(-val) mm5=Y+c|X+c
		pandn	mm1,mm5
		mov		[ebx]FTransSample.Light/*.R*/,edi // Set Light.R=-1, mm6=mm6=Y-c|X-c
		pand	mm6,mm4							
		add		ebx,TYPE FTransTexture			// Increment pointer, mm6=Yc|Xc
		por		mm6,mm1
		movq	mm7,mm6							// mm7=Yc|Xc, mm6=0|Yc
		psrlq	mm6,32
		por		mm7,mm6							// mm7=Yc|ClipVal, save clip val
		movd	[ebx-TYPE FTransTexture]FOutVector.Flags,mm7
		and		al,[ebx-TYPE FTransTexture]FOutVector.Flags	// Combine clip val in eax, loop
		loop	TopLoop
Done:
		// Save return value.
		mov		Outcode,al
		femms	// Just in case.
	}
	return Outcode;
#else
	BYTE Outcode = FVF_OutReject;
	for( INT i=0; i<FrameVerts; i++ )
	{
		// Set this so we know this point has not been lit yet.
		Samples[i].Light.R = -1;
		// Compute the outcode.
		static FLOAT ClipXM, ClipXP, ClipYM, ClipYP;
		static const BYTE OutXMinTab [2] = { 0, FVF_OutXMin };
		static const BYTE OutXMaxTab [2] = { 0, FVF_OutXMax };
		static const BYTE OutYMinTab [2] = { 0, FVF_OutYMin };
		static const BYTE OutYMaxTab [2] = { 0, FVF_OutYMax };
		ClipXM = Frame->PrjXM * Samples[i].Point.Z + Samples[i].Point.X;
		ClipXP = Frame->PrjXP * Samples[i].Point.Z - Samples[i].Point.X;
		ClipYM = Frame->PrjYM * Samples[i].Point.Z + Samples[i].Point.Y;
		ClipYP = Frame->PrjYP * Samples[i].Point.Z - Samples[i].Point.Y;
		Samples[i].Flags  =
		(	OutXMinTab [ClipXM < 0.0]
		+	OutXMaxTab [ClipXP < 0.0]
		+	OutYMinTab [ClipYM < 0.0]
		+	OutYMaxTab [ClipYP < 0.0]);
		// Combine with existing flags.
		Outcode &= Samples[i].Flags;
	}
	return Outcode;
#endif
}

//
// K6 3D optimized version of:
//		FLOAT Unlit  = Clamp( Owner->ScaleGlow*0.5f + Owner->AmbientGlow/256.f, 0.f, 1.f );
//		GUnlitColor  = FVector( Unlit, Unlit, Unlit );
//		if( GIsEditor && (ExtraFlags & PF_Selected) )
//			GUnlitColor = GUnlitColor*0.4 + FVector(0.0,0.6,0.0);
//
// NOT PERFORMANCE CRITICAL - DONE TO REMOVE MMX->FPU->MMX SWITCH.
//
static _inline void DoCalcUnlitColor(AActor* Owner,FVector& DestColor,UBOOL IsEditorAndSelected)
{
	static float PointFive=0.5f;
	static float Inv256=1.0f/256.0f;
	static float One=1.0f;
	static float PointFour=0.4f;
	static float PointSix=0.6f;

	_asm
	{
		// Calculate clamped Unlit intensity value.
		mov		eax,Owner						// eax=ptr to Owner
		movzx	ecx,[eax]AActor.AmbientGlow		// ecx=(BYTE)AmbientGlow
		movd	mm0,PointFive					// mm0=0|0.5
		movd	mm2,[eax]AActor.ScaleGlow		// mm2=0|ScaleGlow
		pfmul	(m2,m0)							// mm2=0|ScaleGlow*0.5
		movd	mm1,Inv256						// mm1=0|1/256
		movd	mm3,ecx							// mm3=0|(int)AmbientGlow
		pxor	mm4,mm4							// mm4=0|0
		pi2fd	(m3,m3)							// mm3=0|AmbientGlow
		movd	mm5,One							// mm5=0|1.0
		pfmul	(m3,m1)							// mm3=0|AmbientGlow/256
		movd	mm6,One							// mm6=0|1.0
		pfadd	(m3,m2)							// mm3=0|ScaleGlow*0.5 + AmbientGlow/256=Val
		pfcmpgt	(m4,m3)							// mm4=?|inv(<0)mask
		pfcmpge	(m5,m3)							// mm5=?|(>1)mask
		pandn	mm4,mm3							// mm4=?|Max(0,Val)
		pand	mm4,mm5							// mm4=?|Val, Val<=1.0; 0 otherwise
		pandn	mm5,mm6							// mm5=?|0  , Val<=1.0; 1 otherwise
		por		mm5,mm4							// mm5=?|Clamp(Val,0,1)
		movq	mm7,mm5							// mm7=?|Clamp(Val,0,1)

		// See if we need to do more calculations.
		mov		eax,IsEditorAndSelected
		cmp		eax,0
		jz		Done

		// Do some more stuff if this is an object selected in the editor.
		movd	mm3,PointFour					// mm3=0|0.4
		pfmul	(m5,m3)							// mm5=0|0.4*Clamp(Val,0,1)
		movd	mm7,PointSix					// mm7=0|0.6
		pfadd	(m7,m5)							// mm7=0|0.4*Clamp(Val,0,1)+0.6

Done:
		// Save new GUnlitColor value.
		mov		ecx,DestColor
		movd	[ecx]FVector.R,mm5
		movd	[ecx]FVector.G,mm7
		movd	[ecx]FVector.B,mm5

		femms	// Just in case.
	}
}

//
// K6 3D optmized version of:
//			Vert.Project( Frame );
//
static _inline void	DoProjection(FSceneNode* Frame,FTransSample& Vert)
{
#if 1
	_asm
	{
		mov		eax,Vert					// eax=ptr to Vert, ebx=ptr to Frame
		mov		ecx,Frame
		movd	mm0,[eax]FOutVector.Point.Z	// mm0=0|Z, mm1=0|Zp
		movd	mm1,[ecx]FSceneNode.Proj.Z
		pfrcp	(m0,m0)						// mm0=1/Z|1/Z, mm2=Y|X
		movq	mm2,[eax]FOutVector.Point.X
		punpckldq mm1,mm1					// mm1=Zp|Zp, mm3=FY15|FX15
		movq	mm3,[ecx]FSceneNode.FX15
		pfmul	(m1,m0)						// mm1=Zp/Z|Zp/Z, save Zp/Z
		movd	[eax]FTransform.RZ,mm1
		pfmul	(m2,m1)						// mm2=ZpY/Z|ZpX/Z, mm3=ZpY/Z+FY15|ZpX/Z+FX15=Ysc|Xsc
		pfadd	(m3,m2)
		movq	[eax]FTransform.ScreenX,mm3	// save Ysc|Xsc, mm4=int(Ysc)|int(Xsc)
		pf2id	(m4,m3)
		psrlq	mm4,32						// mm4=0|int(Ysc), save int(Ysc)
		movd	[eax]FTransform.IntY,mm4
	}
#else
	Vert.RZ      = /*1.0*/Frame->Proj.Z / Vert.Point.Z;
	Vert.ScreenX = Vert.Point.X /* * Frame->Proj.Z*/ * Vert.RZ + Frame->FX15;
	Vert.ScreenY = Vert.Point.Y /* * Frame->Proj.Z*/ * Vert.RZ + Frame->FY15;
	Vert.IntY    = appFloor( Vert.ScreenY );
#endif
}

//
// K6 3D optimzed version of:
//			TriNormals[i] = (V1.Point-V2.Point) ^ (V3.Point-V1.Point);
//			TriNormals[i] *= DivSqrtApprox(TriNormals[i].SizeSquared()+0.001);
//
static _inline void DoComputeTriangleNormal(FVector& TriNormal,FVector& P1,FVector& P2,FVector& P3)
{
#if 1
	static float PointOOOne=0.001;

	_asm
	{
		mov		eax,P1
		mov		ecx,P2
		mov		edx,P3

		movq	mm0,[eax]FVector.X		// mm0=P1.Y|P1.X, mm1=P2.Y|P2.X
		movq	mm1,[ecx]FVector.X
		pfsubr	(m1,m0)					// mm1=Y1|X1, mm2=P3.Y|P3.X
		movq	mm2,[edx]FVector.X
		pfsub	(m2,m0)					// mm2=Y2|X2, mm3=0|P1.Z
		movd	mm3,[eax]FVector.Z
		movq	mm6,mm1					// mm6=Y1|X1, mm4=0|P2.Z
		movd	mm4,[ecx]FVector.Z
		psrlq	mm6,32					// mm6=0|Y1, mm5=0|P3.Z
		movd	mm5,[edx]FVector.Z
		pfsubr	(m4,m3)					// mm4=0|Z1, mm7=Y2|X2
		movq	mm7,mm2
		pfsub	(m5,m3)					// mm5=0|Z2, mm0=Y2|X2
		movq	mm0,mm2
		psrlq	mm7,32					// mm7=0|Y2, mm0=0|Y1X2
		pfmul	(m0,m6)
		movq	mm3,mm1					// mm3=Y1|X1, mm6=0|Y1Z2
		pfmul	(m6,m5)
		pfmul	(m1,m7)					// mm1=0|X1Y2, mm7=0|Z1Y2
		pfmul	(m7,m4)
		pfsub	(m1,m0)					// mm1=0|X1Y2-Y1X2=0|Z, mm4=0|Z1X2
		pfmul	(m4,m2)
		pfsub	(m6,m7)					// mm6=0|Y1Z2-Z1Y2=0|X, mm5=0|X1Z2
		pfmul	(m5,m3)
		movq	mm7,mm1					// mm7=0|Z, mm1=0|ZZ
		pfmul	(m1,m1)
		pfsub	(m4,m5)					// mm4=0|Z1X2-X1Z2=0|Y, mm0=0|0.001
		movd	mm0,PointOOOne
		punpckldq mm6,mm4				// mm6=Y|X, mm1=0|ZZ+0.001
		pfadd	(m1,m0)
		movq	mm5,mm6					// mm5=Y|X, mm6=YY|XX
		pfmul	(m6,m6)
		mov		eax,TriNormal			// eax=ptr to dest, mm6=YY|XX+ZZ+0.001
		pfadd	(m6,m1)
		pfacc	(m6,m6)					// mm6=XX+YY+ZZ+0.001|XX+YY+ZZ+0.001,mm6=1/size|1/size
		pfrsqrt	(m6,m6)
		pfmul	(m5,m6)					// mm5=Y'|X',mm7=0|Z'
		pfmul	(m7,m6)
		movq	[eax]FVector.X,mm5		// save Y'|X', save 0|Z'
		movq	[eax]FVector.Z,mm7
	}
#else
	FVector V1,V2,V;
	
	// Calculate vectors from the points.
	V1.X=P1.X-P2.X;
	V1.Y=P1.Y-P2.Y;
	V1.Z=P1.Z-P2.Z;
	V2.X=P3.X-P1.X;
	V2.Y=P3.Y-P1.Y;
	V2.Z=P3.Z-P1.Z;

	// Calculate cross product.
	V.X=V1.Y * V2.Z - V1.Z * V2.Y;
	V.Y=V1.Z * V2.X - V1.X * V2.Z;
	V.Z=V1.X * V2.Y - V1.Y * V2.X;
	
	// Normalize size.
	FLOAT Scale=DivSqrtApprox(V.X*V.X + V.Y*V.Y + V.Z*V.Z + 0.001);
	TriNormal.X = V.X * Scale;
	TriNormal.Y = V.Y * Scale;
	TriNormal.Z = V.Z * Scale;
#endif
}

//
// K6 3D optimized version of:
//				Frame->Mirror*FTriple(V1.Point,V2.Point,V3.Point)>0.0 )
//
#pragma warning( disable : 4035 )
static _inline UBOOL DoVisibleCheck(FSceneNode* Frame,FVector& P1,FVector& P2,FVector& P3)
{
#if 1
	_asm
	{
		mov		edx,P3					// edx=ptr to P3, ecx=ptr to P2
		mov		ecx,P2					
		movd	mm0,[edx]FVector.Z		// mm0=0|Z3, mm2=0|Y2
		movd	mm2,[ecx]FVector.Y
		movq	mm1,mm0					// mm1=0|Z3, mm3=0|Z2
		movd	mm3,[ecx]FVector.Z
		pfmul	(m0,m2)					// mm0=0|Y2Z3, mm5=0|Y3
		movd	mm5,[edx]FVector.Y
		movq	mm4,mm3					// mm4=0|Z2, mm6=0|X3
		movd	mm6,[edx]FVector.X
		pfmul	(m3,m5)					// mm3=0|Z2Y3, mm7=0|X2
		movd	mm7,[ecx]FVector.X
		pfmul	(m4,m6)					// mm4=0|Z2X3, mm3=Y2Z3-Z2Y3=0|X'
		pfsubr	(m3,m0)
		pfmul	(m1,m7)					// mm1=0|X2Z3, eax=ptr to P1
		mov		eax,P1
		pfmul	(m2,m6)					// mm2=0|Y2X3, mm4=0|Z2X3-X2Z3=0|Y'
		pfsub	(m4,m1)
		pfmul	(m5,m7)					// mm5=0|2XY3, mm0=Y1|X1
		movq	mm0,[eax]FVector.X
		punpckldq mm3,mm4				// mm3=Y'|X', mm1=0|Z1
		movd	mm1,[eax]FVector.Z
		pfsub	(m5,m2)					// mm5=0|2XY3-Y2X3=0|Z', edx=ptr to frame
		mov		edx,Frame
		pfmul	(m3,m0)					// mm3=Y'Y1|X'X1=Y"|X", mm6=0|Mirror
		movd	mm6,[edx]FSceneNode.Mirror
		pfmul	(m5,m1)					// mm5=0|Z'Z1=0|Z", mm3=Y"+X"|Y"+X"
		pfacc	(m3,m3)
		pxor	mm7,mm7					// mm7=0|0, mm5=Y"+X"|Z"+Y"+X"
		pfadd	(m5,m3)
		pfmul	(m6,m5)					// mm6=0|Mirror*(Z"+Y"+X"), mm6=0>0|(Mirror*(Z"+Y"+X") > 0)
		pfcmpgt	(m6,m7)
		movd	eax,mm6					// Get bool result in EAX
	}
#else	
	FLOAT TripleProduct=
		(	(P1.X * (P2.Y * P3.Z - P2.Z * P3.Y))
		+	(P1.Y * (P2.Z * P3.X - P2.X * P3.Z))
		+	(P1.Z * (P2.X * P3.Y - P2.Y * P3.X)) );

	return((Frame->Mirror*TripleProduct)>0.0);
#endif
}
#pragma warning( default : 4035 )

//
// K6 3D optimized version of:
//		appRound( V1.Point.Z + V2.Point.Z + V3.Point.Z )
//
#pragma warning( disable : 4035 )
static _inline INT CalculateSimpleKey(FVector& P1,FVector& P2,FVector& P3)
{
#if 1	
	static float fPointFive=0.5f;

	_asm
	{
		mov		eax,P1				// eax=ptr to P1, mm0=0|0.5
		movd	mm0,fPointFive
		movd	mm1,[eax]FVector.Z	// mm1=0|Z1, ecx=ptr to P2
		mov		ecx,P2
		pfadd	(m1,m0)				// mm1=0|Z1+0.5, mm2=0|Z2
		movd	mm2,[ecx]FVector.Z
		mov		edx,P3				// edx=ptr to P3, mm2=0|Z1+Z2+0.5
		pfadd	(m2,m1)
		movd	mm3,[edx]FVector.Z	// mm3=0|Z3, mm3=0|Z1+Z2+Z3+0.5
		pfadd	(m3,m2)
		pf2id	(m4,m3)				// mm4=0|int(Z1+Z2+Z3+0.5), eax=mm4
		movd	eax,mm4
	}
	return;
#else
	return (appRound( P1.Z + P2.Z + P3.Z ));
#endif
}
#pragma warning( default : 4035 )

//
// K6 3D optimized version of:
//		appRound( FDistSquared(V1.Point,Hack)*FDistSquared(V2.Point,Hack)*FDistSquared(V3.Point,Hack) )
//
#pragma warning( disable : 4035 )
static _inline INT CalculateComplexKey(FVector& P1,FVector& P2,FVector& P3,FVector& Hack)
{
#if 1
	static float PointFive=0.5;
	_asm
	{
		mov		eax,Hack				// eax=ptr to Hack, ebx=ptr to P1
		mov		ebx,P1
		movq	mm0,[eax]FVector.X		// mm0=Yh|Xh, mm1=Y1|X1
		movq	mm1,[ebx]FVector.X
		mov		ecx,P2					// ecx=ptr to P2, mm1=Yh-Y1|Xh-X1=Ya|Xa
		pfsubr	(m1,m0)
		movq	mm2,[ecx]FVector.X		// mm2=Y2|X2, edx=ptr to P3
		mov		edx,P3
		pfsubr	(m2,m0)					// mm2=Yh-Y2|Xh-X2=Yb|Xb, mm3=Y3|X3
		movq	mm3,[edx]FVector.X
		movd	mm4,[eax]FVector.Z		// mm4=0|Zh, mm3=Yh-Y3|Xh-X3=Yc|Xc
		pfsubr	(m3,m0)
		movd	mm5,[ebx]FVector.Z		// mm5=0|Z1, mm1=Yaa|Xaa
		pfmul	(m1,m1)
		movd	mm6,[ecx]FVector.Z		// mm6=0|Z2, mm2=Ybb|Xbb
		pfmul	(m2,m2)
		movq	mm7,[edx]FVector.Z		// mm7=0|Z3, mm4=Zh|Zh
		punpckldq mm4,mm4
		pfacc	(m2,m1)					// mm2=Yaa+Xaa|Ybb+Xbb, mm5=Z2|Z1
		punpckldq mm5,mm6
		pfmul	(m3,m3)					// mm3=Ycc|Xcc, mm7=Zh|Zh-Z3=Zh|Zc
		pfsubr	(m7,m4)
		movd	mm0,PointFive			// mm0=0|0.5, mm5=Zh-Z2|Zh-Z1=Zb|Za
		pfsubr	(m5,m4)
		pfmul	(m7,m7)					// mm7=Zhh|Zcc, mm3=Yaa+Xaa+Ybb+Xbb|Ycc+Xcc
		pfacc	(m3,m2)
		pfmul	(m5,m5)					// mm5=Zbb|Zaa, mm7=0.5|Zcc
		punpckldq mm7,mm0
		pfacc	(m7,m5)					// mm7=Zbb+Zaa|0.5+Zcc, mm7=Yaa+Xaa+Ybb+Xbb+Ycc+Xcc|Zbb+Zaa+0.5+Zcc
		pfacc	(m7,m3)
		pfacc	(m7,m7)					// mm7=same->|Xaa+Yaa+Zaa+Xbb+Ybb+Zbb+Xcc+Ycc+Zcc+0.5, 
		pf2id	(m7,m7)										// mm7=int(^)|int(^)
		movd	eax,mm7					// Get return val in EAX
	}
	return;
#else
	FLOAT Val1=Square(Hack.X-P1.X) + Square(Hack.Y-P1.Y) + Square(Hack.Z-P1.Z);
	FLOAT Val2=Square(Hack.X-P2.X) + Square(Hack.Y-P2.Y) + Square(Hack.Z-P2.Z);
	FLOAT Val3=Square(Hack.X-P3.X) + Square(Hack.Y-P3.Y) + Square(Hack.Z-P3.Z);
	
	return (appRound(Val1+Val2+Val3));
#endif
}
#pragma warning( default : 4035 )

//
// K6 3D optimized version of:
//		FLOAT Fatness = (Owner->Fatness/16.0)-8.0;
//
// NOT PERFORMANCE CRITICAL - DONE TO REMOVE MMX->FPU->MMX SWITCH.
//
static _inline void	DoSetFatness(AActor* Owner,FLOAT &Fatness)
{
	static float InvSixteen=1.0f/16.0f;
	static float Eight=8.0f;

	_asm
	{
		mov		eax,Owner				// eax=ptr to Owner
		movzx	eax,[eax]AActor.Fatness	// eax=(BYTE)Fatness
		movd	mm1,InvSixteen			// mm1=0|1/16
		movd	mm0,eax					// mm0=0|(int)Fatness
		movd	mm2,Eight				// mm2=0|8
		pi2fd	(m0,m0)					// mm0=0|Fatness
		pfmul	(m1,m0)					// mm1=0|Fatness/16
		mov		eax,Fatness				// eax=ptr to Fatness
		pfsubr	(m2,m1)					// mm2=0|Fatness/16-8
		movd	[eax],mm2				// save

		femms	// Just in case.
	}
}

//
// K6 3D optimized version of:
//			Norm += TriNormals[Mesh->VertLinks(Connect.TriangleListOffset + k)];
//
static _inline void DoAddNormals(FVector& Dst,FVector& Src)
{
#if 1
	_asm
	{
		mov		eax,Dst
		mov		edx,Src
		movq	mm0,[eax]FVector.X
		movq	mm1,[edx]FVector.X
		pfadd	(m1,m0)
		movd	mm2,[eax]FVector.Z
		movd	mm3,[edx]FVector.Z
		pfadd	(m3,m2)
		movq	[eax]FVector.X,mm1
		movd	[eax]FVector.Z,mm3
	}
#else
	Dst.X += Src.X;
	Dst.Y += Src.Y;
	Dst.Z += Src.Z;
#endif
}

//
// K6 3D optimized version of:
//			Vert.Normal = FPlane( Vert.Point, Norm * DivSqrtApprox(Norm.SizeSquared()) );
//
static _inline void DoSetVertNormal(FPlane& Dst,FVector& Point,FVector &Norm)
{
#if 1
	_asm
	{
		mov		eax,Norm			// eax=ptr to Norm, ecx=ptr to Point
		mov		ecx,Point
		movq	mm0,[eax]FVector.X	// mm0=Yn|Xn, mm2=Yn|Xn
		movq	mm2,mm0
		pfmul	(m0,m0)				// mm0=Ynn|Xnn, mm1=0|Zn
		movd	mm1,[eax]FVector.Z
		movq	mm3,mm1				// mm3=0|Zn, mm1=0|Znn
		pfmul	(m1,m1)
		pfacc	(m0,m0)				// mm0=Ynn+Xnn|Ynn+Xnn, mm4=Yp|Xp
		movq	mm4,[ecx]FVector.X
		pfadd	(m1,m0)				// mm1=Ynn+Xnn|Xnn+Ynn+Znn, mm5=0|Zp
		movd	mm5,[ecx]FVector.Z
		mov		edx,Dst				// edx=ptr to Dst, mm1=1/size|1/size
		pfrsqrt	(m1,m1)
		pfmul	(m2,m1)				// mm2=Yna|Xna, mm3=0|Zna
		pfmul	(m3,m1)
		movq	[edx]FVector.X,mm2	// save Yna|Xna, mm4=Ypna|Xpna
		pfmul	(m4,m2)
		movd	[edx]FVector.Z,mm3	// save Zna, mm5=0|Zpna
		pfmul	(m5,m3)
		pfacc	(m4,m4)				// mm4=Ypna+Xpna|Ypna+Xpna, mm5=Ypna+Xpna|Xpna+Ypna+Zpna=?|W
		pfadd	(m5,m4)
		movd	[edx]FPlane.W,mm5	// save W
	}
#else
	FLOAT Scale=DivSqrtApprox(Norm.X*Norm.X + Norm.Y*Norm.Y + Norm.Z*Norm.Z);

	FVector NormAdj;
	NormAdj.X = Norm.X * Scale;
	NormAdj.Y = Norm.Y * Scale;
	NormAdj.Z = Norm.Z * Scale;

	Dst.X = NormAdj.X;
	Dst.Y = NormAdj.Y;
	Dst.Z = NormAdj.Z;
	Dst.W = Point.X*NormAdj.X + Point.Y*NormAdj.Y + Point.Z*NormAdj.Z;
#endif
}

//
// K6 3D optmized version of:
//			Vert.Point += Vert.Normal * Fatness;
//			Vert.ComputeOutcode( Frame );
//
static _inline void DoFattenVert(FSceneNode* Frame,FPlane& Normal,FOutVector& Vert,DWORD Fatness)
{
#if 1
	DWORD LocalFatness=Fatness; // Stupid compiler.
	_asm
	{
		mov		eax,Normal							// eax=ptr to Normal, mm1=0|F
		mov		ecx,Vert							// added - ecx=ptr to Vert
		movd	mm1,LocalFatness
		movd	mm0,[eax]FVector.Z					// mm0=0|Zn, mm1=F|F
		punpckldq mm1,mm1
		movq	mm3,[eax]FVector.X					// mm3=Yn|Xn, mm0=0|ZnF
		pfmul	(m0,m1)
		movd	mm2,[ecx]FOutVector.Point.Z			// mm2=0|Z, mm3=YnF|XnF
		pfmul	(m3,m1)
		movq	mm4,[ecx]FOutVector.Point.X			// mm4=Y|X, mm0=0|Z+ZnF=0|Z'
		pfadd	(m0,m2)
		mov		edx,Frame							// edx=ptr to Frame, mm3=Y+YnF|X+XnF=Y'|X'
		pfadd	(m3,m4)
		// This code is based on that from DoComputeOutcodeLoop
		movd	[ecx]FOutVector.Point.Z,mm0		// save Z (mm0=0|Z), mm1=PrjYP|PrjXP
		movq	mm1,[edx]FSceneNode.PrjXP
		punpckldq mm0,mm0						// mm0=Z|Z, mm2=PrjYM|PrjXM
		movq	mm2,[edx]FSceneNode.PrjXM
		pfmul	(m1,m0)							// mm1=Zyp|Zxp, save Y|X (mm3=Y|X)
		movq	[ecx]FOutVector.Point.X,mm3	
		pfmul	(m2,m0)							// mm2=Zym|Zxm, mm4=Negator
		movq	mm4,Negator
		pfcmpge	(m1,m3)							// mm1=inv(+val), mm5=+mask
		movq	mm5,PlusMask
		pxor	mm4,mm2							// mm4=-Zym|-Zxm, mm6=-mask
		movq	mm6,MinusMask
		pfcmpgt	(m4,m3)							// mm4=inv(-val) mm5=Y+c|X+c
		pandn	mm1,mm5
		pand	mm6,mm4							// mm6=mm6=Y-c|X-c, mm6=Yc|Xc
		por		mm6,mm1							
		movq	mm7,mm6							// mm7=Yc|Xc, mm6=0|Yc
		psrlq	mm6,32
		por		mm7,mm6							// mm7=Yc|ClipVal, save clip val
		movd	[ecx]FOutVector.Flags,mm7
	}

#else
	// Scale.
	Vert.Point.X += Normal.X * MAKE_FP(Fatness);
	Vert.Point.Y += Normal.Y * MAKE_FP(Fatness);
	Vert.Point.Z += Normal.Z * MAKE_FP(Fatness);
	// Compute the outcode.
	static FLOAT ClipXM, ClipXP, ClipYM, ClipYP;
	static const BYTE OutXMinTab [2] = { 0, FVF_OutXMin };
	static const BYTE OutXMaxTab [2] = { 0, FVF_OutXMax };
	static const BYTE OutYMinTab [2] = { 0, FVF_OutYMin };
	static const BYTE OutYMaxTab [2] = { 0, FVF_OutYMax };
	ClipXM = Frame->PrjXM * Vert.Point.Z + Vert.Point.X;
	ClipXP = Frame->PrjXP * Vert.Point.Z - Vert.Point.X;
	ClipYM = Frame->PrjYM * Vert.Point.Z + Vert.Point.Y;
	ClipYP = Frame->PrjYP * Vert.Point.Z - Vert.Point.Y;
	Vert.Flags  =
	(	OutXMinTab [ClipXM < 0.0]
	+	OutXMaxTab [ClipXP < 0.0]
	+	OutYMinTab [ClipYM < 0.0]
	+	OutYMaxTab [ClipYP < 0.0]);
#endif	
}

//
// K63D Optimized version of Something>1.0f
// Optimized purely to remove switch overhead.
//				
#pragma warning( disable : 4035 )
static __inline UBOOL DoIsGreaterThanOne(FLOAT &Value)
{
#if 1
	static float One=1.0f;

	_asm
	{
		mov		eax,Value
		movd	mm0,One
		movd	mm1,[eax]
		pfcmpgt	(m1,m0)
		movd	eax,mm1
	}
	return;
#else
	return (Value>1.0);
#endif
}
#pragma warning( default : 4035 )

//
// Draw a mesh map.
//
void URender::AMD3DDrawMesh
(
	FSceneNode*		Frame,
	AActor*			Owner,
	FSpanBuffer*	SpanBuffer,
	AZoneInfo*		Zone,
	const FCoords&	Coords,
	FVolActorLink*	LeafLights,
	FActorLink*		Volumetrics,
	DWORD			ExtraFlags
)
{
	guard(URender::AMD3DDrawMesh);
	STAT(clock(GStat.MeshTime));
	FMemMark Mark(GMem);
	UMesh* Mesh = Owner->Mesh;
	FVector Hack=FVector(0,-8,0);
	UBOOL NotWeaponHeuristic=(Owner->Owner!=Frame->Viewport->Actor);
	if( !Engine->Client->CurvedSurfaces )
		ExtraFlags |= PF_Flat;

#if 0
	// For testing actor span clipping.
	if( SpanBuffer )
		for( INT i=SpanBuffer->StartY; i<SpanBuffer->EndY; i++ )
			for( FSpan* Span=SpanBuffer->Index[i-SpanBuffer->StartY]; Span; Span=Span->Next )
				appMemset( Frame->Screen(Span->Start,i), appRand(), (Span->End-Span->Start)*4 );
#endif

	// Get transformed verts.
	FTransTexture* Samples=NULL;
	UBOOL bWire=0;
	guardSlow(Transform);
	STAT(clock(GStat.MeshGetFrameTime));
	Samples = New<FTransTexture>(GMem,Mesh->FrameVerts);
	bWire = Frame->Viewport->IsOrtho() || Frame->Viewport->Actor->RendMap==REN_Wire;
	Mesh->GetFrame( &Samples->Point, sizeof(Samples[0]), bWire ? GMath.UnitCoords : Coords, Owner );
	STAT(unclock(GStat.MeshGetFrameTime));
	unguardSlow;

	// Compute outcodes.
//	BYTE Outcode = FVF_OutReject;
//	for( INT i=0; i<Mesh->FrameVerts; i++ )
//	{
//		Samples[i].Light.R = -1;
//		Samples[i].ComputeOutcode( Frame );
//		Outcode &= Samples[i].Flags;
//	}
	BYTE Outcode;
	guardSlow(Outcode);
	Outcode = DoComputeOutcodeLoop(Frame,Samples,Mesh->FrameVerts);
	unguardSlow;

	// Render a wireframe view or textured view.
	if( bWire )
	{
		// Render each wireframe triangle.
		guardSlow(RenderWire);
		FPlane Color = Owner->bSelected ? FPlane(.2,.8,.1,0) : FPlane(.6,.4,.1,0);
		for( INT i=0; i<Mesh->Tris.Num(); i++ )
		{
			FMeshTri& Tri    = Mesh->Tris(i);
			FVector*  P1     = &Samples[Tri.iVertex[2]].Point;
			for( int j=0; j<3; j++ )
			{
				FVector* P2 = &Samples[Tri.iVertex[j]].Point;
				if( (Tri.PolyFlags & PF_TwoSided) || P1->X>=P2->X  )
					Draw3DLine( Frame, Color, LINE_DepthCued, *P1, *P2 );
				P1 = P2;
			}
		}
		STAT(unclock(GStat.MeshTime));
		Mark.Pop();
		unguardSlow;
		return;
	}

	// Coloring.
//	FLOAT Unlit  = Clamp( Owner->ScaleGlow*0.5f + Owner->AmbientGlow/256.f, 0.f, 1.f );
//	GUnlitColor  = FVector( Unlit, Unlit, Unlit );
//	if( GIsEditor && (ExtraFlags & PF_Selected) )
//		GUnlitColor = GUnlitColor*0.4 + FVector(0.0,0.6,0.0);
	// This code has been converted to K6 3D purely to remove a MMX->FPU->MMX state change.
	DoCalcUnlitColor(Owner,GUnlitColor,GIsEditor && (ExtraFlags & PF_Selected));

	// Mesh based particle effects.
	if( Owner->bParticles )
	{
		guardSlow(Particles);
		check(Owner->Texture);
		FTransform** SortedPts = New<FTransform*>(GMem,Mesh->FrameVerts);
		INT Count=0;
		// No more x87 code until the next femms.
		for( INT i=0; i<Mesh->FrameVerts; i++ )
		{
//			if( !Samples[i].Flags && Samples[i].Point.Z>1.0)
			if( !Samples[i].Flags && DoIsGreaterThanOne(Samples[i].Point.Z))
			{
//				Samples[i].Project( Frame );
				DoProjection(Frame,Samples[i]);
				SortedPts[Count++] = &Samples[i];
			}
		}
		// Finished K6 3D code
		DoFemms();	// Just in case.

		if( Frame->Viewport->RenDev->SpanBased )
		{
			appSort( SortedPts, Count );
		}
		for( i=0; i<Count; i++ )
		{
			if( !SortedPts[i]->Flags )
			{
				FLOAT XSize = SortedPts[i]->RZ * Owner->Texture->USize * Owner->DrawScale;
				FLOAT YSize = SortedPts[i]->RZ * Owner->Texture->VSize * Owner->DrawScale;
				Frame->Viewport->Canvas->DrawIcon
				(
					Owner->Texture,
					SortedPts[i]->ScreenX - XSize/2,
					SortedPts[i]->ScreenY - XSize/2,
					XSize,
					YSize,
					SpanBuffer,
					Samples[i].Point.Z,
					GUnlitColor,
					FPlane(0,0,0,0),
					ExtraFlags | PF_TwoSided | Owner->Texture->PolyFlags
				);
			}
		}
		Mark.Pop();
		STAT(unclock(GStat.MeshTime));
		unguardSlow;
		return;
	}

	// Set up triangles.
	INT VisibleTriangles = 0;
	HasSpecialCoords = 0;
	FMeshTriSort* TriPool=NULL;
	FVector* TriNormals=NULL;
	if( Outcode == 0 )
	{
		// Process triangles.
		guardSlow(Process);
		TriPool    = New<FMeshTriSort>(GMem,Mesh->Tris.Num());
		TriNormals = New<FVector>(GMem,Mesh->Tris.Num());

		// Set up list for triangle sorting, adding all possibly visible triangles.
		STAT(clock(GStat.MeshProcessTime));
		FMeshTriSort* TriTop = &TriPool[0];

		// Don't do any floating point until next femms.
		for( INT i=0; i<Mesh->Tris.Num(); i++ )
		{
			FMeshTri*   Tri = &Mesh->Tris(i);
			FTransform& V1  = Samples[Tri->iVertex[0]];
			FTransform& V2  = Samples[Tri->iVertex[1]];
			FTransform& V3  = Samples[Tri->iVertex[2]];
			DWORD PolyFlags = ExtraFlags | Tri->PolyFlags;

			// Compute triangle normal.
//			TriNormals[i] = (V1.Point-V2.Point) ^ (V3.Point-V1.Point);
//			TriNormals[i] *= DivSqrtApprox(TriNormals[i].SizeSquared()+0.001);
			DoComputeTriangleNormal(TriNormals[i],V1.Point,V2.Point,V3.Point);

			// See if potentially visible.
			if( !(V1.Flags & V2.Flags & V3.Flags) )
			{
				if
				(	(PolyFlags & (PF_TwoSided|PF_Flat|PF_Invisible))!=(PF_Flat)
//				||	Frame->Mirror*FTriple(V1.Point,V2.Point,V3.Point)>0.0 )
				||	DoVisibleCheck(Frame,V1.Point,V2.Point,V3.Point) )
				{
					// This is visible.
					TriTop->Tri = Tri;

					// Set the sort key.
					TriTop->Key
//					= NotWeaponHeuristic ? appRound( V1.Point.Z + V2.Point.Z + V3.Point.Z )
//					: TriTop->Key=appRound( FDistSquared(V1.Point,Hack)*FDistSquared(V2.Point,Hack)*FDistSquared(V3.Point,Hack) );
					= NotWeaponHeuristic ? CalculateSimpleKey(V1.Point,V2.Point,V3.Point)
					: TriTop->Key=CalculateComplexKey(V1.Point,V2.Point,V3.Point,Hack);

					// Add to list.
					VisibleTriangles++;
					TriTop++;
				}
			}
		}
		// Finished K6 3D code
		DoFemms(); // Just in case.

		STAT(unclock(GStat.MeshProcessTime));
		unguardSlow;
	}

	// Render triangles.
	if( VisibleTriangles>0 )
	{
		guardSlow(Render);

		// Fatness.
		UBOOL Fatten = Owner->Fatness!=128;
//		FLOAT Fatness = (Owner->Fatness/16.0)-8.0;
		// This code is optimized purely to avoid MMX->FPU->MMX switch.
		FLOAT Fatness;
		DoSetFatness(Owner,Fatness);

		// Sort by depth.
		if( Frame->Viewport->RenDev->SpanBased )
			appSort( TriPool, VisibleTriangles );

		// Lock the textures.
		UTexture* EnvironmentMap = NULL;
		guardSlow(Lock);
		check(Mesh->Textures.Num()<=ARRAY_COUNT(TextureInfo));
		for( INT i=0; i<Mesh->Textures.Num(); i++ )
		{
			Textures[i] = Mesh->GetTexture( i, Owner );
			if( Textures[i] )
			{
				Textures[i]->GetInfo( TextureInfo[i], Frame->Viewport->CurrentTime );
				EnvironmentMap = Textures[i];
			}
		}
		if( Owner->Texture )
			EnvironmentMap = Owner->Texture;
		else if( Owner->Region.Zone && Owner->Region.Zone->EnvironmentMap )
			EnvironmentMap = Owner->Region.Zone->EnvironmentMap;
		else if( Owner->Level->EnvironmentMap )
			EnvironmentMap = Owner->Level->EnvironmentMap;
		check(EnvironmentMap);
		EnvironmentMap->GetInfo( EnvironmentInfo, Frame->Viewport->CurrentTime );
		unguardSlow;

		// Build list of all incident lights on the mesh.
		STAT(clock(GStat.MeshLightSetupTime));
		ExtraFlags |= GLightManager->SetupForActor( Frame, Owner, LeafLights, Volumetrics );
		STAT(unclock(GStat.MeshLightSetupTime));

		// Don't do any floating point until next femms.
		DoFemms();

		// Perform all vertex lighting.
		guardSlow(Light);
		for( INT i=0; i<VisibleTriangles; i++ )
		{
			FMeshTri& Tri = *TriPool[i].Tri;
			for( INT j=0; j<3; j++ )
			{
				INT iVert = Tri.iVertex[j];
				FTransSample& Vert = Samples[iVert];
				if( Vert.Light.R == -1 )
				{
					// Compute vertex normal.
					FVector Norm(0,0,0);
					FMeshVertConnect& Connect = Mesh->Connects(iVert);
					for( INT k=0; k<Connect.NumVertTriangles; k++ )
//						Norm += TriNormals[Mesh->VertLinks(Connect.TriangleListOffset + k)];
						DoAddNormals(Norm,TriNormals[Mesh->VertLinks(Connect.TriangleListOffset + k)]);
//					Vert.Normal = FPlane( Vert.Point, Norm * DivSqrtApprox(Norm.SizeSquared()) );
					DoSetVertNormal(Vert.Normal,Vert.Point,Norm);

					// Fatten it if desired.
					if( Fatten )
					{
//						Vert.Point += Vert.Normal * Fatness;
//						Vert.ComputeOutcode( Frame );
						DoFattenVert(Frame,Vert.Normal,Vert,MAKE_DWORD(Fatness));
					}

					// Compute effect of each lightsource on this vertex.
					Vert.Light = GLightManager->Light( Vert, ExtraFlags );
					Vert.Fog   = GLightManager->Fog  ( Vert, ExtraFlags );

					// Project it.
					if( !Vert.Flags )
//						Vert.Project( Frame );
						DoProjection(Frame,Vert);
				}
			}
		}
		// Finished K6 3D code
		DoFemms();
		unguardSlow;

		// Draw the triangles.
		guardSlow(DrawVisible);
		STAT(GStat.MeshPolyCount+=VisibleTriangles);
		for( INT i=0; i<VisibleTriangles; i++ )
		{
			// Set up the triangle.
			FMeshTri& Tri = *TriPool[i].Tri;
			if( !(Tri.PolyFlags & PF_Invisible) )
			{
				// Get texture.
				DWORD PolyFlags = Tri.PolyFlags | ExtraFlags;
				INT Index = TriPool[i].Tri->TextureIndex;
				FTextureInfo& Info = (Textures[Index] && !(PolyFlags & PF_Environment)) ? TextureInfo[Index] : EnvironmentInfo;
				UScale = Info.USize / 256.0;
				VScale = Info.VSize / 256.0;

				// Set up texture coords.
				FTransTexture* Pts[6];
				for( INT j=0; j<3; j++ )
				{
					Pts[j]    = &Samples[Tri.iVertex[j]];
					Pts[j]->U = Tri.Tex[j].U * UScale;
					Pts[j]->V = Tri.Tex[j].V * VScale;
				}
				if( Frame->Mirror == -1 )
					Exchange( Pts[2], Pts[0] );
				RenderSubsurface( Frame, Info, SpanBuffer, Pts, PolyFlags, 0 );
			}
			else
			{
				// Remember coordinate system.
				FVector Mid = 0.5*(Samples[Tri.iVertex[0]].Point + Samples[Tri.iVertex[2]].Point);

				FCoords C;
				C.Origin = FVector(0,0,0);
				C.XAxis	 = (Samples[Tri.iVertex[1]].Point - Mid).SafeNormal();
				C.YAxis	 = (C.XAxis ^ (Samples[Tri.iVertex[0]].Point - Samples[Tri.iVertex[2]].Point)).SafeNormal();
				C.ZAxis	 = C.YAxis ^ C.XAxis;

				SpecialCoords = GMath.UnitCoords * Mid * C;
				HasSpecialCoords = 1;
			}
		}
		GLightManager->FinishActor();
		unguardSlow;
		unguardSlow;
	}

	STAT(GStat.MeshCount++);
	STAT(unclock(GStat.MeshTime));
	Mark.Pop();
	unguardf(( "(%s)", Owner->Mesh->GetName() ));
}
#pragma warning( default : 4799 )
#pragma pack()
#endif
//
// Draw a mesh map.
//
void URender::DrawMesh
(
	FSceneNode*		Frame,
	AActor*			Owner,
	FSpanBuffer*	SpanBuffer,
	AZoneInfo*		Zone,
	const FCoords&	Coords,
	FVolActorLink*	LeafLights,
	FActorLink*		Volumetrics,
	DWORD			ExtraFlags
)
{
#ifndef NOAMD3D
	if (GIsK63D)
	{
		AMD3DDrawMesh(Frame,Owner,SpanBuffer,Zone,Coords,LeafLights,Volumetrics,ExtraFlags);
		return;
	}
#endif

	guard(URender::DrawMesh);
	STAT(clock(GStat.MeshTime));
	FMemMark Mark(GMem);
	UMesh*  Mesh = Owner->Mesh;
	FVector Hack = FVector(0,-8,0);
	UBOOL NotWeaponHeuristic=(Owner->Owner!=Frame->Viewport->Actor);
	if( !Engine->Client->CurvedSurfaces )
		ExtraFlags |= PF_Flat;

#if 0
	// For testing actor span clipping.
	if( SpanBuffer )
		for( INT i=SpanBuffer->StartY; i<SpanBuffer->EndY; i++ )
			for( FSpan* Span=SpanBuffer->Index[i-SpanBuffer->StartY]; Span; Span=Span->Next )
				appMemset( Frame->Screen(Span->Start,i), appRand(), (Span->End-Span->Start)*4 );
#endif

	// Get transformed verts.
	FTransTexture* Samples=NULL;
	UBOOL bWire=0;
	guardSlow(Transform);
	STAT(clock(GStat.MeshGetFrameTime));
	Samples = New<FTransTexture>(GMem,Mesh->FrameVerts);
	bWire = Frame->Viewport->IsOrtho() || Frame->Viewport->Actor->RendMap==REN_Wire;
	Mesh->GetFrame( &Samples->Point, sizeof(Samples[0]), bWire ? GMath.UnitCoords : Coords, Owner );
	STAT(unclock(GStat.MeshGetFrameTime));
	unguardSlow;

	// Compute outcodes.
	BYTE Outcode = FVF_OutReject;
	guardSlow(Outcode);
	for( INT i=0; i<Mesh->FrameVerts; i++ )
	{
		Samples[i].Light.R = -1;
		Samples[i].ComputeOutcode( Frame );
		Outcode &= Samples[i].Flags;
	}
	unguardSlow;

	// Render a wireframe view or textured view.
	if( bWire )
	{
		// Render each wireframe triangle.
		guardSlow(RenderWire);
		FPlane Color = Owner->bSelected ? FPlane(.2,.8,.1,0) : FPlane(.6,.4,.1,0);
		for( INT i=0; i<Mesh->Tris.Num(); i++ )
		{
			FMeshTri& Tri    = Mesh->Tris(i);
			FVector*  P1     = &Samples[Tri.iVertex[2]].Point;
			for( int j=0; j<3; j++ )
			{
				FVector* P2 = &Samples[Tri.iVertex[j]].Point;
				if( (Tri.PolyFlags & PF_TwoSided) || P1->X>=P2->X  )
					Draw3DLine( Frame, Color, LINE_DepthCued, *P1, *P2 );
				P1 = P2;
			}
		}
		STAT(unclock(GStat.MeshTime));
		Mark.Pop();
		unguardSlow;
		return;
	}

	// Coloring.
	FLOAT Unlit  = Clamp( Owner->ScaleGlow*0.5f + Owner->AmbientGlow/256.f, 0.f, 1.f );
	GUnlitColor  = FVector( Unlit, Unlit, Unlit );
	if( GIsEditor && (ExtraFlags & PF_Selected) )
		GUnlitColor = GUnlitColor*0.4 + FVector(0.0,0.6,0.0);

	// Mesh based particle effects.
	if( Owner->bParticles )
	{
		guardSlow(Particles);
		check(Owner->Texture);
		FTransform** SortedPts = New<FTransform*>(GMem,Mesh->FrameVerts);
		INT Count=0;
		for( INT i=0; i<Mesh->FrameVerts; i++ )
		{
			if( !Samples[i].Flags && Samples[i].Point.Z>1.0 )
			{
				Samples[i].Project( Frame );
				SortedPts[Count++] = &Samples[i];
			}
		}
		if( Frame->Viewport->RenDev->SpanBased )
		{
			appSort( SortedPts, Count );
		}
		for( i=0; i<Count; i++ )
		{
			if( !SortedPts[i]->Flags )
			{
				FLOAT XSize = SortedPts[i]->RZ * Owner->Texture->USize * Owner->DrawScale;
				FLOAT YSize = SortedPts[i]->RZ * Owner->Texture->VSize * Owner->DrawScale;
				Frame->Viewport->Canvas->DrawIcon
				(
					Owner->Texture,
					SortedPts[i]->ScreenX - XSize/2,
					SortedPts[i]->ScreenY - XSize/2,
					XSize,
					YSize,
					SpanBuffer,
					Samples[i].Point.Z,
					GUnlitColor,
					FPlane(0,0,0,0),
					ExtraFlags | PF_TwoSided | Owner->Texture->PolyFlags
				);
			}
		}
		Mark.Pop();
		STAT(unclock(GStat.MeshTime));
		unguardSlow;
		return;
	}

	// Set up triangles.
	INT VisibleTriangles = 0;
	HasSpecialCoords = 0;
	FMeshTriSort* TriPool=NULL;
	FVector* TriNormals=NULL;
	if( Outcode == 0 )
	{
		// Process triangles.
		guardSlow(Process);
		TriPool    = New<FMeshTriSort>(GMem,Mesh->Tris.Num());
		TriNormals = New<FVector>(GMem,Mesh->Tris.Num());

		// Set up list for triangle sorting, adding all possibly visible triangles.
		STAT(clock(GStat.MeshProcessTime));
		FMeshTriSort* TriTop = &TriPool[0];
		for( INT i=0; i<Mesh->Tris.Num(); i++ )
		{
			FMeshTri*   Tri = &Mesh->Tris(i);
			FTransform& V1  = Samples[Tri->iVertex[0]];
			FTransform& V2  = Samples[Tri->iVertex[1]];
			FTransform& V3  = Samples[Tri->iVertex[2]];
			DWORD PolyFlags = ExtraFlags | Tri->PolyFlags;

			// Compute triangle normal.
			TriNormals[i] = (V1.Point-V2.Point) ^ (V3.Point-V1.Point);
			TriNormals[i] *= DivSqrtApprox(TriNormals[i].SizeSquared()+0.001);

			// See if potentially visible.
			if( !(V1.Flags & V2.Flags & V3.Flags) )
			{
				if
				(	(PolyFlags & (PF_TwoSided|PF_Flat|PF_Invisible))!=(PF_Flat)
				||	Frame->Mirror*FTriple(V1.Point,V2.Point,V3.Point)>0.0 )
				{
					// This is visible.
					TriTop->Tri = Tri;

					// Set the sort key.
					TriTop->Key
					= NotWeaponHeuristic ? appRound( V1.Point.Z + V2.Point.Z + V3.Point.Z )
					: TriTop->Key=appRound( FDistSquared(V1.Point,Hack)*FDistSquared(V2.Point,Hack)*FDistSquared(V3.Point,Hack) );

					// Add to list.
					VisibleTriangles++;
					TriTop++;
				}
			}
		}
		STAT(unclock(GStat.MeshProcessTime));
		unguardSlow;
	}

	// Render triangles.
	if( VisibleTriangles>0 )
	{
		guardSlow(Render);

		// Fatness.
		UBOOL Fatten = Owner->Fatness!=128;
		FLOAT Fatness = (Owner->Fatness/16.0)-8.0;

		// Sort by depth.
		if( Frame->Viewport->RenDev->SpanBased )
			appSort( TriPool, VisibleTriangles );

		// Lock the textures.
		UTexture* EnvironmentMap = NULL;
		guardSlow(Lock);
		check(Mesh->Textures.Num()<=ARRAY_COUNT(TextureInfo));
		for( INT i=0; i<Mesh->Textures.Num(); i++ )
		{
			Textures[i] = Mesh->GetTexture( i, Owner );
			if( Textures[i] )
			{
				Textures[i]->GetInfo( TextureInfo[i], Frame->Viewport->CurrentTime );
				EnvironmentMap = Textures[i];
			}
		}
		if( Owner->Texture )
			EnvironmentMap = Owner->Texture;
		else if( Owner->Region.Zone && Owner->Region.Zone->EnvironmentMap )
			EnvironmentMap = Owner->Region.Zone->EnvironmentMap;
		else if( Owner->Level->EnvironmentMap )
			EnvironmentMap = Owner->Level->EnvironmentMap;
		check(EnvironmentMap);
		EnvironmentMap->GetInfo( EnvironmentInfo, Frame->Viewport->CurrentTime );
		unguardSlow;

		// Build list of all incident lights on the mesh.
		STAT(clock(GStat.MeshLightSetupTime));
		ExtraFlags |= GLightManager->SetupForActor( Frame, Owner, LeafLights, Volumetrics );
		STAT(unclock(GStat.MeshLightSetupTime));

		// Perform all vertex lighting.
		guardSlow(Light);
		for( INT i=0; i<VisibleTriangles; i++ )
		{
			FMeshTri& Tri = *TriPool[i].Tri;
			for( INT j=0; j<3; j++ )
			{
				INT iVert = Tri.iVertex[j];
				FTransSample& Vert = Samples[iVert];
				if( Vert.Light.R == -1 )
				{
					// Compute vertex normal.
					FVector Norm(0,0,0);
					FMeshVertConnect& Connect = Mesh->Connects(iVert);
					for( INT k=0; k<Connect.NumVertTriangles; k++ )
						Norm += TriNormals[Mesh->VertLinks(Connect.TriangleListOffset + k)];
					Vert.Normal = FPlane( Vert.Point, Norm * DivSqrtApprox(Norm.SizeSquared()) );

					// Fatten it if desired.
					if( Fatten )
					{
						Vert.Point += Vert.Normal * Fatness;
						Vert.ComputeOutcode( Frame );
					}

					// Compute effect of each lightsource on this vertex.
					Vert.Light = GLightManager->Light( Vert, ExtraFlags );
					Vert.Fog   = GLightManager->Fog  ( Vert, ExtraFlags );

					// Project it.
					if( !Vert.Flags )
						Vert.Project( Frame );
				}
			}
		}
		unguardSlow;

		// Draw the triangles.
		guardSlow(DrawVisible);
		STAT(GStat.MeshPolyCount+=VisibleTriangles);
		for( INT i=0; i<VisibleTriangles; i++ )
		{
			// Set up the triangle.
			FMeshTri& Tri = *TriPool[i].Tri;
			if( !(Tri.PolyFlags & PF_Invisible) )
			{
				// Get texture.
				DWORD PolyFlags = Tri.PolyFlags | ExtraFlags;
				INT Index = TriPool[i].Tri->TextureIndex;
				FTextureInfo& Info = (Textures[Index] && !(PolyFlags & PF_Environment)) ? TextureInfo[Index] : EnvironmentInfo;
				UScale = Info.UScale * Info.USize / 256.0;
				VScale = Info.VScale * Info.VSize / 256.0;

				// Set up texture coords.
				FTransTexture* Pts[6];
				for( INT j=0; j<3; j++ )
				{
					Pts[j]    = &Samples[Tri.iVertex[j]];
					Pts[j]->U = Tri.Tex[j].U * UScale;
					Pts[j]->V = Tri.Tex[j].V * VScale;
				}
				if( Frame->Mirror == -1 )
					Exchange( Pts[2], Pts[0] );
				RenderSubsurface( Frame, Info, SpanBuffer, Pts, PolyFlags, 0 );
			}
			else
			{
				// Remember coordinate system.
				FVector Mid = 0.5*(Samples[Tri.iVertex[0]].Point + Samples[Tri.iVertex[2]].Point);

				FCoords C;
				C.Origin = FVector(0,0,0);
				C.XAxis	 = (Samples[Tri.iVertex[1]].Point - Mid).SafeNormal();
				C.YAxis	 = (C.XAxis ^ (Samples[Tri.iVertex[0]].Point - Samples[Tri.iVertex[2]].Point)).SafeNormal();
				C.ZAxis	 = C.YAxis ^ C.XAxis;

				SpecialCoords = GMath.UnitCoords * Mid * C;
				HasSpecialCoords = 1;
			}
		}
		GLightManager->FinishActor();
		unguardSlow;
		unguardSlow;
	}

	STAT(GStat.MeshCount++);
	STAT(unclock(GStat.MeshTime));
	Mark.Pop();
	unguardf(( "(%s)", Owner->Mesh->GetName() ));
}

/*------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------*/
