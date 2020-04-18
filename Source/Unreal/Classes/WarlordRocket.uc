//=============================================================================
// WarlordRocket.
//=============================================================================
class WarlordRocket expands Projectile;

#exec MESH IMPORT MESH=perock ANIVFILE=MODELS\perock_a.3D DATAFILE=MODELS\perock_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=perock X=0 Y=0 Z=0 YAW=0 ROLL=0 PITCH=-64
#exec MESH SEQUENCE MESH=perock SEQ=All       STARTFRAME=0   NUMFRAMES=16
#exec MESH SEQUENCE MESH=perock SEQ=Ignite    STARTFRAME=0   NUMFRAMES=3
#exec MESH SEQUENCE MESH=perock SEQ=Flying    STARTFRAME=3   NUMFRAMES=13
#exec TEXTURE IMPORT NAME=Jpeace1 FILE=MODELS\peaceg.PCX
#exec MESHMAP SCALE MESHMAP=perock  X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=perock NUM=1 TEXTURE=Jpeace1

#exec AUDIO IMPORT FILE="Sounds\General\8blfly2.WAV" NAME="BRocket" GROUP="General"

var vector OriginalDirection;
var float Count,SmokeRate;

function Explode(vector HitLocation, vector HitNormal)
{
	HurtRadius(damage, 200.0, 'exploded', MomentumTransfer, HitLocation);
	Spawn(class'SpriteBallExplosion', Pawn(Owner),,HitLocation);	
	Destroy();
}


function Tick(float DeltaTime)
{
	local SpriteSmokePuff b;

	Count += DeltaTime;
	if ( (Count>(SmokeRate+FRand()*SmokeRate)) && (Level.NetMode!=NM_DedicatedServer) ) 
	{
		b = Spawn(class'SpriteSmokePuff');
		b.RemoteRole = ROLE_None;		
		Count=0.0;
	}
}

auto state Flying
{

/*	function Timer()
	{
		local vector EnemyDir;

		if ( (Instigator != None) && (Instigator.Enemy != None) )
		{
			EnemyDir = Normal(Instigator.Enemy.Location - Location);
			if ( (EnemyDir Dot OriginalDirection) > 0 )
			{
				Velocity =  speed * Normal(EnemyDir * speed * (0.25 + instigator.skill * 0.1) + Velocity);		
				SetRotation(Rotator(Velocity));
			}
		}
	}
*/
	function ProcessTouch (Actor Other, Vector HitLocation)
	{
		if ((PeaceRocket(Other) == none) && (Other != Instigator) ) 
			Explode(HitLocation, vect(0,0,0));
	}


	function BeginState()
	{
		if (Level.bHighDetailMode) SmokeRate = 0.035;
		else SmokeRate = 0.15;	
		PlaySound(SpawnSound);
		OriginalDirection = Vector(Rotation);	
		Velocity = OriginalDirection * 500.0;
		Acceleration = Velocity * 0.4;	
		//SetTimer(0.1,True);
	}

Begin:
	Sleep(7.0);
	Explode(Location, vect(0,0,0));
}

defaultproperties
{
     speed=1200.000000
     MaxSpeed=1200.000000
     Damage=60.000000
     MomentumTransfer=50000
     Mesh=Mesh'Unreal.perock'
     DrawScale=2.500000
     AmbientGlow=1
     bUnlit=True
     SoundRadius=20
     SoundVolume=255
     AmbientSound=Sound'Unreal.General.BRocket'
     bBounce=True
}
