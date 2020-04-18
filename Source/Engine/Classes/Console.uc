//=============================================================================
// Console: A player console, associated with a viewport.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Console expands Object
	transient
	intrinsic;

// Imports.
#exec Texture Import NAME=ConsoleBack File=Textures\Console.pcx
#exec Texture Import File=Textures\Border.pcx

// Internal.
var private const int vtblOut;

// Constants.
const MaxBorder=6;
const MaxLines=64;
const MaxHistory=16;
const TextMsgSize=128;

// Variables.
var viewport Viewport;
var int HistoryTop, HistoryBot, HistoryCur;
var string[128] TypedStr, History[16];
var int Scrollback, NumLines, TopLine, TextLines;
var name MsgType;
var float MsgTime;
var string[128] MsgText[64];
var int BorderSize;
var int ConsoleLines, BorderLines, BorderPixels;
var float ConsolePos, ConsoleDest;
var texture ConBackground, Border;
var bool bNoStuff;

//-----------------------------------------------------------------------------
// Input.

// Input system states.
enum EInputAction
{
	IST_None,    // Not performing special input processing.
	IST_Press,   // Handling a keypress or button press.
	IST_Hold,    // Handling holding a key or button.
	IST_Release, // Handling a key or button release.
	IST_Axis,    // Handling analog axis movement.
};

// Input keys.
enum EInputKey
{
/*00*/	IK_None			,IK_LeftMouse	,IK_RightMouse	,IK_Cancel		,
/*04*/	IK_MiddleMouse	,IK_Unknown05	,IK_Unknown06	,IK_Unknown07	,
/*08*/	IK_Backspace	,IK_Tab         ,IK_Unknown0A	,IK_Unknown0B	,
/*0C*/	IK_Unknown0C	,IK_Enter	    ,IK_Unknown0E	,IK_Unknown0F	,
/*10*/	IK_Shift		,IK_Ctrl	    ,IK_Alt			,IK_Pause       ,
/*14*/	IK_CapsLock		,IK_Unknown15	,IK_Unknown16	,IK_Unknown17	,
/*18*/	IK_Unknown18	,IK_Unknown19	,IK_Unknown1A	,IK_Escape		,
/*1C*/	IK_Unknown1C	,IK_Unknown1D	,IK_Unknown1E	,IK_Unknown1F	,
/*20*/	IK_Space		,IK_PageUp      ,IK_PageDown    ,IK_End         ,
/*24*/	IK_Home			,IK_Left        ,IK_Up          ,IK_Right       ,
/*28*/	IK_Down			,IK_Select      ,IK_Print       ,IK_Execute     ,
/*2C*/	IK_PrintScrn	,IK_Insert      ,IK_Delete      ,IK_Help		,
/*30*/	IK_0			,IK_1			,IK_2			,IK_3			,
/*34*/	IK_4			,IK_5			,IK_6			,IK_7			,
/*38*/	IK_8			,IK_9			,IK_Unknown3A	,IK_Unknown3B	,
/*3C*/	IK_Unknown3C	,IK_Unknown3D	,IK_Unknown3E	,IK_Unknown3F	,
/*40*/	IK_Unknown40	,IK_A			,IK_B			,IK_C			,
/*44*/	IK_D			,IK_E			,IK_F			,IK_G			,
/*48*/	IK_H			,IK_I			,IK_J			,IK_K			,
/*4C*/	IK_L			,IK_M			,IK_N			,IK_O			,
/*50*/	IK_P			,IK_Q			,IK_R			,IK_S			,
/*54*/	IK_T			,IK_U			,IK_V			,IK_W			,
/*58*/	IK_X			,IK_Y			,IK_Z			,IK_Unknown5B	,
/*5C*/	IK_Unknown5C	,IK_Unknown5D	,IK_Unknown5E	,IK_Unknown5F	,
/*60*/	IK_NumPad0		,IK_NumPad1     ,IK_NumPad2     ,IK_NumPad3     ,
/*64*/	IK_NumPad4		,IK_NumPad5     ,IK_NumPad6     ,IK_NumPad7     ,
/*68*/	IK_NumPad8		,IK_NumPad9     ,IK_GreyStar    ,IK_GreyPlus    ,
/*6C*/	IK_Separator	,IK_GreyMinus	,IK_NumPadPeriod,IK_GreySlash   ,
/*70*/	IK_F1			,IK_F2          ,IK_F3          ,IK_F4          ,
/*74*/	IK_F5			,IK_F6          ,IK_F7          ,IK_F8          ,
/*78*/	IK_F9           ,IK_F10         ,IK_F11         ,IK_F12         ,
/*7C*/	IK_F13			,IK_F14         ,IK_F15         ,IK_F16         ,
/*80*/	IK_F17			,IK_F18         ,IK_F19         ,IK_F20         ,
/*84*/	IK_F21			,IK_F22         ,IK_F23         ,IK_F24         ,
/*88*/	IK_Unknown88	,IK_Unknown89	,IK_Unknown8A	,IK_Unknown8B	,
/*8C*/	IK_Unknown8C	,IK_Unknown8D	,IK_Unknown8E	,IK_Unknown8F	,
/*90*/	IK_NumLock		,IK_ScrollLock  ,IK_Unknown92	,IK_Unknown93	,
/*94*/	IK_Unknown94	,IK_Unknown95	,IK_Unknown96	,IK_Unknown97	,
/*98*/	IK_Unknown98	,IK_Unknown99	,IK_Unknown9A	,IK_Unknown9B	,
/*9C*/	IK_Unknown9C	,IK_Unknown9D	,IK_Unknown9E	,IK_Unknown9F	,
/*A0*/	IK_LShift		,IK_RShift      ,IK_LControl    ,IK_RControl    ,
/*A4*/	IK_UnknownA4	,IK_UnknownA5	,IK_UnknownA6	,IK_UnknownA7	,
/*A8*/	IK_UnknownA8	,IK_UnknownA9	,IK_UnknownAA	,IK_UnknownAB	,
/*AC*/	IK_UnknownAC	,IK_UnknownAD	,IK_UnknownAE	,IK_UnknownAF	,
/*B0*/	IK_UnknownB0	,IK_UnknownB1	,IK_UnknownB2	,IK_UnknownB3	,
/*B4*/	IK_UnknownB4	,IK_UnknownB5	,IK_UnknownB6	,IK_UnknownB7	,
/*B8*/	IK_UnknownB8	,IK_UnknownB9	,IK_Semicolon	,IK_Equals		,
/*BC*/	IK_Comma		,IK_Minus		,IK_Period		,IK_Slash		,
/*C0*/	IK_Tilde		,IK_UnknownC1	,IK_UnknownC2	,IK_UnknownC3	,
/*C4*/	IK_UnknownC4	,IK_UnknownC5	,IK_UnknownC6	,IK_UnknownC7	,
/*C8*/	IK_Joy1	        ,IK_Joy2	    ,IK_Joy3	    ,IK_Joy4	    ,
/*CC*/	IK_Joy5	        ,IK_Joy6	    ,IK_Joy7	    ,IK_Joy8	    ,
/*D0*/	IK_Joy9	        ,IK_Joy10	    ,IK_Joy11	    ,IK_Joy12		,
/*D4*/	IK_Joy13		,IK_Joy14	    ,IK_Joy15	    ,IK_Joy16	    ,
/*D8*/	IK_UnknownD8	,IK_UnknownD9	,IK_UnknownDA	,IK_LeftBracket	,
/*DC*/	IK_Backslash	,IK_RightBracket,IK_SingleQuote	,IK_UnknownDF	,
/*E0*/  IK_JoyX			,IK_JoyY		,IK_JoyZ		,IK_JoyR		,
/*E4*/	IK_MouseX		,IK_MouseY		,IK_MouseZ		,IK_MouseW		,
/*E8*/	IK_JoyU			,IK_JoyV		,IK_UnknownEA	,IK_UnknownEB	,
/*EC*/	IK_MouseWheelUp ,IK_MouseWheelDown,IK_Unknown10E,UK_Unknown10F  ,
/*F0*/	IK_UnknownF0	,IK_UnknownF1	,IK_UnknownF2	,IK_UnknownF3	,
/*F4*/	IK_UnknownF4	,IK_UnknownF5	,IK_Attn		,IK_CrSel		,
/*F8*/	IK_ExSel		,IK_ErEof		,IK_Play		,IK_Zoom		,
/*FC*/	IK_NoName		,IK_PA1			,IK_OEMClear
};

//-----------------------------------------------------------------------------
// Intrinsics.

// Execute a command on this console.
intrinsic function bool ConsoleCommand( coerce string[240] S );

//-----------------------------------------------------------------------------
// Exec functions accessible from the console and key bindings.

// Begin typing a command on the console.
exec function Type()
{
	TypedStr="";
	GotoState( 'Typing' );
}
 
exec function Talk()
{
	TypedStr="Say ";
	bNoStuff = true;
	GotoState( 'Typing' );
}

// Size the view up.
exec function ViewUp()
{
	BorderSize = Clamp( BorderSize-1, 0, MaxBorder );
}

// Size the view down.
exec function ViewDown()
{
	BorderSize = Clamp( BorderSize+1, 0, MaxBorder );
}

//-----------------------------------------------------------------------------
// Functions.

// Write to console.
event Message( coerce string[240] Msg, optional name N )
{
	if( Msg!="" )
	{
		TopLine		     = (TopLine+1) % MaxLines;
		NumLines	     = Min(NumLines+1,MaxLines-1);
		MsgType  	     = N;
		MsgTime		     = 3.0;
		TextLines++;
		MsgText[TopLine] = Msg;
	}
}

// Called by the engine when a single key is typed.
event bool KeyType( EInputKey Key );

// Called by the engine when a key, mouse, or joystick button is pressed
// or released, or any analog axis movement is processed.
event bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
{
	if( Action!=IST_Press )
	{
		return false;
	}
	else if( Key==IK_Tilde )
	{
		if( ConsoleDest==0.0 )
		{
			ConsoleDest=0.6;
			GotoState('Typing');
		}
		else GotoState('');
		return true;
	}
	else return false;
}

// Called each rendering iteration to update any time-based display.
event Tick( float Delta )
{
	// Slide console up or down.
	if( ConsolePos < ConsoleDest )
		ConsolePos = FMin(ConsolePos+Delta,ConsoleDest);
	else if( ConsolePos > ConsoleDest )
		ConsolePos = FMax(ConsolePos-Delta,ConsoleDest);

	// Update status message.
	if( ((MsgTime-=Delta) <= 0.0) && (TextLines > 0) )
		TextLines--;
}

// Called before rendering the world view.
event PreRender( canvas C );

// Called after rendering the world view.
event PostRender( canvas C );

//-----------------------------------------------------------------------------
// State used while typing a command on the console.

state Typing
{
	exec function Type()
	{
		TypedStr="";
		gotoState( '' );
	}
	function bool KeyType( EInputKey Key )
	{
		if( Key>=0x20 && Key<0x80 && Key!=Asc("~") && Key!=Asc("`") )
		{
			if ( bNoStuff )
			{
				bNoStuff = false;
				return true;
			}
			TypedStr = TypedStr $ Chr(Key);
			Scrollback=0;
			return true;
		}
	}
	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		local string[255] Temp;
		if( global.KeyEvent( Key, Action, Delta ) || Action!=IST_Press )
		{
			return true;
		}
		else if( Key==IK_Escape )
		{
			if( Scrollback!=0 )
			{
				Scrollback=0;
			}
			else if( TypedStr!="" )
			{
				TypedStr="";
			}
			else
			{
				ConsoleDest=0.0;
				GotoState( '' );
			}
			Scrollback=0;
		}
		else if( Key==IK_Enter )
		{
			if( Scrollback!=0 )
			{
				Scrollback=0;
			}
			else
			{
				if( TypedStr!="" )
				{
					// Print to console.
					if( ConsoleLines!=0 )
						Message( "(> " $ TypedStr, 'Console' );

					// Update history buffer.
					History[HistoryCur++ % MaxHistory] = TypedStr;
					if( HistoryCur > HistoryBot )
						HistoryBot++;
					if( HistoryCur - HistoryTop >= MaxHistory )
						HistoryTop = HistoryCur - MaxHistory + 1;

					// Make a local copy of the string.
					Temp=TypedStr;
					if( !ConsoleCommand( TypedStr ) )
						Message( Localize('Errors','Exec','Core') );
					Message( "" );
				}
				TypedStr="";
				if( ConsoleDest==0.0 )
					GotoState('');
				Scrollback=0;
			}
		}
		else if( Key==IK_Up )
		{
			if( HistoryCur > HistoryTop )
			{
				History[HistoryCur % MaxHistory] = TypedStr;
				TypedStr = History[--HistoryCur % MaxHistory];
			}
			Scrollback=0;
		}
		else if( Key==IK_Down )
		{
			History[HistoryCur % MaxHistory] = TypedStr;
			if( HistoryCur < HistoryBot )
				TypedStr = History[++HistoryCur % MaxHistory];
			else
				TypedStr="";
			Scrollback=0;
		}
		else if( Key==IK_PageUp )
		{
			if( ++Scrollback >= MaxLines )
				Scrollback = MaxLines-1;
		}
		else if( Key==IK_PageDown )
		{
			if( --Scrollback < 0 )
				Scrollback = 0;
		}
		else if( Key==IK_Backspace || Key==IK_Left )
		{
			if( Len(TypedStr)>0 )
				TypedStr = Left(TypedStr,Len(TypedStr)-1);
			Scrollback = 0;
		}
		return true;
	}
	function EndState()
	{
		log("Console leaving Typing");
		ConsoleDest=0.0;
	}
}

//-----------------------------------------------------------------------------
// State used while in a menu.

state Menuing
{
	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		local Menu PlayerMenu;

		if ( Action != IST_Press )
			return false;

		if ( (Viewport.Actor.myHUD == None) || (Viewport.Actor.myHUD.MainMenu == None) )
			return false;
		
		PlayerMenu = Viewport.Actor.myHUD.MainMenu;
		PlayerMenu.MenuProcessInput(Key, Action);
		Scrollback=0;
		return true;
	}
	function BeginState()
	{
		log("Console entering Menuing");
	}
	function EndState()
	{
		log("Console leaving Menuing");
	}
}

//-----------------------------------------------------------------------------
// State used while typing in a menu.

state MenuTyping
{
	function bool KeyType( EInputKey Key )
	{
		if( Key>=0x20 && Key<0x80 && Key!=Asc("~") && Key!=Asc("`") && Key!=Asc(" ") )
		{
			TypedStr = TypedStr $ Chr(Key);
			Scrollback=0;
			if ( (Viewport.Actor.myHUD != None) && (Viewport.Actor.myHUD.MainMenu != None) )
				Viewport.Actor.myHUD.MainMenu.ProcessMenuUpdate( TypedStr );
			return true;
		}
	}	
	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		local Menu PlayerMenu;

		if( Action != IST_Press )
			return false;

		if( Viewport.Actor.myHUD==None || Viewport.Actor.myHUD.MainMenu==None )
			return false;
		
		PlayerMenu = Viewport.Actor.myHUD.MainMenu;

		if( Key==IK_Escape )
		{
			if( Scrollback!=0 )
				Scrollback = 0;
			else if( TypedStr!="" )
				TypedStr="";
			else
				GotoState( 'Menuing' );
			PlayerMenu.ProcessMenuEscape();
			Scrollback=0;
		}
		else if( Key==IK_Enter )
		{
			if( Scrollback!=0 )
				Scrollback = 0;
			else
			{
				if( TypedStr!="" )
					PlayerMenu.ProcessMenuInput( TypedStr );	
				TypedStr="";
				GotoState( 'Menuing' );
				Scrollback = 0;
			}
		}
		else if( Key==IK_Backspace || Key==IK_Left )
		{
			if( Len(TypedStr)>0 )
				TypedStr = Left(TypedStr,Len(TypedStr)-1);
			Scrollback = 0;
			PlayerMenu.ProcessMenuUpdate( TypedStr );	
		}
		return true;
	}
	function BeginState()
	{
		log("Console entering MenuTyping");
	}
	function EndState()
	{
		log("Console leaving MenuTyping");
	}
}

//-----------------------------------------------------------------------------
// State used while expecting single key input in a menu.

state KeyMenuing
{
	function bool KeyType( EInputKey Key )
	{
		ConsoleDest=0.0;
		if( Viewport.Actor.myHUD!=None && Viewport.Actor.myHUD.MainMenu!=None )
			Viewport.Actor.myHUD.MainMenu.ProcessMenuKey( Key, Chr(Key) );
		Scrollback=0;
		GotoState( 'Menuing' );
	}
	function bool KeyEvent( EInputKey Key, EInputAction Action, FLOAT Delta )
	{
		if( Action==IST_Press )
		{
			ConsoleDest=0.0;
			if( Viewport.Actor.myHUD!=None && Viewport.Actor.myHUD.MainMenu!=None )
				Viewport.Actor.myHUD.MainMenu.ProcessMenuKey( Key, mid(string(GetEnum(enum'EInputKey',Key)),3) );
			Scrollback=0;
			GotoState( 'Menuing' );
			return true;
		}
	}
	function BeginState()
	{
		log( "Console entering KeyMenuing" );
	}
	function EndState()
	{
		log( "Console leaving KeyMenuing" );
	}
}

defaultproperties
{
	ConBackground=ConsoleBack
	Border=Border
}
