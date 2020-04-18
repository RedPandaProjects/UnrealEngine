/*=============================================================================
	Line2D.cpp: 2D line and point drawing.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "SoftDrvPrivate.h"

/*-----------------------------------------------------------------------------
	Line drawing.
-----------------------------------------------------------------------------*/

//
// Draw a pre-clipped 2D line.
//
void USoftwareRenderDevice::Draw2DLine
(
	FSceneNode*	Frame,
	FPlane			InColor,
	DWORD			LineFlags,
	FVector			P1,
	FVector			P2
)
{
	guard(URender::Draw2DLine);
	FColor Color(InColor);
	if( LineFlags & LINE_DepthCued )
	{
		// Get palette table.
		FRainbowPtr ColorTab;
		FCacheItem* Item;
		QWORD CacheID    = MakeCacheID( CID_AALineTable, Color.HiColor565(), Viewport->ByteID(), NULL );
		ColorTab.PtrVOID = GCache.Get( CacheID, Item );
		if( ColorTab.PtrVOID == NULL )
		{
			ColorTab.PtrVOID      = GCache.Create( CacheID, Item, 256 * Viewport->ColorBytes );
			FVector C             = Color.Plane();
			FLOAT GammaCorrection = 0.56;
			for( int i=0; i<256; i++ )
			{
				FLOAT GFactor = appPow( i/256.0, GammaCorrection );
				if( Viewport->ColorBytes==2 && (Viewport->Caps & CC_RGB565) )
					ColorTab.PtrWORD[i]
					=	( appFloor(256 * C.B * GFactor / 8.0f) << 0 )
					+	( appFloor(256 * C.G * GFactor / 4.0f) << 5 )
					+	( appFloor(256 * C.R * GFactor / 8.0f) << 11);
				else if( Viewport->ColorBytes==2 )
					ColorTab.PtrWORD[i]
					=	( appFloor(256 * C.B * GFactor / 8.0f) << 0 )
					+	( appFloor(256 * C.G * GFactor / 8.0f) << 5 )
					+	( appFloor(256 * C.R * GFactor / 8.0f) << 10);
				else if( Viewport->ColorBytes==4 )
					ColorTab.PtrDWORD[i]
					=	( appFloor(256 * C.B * GFactor) << 0 )
					+	( appFloor(256 * C.G * GFactor) << 8 )
					+	( appFloor(256 * C.R * GFactor) << 16);
				else appErrorf( "Invalid ColorBytes" );
			}
		}

		// Depth cued line drawer.
		#define DEPTHSETUP(Arclen) FixDG = (FixG2-FixG1)/Arclen;
		INT FixG1 = ::Clamp( appRound(P1.Z * 65536.0 * 10000.0), 100*65536, 255*65536 );
		INT FixG2 = ::Clamp( appRound(P2.Z * 65536.0 * 10000.0), 100*65536, 255*65536 );
		INT FixDG;
		if( Viewport->ColorBytes == 1 )
		{
			#define DRAWPIXEL(Dest) *(BYTE*)(Dest)=ColorTab.PtrBYTE[Unfix(FixG1 += FixDG)];
			#define SHIFT 0
			#define LABEL1(X) X##D1
			#include "Line1.h"
			#undef  LABEL1
			#undef  SHIFT
			#undef  DRAWPIXEL
			#undef  ASMPIXEL
		}
		else if( Viewport->ColorBytes == 2 )
		{
			#define DRAWPIXEL(Dest) *(_WORD *)(Dest)=ColorTab.PtrWORD[Unfix(FixG1 += FixDG)];
			#define SHIFT 1
			#define LABEL1(X) X##D2_565
			#include "Line1.h"
			#undef  LABEL1
			#undef  SHIFT
			#undef  DRAWPIXEL
		}
		else
		{
			#define DRAWPIXEL(Dest) *(DWORD *)(Dest)=ColorTab.PtrDWORD[Unfix(FixG1 += FixDG)];
			#define SHIFT 2
			#define LABEL1(X) X##D4
			#include "Line1.h"
			#undef  SHIFT
			#undef  LABEL1
			#undef  DRAWPIXEL
		}
		#undef DEPTHSETUP
		Item->Unlock();
	}
	else
	{
		// Flat shaded line drawer.
		if( Viewport->ColorBytes==1 )
		{
			INT NewColor = Color.A-16;
			#define DRAWPIXEL(Dest) *(Dest)=NewColor
			#define ASMPIXEL mov [edi],al
			#define SHIFT 0
			#define LABEL1(X) X##C1
			#include "Line1.h"
			#undef  LABEL1
			#undef  SHIFT
			#undef  DRAWPIXEL
			#undef  ASMPIXEL
		}
		else if( Viewport->ColorBytes == 2 )
		{
			_WORD HiColor = (Viewport->Caps & CC_RGB565) ? Color.HiColor565() : Color.HiColor555();
			#define DRAWPIXEL(Dest) *(_WORD*)(Dest)=HiColor
			#define SHIFT 1
			#define LABEL1(X) X##C2
			#include "Line1.h"
			#undef  LABEL1
			#undef  SHIFT
			#undef  DRAWPIXEL
		}
		else
		{
			DWORD TrueColor = Color.TrueColor();
			#define DRAWPIXEL(Dest) *(DWORD*)(Dest)=TrueColor
			#define SHIFT 2
			#define LABEL1(X) X##C4
			#include "Line1.h"
			#undef  LABEL1
			#undef  SHIFT
			#undef  DRAWPIXEL
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Point drawing.
-----------------------------------------------------------------------------*/

//
// Draw a clipped rectangle.
//
void USoftwareRenderDevice::Draw2DPoint
(
	FSceneNode*	Frame,
	FPlane			InColor,
	DWORD			LineFlags,
	FLOAT			FX1,
	FLOAT			FY1,
	FLOAT			FX2,
	FLOAT			FY2
)
{
	guard(URender::Draw2DPoint);
	FColor Color(InColor);
	INT X1 = appFloor(FX1);
	INT X2 = appFloor(FX2);
	INT Y1 = appFloor(FY1);
	INT Y2 = appFloor(FY2);
	if( X2<0 || Y2<0 || X1>=Frame->X || Y2>=Frame->Y )
		return;

	if( X1<0          ) X1 = 0;
	if( Y1<0          ) Y1 = 0;
	if( ++X2>Frame->X ) X2 = Frame->X; 
	if( ++Y2>Frame->Y ) Y2 = Frame->Y;

	INT         YL    = Y2-Y1;
	INT         XL    = X2-X1;
	FRainbowPtr Dest1 = Frame->Screen(X1,Y1);
	if( Viewport->ColorBytes==1 )
	{
		for( INT Y=0; Y<YL; Y++ )
		{
			FRainbowPtr Dest = Dest1;
			for( INT X=0; X<XL; X++ )
				*Dest.PtrBYTE++ = Color.A;
			Dest1.PtrBYTE += Viewport->Stride;
		}
	}
	else if( Viewport->ColorBytes==2 )
	{
		_WORD HiColor = (Viewport->Caps & CC_RGB565) ? Color.HiColor565() : Color.HiColor555();
		for( INT Y=0; Y<YL; Y++ )
		{
			FRainbowPtr Dest = Dest1;
			for( INT X=0; X<XL; X++ )
				*Dest.PtrWORD++ = HiColor;
			Dest1.PtrWORD += Viewport->Stride;
		}
	}
	else
	{
		DWORD TrueColor = Color.TrueColor();
		for( INT Y=0; Y<YL; Y++ )
		{
			FRainbowPtr Dest = Dest1;
			for( INT X=0; X<XL; X++ )
				*Dest.PtrDWORD++ = TrueColor;
			Dest1.PtrDWORD += Viewport->Stride;
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
