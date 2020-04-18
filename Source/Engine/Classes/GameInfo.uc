//=============================================================================
// GameInfo.
//
// default game info is normal single player
//
//=============================================================================
class GameInfo expands Info
	config
	localized
	intrinsic;

//-----------------------------------------------------------------------------
// Variables.

var int ItemGoals, KillGoals, SecretGoals;
var globalconfig byte	Difficulty;		// 0=easy, 1=medium, 2=hard, 3=very hard.
var() config bool   	bNoMonsters;	// Whether monsters are allowed in this play mode.
var() config bool		bMuteSpectators; //Whether spectators are allowed to speak
var() config bool		bHumansOnly;
var() bool				bRestartLevel;
var() config bool		bAllowRemoteAdmin;
var() bool				bPauseable;		// Whether the level is pauseable.
var() config bool		bCoopWeaponMode;
var   globalconfig bool	bLowGore;
var	  globalconfig bool bShareware;		// If running shareware version
var	  bool				bCanChangeSkin;

var() config float		AutoAim;		//how much autoaiming to do (1 = none, 0 = always)
										// (cosine of max error to correct)
var() globalconfig float		GameSpeed;
var   float		StartTime;
var() class<playerpawn> DefaultPlayerClass;
var() class<weapon> DefaultWeapon;	// default weapon given to player at start
var() config int		MaxSpectators; //Maximum number of spectators
var() private config string[32] AdminPassword; //Password to receive bAdmin privileges
var	  Scoreboard Scores;
var() class<scoreboard> ScoreBoardType;
var() class<menu> GameMenuType;
var() class<hud> HUDType;
var() class<MapList> MapListType;
var() string[16]	MapPrefix; // Prefix characters for names of maps for this game type
var() string[15] BeaconName;
var() string[64] SpecialDamageString;
var localized string[64] SwitchLevelMessage;
var	  int		SentText;
var localized string[16] DefaultPlayerName;
var localized string[255] LeftMessage;
var localized string[255] FailedSpawnMessage;
var localized string[255] FailedPlaceMessage;
var localized string[255] NameChangedMessage;
var localized string[255] EnteredMessage;

// Default waterzone entry and exit effects
var class<actor> DefaultWaterEntryActor;
var class<actor> DefaultWaterExitActor;
var sound DefaultWaterEntrySound;
var sound DefaultWaterExitSound;

//------------------------------------------------------------------------------
// Engine notifications.

function PreBeginPlay()
{
	StartTime = Level.TimeSeconds;
	SetTimer(1.0, true);
	Level.TimeDilation = GameSpeed;
	// don't call parent prebeginplay
}

function PostBeginPlay()
{
	if( (ScoreBoardType != None) && (Scores == None) )
		Scores = spawn(ScoreBoardType);
	Super.PostBeginPlay();
}

function Timer()
{
	SentText = 0;
	if( Scores != None )
	{
		Scores.UpdateScores();
	}
}

//------------------------------------------------------------------------------
// Game parameters.

//
// Set gameplay speed.
//
function SetGameSpeed( Float T )
{
	GameSpeed = FMax(T, 0.1);
	Level.TimeDilation = GameSpeed;
}

//
// Called after setting low or high detail mode.
//
event DetailChange()
{
	local actor A;
	local zoneinfo Z;
	local skyzoneinfo S;
	if( !Level.bHighDetailMode )
	{
		foreach AllActors(class'Actor', A)
		{
			if( A.bHighDetail && !A.bAlwaysRelevant )
				A.Destroy();
		}
	}
	foreach AllActors(class'ZoneInfo', Z)
	{
		foreach AllActors(class'SkyZoneInfo', S)
			Z.SkyZone = S;
		foreach AllActors(class'SkyZoneInfo', S)
			if( S.bHighDetail == Level.bHighDetailMode )
				Z.SkyZone = S;
	}
}

//
// Return whether an actor should be destroyed in
// this type of game.
//	
function bool IsRelevant( actor Other )
{
	if
	(	(Difficulty==0 && !Other.bDifficulty0 )
	||  (Difficulty==1 && !Other.bDifficulty1 )
	||  (Difficulty==2 && !Other.bDifficulty2 )
	||  (Difficulty==3 && !Other.bDifficulty3 )
	||  (!Other.bSinglePlayer && (Level.NetMode==NM_Standalone) ) 
	||  (!Other.bNet && ((Level.NetMode == NM_DedicatedServer) || (Level.NetMode == NM_ListenServer)) )
	||  (!Other.bNetSpecial  && (Level.NetMode==NM_Client)) )
		return False;

	if( bNoMonsters && (Pawn(Other) != None) && !Pawn(Other).bIsPlayer )
		return False;
		
	if( FRand() > Other.OddsOfAppearing )
		return False;

    // Update the level info goal counts.
    if( Other.bIsSecretGoal )
       SecretGoals++;

    if( Other.bIsItemGoal )
       ItemGoals++;

    if( Other.bIsKillGoal )
       KillGoals++;

	return True;
}

//------------------------------------------------------------------------------
// Player start functions

//
// Grab the next option from a string.
//
function bool GrabOption( out string[120] Options, out string[64] Result )
{
	if( Left(Options,1)=="?" )
	{
		// Get result.
		Result = Mid(Options,1);
		if( InStr(Result,"?")>=0 )
			Result = Left( Result, InStr(Result,"?") );

		// Update options.
		Options = Mid(Options,1);
		if( InStr(Options,"?")>=0 )
			Options = Mid( Options, InStr(Options,"?") );
		else
			Options = "";

		return true;
	}
	else return false;
}

//
// Break up a key=value pair into its key and value.
//
function GetKeyValue( string[64] Pair, out string[64] Key, out string[64] Value )
{
	if( InStr(Pair,"=")>=0 )
	{
		Key   = Left(Pair,InStr(Pair,"="));
		Value = Mid(Pair,InStr(Pair,"=")+1);
	}
	else
	{
		Key   = Pair;
		Value = "";
	}
}

//
// See if an option was specified in the options string.
//
function bool HasOption( string[120] Options, string[32] InKey )
{
	local string[64] Pair, Key, Value;
	while( GrabOption( Options, Pair ) )
	{
		GetKeyValue( Pair, Key, Value );
		if( Key ~= InKey )
			return true;
	}
	return false;
}

//
// Find an option in the options string and return it.
//
function String[32] ParseOption( string[120] Options, string[32] InKey )
{
	local string[64] Pair, Key, Value;
	while( GrabOption( Options, Pair ) )
	{
		GetKeyValue( Pair, Key, Value );
		if( Key ~= InKey )
			return Value;
	}
	return "";
}

//
// Initialize the game.
//warning: this is called before actors' PreBeginPlay.
//
event InitGame( string[120] Options, out string[80] Error )
{
	local string[64] InDiff, InAllowAdmin;

	log( "InitGame: " $ Options );
	InDiff = ParseOption( Options, "Difficulty" );
	if ( InDiff != "" )
		Difficulty = int(InDiff);
	log("Difficulty "$Difficulty);

	InAllowAdmin = ParseOption( Options, "AllowAdmin" );
	if ( InAllowAdmin != "" )
	{
		bAllowRemoteAdmin = bool(InAllowAdmin);
		if ( bAllowRemoteAdmin )
		{
			AdminPassword = ParseOption( Options, "AdminPassword");
			log ("Remote Administration with Password "$AdminPassWord);
		}
	} 
}

//
// Return beacon text for serverbeacon.
//
event GetBeaconText( out string[240] Result )
{	
	Result = Level.ComputerName $ " " $ Left(Level.Title,24) $ " " $ BeaconName;
}

//
// Optional handling of ServerTravel for network games.
//
function ProcessServerTravel( string[240] URL, bool bItems )
{
	local playerpawn P;

	// Notify clients we're switching level and give them time to receive.
	foreach AllActors( class'PlayerPawn', P )
		if( P.Player!=None )
			P.ClientTravel( Level.GetAddressURL(), TRAVEL_Relative, bItems );
	
	// Switch immediately if not networking.
	if( Level.NetMode!=NM_DedicatedServer && Level.NetMode!=NM_ListenServer )
		Level.NextSwitchCountdown = 0.0;
}

//
// Accept or reject a player on the server.
// Fails login if you set the Error to a non-empty string.
//
event playerpawn PreLogin
(
	string[120] Options,
	out string[80] Error
)
{
	// Do any name or password or name validation here.
	Error="";
}

//
// Log a player in.
// Fails login if you set the Error string.
//
event playerpawn Login
(
	string[32] Portal,
	string[120] Options,
	out string[80] Error,
	class<playerpawn> SpawnClass
)
{
	local NavigationPoint StartSpot;
	local PlayerPawn      NewPlayer, TestPlayer;
	local Pawn            PawnLink;
	local string[64]      InName, InTeam, InPassword, InSkin;

	// Get URL options.
	InName     = ParseOption( Options, "Name"     );
	InTeam     = ParseOption( Options, "Team"     );
	InPassword = ParseOption( Options, "Password" );
	InSkin	   = ParseOption( Options, "Skin" );
	log( "Login: " $ InName );

	// Find a start spot.
	StartSpot = FindPlayerStart( 0, Portal );
	if( StartSpot == None )
	{
		Error = FailedPlaceMessage;
		return None;
	}

	// Try to match up to existing unoccupied player in level,
	// for savegames and coop level switching.
	for( PawnLink=Level.PawnList; PawnLink!=None; PawnLink=PawnLink.NextPawn )
	{
		TestPlayer = PlayerPawn(PawnLink);
		if
		(	TestPlayer!=None
		&&	TestPlayer.Player==None )
		{
			if
			(	(Level.NetMode==NM_Standalone)
			||	(TestPlayer.PlayerName~=InName && TestPlayer.Password~=InPassword) )
			{
				// Found matching unoccupied player, so use this one.
				NewPlayer = TestPlayer;
				break;
			}
		}
	}
	if( NewPlayer==None )
	{
		// Not found, so spawn a new player.

		NewPlayer = Spawn(SpawnClass,,,StartSpot.Location,StartSpot.Rotation);
		if( NewPlayer!=None )
			NewPlayer.ViewRotation = StartSpot.Rotation;
	}

	// Make sure this kind of player is allowed.
	if( bHumansOnly && NewPlayer!=None && !NewPlayer.IsA('Human') )
	{
		NewPlayer.Destroy();
		NewPlayer = None;
	}
	else if( Spectator(NewPlayer) != None )
	{
		if( MaxSpectators <= 0 )
		{
			NewPlayer.Destroy();
			Error="Max spectators exceeded";
			return None;
		}
		if ( Level.NetMode == NM_DedicatedServer )
			MaxSpectators--;
	}

	// If player class not allowed, use default class.
	if( NewPlayer==None && DefaultPlayerClass!=None )
	{
		NewPlayer = Spawn(DefaultPlayerClass,,,StartSpot.Location,StartSpot.Rotation);
		if( NewPlayer!=None )
			NewPlayer.ViewRotation = StartSpot.Rotation;
	}

	// Handle spawn failure.
	if( NewPlayer == None )
	{
		Error = FailedSpawnMessage;
		return None;
	}

	// Init player's administrative privileges
	NewPlayer.Password = InPassword;
	NewPlayer.bAdmin = bAllowRemoteAdmin && InPassword~=AdminPassword;
	if ( NewPlayer.bAdmin )
		log("Administrator logged in");

	// Init player's information.
	NewPlayer.ClientSetRotation(NewPlayer.Rotation);
	if( InName=="" )
		InName=DefaultPlayerName;
	if( Level.NetMode!=NM_Standalone || NewPlayer.PlayerName=="" )
		ChangeName( NewPlayer, InName, false );
	
	// Start player's music.
	NewPlayer.ClientSetMusic( Level.Song, Level.SongSection, Level.CdTrack, MTRAN_Fade );

	// If we are a server, broadcast a welcome message.
	if( Level.NetMode==NM_DedicatedServer || Level.NetMode==NM_ListenServer )
		BroadcastMessage( NewPlayer.PlayerName$EnteredMessage, true );

	// Teleport-in effect.
	StartSpot.PlayTeleportEffect( NewPlayer, true );

	if ( InSkin != "" )
		newPlayer.ServerChangeSkin(InSkin);

	return newPlayer;
}	

//
// Add bot to game.
//
function bool AddBot();

//
// Pawn exits.
//
function Logout( pawn Exiting )
{
	if( (Spectator(Exiting) != None) && (Level.NetMode == NM_DedicatedServer) )
		MaxSpectators++;
	if( Level.NetMode==NM_DedicatedServer || Level.NetMode==NM_ListenServer )
		BroadcastMessage( Exiting.PlayerName$LeftMessage, false );
}

//
// Examine the passed player's inventory, and accept or discard each item.
// AcceptInventory needs to gracefully handle the case of some inventory
// being accepted but other inventory not being accepted (such as the default
// weapon).  There are several things that can go wrong: A weapon's
// AmmoType not being accepted but the weapon being accepted -- the weapon
// should be killed off. Or the player's selected inventory item, active
// weapon, etc. not being accepted, leaving the player weaponless or leaving
// the HUD inventory rendering messed up (AcceptInventory should pick another
// applicable weapon/item as current).
//
event AcceptInventory(pawn PlayerPawn)
{
	//default accept all inventory except default weapon (spawned explicitly)
	//FIXME - fix powerups ,order of respawning default weapon

	local inventory inv;

	// Initialize the inventory.
	AddDefaultInventory( PlayerPawn );

	log( "All inventory from " $ PlayerPawn.PlayerName $ " is accepted" );
}

//
// Spawn any default inventory for the player.
//
function AddDefaultInventory( pawn PlayerPawn )
{
	local Weapon newWeapon;

	if( PlayerPawn.IsA('Spectator') )
		return;

	// Spawn default weapon.
	if( DefaultWeapon==None || PlayerPawn.FindInventoryType(DefaultWeapon)!=None )
		return;
	newWeapon = Spawn(DefaultWeapon,,, Location);
	if( newWeapon != None )
	{
		newWeapon.BecomeItem();
		PlayerPawn.AddInventory(newWeapon);
		newWeapon.BringUp();
		newWeapon.Instigator = PlayerPawn;
		newWeapon.GiveAmmo(PlayerPawn);
		newWeapon.SetSwitchPriority(PlayerPawn);
		newWeapon.WeaponSet(PlayerPawn);
		PlayerPawn.SwitchToBestWeapon(); 
	}
}

//
// Return the 'best' player start for this player to start from.
// Re-implement for each game type.
//
function NavigationPoint FindPlayerStart( optional byte Team, optional string[32] incomingName )
{
	local PlayerStart Dest;
	local Teleporter Tel;
	if( incomingName!="" )
		foreach AllActors( class 'Teleporter', Tel )
			if( string(Tel.Tag)~=incomingName )
				return Tel;
	foreach AllActors( class 'PlayerStart', Dest )
		if( Dest.bSinglePlayerStart )
			return Dest;
	log( "No single player start found" );
	return None;
}

//
// Restart a player.
//
function bool RestartPlayer( pawn aPlayer )	
{
	local NavigationPoint startSpot;
	local bool foundStart;

	if( bRestartLevel && Level.NetMode!=NM_DedicatedServer && Level.NetMode!=NM_ListenServer )
		return true;

	startSpot = FindPlayerStart();
	if( startSpot == None )
		return false;
		
	foundStart = aPlayer.SetLocation(startSpot.Location);
	if( foundStart )
	{
		startSpot.PlayTeleportEffect(aPlayer, true);
		aPlayer.SetRotation(startSpot.Rotation);
		aPlayer.ViewRotation = aPlayer.Rotation;
		aPlayer.Acceleration = vect(0,0,0);
		aPlayer.Velocity = vect(0,0,0);
		aPlayer.Health = aPlayer.Default.Health;
		aPlayer.SetCollision( true, true, true );
		aPlayer.ClientSetRotation( startSpot.Rotation );
		aPlayer.bHidden = false;
		aPlayer.SoundDampening = aPlayer.Default.SoundDampening;
		AddDefaultInventory(aPlayer);
	}
	return foundStart;
}

//
// Start a player.
//
function StartPlayer(PlayerPawn Other)
{
	if( Level.NetMode==NM_DedicatedServer || Level.NetMode==NM_ListenServer || !bRestartLevel )
		Other.GotoState('PlayerWalking');
	else
		Other.ClientTravel( "?restart", TRAVEL_Relative, false );
}

//------------------------------------------------------------------------------
// Level death message functions.

//
// Display a death message.
//
function Killed( pawn killer, pawn Other, name damageType )
{
	local string[32] killed;
	local string[64] message;

	Other.DieCount++;
	if (Other.bIsPlayer)
	{
		killed = Other.PlayerName;
		if( killer==None || killer==other )
			message = KillMessage(damageType, Other);
		else 
			message = killer.KillMessage(damageType, Other);
		BroadcastMessage(killed$message, false);
	}

	if( killer == Other )
	{
		Other.Score -= 1;
	}
	else if( killer != None )
	{
		killer.killCount++;
		killer.Score += 1;
	}
}	

//
// Default death message.
//
function string[64] KillMessage( name damageType, pawn Other )
{
	return "died";
}

//-------------------------------------------------------------------------------------
// Level gameplay modification.

//
// Return whether Viewer is allowed to spectate from the
// point of view of ViewTarget.
//
function bool CanSpectate( pawn Viewer, actor ViewTarget )
{
	return true;
}

//
// Use reduce damage for teamplay modifications, etc.
//
function int ReduceDamage( int Damage, name DamageType, pawn injured, pawn instigatedBy )
{
	if( injured.Region.Zone.bNeutralZone )
		return 0;	
	return Damage;
}

//
// Award a score to an actor.
//
function ScoreEvent( name EventName, actor EventActor, pawn InstigatedBy )
{
}

//
// Return whether an item should respawn.
//
function bool ShouldRespawn( actor Other )
{
	if( Level.NetMode == NM_StandAlone )
		return false;
	return Inventory(Other)!=None && Inventory(Other).ReSpawnTime!=0.0;
}

//
// Called when pawn has a chance to pick Item up (i.e. when 
// the pawn touches a weapon pickup). Should return true if 
// he wants to pick it up, false if he does not want it.
//
function bool PickupQuery( Pawn Other, Inventory item )
{
	if ( Other.Inventory == None )
		return true;
	if ( bCoopWeaponMode && item.IsA('Weapon') && !Weapon(item).bHeldItem && (Other.FindInventoryType(item.class) != None) )
		return false;
	else
		return !Other.Inventory.HandlePickupQuery(Item);
}
		
//
// Discard a player's inventory after he dies.
//
function DiscardInventory( Pawn Other )
{
	local actor dropped;
	local inventory Inv;
	local weapon weap;
	local float speed;

	if( Other.DropWhenKilled != None )
	{
		dropped = Spawn(Other.DropWhenKilled,,,Other.Location);
		Inv = Inventory(dropped);
		if ( Inv != None )
		{ 
			Inv.RespawnTime = 0.0; //don't respawn
			Inv.BecomePickup();		
		}
		if ( dropped != None )
		{
			dropped.RemoteRole = ROLE_DumbProxy;
			dropped.SetPhysics(PHYS_Falling);
			dropped.bCollideWorld = true;
			dropped.Velocity = Other.Velocity + VRand() * 280;
		}
		if ( Inv != None )
			Inv.GotoState('PickUp', 'Dropped');
	}					
	if( Other.Weapon!=None && Other.Weapon.Class!=DefaultWeapon )
	{
		speed = VSize(Other.Velocity);
		weap = Other.Weapon;
		weap.Velocity = Normal(Other.Velocity/speed + 0.5 * VRand()) * (speed + 280);
		Other.TossWeapon();
		if ( weap.PickupAmmoCount == 0 )
			weap.PickupAmmoCount = 1;
		Other.Weapon = None;
	}
	Other.SelectedItem = None;	
	for( Inv=Other.Inventory; Inv!=None; Inv=Inv.Inventory )
		Inv.Destroy();
}

//
// Try to change a player's name.
//	
function ChangeName( Pawn Other, coerce string[20] S, bool bNameChange )
{
	if( S == "" )
		return;
	Other.PlayerName = S;
	if( bNameChange )
		Other.ClientMessage( NameChangedMessage $ Other.PlayerName );
}

//
// Return whether a team change is allowed.
//
function bool ChangeTeam(Pawn Other, coerce string[20] S)
{
	return true;
}

//
// Play an inventory respawn effect.
//
function PlaySpawnEffect( inventory Inv );

//
// Generate a player killled message.
//
function string[64] PlayerKillMessage(name damageType, pawn Other)
{
	local string[64] message;

	message = " was killed by ";
	return message;
}

//
// Generate a killed by creature message.
//
function string[64] CreatureKillMessage(name damageType, pawn Other)
{
	return " was killed by a ";
}

//
// Send a player to a URL.
//
function SendPlayer(PlayerPawn aPlayer, string[64] URL)
{
	aPlayer.ClientTravel( URL, TRAVEL_Relative, true );
}

//
// Play a teleporting special effect.
//
function PlayTeleportEffect( actor Incoming, bool bOut, bool bSound);

//
// Restart the game.
//
function RestartGame()
{
	Level.ServerTravel( "?Restart", false );
}

//
// Whether players are allowed to broadcast messages now.
//
function bool AllowsBroadcast( actor broadcaster, int Len )
{
	SentText += Len;

	return (SentText < 260);
}

//
// End of game.
//
function EndGame()
{
	local actor A;
	local pawn aPawn;

	foreach AllActors(class'Actor', A, 'EndGame')
		A.trigger(self, none);

	aPawn = Level.PawnList;
	while( aPawn != None )
	{
		if ( aPawn.bIsPlayer )
		{
			aPawn.GotoState('GameEnded');
			aPawn.ClientGameEnded();
		}	
		aPawn = aPawn.NextPawn;
	}
}
	
defaultproperties
{
     Difficulty=1
     AutoAim=+00000.930000
     GameSpeed=+00001.000000
	 bRestartLevel=True
	 bPauseable=True
	 bCanChangeSkin=True
	 MapPrefix=""
	 MaxSpectators=2
	 SwitchLevelMessage="Switching Levels"
	 DefaultPlayerName="Player"
	 FailedSpawnMessage="Failed to spawn player actor"
	 FailedPlaceMessage="Could not find starting spot (level might need a 'PlayerStart' actor)"
	 LeftMessage=" left the game."
	 EnteredMessage=" entered the game."
	 NameChangedMessage="Name changed to "
}
