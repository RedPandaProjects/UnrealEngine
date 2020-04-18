class Health expands PickUp;

#exec AUDIO IMPORT FILE="Sounds\Pickups\HEALTH2.WAV"  NAME="Health2"     GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_Health FILE=TEXTURES\HUD\i_Health.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=HealthM ANIVFILE=MODELS\aniv38.3D DATAFILE=MODELS\data38.3D X=0 Y=0 Z=0
#exec  MESH ORIGIN MESH=HealthM X=0 Y=0 Z=0 YAW=0
#exec  MESH SEQUENCE MESH=HealthM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec  TEXTURE IMPORT NAME=Jhealth1 FILE=MODELS\health.PCX GROUP="Skins" FLAGS=2
#exec  MESHMAP SCALE MESHMAP=HealthM X=0.02 Y=0.02 Z=0.04
#exec  MESHMAP SETTEXTURE MESHMAP=HealthM NUM=0 TEXTURE=Jhealth1

var() int HealingAmount;
var() bool bSuperHeal;

event float BotDesireability(Pawn Bot)
{
	local float desire;

	desire = HealingAmount;
	if ( !bSuperHeal )
		desire = Min(desire, 100 - Bot.Health);

	if ( (Bot.Weapon != None) && (Bot.Weapon.AIRating > 0.3) )
		desire *= 2;
	if ( Bot.Health < 40 )
		return ( FMin(0.03 * desire, 2) );
	else if ( Bot.Health < 100 )
		return ( 0.01 * desire ); 
	else
		return 0;
}

auto state Pickup
{	
	function Touch( actor Other )
	{
		local int HealMax;
	
		if ( ValidTouch(Other) ) 
		{		
			HealMax = Pawn(Other).default.health;
			if (bSuperHeal) HealMax = HealMax * 2.0;
			if (Pawn(Other).Health < HealMax) 
			{
				Pawn(Other).Health += HealingAmount;
				if (Pawn(Other).Health > HealMax) Pawn(Other).Health = HealMax;
				if ( bSuperHeal )
					Pawn(Other).ClientMessage(PickupMessage);
				else				
					Pawn(Other).ClientMessage(PickupMessage $ HealingAmount);				
				PlaySound (PickupSound,,2.5);
				if ( Level.Game.Difficulty > 1 )
					Other.MakeNoise(0.1 * Level.Game.Difficulty);		
				SetRespawn();
			}
		}
	}
}

defaultproperties
{
     HealingAmount=20
     PickupMessage="You picked up a Health Pack +"
     RespawnTime=20.000000
     PickupViewMesh=Mesh'Unreal.HealthM'
     MaxDesireability=0.500000
     PickupSound=Sound'Unreal.Pickups.Health2'
     Icon=Texture'Unreal.Icons.I_Health'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.HealthM'
     AmbientGlow=64
     bMeshCurvy=False
     CollisionRadius=22.000000
     CollisionHeight=8.000000
     Mass=10.000000
}
