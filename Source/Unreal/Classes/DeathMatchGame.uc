//=============================================================================
// DeathMatchGame.
//=============================================================================
class DeathMatchGame expands UnrealGameInfo
	localized
	config;

var() globalconfig int	MaxPlayers; 
var   int			NumPlayers;
var() globalconfig int	FragLimit; 
var() globalconfig int	TimeLimit; // time limit in minutes
var() globalconfig bool	bMultiPlayerBots;
var() globalconfig bool bChangeLevels;
var		bool	bGameEnded;
var		bool	bAlreadyChanged;

var	  int RemainingTime;

// Bot related info
var   int			NumBots;
var	  int			RemainingBots;
var() globalconfig int	InitialBots;
var		BotInfo		BotConfig;
var localized string[255] GlobalNameChange;
var localized string[255] NoNameChange;
var		localized string[128] MaxedOutMessage;

function PostBeginPlay()
{
	BotConfig = spawn(class'BotInfo');
	RemainingTime = 60 * TimeLimit;
	if ( (Level.NetMode == NM_Standalone) || bMultiPlayerBots )
		RemainingBots = InitialBots;
	Super.PostBeginPlay();
}

function ForceMeshesToExist()
{
	//to avoid dynamic load pauses
	// this function never called
	spawn(class'FemaleOne');
	spawn(class'MaleThree');
	spawn(class'FemaleOneBot');
	spawn(class'MaleThreeBot');

	if ( !bShareware )
	{
		spawn(class'FemaleTwo');
		spawn(class'MaleOne');
		spawn(class'MaleTwo');
		spawn(class'SkaarjPlayer');
		spawn(class'FemaleTwoBot');
		spawn(class'MaleOneBot');
		spawn(class'MaleTwoBot');
		spawn(class'SkaarjPlayerBot');
	}
}

function int ReduceDamage(int Damage, name DamageType, pawn injured, pawn instigatedBy)
{
	if (injured.Region.Zone.bNeutralZone)
		return 0;

	if ( instigatedBy == None)
		return Damage;
	//skill level modification
	if ( (instigatedBy.Skill < 1.5) && instigatedBy.IsA('Bot') && injured.IsA('PlayerPawn') )
		Damage = Damage * (0.7 + 0.15 * instigatedBy.skill);

	return (Damage * instigatedBy.DamageScaling);
}

function PlaySpawnEffect(inventory Inv)
{
	if ( !bCoopWeaponMode || !Inv.IsA('Weapon') )
		spawn( class 'ReSpawn',,, Inv.Location );
}

// Return beacon text for server
event GetBeaconText( out string[240] Result )
{
	Super.GetBeaconText( Result );
	Result = Result $ " " $ NumPlayers $ "/" $ MaxPlayers;
}

function RestartGame()
{
	local string[32] NextMap;
	local MapList myList;

	log("restart game");

	// these server travels should all be relative to the current URL
	if ( bChangeLevels && !bAlreadyChanged && (MapListType != None) )
	{
		// open a the nextmap actor for this game type and get the next map
		bAlreadyChanged = true;
		myList = spawn(MapListType);
		NextMap = myList.GetNextMap();
		myList.Destroy();
		if ( NextMap == "" )
			NextMap = GetMapName(MapPrefix, NextMap,1);
		if ( NextMap != "" )
		{
			log("Changing to "$NextMap);
			Level.ServerTravel(NextMap, false);
			return;
		}
	}

	Level.ServerTravel("?Restart" , false);
}

event playerpawn PreLogin
(
	string[120] Options,
	out string[80] Error
)
{
	Super.PreLogin(Options, Error);

	// Do any name or password or name validation here.
	if ( (MaxPlayers > 0) && (NumPlayers >= MaxPlayers) )
		Error=MaxedOutMessage;
}

event playerpawn Login
(
	string[32] Portal,
	string[120] Options,
	out string[80] Error,
	class<playerpawn> SpawnClass
)
{
	local playerpawn NewPlayer;

	if ( (MaxPlayers > 0) && (NumPlayers >= MaxPlayers) )
	{
		Error = MaxedOutMessage;
		return None;
	}

	NewPlayer = Super.Login(Portal, Options, Error, SpawnClass );
	if ( NewPlayer != None )
	{
		NumPlayers++;
		if ( Left(NewPlayer.PlayerName, 6) == DefaultPlayerName )
			ChangeName( NewPlayer, (DefaultPlayerName$NumPlayers), false );
	}
	NewPlayer.bAutoActivate = true;

	return NewPlayer;
}

function bool AddBot()
{
	local NavigationPoint StartSpot;
	local bots NewBot;
	local int BotN;

	BotN = BotConfig.ChooseBotInfo();
	
	// Find a start spot.
	StartSpot = FindPlayerStart();
	if( StartSpot == None )
	{
		log("Could not find starting spot for Bot");
		return false;
	}

	// Try to spawn the player.
	NewBot = Spawn(BotConfig.GetBotClass(BotN),,,StartSpot.Location,StartSpot.Rotation);

	if ( (NewBot == None) || (bHumansOnly && !NewBot.IsA('HumanBot')) )
	{
		NewBot.Destroy();
		log("Failed to spawn bot");
		return false;
	}

	StartSpot.PlayTeleportEffect(NewBot, true);

	// Init player's information.
	BotConfig.Individualize(NewBot, BotN, NumBots);
	NewBot.ViewRotation = StartSpot.Rotation;

	// broadcast a welcome message.
	BroadcastMessage( NewBot.PlayerName$EnteredMessage, true );

	AddDefaultInventory( NewBot );
	NumBots++;
	return true;
}

function Logout(pawn Exiting)
{
	Super.Logout(Exiting);
	if ( Exiting.IsA('PlayerPawn') )
		NumPlayers--;
	else if ( Exiting.IsA('Bots') )
		NumBots--;
}
	
function Timer()
{
	Super.Timer();

	if ( (RemainingBots > 0) && AddBot() )
		RemainingBots--;

	if ( TimeLimit > 0 )
	{
		RemainingTime--;
		if ( bGameEnded )
		{
			if ( RemainingTime < -7 )
				RestartGame();
		}
		else if ( RemainingTime <= 0 )
			EndGame();
	}
}

/* FindPlayerStart()
returns the 'best' player start for this player to start from.
Re-implement for each game type
*/
function NavigationPoint FindPlayerStart(optional byte Team, optional string[32] incomingName)
{
	local PlayerStart Dest, Candidate[4], Best;
	local float Score[4], BestScore, NextDist;
	local pawn OtherPlayer;
	local int i, num;
	local Teleporter Tel;
	local NavigationPoint N;

	if( incomingName!="" )
		foreach AllActors( class 'Teleporter', Tel )
			if( string(Tel.Tag)~=incomingName )
				return Tel;

	num = 0;
	//choose candidates	
	N = Level.NavigationPointList;
	While ( N != None )
	{
		if ( N.IsA('PlayerStart') && !N.Region.Zone.bWaterZone )
		{
			if (num<4)
				Candidate[num] = PlayerStart(N);
			else if (Rand(num) < 4)
				Candidate[Rand(4)] = PlayerStart(N);
			num++;
		}
		N = N.nextNavigationPoint;
	}

	if (num == 0 )
		foreach AllActors( class 'PlayerStart', Dest )
		{
			if (num<4)
				Candidate[num] = Dest;
			else if (Rand(num) < 4)
				Candidate[Rand(4)] = Dest;
			num++;
		}

	if (num>4) num = 4;
	else if (num == 0)
		return None;
		
	//assess candidates
	for (i=0;i<num;i++)
		Score[i] = 4000 * FRand(); //randomize
		
	OtherPlayer = Level.PawnList;
	while ( OtherPlayer != None )
	{
		if (OtherPlayer.bIsPlayer)
			for (i=0;i<num;i++)
			{
				NextDist = VSize(OtherPlayer.Location - Candidate[i].Location);
				Score[i] += NextDist;
				if (NextDist < OtherPlayer.CollisionRadius + OtherPlayer.CollisionHeight)
					Score[i] -= 1000000.0;
				else if ( (NextDist < 1400) && OtherPlayer.LineOfSightTo(Candidate[i]) )
					Score[i] -= 2000.0;
			}
		OtherPlayer = OtherPlayer.NextPawn;
	}
	
	BestScore = Score[0];
	Best = Candidate[0];
	for (i=1;i<num;i++)
		if (Score[i] > BestScore)
		{
			BestScore = Score[i];
			Best = Candidate[i];
		}

	return Best;
}

/* AcceptInventory()
Examine the passed player's inventory, and accept or discard each item
* AcceptInventory needs to gracefully handle the case of some inventory
being accepted but other inventory not being accepted (such as the default
weapon).  There are several things that can go wrong: A weapon's
AmmoType not being accepted but the weapon being accepted -- the weapon
should be killed off. Or the player's selected inventory item, active
weapon, etc. not being accepted, leaving the player weaponless or leaving
the HUD inventory rendering messed up (AcceptInventory should pick another
applicable weapon/item as current).
*/
function AcceptInventory(pawn PlayerPawn)
{
	//deathmatch accepts no inventory
	local inventory Inv;
	for( Inv=PlayerPawn.Inventory; Inv!=None; Inv=Inv.Inventory )
		Inv.Destroy();
	PlayerPawn.Weapon = None;
	PlayerPawn.SelectedItem = None;
	AddDefaultInventory( PlayerPawn );
}

function ChangeName(Pawn Other, coerce string[20] S, bool bNameChange)
{
	local pawn APlayer;

	if ( S == "" )
		return;

	if (Other.PlayerName~=S)
		return;
	
	APlayer = Level.PawnList;
	
	While ( APlayer != None )
	{	
		if ( APlayer.bIsPlayer && (APlayer.PlayerName~=S) )
		{
			Other.ClientMessage(S$NoNameChange);
			return;
		}
		APlayer = APlayer.NextPawn;
	}

	if (bNameChange)
		BroadcastMessage(Other.PlayerName$GlobalNameChange$S, false);
			
	Other.PlayerName = S;
}

function bool ShouldRespawn(Actor Other)
{
	if ( bCoopWeaponMode && Other.IsA('Weapon') 
	&& !Inventory(Other).bHeldItem &&  (Inventory(Other).ReSpawnTime != 0.0) )
	{
		Inventory(Other).ReSpawnTime = 1.0;
		return true;
	}
	return ( (Inventory(Other) != None) && (Inventory(Other).ReSpawnTime!=0.0) );
}

function bool CanSpectate( pawn Viewer, actor ViewTarget )
{
	return ( (Level.NetMode == NM_Standalone) || (Spectator(Viewer) != None) );
}

// Monitor killed messages for fraglimit
function Killed(pawn killer, pawn Other, name damageType)
{
	Super.Killed(killer, Other, damageType);

	if ( (killer == None) || (Other == None) )
		return;
	if ( (FragLimit > 0) && (killer.Score >= FragLimit) )
		EndGame();

	if ( BotConfig.bAdjustSkill && (killer.IsA('PlayerPawn') || Other.IsA('PlayerPawn')) )
	{
		if ( killer.IsA('Bots') )
			Bots(killer).AdjustSkill(true);
		if ( Other.IsA('Bots') )
			Bots(Other).AdjustSkill(false);
	}
}	

function EndGame()
{
	local actor A;
	local pawn aPawn;

	Super.EndGame();

	bGameEnded = true;
	aPawn = Level.PawnList;
	TimeLimit = 1;
	RemainingTime = -1; // use timer to force restart
	while( aPawn != None )
	{
		if ( aPawn.IsA('Bots') )
			aPawn.GotoState('GameEnded');
		aPawn = aPawn.NextPawn;
	}
}

defaultproperties
{
     MaxPlayers=10
     bChangeLevels=True
     InitialBots=4
     bNoMonsters=True
     bRestartLevel=False
     bPauseable=False
     AutoAim=1.000000
     DefaultPlayerClass=Class'Unreal.MaleOne'
     ScoreBoardType=Class'Unreal.UnrealScoreBoard'
     GameMenuType=Class'Unreal.UnrealDMGameOptionsMenu'
     MapListType=Class'Unreal.DMmaplist'
     MapPrefix="DM"
     BeaconName="DM"
	 NoNameChange=" is already in use"
	 GlobalNameChange=" changed name to "
	 MaxedOutMessage="Server is already at capacity."
}
