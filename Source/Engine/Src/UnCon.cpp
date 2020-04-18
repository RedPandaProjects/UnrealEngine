/*=============================================================================
	UnCon.cpp: Implementation of UConsole class
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"

/*------------------------------------------------------------------------------
	UConsole object implementation.
------------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UConsole);

/*------------------------------------------------------------------------------
	Console.
------------------------------------------------------------------------------*/

//
// Constructor.
//
UConsole::UConsole()
{}

//
// Init console.
//
void UConsole::_Init( UViewport* InViewport )
{
	guard(UConsole::_Init);
	check(sizeof(UConsole)==UConsole::StaticClass->GetPropertiesSize());

	// Set properties.
	Viewport		= InViewport;
	TopLine			= MAX_LINES-1;
	BorderSize		= 1; 

	// Init scripting.
	InitExecution();

	// Start console log.
	Logf(LocalizeGeneral("Engine","Core"));
	Logf(LocalizeGeneral("Copyright","Core"));
	Logf(" ");
	Logf(" ");

	unguard;
}

/*------------------------------------------------------------------------------
	Viewport console output.
------------------------------------------------------------------------------*/

//
// Print a message on the playing screen.
// Time = time to keep message going, or 0=until next message arrives, in 60ths sec
//
void UConsole::WriteBinary( const void* Data, INT Length, EName ThisType )
{
	guard(UConsole::WriteBinary);
	eventMessage( (const char*)Data, ThisType );
	unguard;
}

void UConsole::execConsoleCommand( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UConsole::execLog);

	P_GET_STRING(S);
	P_FINISH;

	*(DWORD*)Result = Viewport->Exec( S, this );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UConsole, INDEX_NONE, execConsoleCommand );

/*------------------------------------------------------------------------------
	Rendering.
------------------------------------------------------------------------------*/

//
// Called before rendering the world view.  Here, the
// Viewport console code can affect the screen's Viewport,
// for example by shrinking the view according to the
// size of the status bar.
//
FSceneNode SavedFrame;
void UConsole::PreRender( FSceneNode* Frame )
{
	guard(UConsole::PreRender);

	// Prevent status redraw due to changing.
	eventTick( Viewport->CurrentTime - Viewport->LastUpdateTime );

	// Save the Viewport.
	SavedFrame = *Frame;

	// Compute new status info.
	BorderLines		= 0;
	BorderPixels	= 0;
	ConsoleLines	= 0;

	// Compute sizing of all visible status bar components.
	if( ConsolePos > 0.0 )
	{
		// Show console.
		ConsoleLines = Min(ConsolePos * (FLOAT)Frame->Y, (FLOAT)Frame->Y);
		Frame->Y -= ConsoleLines;
	}
	if( BorderSize>=2 )
	{
		// Encroach on screen area.
		FLOAT Fraction = (FLOAT)(BorderSize-1) / (FLOAT)(MAX_BORDER-1);

		BorderLines = (int)Min((FLOAT)Frame->Y * 0.25f * Fraction,(FLOAT)Frame->Y);
		BorderLines = ::Max(0,BorderLines - ConsoleLines);
		Frame->Y -= 2 * BorderLines;

		BorderPixels = (int)Min((FLOAT)Frame->X * 0.25f * Fraction,(FLOAT)Frame->X) & ~3;
		Frame->X -= 2 * BorderPixels;
	}
	Frame->XB += BorderPixels;
	Frame->YB += ConsoleLines + BorderLines;
	Frame->ComputeRenderSize();

	unguard;
}

//
// Refresh the player console on the specified Viewport.  This is called after
// all in-game graphics are drawn in the rendering loop, and it overdraws stuff
// with the status bar, menus, and chat text.
//
void UConsole::PostRender( FSceneNode* Frame )
{
	guard(UConsole::PostRender);
	check(Viewport->Client->Engine->Client);
	*Frame = SavedFrame;

	// Big status message.
	UFont* LargeFont = Viewport->Canvas->LargeFont;
	char BigMessage[256]="";
	if( Viewport->Actor->bShowMenu )
		appStrcpy( BigMessage, "" );		
	else if( Viewport->Actor->Level->LevelAction==LEVACT_Loading )
		appStrcpy( BigMessage, LocalizeProgress("Loading") );
	else if( Viewport->Actor->Level->LevelAction==LEVACT_Saving )
		appStrcpy( BigMessage, LocalizeProgress("Saving") );
	else if( Viewport->Actor->Level->LevelAction==LEVACT_Connecting )
		appStrcpy( BigMessage, LocalizeProgress("Connecting") );
	else if( Viewport->Actor->Level->Pauser[0] )
	{
		LargeFont = Viewport->Canvas->MedFont;
		appSprintf( BigMessage, LocalizeProgress("Paused"), Viewport->Actor->Level->Pauser );
	}
	if( BigMessage[0] )
	{
		appStrupr( BigMessage );
		INT XL, YL;
		Viewport->Canvas->StrLen( LargeFont, XL, YL, BigMessage );
		Viewport->Canvas->Printf( LargeFont, Frame->X/2-XL/2, Frame->Y/2-YL/2, "%s", BigMessage );
	}

	// If the console has changed since the previous frame, draw it.
	INT YStart	   = BorderLines;
	INT YEnd	   = Frame->Y - BorderLines;
	if( ConsoleLines > 0 )
		Viewport->Canvas->DrawPattern( ConBackground, 0.0, 0.0, Frame->X, ConsoleLines, 1.0, 0.0, ConsoleLines, NULL, 1.0, FPlane(0.7,0.7,0.7,0), FPlane(0,0,0,0), 0 );

	// Draw border.
	if( BorderLines>0 || BorderPixels>0 )
	{
		YStart += ConsoleLines;
		FLOAT V = ConsoleLines/2;
		if( BorderLines > 0 )
		{
			Viewport->Canvas->DrawPattern( Border, 0, 0, Frame->X, BorderLines, 1.0, 0.0, 0.0, NULL, 1.0, FPlane(1,1,1,0), FPlane(0,0,0,0), 0 );
			Viewport->Canvas->DrawPattern( Border, 0, YEnd, Frame->X, BorderLines, 1.0, 0.0, 0.0, NULL, 1.0, FPlane(1,1,1,0), FPlane(0,0,0,0), 0 );
		}
		if( BorderPixels > 0 )
		{
			Viewport->Canvas->DrawPattern( Border, 0, YStart, BorderPixels, YEnd-YStart, 1.0, 0.0, 0.0, NULL, 1.0, FPlane(1,1,1,0), FPlane(0,0,0,0), 0 );
			Viewport->Canvas->DrawPattern( Border, Frame->X-BorderPixels, YStart, BorderPixels, YEnd-YStart, 1.0, 0.0, 0.0, NULL, 1.0, FPlane(1,1,1,0), FPlane(0,0,0,0), 0 );
		}
	}

	// Draw console text.
	if( ConsoleLines )
	{
		// Console is visible; display console view.
		INT Y = ConsoleLines-1;
		appSprintf(MsgText[(TopLine + 1 + MAX_LINES) % MAX_LINES],"(> %s_",TypedStr);
		for( INT i=Scrollback; i<(NumLines+1); i++ )
		{
			// Display all text in the buffer.
			INT Line = (TopLine + MAX_LINES*2 - (i-1)) % MAX_LINES;

			INT XL,YL;
			Viewport->Canvas->WrappedStrLen( Viewport->Canvas->MedFont, XL, YL, Frame->X-8, MsgText[Line] );

			// Half-space blank lines.
			if( YL == 0 )
				YL = 5;

			Y -= YL;
			if( (Y+YL)<0 )
				break;
			Viewport->Canvas->CurX = 4;
			Viewport->Canvas->CurY = Y;
			Viewport->Canvas->WrappedPrintf( Viewport->Canvas->MedFont, 0, "%s", MsgText[Line] );
		}
	}
	else
	{
		// Console is hidden; display single-line view.
		if( TextLines>0 && MsgType!=NAME_None && (!Viewport->Actor->bShowMenu || Viewport->Actor->bShowScores) )
		{
			int iLine=TopLine;
			for( int i=0; i<NumLines; i++ )
			{
				if( *MsgText[iLine] )
					break;
				iLine = (iLine-1+MAX_LINES)%MAX_LINES;
			}
			Viewport->Canvas->CurX = 4;
			Viewport->Canvas->CurY = 2;
			Viewport->Canvas->WrappedPrintf( Viewport->Canvas->MedFont, 1, "%s", MsgText[iLine] );
			if ( TextLines > 1 )
			{
				iLine = (iLine-1+MAX_LINES)%MAX_LINES;
				for ( int j=0; j<i; j++ )
				{
					if( *MsgText[iLine] )
						break;
					iLine = (iLine-1+MAX_LINES)%MAX_LINES;
				}
				Viewport->Canvas->CurY = 12;
				Viewport->Canvas->WrappedPrintf( Viewport->Canvas->MedFont, 1, "%s", MsgText[iLine] );
			}
		}
		if( GetMainFrame()->Node && GetMainFrame()->Node->GetFName()=="Typing" )
		{
			// Draw stuff being typed.
			int XL,YL;
			char S[256];
			appSprintf( S, "(> %s_", TypedStr );
			Viewport->Canvas->WrappedStrLen( Viewport->Canvas->MedFont, XL, YL, Frame->X-8, S );
			Viewport->Canvas->CurX = 2;
			Viewport->Canvas->CurY = Frame->Y - ConsoleLines - YL - 1;
			Viewport->Canvas->WrappedPrintf( Viewport->Canvas->MedFont, 0, "%s", S );
		}
	}
	unguard;
}

/*------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------*/
