//=============================================================================
// BigRock.
//=============================================================================
class BigRock expands Projectile;

#exec MESH IMPORT MESH=TBoulder ANIVFILE=MODELS\rock_a.3D DATAFILE=MODELS\rock_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TBoulder X=0 Y=0 Z=0 YAW=64

#exec MESH SEQUENCE MESH=TBoulder SEQ=All  STARTFRAME=0  NUMFRAMES=4
#exec MESH SEQUENCE MESH=TBoulder SEQ=Pos1  STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=TBoulder SEQ=Pos2  STARTFRAME=1   NUMFRAMES=1
#exec MESH SEQUENCE MESH=TBoulder SEQ=Pos3  STARTFRAME=2   NUMFRAMES=1
#exec MESH SEQUENCE MESH=TBoulder SEQ=Pos4  STARTFRAME=3   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JBoulder1 FILE=MODELS\rock.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=TBoulder X=0.01 Y=0.01 Z=0.02
#exec MESHMAP SETTEXTURE MESHMAP=TBoulder NUM=1 TEXTURE=JBoulder1

#exec AUDIO IMPORT FILE="Sounds\Titan\Rockhit1.wav" NAME="Rockhit" GROUP="Titan"

function PostBeginPlay()
{
	local float decision;

	Super.PostBeginPlay();
	Velocity = Vector(Rotation) * (0.8 + (0.3 * FRand())) * speed;
	DesiredRotation.Pitch = Rotation.Pitch + Rand(2000) - 1000;
	DesiredRotation.Roll = Rotation.Roll + Rand(2000) - 1000;
	DesiredRotation.Yaw = Rotation.Yaw + Rand(2000) - 1000; 
	decision = FRand();
	if (decision<0.25) 
		PlayAnim('Pos2', 1.0, 0.0);
	else if (decision<0.5) 
		PlayAnim('Pos3', 1.0, 0.0);
	else if (decision <0.75) 
		PlayAnim('Pos4', 1.0, 0.0);
	if (FRand() < 0.5)
		RotationRate.Pitch = Rand(180000);
	if ( (RotationRate.Pitch == 0) || (FRand() < 0.8) )
		RotationRate.Roll = Max(0, 50000 + Rand(200000) - RotationRate.Pitch);
}

function TakeDamage( int NDamage, Pawn instigatedBy, 
				Vector hitlocation, Vector momentum, name damageType) {

	// If a rock is shot, it will fragment into a number of smaller
	// pieces.  The player can fragment a giant boulder which would
	// otherwise crush him/her, and escape with minor or no wounds
	// when a multitude of smaller rocks hit.
	
	//log ("Rock gets hit by something...");
	Velocity += Momentum/(DrawScale * 10);
	if (Physics == PHYS_None )
	{
		SetPhysics(PHYS_Falling);
		Velocity.Z += 0.4 * VSize(momentum);
	}
	SpawnChunks(4);
}

function SpawnChunks(int num)
{
	local int    NumChunks,i;
	local BigRock   TempRock;
	local float scale;

	if ( DrawScale < 1 + FRand() )
		return;

	NumChunks = 1+Rand(num);
	scale = sqrt(0.52/NumChunks);
	if ( scale * DrawScale < 1 )
	{
		NumChunks *= scale * DrawScale;
		scale = 1/DrawScale;
	}
	speed = VSize(Velocity);
	for (i=0; i<NumChunks; i++) 
	{
		TempRock = Spawn(class'BigRock');
		if (TempRock != None )
			TempRock.InitFrag(self, scale);
	}
	InitFrag(self, 0.5);
}

function InitFrag(BigRock myParent, float scale)
{
	local rotator newRot;

	// Pick a random size for the chunks
	RotationRate = RotRand();
	scale *= (0.5 + FRand());
	DrawScale = scale * myParent.DrawScale;
	if ( DrawScale <= 2 )
		SetCollisionSize(0,0);
	else
		SetCollisionSize(CollisionRadius * DrawScale/Default.DrawScale, CollisionHeight * DrawScale/Default.DrawScale);

	Velocity = Normal(VRand() + myParent.Velocity/myParent.speed) 
				* (myParent.speed * (0.4 + 0.3 * (FRand() + FRand())));
}	

auto state Flying
{
	function ProcessTouch (Actor Other, Vector HitLocation)
	{
		local int hitdamage;

		if ( Other == instigator )
			return;
		PlaySound(ImpactSound, SLOT_Interact, DrawScale/10);	

		if ( !Other.IsA('BigRock') && !Other.IsA('Titan') )
		{
			Hitdamage = Damage * 0.00002 * (DrawScale**3) * speed;
			if ( (HitDamage > 6) && (speed > 150) )
				Other.TakeDamage(hitdamage, instigator,HitLocation,
					(35000.0 * Normal(Velocity)), 'crushed' );
		}
	}
	
	simulated function Landed(vector HitNormal)
	{
		HitWall(HitNormal, None);
	}

	simulated function HitWall (vector HitNormal, actor Wall)
	{
		local vector RealHitNormal;
		local float soundRad;

		if ( (Mover(Wall) != None) && Mover(Wall).bDamageTriggered )
			Wall.TakeDamage( Damage, instigator, Location, MomentumTransfer * Normal(Velocity), '');
		if ( Drawscale > 2.0 )
			soundRad = 500 * DrawScale;
		else
			soundRad = 100;
		PlaySound(ImpactSound, SLOT_Misc, DrawScale/8,,soundRad);	
		speed = VSize(velocity);
		if ( (HitNormal.Z > 0.8) && (speed < 60 - DrawScale) )
		{
			SetPhysics(PHYS_None);
			GotoState('Sitting');	
		}
		else
		{			
			SetPhysics(PHYS_Falling);
			RealHitNormal = HitNormal;
			if ( FRand() < 0.5 )
				RotationRate.Pitch = Max(RotationRate.Pitch, 100000);
			else
				RotationRate.Roll = Max(RotationRate.Roll, 100000);
			HitNormal = Normal(HitNormal + 0.5 * VRand()); 
			if ( (RealHitNormal Dot HitNormal) < 0 )
				HitNormal.Z *= -0.7;
			Velocity = 0.7 * (Velocity - 2 * HitNormal * (Velocity Dot HitNormal));
			DesiredRotation = rotator(HitNormal);
			if ( (speed > 150) && (FRand() * 30 < DrawScale) )
				SpawnChunks(4);
		}
	}

Begin:
	Sleep(5.0);
	SetPhysics(PHYS_Falling);
}

State Sitting
{
Begin:
	SetPhysics(PHYS_None);
	Sleep(DrawScale * 0.5);
	Destroy();
}

defaultproperties
{
     speed=+00900.000000
     MaxSpeed=+01000.000000
     Damage=+00040.000000
     ImpactSound=Unreal.Rockhit
	 Physics=PHYS_Falling
     CollisionRadius=+00030.000000
     CollisionHeight=+00030.000000
     Physics=PHYS_Falling
     bBounce=True
	 bFixedRotationDir=true
     LifeSpan=+00020.000000
     RemoteRole=ROLE_SimulatedProxy
	 AnimSequence=Pos1
     NetPriority=+00006.000000
     Mesh=Unreal.TBoulder
     DrawScale=+00007.500000
	 bMeshCurvy=False
}
