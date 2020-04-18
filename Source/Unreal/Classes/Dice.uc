//=============================================================================
// Dice.
//=============================================================================
class Dice expands Decoration;

#exec MESH IMPORT MESH=DiceM ANIVFILE=MODELS\dice_a.3D DATAFILE=MODELS\dice_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=DiceM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=DiceM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=DiceM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JDice1 FILE=MODELS\Dice.PCX GROUP=Skins

#exec MESHMAP SCALE MESHMAP=DiceM X=0.006 Y=0.006 Z=0.012
#exec MESHMAP SETTEXTURE MESHMAP=DiceM NUM=0 TEXTURE=JDice1

var bool bHasBounced;
var	int numBounces;

function Throw(vector Y)
{
	bHidden = false;
	numBounces = 0;
	Roll();
	Velocity = 40 * VRand() - 80 * FRand() * Y;
	SetPhysics(PHYS_Falling);
}

simulated function Roll()
{
	DesiredRotation.Pitch = 16384 * Rand(4);	
	DesiredRotation.Yaw = 16384 * Rand(4);
	DesiredRotation.roll = 16384 * Rand(4);
}

auto state Playing
{
	ignores BaseChange, Bump;

	simulated function HitWall (vector HitNormal, actor Wall)
	{
		local vector landspot;

		numBounces++;
		if ( (instigator == None) || (numBounces > 20) )
		{
			bBounce = false;
			Velocity.Z = -0.5 * Velocity.Z;
			return;
		}
		landspot = instigator.location + vector(instigator.rotation) * 2.3 * instigator.CollisionRadius;
		landspot.Z = location.Z;
		
		if ( bHasBounced && (Vsize(landspot - location) < 15) )
		{
			SetPhysics(PHYS_None);
			SetRotation(DesiredRotation);
			return;
		}

		Velocity = landspot - location + 3 * VRand();
		Velocity.Z *= -0.6;
		if (Velocity.Z < 50) 
			Velocity.Z = 50;
		bHasBounced = True;
		Roll();
		//PlaySound(ImpactSound);
	}
}

defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Mesh=DiceM
     bMeshCurvy=False
     CollisionRadius=+00003.000000
     CollisionHeight=+00003.000000
     bCollideActors=False
     bCollideWorld=True
     bBlockActors=False
     bBlockPlayers=False
	 bBounce=True
     bFixedRotationDir=True
	 bRotateToDesired=True
	 RotationRate=(Pitch=60000, Yaw=60000, Roll=60000)
}