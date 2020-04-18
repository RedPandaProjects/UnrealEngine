/*=============================================================================
	UnCanvas.cpp: Unreal canvas rendering.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	UCanvas scaled sprites.
-----------------------------------------------------------------------------*/

void UCanvas::DrawTile
(
	UTexture*		Texture,
	FLOAT			X,
	FLOAT			Y,
	FLOAT			XL,
	FLOAT			YL,
	FLOAT			U,
	FLOAT			V,
	FLOAT			UL,
	FLOAT			VL,
	FSpanBuffer*	SpanBuffer,
	FLOAT			Z,
	FPlane			Color,
	FPlane			Fog,
	DWORD			PolyFlags
)
{
	guard(UCanvas::DrawTile);
	check(Texture);

	// Compute clipping region.
	FLOAT ClipY0 = /*SpanBuffer ? SpanBuffer->StartY :*/ 0;
	FLOAT ClipY1 = /*SpanBuffer ? SpanBuffer->EndY   :*/ Frame->FY;

	// Reject.
	if( XL<=0.f || YL<=0.f || X+XL<=0.f || Y+YL<=ClipY0 || X>=Frame->FX || Y>=ClipY1 )
		return;

	// Clip.
	if( X<0.f )
		{FLOAT C=X*UL/XL; U-=C; UL+=C; XL+=X; X=0.f;}
	if( Y<0.f )
		{FLOAT C=Y*VL/YL; V-=C; VL+=C; YL+=Y; Y=0.f;}
	if( XL>Frame->FX-X )
		{UL+=(Frame->FX-X-XL)*UL/XL; XL=Frame->FX-X;}
	if( YL>Frame->FY-Y )
		{VL+=(Frame->FY-Y-YL)*VL/YL; YL=Frame->FY-Y;}

	// Draw it.
	FTextureInfo Info;
	Texture->GetInfo( Info, Viewport->CurrentTime );
	U *= Info.UScale; UL *= Info.UScale;
	V *= Info.VScale; VL *= Info.VScale;
	Viewport->RenDev->DrawTile( Frame, Info, X, Y, XL, YL, U, V, UL, VL, SpanBuffer, Z, Color, Fog, PolyFlags );

	unguard;
}

void UCanvas::DrawPattern
(
	UTexture*		Texture,
	FLOAT			X,
	FLOAT			Y,
	FLOAT			XL,
	FLOAT			YL,
	FLOAT			Scale,
	FLOAT			OrgX,
	FLOAT			OrgY,
	FSpanBuffer*	SpanBuffer,
	FLOAT			Z,
	FPlane			Color,
	FPlane			Fog,
	DWORD			PolyFlags
)
{
	guard(UCanvas::DrawPattern);
	DrawTile( Texture, X, Y, XL, YL, (X-OrgX)*Scale + Texture->USize, (Y-OrgY)*Scale + Texture->VSize, XL*Scale, YL*Scale, SpanBuffer, Z, Color, Fog, PolyFlags );
	unguard;
}

//
// Draw a scaled sprite.  Takes care of clipping.
// XSize and YSize are in pixels.
//
void UCanvas::DrawIcon
(
	UTexture*			Texture,
	FLOAT				ScreenX, 
	FLOAT				ScreenY, 
	FLOAT				XSize, 
	FLOAT				YSize, 
	FSpanBuffer*		SpanBuffer,
	FLOAT				Z,
	FPlane				Color,
	FPlane				Fog,
	DWORD				PolyFlags
)
{
	guard(UCanvas::DrawIcon);
	DrawTile( Texture, ScreenX, ScreenY, XSize, YSize, 0, 0, Texture->USize, Texture->VSize, SpanBuffer, Z, Color, Fog, PolyFlags );
	unguard;
}

/*-----------------------------------------------------------------------------
	UCanvas text drawing.
-----------------------------------------------------------------------------*/

//
// Calculate the length of a string built from a font, starting at a specified
// position and counting up to the specified number of characters (-1 = infinite).
//
void UCanvas::StrLen
(
	UFont*		Font,
	INT&		XL, 
	INT&		YL, 
	const char*	Text,
	INT			iStart,
	INT			NumChars
)
{
	guard(UCanvas::StrLen);

	XL = YL = 0;
	for( const char* c=Text+iStart; *c && NumChars>0; c++,NumChars-- )
	{
		if( *c < Font->Characters.Num() )
		{
			XL += Font->Characters(*c).USize + SpaceX;
			YL = ::Max(YL,Font->Characters(*c).VSize);
		}
	}
	YL += SpaceY;

	unguard;
}

//
// Calculate the size of a string built from a font, word wrapped
// to a specified region.
//
void UCanvas::WrappedStrLen
(
	UFont*		Font,
	INT&		XL, 
	INT&		YL, 
	INT			MaxWidth, 
	const char*	Text
)
{
	guard(UCanvas::WrappedStrLen);
	check(Font);

	int iLine=0;
	int TestXL,TestYL;
	XL = YL = 0;

	// Process each output line.
	while( Text[iLine] )
	{
		// Process each word until the current line overflows.
		int iWord, iTestWord=iLine;
		do
		{
			iWord = iTestWord;
			if( !Text[iTestWord] )
				break;
			while( Text[iTestWord] && Text[iTestWord]!=' ' )
				iTestWord++;
			while( Text[iTestWord]==' ' )
				iTestWord++;
			StrLen( Font, TestXL, TestYL, Text, iLine, iTestWord-iLine );
		} while( TestXL <= MaxWidth );
		
		if( iWord == iLine )
		{
			// The text didn't fit word-wrapped onto this line, so chop it.
			int iTestWord = iLine;
			do
			{
				iWord = iTestWord;
				if( !Text[iTestWord] )
					break;
				iTestWord++;
				StrLen( Font, TestXL, TestYL, Text, iLine, iTestWord-iLine );
			} while( TestXL <= MaxWidth );
			
			// Word wrap failed because window is too small to hold a single character.
			if( iWord == iLine )
				return;
		}

		// Sucessfully split this line.
		StrLen( Font, TestXL, TestYL, Text, iLine, iWord-iLine );
		check(TestXL<=MaxWidth);
		YL += TestYL;
		if( TestXL > XL )
			XL = TestXL;

		// Go to the next line.
		while( Text[iWord]==' ' )
			iWord++;
		
		iLine = iWord;
	}
	unguard;
}

//
// Font printing.
//
static inline void DrawChar( UCanvas* Canvas, FTextureInfo& Info, INT X, INT Y, INT XL, INT YL, INT U, INT V, INT UL, INT VL, FPlane Color )
{
	// Reject.
	FSceneNode* Frame=Canvas->Frame;
	if( X+XL<=0.0 || Y+YL<=0 || X>=Frame->FX || Y>=Frame->FY )
		return;

	// Clip.
	if( X<0.f )
		{FLOAT C=X*UL/XL; U-=C; UL+=C; XL+=X; X=0.f;}
	if( Y<0.f )
		{FLOAT C=Y*VL/YL; V-=C; VL+=C; YL+=Y; Y=0.f;}
	if( XL>Frame->FX-X )
		{UL+=(Frame->FX-X-XL)*UL/XL; XL=Frame->FX-X;}
	if( YL>Frame->FY-Y )
		{VL+=(Frame->FY-Y-YL)*VL/YL; YL=Frame->FY-Y;}

	// Draw.
	Frame->Viewport->RenDev->DrawTile( Frame, Info, X, Y, UL, VL, U, V, UL, VL, NULL, Canvas->Z, Color, FPlane(0,0,0,0), PF_NoSmooth | PF_Masked | PF_RenderHint );
}
void VARARGS UCanvas::Printf
(
	UFont*		Font,
	INT			X,
	INT			Y,
	const char* Fmt,
	...
)
{
	char Text[4096];
	GET_VARARGS( Text, Fmt );

	guard(UCanvas::Printf);
	check(Font);

	FTextureInfo Info;
	Font->GetInfo( Info, Viewport->CurrentTime );
	FPlane DrawColor = Color.Plane();
	for( BYTE* c=(BYTE*)&Text[0]; *c; c++ )
	{
		//const char* C=LocalizeGeneral("Copyright","Core");
		//while( *C ) 
		//	debugf("%i",*C);
		FFontCharacter& Char = Font->Characters( *c );
		DrawChar( this, Info, OrgX+X, OrgY+Y, Char.USize, Char.VSize, Char.StartU, Char.StartV, Char.USize, Char.VSize, DrawColor );
		X += Char.USize + SpaceX;
	}
	unguard;
}

//
// Wrapped printf.
//
void VARARGS UCanvas::WrappedPrintf( UFont* Font, UBOOL Center, const char* Fmt, ... )
{
	char Text[4096];
	GET_VARARGS(Text,Fmt);

	guard(UCanvas::WrappedPrintf);
	check(Font);

	int iLine=0;
	int TestXL, TestYL;

	// Process each output line.
	while( Text[iLine] )
	{
		// Process each word until the current line overflows.
		int iWord, iTestWord=iLine;
		do
		{
			iWord = iTestWord;
			if( !Text[iTestWord] )
				break;
			while( Text[iTestWord] && Text[iTestWord]!=' ' ) 
				iTestWord++;
			while( Text[iTestWord]==' ' )
				iTestWord++;
			StrLen( Font, TestXL, TestYL, Text, iLine, iTestWord-iLine );
		} while( TestXL <= ClipX );

		// If the text didn't fit word-wrapped onto this line, chop it.
		if( iWord==iLine )
		{
			int iTestWord = iLine;
			do
			{
				iWord = iTestWord;
				if( !Text[iTestWord] )
					break;
				iTestWord++;
				StrLen( Font, TestXL, TestYL, Text, iLine, iTestWord-iLine );
			} while( TestXL <= ClipX );
			if( iWord==iLine ) 
			{
				// Word wrap failed.
				return;
			}
		}

		// Sucessfully split this line, now draw it.
		char Temp[256];
		appStrcpy( Temp, &Text[iLine] );
		Temp[iWord-iLine]=0;
		StrLen( Font, TestXL, TestYL, Text, iLine, iWord-iLine );
		check(TestXL<=ClipX);
		Printf( Font, Center ? CurX+(ClipX-TestXL)/2 : CurX, CurY, "%s", Temp );
		CurY += TestYL;

		// Go to the next line.
		while( Text[iWord]==' ' )
			iWord++;
		iLine = iWord;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	UCanvas object functions.
-----------------------------------------------------------------------------*/

void UCanvas::Init( UViewport* InViewport )
{
	guard(UCanvas::UCanvas);
	Viewport = InViewport;
	unguard;
}
void UCanvas::Update( FSceneNode* InFrame )
{
	guard(UCanvas::Update);

	// Call UnrealScript to reset.
	ProcessEvent( FindFunctionChecked("Reset"), NULL );

	// Copy size parameters from viewport.
	Frame = InFrame;
	X = ClipX = Frame->X;
	Y = ClipY = Frame->Y;

	unguard;
}

/*-----------------------------------------------------------------------------
	UCanvas intrinsics.
-----------------------------------------------------------------------------*/

void UCanvas::execDrawText( FFrame& Stack, BYTE*& Result )
{
	guard(UCanvas::execDrawText);
	P_GET_STRING(Text);
	P_GET_UBOOL_OPT(CR,1);
	P_FINISH;
	if( !Font )
	{
		Stack.ScriptWarn( 0, "DrawText: No font" );
		return;
	}

	//debugf( "DrawText: '%s' %i", Text, CR );
	if( Style!=STY_None )
		WrappedPrintf( Font, bCenter, "%s", Text );
	INT XL, YL;
	WrappedStrLen( Font, XL, YL, ClipX, Text );
	CurX += XL;
	CurYL = Max(CurYL,(FLOAT)YL);
	if( CR )
	{
		CurX  = 0;
		CurY += CurYL;
		CurYL = 0;
	}

	unguardexec;
}
AUTOREGISTER_INTRINSIC( UCanvas, 465, execDrawText );

void UCanvas::execDrawTile( FFrame& Stack, BYTE*& Result )
{
	guard(UCanvas::execDrawTile);
	P_GET_OBJECT(UTexture,Tex);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;
	if( !Tex )
	{
		Stack.ScriptWarn( 0, "DrawTile: Missing Texture" );
		return;
	}
	//debugf( "DrawTile: %s %f %f %f %f %f %f", Tex->GetPathName(), XL, YL, U0, V0, U1, V1 );
	if( Style!=STY_None ) DrawTile
	(
		Tex,
		OrgX+CurX,
		OrgX+CurY,
		XL,
		YL,
		U,
		V,
		UL,
		VL,
		NULL,
		Z,
		Color.Plane(),
		FPlane(0,0,0,0),
		PF_TwoSided | (Style==STY_Masked ? PF_Masked : Style==STY_Translucent ? PF_Translucent : Style==STY_Modulated ? PF_Modulated : 0) | (bNoSmooth ? PF_NoSmooth : 0)
	);
	CurX += XL + SpaceX;
	CurYL = Max(CurYL,YL);
	unguardexec;
}
AUTOREGISTER_INTRINSIC( UCanvas, 466, execDrawTile );

IMPLEMENT_CLASS(UCanvas);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
