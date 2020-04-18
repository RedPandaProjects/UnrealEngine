/*=============================================================================
    UnSpan.cpp: Unreal span buffering functions
    Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#include "RenderPrivate.h"

#define UPDATE_PREVLINK(START,END)\
{\
	TopSpan         = New<FSpan>(*Mem,1,4);\
    *PrevLink       = TopSpan;\
    TopSpan->Start  = START;\
    TopSpan->End    = END;\
    PrevLink        = &TopSpan->Next;\
    ValidLines++;\
};

#define UPDATE_PREVLINK_ALLOC(START,END)\
{\
    NewSpan         = New<FSpan>(*Mem,1,4);\
    *PrevLink       = NewSpan;\
    NewSpan->Start  = START;\
    NewSpan->End    = END;\
    PrevLink        = &(NewSpan->Next);\
    ValidLines++;\
};

/*-----------------------------------------------------------------------------
    Allocation.
-----------------------------------------------------------------------------*/

//
// Allocate a linear span buffer in temporary memory.  Allocates zero bytes
// for the list; must call spanAllocLinear to allocate the proper amount of memory
// for it.
//
void FSpanBuffer::AllocIndex( int AllocStartY, int AllocEndY, FMemStack* MemStack )
{
    guard(FSpanBuffer::AllocIndex);

    Mem         = MemStack;
    StartY      = AllocStartY;
    EndY        = AllocEndY;
    ValidLines  = 0;

    if( StartY <= EndY )
        Index = New<FSpan*>(*Mem,AllocEndY-AllocStartY);
    else
        Index = NULL;

	Mark = FMemMark(*MemStack);
    unguardf(("(%i-%i)", AllocStartY, AllocEndY));
}

//
// Allocate a linear span buffer and initialize it to represent
// the yet-undrawn region of a viewport.
//
void FSpanBuffer::AllocIndexForScreen( INT SXR, INT SYR, FMemStack* MemStack )
{
    guard(FSpanBuffer::AllocIndexForScreen);
    int  i;

    Mem     = MemStack;
    StartY  = 0;
    EndY    = ValidLines = SYR;

    Index       = New<FSpan*>(*Mem,SYR,4);
    FSpan *List = New<FSpan>(*Mem,SYR,4);
    for( i=0; i<SYR; i++ )
    {
        Index[i]        = &List[i];
        List [i].Start  = 0;
        List [i].End    = SXR;
        List [i].Next   = NULL;
    }
    unguard;
}

//
// Free a linear span buffer in temporary rendering pool memory.
// Works whether actually saved or not.
//
void FSpanBuffer::Release()
{
    guard(FSpanBuffer::Release);
    Mark.Pop();
    unguard;
}

//
// Compute's a span buffer's valid range StartY-EndY range.
// Sets to 0,0 if the span is entirely empty.  You can also detect
// this condition by comparing ValidLines to 0.
//
void FSpanBuffer::GetValidRange( SWORD* ValidStartY, SWORD* ValidEndY )
{
    if( ValidLines )
    {
        FSpan **TempIndex;
        int NewStartY,NewEndY;

        NewStartY = StartY;
        TempIndex = &Index [0];
        while( *TempIndex==NULL )
		{
			TempIndex++;
			NewStartY++;
		}

        NewEndY   = EndY;
        TempIndex = &Index [EndY-StartY-1];
        while( *TempIndex==NULL )
		{
			TempIndex--;
			NewEndY--;
		}

        *ValidStartY = NewStartY;
        *ValidEndY   = NewEndY;
	}
    else *ValidStartY = *ValidEndY = 0;
}

/*-----------------------------------------------------------------------------
    Span occlusion.
-----------------------------------------------------------------------------*/

//
// See if a rectangle is visible.  Returns 1 if all or partially visible,
// 0 if totally occluded.
//
// Status: Performance critical.
//
INT FSpanBuffer::BoxIsVisible( INT X1, INT Y1, INT X2, INT Y2 )
{
    guard(FSpanBuffer::BoxIsVisible);

    FSpan **ScreenIndex, *Span;
    if( Y1 >= EndY )
    {
        return 0;
    }
    if( Y2 <= StartY )
    {
        return 0;
    }
    if (Y1 < StartY)    Y1 = StartY;
    if (Y2 > EndY)      Y2 = EndY;

    // Check box occlusion with span buffer.
    ScreenIndex = &Index [Y1-StartY];
    int Count   = Y2-Y1;

    // Start checking last line, then first and the rest.
    Span = *(ScreenIndex + Count - 1 );
    while( --Count >= 0 )
    {
        while( Span && X2>Span->Start )
		{
    		if( X1 < Span->End )
    		{
    			return 1;
    		}
    		Span = Span->Next;
        }
        Span = *ScreenIndex++;
    }
    return 0;
    unguard;
}

/*-----------------------------------------------------------------------------
    Span grabbing and updating.
-----------------------------------------------------------------------------*/

//
// Grind this polygon through the span buffer and:
// - See if the poly is totally occluded.
// - Update the span buffer by adding this poly to it.
// - Build a new, temporary span buffer for raster and span clipping the poly.
//
// Returns 1 if poly is all or partially visible, 0 if completely obscured.
// If 0 was returned, no screen span buffer memory was allocated and the resulting
// span index can be safely freed.
//
// Requires that StartY <= Raster.StartY, EndY >= Raster.EndY;
//
// If the destination FSpanBuffer and the screen's FSpanBuffer are using the same memory
// pool, the newly-allocated screen spans will be intermixed with the destination
// screen spans.  Freeing the destination in this case will overwrite the screen span buffer
// with garbage.
//
// Status: Extremely performance critical.
//
INT FSpanBuffer::CopyFromRasterUpdate( FSpanBuffer& Screen, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster )
{
    guard(FSpanBuffer::CopyFromRasterUpdate);

    FRasterSpan *Line;
    FSpan       **ScreenIndex,*NewScreenSpan,*NewSpan,*ScreenSpan,**PrevScreenLink;
    FSpan       **TempIndex,**PrevLink;
	int			i,OurStart,OurEnd,Accept=0;

    if( StartY>RasterStartY || EndY<RasterEndY )
	{
		debugf( NAME_Warning, "Illegal span range <%i,%i> <%i,%i>", StartY, EndY, RasterStartY, RasterEndY );
		return 0;
       //appErrorf( "Illegal span range <%i,%i> <%i,%i>", StartY, EndY, RasterStartY, RasterEndY );
	}

    OurStart  = Max( RasterStartY, Screen.StartY );
    OurEnd    = Min( RasterEndY,   Screen.EndY   );
 	TempIndex = &Index[ 0 ];

    // Extra check for OurStart>OurEnd = screen and rasterpoly don't overlap, so all-null output.
    if( OurStart>=OurEnd )
    {
    	for( i=StartY; i<EndY; i++ )
			*(TempIndex++) = NULL;
        return 0;
    }

    for( i=StartY; i<OurStart; i++ )
		*(TempIndex++) = NULL;

    Line        = Raster + OurStart - RasterStartY;
    ScreenIndex = Screen.Index + OurStart - Screen.StartY;

	for( i=OurStart; i<OurEnd; i++ )
    {
        PrevScreenLink  = ScreenIndex;
        ScreenSpan      = *(ScreenIndex++);
        PrevLink        = TempIndex++;

        // Skip if this screen span is already full, or if the raster is empty.
        if( (!ScreenSpan) || (Line->X[1] <= Line->X[0]) )
			goto NextLine;

        // Skip past all spans that occur before the raster.
        while( ScreenSpan->End <= Line->X[0] )
        {
            PrevScreenLink  = &(ScreenSpan->Next);
            ScreenSpan      = ScreenSpan->Next;
            if( ScreenSpan == NULL )
				goto NextLine; // This line is full.
        }

        // ASSERT: ScreenSpan->End.X > Line->Start.X.

        // See if this span straddles the raster's starting point.
        if( ScreenSpan->Start < Line->X[0] )
        {
            // Add partial chunk to span buffer.
            Accept = 1;
            UPDATE_PREVLINK_ALLOC(Line->X[0],Min(Line->X[1], ScreenSpan->End));

            // See if span entirely encloses raster; if so, break span
            // up into two pieces and we're done.
            if( ScreenSpan->End > Line->X[1] )
            {
                // Get memory for the new span.  Note that this may be drawing from
                // the same memory pool as the destination.
                NewScreenSpan        = New<FSpan>(*Screen.Mem,1,4);
                NewScreenSpan->Start = Line->X[1];
                NewScreenSpan->End   = ScreenSpan->End;
                NewScreenSpan->Next  = ScreenSpan->Next;

                ScreenSpan->Next     = NewScreenSpan;
                ScreenSpan->End      = Line->X[0];

                Screen.ValidLines++;

                goto NextLine; // Done (everything is clean).
            }
            else
            {
                // Remove partial chunk from the span buffer.
                ScreenSpan->End = Line->X[0];

                PrevScreenLink  = &(ScreenSpan->Next);
                ScreenSpan      = ScreenSpan->Next;
                if (ScreenSpan == NULL) goto NextLine; // Done (everything is clean).
            }
        }

        // ASSERT: Span->Start >= Line->Start.X
        // if (ScreenSpan->Start < Line->Start.X) appError ("Span2");

        // Process all screen spans that are entirely within the raster.
        while( ScreenSpan->End <= Line->X[1] )
        {
            // Add entire chunk to temporary span buffer.
            Accept = 1;
            UPDATE_PREVLINK_ALLOC(ScreenSpan->Start,ScreenSpan->End);

            // Delete this span from the span buffer.
            *PrevScreenLink = ScreenSpan->Next;
            ScreenSpan      = ScreenSpan->Next;
            Screen.ValidLines--;
            if( ScreenSpan==NULL )
				goto NextLine; // Done (everything is clean).
        }

        // ASSERT: Span->End > Line->End.X
        // if (ScreenSpan->End <= Line->End.X) appError ("Span3");

        // If span overlaps raster's end point, process the partial chunk:
        if( ScreenSpan->Start < Line->X[1] )
        {
            // Add chunk from Span->Start to Line->End.X to temp span buffer.
            Accept = 1;
            UPDATE_PREVLINK_ALLOC(ScreenSpan->Start,Line->X[1]);

            // Shorten this span line by removing the raster.
            ScreenSpan->Start = Line->X[1];
        }
        NextLine:
        *PrevLink = NULL;
        Line ++;
    }
    for( i=OurEnd; i<EndY; i++ )
		*(TempIndex++) = NULL;

#if CHECK_ALL
    AssertGoodEnough("CopyFromRasterUpdate - Output spanbuffer");
    Screen.AssertValid("CopyFromRasterUpdate - Screen spanbuffer");
#endif
    return Accept;
    unguard;
}

//
// Grind this polygon through the span buffer and:
// - See if the poly is totally occluded
// - Build a new, temporary span buffer for raster and span clipping the poly
//
// Doesn't affect the span buffer no matter what.
// Returns 1 if poly is all or partially visible, 0 if completely obscured.
//
INT FSpanBuffer::CopyFromRaster( FSpanBuffer& Screen, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster )
{
    guard(FSpanBuffer::CopyFromRaster);

    FRasterSpan *Line;
    FSpan       **ScreenIndex,*ScreenSpan;
    FSpan       **TempIndex,**PrevLink,*NewSpan;
	int			i,OurStart,OurEnd,Accept=0;

    OurStart = Max(RasterStartY,Screen.StartY);
    OurEnd   = Min(RasterEndY,Screen.EndY);

 	TempIndex = &Index [0];

    // Extra check for OurStart>OurEnd = screen and rasterpoly don't overlap, so all-null output.
    if( OurStart>=OurEnd )
    {
    	for( i=StartY; i<EndY; i++ )
			*(TempIndex++) = NULL;
        return 0;
    }

    for( i=StartY; i<OurStart; i++ )
		*(TempIndex++) = NULL;

    Line        = Raster + OurStart - RasterStartY;
    ScreenIndex = Screen.Index + OurStart - Screen.StartY;

    for( i=OurStart; i<OurEnd; i++ )
    {
        ScreenSpan      = *(ScreenIndex++);
        PrevLink        = TempIndex++;

        if( !ScreenSpan || Line->X[1] <= Line->X[0] )
			// This span is already full, or raster is empty.
			goto NextLine; 

        // Skip past all spans that occur before the raster.
        while( ScreenSpan->End <= Line->X[0] )
        {
            ScreenSpan = ScreenSpan->Next;
            if( !ScreenSpan )
				// This line is full.
				goto NextLine;
        }

        debug(ScreenSpan->End > Line->X[0]);

        // See if this span straddles the raster's starting point.
        if( ScreenSpan->Start < Line->X[0] )
        {
            Accept = 1;

            // Add partial chunk to temporary span buffer.
            UPDATE_PREVLINK_ALLOC(Line->X[0],Min(Line->X[1], ScreenSpan->End));
            ScreenSpan = ScreenSpan->Next;
            if( !ScreenSpan )
				goto NextLine;
        }

        debug(ScreenSpan->Start >= Line->X[0]);

        // Process all spans that are entirely within the raster.
        while( ScreenSpan->End <= Line->X[1] )
        {
            Accept = 1;

            // Add entire chunk to temporary span buffer.
            UPDATE_PREVLINK_ALLOC(ScreenSpan->Start,ScreenSpan->End);
            ScreenSpan = ScreenSpan->Next;
            if( !ScreenSpan )
				goto NextLine;
        }

        debug(ScreenSpan->End > Line->X[1]);

        // If span overlaps raster's end point, process the partial chunk.
        if( ScreenSpan->Start < Line->X[1] )
        {
            // Add chunk from Span->Start to Line->End.X to temp span buffer.
            Accept = 1;
            UPDATE_PREVLINK_ALLOC(ScreenSpan->Start,Line->X[1]);
        }
        NextLine:
        *PrevLink = NULL;
        Line++;
    }
    for( i=OurEnd; i<EndY; i++ )
		*(TempIndex++) = NULL;

#if CHECK_ALL
    AssertGoodEnough("CopyFromRaster");
#endif
    return Accept;
    unguard;
}

/*-----------------------------------------------------------------------------
    Merging.
-----------------------------------------------------------------------------*/

//
// Macro for copying a span.
//
#define COPY_SPAN(SRC_INDEX)\
{\
    PrevLink         = DestIndex++;\
    Span             = *(SRC_INDEX++);\
    while( Span )\
    {\
        UPDATE_PREVLINK(Span->Start,Span->End);\
        Span = Span->Next;\
    }\
    *PrevLink = NULL;\
}

//
// Merge this existing span buffer with another span buffer.  Overwrites the appropriate
// parts of this span buffer.  If this span buffer's index isn't large enough
// to hold everything, reallocates the index.
//
// This is meant to be called with this span buffer using GDynMem and the other span
// buffer using GMem.
//
// Status: This is currently unused and doesn't need to be optimized.
//
void FSpanBuffer::MergeWith( const FSpanBuffer& Other )
{
    guard(FSpanBuffer::MergeWith);

    // See if the existing span's index is large enough to hold the merged result.
    if( Other.StartY<StartY || Other.EndY>EndY )
    {
		// Must reallocate and copy index.
        int NewStartY = Min(StartY,Other.StartY);
        int NewEndY   = Max(EndY,  Other.EndY);
        int NewNum    = NewEndY - NewStartY;
        FSpan **NewIndex = New<FSpan*>(*Mem,NewNum);

        appMemset(&NewIndex[0                    ],0,    (StartY-NewStartY)*sizeof(FSpan *));
        appMemcpy(&NewIndex[StartY-NewStartY     ],Index,(EndY     -StartY)*sizeof(FSpan *));
        appMemset(&NewIndex[NewNum-(NewEndY-EndY)],0,    (NewEndY  -EndY  )*sizeof(FSpan *));

        StartY = NewStartY;
        EndY   = NewEndY;
        Index  = NewIndex;
    }

    // Now merge other span into this one.
    FSpan **ThisIndex  = &Index       [Other.StartY - StartY];
    FSpan **OtherIndex = &Other.Index [0];
    FSpan *ThisSpan,*OtherSpan,*TempSpan,**PrevLink;
    for( int i=Other.StartY; i<Other.EndY; i++ )
    {
        PrevLink    = ThisIndex;
        ThisSpan    = *(ThisIndex++);
        OtherSpan   = *(OtherIndex++);

        // Do everything relative to ThisSpan.
        while( ThisSpan && OtherSpan )
        {
            if( OtherSpan->End < ThisSpan->Start )
            {
				// Link OtherSpan in completely before ThisSpan.
                *PrevLink = TempSpan= New<FSpan>(*Mem,1,4);
                TempSpan->Start     = OtherSpan->Start;
                TempSpan->End       = OtherSpan->End;
                TempSpan->Next      = ThisSpan;
                PrevLink            = &TempSpan->Next;

                OtherSpan           = OtherSpan->Next;

                ValidLines++;
            }
            else if (OtherSpan->Start <= ThisSpan->End)
            {
				// Merge OtherSpan into ThisSpan.
                *PrevLink           = ThisSpan;
                ThisSpan->Start     = Min(ThisSpan->Start,OtherSpan->Start);
                ThisSpan->End       = Max(ThisSpan->End,  OtherSpan->End);
                TempSpan            = ThisSpan; // For maintaining End and Next.

                PrevLink            = &ThisSpan->Next;
                ThisSpan            = ThisSpan->Next;
                OtherSpan           = OtherSpan->Next;

                for(;;)
                {
                    if( ThisSpan&&(ThisSpan->Start <= TempSpan->End) )
                    {
                        TempSpan->End = Max(ThisSpan->End,TempSpan->End);
                        ThisSpan      = ThisSpan->Next;
                        ValidLines--;
                    }
                    else if( OtherSpan&&(OtherSpan->Start <= TempSpan->End) )
                    {
                        TempSpan->End = Max(TempSpan->End,OtherSpan->End);
                        OtherSpan     = OtherSpan->Next;
                    }
                    else break;
                }
            }
            else
            {
				// This span is entirely before the other span; keep it.
                *PrevLink           = ThisSpan;
                PrevLink            = &ThisSpan->Next;
                ThisSpan            = ThisSpan->Next;
            }
        }
        while( OtherSpan )
        {
			// Just append spans from OtherSpan.
            *PrevLink = TempSpan    = New<FSpan>(*Mem,1,4);
            TempSpan->Start         = OtherSpan->Start;
            TempSpan->End           = OtherSpan->End;
            PrevLink                = &TempSpan->Next;

            OtherSpan               = OtherSpan->Next;

            ValidLines++;
        }
        *PrevLink = ThisSpan;
    }

#if CHECK_ALL
    AssertGoodEnough("MergeWith");
#endif

    unguard;
}

/*-----------------------------------------------------------------------------
    Duplicating.
-----------------------------------------------------------------------------*/

//
// Copy the index from one span buffer to another span buffer.
//
// Status: Seldom called, no need to optimize.
//
void FSpanBuffer::CopyIndexFrom( const FSpanBuffer& Source, FMemStack* Mem )
{
    guard(FSpanBuffer::CopyIndexFrom);

    StartY   = Source.StartY;
    EndY     = Source.EndY;
 
    Index = New<FSpan*>(*Mem,Source.EndY-Source.StartY);
    appMemcpy( &Index[0], &Source.Index[0], (Source.EndY-Source.StartY) * sizeof(FSpan *) );

	unguard;
}

/*-----------------------------------------------------------------------------
    Debugging.
-----------------------------------------------------------------------------*/

//
// These debugging functions are available while writing span buffer code.
// They perform various checks to make sure that span buffers don't become
// corrupted.  They don't need optimizing, of course.
//

//
// Make sure that a span buffer is completely empty.
//
void FSpanBuffer::AssertEmpty( char* Name )
{
    guard(FSpanBuffer::AssertEmpty);
    FSpan **TempIndex,*Span;
    int i;

    TempIndex = Index;
    for( i=StartY; i<EndY; i++ )
    {
        Span = *(TempIndex++);
        while (Span!=NULL)
        {
            appErrorf("%s not empty, line=%i<%i>%i, start=%i, end=%i",Name,StartY,i,EndY,Span->Start,Span->End);
            Span=Span->Next;
        }
    }
    unguard;
}

//
// Assure that a span buffer isn't empty.
//
void FSpanBuffer::AssertNotEmpty( char* Name )
{
    guard(FSpanBuffer::AssertNotEmpty);
    FSpan **TempIndex,*Span;
    int i,NotEmpty=0;

    TempIndex = Index;
    for( i=StartY; i<EndY; i++ )
    {
        Span = *(TempIndex++);
        while (Span!=NULL)
        {
            if( Span->Start>=Span->End )
				appErrorf("%s contains %i-length span",Name,Span->End-Span->Start);
            NotEmpty=1;
            Span=Span->Next;
        }
    }
    if( !NotEmpty )
		appErrorf ("%s is empty",Name);
    unguard;
}

//
// Make sure that a span buffer is valid.  Performs the following checks:
// - Make sure there are no zero-length spans
// - Make sure there are no negative-length spans
// - Make sure there are no overlapping spans
// - Make sure all span pointers are valid (otherwise GPF's)
//
void FSpanBuffer::AssertValid( char* Name )
{
    guard(FSpanBuffer::AssertValid);

    FSpan **TempIndex,*Span;
    int i,PrevEnd,c=0;

    TempIndex = Index;
    for( i=StartY; i<EndY; i++ )
    {
        PrevEnd = -1000;
        Span = *(TempIndex++);
        while( Span )
        {
            if ((i==StartY)||(i==(EndY-1)))
            {
                if ((PrevEnd!=-1000) && (PrevEnd >= Span->Start)) appErrorf("%s contains %i-length overlap, line %i/%i",Name,PrevEnd-Span->Start,i-StartY,EndY-StartY);
                if (Span->Start>=Span->End) appErrorf("%s contains %i-length span, line %i/%i",Name,Span->End-Span->Start,i-StartY,EndY-StartY);
                PrevEnd = Span->End;
            }
            Span=Span->Next;
            c++;
        }
    }
    if( c!=ValidLines )
		appErrorf ("%s bad ValidLines: claimed=%i, correct=%i",Name,ValidLines,c);
    unguard;
}

//
// Like AssertValid, but 'ValidLines' checked for zero/nonzero only.
//
void FSpanBuffer::AssertGoodEnough( char* Name )
{
    guard(FSpanBuffer::AssertGoodEnough);
    FSpan **TempIndex,*Span;
    int i,PrevEnd,c=0;

    TempIndex = Index;
    for( i=StartY; i<EndY; i++ )
    {
        PrevEnd = -1000;
        Span = *(TempIndex++);
        while (Span)
        {
            if( (i==StartY)||(i==(EndY-1)) )
            {
                if ((PrevEnd!=-1000) && (PrevEnd >= Span->Start)) appErrorf("%s contains %i-length overlap, line %i/%i",Name,PrevEnd-Span->Start,i-StartY,EndY-StartY);
                if (Span->Start>=Span->End) appErrorf("%s contains %i-length span, line %i/%i",Name,Span->End-Span->Start,i-StartY,EndY-StartY);
                PrevEnd = Span->End;
            }
            Span=Span->Next;
            c++;
        }
    }
    if( (c==0) != (ValidLines==0) )
		appErrorf ("%s bad ValidLines: claimed=%i, correct=%i",Name,ValidLines,c);
    unguard;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
