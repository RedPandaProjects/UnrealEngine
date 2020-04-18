/*=============================================================================
	Line1.h: Line drawing include.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

if( !(LineFlags & LINE_Transparent) )
{
	// Regular line.
	#define L_DRAWPIXEL(Dest)	DRAWPIXEL(Dest)
	#define ISDOTTED 0
	#define LABEL2(X) LABEL1(X)##Normal
	#include "Line.h"
	#undef  LABEL2
	#undef  ISDOTTED
	#undef  L_DRAWPIXEL
}
else
{
	// Dotted line.
	int LineToggle=0;
	#define L_DRAWPIXEL(Dest)  if( (LineToggle^=1) != 0 ) DRAWPIXEL(Dest)
	#define ISDOTTED 1
	#define LABEL2(X) LABEL1(X)##Dotted
	#include "Line.h"
	#undef LABEL2
	#undef ISDOTTED
	#undef L_DRAWPIXEL
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
