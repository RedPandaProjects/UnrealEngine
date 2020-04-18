/*=============================================================================
	UnCon.h: UConsole game-specific definition
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Contains routines for: Messages, menus, status bar
=============================================================================*/

/*------------------------------------------------------------------------------
	UConsole definition.
------------------------------------------------------------------------------*/

//
// Viewport console.
//
class ENGINE_API UConsole : public UObject, public FOutputDevice
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UConsole,UObject,CLASS_Transient)

	// Constructor.
	UConsole();

	// UConsole interface.
	virtual void _Init( UViewport* Viewport );
	virtual void PreRender( FSceneNode* Frame );
	virtual void PostRender( FSceneNode* Frame );
	virtual void WriteBinary( const void* Data, INT Length, EName MsgType=NAME_None );

	// Intrinsics.
	INTRINSIC(execConsoleCommand);

	// Script events.
    void eventMessage(const CHAR* S,FName Name)
    {
        struct {CHAR S[240]; FName N;} Parms;
        appStrncpy(Parms.S,S,240);
		Parms.N=Name;
        ProcessEvent(FindFunctionChecked(NAME_Message),&Parms);
    }
    void eventTick(FLOAT DeltaTime)
    {
        struct {FLOAT DeltaTime; } Parms;
        Parms.DeltaTime=DeltaTime;
        ProcessEvent(FindFunctionChecked(ENGINE_Tick),&Parms);
    }
    void eventPostRender(class UCanvas* C)
    {
        struct {class UCanvas* C; } Parms;
        Parms.C=C;
        ProcessEvent(FindFunctionChecked(ENGINE_PostRender),&Parms);
    }
    void eventPreRender(class UCanvas* C)
    {
        struct {class UCanvas* C; } Parms;
        Parms.C=C;
        ProcessEvent(FindFunctionChecked(ENGINE_PreRender),&Parms);
    }
    DWORD eventKeyType(BYTE Key)
    {
        struct {BYTE Key; DWORD ReturnValue; } Parms;
        Parms.Key=Key;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(NAME_KeyType),&Parms);
        return Parms.ReturnValue;
    }
    DWORD eventKeyEvent(BYTE Key, BYTE Action, FLOAT Delta)
    {
        struct {BYTE Key; BYTE Action; FLOAT Delta; DWORD ReturnValue; } Parms;
        Parms.Key=Key;
        Parms.Action=Action;
        Parms.Delta=Delta;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(NAME_KeyEvent),&Parms);
        return Parms.ReturnValue;
    }
private:
	// Constants.
	enum {MAX_BORDER     = 6};
	enum {MAX_LINES		 = 64};
	enum {MAX_HISTORY	 = 16};
	enum {TEXTMSG_LENGTH = 128};
	typedef char TEXTMSG[TEXTMSG_LENGTH];

	// Variables.
	UViewport*  Viewport;
	INT			HistoryTop;
	INT			HistoryBot;
	INT			HistoryCur;
	TEXTMSG		TypedStr;
	TEXTMSG		History[MAX_HISTORY];
	INT			Scrollback;
	INT			NumLines;
	INT			TopLine;
	INT			TextLines;
	FName  		MsgType;
	FLOAT		MsgTime;
	TEXTMSG 	MsgText[MAX_LINES];
	INT			BorderSize;
	INT			ConsoleLines;
	INT			BorderLines;
	INT			BorderPixels;
	FLOAT		ConsolePos;
	FLOAT		ConsoleDest;
	UTexture*	ConBackground;
	UTexture*	Border;
    DWORD bNoStuff:1;
};

/*------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------*/
