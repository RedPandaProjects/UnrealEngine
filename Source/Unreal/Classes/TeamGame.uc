//=============================================================================
// TeamGame.
//=============================================================================
class TeamGame expands DeathMatchGame
	localized
	config;

var() config bool   bSpawnInTeamArea;
var() config float  FriendlyFireScale; //scale friendly fire damage by this value
var() config int	MaxTeams; //Maximum number of teams allowed in (up to 16)
var	TeamInfo Teams[16]; 
var() config float  GoalTeamScore; //like fraglimit
var() config int	MaxTeamSize;
var  localized string[128] NewTeamMessage;
var		int			NextBotTeam;
var bool		bBlueTaken, bRedTaken, bYellowTaken, bGreenTaken;

function PostBeginPlay()
{
	local int i;
	for (i=0;i<MaxTeams;i++)
	{
		Teams[i] = Spawn(class'TeamInfo');
		Teams[i].Size = 0;
		Teams[i].Score = 0;
	}
	
	Super.PostBeginPlay();
}

//------------------------------------------------------------------------------
// Player start functions


//FindPlayerStart
//- add teamnames as new teams enter
//- choose team spawn point if bSpawnInTeamArea

function playerpawn Login
(
	string[32] Portal,
	string[120] Options,
	out string[80 ] Error,
	class<playerpawn> SpawnClass
)
{
	local PlayerPawn newPlayer;
	local string[64]      InTeam;

	newPlayer = Super.Login(Portal, Options, Error, SpawnClass);
	if ( newPlayer == None)
		return None;

	InTeam     = ParseOption( Options, "Team"     );
	if ( !ChangeTeam(newPlayer, InTeam) )
	{
		Error = "Could not find team for player";
		return None;
	}
		
	return newPlayer;
}

function Logout(pawn Exiting)
{
	Teams[Exiting.Team].Size--;
	Super.Logout(Exiting);
}
	
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
		if ( N.IsA('PlayerStart') )
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
			if ( !bSpawnInTeamArea || (Team == Dest.TeamNumber) )
			{  
				if (num<4)
					Candidate[num] = Dest;
				else if (Rand(num) < 4)
					Candidate[Rand(4)] = Dest;
				num++;
			}
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
		{
			for (i=0;i<num;i++)
			{
				NextDist = VSize(OtherPlayer.Location - Candidate[i].Location);
				Score[i] += NextDist;
				if (NextDist < CollisionRadius + CollisionHeight)
					Score[i] -= 1000000.0;
				else if ( (NextDist < 1400) && (Team != OtherPlayer.Team) && OtherPlayer.LineOfSightTo(Candidate[i]) )
					Score[i] -= 2000.0;
			}
		}
		OtherPlayer = OtherPlayer.NextPawn;
	}
	
	BestScore = Score[0];
	Best = Candidate[0];
	for (i=1;i<num;i++)
	{
		if (Score[i] > BestScore)
		{
			BestScore = Score[i];
			Best = Candidate[i];
		}
	}			
	
	//if (BestScore < -500000.0) //then would telefrag - but then how to handle re-try?
	//	return None;
				
	return Best;
}

function bool AddBot()
{
	local NavigationPoint StartSpot;
	local bots NewBot;
	local int BotN;
	local string[20] DesiredTeam;

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

	DesiredTeam = BotConfig.GetBotTeam(BotN);
	if ( (DesiredTeam == "") || !ChangeTeam(NewBot, DesiredTeam) )
	{
		ChangeTeam(NewBot, Teams[NextBotTeam].TeamName);
		NextBotTeam++;
		if ( NextBotTeam >= MaxTeams )
			NextBotTeam = 0;
	}

	return true;
}

//-------------------------------------------------------------------------------------
// Level gameplay modification

//Use reduce damage for teamplay modifications, etc.
function int ReduceDamage(int Damage, name DamageType, pawn injured, pawn instigatedBy)
{
	local int reducedDamage;

	if (injured.Region.Zone.bNeutralZone)
		return 0;
	
	if ( instigatedBy == None )
		return Damage;

	Damage *= instigatedBy.DamageScaling;

	if ( (instigatedBy != injured) 
		&& (injured.Team ~= instigatedBy.Team) )
		return (Damage * FriendlyFireScale);
	else
		return Damage;
}

function Killed(pawn killer, pawn Other, name damageType)
{
	Super.Killed(killer, Other, damageType);

	if ( (killer != Other) && (killer != None) )
		Teams[killer.Team].Score += 1.0;

	if ( (GoalTeamScore > 0) && (Teams[killer.Team].Score >= GoalTeamScore) )
		EndGame();
}

function bool ChangeTeam(Pawn Other, coerce string[20] PlayerTeam)
{
	local int i, s;
	local pawn APlayer;
	local teaminfo SmallestTeam;

	i = 0;
	SmallestTeam = None;

	while (i<MaxTeams)
	{
		if ( (Teams[i].Size < MaxTeamSize) 
				&& ((SmallestTeam == None) || (SmallestTeam.Size > Teams[i].Size)) )
		{
			s = i;
			SmallestTeam = Teams[i];
		}

		if ( (Teams[i].TeamName ~= PlayerTeam) || (Teams[i].Size == 0) )
		{
			if (Teams[i].Size < MaxTeamSize)
			{
				if ( PlayerTeam ~= "" )
					PlayerTeam = ("Team"$i);
				Teams[i].TeamName = PlayerTeam;
				ValidateTeamName(Teams[i], Other);
				Teams[i].Size++;
				Other.Team = i;
				Other.TeamName = Teams[i].TeamName;
				BroadcastMessage(Other.PlayerName$NewTeamMessage$Teams[i].TeamName, false);
				return true;
			}
			if ( (PlayerTeam ~= "") && (SmallestTeam != None) )
			{
				SmallestTeam.Size++;
				Other.Team = s;
				ValidateTeamName(SmallestTeam, Other);
				Other.TeamName = SmallestTeam.TeamName;
				BroadcastMessage(Other.PlayerName$NewTeamMessage$SmallestTeam.TeamName, false);
				return true;
			}
		}
		i++;
	}
	if ( SmallestTeam.Size < MaxTeamSize )
	{
		SmallestTeam.Size++;
		Other.Team = s;
		ValidateTeamName(SmallestTeam, Other);
		Other.TeamName = SmallestTeam.TeamName;
		BroadcastMessage(Other.PlayerName$NewTeamMessage$SmallestTeam.TeamName, false);
		return true;
	}

	return false;
}

function ValidateTeamName( teaminfo aTeam, Pawn Other )
{
	local texture NewSkin;
	local string[20] OldTeamName;

	OldTeamName = aTeam.TeamName;
	NewSkin = texture(DynamicLoadObject(string(Other.Mesh)$"Skins.T_"$aTeam.TeamName, class'Texture'));
	log("NewSkin is "$NewSkin);

	if ( (NewSkin == None) && !bRedTaken )
	{
		aTeam.TeamName = "Red";
		NewSkin = texture(DynamicLoadObject(string(Other.Mesh)$"Skins.T_"$aTeam.TeamName, class'Texture'));
		bRedTaken = (NewSkin != None);
	}
	if ( (NewSkin == None) && !bBlueTaken )
	{
		aTeam.TeamName = "Blue";
		NewSkin = texture(DynamicLoadObject(string(Other.Mesh)$"Skins.T_"$aTeam.TeamName, class'Texture'));
		bBlueTaken = (NewSkin != None);
	}
	if ( (NewSkin == None) && !bGreenTaken )
	{
		aTeam.TeamName = "Green";
		NewSkin = texture(DynamicLoadObject(string(Other.Mesh)$"Skins.T_"$aTeam.TeamName, class'Texture'));
		bGreenTaken = (NewSkin != None);
	}
	if ( (NewSkin == None) && !bYellowTaken )
	{
		aTeam.TeamName = "Yellow";
		NewSkin = texture(DynamicLoadObject(string(Other.Mesh)$"Skins.T_"$aTeam.TeamName, class'Texture'));
		bYellowTaken = (NewSkin != None);
		if ( !bYellowTaken )
			aTeam.TeamName = OldTeamName;
	}

	if ( NewSkin != None )
		Other.Skin = NewSkin;
}

function bool CanSpectate( pawn Viewer, actor ViewTarget )
{
	return ( (Spectator(Viewer) != None) 
			|| ((Pawn(ViewTarget) != None) && (Pawn(ViewTarget).Team == Viewer.Team)) );
}

defaultproperties
{
     MaxTeams=2
     MaxTeamSize=16
	 bCanChangeSkin=False
	 bHumansOnly=true
	 GameMenuType=UnrealTeamGameOptionsMenu
	 NewTeamMessage=" is now on "
     HUDType=Class'Unreal.UnrealTeamHUD'
}
