/*=============================================================================
	UnMesh.cpp: Unreal mesh animation functions
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"
#include "amd3d.h"

/*-----------------------------------------------------------------------------
	UMesh object implementation.
-----------------------------------------------------------------------------*/

UMesh::UMesh()
{
	guard(UMesh::UMesh);

	// Scaling.
	Scale			= FVector(1,1,1);
	Origin			= FVector(0,0,0);
	RotOrigin		= FRotator(0,0,0);

	// Flags.
	AndFlags		= ~(DWORD)0;
	OrFlags			= 0;

	unguardobj;
}
void UMesh::Serialize( FArchive& Ar )
{
	guard(UMesh::Serialize);

	// Serialize parent.
	UPrimitive::Serialize(Ar);

	// Serialize this.
	Ar << Verts << Tris << AnimSeqs;
	Ar << Connects << BoundingBox << BoundingSphere << VertLinks << Textures;
	Ar << BoundingBoxes << BoundingSpheres;
	Ar << FrameVerts << AnimFrames;
	Ar << AndFlags << OrFlags;
	Ar << Scale << Origin << RotOrigin;
	Ar << CurPoly << CurVertex;

	unguard;
}
IMPLEMENT_CLASS(UMesh);

/*-----------------------------------------------------------------------------
	UMesh collision interface.
-----------------------------------------------------------------------------*/

//
// Get the rendering bounding box for this primitive, as owned by Owner.
//
FBox UMesh::GetRenderBoundingBox( const AActor* Owner, UBOOL Exact ) const
{
	guard(UMesh::GetRenderBoundingBox);
	FBox Bound;

	// Get frame indices.
	INT iFrame1 = 0, iFrame2 = 0;
	const FMeshAnimSeq *Seq = GetAnimSeq( Owner->AnimSequence );
	if( Seq && Owner->AnimFrame>=0.0 )
	{
		// Animating, so use bound enclosing two frames' bounds.
		INT iFrame = appFloor((Owner->AnimFrame+1.0) * Seq->NumFrames);
		iFrame1    = Seq->StartFrame + ((iFrame + 0) % Seq->NumFrames);
		iFrame2    = Seq->StartFrame + ((iFrame + 1) % Seq->NumFrames);
		Bound      = BoundingBoxes(iFrame1) + BoundingBoxes(iFrame2);
	}
	else
	{
		// Interpolating, so be pessimistic and use entire-mesh bound.
		Bound = BoundingBox;
	}

	// Transform Bound by owner's scale and origin.
	FLOAT DrawScale = Owner->bParticles ? 1.5 : Owner->DrawScale;
	Bound = FBox( Scale*DrawScale*(Bound.Min - Origin), Scale*DrawScale*(Bound.Max - Origin) ).ExpandBy(1.0);
	FCoords Coords = GMath.UnitCoords / RotOrigin / Owner->Rotation;
	Coords.Origin  = Owner->Location + Owner->PrePivot;
	return Bound.TransformBy( Coords.Transpose() );
	unguardobj;
}

//
// Get the rendering bounding sphere for this primitive, as owned by Owner.
//
FSphere UMesh::GetRenderBoundingSphere( const AActor* Owner, UBOOL Exact ) const
{
	guard(UMesh::GetRenderBoundingSphere);
	return FSphere(0);
	unguardobj;
}

//
// Primitive box line check.
//
UBOOL UMesh::LineCheck
(
	FCheckResult	&Result,
	AActor			*Owner,
	FVector			End,
	FVector			Start,
	FVector			Extent,
	DWORD           ExtraNodeFlags
)
{
	guard(UMesh::LineCheck);
	if( Extent != FVector(0,0,0) )
	{
		// Use cylinder.
		return UPrimitive::LineCheck( Result, Owner, End, Start, Extent, ExtraNodeFlags );
	}
	else
	{
		// Could use exact mesh collision.
		// 1. Reject with local bound.
		// 2. x-wise intersection test with all polygons.
		return UPrimitive::LineCheck( Result, Owner, End, Start, FVector(0,0,0), ExtraNodeFlags );
	}
	unguardobj;
}

/*-----------------------------------------------------------------------------
	UMesh animation interface.
-----------------------------------------------------------------------------*/

#ifndef NOAMD3D
#pragma pack( 8 )
#pragma warning( disable : 4799 )

//
// K6 3D Optimized version of the interpolation loop of UMesh::GetFrame
//
static _inline void DoInterpolateLoop(FMeshVert* MeshVertex1,FMeshVert* MeshVertex2,INT FrameVerts,
									  FVector& Origin,FLOAT Alpha,FCoords& Coords,
									  FVector* CachedVerts,FVector* ResultVerts, INT ResultVertSize)
{
#if 1
	FVector CombinedOrigin;
	FLOAT TmpAlpha=Alpha; // Bloody compiler.
	struct
	{
		float X,Y;
	} Vertex1, Vertex2;

	_asm
	{
		femms

		// Calculate combined origin.
		mov		ebx,Origin
		mov		edx,Coords
		movq	mm2,[ebx]FVector.X
		movq	mm4,[edx]FCoords.Origin.X
		pfadd	(m4,m2)
		movd	mm3,[ebx]FVector.Z
		movd	mm5,[edx]FCoords.Origin.Z
		pfadd	(m5,m3)
		movq	CombinedOrigin.X,mm4
		movd	CombinedOrigin.Z,mm5

		// Set up for loop.
		mov		eax,MeshVertex1
		mov		ecx,FrameVerts
		mov		esi,ResultVerts
		mov		edi,CachedVerts
		sub		esi,4				// Move ESI back a bit to prevent plain [esi] addressing.
		cmp		ecx,0
		jz		Done

InterpolateLoop:
		// Expand packed FMeshVerts
		mov		FrameVerts,ecx		// Save count, mm7=0|A
		movd	mm7,TmpAlpha
		mov		ebx,[eax]			// Get packed V1
		shl		ebx,21				// Extract X1
		sar		ebx,21
		mov		ecx,[eax]			// Get packed V1
		mov		Vertex1.X,ebx		// Save X1
		shl		ecx,10				// Extract Y1
		sar		ecx,21
		mov		edx,[eax]			// Get packed V1
		mov		Vertex1.Y,ecx		// Save Y1
		add		eax,TYPE FMeshVert	// Increment MeshVertex1 ptr
		mov		ebx,MeshVertex2		// Get ptr to MeshVertes2
		mov		MeshVertex1,eax		// Save updated MeshVertex1
		sar		edx,22				// Extract Z1
		movq	mm0,Vertex1.X		// mm0=(int)Y1|X1 
		movd	mm1,edx				// Save Z1 directly to MMX register (mm1=(int)0|Z1)
		mov		ecx,[ebx]			// Get packed V2
		shl		ecx,21				// Extract X2
		sar		ecx,21
		mov		eax,[ebx]			// Get packed V2
		mov		Vertex2.X,ecx		// Save X2
		shl		eax,10				// Extract Y2
		sar		eax,21
		mov		edx,[ebx]			// Get packed V2
		mov		Vertex2.Y,eax		// Save Y2
		add		ebx,TYPE FMeshVert	// Increment MeshVertex2 ptr
		sar		edx,22				// Extract Z2
		mov		MeshVertex2,ebx		// Save updated MeshVertex2
		movq	mm2,Vertex2.X		// mm2=int(Y2|X2)
		movd	mm3,edx				// Save Z2 directly to MMX register (mm3=(int)0|Z2)

		// Now do interpolation and transformation.
		pi2fd	(m0,m0)					// mm0=Y1|X1, edx=ptr to Coords
		mov		ecx,Coords
		pi2fd	(m2,m2)					// mm2=Y2|X2, mm7=A|A
		punpckldq mm7,mm7
		pfsub	(m2,m0)					// mm2=Y2-Y1|X2-X1, mm1=0|Z1
		pi2fd	(m1,m1)
		pfmul	(m2,m7)					// mm2=(Y2-Y1)A|(X2-X1)A, mm3=0|Z2
		pi2fd	(m3,m3)
		pfsub	(m3,m1)					// mm3=0|Z2-Z1, mm4=Yo|Xo
		movq	mm4,CombinedOrigin.X
		pfmul	(m3,m7)					// mm3=0|(Z2-Z1)A, mm5=0|Zo
		movd	mm5,CombinedOrigin.Z
		pfadd	(m0,m2)					// mm0=Y1+(Y2-Y1)A|X1+(X2-X1)A=Ycv|Xcv, mm6=Yxa|Xxa
		movq	mm6,[ecx]FCoords.XAxis.X
		pfadd	(m1,m3)					// mm1=0|Z1+(Z2-Z1)A=0|Zcv, mm7=0|Zxa
		movd	mm7,[ecx]FCoords.XAxis.Z
		pfsubr	(m4,m0)					// mm4=Ycv-Yo|Xcv-Xo=Y|X, mm2=Yya|Xya
		movq	mm2,[ecx]FCoords.YAxis.X
		pfsubr	(m5,m1)					// mm5=0|Z0-Zcv=0|Z, mm3=0|Zya
		movd	mm3,[ecx]FCoords.YAxis.Z
		pfmul	(m6,m4)					// mm6=YxaY|XxaX=Yx|Xx, save Ycv|Xcv
		movq	[edi]FVector.X,mm0
		pfmul	(m7,m5)					// mm7=0|ZxaZ=0|Zx, mm0=Yza|Xza
		movq	mm0,[ecx]FCoords.ZAxis.X
		pfmul	(m2,m4)					// mm2=YyaY|XyaX=Yy|Xy, save Zcv
		movd	[edi]FVector.Z,mm1
		pfmul	(m3,m5)					// mm3=0|ZyaZ=0|Zy, mm1=0|Zza
		movd	mm1,[ecx]FCoords.ZAxis.Z
		pfmul	(m4,m0)					// mm4=YzaY|XzaX=Yz|Xz,mm6=Xy+Yy|Xx+Yx
		pfacc	(m6,m2)
		punpckldq mm7,mm3				// mm7=Zy|Zx, mm5=0|ZzaZ=0|Zz
		pfmul	(m5,m1)
		pfacc	(m4,m4)					// mm4=Yz+Xz|Yz+Xz, ecx=Count
		mov		ecx,FrameVerts			
		pfadd	(m6,m7)					// mm6=Xy+Yy+Zy|Xx+Yx+Zx=Y'|X', eax=ptr to MeshVertex1
		mov		eax,MeshVertex1
		pfadd	(m5,m4)					// mm5=Yz+Xz|Zz+Yz+Xz=?|Z', inc ptr to CachedVerts
		add		edi,TYPE FVector
		movq	[esi+4]FVector.X,mm6	// save Y'|X', save Z'
		movd	[esi+4]FVector.Z,mm5
		add		esi,ResultVertSize		// inc ptr to ResultVerts, loop
		dec		ecx
		jnz		InterpolateLoop
Done:
		femms
	}
#else
	// Calculate combined origin.
	FVector CombinedOrigin;
	CombinedOrigin.X = Origin.X + Coords.Origin.X;
	CombinedOrigin.Y = Origin.Y + Coords.Origin.Y;
	CombinedOrigin.Z = Origin.Z + Coords.Origin.Z;

	// Do loop.
	for( INT i=0; i<FrameVerts; i++ )
	{
		// Get source points.
		FVector V1,V2;
		V1.X=MeshVertex1[i].X, V1.Y=MeshVertex1[i].Y, V1.Z=MeshVertex1[i].Z;
		V2.X=MeshVertex2[i].X, V2.Y=MeshVertex2[i].Y, V2.Z=MeshVertex2[i].Z;
		// Calculate cached values.
		CachedVerts[i].X = V1.X + (V2.X-V1.X)*Alpha;
		CachedVerts[i].Y = V1.Y + (V2.Y-V1.Y)*Alpha;
		CachedVerts[i].Z = V1.Z + (V2.Z-V1.Z)*Alpha;
		// Transform and place result in result verts.
		FLOAT X,Y,Z;
		X = CachedVerts[i].X - CombinedOrigin.X /* Origin.X - Coords.Origin.X*/;
		Y = CachedVerts[i].Y - CombinedOrigin.Y /* Origin.Y - Coords.Origin.Y*/;
		Z = CachedVerts[i].Z - CombinedOrigin.Z /* Origin.Z - Coords.Origin.Z*/;
		ResultVerts->X = X*Coords.XAxis.X + Y*Coords.XAxis.Y + Z*Coords.XAxis.Z;
		ResultVerts->Y = X*Coords.YAxis.X + Y*Coords.YAxis.Y + Z*Coords.YAxis.Z;
		ResultVerts->Z = X*Coords.ZAxis.X + Y*Coords.ZAxis.Y + Z*Coords.ZAxis.Z;
		// Increment to next result vert.
		*(BYTE**)&ResultVerts += ResultVertSize;
	}
#endif
}

//
// K6 3D Optimized version of the tween loop of UMesh::GetFrame
// This routine is almost identical to DoInterpolateLoop except CacheVerts is used
// in place of MeshVertex1
//
static _inline void DoTweenLoop(FMeshVert* MeshVertex,INT FrameVerts,
								FVector& Origin,FLOAT Alpha,FCoords& Coords,
								FVector* CachedVerts,FVector* ResultVerts, INT ResultVertSize)
{
#if 1
	FVector CombinedOrigin;
	FLOAT TmpAlpha=Alpha; // Bloody compiler.
	struct
	{
		float X,Y;
	} Vertex;

	_asm
	{
		femms

		// Calculate combined origin.
		mov		ebx,Origin
		mov		edx,Coords
		movq	mm2,[ebx]FVector.X
		movq	mm4,[edx]FCoords.Origin.X
		pfadd	(m4,m2)
		movd	mm3,[ebx]FVector.Z
		movd	mm5,[edx]FCoords.Origin.Z
		pfadd	(m5,m3)
		movq	CombinedOrigin.X,mm4
		movd	CombinedOrigin.Z,mm5

		// Set up for loop.
		mov		eax,MeshVertex
		mov		ecx,FrameVerts
		mov		esi,ResultVerts
		mov		edi,CachedVerts
		sub		esi,4				// Move ESI back a bit to prevent plain [esi] addressing.
		cmp		ecx,0
		jz		Done

InterpolateLoop:
		// Expand packed FMeshVerts
		mov		FrameVerts,ecx		// Save count, mm7=0|A
		movd	mm7,TmpAlpha
		mov		ebx,[eax]			// Get packed V1
		shl		ebx,21				// Extract X1
		sar		ebx,21
		mov		ecx,[eax]			// Get packed V1
		mov		Vertex.X,ebx		// Save X1
		shl		ecx,10				// Extract Y1
		sar		ecx,21
		mov		edx,[eax]			// Get packed V1
		mov		Vertex.Y,ecx		// Save Y1
		add		eax,TYPE FMeshVert	// Increment MeshVertex1 ptr
		mov		MeshVertex,eax		// Save updated MeshVertex1
		sar		edx,22				// Extract Z1
		movq	mm2,Vertex.X		// mm0=(int)Y1|X1 
		movd	mm3,edx				// Save Z1 directly to MMX register (mm1=(int)0|Z1)
		// Now do interpolation and transformation.
		movq	mm0,[edi]FVector.X		// mm0=Y1|X1, edx=ptr to Coords
		mov		ecx,Coords
		pi2fd	(m2,m2)					// mm2=Y2|X2, mm7=A|A
		punpckldq mm7,mm7
		pfsub	(m2,m0)					// mm2=Y2-Y1|X2-X1, mm1=0|Z1
		movd	mm1,[edi]FVector.Z
		pfmul	(m2,m7)					// mm2=(Y2-Y1)A|(X2-X1)A, mm3=0|Z2
		pi2fd	(m3,m3)
		pfsub	(m3,m1)					// mm3=0|Z2-Z1, mm4=Yo|Xo
		movq	mm4,CombinedOrigin.X
		pfmul	(m3,m7)					// mm3=0|(Z2-Z1)A, mm5=0|Zo
		movd	mm5,CombinedOrigin.Z
		pfadd	(m0,m2)					// mm0=Y1+(Y2-Y1)A|X1+(X2-X1)A=Ycv|Xcv, mm6=Yxa|Xxa
		movq	mm6,[ecx]FCoords.XAxis.X
		pfadd	(m1,m3)					// mm1=0|Z1+(Z2-Z1)A=0|Zcv, mm7=0|Zxa
		movd	mm7,[ecx]FCoords.XAxis.Z
		pfsubr	(m4,m0)					// mm4=Ycv-Yo|Xcv-Xo=Y|X, mm2=Yya|Xya
		movq	mm2,[ecx]FCoords.YAxis.X
		pfsubr	(m5,m1)					// mm5=0|Z0-Zcv=0|Z, mm3=0|Zya
		movd	mm3,[ecx]FCoords.YAxis.Z
		pfmul	(m6,m4)					// mm6=YxaY|XxaX=Yx|Xx, save Ycv|Xcv
		movq	[edi]FVector.X,mm0
		pfmul	(m7,m5)					// mm7=0|ZxaZ=0|Zx, mm0=Yza|Xza
		movq	mm0,[ecx]FCoords.ZAxis.X
		pfmul	(m2,m4)					// mm2=YyaY|XyaX=Yy|Xy, save Zcv
		movd	[edi]FVector.Z,mm1
		pfmul	(m3,m5)					// mm3=0|ZyaZ=0|Zy, mm1=0|Zza
		movd	mm1,[ecx]FCoords.ZAxis.Z
		pfmul	(m4,m0)					// mm4=YzaY|XzaX=Yz|Xz,mm6=Xy+Yy|Xx+Yx
		pfacc	(m6,m2)
		punpckldq mm7,mm3				// mm7=Zy|Zx, mm5=0|ZzaZ=0|Zz
		pfmul	(m5,m1)
		pfacc	(m4,m4)					// mm4=Yz+Xz|Yz+Xz, ecx=Count
		mov		ecx,FrameVerts			
		pfadd	(m6,m7)					// mm6=Xy+Yy+Zy|Xx+Yx+Zx=Y'|X', eax=ptr to MeshVertex1
		mov		eax,MeshVertex
		pfadd	(m5,m4)					// mm5=Yz+Xz|Zz+Yz+Xz=?|Z', inc ptr to CachedVerts
		add		edi,TYPE FVector
		movq	[esi+4]FVector.X,mm6	// save Y'|X', save Z'
		movd	[esi+4]FVector.Z,mm5
		add		esi,ResultVertSize		// inc ptr to ResultVerts, loop
		dec		ecx
		jnz		InterpolateLoop
Done:
		femms
	}
#else
	// Calculate combined origin.
	FVector CombinedOrigin;
	CombinedOrigin.X = Origin.X + Coords.Origin.X;
	CombinedOrigin.Y = Origin.Y + Coords.Origin.Y;
	CombinedOrigin.Z = Origin.Z + Coords.Origin.Z;

	// Do loop.
	for( INT i=0; i<FrameVerts; i++ )
	{
		// Get source points.
		FLOAT X2,Y2,Z2;
		X2=MeshVertex[i].X, Y2=MeshVertex[i].Y, Z2=MeshVertex[i].Z;
		// Calculate cached value.
		CachedVerts[i].X += (X2 - CachedVerts[i].X)*Alpha;
		CachedVerts[i].Y += (Y2 - CachedVerts[i].Y)*Alpha;
		CachedVerts[i].Z += (Z2 - CachedVerts[i].Z)*Alpha;
		// Transform and place result in result verts.
		FLOAT X,Y,Z;
		X = CachedVerts[i].X - CombinedOrigin.X /* Origin.X - Coords.Origin.X*/;
		Y = CachedVerts[i].Y - CombinedOrigin.Y /* Origin.Y - Coords.Origin.Y*/;
		Z = CachedVerts[i].Z - CombinedOrigin.Z /* Origin.Z - Coords.Origin.Z*/;
		ResultVerts->X = X*Coords.XAxis.X + Y*Coords.XAxis.Y + Z*Coords.XAxis.Z;
		ResultVerts->Y = X*Coords.YAxis.X + Y*Coords.YAxis.Y + Z*Coords.YAxis.Z;
		ResultVerts->Z = X*Coords.ZAxis.X + Y*Coords.ZAxis.Y + Z*Coords.ZAxis.Z;
		// Increment to next result vert.
		*(BYTE**)&ResultVerts += ResultVertSize;
	}
#endif
}

//
// Get the transformed point set corresponding to the animation frame 
// of this primitive owned by Owner. Returns the total outcode of the points.
//
__inline void UMesh::AMD3DGetFrame
(
	FVector*	ResultVerts,
	INT			Size,
	FCoords		Coords,
	AActor*		Owner
)
{
	guard(UMesh::AMD3DGetFrame);

	// Create or get cache memory.
	FCacheItem* Item;
	UBOOL WasCached = 1;
	QWORD CacheID   = MakeCacheID( CID_TweenAnim, Owner, NULL );
	BYTE* Mem       = GCache.Get( CacheID, Item );
	if( Mem==NULL || *(UMesh**)Mem!=this )
	{
		if( Mem != NULL )
		{
			// Actor's mesh changed.
			Item->Unlock();
			GCache.Flush( CacheID );
		}
		Mem = GCache.Create( CacheID, Item, sizeof(UMesh*) + sizeof(FLOAT) + sizeof(FName) + FrameVerts * sizeof(FVector) );
		WasCached = 0;
	}
	UMesh*& CachedMesh  = *(UMesh**)Mem; Mem += sizeof(UMesh*);
	FLOAT&  CachedFrame = *(FLOAT *)Mem; Mem += sizeof(FLOAT );
	FName&  CachedSeq   = *(FName *)Mem; Mem += sizeof(FName);
	if( !WasCached )
	{
		CachedMesh  = this;
		CachedSeq   = NAME_None;
		CachedFrame = 0.0;
	}

	// Get stuff.
	FLOAT    DrawScale      = Owner->bParticles ? 1.0 : Owner->DrawScale;
	FVector* CachedVerts    = (FVector*)Mem;
	Coords                  = Coords * (Owner->Location + Owner->PrePivot) * Owner->Rotation * RotOrigin * FScale(Scale * DrawScale,0.0,SHEER_None);
	const FMeshAnimSeq* Seq = GetAnimSeq( Owner->AnimSequence );

	// Transform all points into screenspace.
	if( Owner->AnimFrame>=0.0 || !WasCached )
	{
		// Compute interpolation numbers.
		FLOAT Alpha=0.0;
		INT iFrameOffset1=0, iFrameOffset2=0;
		if( Seq )
		{
			FLOAT Frame   = ::Max(Owner->AnimFrame,0.f) * Seq->NumFrames;
			INT iFrame    = appFloor(Frame);
			Alpha         = Frame - iFrame;
			iFrameOffset1 = (Seq->StartFrame + ((iFrame + 0) % Seq->NumFrames)) * FrameVerts;
			iFrameOffset2 = (Seq->StartFrame + ((iFrame + 1) % Seq->NumFrames)) * FrameVerts;
		}

		// Interpolate two frames.
		FMeshVert* MeshVertex1 = &Verts( iFrameOffset1 );
		FMeshVert* MeshVertex2 = &Verts( iFrameOffset2 );
//		for( INT i=0; i<FrameVerts; i++ )
//		{
//			FVector V1( MeshVertex1[i].X, MeshVertex1[i].Y, MeshVertex1[i].Z );
//			FVector V2( MeshVertex2[i].X, MeshVertex2[i].Y, MeshVertex2[i].Z );
//			CachedVerts[i] = V1 + (V2-V1)*Alpha;
//			*ResultVerts = (CachedVerts[i] - Origin).TransformPointBy(Coords);
//			*(BYTE**)&ResultVerts += Size;
//		}
		DoInterpolateLoop(MeshVertex1,MeshVertex2,FrameVerts,
						  Origin,Alpha,Coords,
						  CachedVerts,ResultVerts,Size);
	}
	else
	{
		// Compute tweening numbers.
		FLOAT StartFrame = Seq ? (-1.0 / Seq->NumFrames) : 0.0;
		INT iFrameOffset = Seq ? Seq->StartFrame * FrameVerts : 0;
		FLOAT Alpha = 1.0 - Owner->AnimFrame / CachedFrame;
		if( CachedSeq!=Owner->AnimSequence || Alpha<0.0 || Alpha>1.0)
		{
			CachedSeq   = Owner->AnimSequence;
			CachedFrame = StartFrame;
			Alpha       = 0.0;
		}

		// Tween all points.
		FMeshVert* MeshVertex = &Verts( iFrameOffset );
//		for( INT i=0; i<FrameVerts; i++ )
//		{
//			FVector V2( MeshVertex[i].X, MeshVertex[i].Y, MeshVertex[i].Z );
//			CachedVerts[i] += (V2 - CachedVerts[i]) * Alpha;
//			*ResultVerts = (CachedVerts[i] - Origin).TransformPointBy(Coords);
//			*(BYTE**)&ResultVerts += Size;
//		}
		DoTweenLoop(MeshVertex,FrameVerts,
					Origin,Alpha,Coords,
					CachedVerts,ResultVerts,Size);

		// Update cached frame.
		CachedFrame = Owner->AnimFrame;
	}
	Item->Unlock();
	unguardobj;
}

#pragma warning( default : 4799 )
#pragma pack()
#endif

//
// Get the transformed point set corresponding to the animation frame 
// of this primitive owned by Owner. Returns the total outcode of the points.
//
void UMesh::GetFrame
(
	FVector*	ResultVerts,
	INT			Size,
	FCoords		Coords,
	AActor*		Owner
)
{
#ifndef NOAMD3D
	if (GIsK63D)
	{
		AMD3DGetFrame(ResultVerts,Size,Coords,Owner);
		return;
	}
#endif
	guard(UMesh::GetFrame);

	// Create or get cache memory.
	FCacheItem* Item;
	UBOOL WasCached = 1;
	QWORD CacheID   = MakeCacheID( CID_TweenAnim, Owner, NULL );
	BYTE* Mem       = GCache.Get( CacheID, Item );
	if( Mem==NULL || *(UMesh**)Mem!=this )
	{
		if( Mem != NULL )
		{
			// Actor's mesh changed.
			Item->Unlock();
			GCache.Flush( CacheID );
		}
		Mem = GCache.Create( CacheID, Item, sizeof(UMesh*) + sizeof(FLOAT) + sizeof(FName) + FrameVerts * sizeof(FVector) );
		WasCached = 0;
	}
	UMesh*& CachedMesh  = *(UMesh**)Mem; Mem += sizeof(UMesh*);
	FLOAT&  CachedFrame = *(FLOAT *)Mem; Mem += sizeof(FLOAT );
	FName&  CachedSeq   = *(FName *)Mem; Mem += sizeof(FName);
	if( !WasCached )
	{
		CachedMesh  = this;
		CachedSeq   = NAME_None;
		CachedFrame = 0.0;
	}

	// Get stuff.
	FLOAT    DrawScale      = Owner->bParticles ? 1.0 : Owner->DrawScale;
	FVector* CachedVerts    = (FVector*)Mem;
	Coords                  = Coords * (Owner->Location + Owner->PrePivot) * Owner->Rotation * RotOrigin * FScale(Scale * DrawScale,0.0,SHEER_None);
	const FMeshAnimSeq* Seq = GetAnimSeq( Owner->AnimSequence );

	// Transform all points into screenspace.
	if( Owner->AnimFrame>=0.0 || !WasCached )
	{
		// Compute interpolation numbers.
		FLOAT Alpha=0.0;
		INT iFrameOffset1=0, iFrameOffset2=0;
		if( Seq )
		{
			FLOAT Frame   = ::Max(Owner->AnimFrame,0.f) * Seq->NumFrames;
			INT iFrame    = appFloor(Frame);
			Alpha         = Frame - iFrame;
			iFrameOffset1 = (Seq->StartFrame + ((iFrame + 0) % Seq->NumFrames)) * FrameVerts;
			iFrameOffset2 = (Seq->StartFrame + ((iFrame + 1) % Seq->NumFrames)) * FrameVerts;
		}

		// Interpolate two frames.
		FMeshVert* MeshVertex1 = &Verts( iFrameOffset1 );
		FMeshVert* MeshVertex2 = &Verts( iFrameOffset2 );
		for( INT i=0; i<FrameVerts; i++ )
		{
			FVector V1( MeshVertex1[i].X, MeshVertex1[i].Y, MeshVertex1[i].Z );
			FVector V2( MeshVertex2[i].X, MeshVertex2[i].Y, MeshVertex2[i].Z );
			CachedVerts[i] = V1 + (V2-V1)*Alpha;
			*ResultVerts = (CachedVerts[i] - Origin).TransformPointBy(Coords);
			*(BYTE**)&ResultVerts += Size;
		}
	}
	else
	{
		// Compute tweening numbers.
		FLOAT StartFrame = Seq ? (-1.0 / Seq->NumFrames) : 0.0;
		INT iFrameOffset = Seq ? Seq->StartFrame * FrameVerts : 0;
		FLOAT Alpha = 1.0 - Owner->AnimFrame / CachedFrame;
		if( CachedSeq!=Owner->AnimSequence || Alpha<0.0 || Alpha>1.0)
		{
			CachedSeq   = Owner->AnimSequence;
			CachedFrame = StartFrame;
			Alpha       = 0.0;
		}

		// Tween all points.
		FMeshVert* MeshVertex = &Verts( iFrameOffset );
		for( INT i=0; i<FrameVerts; i++ )
		{
			FVector V2( MeshVertex[i].X, MeshVertex[i].Y, MeshVertex[i].Z );
			CachedVerts[i] += (V2 - CachedVerts[i]) * Alpha;
			*ResultVerts = (CachedVerts[i] - Origin).TransformPointBy(Coords);
			*(BYTE**)&ResultVerts += Size;
		}

		// Update cached frame.
		CachedFrame = Owner->AnimFrame;
	}
	Item->Unlock();
	unguardobj;
}

/*-----------------------------------------------------------------------------
	UMesh constructor.
-----------------------------------------------------------------------------*/

//
// UMesh constructor.
//
UMesh::UMesh( INT NumPolys, INT NumVerts, INT NumFrames )
{
	guard(UMesh::UMesh);

	// Set counts.
	FrameVerts	= NumVerts;
	AnimFrames	= NumFrames;

	// Allocate all stuff.
	Tris			.Add(NumPolys);
	Verts			.Add(NumVerts * NumFrames);
	Connects		.Add(NumVerts);
	BoundingBoxes	.Add(NumFrames);
	BoundingSpheres	.Add(NumFrames);

	// Init textures.
	for( int i=0; i<Textures.Num(); i++ )
		Textures(i) = NULL;

	unguardobj;
}

/*-----------------------------------------------------------------------------
	The end.
-----------------------------------------------------------------------------*/
