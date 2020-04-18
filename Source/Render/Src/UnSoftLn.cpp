/*=============================================================================
	UnSoftLn.cpp: Unreal software line drawing.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "RenderPrivate.h"

/*-----------------------------------------------------------------------------
	Line drawing.
-----------------------------------------------------------------------------*/

//
// Draw a clipped 2D line.
//
void URender::Draw2DClippedLine
(
	FSceneNode*	Frame,
	FPlane			Color,
	DWORD			LineFlags,
	FVector			P1,
	FVector			P2
)
{
	guard(URender::Draw2DClippedLine);

	// X clip it.
	if( P1.X > P2.X )
		Exchange( P1, P2 );
	if( P2.X<0 || P1.X>Frame->FX )
		return;
	if( P1.X<0 )
	{
		if( Abs(P2.X-P1.X)<0.001 )
			return;
		P1.Y += (0-P1.X)*(P2.Y-P1.Y)/(P2.X-P1.X);
		P1.X  = 0;
	}
	if( P2.X>=Frame->FX )
	{
		if( Abs(P2.X-P1.X)<0.001 )
			return;
		P2.Y += ((Frame->FX-1.0)-P2.X)*(P2.Y-P1.Y)/(P2.X-P1.X);
		P2.X  = Frame->FX-1.0;
	}

	// Y clip it.
	if( P1.Y > P2.Y )
		Exchange( P1, P2 );
	if( P2.Y < 0 || P1.Y > Frame->FY )
		return;
	if( P1.Y < 0 )
	{
		if( Abs(P2.Y-P1.Y)<0.001 )
			return;
		P1.X += (0-P1.Y)*(P2.X-P1.X)/(P2.Y-P1.Y);
		P1.Y  = 0;
	}
	if( P2.Y >= Frame->FY )
	{
		if( Abs(P2.Y-P1.Y)<0.001 )
			return;
		P2.X += ((Frame->FY-1.0)-P2.Y)*(P2.X-P1.X)/(P2.Y-P1.Y);
		P2.Y  = Frame->FY-1.0;
	}

	// Draw it. 
	ClipFloatFromZero(P1.X,Frame->FX); // performs *fast* Clamp(P1.X,0.f,Frame->FX)
	ClipFloatFromZero(P2.X,Frame->FX);
	ClipFloatFromZero(P1.Y,Frame->FY);
	ClipFloatFromZero(P2.Y,Frame->FY);

	Frame->Viewport->RenDev->Draw2DLine( Frame, Color, LineFlags, P1, P2 );

	unguard;
}

//
// Draw a clipped 3D line.
//
void URender::Draw3DLine
(
	FSceneNode*	Frame,
	FPlane			Color,
	DWORD			LineFlags,
	FVector			P1,
	FVector			P2
)
{
	guard(URender::Draw3DLine);

	FLOAT SX  = Frame->FX-1;
	FLOAT SY  = Frame->FY-1;
	FLOAT SX2 = Frame->FX2;
	FLOAT SY2 = Frame->FY2;

	// Transform.
	P1 = P1.TransformPointBy( Frame->Coords );
	P2 = P2.TransformPointBy( Frame->Coords );

	if( Frame->Viewport->IsOrtho() )
	{
		// Zoom.
		P1 = P1 * Frame->RZoom + FVector( SX2, SY2, 0 );
		P2 = P2 * Frame->RZoom + FVector( SX2, SY2, 0 );

		// See if points form a line parallel to our line of sight (i.e. line appears as a dot).
		if( Abs(P2.X-P1.X)+Abs(P1.Y-P2.Y)<0.2 )
		{
			// Line is visible as a point.
			if( Frame->Viewport->Actor->OrthoZoom < ORTHO_LOW_DETAIL )
				Frame->Viewport->RenDev->Draw2DPoint( Frame, Color, LINE_None, P1.X-1, P1.Y-1, P1.X+1, P1.Y+1 );
			return;
		}
	}
	else
	{
		// Calculate delta, discard line if points are identical.
		FVector D = P2-P1;
		if( D.SizeSquared() < Square(0.01) )
			return;

		// Clip to near clipping plane.
		if( P1.Z <= LINE_NEAR_CLIP_Z )
		{
			// Clip P1 to NCP.
			if( P2.Z<(LINE_NEAR_CLIP_Z-0.01) )
				return;
			P1.X +=  (LINE_NEAR_CLIP_Z-P1.Z) * D.X/D.Z;
			P1.Y +=  (LINE_NEAR_CLIP_Z-P1.Z) * D.Y/D.Z;
			P1.Z  =  (LINE_NEAR_CLIP_Z);
		}
		else if( P2.Z<(LINE_NEAR_CLIP_Z-0.01) )
		{
			// Clip P2 to NCP.
			P2.X += (LINE_NEAR_CLIP_Z-P2.Z) * D.X/D.Z;
			P2.Y += (LINE_NEAR_CLIP_Z-P2.Z) * D.Y/D.Z;
			P2.Z =  (LINE_NEAR_CLIP_Z);
		}

		// Calculate perspective.
		P1.Z = 1.0/P1.Z; P1.X = P1.X * Frame->Proj.Z * P1.Z + SX2; P1.Y = P1.Y * Frame->Proj.Z * P1.Z + SY2;
		P2.Z = 1.0/P2.Z; P2.X = P2.X * Frame->Proj.Z * P2.Z + SX2; P2.Y = P2.Y * Frame->Proj.Z * P2.Z + SY2;
	}

	// Clip it and draw it.
	Draw2DClippedLine( Frame, Color, LineFlags, P1, P2 );

	unguard;
}

/*-----------------------------------------------------------------------------
	Projection.
-----------------------------------------------------------------------------*/

//
// Figure out the unclipped screen location of a 3D point taking into account either
// a perspective or orthogonal projection.  Returns 1 if view is orthogonal or point 
// is visible in 3D view, 0 if invisible in 3D view (behind the viewer).
//
// Scale = scale of one world unit (at this point) relative to screen pixels,
// for example 0.5 means one world unit is 0.5 pixels.
//
UBOOL URender::Project( FSceneNode* Frame, const FVector& V, FLOAT& ScreenX, FLOAT& ScreenY, FLOAT* Scale )
{
	guard(URender::Project);

	FVector	Temp = V - Frame->Coords.Origin;
	if( Frame->Viewport->Actor->RendMap == REN_OrthXY )
	{
		ScreenX = +Temp.X * Frame->RZoom + Frame->FX2;
		ScreenY = +Temp.Y * Frame->RZoom + Frame->FY2;
		if( Scale )
			*Scale = Frame->RZoom;
		return 1;
	}
	else if( Frame->Viewport->Actor->RendMap==REN_OrthXZ )
	{
		ScreenX = +Temp.X * Frame->RZoom + Frame->FX2;
		ScreenY = -Temp.Z * Frame->RZoom + Frame->FY2;
		if( Scale )
			*Scale = Frame->RZoom;
		return 1;
	}
	else if( Frame->Viewport->Actor->RendMap==REN_OrthYZ )
	{
		ScreenX = +Temp.Y * Frame->RZoom + Frame->FX2;
		ScreenY = -Temp.Z * Frame->RZoom + Frame->FY2;
		if( Scale )
			*Scale = Frame->RZoom;
		return 1;
	}
	else
	{
		Temp     = Temp.TransformVectorBy( Frame->Coords );
		FLOAT Z  = Temp.Z; if (Abs (Z)<0.01) Z+=0.02;
		FLOAT RZ = Frame->Proj.Z / Z;
		ScreenX = Temp.X * RZ + Frame->FX2;
		ScreenY = Temp.Y * RZ + Frame->FY2;

		if( Scale  )
			*Scale = RZ;

		return Z > 1.0;
	}
	unguard;
}

//
// Convert a particular screen location to a world location.  In ortho views,
// sets non-visible component to zero.  In persp views, places at viewport location
// unless UseEdScan=1 and the user just clicked on a wall (a Bsp polygon).
// Sets V to location and returns 1, or returns 0 if couldn't perform conversion.
//
UBOOL URender::Deproject( FSceneNode* Frame, INT ScreenX, INT ScreenY, FVector& V )
{
	guard(URender::Deproject);

	FVector  Origin = Frame->Coords.Origin;
	FLOAT	 SX		= (FLOAT)ScreenX - Frame->FX2;
	FLOAT	 SY		= (FLOAT)ScreenY - Frame->FY2;

	switch( Frame->Viewport->Actor->RendMap )
	{
		case REN_OrthXY:
			V.X = +SX / Frame->RZoom + Origin.X;
			V.Y = +SY / Frame->RZoom + Origin.Y;
			V.Z = 0;
			return 1;
		case REN_OrthXZ:
			V.X = +SX / Frame->RZoom + Origin.X;
			V.Y = 0.0;
			V.Z = -SY / Frame->RZoom + Origin.Z;
			return 1;
		case REN_OrthYZ:
			V.X = 0.0;
			V.Y = +SX / Frame->RZoom + Origin.Y;
			V.Z = -SY / Frame->RZoom + Origin.Z;
			return 1;
		default:
			V = Origin;
			return 0;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Circle drawing.
-----------------------------------------------------------------------------*/

//
// Draw a circle.
//
void URender::DrawCircle
(
	FSceneNode*	Frame,
	FPlane			Color,
	DWORD			LineFlags,
	FVector&		Location,
	FLOAT			Radius
)
{
	guard(URender::DrawCircle);

	FVector A = Frame->Coords.XAxis;
	FVector B = Frame->Coords.YAxis;

	int Subdivide = 8;
	for
	(	FLOAT Thresh = Frame->Viewport->Actor->OrthoZoom/Radius
	;	Thresh<2048 && Subdivide<256
	;	Thresh*=2,Subdivide*=2 );

	FLOAT   F  = 0.0;
	FLOAT   AngleDelta = 2.0f*PI / Subdivide;

	FVector P1 = Location + Radius * (A * appCos(F) + B * appSin(F));

	for( int i=0; i<Subdivide; i++ )
	{
		F          += AngleDelta;
		FVector P2  = Location + Radius * (A * appCos(F) + B * appSin(F));
		Draw3DLine( Frame, Color, LineFlags, P1, P2 );
		P1 = P2;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Box drawing.
-----------------------------------------------------------------------------*/

//
// Draw a box centered about a location.
//
void URender::DrawBox
(
	FSceneNode*		Frame,
	FPlane			Color,
	DWORD			LineFlags,
	FVector			Min,
	FVector			Max
)
{
	guard(URender::DrawBox);

	FVector A,B;
	FVector Location = Min+Max;
	if	   ( Frame->Viewport->Actor->RendMap==REN_OrthXY )	{A=FVector(Max.X-Min.X,0,0); B=FVector(0,Max.Y-Min.Y,0);}
	else if( Frame->Viewport->Actor->RendMap==REN_OrthXZ )	{A=FVector(Max.X-Min.X,0,0); B=FVector(0,0,Max.Z-Min.Z);}
	else													{A=FVector(0,Max.Y-Min.Y,0); B=FVector(0,0,Max.Z-Min.Z);}

	Draw3DLine( Frame, Color, LineFlags, (Location+A+B)/2, (Location+A-B)/2 );
	Draw3DLine( Frame, Color, LineFlags, (Location-A+B)/2, (Location-A-B)/2 );
	Draw3DLine( Frame, Color, LineFlags, (Location+A+B)/2, (Location-A+B)/2 );
	Draw3DLine( Frame, Color, LineFlags, (Location+A-B)/2, (Location-A-B)/2 );

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
