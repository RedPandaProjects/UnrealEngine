//=============================================================================
// LevelInfo contains information about the current level. There should 
// be one per level and it should be actor 0. UnrealEd creates each level's 
// LevelInfo automatically so you should never have to place one
// manually.
//
// The ZoneInfo properties in the LevelInfo are used to define
// the properties of all zones which don't themselves have ZoneInfo.
//=============================================================================
class LevelInfo expands ZoneInfo
	localized
	intrinsic;

// Textures.
#exec Texture Import File=Textures\DefaultTexture.pcx

//-----------------------------------------------------------------------------
// Level time.

// Time passage.
var() float TimeDilation;          // Normally 1 - scales real time passage.

// Current time.
var           float	TimeSeconds;   // Time in seconds since level began play.
var transient int   Year;          // Year.
var transient int   Month;         // Month.
var transient int   Day;           // Day of month.
var transient int   DayOfWeek;     // Day of week.
var transient int   Hour;          // Hour.
var transient int   Minute;        // Minute.
var transient int   Second;        // Second.
var transient int   Millisecond;   // Millisecond.

//-----------------------------------------------------------------------------
// Text info about level.

var() localized string[64] Title;
var()           string[64] Author;		    // Who built it.
var() localized string[64] LevelEnterText;  // Message to tell players when they enter.
var()           string[64] LocalizedPkg;    // Package to look in for localizations.
var   string[32] Pauser;                    // If paused, name of person pausing the game.

//-----------------------------------------------------------------------------
// Flags affecting the level.

var() bool           bLonePlayer;     // No multiplayer coordination, i.e. for entranceways.
var bool             bBegunPlay;      // Whether gameplay has begun.
var bool             bPlayersOnly;    // Only update players.
var bool             bHighDetailMode; // Client high-detail mode.
var bool             bStartup;        // Starting gameplay.

//-----------------------------------------------------------------------------
// Audio properties.

var(Audio) const music  Song;          // Default song for level.
var(Audio) const byte   SongSection;   // Default song order for level.
var(Audio) const byte   CdTrack;       // Default CD track for level.
var(Audio) float        PlayerDoppler; // Player doppler shift, 0=none, 1=full.

//-----------------------------------------------------------------------------
// Miscellaneous information.

var() float Brightness;
var texture DefaultTexture;
var int HubStackLevel;
var transient enum ELevelAction
{
	LEVACT_None,
	LEVACT_Loading,
	LEVACT_Saving,
	LEVACT_Connecting
} LevelAction;

//-----------------------------------------------------------------------------
// Networking.

var enum ENetMode
{
	NM_Standalone,        // Standalone game.
	NM_DedicatedServer,   // Dedicated server, no local client.
	NM_ListenServer,      // Listen server.
	NM_Client             // Client only, no local server.
} NetMode;
var bool bInternet;		  // Whether Internet support is active.
var string[32] ComputerName; // Machine's name according to the OS.
var string[32] EngineVersion; // Engine version.

//-----------------------------------------------------------------------------
// Gameplay rules

var() class<gameinfo> DefaultGameType;
var GameInfo Game;

//-----------------------------------------------------------------------------
// Navigation point and Pawn lists (chained using nextNavigationPoint and nextPawn)

var const NavigationPoint NavigationPointList;
var const Pawn PawnList;

//-----------------------------------------------------------------------------
// Server related.

var string[240] NextURL;
var bool bNextItems;
var float NextSwitchCountdown;

//-----------------------------------------------------------------------------
// Functions.

//
// Return the URL of this level on the local machine.
//
intrinsic function string[240] GetLocalURL();

//
// Return the URL of this level, which may possibly
// exist on a remote machine.
//
intrinsic function string[240] GetAddressURL();

//
// Jump the server to a new level.
//
function ServerTravel( string[240] URL, bool bItems )
{
	if( NextURL=="" )
	{
		bNextItems          = bItems;
		NextURL             = URL;
		if( Game!=None )
			Game.ProcessServerTravel( URL, bItems );
		else
			NextSwitchCountdown = 0;
	}
}

//-----------------------------------------------------------------------------
// Network replication.

replication
{
	reliable if( Role==ROLE_Authority )
		Pauser, TimeDilation;
}

defaultproperties
{
     DayFactor=+00001.000000
     TimeDilation=+00001.000000
     SkyScale=+00001.000000
     SkyFlicker=+00000.040000
     SkyDayBright=+00001.000000
     SkyNightBright=+00000.600000
     SkyUPanSpeed=+00000.200000
     SkyVPanSpeed=+00000.200000
     SkyWavyness=+00002.000000
     SkyWavySpeed=+00001.000000
	 Brightness=1
     Title="Untitled"
     bHiddenEd=True
	 CdTrack=255
	 DefaultTexture=DefaultTexture
	 HubStackLevel=0
	 SkyBias=96
	 bHighDetailMode=True
	 PlayerDoppler=0
	 bAcceptClients=True
}
