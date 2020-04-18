/*=============================================================================
	UnRender.cpp: Main Unreal rendering functions and pipe
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "RenderPrivate.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

#if STATS
FRenderStats GStat;
#endif

/*-----------------------------------------------------------------------------
	Object implementations.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(URender);

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// URender statics.
DWORD 								URender::Stamp;
FMemStack							URender::VectorMem;
URender::FStampedPoint*				URender::PointCache;
URender::FDynamicsCache*			URender::DynamicsCache;
INT									URender::NumDynLightSurfs;
INT									URender::NumDynLightLeaves;
INT									URender::MaxSurfLights;
INT									URender::MaxLeafLights;
FActorLink**						URender::SurfLights=NULL;
FVolActorLink**						URender::LeafLights=NULL;
INT									URender::DynLightSurfs[MAX_DYN_LIGHT_SURFS];
INT									URender::DynLightLeaves[MAX_DYN_LIGHT_LEAVES];

// Optimization globals.
INT         GFrameStamp=0;
FSceneNode* GFrame;
FBspNode*   GNodes;
FBspSurf*   GSurfs;
FVert*      GVerts;
FVector*    GPoints;

/*-----------------------------------------------------------------------------
	URender init & exit.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
URender::URender()
{
	guard(URender::URender);

	// Validate stuff.
	if(sizeof(*this)!=GetClass()->GetPropertiesSize())
		appErrorf( "Render size mismatch: C=%i U=%i", sizeof(*this), GetClass()->GetPropertiesSize() );
	check(FBspNode::MAX_NODE_VERTICES<=FPoly::MAX_VERTICES);
	check(sizeof(FVector)==12);
	check(sizeof(FRotator)==12);
	check(sizeof(FCoords)==48);

	unguard;
}

//
// Static class initializer.
//
void URender::InternalClassInitializer( UClass* Class )
{
	guard(URender::InternalClassInitializer);
	unguard;
}

//
// Initialize the rendering engine and allocate all temporary buffers.
// Calls appError if failure.
//
void URender::Init( UEngine* InEngine )
{
	guard(URender::Init);

	// Call base.
	URenderBase::Init( InEngine );

	// Set global render pointer.
	GRender = this;

	// Init.
	NumDynLightSurfs  = 0;
	NumDynLightLeaves = 0;

	// Allocate rendering stuff.
	PointCache		= new FStampedPoint [MAX_POINTS];
	DynamicsCache   = new FDynamicsCache[MAX_NODES];
	appMemset( DynamicsCache, 0, MAX_NODES * sizeof(FDynamicsCache) );

	GCache.Flush();

	// Caches.
	for( INT i=0; i<MAX_POINTS;  i++ )
		PointCache [i].Stamp = Stamp;
	VectorMem.Init( 16384 );

	// Init stats.
	STAT(appMemset(&GStat,0,sizeof(GStat));)

	// Light manager.
	GLightManager->Init();

	debugf( NAME_Init, "Rendering initialized" );
	unguard;
}

//
// Shut down the rendering engine
//
void URender::Destroy()
{
	guard(URender::Destroy);

	delete PointCache;
	delete DynamicsCache;
	if( SurfLights ) appFree(SurfLights);
	if( LeafLights ) appFree(LeafLights);

	GLightManager->Exit();
	VectorMem.Exit();

	debugf( NAME_Exit, "Rendering shut down" );

	URenderBase::Destroy();
	unguard;
}

/*-----------------------------------------------------------------------------
	URender Stats display.
-----------------------------------------------------------------------------*/

enum {STAT_Y = 16};

void URender::DrawStats( FSceneNode* Frame )
{
	guard(URender::DrawStats);
	ThisEndTime      = appCycles();
	DWORD FrameTime  = ThisEndTime - LastEndTime;
	DWORD RenderTime = ThisEndTime - ThisStartTime;
	char TempStr[256];

	if( FpsStats )
	{
		INT XL,YL;
		appSprintf
		(
			TempStr,
			"Frame=%05.1f MSEC Render=%05.1f MSEC Nodes=%03i Polys=%03i",
			GSecondsPerCycle*1000 * FrameTime,
			GSecondsPerCycle*1000 * RenderTime,
			NodesDraw,
			PolysDraw
		);
		Frame->Viewport->Canvas->StrLen( Frame->Viewport->Canvas->SmallFont, XL, YL, TempStr );
		INT Y=Frame->Y;
		Frame->Viewport->Canvas->Printf( Frame->Viewport->Canvas->SmallFont, (Frame->X-XL)/2, Y-YL-2, "%s", TempStr );
	}
#if STATS
	INT	StatYL=0;
	if( GlobalStats )
	{
		ShowStat( Frame, StatYL, "GLOBAL:" );
		ShowStat
		(
			Frame,
			StatYL,
			"  FRAME=%04.1f: GAME=%04.1f CLI=%04.1f BLIT=%04.1f PLAT=%04.1f",
			GSecondsPerCycle*1000 * FrameTime,
			GSecondsPerCycle*1000 * Engine->GameCycles,
			GSecondsPerCycle*1000 * (Engine->ClientCycles-Frame->Viewport->Client->DrawCycles),
			GSecondsPerCycle*1000 * Frame->Viewport->Client->DrawCycles,
			GSecondsPerCycle*1000 * Abs((INT)FrameTime - (INT)Engine->TickCycles)
		);
		Frame->Viewport->Actor->XLevel->GetStats( TempStr );
		ShowStat( Frame, StatYL, "  %s", TempStr );
		ShowStat
		(
			Frame,
			StatYL,
			"  RENDER=%04.1f MESH=%04.1f POLYV=%04.1f ILLUM=%04.1f OCC=%04.1f FILT=%04.1f EX=%04.1f",
			GSecondsPerCycle*1000 * RenderTime,
			GSecondsPerCycle*1000 * GStat.MeshTime,
			GSecondsPerCycle*1000 * GStat.PolyVTime,
			GSecondsPerCycle*1000 * GStat.IllumTime,
			GSecondsPerCycle*1000 * GStat.OcclusionTime,
			GSecondsPerCycle*1000 * GStat.FilterTime,
			GSecondsPerCycle*1000 * GStat.ExtraTime
		);
		ShowStat( Frame, StatYL, "" );
	}
	if( HardwareStats )
	{
		ShowStat( Frame, StatYL, "HARDWARE:" );
		Frame->Viewport->RenDev->GetStats( TempStr );
		ShowStat( Frame, StatYL, "  %s", TempStr );
		ShowStat( Frame, StatYL, "" );
	}
	if( PolyVStats )
	{
	}
	if( PolyCStats )
	{
	}
	if( IllumStats )
	{
	}
	if( MeshStats )
	{
		ShowStat( Frame, StatYL, "MESH: %04.1f", GSecondsPerCycle*1000 * GStat.MeshTime );
		ShowStat
		(
			Frame,
			StatYL,
			"  GetFrame=%04.1f Process=%04.1f LightSet=%04.1f Light=%04.1f",
			GSecondsPerCycle*1000 * GStat.MeshGetFrameTime,
			GSecondsPerCycle*1000 * GStat.MeshProcessTime,
			GSecondsPerCycle*1000 * GStat.MeshLightSetupTime,
			GSecondsPerCycle*1000 * GStat.MeshLightTime
		);
		ShowStat
		(
			Frame,
			StatYL,
			"  Sub=%04.1f Clip=%04.1f Tmap=%04.1f",
			GSecondsPerCycle*1000 * GStat.MeshSubTime,
			GSecondsPerCycle*1000 * GStat.MeshClipTime,
			GSecondsPerCycle*1000 * GStat.MeshTmapTime
		);
		ShowStat
		(
			Frame,
			StatYL,
			"  MeshCount=%i MeshPolyCount=%i MeshSubCount=%i MeshLights=%i MeshVtrics=%i VertLights=%i",
			GStat.MeshCount,
			GStat.MeshPolyCount,
			GStat.MeshSubCount,
			GStat.MeshLightCount,
			GStat.MeshVtricCount,
			GStat.MeshVertLightCount
		);
		ShowStat( Frame, StatYL, "" );
	}
	if( ActorStats )
	{
	}
	if( FilterStats )
	{
	}
	if( RejectStats )
	{
	}
	if( SpanStats )
	{
	}
	if( ZoneStats )
	{
		ShowStat
		(
			Frame,
			StatYL,
			"Zones: Visible=%i/%i Reject=%i",
			GStat.VisibleZones,
			GStat.NumZones,
			GStat.MaskRejectZones
		);
		ShowStat( Frame, StatYL, "" );
	}
	if( LightStats )
	{
	}
	if( OcclusionStats )
	{
		ShowStat
		(
			Frame,
			StatYL,
			"Occlusion=%04.1f:",
			GSecondsPerCycle*1000 * GStat.OcclusionTime
		);
		ShowStat
		(
			Frame,
			StatYL,
			"   Clip=%04.1f Raster=%04.1f Span=%04.1f Visit=%i/%i Points=%i",
			GSecondsPerCycle*1000 * GStat.ClipTime,
			GSecondsPerCycle*1000 * GStat.RasterTime,
			GSecondsPerCycle*1000 * GStat.SpanTime,
			GStat.NodesDone,
			GStat.NodesTotal,
			GStat.NumPoints
		);
		ShowStat
		(
			Frame,
			StatYL,
			"   Transform=%i Clip=%i Raster=%i RasterAccept=%i DrawNodes=%i",
			GStat.NumTransform,
			GStat.NumClip,
			GStat.NumRasterPolys+GStat.NumRasterBoxReject,
			GStat.NumRasterPolys,
			NodesDraw
		);
		ShowStat
		(
			Frame,
			StatYL,
			"   BoxTime=%04.1f BoxChecks=%i BoxBacks=%i BoxIn=%i BoxOutPyr=%i BoxSpanOcc=%i",
			GSecondsPerCycle*1000 * GStat.BoxTime,
			GStat.BoxChecks,
			GStat.BoxBacks,
			GStat.BoxIn,
			GStat.BoxOutOfPyramid,
			GStat.BoxSpanOccluded
		);
		ShowStat( Frame, StatYL, "" );
	}
	if( GameStats )
	{
		ShowStat( Frame, StatYL, "GAME:" );
		Frame->Viewport->Actor->XLevel->GetStats( TempStr );
		ShowStat( Frame, StatYL, "   %s", TempStr );
		ShowStat( Frame, StatYL, "" );
	}
	if( SoftStats )
	{
	}
	if( CacheStats )
	{
		ShowStat( Frame, StatYL, "CACHE:" );
		GCache.Status( TempStr );
		ShowStat( Frame, StatYL, "   %s", TempStr );
		ShowStat( Frame, StatYL, "" );
	}
#endif // STATS
	unguard;
}

//
// Show one statistic and update the pointer.
//
void URender::ShowStat( FSceneNode* Frame, INT& StatYL, const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);

	guard(URender::ShowStat);
	INT XL, YL;
	Frame->Viewport->Canvas->Printf( Frame->Viewport->Canvas->SmallFont, 8, STAT_Y + StatYL, "%s", TempStr );
    Frame->Viewport->Canvas->StrLen( Frame->Viewport->Canvas->SmallFont, XL, YL, " " );
	StatYL += YL;
	unguard;
}

/*-----------------------------------------------------------------------------
	URender PreRender & PostRender.
-----------------------------------------------------------------------------*/

//
// Set up for rendering a frame.
//
static FMemMark Mark;
void URender::PreRender( FSceneNode* Frame )
{
	guard(URender::PreRender);

	// Set math to low precision.
	appEnableFastMath(1);

	// Tick stuff.
	GRandoms->Tick( Frame->Viewport->Actor->XLevel->GetLevelInfo()->TimeSeconds );

	// Init stats.
	STAT(appMemset(&GStat,0,sizeof(GStat)));
	LastEndTime   = ThisEndTime;
	ThisStartTime = appCycles();

	// Init counts.
	NodesDraw		= 0;
	PolysDraw		= 0;

	// Bump the iteration count.
	Mark = FMemMark(GMem);

	unguard;
}

//
// Clean up after rendering a frame.
//
void URender::PostRender( FSceneNode* Frame )
{
	guard(URender::PostRender);

	// Draw whatever stats were requested.
	if( Frame->Viewport->Actor->RendMap==REN_Polys || Frame->Viewport->Actor->RendMap==REN_PolyCuts || Frame->Viewport->Actor->RendMap==REN_DynLight || Frame->Viewport->Actor->RendMap==REN_PlainTex )
		DrawStats( Frame );

	// Restore default precision.
	appEnableFastMath(0);
	unguard;
}

/*-----------------------------------------------------------------------------
	URender command line.
-----------------------------------------------------------------------------*/

//
// Execute a command line.
//
UBOOL URender::Exec(const char *Cmd,FOutputDevice *Out)
{
	guard(URender::Exec);
	const char* Str = Cmd;

	if( ParseCommand(&Str,"STAT") )
	{
		if( ParseCommand(&Str,"Fps"         ) ) FpsStats       ^= 1;
		if( ParseCommand(&Str,"Global"      ) ) GlobalStats    ^= 1;
		if( ParseCommand(&Str,"Mesh"        ) ) MeshStats      ^= 1;
		if( ParseCommand(&Str,"Actor"       ) ) ActorStats     ^= 1;
		if( ParseCommand(&Str,"Filter"      ) ) FilterStats    ^= 1;
		if( ParseCommand(&Str,"Reject"      ) ) RejectStats    ^= 1;
		if( ParseCommand(&Str,"Span"        ) ) SpanStats      ^= 1;
		if( ParseCommand(&Str,"Zone"        ) ) ZoneStats      ^= 1;
		if( ParseCommand(&Str,"Light"       ) ) LightStats     ^= 1;
		if( ParseCommand(&Str,"Occlusion"   ) ) OcclusionStats ^= 1;
		if( ParseCommand(&Str,"Game"        ) ) GameStats      ^= 1;
		if( ParseCommand(&Str,"Soft"        ) ) SoftStats      ^= 1;
		if( ParseCommand(&Str,"Cache"       ) ) CacheStats     ^= 1;
		if( ParseCommand(&Str,"PolyV"       ) ) PolyVStats     ^= 1;
		if( ParseCommand(&Str,"PolyC"       ) ) PolyCStats     ^= 1;
		if( ParseCommand(&Str,"Illum"       ) ) IllumStats     ^= 1;
		if( ParseCommand(&Str,"Hardware"    ) ) HardwareStats  ^= 1;
		return 1;
	}
	else if( ParseCommand(&Str,"REND") )
	{
		if      (ParseCommand(&Str,"LEAK"))			LeakCheck		^= 1;
		else if (ParseCommand(&Str,"T"))			Toggle			^= 1;
		else return 0;
		Out->Log( "Rendering option recognized" );
		return 1;
	}
	else return 0; // Not executed
	unguard;
}

/*--------------------------------------------------------------------------
	Pipe.
--------------------------------------------------------------------------*/

//
// Basic transform, outcode, project.
//
static void Pipe( FTransform& Result, const FSceneNode* Frame, const FVector& InVector )
{
	static FLOAT Half=0.5;
	static FLOAT ClipXM, ClipXP, ClipYM, ClipYP;
	static const BYTE OutXMinTab [2] = { 0, FVF_OutXMin };
	static const BYTE OutXMaxTab [2] = { 0, FVF_OutXMax };
	static const BYTE OutYMinTab [2] = { 0, FVF_OutYMin };
	static const BYTE OutYMaxTab [2] = { 0, FVF_OutYMax };
#if ASM
	__asm
	{
		; Load pointers.
		mov	    esi, [Frame]
		mov     ecx, [Result]
		mov     eax, [InVector]
		lea     edx, [esi]FSceneNode.Coords

		; Transform the point.
		mov		esi, [Frame]
		mov     eax, [InVector]
		lea     edx, [esi]FSceneNode.Coords
		mov     ecx, [Result]

		fld     dword ptr [eax+0]
		fld     dword ptr [eax+4]
		fld     dword ptr [eax+8]
		fxch    st(2)

		fsub    dword ptr [edx + 0]
		fxch    st(1)
		fsub	dword ptr [edx + 4]
		fxch    st(2)
		fsub	dword ptr [edx + 8]
		fxch    st(1)

		fld     st(0)
        fmul    dword ptr [edx+12]
        fld     st(1)
        fmul    dword ptr [edx+24]
		fxch    st(2)
		fmul    dword ptr [edx+36]
		fxch    st(4)

		fld     st(0)
		fmul    dword ptr [edx+16]     
		fld     st(1)
        fmul    dword ptr [edx+28]    
		fxch    st(2)
		fmul    dword ptr [edx+40]
		fxch    st(1)

        faddp   st(3),st(0)
        faddp   st(5),st(0)
        faddp   st(2),st(0)
		fxch    st(2)

		fld     st(0)
		fmul    dword ptr [edx+20]     
		fld     st(1)
        fmul    dword ptr [edx+32]      
		fxch    st(2)
		fmul    dword ptr [edx+44]
		fxch    st(1)

		faddp   st(4),st(0)
		faddp   st(4),st(0)
		faddp   st(1),st(0)
		fxch    st(1)                   ; X Y Z

		fstp    dword ptr [ecx+0]       ; X
        fstp    dword ptr [ecx+4]       ; Y                     
        fstp    dword ptr [ecx+8]       ; Z

		; Compute clipping numbers.
		fld  [ecx]FVector.Z				; Z
		fld  [ecx]FVector.Z				; Z Z
		fxch							; Z Z
		fmul [esi]FSceneNode.PrjXM		; Z*ProjZM Z
		fxch							; Z Z*ProjXM
		fmul [esi]FSceneNode.PrjYM		; Z*ProjYM Z*ProjXM
		fld  [ecx]FVector.Z				; Z Z*ProjYM Z*ProjXM
		fld  [ecx]FVector.Z				; Z Z Z*ProjYM Z*ProjXM
		fxch                            ; Z Z Z*ProjYM Z*ProjXM
		fmul [esi]FSceneNode.PrjXP      ; Z*ProjXP Z Z*ProjYM Z*ProjXM
		fxch                            ; Z Z*ProjXP Z*ProjYM Z*ProjXM
		fmul [esi]FSceneNode.PrjYP      ; Z*ProjYP Z*ProjXP Z*ProjYM Z*ProjXM
		fxch st(3)                      ; Z*ProjXM Z*ProjXP Z*ProjYM Z*ProjYP
		fadd [ecx]FVector.X             ; X+Z*ProjXM Z*ProjXP Z*ProjYM Z*ProjYP
		fxch st(2)                      ; Z*ProjYM Z*ProjXP X+Z*ProjXM Z*ProjYP
		fadd [ecx]FVector.Y             ; Y+Z*ProjYM Z*ProjXP X+Z*ProjXM Z*ProjYP
		fxch st(1)                      ; Z*ProjXP Y+Z*ProjYM X+Z*ProjXM Z*ProjYP
		fsub [ecx]FVector.X             ; X-Z*ProjXP Y+Z*ProjYM X+Z*ProjXM Z*ProjYP
		fxch st(3)                      ; Z*ProjYP Z+Y*ProjYM Z+X*ProjXM Z+X*ProjXP
		fsub [ecx]FVector.Y             ; Y-Z*ProjYP Z+Y*ProjYM Z+X*ProjXM Z+X*ProjXP
		fxch st(2)                      ; Z+X*ProjXM Z+Y*ProjYM Z+Y*ProjYP Z+X*ProjXP
		fstp ClipXM                     ; Z+Y*ProjYM Z+Y*ProjYP Z+X*ProjXP
		fstp ClipYM                     ; Z+Y*ProjYP Z+X*ProjXP
		fstp ClipYP                     ; Z+X*ProjXP
		fstp ClipXP                     ; (empty)

		; Start speculative 1/Z divide.
		fld  [esi]FSceneNode.Proj_Z     ; ProjZ
		fdiv [ecx]FVector.Z             ; ProjZ/Z

		; Compute flags.
		mov  ebx,ClipXM					; ebx = XM clipping number as integer
		mov  edx,ClipYM					; edx = YM clipping number as integer
		shr  ebx,31						; ebx = XM: 0 iff clip>=0.0, 1 iff clip<0.0
		mov  edi,ClipXP					; edi = XP
		shr  edx,31                     ; edx = YM: 0 or 1
		mov  esi,ClipYP					; esi = YP: 0 or 1
		shr  edi,31						; edi = XP: 0 or 1
		mov  al,OutXMinTab[ebx]			; al = 0 or FVF_OutXMin
		shr  esi,31						; esi = YP: 0 or 1
		mov  bl,OutYMinTab[edx]			; bl = FVF_OutYMin
		or   bl,al						; bl = FVF_OutXMin, FVF_OutYMin
		mov  ah,OutXMaxTab[edi]			; ah = FVF_OutXMax
		or   bl,ah						; bl = FVF_OutXMin, FVF_OutYMin, OutYMax
		mov  al,OutYMaxTab[esi]			; bh = FVF_OutYMax
		or   al,bl                      ; al = FVF_OutYMin and FVF_OutYMax
		mov  [ecx]FOutVector.Flags, al	; Store flags

		; Projection.
		fstp [ecx]FTransform.RZ
		jne SkipProjection
		mov esi, [Frame]
		fld [ecx]FVector.X
		fld [ecx]FVector.Y
		fxch
		fmul [ecx]FTransform.RZ
		fxch
		fmul [ecx]FTransform.RZ
		fxch
		fadd [esi]FSceneNode.FX15
		fxch
		fadd [esi]FSceneNode.FY15
		fxch
		fstp [ecx]FTransform.ScreenX
		fst  [ecx]FTransform.ScreenY
		fsub [Half]
		fistp DWORD PTR [ecx]Result.IntY

		; Finished.
		SkipProjection:
	}
#else
	Result.Point = InVector.TransformPointBy( Frame->Coords );
	ClipXM = Frame->PrjXM * Result.Point.Z + Result.Point.X;
	ClipXP = Frame->PrjXP * Result.Point.Z - Result.Point.X;
	ClipYM = Frame->PrjYM * Result.Point.Z + Result.Point.Y;
	ClipYP = Frame->PrjYP * Result.Point.Z - Result.Point.Y;
	Result.Flags  =
	(	OutXMinTab [ClipXM < 0.0]
	+	OutXMaxTab [ClipXP < 0.0]
	+	OutYMinTab [ClipYM < 0.0]
	+	OutYMaxTab [ClipYP < 0.0]);
	if( !Result.Flags )
	{
		Result.RZ      = Frame->Proj.Z / Result.Point.Z;
		Result.ScreenX = Result.Point.X * Result.RZ + Frame->FX15;
		Result.ScreenY = Result.Point.Y * Result.RZ + Frame->FY15;
		Result.IntY    = appFloor( Result.ScreenY );
	}
#endif
}

//
// Clipping helper.
//
static FLOAT Dot[FBspNode::MAX_FINAL_VERTICES];

#ifndef NOAMD3D
#pragma warning( disable : 4799 )
#pragma pack( 8 )
// Defines
#define MAKE_FP(Val) *((FLOAT *)&(Val))
#define MAKE_DWORD(Val) *((DWORD *)&(Val))

static _inline void DoFemms(void)
{
	_asm femms
}

static _inline void TransformAndComputeOutcodeAndProject(FTransform*       OutPoint,
										  const FVector&    InPoint,
										  const FSceneNode* Frame)
{
#if 1
	// Unions containing QWORDS are used here so the compiler will align them properly.
	static union
	{
		DWORD d[2];
		double q;
	} Negator={0x80000000,0x80000000},PlusMask={FVF_OutXMax,FVF_OutYMax},MinusMask={FVF_OutXMin,FVF_OutYMin};

	_asm
	{
		mov		ebx,InPoint
		mov		ecx,Frame
		mov		edx,OutPoint

		// Approx 27 clocks.
		movq	mm0,[ebx]FVector.X					// mm0=Y|X, mm2=Yo|Xo
		movq	mm2,[ecx]FSceneNode.Coords.Origin.X
		movd	mm1,[ebx]FVector.Z					// mm1=0|Z, mm3=0|Zo
		movd	mm3,[ecx]FSceneNode.Coords.Origin.Z
		pfsub	(m0,m2)								// mm0=Y'|X', mm4=Yz|Xz
		movq	mm4,[ecx]FSceneNode.Coords.ZAxis.X
		pfsub	(m1,m3)								// mm1=0 |Z', mm5=0 |Zz
		movd	mm5,[ecx]FSceneNode.Coords.ZAxis.Z
		pfmul	(m4,m0)								// mm4=YzY'|XzX', mm6=Yx|Xx
		movq	mm6,[ecx]FSceneNode.Coords.XAxis.X
		pfmul	(m5,m1)								// mm5=0   |ZzZ', mm7=0 |Zx
		movd	mm7,[ecx]FSceneNode.Coords.XAxis.Z		
		pfmul	(m6,m0)								// mm6=YxY'|XxX', mm2=Yy|Xy
		movq	mm2,[ecx]FSceneNode.Coords.YAxis.X
		pfmul	(m7,m1)								// mm7=0   |ZxZ', mm3=0 |Zy
		movd	mm3,[ecx]FSceneNode.Coords.YAxis.Z
		pfmul	(m2,m0)								// mm2=YyY'|XyX', mm4=ZzZ'|YzY'+XzX'
		pfacc	(m4,m5)
		pfmul	(m3,m1)								// mm3=0   |ZyZ', mm6=ZxZ'|YxY'+XxX'
		pfacc	(m6,m7)
		pfacc	(m4,m4)								// mm4=Z"|Z", mm0=Ym|Xm
		movq	mm0,[ecx]FSceneNode.PrjXM		
		pfacc	(m2,m3)								// mm2=ZyZ'|YyY'+XyX', mm1=Yp|Xp
		movq	mm1,[ecx]FSceneNode.PrjXP
		pfmul	(m0,m4)								// mm0=YmZ"|XmZ", mm5=0x80000000|0x80000000
		movq	mm5,Negator
		pfmul	(m1,m4)								// mm1=YpZ"|XpZ", mm7=+Mask
		movq	mm7,PlusMask
		pxor	mm0,mm5								// mm0=-YmZ"|-XmZ", mm3=-Mask
		movq	mm3,MinusMask
		pfacc	(m6,m2)								// mm6=Y"|X", save Z"
		movd	[edx]FOutVector.Point.Z,mm4
		pfrcp	(m4,m4)								// mm4=1/Z"|1/Z", mm5=0|Zp
		movd	mm5,[ecx]FSceneNode.Proj.Z
		pfcmpge	(m1,m6)								// mm1=+ClipVal, mm2=FY15 | FX15
		movq	mm2,[ecx]FSceneNode.FX15
		pfcmpgt	(m0,m6)								// mm0=+ClipVal, save Y"|X"
		movq	[edx]FOutVector.Point.X,mm6
		punpckldq mm5,mm5							// mm5=Zp|Zp, mm1=Y+c|X+c
		pandn	mm1,mm7
		pfmul	(m5,m4)								// mm5=Zp1/Z"|Zp1/Z", mm0=Y-c|X-c
		pand	mm0,mm3								
		por		mm0,mm1								// mm0=Yc|Xc, mm1=Yc|Xc
		movq	mm1,mm0
		pfmul	(m6,m5)								// mm6=Zp1/Z"Y"|Zp1/Z"X", mm0=0|Yc
		psrlq	mm0,32								
		movd	[edx]FTransform.RZ,mm5				// Save 1/Z", mm0=Clip Flag
		por		mm0,mm1								
		pfadd	(m6,m2)								// mm6=ScreenY|ScreenX, save flags
		movd	[edx]FOutVector.Flags,mm0			
		pf2id	(m3,m6)								// mm3=IntY|IntX, Save ScreenY|ScreenX
		movq	[edx]FTransform.ScreenX,mm6			
		psrlq	mm3,32								// mm3=0|IntY, Save IntY
		movd	[edx]FTransform.IntY,mm3			
//done:
	}

#else
	FLOAT TmpX,TmpY,TmpZ; // X',Y',Z'

	// Transform.
	TmpX = InPoint.X - Frame->Coords.Origin.X;
	TmpY = InPoint.Y - Frame->Coords.Origin.Y;
	TmpZ = InPoint.Z - Frame->Coords.Origin.Z;

	OutPoint->Point.X = TmpX*Frame->Coords.XAxis.X + TmpY*Frame->Coords.XAxis.Y + TmpZ*Frame->Coords.XAxis.Z;
	OutPoint->Point.Y = TmpX*Frame->Coords.YAxis.X + TmpY*Frame->Coords.YAxis.Y + TmpZ*Frame->Coords.YAxis.Z;
	OutPoint->Point.Z = TmpX*Frame->Coords.ZAxis.X + TmpY*Frame->Coords.ZAxis.Y + TmpZ*Frame->Coords.ZAxis.Z;

	// Compute outcode.
	static FLOAT ClipXM, ClipXP, ClipYM, ClipYP;
	static const BYTE OutXMinTab [2] = { 0, FVF_OutXMin };
	static const BYTE OutXMaxTab [2] = { 0, FVF_OutXMax };
	static const BYTE OutYMinTab [2] = { 0, FVF_OutYMin };
	static const BYTE OutYMaxTab [2] = { 0, FVF_OutYMax };

	ClipXM = Frame->PrjXM * OutPoint->Point.Z + OutPoint->Point.X;
	ClipXP = Frame->PrjXP * OutPoint->Point.Z - OutPoint->Point.X;
	ClipYM = Frame->PrjYM * OutPoint->Point.Z + OutPoint->Point.Y;
	ClipYP = Frame->PrjYP * OutPoint->Point.Z - OutPoint->Point.Y;

	OutPoint->Flags  =
	(	OutXMinTab [ClipXM < 0.0]
	+	OutXMaxTab [ClipXP < 0.0]
	+	OutYMinTab [ClipYM < 0.0]
	+	OutYMaxTab [ClipYP < 0.0]);

//	if (TmpFlags!=OutPoint->Flags)
//		debugf(NAME_Log,"Flags incorrect: 0x%02X, should be 0x%02X",OutPoint->Flags,TmpFlags);
//	OutPoint->Flags=TmpFlags;

	// Project if not clipped.
	if (!OutPoint->Flags)
	{
		OutPoint->RZ      = Frame->Proj.Z / OutPoint->Point.Z;
		OutPoint->ScreenX = OutPoint->Point.X * OutPoint->RZ + Frame->FX15;
		OutPoint->ScreenY = OutPoint->Point.Y * OutPoint->RZ + Frame->FY15;
//		OutPoint->IntY    = (int)(OutPoint->ScreenY - 0.5);
		OutPoint->IntY    = appFloor( OutPoint->ScreenY );
	}
#endif 
}

//
// K6 3D assembler version of:
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = Frame->PrjXM * Pts[i]->Point.Z + Pts[i]->Point.X;
//
static _inline void SetXMinDotVals(FSceneNode *Frame, INT NumPts, FTransform** Pts)
{
	_asm
	{
		// Get ready for loop
		mov		eax,offset Dot
		mov		ebx,Frame
		mov		ecx,NumPts
		mov		edx,Pts
		movd	mm0,[ebx]FSceneNode.PrjXM
		cmp		ecx,0
		jz		Done
TopLoop:
		// Do: Dot[i] = Frame->PrjXM * Pts[i]->Point.Z + Pts[i]->Point.X;
		mov		edi,[edx]	// Get ptr to Pt
		movd	mm1,[edi]FOutVector.Point.Z
		movd	mm2,[edi]FOutVector.Point.X
		pfmul	(m1,m0)
		pfadd	(m2,m1)
		movd	[eax],mm2

		// Loop
		add		edx,4
		add		eax,4
		loop	TopLoop	
Done:	
	}
}

//
// K6 3D assembler version of:
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = Frame->PrjXP * Pts[i]->Point.Z - Pts[i]->Point.X;
//
static _inline void SetXMaxDotVals(FSceneNode *Frame, INT NumPts, FTransform** Pts)
{
	_asm
	{
		// Get ready for loop
		mov		eax,offset Dot
		mov		ebx,Frame
		mov		ecx,NumPts
		mov		edx,Pts
		movd	mm0,[ebx]FSceneNode.PrjXP
		cmp		ecx,0
		jz		Done
TopLoop:
		// Do: Dot[i] = Frame->PrjXP * Pts[i]->Point.Z - Pts[i]->Point.X;
		mov		edi,[edx]	// Get ptr to Pt
		movd	mm1,[edi]FOutVector.Point.Z
		movd	mm2,[edi]FOutVector.Point.X
		pfmul	(m1,m0)
		pfsubr	(m2,m1)
		movd	[eax],mm2

		// Loop
		add		edx,4
		add		eax,4
		loop	TopLoop	
Done:	
	}
}

//
// K6 3D assembler version of:
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = Frame->PrjYM * Pts[i]->Point.Z + Pts[i]->Point.Y;
//
static _inline void SetYMinDotVals(FSceneNode *Frame, INT NumPts, FTransform** Pts)
{
	_asm
	{
		// Get ready for loop
		mov		eax,offset Dot
		mov		ebx,Frame
		mov		ecx,NumPts
		mov		edx,Pts
		movd	mm0,[ebx]FSceneNode.PrjYM
		cmp		ecx,0
		jz		Done
TopLoop:
		// Do: Dot[i] = Frame->PrjYM * Pts[i]->Point.Z + Pts[i]->Point.Y;
		mov		edi,[edx]	// Get ptr to Pt
		movd	mm1,[edi]FOutVector.Point.Z
		movd	mm2,[edi]FOutVector.Point.Y
		pfmul	(m1,m0)
		pfadd	(m2,m1)
		movd	[eax],mm2

		// Loop
		add		edx,4
		add		eax,4
		loop	TopLoop	
Done:	
	}
}

//
// K6 3D assembler version of:
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = Frame->PrjYP * Pts[i]->Point.Z - Pts[i]->Point.Y;
//
static _inline void SetYMaxDotVals(FSceneNode *Frame, INT NumPts, FTransform** Pts)
{
	_asm
	{
		// Get ready for loop
		mov		eax,offset Dot
		mov		ebx,Frame
		mov		ecx,NumPts
		mov		edx,Pts
		movd	mm0,[ebx]FSceneNode.PrjYP
		cmp		ecx,0
		jz		Done
TopLoop:
		// Do: Dot[i] = Frame->PrjYP * Pts[i]->Point.Z - Pts[i]->Point.Y;
		mov		edi,[edx]	// Get ptr to Pt
		movd	mm1,[edi]FOutVector.Point.Z
		movd	mm2,[edi]FOutVector.Point.Y
		pfmul	(m1,m0)
		pfsubr	(m2,m1)
		movd	[eax],mm2

		// Loop
		add		edx,4
		add		eax,4
		loop	TopLoop	
Done:	
	}
}

//
// Routine to calculate intersection of two points against a clipping plane
// and then project the resultant point.
//
static _inline void IntersectAndProject(FSceneNode*	Frame,
										FTransform*	OutPoint,
										FTransform*	InPoint1,
										FTransform*	InPoint2,
										DWORD		Dot1,
										DWORD		Dot2)
{
#if 1
	_asm
	{
		// Get pointers
		mov		eax,InPoint1
		mov		ebx,InPoint2

		// Do intersect and project
		movd	mm0,Dot1						// mm0=0|Dot1, mm1=0|Dot2
		movd	mm1,Dot2
		movd	mm2,[eax]FOutVector.Point.Z		// mm2=0|Z1, mm3=0|Z2
		movd	mm3,[ebx]FOutVector.Point.Z
		pfsubr	(m1,m0)							// mm1=0|Dot1-Dot2, mm4=Y1|X1
		movq	mm4,[eax]FOutVector.Point.X
		mov		ecx,Frame						// added - ecx=ptr to Frame, mm0=Dot1|Dot1
		punpckldq mm0,mm0							
		pfsub	(m3,m2)							// mm3=0|Z2-Z1, mm5=Y2|X2
		movq	mm5,[ebx]FOutVector.Point.X
		pfrcp	(m1,m1)							// mm1=1/(Dot1-Dot2), mm6=0|ProjZ
		movd	mm6,[ecx]FSceneNode.Proj.Z
		pfsub	(m5,m4)							// mm5=Y2-Y1|X2-X1, mm7=FY15|FX15
		movq	mm7,[ecx]FSceneNode.FX15
		pfmul	(m3,m0)							// mm3=0|(Z2-Z1)*Dot1, mm6=ProjZ|ProjZ
		punpckldq mm6,mm6
		pfmul	(m0,m1)							// mm0=Dot1/(Dot1-Dot2), mm3=0|Zs
		pfmul	(m3,m1)
		mov		edx,OutPoint					// added - Try and eat a bit of scheduler latency
		pfmul	(m5,m0)							// mm5=Ys|Xs, mm2=0|Z'
		pfadd	(m2,m3)
		pfadd	(m4,m5)							// mm4=Y'|X', save Z'
		movd	[edx]FOutVector.Point.Z,mm2		
		pfrcp	(m1,m2)							// mm1=1/Z'|1/Z', save Y'|X'
		movq	[edx]FOutVector.Point.X,mm4
		pfmul	(m6,m1)							// mm6=ProjZ/Z'|ProjZ/Z', save mm6
		movd	[edx]FTransform.RZ,mm6
		// We have some scheduler stalls. Need some stuff to stick in here.
		pfmul	(m6,m4)							// mm6=Y'ProjZ1/Z'|X'ProjZ1/Z', mm7=ScrY|ScrX
		pfadd	(m7,m6)
		movq	[edx]FTransform.ScreenX,mm7		// save ScrY|ScrX, mm0=Int(ScrY)|Int(ScrX)
		pf2id	(m0,m7)
		psrlq	mm0,32							// mm0=0|Int(ScrY), save Int(ScrY)
		movd	[edx]FTransform.IntY,mm0
	}
#else
	// Intersect.
//	OutPoint->Point=InPoint1->Point + (InPoint2->Point-InPoint1->Point) * (MAKE_FP(Dot1)/(MAKE_FP(Dot1)-MAKE_FP(Dot2)));
	FLOAT Scale=MAKE_FP(Dot1)/(MAKE_FP(Dot1)-MAKE_FP(Dot2));

	OutPoint->Point.X=InPoint1->Point.X + (InPoint2->Point.X-InPoint1->Point.X) * Scale;
	OutPoint->Point.Y=InPoint1->Point.Y + (InPoint2->Point.Y-InPoint1->Point.Y) * Scale;
	OutPoint->Point.Z=InPoint1->Point.Z + (InPoint2->Point.Z-InPoint1->Point.Z) * Scale;

	// Project.
//	OutPoint->Project(Frame);
	OutPoint->RZ      = Frame->Proj.Z  / OutPoint->Point.Z;
	OutPoint->ScreenX = OutPoint->Point.X * OutPoint->RZ + Frame->FX15;
	OutPoint->ScreenY = OutPoint->Point.Y * OutPoint->RZ + Frame->FY15;
//	OutPoint->IntY    = (int)(OutPoint->ScreenY - 0.5);
	OutPoint->IntY    = appFloor( OutPoint->ScreenY );
#endif
}

static inline INT AMD3DClip( FTransform** Dest, FTransform** Src, INT SrcNum )
{
	INT DestNum=0;
	for( INT i=0,j=SrcNum-1; i<SrcNum; j=i++ )
	{
		if( *(INT*)(Dot+j) >= 0 )
		{
			Dest[DestNum++] = Src[j];
		}
		if( (*(INT*)(Dot+j) ^ *(INT*)(Dot+i)) < 0 )
		{
			FTransform* T = Dest[DestNum++] = New<FTransform>( GDynMem );
//			FLOAT Alpha   = Dot[j] / (Dot[j]-Dot[i]);
//			T->Point.X    = Src[j]->Point.X + (Src[i]->Point.X-Src[j]->Point.X) * Alpha;
//			T->Point.Y    = Src[j]->Point.Y + (Src[i]->Point.Y-Src[j]->Point.Y) * Alpha;
//			T->Point.Z    = Src[j]->Point.Z + (Src[i]->Point.Z-Src[j]->Point.Z) * Alpha;
//			T->Project( GFrame );
			IntersectAndProject(GFrame,T,Src[j],Src[i],MAKE_DWORD(Dot[j]),MAKE_DWORD(Dot[i]));
		}
	}
	return DestNum;
}

//
// Transform and clip a polygon.
//
_inline INT URender::AMD3DClipBspSurf( INT iNode, FTransform**& Result )
{
	guard(URender::AMD3DClipBspSurf);
	static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];

	// Transform.
	STAT(GStat.NumTransform++);
	FBspNode* Node		= &GNodes[iNode];
	INT       NumPts    = Node->NumVertices;
	FVert*	  VertPool	= &GVerts[Node->iVertPool];
	BYTE      Outcode   = FVF_OutReject;
	BYTE      AllCodes  = 0;

	DoFemms();

	for( INT i=0; i<NumPts; i++ )
	{
		INT pPoint = VertPool[i].pVertex;
		FStampedPoint& S = PointCache[pPoint];
		if( S.Stamp != Stamp )
		{
			S.Stamp = Stamp;
			S.Point = new(VectorMem)FTransform;
//			Pipe( *S.Point, GFrame, GPoints[pPoint] );
			TransformAndComputeOutcodeAndProject(S.Point,GPoints[pPoint],GFrame);
			STAT(GStat.NumPoints++);
		}
		LocalPts[i] = S.Point;
		BYTE Flags  = S.Point->Flags; 
		Outcode    &= Flags;
		AllCodes   |= Flags;
	}

	if( Outcode )
	{
		DoFemms();
		return 0;
	}

	// Clip.
	STAT(GStat.NumClip++);
	FTransform** Pts = LocalPts;
	if( AllCodes )
	{
		if( AllCodes & FVF_OutXMin )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = GFrame->PrjXM * Pts[i]->Point.Z + Pts[i]->Point.X;
			SetXMinDotVals(GFrame,NumPts,Pts);
			NumPts = AMD3DClip( LocalPts, Pts, NumPts );
			if( !NumPts )
			{
				DoFemms();
				return 0;
			}
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutXMax )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = GFrame->PrjXP * Pts[i]->Point.Z - Pts[i]->Point.X;
			SetXMaxDotVals(GFrame,NumPts,Pts);
			NumPts = AMD3DClip( LocalPts, Pts, NumPts );
			if( !NumPts )
			{
				DoFemms();
				return 0;
			}
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutYMin )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = GFrame->PrjYM * Pts[i]->Point.Z + Pts[i]->Point.Y;
			SetYMinDotVals(GFrame,NumPts,Pts);
			NumPts = AMD3DClip( LocalPts, Pts, NumPts );
			if( !NumPts )
			{
				DoFemms();
				return 0;
			}
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutYMax )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
//			for( INT i=0; i<NumPts; i++ )
//				Dot[i] = GFrame->PrjYP * Pts[i]->Point.Z - Pts[i]->Point.Y;
			SetYMaxDotVals(GFrame,NumPts,Pts);
			NumPts = AMD3DClip( LocalPts, Pts, NumPts );
			if( !NumPts )
			{
				DoFemms();
				return 0;
			}
			Pts = LocalPts;
		}
	}
//	if( GFrame->NearClip.W != 0.0 )
	if( MAKE_DWORD(GFrame->NearClip.W) != 0 )
	{
		UBOOL Clipped=0;
		// Can't K63Dize this since it doesn't seem to be used so we can't test it.
		DoFemms();
		for( INT i=0; i<NumPts; i++ )
		{
			Dot[i] = GFrame->NearClip.PlaneDot(Pts[i]->Point);
			Clipped |= (Dot[i]<0.0);
		}
		DoFemms();

		if( Clipped )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
			NumPts = AMD3DClip( LocalPts, Pts, NumPts );
			if( !NumPts )
			{
				DoFemms();
				return 0;
			}
			Pts = LocalPts;
		}
	}
	Result = Pts;

	DoFemms();

	return NumPts;
	unguard;
}
#pragma warning( disable : 4799 )
#pragma pack( )
#endif

static inline INT Clip( FTransform** Dest, FTransform** Src, INT SrcNum )
{
	INT DestNum=0;
	for( INT i=0,j=SrcNum-1; i<SrcNum; j=i++ )
	{
		if( *(INT*)(Dot+j) >= 0 )
		{
			Dest[DestNum++] = Src[j];
		}
		if( (*(INT*)(Dot+j) ^ *(INT*)(Dot+i)) < 0 )
		{
			FTransform* T = Dest[DestNum++] = New<FTransform>( GDynMem );
			FLOAT Alpha   = Dot[j] / (Dot[j]-Dot[i]);
			T->Point.X    = Src[j]->Point.X + (Src[i]->Point.X-Src[j]->Point.X) * Alpha;
			T->Point.Y    = Src[j]->Point.Y + (Src[i]->Point.Y-Src[j]->Point.Y) * Alpha;
			T->Point.Z    = Src[j]->Point.Z + (Src[i]->Point.Z-Src[j]->Point.Z) * Alpha;
			T->Project( GFrame );
		}
	}
	return DestNum;
}

//
// Transform and clip a polygon.
//
INT URender::ClipBspSurf( INT iNode, FTransform**& Result )
{
#ifndef NOAMD3D
	if (GIsK63D)
		return(AMD3DClipBspSurf(iNode,Result));
#endif		
	guard(URender::ClipBspSurf);
	static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];

	// Transform.
	STAT(GStat.NumTransform++);
	FBspNode* Node		= &GNodes[iNode];
	INT       NumPts    = Node->NumVertices;
	FVert*	  VertPool	= &GVerts[Node->iVertPool];
	BYTE      Outcode   = FVF_OutReject;
	BYTE      AllCodes  = 0;
	for( INT i=0; i<NumPts; i++ )
	{
		INT pPoint = VertPool[i].pVertex;
		FStampedPoint& S = PointCache[pPoint];
		if( S.Stamp != Stamp )
		{
			S.Stamp = Stamp;
			S.Point = new(VectorMem)FTransform;
			Pipe( *S.Point, GFrame, GPoints[pPoint] );
			STAT(GStat.NumPoints++);
		}
		LocalPts[i] = S.Point;
		BYTE Flags  = S.Point->Flags; 
		Outcode    &= Flags;
		AllCodes   |= Flags;
	}
	if( Outcode )
		return 0;

	// Clip.
	STAT(GStat.NumClip++);
	FTransform** Pts = LocalPts;
	if( AllCodes )
	{
		if( AllCodes & FVF_OutXMin )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = GFrame->PrjXM * Pts[i]->Point.Z + Pts[i]->Point.X;
			NumPts = Clip( LocalPts, Pts, NumPts );
			if( !NumPts )
				return 0;
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutXMax )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = GFrame->PrjXP * Pts[i]->Point.Z - Pts[i]->Point.X;
			NumPts = Clip( LocalPts, Pts, NumPts );
			if( !NumPts )
				return 0;
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutYMin )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = GFrame->PrjYM * Pts[i]->Point.Z + Pts[i]->Point.Y;
			NumPts = Clip( LocalPts, Pts, NumPts );
			if( !NumPts )
				return 0;
			Pts = LocalPts;
		}
		if( AllCodes & FVF_OutYMax )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
			for( INT i=0; i<NumPts; i++ )
				Dot[i] = GFrame->PrjYP * Pts[i]->Point.Z - Pts[i]->Point.Y;
			NumPts = Clip( LocalPts, Pts, NumPts );
			if( !NumPts )
				return 0;
			Pts = LocalPts;
		}
	}
	if( GFrame->NearClip.W != 0.0 )
	{
		UBOOL Clipped=0;
		for( INT i=0; i<NumPts; i++ )
		{
			Dot[i] = GFrame->NearClip.PlaneDot(Pts[i]->Point);
			Clipped |= (Dot[i]<0.0);
		}
		if( Clipped )
		{
			static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
			NumPts = Clip( LocalPts, Pts, NumPts );
			if( !NumPts )
				return 0;
			Pts = LocalPts;
		}
	}
	Result = Pts;
	return NumPts;
	unguard;
}

/*-----------------------------------------------------------------------------
	Scene frames.
-----------------------------------------------------------------------------*/

//
// Build a master scene frame.
//
FSceneNode* URender::CreateMasterFrame( UViewport* Viewport, FVector Location, FRotator Rotation, FScreenBounds* Bounds )
{
	guard(URender::CreateMasterFrame);

	// Set base info.
	FSceneNode* Frame	= new(GSceneMem)FSceneNode;
	Frame->Viewport		= Viewport;
	Frame->X			= Viewport->SizeX;
	Frame->Y			= Viewport->SizeY;
	Frame->XB			= 0;
	Frame->YB			= 0;
	Frame->Level		= Viewport->Actor->XLevel;
	Frame->Parent		= NULL;
	Frame->Sibling		= NULL;
	Frame->Child		= NULL;
	Frame->iSurf		= INDEX_NONE;
	Frame->Recursion	= 0;
	Frame->Mirror		= 1.0;
	Frame->Recursion	= 0;
	Frame->NearClip		= FPlane(0,0,0,0);
	Frame->Draw[0]		= NULL;
	Frame->Draw[1]		= NULL;
	Frame->Draw[2]		= NULL;
	Frame->Sprite		= NULL;
	Frame->Span			= new(GSceneMem)FSpanBuffer;
	Frame->Span->AllocIndexForScreen( Viewport->SizeX, Viewport->SizeY, &GSceneMem );

	// Compute coords.
	Frame->ComputeRenderCoords( Location, Rotation );

	// Compute zone.
	Frame->ZoneNumber   = Viewport->Actor->XLevel->Model->PointRegion( Viewport->Actor->XLevel->GetLevelInfo(), Frame->Coords.Origin ).ZoneNumber;

	return Frame;
	unguard;
}

//
// Build a new child scene frame.
//
FSceneNode* URender::CreateChildFrame
(
	FSceneNode*		Parent,
	FSpanBuffer*	Span,
	ULevel*			Level,
	INT				iSurf,
	INT				iZone,
	FLOAT			Mirror,
	const FPlane&	NearClip,
	const FCoords&	Coords,
	FScreenBounds*	Bounds
)
{
	guard(URender::CreateChildFrame);

	// See if the scene frame already exists.
	for( FSceneNode* Frame=Parent->Child; Frame; Frame=Frame->Sibling )
	{
		if
		(	Frame->Level==Level
		&&	Frame->iSurf==iSurf
		&&	Frame->Parent==Parent
		&&	Frame->NearClip==NearClip
		&&	Frame->ZoneNumber==iZone )
		{
			// Merge with existing scene frame.
			Frame->Span->MergeWith( *Span );
			if( Bounds )
			{
				Frame->PrjXM = Max( Frame->PrjXM, (Bounds->MinX - Frame->FX2)*(-Frame->RProj.Z) );
				Frame->PrjXP = Max( Frame->PrjXP, (Bounds->MaxX - Frame->FX2)*(+Frame->RProj.Z) );
				Frame->PrjYM = Max( Frame->PrjYM, (Bounds->MinY - Frame->FY2)*(-Frame->RProj.Z) );
				Frame->PrjYP = Max( Frame->PrjYP, (Bounds->MaxY - Frame->FY2)*(+Frame->RProj.Z) );
			}
			break;
		}
	}
	if( Frame == NULL )
	{
		// Make a new scene frame.
		Frame				= new(GSceneMem)FSceneNode(*Parent);
		Frame->Span        	= new(GSceneMem)FSpanBuffer;
		Frame->Viewport     = Parent->Viewport;
		Frame->Level		= Level;
		Frame->iSurf		= iSurf;
		Frame->ZoneNumber	= iZone;
		Frame->Recursion	= Parent->Recursion+1;
		Frame->Mirror		= Mirror;
		Frame->NearClip		= NearClip;
		Frame->Coords		= Coords;
		Frame->Uncoords		= Coords.Transpose(); //!!inverse() - assumes orthogonal.
		Frame->Draw[0]		= NULL;
		Frame->Draw[1]		= NULL;
		Frame->Draw[2]		= NULL;
		Frame->Sprite		= NULL;

		// Insert into linked list of scene frames.
		Frame->Parent		= Parent;
		Frame->Child		= NULL;
		Frame->Sibling		= Parent->Child;
		Parent->Child		= Frame;

		// Compute rendering information.
		//!!clip to parent
		Frame->ComputeRenderSize();
		if( Bounds )
		{
			Frame->PrjXM = (Bounds->MinX - Frame->FX2)*(-Frame->RProj.Z);
			Frame->PrjXP = (Bounds->MaxX - Frame->FX2)*(+Frame->RProj.Z);
			Frame->PrjYM = (Bounds->MinY - Frame->FY2)*(-Frame->RProj.Z);
			Frame->PrjYP = (Bounds->MaxY - Frame->FY2)*(+Frame->RProj.Z);
		}

		// Make span buffer.
		Frame->Span->AllocIndex( 0, 0, &GSceneMem );
		Frame->Span->MergeWith( *Span );
	}
	return Frame;
	unguard;
}

/*-----------------------------------------------------------------------------
	World polygon rasterizer.
-----------------------------------------------------------------------------*/

FRasterSpan HackRaster[1200];//max y res 1200!!
INT RasterStartY, RasterEndY, RasterStartX, RasterEndX;
static UBOOL SetupRaster( FTransform** Pts, INT NumPts, FSpanBuffer* Span, INT EndY )
{
	guard(SetupRaster);

	// Compute integer coords.
	RasterStartY = RasterEndY = Pts[0]->IntY;
	RasterStartX = RasterEndX = appFloor( Pts[0]->ScreenX );
	for( INT i=1; i<NumPts; i++ )
	{
		INT Y = Pts[i]->IntY;
		if( Y < RasterStartY )
			RasterStartY = Y;
		else if( Y > RasterEndY )
			RasterEndY = Y;

		INT X = appFloor( Pts[i]->ScreenX );
		if( X < RasterStartX )
			RasterStartX = X;
		else if( X > RasterEndX )
			RasterEndX = X;
	}
	if( RasterStartY<0 || RasterEndY>EndY )
	{
		RasterStartY = Clamp( RasterStartY, 0, EndY );
		RasterEndY   = Clamp( RasterEndY,   0, EndY );
		for( INT i=0; i<NumPts; i++ )
		{
			Pts[i]->IntY    = Clamp( Pts[i]->IntY, 0, EndY );
			Pts[i]->ScreenY = Clamp( Pts[i]->IntY, 0, EndY );
		}
	}

	// Check bounds for visibility.
	if( Span && !Span->BoxIsVisible( RasterStartX, RasterStartY, RasterEndX, RasterEndY ) )
	{
		STAT(GStat.NumRasterBoxReject++);
		return 0;
	}
	STAT(GStat.NumRasterPolys++);

	// Rasterize the edges.
	FTransform **Last=Pts+NumPts, *P[2], *Top, *Bot;
	for( P[0]=Pts[NumPts-1],P[1]=Pts[0]; Pts<Last; P[0]=*Pts,Pts++,P[1]=*Pts )
	{
		if( P[1]->IntY != P[0]->IntY )
		{
			INT Index    = P[1]->IntY > P[0]->IntY;
			Bot          = P[Index];
			Top          = P[1-Index];
			INT*   Set   = HackRaster->X + Top->IntY*2 + Index;
			DOUBLE YAdj  = Top->IntY - Top->ScreenY;
			DOUBLE FDX   = 65536.0 * (Bot->ScreenX - Top->ScreenX) / (Bot->ScreenY - Top->ScreenY);
			DWORD  X     = appFloor( 65536.0 * Top->ScreenX + YAdj * FDX );
			DWORD  DX    = appFloor( FDX );
			INT    Count = Bot->IntY - Top->IntY;
			while( Count >= 4 )
			{
				Set[0]  = Unfix(X+=DX);
				Set[2]  = Unfix(X+=DX);
				Set[4]  = Unfix(X+=DX);
				Set[6]  = Unfix(X+=DX);
				Set    += 2*4;
				Count  -= 4;
			}
			while( Count > 0 )
			{
				*Set   = Unfix(X+=DX);
				Set   += 2;
				Count -= 1;
			}
		}
	}
	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Visible nodes.
-----------------------------------------------------------------------------*/

void URender::GetVisibleSurfs( UViewport* Viewport, TArray<INT>& iSurfs )
{
	guard(URender::GetVisibleSurfs);

	// Try six views.
	for( INT i=0; i<6; i++ )
	{
		// Set up rendering.
		FMemMark MemMark(GMem);
		FMemMark DynMemMark(GDynMem);
		FMemMark SceneMark(GSceneMem);
		FMemMark VectorMark(VectorMem);
		Viewport->Actor->ViewRotation
		=	i==0 ? FRotator(0x4000,0     ,0)
		:	i==1 ? FRotator(0xC000,0     ,0)
		:	i==2 ? FRotator(0     ,0     ,0)
		:	i==3 ? FRotator(0     ,0x8000,0)
		:	i==4 ? FRotator(0     ,0xC000,0)
		:          FRotator(0     ,0x4000,0);
		FSceneNode* Frame = CreateMasterFrame( Viewport, Viewport->Actor->Location, Viewport->Actor->ViewRotation, NULL );

		// Add all visible nodes.
		UBOOL SavedFogZone = Viewport->RenDev->VolumetricLighting;
		Viewport->RenDev->VolumetricLighting = 0;
		OccludeBsp( Frame );
		for( INT i=0; i<3; i++ )
			for( FBspDrawList* Draw=Frame->Draw[i]; Draw; Draw=Draw->Next )
				iSurfs.AddUniqueItem( Draw->iSurf );
		Viewport->RenDev->VolumetricLighting = SavedFogZone;
		SceneMark.Pop();
		MemMark.Pop();
		DynMemMark.Pop();
		VectorMark.Pop();
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Bsp occlusion functions.
-----------------------------------------------------------------------------*/

//
// Checks whether the node's bouding box is totally occluded.  Returns 0 if
// total occlusion, 1 if all or partial visibility.
//
UBOOL URender::BoundVisible
(
	FSceneNode*		Frame,
	FBox*			Bound,
	FSpanBuffer*	SpanBuffer,
	FScreenBounds&	Result
)
{
	guard(URender::BoundVisible);
	STAT(GStat.BoxChecks++);
	clock(GStat.BoxTime);

	FCoords		BoxDot[2];
	FTransform	Pts[8], *Pt;
	FVector 	ViewportLoc;
	FLOAT       BoxMinZ, BoxMaxZ;
	INT         BoxX, BoxY;
	INT 		BoxMinX, BoxMaxX, BoxMinY, BoxMaxY;
	INT			OutCode;

	// Handle rejection in orthogonal views.
	if( Frame->Viewport->IsOrtho() )
	{
		Project( Frame, Bound->Min, Result.MinX, Result.MinY, NULL );
		Project( Frame, Bound->Max, Result.MaxX, Result.MaxY, NULL );
		if( Result.MinX > Result.MaxX ) Exchange( Result.MinX, Result.MaxX );
		if( Result.MinY > Result.MaxY ) Exchange( Result.MinY, Result.MaxY );
		unclock(GStat.BoxTime);
		return Result.MaxX>0.0 && Result.MaxY>0.0 && Result.MinX<Frame->FX && Result.MinY<Frame->FY;
	}

	// Compute hull position code.
	INT HullCode = 0;
	FVector NewMin = Bound->Min - Frame->Coords.Origin;
	FVector NewMax = Bound->Max - Frame->Coords.Origin;
	if     ( NewMin.X > 0.0 ) HullCode  = 1;
	else if( NewMax.X < 0.0 ) HullCode  = 2;
	if     ( NewMin.Y > 0.0 ) HullCode += 3;
	else if( NewMax.Y < 0.0 ) HullCode += 6;
	if     ( NewMin.Z > 0.0 ) HullCode += 9;
	else if( NewMax.Z < 0.0 ) HullCode += 17;

	static const BYTE HullVerts[27][8] = 
	{
	/*  0 */ {255},
	/*  1 */ {0,4,6,2,255},
	/*  2 */ {1,3,7,5,255},
	/*  3 */ {0,1,5,4,255},
	/*  4 */ {0,1,5,4,6,2,255},
	/*  5 */ {0,1,3,7,5,4,255},
	/*  6 */ {2,6,7,3,255},
	/*  7 */ {0,4,6,7,3,2,255},
	/*  8 */ {1,3,2,6,7,5,255},
	/*  9 */ {0,2,3,1,255},
	/* 10 */ {0,4,6,2,3,1,255},
	/* 11 */ {0,2,3,7,5,1,255},
	/* 12 */ {0,2,3,1,5,4,255},
	/* 13 */ {1,5,4,6,2,3,255},
	/* 14 */ {0,2,3,7,5,4,255},
	/* 15 */ {0,2,6,7,3,1,255},
	/* 16 */ {0,4,6,7,3,1,255},
	/* 17 */ {0,2,6,7,5,1,255},
	/* 18 */ {5,7,6,4,255},
	/* 19 */ {0,4,5,7,6,2,255},
	/* 20 */ {1,3,7,6,4,5,255},
	/* 21 */ {0,1,5,7,6,4,255},
	/* 22 */ {0,1,5,7,6,2,255},
	/* 23 */ {0,1,3,7,6,4,255},
	/* 24 */ {2,6,4,5,7,3,255},
	/* 25 */ {0,4,5,7,3,2,255},
	/* 27 */ {1,3,2,6,4,5,255},
	};
	/*{for( INT i=0; i<27; i++ )
	{
		INT Verts[8];
		INT Found[9];
		for( INT j=0; j<8; j++ )
			Verts[j]=Found[j]=0;
		INT x=i;
		if( x>=18 )
		{
			Verts[7]++;
			Verts[6]++;
			Verts[4]++;
			Verts[5]++;
			x -= 18;
		}
		if( x>=9 )
		{
			Verts[2]++;
			Verts[3]++;
			Verts[1]++;
			Verts[0]++;
			x -= 9;
		}
		if( x>=6 )
		{
			Verts[2]++;
			Verts[3]++;
			Verts[7]++;
			Verts[6]++;
			x -= 6;
		}
		if( x>=3 )
		{
			Verts[0]++;
			Verts[1]++;
			Verts[5]++;
			Verts[4]++;
			x -= 3;
		}
		if( x>=2 )
		{
			Verts[1]++;
			Verts[3]++;
			Verts[7]++;
			Verts[5]++;
			x -= 2;
		}
		if( x>=1 )
		{
			Verts[0]++;
			Verts[2]++;
			Verts[6]++;
			Verts[4]++;
			x -= 1;
		}
		check(x==0);
		for( INT u=0; u<8 && HullVerts[i][u]!=255; u++ )
		{
			check(HullVerts[i][u]>=0);
			check(HullVerts[i][u]<=7);
			if( !Verts[HullVerts[i][u]] )
				appErrorf("b %i [%i]",i,HullVerts[i][u]);
			Verts[HullVerts[i][u]]=0;
		}
		for( INT q=0; q<8; q++ )
			if( Verts[q]!=0 && Verts[q]!=3 )
				appErrorf("a %i [%i]",i,q);
	}}*/

	// Trivial in-bound accept.
	if( HullCode==0 )
	{
		Result.MinX = 0;
		Result.MinY = 0;
		Result.MaxX = Frame->FX;
		Result.MaxY = Frame->FY;
		Result.MinZ = 0;
		STAT(GStat.BoxIn++);
		unclock(GStat.BoxTime);
		return 1;
	}

	/* Bounding sphere reject: Not worth it (slower).
	if( Toggle )
	{
		FVector Center   = 0.5*(Bound->Min + Bound->Max);
		FLOAT   RadiusSq = FDistSquared(Center,Bound->Min);
		for( INT i=0; i<4; i++ )
		{
			FLOAT Dot = Frame->ViewPlanes[i].PlaneDot(Center);
			if( Dot<0.0 && Square(Dot)>RadiusSq )
			{
				unclock(GStat.BoxTime);
				return 0;
			}
		}
	}*/

	// Test bounding-box side-of-viewport rejection.  Since box is axis-aligned,
	// this can be optimized: Box can only be rejected if all 8 dot products of the
	// 8 box sides are less than zero. This is the case iff each dot product
	// component is less than zero.
	BoxDot[0].ZAxis = NewMin * Frame->Coords.ZAxis;
	BoxDot[1].ZAxis = NewMax * Frame->Coords.ZAxis;
	if
	(	BoxDot[0].ZAxis.X<0.0 && BoxDot[0].ZAxis.Y<0.0 && BoxDot[0].ZAxis.Z<0.0
	&&	BoxDot[1].ZAxis.X<0.0 && BoxDot[1].ZAxis.Y<0.0 && BoxDot[1].ZAxis.Z<0.0 )
	{
		STAT(GStat.BoxBacks++);
		unclock(GStat.BoxTime);
		return 0;
	}

	// Transform bounding box min and max coords into screenspace.
	BoxDot[0].XAxis = NewMin * Frame->Coords.XAxis;
	BoxDot[1].XAxis = NewMax * Frame->Coords.XAxis;
	BoxDot[0].YAxis = NewMin * Frame->Coords.YAxis;
	BoxDot[1].YAxis = NewMax * Frame->Coords.YAxis;

	// View-pyramid reject with an outcode test.
	INT ThisCode, AllCodes;
	BoxMinZ = Pts[0].Point.Z;
	BoxMaxZ = Pts[0].Point.Z;
	OutCode  = 1|2|4|8;
	AllCodes = 0;
	#define CMD(i,j,k,First,P)\
		ThisCode = 0;\
		\
		P.Point.Z = BoxDot[i].ZAxis.X + BoxDot[j].ZAxis.Y + BoxDot[k].ZAxis.Z;\
		if( First || P.Point.Z < BoxMinZ ) BoxMinZ = P.Point.Z;\
		if( First || P.Point.Z > BoxMaxZ ) BoxMaxZ = P.Point.Z;\
		\
		P.Point.X = BoxDot[i].XAxis.X + BoxDot[j].XAxis.Y + BoxDot[k].XAxis.Z;\
		if( Frame->PrjXM * P.Point.Z + P.Point.X < 0 ) ThisCode |= 1;\
		if( Frame->PrjXP * P.Point.Z - P.Point.X < 0 ) ThisCode |= 2;\
		\
		P.Point.Y = BoxDot[i].YAxis.X + BoxDot[j].YAxis.Y + BoxDot[k].YAxis.Z;\
		if( Frame->PrjYM * P.Point.Z + P.Point.Y < 0 ) ThisCode |= 4;\
		if( Frame->PrjYP * P.Point.Z - P.Point.Y < 0 ) ThisCode |= 8;\
		\
		OutCode  &= ThisCode;\
		AllCodes |= ThisCode;
	CMD(0,0,0,1,Pts[0]); CMD(1,0,0,0,Pts[1]); CMD(0,1,0,0,Pts[2]); CMD(1,1,0,0,Pts[3]);
	CMD(0,0,1,0,Pts[4]); CMD(1,0,1,0,Pts[5]); CMD(0,1,1,0,Pts[6]); CMD(1,1,1,0,Pts[7]);
	#undef CMD
	if( OutCode )
	{
		// Invisible - pyramid reject.
		STAT(GStat.BoxOutOfPyramid++;);
		unclock(GStat.BoxTime);
		return 0;
	}

	// Calculate projections of 8 points and take X,Y min/max bounded to Span X,Y window.
	Pt       = &Pts[0];
	FLOAT RZ = Frame->Proj.Z / Pt->Point.Z;
	BoxMinX  = BoxMaxX = appFloor( Frame->FX2 + Pt->Point.X * RZ );
	BoxMinY  = BoxMaxY = appFloor( Frame->FY2 + Pt->Point.Y * RZ );
	if( AllCodes & 1 ) BoxMinX = 0;
	if( AllCodes & 2 ) BoxMaxX = Frame->X;
	if( AllCodes & 4 ) BoxMinY = 0;
	if( AllCodes & 8 ) BoxMaxY = Frame->Y;
	for( INT i=1; i<8; i++,Pt++ )
	{
		FLOAT RZ = Frame->Proj.Z / Pt->Point.Z;
		BoxX     = appFloor( Frame->FX2 + Pt->Point.X * RZ );
		BoxY     = appFloor( Frame->FY2 + Pt->Point.Y * RZ );
		if      ( BoxX < BoxMinX ) BoxMinX = BoxX;
		else if ( BoxX > BoxMaxX ) BoxMaxX = BoxX;
		if      ( BoxY < BoxMinY ) BoxMinY = BoxY;
		else if ( BoxY > BoxMaxY ) BoxMaxY = BoxY;
	}
	Result.MinX  = ::Max(BoxMinX,0);
	Result.MinY  = ::Max(BoxMinY,0);
	Result.MaxX  = ::Min(BoxMaxX,Frame->X);
	Result.MaxY  = ::Min(BoxMaxY,Frame->Y);
	Result.MinZ  = ::Max( BoxMinZ, 0.f );
	if( !SpanBuffer || SpanBuffer->BoxIsVisible( BoxMinX, BoxMinY, BoxMaxX, BoxMaxY ) )
	{
		unclock(GStat.BoxTime);
		return 1;
	}
	else
	{
		STAT(GStat.BoxSpanOccluded++);
		unclock(GStat.BoxTime);
		return 0;
	}
	unguard;
}

enum ENodePass
{
	PASS_Front=0,
	PASS_Plane=1,
};

struct FNodeStack
{
	INT				iNode;
	INT				iFarNode;
	INT				FarOutside;
	INT				Outside;
	ENodePass		Pass;
	FNodeStack*		Next;
};

static UBOOL VolumetricOccludes( const FVolActorLink* Link, FVector* VolCross, INT NumVolCross )
{
	for( INT i=0; i<NumVolCross; i++ )
		if( (Link->Location | VolCross[i]) > Link->Actor->WorldVolumetricRadius() )
			return 0;
	return 1;
}

void URender::LeafVolumetricLighting( FSceneNode* Frame, UModel* Model, INT iLeaf )
{
	guardSlow(URender::LeafVolumetricLighting);

	// Static volumetrics.
	FLeaf& Leaf = Model->Leaves(iLeaf);
	if( Leaf.iVolumetric != INDEX_NONE )
	{
		AActor* Actor;
		for( INT i=Leaf.iVolumetric; (Actor=Model->Lights(i))!=NULL; i++ )
		{
			if( Actor->LightingTag!=(INT)Stamp )
			{
				Actor->LightingTag = Stamp;
				if( FDistSquared( Frame->Coords.Origin, Actor->Location ) > Square(Actor->WorldVolumetricRadius()) )
					for( int i=0; i<4; i++ )
						if( Frame->ViewPlanes[i].PlaneDot(Actor->Location) < -Actor->WorldVolumetricRadius() )
							goto Skip;
				FirstVolumetric = new(GDynMem)FVolActorLink( Frame->Coords, Actor, FirstVolumetric, 1 );
				Skip:;
			}
		}
	}

	// Dynamic volumetrics.
	for( FVolActorLink* Link=LeafLights[iLeaf]; Link; Link=Link->Next )
	{
		if( Link->Volumetric && Link->Actor->LightingTag!=(INT)Stamp )
		{
			Link->Actor->LightingTag = Stamp;
			FirstVolumetric = new(GDynMem)FVolActorLink( *Link, FirstVolumetric );
		}
	}
	unguardSlow;
}

FVert* _Verts;
FBspNode* _Nodes;
void Traverse( FSceneNode* Frame, INT iNode )
{
	UModel*   Model   = Frame->Level->Model;
	FBspNode* Node    = _Nodes+iNode;
	INT       IsFront = Node->Plane.PlaneDot(Frame->Coords.Origin) > 0.0;
	if( Node->iChild[IsFront] != INDEX_NONE )
	{
		Traverse( Frame, Node->iChild[IsFront] );
	}
	FBspNode* Plane;
	for( INT iPlane=iNode; iPlane!=INDEX_NONE; iPlane=Plane->iPlane )
	{
		Plane = _Nodes+iPlane;
		for( INT i=0; i<Plane->NumVertices; i++ )
		{
			INT pPoint = _Verts[Plane->iVertPool+i].pVertex;
			URender::FStampedPoint& S = URender::PointCache[pPoint];
			if( S.Stamp != URender::Stamp )
			{
				S.Stamp = URender::Stamp;
				S.Point = New<FTransform>( URender::VectorMem );
				Pipe( *S.Point, Frame, Model->Points->Element(pPoint) );
				STAT(GStat.NumPoints++);
			}
			if( !S.Point->Flags )
			{
				INT X = Clamp( appFloor( S.Point->ScreenX ), 0, Frame->X-1 );
				INT Y = Clamp( S.Point->IntY, 0, Frame->Y-1 );
				*Frame->Screen(X,Y) = 0xfe;
			}
		}
	}
	if( Node->iChild[1-IsFront] != INDEX_NONE )
		Traverse( Frame, Node->iChild[1-IsFront] );	
}

void URender::OccludeBsp( FSceneNode* Frame )
{
	UModel*				Model;
	FSpanBuffer			ZoneSpanBuffer[UBspNodes::MAX_ZONES];
	FSpanBuffer*		SpanBuffer;
	FBspDrawList*		TempDrawList;
	FBspDrawList**		AllPolyDrawLists;
	FBspDrawList*		Merge;
	FBspNode*			Node;
	FBspSurf*			Poly;
	FNodeStack*			Stack;
	FTransform 			**Pts;
	FVector				Origin;
	DWORD				PolyFlags, PolyFlagMask, ExtraPolyFlags;
	QWORD				ActiveZoneMask;
	INT          		iNode,iOriginalNode,iThingZone;
	BYTE				iViewZone;
	BYTE				iZone;
	BYTE				ViewZoneMask;
	INT           		Visible;
	INT					Mergeable;
	INT					Outside;
	INT					Pass;
	INT					NumPts;
	INT					IsVolumetric;
	INT					DrawBin;
	INT                 NumActiveZones;
	BYTE                ActiveZones[64];
	guard(URender::OccludeBsp);
	check(Frame->Level->Model->Nodes->Max()<MAX_NODES);
	check(Frame->Level->Model->Points->Max()<MAX_POINTS);

	// If unrenderable.
	Model = Frame->Level->Model;
	if( !Model->Nodes->Num() )
		return;

///////////////////////////////////////////////////////////////////////////////
#if 0
	{
		Stamp++;
		UModel* Model = Frame->Level->Model;
		appMemset( Frame->Screen(0,0), 0, Frame->X*Frame->Y*4 );
		_Verts = &Model->Verts->Element(0);
		_Nodes = &Model->Nodes->Element(0);
		clock(GStat.ExtraTime);
		Traverse( Frame, 0 );
		/*
		static FTransform V;
		for( INT i=0; i<Model->Points->Num(); i++ )
		{
			Pipe( V, Frame, Model->Points->Element(i) );
			if( !V.Flags )
			{
				INT X = Clamp( appFloor( V.ScreenX ), 0, Frame->X-1 );
				INT Y = Clamp( V.IntY, 0, Frame->Y-1 );
				*Frame->Screen(X,Y) = 0xfe;
			}
		}*/
		unclock(GStat.ExtraTime);
		return;
	}
#endif
///////////////////////////////////////////////////////////////////////////////

	// Start clocking stats.
	STAT(clock(GStat.OcclusionTime));

	// Init temporary caches.
	Stamp++;

	// Init.
	UViewport* Viewport = Frame->Viewport;
	URenderDevice* RenDev = Viewport->RenDev;
	TempDrawList		= new(GMem)FBspDrawList;
	AllPolyDrawLists    = new(GMem,MEM_Zeroed,Model->Surfs->Max())FBspDrawList*;
	Origin				= Frame->Coords.Origin;
	iViewZone			= Frame->ZoneNumber;
	ViewZoneMask		= iViewZone ? ~0 : 0;
	NumActiveZones      = 1;
	ActiveZones[0]      = iViewZone;
	ActiveZoneMask		= ((QWORD)1) << iViewZone;
	IsVolumetric        = RenDev->VolumetricLighting && RenDev->SupportsFogMaps && Viewport->Actor->Region.Zone->bFogZone;
	PolyFlagMask        = (Viewport->Actor->ShowFlags & SHOW_PlayerCtrl) ? ~0 : ~PF_Invisible;
	ExtraPolyFlags		= Viewport->ExtraPolyFlags;
	FirstVolumetric		= NULL;
	GFrame              = Frame;
	GNodes				= &Model->Nodes->Element(0);
	GSurfs				= &Model->Surfs->Element(0);
	GVerts				= &Model->Verts->Element(0);
	GPoints				= &Model->Points->Element(0);
	FLOAT TimeSeconds   = Frame->Level->GetLevelInfo()->TimeSeconds;
	Model->Nodes->Zones[iViewZone].LastRenderTime = TimeSeconds;

	// If inside a warp zone, skip out and give this span buffer to the other side.
	AWarpZoneInfo* Warp = (AWarpZoneInfo*)Model->Nodes->Zones[iViewZone].ZoneActor;
	if( Warp && Warp->IsA(AWarpZoneInfo::StaticClass) && Warp->OtherSideActor && Warp->OtherSideLevel )
	{
		guard(HandleInWarp);
		CreateChildFrame
		(
			Frame,
			Frame->Span,
			Frame->Level,
			INDEX_NONE,
			Warp->OtherSideActor->iWarpZone,
			Frame->Mirror,
			Frame->NearClip,
			Frame->Coords * Warp->WarpCoords * Warp->OtherSideActor->WarpCoords.Transpose(),
			NULL
		);
		STAT(unclock(GStat.OcclusionTime));
		unguard;
		return;
	}

	// Init zone span buffers.
	for( INT i=0; i<UBspNodes::MAX_ZONES; i++ )
		ZoneSpanBuffer[i].AllocIndex(0,0,&GDynMem);
	ZoneSpanBuffer[iViewZone] = *Frame->Span;

	// Init unrolled recursion stack.
	Stack				= New<FNodeStack>(GMem);
	Stack->Next			= NULL;
	iNode				= 0;
	Outside				= Model->RootOutside;
	Pass				= PASS_Front;

	// Process everything in the world.
	for( ;; )
	{
		Node = &GNodes[iNode];

		// Pass 1: Process node for the first time and optionally recurse with front node.
		if( Pass==PASS_Front )
		{
			// Zone mask rejection.
			if( iViewZone && !(Node->ZoneMask & ActiveZoneMask))
			{
				// Use pure zone rejection.
				STAT(GStat.MaskRejectZones++);
				goto PopStack;
			}

			// Bound rejection.
			if
			(	Node->iRenderBound!=INDEX_NONE
			&&	iNode<Model->Nodes->Num() 
			&&	((Node->NodeFlags&NF_BoxOccluded) || !((iNode^GFrameStamp)&15)) )
			{
				// Use bounding box rejection.
				Node->NodeFlags &= ~NF_BoxOccluded;
				FScreenBounds Results;
				if( !BoundVisible(Frame,&Model->Bounds(Node->iRenderBound),iViewZone?NULL:&ZoneSpanBuffer[0],Results) )
				{
					Node->NodeFlags |= NF_BoxOccluded;
					goto PopStack;
				}
				if( iViewZone )
				{
					for( INT i=0; i<NumActiveZones; i++ )
					{
						BYTE iZone = ActiveZones[i];
						if
						(	(Node->ZoneMask & ((QWORD)1<<iZone))
						&&	(ZoneSpanBuffer[iZone].BoxIsVisible(Results.MinX,Results.MinY,Results.MaxX,Results.MaxY)) )
							break;
					}
					if( i==NumActiveZones )
					{
						STAT(GStat.BoxSpanOccluded++;);
						Node->NodeFlags |= NF_BoxOccluded;
						goto PopStack;
					}
				}
			}

			// Filter dynamics.
			for( FDynamicItem* Item = Dynamic(iNode,0); Item; Item=Item->FilterNext )
				Item->Filter( Viewport, Frame, iNode, Outside );

			// Set up stack to recurse into front.
			INT IsFront       = Node->Plane.PlaneDot(Origin) > 0.0;
			Stack->iFarNode   = Node->iChild[1-IsFront];
			Stack->FarOutside = Node->ChildOutside(1-IsFront,Outside);
			if( Node->iChild[IsFront] != INDEX_NONE )
			{
				Stack->iNode		= iNode;
				Stack->Outside		= Outside;
				Stack->Pass  		= PASS_Plane;

				FNodeStack* Next	= Stack;
				Stack				= New<FNodeStack>(GMem);
				Stack->Next			= Next;

				iNode				= Node->iChild[IsFront];
				Outside				= Node->ChildOutside(IsFront,Outside);
				Pass				= PASS_Front;

				continue;
			}
			Pass = PASS_Plane;
		}

		// Pass 2: Process polys within this node and optionally recurse with back.
		if( Pass == PASS_Plane )
		{
			// Zone mask rejection.
			if( iViewZone && !(Node->ZoneMask & ActiveZoneMask) )
			{
				STAT(GStat.MaskRejectZones++);
				goto PopStack;
			}

			// Setup.
			iOriginalNode	= iNode;
			FLOAT Dot		= Node->Plane.PlaneDot(Origin);
			INT IsFront		= Dot>0.0;
			iThingZone      = Node->iZone[IsFront] & ViewZoneMask;

			// Render dynamic stuff in front of the plane.
			if( IsVolumetric && Node->iLeaf[IsFront]!=INDEX_NONE )
				LeafVolumetricLighting( Frame, Model, Node->iLeaf[IsFront] );
			if( ZoneSpanBuffer[iThingZone].ValidLines && (Node->ChildOutside(IsFront,Outside)||Toggle) )
				for( FDynamicItem* Item = Dynamic(iNode,1-IsFront); Item; Item=Item->FilterNext )
					Item->PreRender( Viewport, Frame, &ZoneSpanBuffer[iThingZone], iNode, FirstVolumetric );

			// View frustrum rejection of the plane and back subtree.
			FLOAT Sign = IsFront ? 1.0 : -1.0;
			if
			(	(Sign * (Node->Plane | Frame->ViewSides[0]) > 0.0)
			&&	(Sign * (Node->Plane | Frame->ViewSides[1]) > 0.0)
			&&	(Sign * (Node->Plane | Frame->ViewSides[2]) > 0.0)
			&&	(Sign * (Node->Plane | Frame->ViewSides[3]) > 0.0) )
				goto PopStack;

			// Process node and all of its coplanars.
			for( ;; )
			{
				// Note: Can't zone mask reject coplanars due to moving brush rules.
				Poly		= &GSurfs[Node->iSurf];
				PolyFlags	= Poly->PolyFlags | ExtraPolyFlags;

				// Backface and portal reject.
				if( !IsFront && Dot<-1.0 && !(PolyFlags & (PF_TwoSided|PF_Portal)) )
					goto NextCoplanar;
				if( (PolyFlags & PF_Portal) && iViewZone==0 )
					goto NextCoplanar;

				// Get zones.
				iZone         = Node->iZone[IsFront  ] & ViewZoneMask;
				SpanBuffer    = &ZoneSpanBuffer[iZone];

				// Span reject.
				if( SpanBuffer->ValidLines <= 0 )
					goto NextCoplanar;

				// Clip it.
				clock(GStat.ClipTime);
				STAT(GStat.NodesDone++);
				NumPts = ClipBspSurf( iNode, Pts );
				unclock(GStat.ClipTime);
				if( !NumPts )
					goto NextCoplanar;

				// Fix facing.
				if( (!IsFront && (PolyFlags & (PF_TwoSided | PF_Portal))) ^ Frame->Mirror==-1.f )
					for( INT i=0; i<NumPts/2; i++ )
						Exchange( Pts[i], Pts[NumPts-i-1] );

				// Setup.
				clock(GStat.RasterTime);
				if( !SetupRaster( Pts, NumPts, (Node->NodeFlags & NF_PolyOccluded) ? SpanBuffer : NULL, Frame->Y ) )
				{
					unclock(GStat.RasterTime);
					goto NextCoplanar;
				}
				unclock(GStat.RasterTime);

				// Assimilate the texture's flags.
				if( Poly->Texture )
					PolyFlags |= Poly->Texture->PolyFlags;
				PolyFlags &= PolyFlagMask;

				// See if we should merge.
				Mergeable = !(PolyFlags & (PF_NoOcclude|PF_NoMerge|PF_Portal));
				Merge = NULL;
				if( Mergeable )
				{
					AZoneInfo* ZoneActor = Frame->Level->GetZoneActor(Node->iZone[IsFront]);
					for( Merge=AllPolyDrawLists[Node->iSurf]; Merge; Merge=Merge->SurfNext )
						if( Merge->Zone==ZoneActor )
							break;
				}

				// Allocate fragment span buffer.
				TempDrawList->Span.AllocIndex( RasterStartY, RasterEndY, (Merge || !RenDev->SpanBased) ? &GMem : &GDynMem );

				// Perform the span buffer clipping and updating.
				clock(GStat.SpanTime);
				if
				(	!(PolyFlags & PF_NoOcclude)
				||	(PolyFlags&(PF_Portal|PF_Invisible))==(PF_Portal|PF_Invisible) 
				||	(PolyFlags&(PF_Mirrored)) )
					Visible = TempDrawList->Span.CopyFromRasterUpdate( *SpanBuffer, RasterStartY, RasterEndY, HackRaster+RasterStartY );
				else		
					Visible = TempDrawList->Span.CopyFromRaster( *SpanBuffer, RasterStartY, RasterEndY, HackRaster+RasterStartY );
				unclock(GStat.SpanTime);

				// Process the spans.
				DrawBin = 1 + ((PolyFlags & PF_NoOcclude)!=0);
				if( !Visible )
				{
					// Rejected, span buffer wasn't affected.
					Node->NodeFlags |= NF_PolyOccluded;
					TempDrawList->Span.Release();
				}
				else if
				(	(PolyFlags & PF_FakeBackdrop)
				&&	(Frame->Level->GetZoneActor(iZone)->SkyZone)
				&&	(Frame->Recursion<MAX_FRAME_RECURSION-1)
				&&	(Viewport->Actor->ShowFlags & SHOW_PlayerCtrl) )
				{
					// Handle sky portal.
					guard(HandleSkyWarp);
					AZoneInfo* SkyZone = (AZoneInfo*)Frame->Level->GetZoneActor(iZone)->SkyZone;
					FCoords Coords = Frame->Coords;
					Coords *= Frame->Coords.Origin;
					Coords /= SkyZone->Rotation;
					Coords /= SkyZone->Location;

					FScreenBounds Bounds;
					Bounds.MinY = RasterStartY;
					Bounds.MaxY = RasterEndY;
					Bounds.MinX = RasterStartX;
					Bounds.MaxX = RasterEndX;

					CreateChildFrame
					(
						Frame,
						&TempDrawList->Span,
						Frame->Level,
						0,
						SkyZone->Region.ZoneNumber,
						Frame->Mirror,
						Frame->NearClip,
						Coords,
						Toggle ? NULL : &Bounds
					);
					unguard;
				}
				else if
				(	(PolyFlags & PF_Mirrored)
				&&	(Frame->Recursion<MAX_FRAME_RECURSION-1)
				&&	(Viewport->Actor->ShowFlags & SHOW_PlayerCtrl) )
				{
					// Handle mirrored surface.
					guard(HandleMirrorWarp);
					if( (PolyFlags & PF_Translucent) && !RenDev->ShinySurfaces )
					{
						PolyFlags &= ~PF_Translucent;
						PolyFlags |= PF_Occlude;
						DrawBin = 1;
						goto DrawIt;
					}

					FScreenBounds Bounds;
					Bounds.MinY = RasterStartY;
					Bounds.MaxY = RasterEndY;
					Bounds.MinX = RasterStartX;
					Bounds.MaxX = RasterEndX;

					CreateChildFrame
					(
						Frame,
						&TempDrawList->Span,
						Frame->Level,
						0,
						iZone,
						-Frame->Mirror,
						Node->Plane.TransformPlaneByOrtho( Frame->Coords ).Flip(),
						Frame->Coords.MirrorByPlane(Node->Plane),
						Toggle ? NULL : &Bounds
					);
					DrawBin = 0;
					if( !(PolyFlags & PF_NoOcclude) )
					{
						if( RenDev->SpanBased )
							goto NextCoplanar;
						PolyFlags |= PF_Invisible;
					}
					PolyFlags |= PF_Occlude;
					goto DrawIt;
					unguard;
				}
				else if( (PolyFlags & PF_Portal) && (Viewport->Actor->RendMap!=REN_Zones || (PolyFlags&PF_NoOcclude) ) )
				{
					UBOOL RenderPortal = !(PolyFlags & PF_Invisible);
					BYTE iOppositeZone = Node->iZone[1-IsFront] & ViewZoneMask;
					if( iOppositeZone!=0 && (PolyFlags&PF_NoOcclude) )
					{
						AWarpZoneInfo* Warp = (AWarpZoneInfo*)Model->Nodes->Zones[iOppositeZone].ZoneActor;
						if( !Warp || !Warp->IsA(AWarpZoneInfo::StaticClass) || Frame->Recursion>=MAX_FRAME_RECURSION-1 )
						{
							// Normal zone.
							NormalZone:;
							QWORD OldMask = ActiveZoneMask;
							ActiveZoneMask |= ((QWORD)1)<<iOppositeZone;
							if( ActiveZoneMask != OldMask )
								ActiveZones[NumActiveZones++] = iOppositeZone;
							clock(GStat.SpanTime);
							if( RenderPortal )
								ZoneSpanBuffer[iOppositeZone].MergeWith( FSpanBuffer( TempDrawList->Span, GDynMem) );
							else
								ZoneSpanBuffer[iOppositeZone].MergeWith( TempDrawList->Span );
							Model->Nodes->Zones[iOppositeZone].LastRenderTime = TimeSeconds;
							unclock(GStat.SpanTime);
						}
						else
						{
							// Warp zone.
							if( Warp->OtherSideLevel==NULL || Warp->OtherSideActor==NULL )
							{
								Warp->eventGenerate();
								if( Warp->OtherSideLevel==NULL || Warp->OtherSideActor==NULL )
									goto NormalZone;
							}

							// Handle warp zone.
							guard(HandleWarpPortal);
							CreateChildFrame
							(
								Frame,
								&TempDrawList->Span,
								(ULevel*)Warp->OtherSideLevel,
								Node->iSurf,
								Warp->OtherSideActor->iWarpZone,
								Frame->Mirror,
								(IsFront ? Node->Plane.Flip() : Node->Plane).TransformPlaneByOrtho(Frame->Coords),
								Frame->Coords * Warp->WarpCoords * Warp->OtherSideActor->WarpCoords.Transpose(),
								NULL
							);
							DrawBin = 0;
							if( (PolyFlags & PF_Invisible) && RenDev->SpanBased )
								goto NextCoplanar;
							PolyFlags |= PF_Occlude;
							goto DrawIt;
							unguard;
						}
					}
					if( RenderPortal )
					{
						// Actually display zone portals.
						Merge = 0;
						goto DrawIt;
					}
				}
				else if( !(PolyFlags & PF_Invisible) )
				{
					// Draw it.
					DrawIt:

					// Handle volumetrics.
					static FVector VolCross[32];
					INT NumVolCross=0;
					if( FirstVolumetric )
					{
						for( FTransform** P1=Pts,**P2=Pts+NumPts-1; P1<Pts+NumPts; P2=P1++ )
						{
							if( (*P2)->IntY!=INDEX_NONE )
							{
								VolCross[NumVolCross] = (*P1)->Point ^ (*P2)->Point;
								VolCross[NumVolCross] *= DivSqrtApprox(VolCross[NumVolCross].SizeSquared());
								NumVolCross++;
							}
						}
					}

					// Save drawing info.
					if( !Merge )
					{
						// Create new draw-list entry.
						TempDrawList->iNode		 = iNode;
						TempDrawList->iZone		 = Node->iZone[IsFront];
						TempDrawList->Zone       = Frame->Level->GetZoneActor(TempDrawList->iZone);
						TempDrawList->iSurf		 = Node->iSurf;
						TempDrawList->PolyFlags	 = PolyFlags;
						TempDrawList->Next       = Frame->Draw[DrawBin];

						// Add to linked list.
						TempDrawList->SurfNext  = AllPolyDrawLists[Node->iSurf];
						AllPolyDrawLists[Node->iSurf] = TempDrawList;

						// Save applicable volumetric lights.
						TempDrawList->Volumetrics = NULL;
						for( FVolActorLink* Link=FirstVolumetric; Link; Link=Link->Next )
							if( VolumetricOccludes( Link, VolCross, NumVolCross ) )
								TempDrawList->Volumetrics = new(GDynMem)FActorLink( Link->Actor, TempDrawList->Volumetrics );

						// Save stuff out for hardware rendering.
						if( !RenDev->SpanBased )
						{
							FSavedPoly* Saved   = (FSavedPoly*)New<BYTE>(GDynMem,sizeof(FSavedPoly)+NumPts*sizeof(FTransform*));
							Saved->Next         = NULL;
							TempDrawList->Polys = Saved;
							Saved->NumPts       = NumPts;
							for( INT i=0; i<NumPts; i++ )
								Saved->Pts[i] = Pts[i];;
							TempDrawList->Span.Release();
						}

						// Sort key.
						TempDrawList->Key = TempDrawList->Zone->GetIndex() << (32-6);
						if( Poly->Texture )
						{
							TempDrawList->Key += Poly->Texture->GetIndex();
							if( Poly->Texture->Palette )	
								TempDrawList->Key += Poly->Texture->Palette->GetIndex() << 12;
						}

						Frame->Draw[DrawBin] = TempDrawList;
						TempDrawList = New<FBspDrawList>(GDynMem);
						PolysDraw++;
					}
					else
					{
						// Add to existing draw-list entry.
						if( !RenDev->SpanBased )
						{
							// Save stuff out for hardware rendering.
							FSavedPoly* Saved   = (FSavedPoly*)New<BYTE>(GDynMem,sizeof(FSavedPoly)+NumPts*sizeof(FTransform*));
							Saved->Next         = Merge->Polys;
							Merge->Polys        = Saved;
							Saved->NumPts       = NumPts;
							for( INT i=0; i<NumPts; i++ )
								Saved->Pts[i] = Pts[i];
						}
						else
						{
							clock(GStat.SpanTime);
							Merge->Span.MergeWith( TempDrawList->Span );
							unclock(GStat.SpanTime);
						}
						TempDrawList->Span.Release();

						// Merge in the new volumetrics.
						for( FVolActorLink* Link=FirstVolumetric; Link; Link=Link->Next )
						{
							for( FActorLink* Other=Merge->Volumetrics; Other; Other=Other->Next )
								if( Other->Actor == Link->Actor )
									break;
							if( Other==NULL && VolumetricOccludes( Link, VolCross, NumVolCross ) )
								Merge->Volumetrics = new(GDynMem)FActorLink( Link->Actor, Merge->Volumetrics );
						}
					}
					Node->NodeFlags &= ~NF_PolyOccluded;
					NodesDraw++;

					// See if filled up.
					if( SpanBuffer->ValidLines <= 0 )
					{
						for( INT i=0,j=0; i<NumActiveZones; j+=ActiveZones[i++]!=iZone )
							ActiveZones[j] = ActiveZones[i];
						NumActiveZones=j;
						ActiveZoneMask &= ~(((QWORD)1)<<iZone);
						if( NumActiveZones==0 )
							goto DoneRendering;
					}
				}

				NextCoplanar:
				iNode = Node->iPlane;
				if( iNode==INDEX_NONE )
					break;
				Node	= &GNodes[iNode];
				Dot		= Node->Plane.PlaneDot(Origin);
				IsFront	= Dot > 0.0;
			}
			iNode      = iOriginalNode;
			Node       = &GNodes[iNode];
			Dot		   = Node->Plane.PlaneDot( Origin );
			IsFront	   = Dot > 0.0;
			iThingZone = Node->iZone[1-IsFront] & ViewZoneMask;

			// Render dynamic stuff behind the plane.
			if( IsVolumetric && Node->iLeaf[1-IsFront]!=INDEX_NONE )
				LeafVolumetricLighting( Frame, Model, Node->iLeaf[1-IsFront] );
			if( ZoneSpanBuffer[iThingZone].ValidLines && (Node->ChildOutside(1-IsFront,Outside)||Toggle) )
				for( FDynamicItem* Item = Dynamic(iNode,IsFront); Item; Item=Item->FilterNext )
					Item->PreRender( Viewport, Frame, &ZoneSpanBuffer[iThingZone], iNode, FirstVolumetric );

			// Set up recursion for back.
			if( Stack->iFarNode != INDEX_NONE )
			{
				iNode				= Stack->iFarNode;
				Outside				= Stack->FarOutside;
				Pass				= PASS_Front;
				continue;
			}
		}

		// Return from recursion, noting that the node we're returning to is guaranteed visible if the
		// child we're processing now is visible.
		PopStack:
		Stack = Stack->Next;
		if( !Stack )
			break;

		iNode		= Stack->iNode;
		Outside		= Stack->Outside;
		Pass		= Stack->Pass;
	}
	DoneRendering:

	// Update stats.
	STAT(GStat.NumZones += Model->Nodes->NumZones);
	STAT(GStat.CurZone=iViewZone);
	for( i=0; i<UBspNodes::MAX_ZONES; i++ )
		if( ZoneSpanBuffer[i].EndY )
			STAT(GStat.VisibleZones++);

	STAT(unclock(GStat.OcclusionTime));
	unguard;
}

/*-----------------------------------------------------------------------------
	Span buffer Bsp rendering.
-----------------------------------------------------------------------------*/

//
// BSP draw list pointer for sorting.
//
struct FBspDrawListPtr
{
	FBspDrawList *Ptr;
	friend inline INT Compare( const FBspDrawListPtr &A, const FBspDrawListPtr &B )
		{ return A.Ptr->Key - B.Ptr->Key; }
};

//
// Temporary optics.
//
struct FCoronaLight
{
	AActor* _Actor;
	INT     iActor;
	FLOAT   Bright;
};
#define MAX_CORONA_LIGHTS 32
static void GAddCorona( FSceneNode* Frame, FCoronaLight* CoronaLights, INT& iFree, AActor* Light, FLOAT Delta )
{
	FCheckResult Hit;
	if
	(	Light->bCorona
	&&	Light->Skin
	&& !Light->bDeleteMe
	&&  Frame->Level->SingleLineCheck( Hit, NULL, Light->Location, Frame->Coords.Origin, TRACE_VisBlocking, FVector(0,0,0) ) )
	{
		for( int i=0; i<MAX_CORONA_LIGHTS; i++ )
			if( CoronaLights[i]._Actor == Light )
				break;
		if( i<MAX_CORONA_LIGHTS )
		{
			CoronaLights[i].Bright = Min(1.f,CoronaLights[i].Bright+2.f*Delta);
		}
		else
		{
			while( iFree<MAX_CORONA_LIGHTS && CoronaLights[iFree]._Actor )
				iFree++;
			if( iFree<MAX_CORONA_LIGHTS )
			{
				CoronaLights[iFree]._Actor = Light;
				CoronaLights[iFree].iActor = Light->XLevel->GetActorIndex(Light);
				CoronaLights[iFree].Bright = Min(1.f,2.f*Delta);
			}
		}
	}
}

//
// Draw the entire world.
//
void URender::OccludeFrame( FSceneNode* Frame )
{
	guard(URender::DrawLevel);
	UViewport* Viewport = Frame->Viewport;
	ULevel*    Level    = Frame->Level;
	UModel*	   Model    = Level->Model;
	check(Model->Nodes->Num()>0);

	// Init rendering info.
	if( SurfLights==NULL || Level->Model->Surfs->Max()>MaxSurfLights )
	{
		MaxSurfLights = Level->Model->Surfs->Max();
		SurfLights    = (FActorLink**)appRealloc( SurfLights, MaxSurfLights * sizeof(FActorLink*), "SurfLights" );
		appMemset( SurfLights, 0, MaxSurfLights * sizeof(FActorLink*) );
	}
	if( Level->Model->Leaves.Num() && (LeafLights==NULL || Level->Model->Leaves.Num()>MaxLeafLights) )
	{
		MaxLeafLights = Level->Model->Leaves.Num();
		LeafLights    = (FVolActorLink**)appRealloc( LeafLights, MaxLeafLights * sizeof(FActorLink*), "LeafLights" );
		appMemset( LeafLights, 0, MaxLeafLights * sizeof(FVolActorLink*) );
	}
	NumDynLightSurfs  = 0;
	NumDynLightLeaves = 0;
	NumPostDynamics   = 0;
	PostDynamics      = new(GDynMem,Level->Model->Nodes->Max())URender::FDynamicsCache*;

	// Perform occlusion checking.
	SetupDynamics( Frame, (Viewport->Actor->bBehindView || Frame->Parent!=NULL) ? NULL : Viewport->Actor->ViewTarget ?  Viewport->Actor->ViewTarget : Viewport->Actor );
	OccludeBsp( Frame );

	// Remember surface lights.
	for( INT i=0; i<3; i++ )
		for( FBspDrawList* Draw=Frame->Draw[i]; Draw; Draw=Draw->Next )
			Draw->SurfLights = SurfLights[Draw->iSurf];

	// Remember visible actor leaf lights.
	if( Level->Model->Leaves.Num() )
		for( FDynamicSprite* Sprite = Frame->Sprite; Sprite; Sprite=Sprite->RenderNext )
			if( Sprite->Actor->Region.iLeaf!=INDEX_NONE )
				Sprite->LeafLights = LeafLights[ Sprite->Actor->Region.iLeaf ];

	// Cleanup rendering info.
	for( i=0; i<NumPostDynamics; i++ )
	{
		PostDynamics[i]->Dynamics[0] = NULL;
		PostDynamics[i]->Dynamics[1] = NULL;
	}
	for( i=0; i<NumDynLightSurfs; i++ )
	{
		SurfLights[ DynLightSurfs[i] ] = NULL;
	}
	for( i=0; i<NumDynLightLeaves; i++ )
	{
		LeafLights[ DynLightLeaves[i] ] = NULL;
	}

	// Occlude child frames.
	for( FSceneNode* F=Frame->Child; F; F=F->Sibling )
		OccludeFrame( F );

	unguard;
}

void URender::DrawFrame( FSceneNode* Frame )
{
	guard(URender::DrawFrame);
	UViewport* Viewport = Frame->Viewport;
	UModel*	   Model    = Frame->Level->Model;
	check(Model->Nodes->Num()>0);

	// First, draw children.
	for( FSceneNode* F=Frame->Child; F; F=F->Sibling )
		DrawFrame( F );

	// Clear the Z-buffer if portal surfaces are visible.
	if( Frame->Draw[0] )
		Viewport->RenDev->ClearZ( Frame );

	// Count surfaces to draw.
	INT Num[3]={0,0,0};
	for( INT Pass=0; Pass<3; Pass++ )
		for( FBspDrawList* Draw = Frame->Draw[Pass]; Draw; Draw = Draw->Next )
			Num[Pass]++;

	// Group surfaces into solid (draw-order invariant) and transparent.
	FBspDrawListPtr* FirstDraw [3] = {new(GMem,Num[0])FBspDrawListPtr,new(GMem,Num[1])FBspDrawListPtr,new(GMem,Num[2])FBspDrawListPtr};
	FBspDrawListPtr* LastDraw  [3] = {FirstDraw[0],FirstDraw[1],FirstDraw[2]};
	for( Pass=0; Pass<3; Pass++ )
		for( FBspDrawList* Draw = Frame->Draw[Pass]; Draw; Draw = Draw->Next )
			(LastDraw[Pass]++)->Ptr = Draw;
	for( INT i=0; i<Num[0]/2; i++ )
		Exchange( FirstDraw[0][i], FirstDraw[0][Num[0]-i-1] );

	// Sort solid surfaces by texture and then by palette for cache coherence.
	appSort( FirstDraw[1], Num[1] );

	// Render everything.
	for( Pass=0; Pass<3; Pass++ )
	{
		// Draw everything in the world.
		for( FBspDrawListPtr* DrawPtr = FirstDraw[Pass]; DrawPtr<LastDraw[Pass]; DrawPtr++ )
		{
			// Setup for this surface.
			FBspDrawList*	Draw = DrawPtr->Ptr;
			FBspSurf*		Surf = &Model->Surfs->Element( Draw->iSurf );

			// Setup panning.
			FLOAT PanU = Surf->PanU;
			if( Surf->PolyFlags & PF_AutoUPan )
			{
				PanU += ((INT)(Frame->Level->GetLevelInfo()->TimeSeconds * 35.f * Draw->Zone->TexUPanSpeed * 256.0)&0x3ffff)/256.0;
			}
			FLOAT PanV = Surf->PanV;
			if( Surf->PolyFlags & PF_AutoVPan )
			{
				PanV += ((INT)(Frame->Level->GetLevelInfo()->TimeSeconds * 35.f * Draw->Zone->TexVPanSpeed * 256.0)&0x3ffff)/256.0;
			}
			if( Surf->PolyFlags & (PF_SmallWavy | PF_BigWavy) )
			{
				FLOAT T = Frame->Level->GetLevelInfo()->TimeSeconds;
				PanU += 8.0 * appSin(T) + 4.0 * appCos(2.3*T);
				PanV += 8.0 * appCos(T) + 4.0 * appSin(2.3*T);
			}

			// Make SurfaceInfo.
			FSurfaceInfo Surface;
			Surface.Level			= Frame->Level;
			Surface.PolyFlags		= Draw->PolyFlags;
			Surface.LightMap		= NULL;
			Surface.MacroTexture	= NULL;
			Surface.DetailTexture	= NULL;
			Surface.FogMap			= NULL;
			Surface.BumpMap			= NULL;

			// Make TextureMap.
			FTextureInfo TextureMap;
			UTexture* Texture		= Surf->Texture ? Surf->Texture->Get(Viewport->CurrentTime) : Viewport->Actor->Level->DefaultTexture;
			Texture->GetInfo( TextureMap, Viewport->CurrentTime );
			TextureMap.Pan			= FVector( -PanU, -PanV, 0 );
			Surface.Texture			= &TextureMap;

			// Make BumpMap.
			FTextureInfo BumpMap;
			if( Texture->BumpMap )
			{
				Texture->BumpMap->GetInfo( BumpMap, Viewport->CurrentTime );
				Surface.BumpMap = &BumpMap;
			}

			// Make DetailTexture.
			FTextureInfo DetailTexture;
			if( Texture->DetailTexture && !(Surface.PolyFlags & PF_Portal) )
			{
				Texture->DetailTexture->GetInfo( DetailTexture, Viewport->CurrentTime );
				Surface.DetailTexture = &DetailTexture;
			}

			// Make MacroTexture.
			FTextureInfo MacroTexture;
			if( Texture->MacroTexture )
			{
				Texture->MacroTexture->GetInfo( MacroTexture, Viewport->CurrentTime );
				Surface.MacroTexture = &MacroTexture;
			}

			// Make SurfaceFacet.
			FSurfaceFacet Facet;
			Facet.Polys = Draw->Polys;
			Facet.Span = &Draw->Span;
			Facet.MapCoords = FCoords
			(
				Model->Points->Element (Surf->pBase),
				Model->Vectors->Element(Surf->vTextureU),
				Model->Vectors->Element(Surf->vTextureV),
				Model->Vectors->Element(Surf->vNormal)
			);

			// Setup lighting for this surface.
			if
			(	Surf->iLightMap!=INDEX_NONE
			&&	Viewport->Actor->RendMap==REN_DynLight
			&&	Model->LightMap.Num() 
			&&	!Viewport->Client->NoLighting )
				GLightManager->SetupForSurf
				(
					Frame,
					Facet.MapCoords,
					Draw,
					Surface.LightMap,
					Surface.FogMap,
					Surface.BumpMap,
					Pass==0
				);

			// Update facet.
			Facet.MapCoords *= Frame->Coords;

			// Handle flatshading.
			if
			(	Viewport->Actor->RendMap==REN_Polys
			||	Viewport->Actor->RendMap==REN_PolyCuts
			||	Viewport->Actor->RendMap==REN_Zones )
			{
				UModel*		Model		= Viewport->Actor->XLevel->Model;
				FBspNode*	Node 		= &Model->Nodes->Element( Draw->iNode );
				FBspSurf*	Surf 		= &Model->Surfs->Element( Node->iSurf );
				UTexture*	Texture		= Surf->Texture ? Surf->Texture->Get(Viewport->CurrentTime) : Viewport->Actor->Level->DefaultTexture;
				FVector Color;
				if( Viewport->Actor->RendMap!=REN_Zones || Model->Nodes->NumZones==0 )
				{
					INT Index = Viewport->Actor->RendMap==REN_Polys ? Node->iSurf : Draw->iNode;
					Color = Texture->MipZero.Plane() * (0.5 + (Index&7)/16.0);
				}
				else
				{
					if( Draw->iZone == 0 )
						Color = Texture->MipZero.Plane();
					else
						Color = FVector( (Draw->iZone*67)&255, (Draw->iZone*1371)&255, (Draw->iZone*1991)&255 )/256.0;
					Color *= (0.5 + (Draw->iNode&7)/16.0);
				}
				Surface.FlatColor = FColor(Color);
				Surface.PolyFlags |= PF_FlatShaded;
			}

			// Draw the surface.
			PUSH_HIT(Frame,HBspSurf, Draw->iSurf);
			clock(GStat.PolyVTime);
			Viewport->RenDev->DrawComplexSurface( Frame, Surface, Facet );
			unclock(GStat.PolyVTime);
			POP_HIT(Frame);

			// Finish up.
			if( Surface.LightMap || Surface.FogMap )
				GLightManager->FinishSurf();
		}
		if( Viewport->RenDev->SpanBased ? Pass==2 : Pass==1 )
			for( FDynamicSprite* Sprite = Frame->Sprite; Sprite; Sprite=Sprite->RenderNext )
				DrawActorSprite( Frame, Sprite );
	}

	// Optics.
	if
	(	Viewport->Actor->Region.iLeaf!=INDEX_NONE
	&&	Viewport->Actor->Region.Zone 
	&&	Viewport->RenDev->Coronas 
	&&	Frame->Recursion==0 )
	{
		// Do coronas.
		FCacheItem* Item;
		QWORD CacheID = MakeCacheID( CID_CoronaCache, (UObject*)0 );
		FCoronaLight* CoronaLights = (FCoronaLight*)GCache.Get( CacheID, Item );
		if( !CoronaLights )
		{
			CoronaLights = (FCoronaLight*)GCache.Create( CacheID, Item, MAX_CORONA_LIGHTS * sizeof(FCoronaLight) );
			for( int i=0; i<MAX_CORONA_LIGHTS; i++ )
				CoronaLights[i]._Actor = NULL;
		}

		// Corona and lens flare lighting.
		static DOUBLE LastTime = appSeconds();
		FLOAT Delta            = 3.0 * (appSeconds() - LastTime);
		LastTime               = appSeconds();
		for( int i=0; i<MAX_CORONA_LIGHTS; i++ )
			if( CoronaLights[i]._Actor && (CoronaLights[i].Bright-=Delta)<0 )
				CoronaLights[i]._Actor = NULL;
		INT iFree=0;
		INT iPermeating = Viewport->Actor->XLevel->Model->Leaves(Viewport->Actor->Region.iLeaf).iPermeating;
		if( iPermeating!=INDEX_NONE )
		{
			AActor** LightPtr = &Viewport->Actor->XLevel->Model->Lights(iPermeating);
			while( *LightPtr )
				GAddCorona( Frame, CoronaLights, iFree, *LightPtr++, Delta );
		}
		for( FVolActorLink* Link=LeafLights[Viewport->Actor->Region.iLeaf]; Link; Link=Link->Next )
			GAddCorona( Frame, CoronaLights, iFree, Link->Actor, Delta );
		for( i=0; i<MAX_CORONA_LIGHTS; i++ )
		{
			AActor* Light = CoronaLights[i]._Actor;
			if( Light && CoronaLights[i].iActor<=Viewport->Actor->XLevel->Num() && Viewport->Actor->XLevel->Actors(CoronaLights[i].iActor)==Light )
			{
				check(Light->IsValid());
				FVector Loc = Light->Location.TransformPointBy( Frame->Coords );
				if( Loc.Z > 1.0 )
				{
					BYTE    H     = Light->LightHue;
					FVector Hue   = (H<86) ? FVector((85-H)/85.f,(H-0)/85.f,0) : (H<171) ? FVector(0,(170-H)/85.f,(H-85)/85.f) : FVector((H-170)/85.f,0,(255-H)/84.f);
					FLOAT	Alpha = Light->LightSaturation / 255.0;
					FVector Color = (Hue + Alpha * (FVector(1,1,1) - Hue));
					FLOAT   RZ    = Frame->Proj.Z / Loc.Z;
					FLOAT   X     = Loc.X * RZ + Frame->FX2;
					FLOAT   Y     = Loc.Y * RZ + Frame->FY2;
					FLOAT   Scale = 512.f * Light->DrawScale * Frame->X/640;
					/*for( int j=0; j<5; j++ )
					{
						FLOAT A  = (j-2)/1.3;
						FLOAT XX = X + (Viewport->FX2-X)*A;
						FLOAT YY = Y + (Viewport->FY2-Y)*A;
						FLOAT Sc = Scale * (1.0-Abs(j-2)/2.2);
						GRend->DrawIcon( Viewport, Light->Region.Zone->LensFlares[Abs(j-2)], XX-Sc/2, YY-Sc/2, Sc, Sc, NULL, 1.0, GCoronaLights[i].Bright * Color );
					}*/
					Viewport->Canvas->DrawIcon( Light->Skin, X-Scale/2, Y-Scale/2, Scale, Scale, NULL, 1.0, CoronaLights[i].Bright * Color, FPlane(0,0,0,0), PF_TwoSided | PF_Translucent );
				}
			}
		}
		Item->Unlock();
	}

	// Sky cleanup.
	if( Cast<ASkyZoneInfo>(Frame->Level->Model->Nodes->Zones[Frame->ZoneNumber].ZoneActor) )
		Viewport->RenDev->ClearZ( Frame );

	// Finish up.
	STAT(GStat.GMem += GMem.GetByteCount());
	STAT(GStat.GDynMem += GDynMem.GetByteCount());
	STAT(GStat.NodesTotal += Frame->Level->Model->Nodes->Num());
	unguard;
}

//
// Draw the entire world.
//
void URender::DrawWorld( FSceneNode* Frame )
{
	guard(URender::DrawWorld);
	FMemMark SceneMark(GSceneMem);
	FMemMark MemMark(GMem);
	FMemMark DynMark(GDynMem);
	FMemMark VectorMark( VectorMem );
	GFrameStamp++;

	// Occlude and render all scene frames.
	OccludeFrame( Frame );
	DrawFrame( Frame );

	// Draw the player's weapon on top.
	APawn* Actor
	= Frame->Viewport->Actor->ViewTarget ? Cast<APawn>( Frame->Viewport->Actor->ViewTarget )
	: Frame->Viewport->Actor->bBehindView ? NULL 
	: Frame->Viewport->Actor;
	if
	(	!GIsEditor
	&&	Actor
	&&	Actor->Weapon
	&&	(Frame->Viewport->Actor->ShowFlags & SHOW_Actors) )
	{
		Actor->Weapon->eventInvCalcView();
		Actor->Weapon->bHidden = 0;
		Actor->XLevel->SetActorZone( Actor->Weapon, 1, 0 );
		GRender->DrawActor( Frame, Actor->Weapon );
		Actor->Weapon->bHidden = 1;
	}

	MemMark.Pop();
	DynMark.Pop();
	SceneMark.Pop();
	VectorMark.Pop();
	unguard;
}

/*-----------------------------------------------------------------------------
	Global subsystem instantiation.
-----------------------------------------------------------------------------*/

RENDER_API URender*	GRender = NULL;

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
