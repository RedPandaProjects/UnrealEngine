//=============================================================================
// UnrealGameInfo.
//
// default game info is normal single player
//
//=============================================================================
class UnrealGameInfo expands GameInfo
	localized;

#exec AUDIO IMPORT FILE="Sounds\Generic\land1.WAV" NAME="Land1" GROUP="Generic"
#exec AUDIO IMPORT FILE="Sounds\Generic\lsplash.WAV" NAME="LSplash" GROUP="Generic"
#exec AUDIO IMPORT FILE="Sounds\Generic\dsplash.WAV" NAME="DSplash" GROUP="Generic"
#exec AUDIO IMPORT FILE="Sounds\Generic\wtrexit1.WAV" NAME="WtrExit1" GROUP="Generic"
#exec AUDIO IMPORT FILE="Sounds\pickups\genwep1.WAV" NAME="WeaponPickup" GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Generic\teleport1.WAV" NAME="Teleport1" GROUP="Generic"

var(DeathMessage) localized string[32]   DeathMessage[32];    // Player name, or blank if none.
var(DeathMessage) localized string[32]   DeathModifier[5];
var(DeathMessage) localized string[128]	 MajorDeathMessage[8];
var(DeathMessage) localized string[32]	 HeadLossMessage[2];
var(DeathMessage) localized string[32]	 DeathVerb;
var(DeathMessage) localized string[32]	 DeathPrep;
var(DeathMessage) localized string[32]	 DeathTerm;
var(DeathMessage) localized string[32]	 DeathArticle;
var(DeathMessage) localized string[32]	 ExplodeMessage;
var(DeathMessage) localized string[32]	 SuicideMessage;
var(DeathMessage) localized string[32]	 FallMessage;
var(DeathMessage) localized string[32]	 DrownedMessage;
var(DeathMessage) localized string[32]	 BurnedMessage;
var(DeathMessage) localized string[32]	 CorrodedMessage;
var(DeathMessage) localized string[32]	 HackedMessage;

function int ReduceDamage(int Damage, name DamageType, pawn injured, pawn instigatedBy)
{
	if (injured.Region.Zone.bNeutralZone)
		return 0;

	if ( instigatedBy == None)
		return Damage;
	//skill level modification
	if ( instigatedBy.bIsPlayer )
	{
		if ( injured == instigatedby )
		{ 
			if ( instigatedby.skill == 0 )
				Damage = 0.25 * Damage;
			else if ( instigatedby.skill == 1 )
				Damage = 0.5 * Damage;
		}
		else if ( !injured.bIsPlayer )
			Damage = float(Damage) * (1.1 - 0.1 * injured.skill);
	}
	else if ( injured.bIsPlayer )
		Damage = Damage * (0.4 + 0.2 * instigatedBy.skill);
	return (Damage * instigatedBy.DamageScaling);
}

function PlaySpawnEffect(inventory Inv)
{
	spawn( class 'ReSpawn',,, Inv.Location );
}

function bool ShouldRespawn(Actor Other)
{
	return false;
}

function string[64] KillMessage(name damageType, pawn Other)
{
	local string[64] message;
	
	if (damageType == 'exploded')
		message = ExplodeMessage;
	else if (damageType == 'suicided')
		message = SuicideMessage;
	else if ( damageType == 'fell' )
		message = FallMessage;
	else if ( damageType == 'drowned' )
		message = DrownedMessage;
	else if ( damageType == 'Special' )
		message = SpecialDamageString;
	else if ( damageType == 'Burned' )
		message = BurnedMessage;
	else if ( damageType == 'Corroded' )
		message = CorrodedMessage;
	else
		message = DeathVerb$DeathTerm;
		
	return message;	
}

function string[64] CreatureKillMessage(name damageType, pawn Other)
{
	local string[64] message;
	
	if (damageType == 'exploded')
		message = ExplodeMessage;
	else if ( damageType == 'Burned' )
		message = BurnedMessage;
	else if ( damageType == 'Corroded' )
		message = CorrodedMessage;
	else if ( damageType == 'Hacked' )
		message = HackedMessage;
	else
		message = DeathVerb$DeathTerm;

	return ( message$DeathPrep$DeathArticle );
}

function string[64] PlayerKillMessage(name damageType, pawn Other)
{
	local string[64] message;
	local float decision;
	
	decision = FRand();

	if ( (Other.Health < - 25) && (decision < 0.3) )
	{
		decision = FRand();
		if ( decision < 0.35 )
			message = MajorDeathMessage[0];
		else if ( decision < 0.7 )
			message = MajorDeathMessage[1];
		else 
			message = MajorDeathMessage[2];
	}	
	else
	{
		if ( DamageType == 'Decapitated' )
		{
			if ( FRand() < 0.4 )
				message = HeadLossMessage[1];
			else
				message = HeadLossMessage[0];
		}
		else
			message = DeathMessage[Rand(32)];

		decision = Other.Health * FRand();
		if ( decision < -8 )
			message = DeathModifier[0]$message;
		else if ( decision < -15 )
			message = DeathModifier[1]$message;
		else if ( decision < -22 )
			message = DeathModifier[2]$message;
		else if ( decision < -30 )
			message = DeathModifier[3]$message;
		else if ( decision < -35 )
			message = DeathModifier[4]$message;
	}	
	
	return ( DeathVerb$message$DeathPrep );
} 	

function PlayTeleportEffect( actor Incoming, bool bOut, bool bSound)
{
	local PawnTeleportEffect PTE;

	if ( Incoming.IsA('Pawn') )
	{
		if ( bSound )
		{
			PTE = Spawn(class'PawnTeleportEffect',,, Incoming.Location, Incoming.Rotation);
			PTE.Initialize(Pawn(Incoming), bOut);
			if ( Incoming.IsA('PlayerPawn') )
				PlayerPawn(Incoming).SetFOVAngle(170);
				Incoming.PlaySound(sound'Teleport1',, 10.0);
		}
	}
}

defaultproperties
{
     deathmessage(0)="killed"
     deathmessage(1)="ruled"
     deathmessage(2)="smoked"
     deathmessage(3)="slaughtered"
     deathmessage(4)="annihilated"
     deathmessage(5)="put down"
     deathmessage(6)="splooged"
     deathmessage(7)="perforated"
     deathmessage(8)="shredded"
     deathmessage(9)="destroyed"
     deathmessage(10)="whacked"
     deathmessage(11)="canned"
     deathmessage(12)="busted"
     deathmessage(13)="creamed"
     deathmessage(14)="smeared"
     deathmessage(15)="shut out"
     deathmessage(16)="beaten down"
     deathmessage(17)="smacked down"
     deathmessage(18)="pureed"
     deathmessage(19)="sliced"
     deathmessage(20)="diced"
     deathmessage(21)="ripped"
     deathmessage(22)="blasted"
     deathmessage(23)="torn up"
     deathmessage(24)="spanked"
     deathmessage(25)="eviscerated"
     deathmessage(26)="neutered"
     deathmessage(27)="whipped"
     deathmessage(28)="shafted"
     deathmessage(29)="trashed"
     deathmessage(30)="smashed"
     deathmessage(31)="trounced"
     DeathModifier(0)="thoroughly "
     DeathModifier(1)="completely "
     DeathModifier(2)="absolutely "
     DeathModifier(3)="totally "
     DeathModifier(4)="utterly "
     MajorDeathMessage(0)="ripped a new one"
     MajorDeathMessage(1)="messed up real bad"
     MajorDeathMessage(2)="given a new definition of pain"
     HeadLossMessage(0)="decapitated"
     HeadLossMessage(1)="beheaded"
     DeathVerb=" was "
     DeathPrep=" by "
     DeathTerm="killed"
     DeathArticle="a "
     ExplodeMessage=" was blown up"
     SuicideMessage=" had a sudden heart attack."
     FallMessage=" left a small crater."
     DrownedMessage=" forgot to come up for air."
	 BurnedMessage=" was incinerated"
	 CorrodedMessage=" was slimed"
	 HackedMessage=" was hacked"
     DefaultWeapon=Class'Unreal.DispersionPistol'
     GameMenuType=Class'Unreal.UnrealGameOptionsMenu'
     HUDType=Class'Unreal.UnrealHUD'
     DefaultWaterEntryActor=Class'Unreal.WaterImpact'
     DefaultWaterExitActor=Class'Unreal.WaterImpact'
     DefaultWaterEntrySound=Sound'Unreal.Generic.DSplash'
     DefaultWaterExitSound=Sound'Unreal.Generic.WtrExit1'
	 DefaultPlayerClass=Class'Unreal.MaleOne'
}
