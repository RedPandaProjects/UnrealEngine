//=============================================================================
// CoopGame.
//=============================================================================
class CoopGame expands UnrealGameInfo
	config;

var() config bool	bNoFriendlyFire;

function bool IsRelevant(actor Other)
{
	// hide all playerpawns

	if ( Other.IsA('PlayerPawn') )
	{
		Other.SetCollision(false,false,false);
		Other.bHidden = true;
	}
	return Super.IsRelevant(Other);
}

function PlaySpawnEffect(inventory Inv)
{
	Playsound(sound'RespawnSound');
	if ( !bCoopWeaponMode || !Inv.IsA('Weapon') )
		spawn( class 'ReSpawn',,, Inv.Location );
}

event playerpawn Login
(
	string[32] Portal,
	string[120] Options,
	out string[80] Error,
	class<playerpawn> SpawnClass
)
{
	local PlayerPawn      NewPlayer;
	local string[64]      InName, InPassword;
	local pawn			  aPawn;

	NewPlayer =  Super.Login(Portal, Options, Error, SpawnClass);
	if ( NewPlayer != None )
	{
		NewPlayer.bHidden = false;
		NewPlayer.SetCollision(true,true,true);
	}
	return NewPlayer;
}
	
function NavigationPoint FindPlayerStart(optional byte team, optional string[32] incomingName)
{
	local PlayerStart Dest, Candidate[8], Best;
	local float Score[8], BestScore, NextDist;
	local pawn OtherPlayer;
	local int i, num;
	local Teleporter Tel;

	num = 0;
	//choose candidates	
	foreach AllActors( class 'PlayerStart', Dest )
	{
		if ( (Dest.bSinglePlayerStart || Dest.bCoopStart) && !Dest.Region.Zone.bWaterZone )
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
		
	foreach AllActors( class 'Pawn', OtherPlayer )
	{
		if (OtherPlayer.bIsPlayer)
		{
			for (i=0;i<num;i++)
			{
				NextDist = VSize(OtherPlayer.Location - Candidate[i].Location);
				Score[i] += NextDist;
				if (NextDist < OtherPlayer.CollisionRadius + OtherPlayer.CollisionHeight)
					Score[i] -= 1000000.0;
			}
		}
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
				
	return Best;
}

function int ReduceDamage(int Damage, name DamageType, pawn injured, pawn instigatedBy)
{
	if ( bNoFriendlyFire && (instigatedBy != None) 
		&& instigatedBy.bIsPlayer && injured.bIsPlayer && (instigatedBy != injured) )
		return 0;

	return Super.ReduceDamage(Damage, DamageType, injured, instigatedBy);
}

function bool ShouldRespawn(Actor Other)
{
	if ( Other.IsA('Weapon') && !Weapon(Other).bHeldItem )
	{
		Inventory(Other).ReSpawnTime = 1.0;
		return true;
	}
	return false;
}

function SendPlayer(PlayerPawn aPlayer, string[64] URL)
{
	Level.ServerTravel( URL, true );
}

function PlayTeleportEffect( actor Incoming, bool bOut, bool bSound)
{
}

function Killed(pawn killer, pawn Other, name damageType)
{
	super.Killed(killer, Other, damageType);
	if ( Other.bIsPlayer || Other.IsA('Nali') )
		killer.Score -= 2;
}	

defaultproperties
{
     bNoFriendlyFire=True
     bHumansOnly=True
     bRestartLevel=False
     bCoopWeaponMode=True
     bPauseable=False
     BeaconName="Coop"
}
