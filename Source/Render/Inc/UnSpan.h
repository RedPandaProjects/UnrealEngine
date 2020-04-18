/*=============================================================================
	UnSpan.h: Span buffering functions and structures
	Copyright 1995 Epic MegaGames, Inc.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*------------------------------------------------------------------------------------
	General span buffer related classes.
------------------------------------------------------------------------------------*/

//
// A span buffer linked-list entry representing a free (undrawn) 
// portion of a scanline. 
//
class FSpan
{
public:
	// Variables.
	INT Start, End;
	FSpan* Next;

	// Constructors.
	FSpan()
	{}
	FSpan( INT InStart, INT InEnd )
	:	Start		(InStart)
	,	End			(InEnd)
	{}
};

//
// A raster span.
//
struct FRasterSpan
{
	INT X[2];
};

//
// A raster polygon.
//
class FRasterPoly
{
public:
	INT	StartY;
	INT EndY;
	FRasterSpan Lines[];
};

//
// A span buffer, which represents all free (undrawn) scanlines on
// the screen.
//
class RENDER_API FSpanBuffer
{
public:
	INT			StartY;		// Starting Y value.
	INT			EndY;		// Last Y value + 1.
	INT			ValidLines;	// Number of lines at beginning (for screen).
	FSpan**		Index;		// Contains (EndY-StartY) units pointing to first span or NULL.
	FMemStack*	Mem;		// Memory pool everything is stored in.
	FMemMark	Mark;		// Top of memory pool marker.

	// Constructors.
	FSpanBuffer()
	{}
	FSpanBuffer( const FSpanBuffer& Source, FMemStack& InMem )
	:	StartY		(Source.StartY)
	,	EndY		(Source.EndY)
	,	ValidLines	(Source.ValidLines)
	,	Index		(new(InMem,EndY-StartY)FSpan*)
	,	Mem			(&InMem)
	,	Mark		(InMem)
	{
		for( int i=0; i<EndY-StartY; i++ )
		{
			FSpan** PrevLink = &Index[i];
			for( FSpan* Other=Source.Index[i]; Other; Other=Other->Next )
			{
				*PrevLink = new( *Mem, 1, 4 )FSpan( Other->Start, Other->End );
				PrevLink  = &(*PrevLink)->Next;
			}
			*PrevLink = NULL;
		}
	}

	// Allocation.
	void AllocIndex( INT AllocStartY, INT AllocEndY, FMemStack* Mem );
	void AllocIndexForScreen( INT SXR, INT SYR, FMemStack* Mem );
	void Release();
	void GetValidRange( SWORD* ValidStartY, SWORD* ValidEndY );

	// Merge/copy/alter operations.
	void CopyIndexFrom( const FSpanBuffer& Source, FMemStack* Mem );
	void MergeWith( const FSpanBuffer& Other );

	// Grabbing and updating from rasterizations.
	INT CopyFromRaster( FSpanBuffer& ScreenSpanBuffer, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster );
	INT CopyFromRasterUpdate( FSpanBuffer& ScreenSpanBuffer, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster );

	// Occlusion.
	INT BoxIsVisible( INT X1, INT Y1, INT X2, INT Y2 );

	// Debugging.
	void AssertEmpty( char* Name );
	void AssertNotEmpty( char* Name );
	void AssertValid( char* Name );
	void AssertGoodEnough( char* Name );
};

/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
