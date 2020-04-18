/*=============================================================================
	Line.h: Line drawing include.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

	FLOAT	FloatFixDX;
	BYTE*	Screen;
	INT		FixDX, FixX, X, Y, DY, Count, DestX, ScreenInc, ColorBytes;

	guard(UnLine);

	// Bounds check.
	if
	(	P1.X < 0.0
	||	P2.X < 0.0
	||	P1.Y < 0.0
	||	P2.Y < 0.0
	||	P1.X > Frame->FX
	||	P2.X > Frame->FX
	||	P1.Y > Frame->FY
	||	P2.Y > Frame->FY )
	{
		debugf( NAME_Warning, "Line: Bounds (%f,%f) (%f,%f)", P1.X, P1.Y, P2.X, P2.Y );
		goto LABEL2(Out);
	}
	ColorBytes = Viewport->ColorBytes;

	// Arrange so that P2.Y >= P1.Y.
	if( P2.Y < P1.Y )
	{
		Exchange( P1, P2 );
#ifdef DEPTHSETUP
		Exchange( FixG1, FixG2 );
#endif
	}
	DestX	= appRound(P2.X);
	Y		= appFloor(P1.Y);
	DY      = appFloor(P2.Y)-Y;
	if( DY==0 )
	{
		// Horizontal line.
		if( P2.X > P1.X )
		{
			X = appFloor(P1.X); Count = appFloor(P2.X) - X;
		}
		else
		{
			X = appFloor(P2.X); Count = appFloor(P1.X) - X;
#ifdef DEPTHSETUP
			Exchange(FixG1,FixG2);
#endif
		}
#if ISDOTTED
		LineToggle = X&1;
#endif
		if( Count>0 )
		{
			guard(Case 1);
#ifdef DEPTHSETUP
			DEPTHSETUP(Count);
#endif
			Screen = Frame->Screen(X,Y);
			while( Count-- > 0 )
			{
				L_DRAWPIXEL(Screen); 
				Screen += ColorBytes;
			}
			unguard;
		}
		goto LABEL2(Out);
	}
	FloatFixDX = 65536.0 * (P2.X-P1.X) / (P2.Y-P1.Y);
	FixDX      = appRound(FloatFixDX);
	FixX       = appRound(65536.0 * P1.X + FloatFixDX * ((FLOAT)(Y+1) - P1.Y));
	if( FixDX < -Fix(1) )
	{		   
		// From -infinity to -1 (Horizontal major).
		guard(Case 3);
		X      = appFloor(P1.X);
		Screen = Frame->Screen(X,Y);

		#if ISDOTTED
			LineToggle = X&1;
		#endif

#ifdef DEPTHSETUP
		DEPTHSETUP(::Max(DY,X-appFloor(P2.X)));
#endif

		while( --DY >= 0 )
		{
			Count 	 = X;
			X 		 = Unfix(FixX);
			Count 	-= X;
			while( Count-- > 0 )
			{
				Screen -= ColorBytes; 
				L_DRAWPIXEL(Screen);
			}
			Screen 	+= Viewport->Stride * ColorBytes;
			FixX   	+= FixDX;
		}
		while( X-- > DestX )
		{
			Screen -= ColorBytes;
			L_DRAWPIXEL(Screen);
		}
		unguard;
	}
	else if( FixDX > Fix(1) )
	{
		// From 1 to +infinity (Horizontal major).
		guard(Case 4);
		X 	   = appFloor( P1.X );
		Screen = Frame->Screen(X,Y);

		#if ISDOTTED
			LineToggle = X&1;
		#endif

#ifdef DEPTHSETUP
		DEPTHSETUP(::Max(DY,appFloor(P2.X)-X));
#endif

		while( --DY >= 0 )
		{
			Count 	 = X;
			X 		 = Unfix(FixX);
			Count 	-= X;
			while( Count++ < 0 )
			{
				L_DRAWPIXEL(Screen);
				Screen += ColorBytes;
			}
			Screen 	+= Viewport->Stride * ColorBytes;
			FixX   	+= FixDX;
		}
		while( ++X < DestX )
		{
			L_DRAWPIXEL(Screen);
			Screen += ColorBytes;
		}
		unguard;
	}
	else if( DY > 0 )
	{
		// Vertical major.
		ScreenInc = Viewport->Stride * ColorBytes;
		Screen = Frame->Screen(0,Y);
		#if ISDOTTED
			LineToggle = Y & 1;
		#endif

#ifdef DEPTHSETUP
		DEPTHSETUP(DY);
#endif

		int D=DY,XX=FixX;
		guard(Case 2);
		do
		{
			L_DRAWPIXEL(Screen + (Unfix(FixX)<<SHIFT));
			FixX 	+= FixDX;
			Screen 	+= ScreenInc;
		} while (--DY > 0);
		unguard;
	}
	unguard;
	LABEL2(Out):;

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
