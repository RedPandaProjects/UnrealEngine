//=============================================================================
// StingerProjectile.
//=============================================================================
class StingerProjectile expands Projectile;

#exec MESH IMPORT MESH=TarydiumProjectile ANIVFILE=MODELS\aniv52.3D DATAFILE=MODELS\data52.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TarydiumProjectile X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=TarydiumProjectile SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Tarydium1 FILE=MODELS\shells.PCX 
#exec MESHMAP SCALE MESHMAP=TarydiumProjectile X=0.015 Y=0.015 Z=0.03
#exec MESHMAP SETTEXTURE MESHMAP=TarydiumProjectile NUM=4 TEXTURE=Tarydium1

#exec TEXTURE IMPORT NAME=ExplosionPal3 FILE=textures\expal2.pcx GROUP=Effects

#exec MESH IMPORT MESH=burst ANIVFILE=MODELS\burst_a.3D DATAFILE=MODELS\burst_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=burst X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=burst SEQ=All       STARTFRAME=0   NUMFRAMES=6
#exec MESH SEQUENCE MESH=burst SEQ=Explo     STARTFRAME=0   NUMFRAMES=6
#exec TEXTURE IMPORT NAME=Jburst1 FILE=MODELS\burst.PCX GROUP=Skin
#exec MESHMAP SCALE MESHMAP=burst X=0.2 Y=0.2 Z=0.4 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=burst NUM=0 TEXTURE=Jburst1

#exec AUDIO IMPORT FILE="Sounds\stinger\Ricochet.WAV" NAME="Ricochet" GROUP="Stinger"
#exec AUDIO IMPORT FILE="Sounds\Razor\bladehit.wav" NAME="BladeHit" GROUP="RazorJack"

var bool bLighting;
var float DelayTime;

/////////////////////////////////////////////////////
auto state Flying
{
	function ProcessTouch( Actor Other, Vector HitLocation )
	{
		local int hitdamage;
		local vector hitDir;
		
		if (Other != instigator && StingerProjectile(Other) == none)
		{
			hitDir = Normal(Velocity);
			if ( FRand() < 0.2 )
				hitDir *= 5;
			Other.TakeDamage(damage, instigator,HitLocation,
				(MomentumTransfer * hitDir), 'shot');
			Destroy();			
		}
	}

	simulated function HitWall( vector HitNormal, actor Wall )
	{
		Super.HitWall(HitNormal, Wall);	
		if (FRand()<0.3) 
			PlaySound(ImpactSound, SLOT_Misc, 0.5,,, 0.5+FRand());
		else  
			PlaySound(MiscSound, SLOT_Misc, 0.6,,,1.0);

		MakeNoise(0.3);
	  	SetPhysics(PHYS_None);
		SetCollision(false,false,false);
		RemoteRole = ROLE_None;
		Mesh = mesh'Burst';
		SetRotation( RotRand() );
		PlayAnim   ( 'Explo', 0.9 );
		GotoState('Exploding');
	}

	simulated function Timer()
	{
		local bubble1 b;
		if (Level.NetMode!=NM_DedicatedServer)
		{
	 		b=spawn(class'Bubble1'); 
 			b.DrawScale= 0.1 + FRand()*0.2;
 			b.SetLocation(Location+FRand()*vect(2,0,0)+FRand()*Vect(0,2,0)+FRand()*Vect(0,0,2));
 			b.buoyancy = b.mass+(FRand()*0.4+0.1);
 		}
		DelayTime+=FRand()*0.1+0.1;
		SetTimer(DelayTime,False); 	
	}

	simulated function ZoneChange( Zoneinfo NewZone )
	{
		if (NewZone.bWaterZone) 
		{
			Velocity=0.7*Velocity;	
			DelayTime=0.03;		
			SetTimer(DelayTime,False);
		}
	}

	function BeginState()
	{
		local rotator RandRot;

		Velocity = Vector(Rotation) * speed;
		RandRot.Pitch = FRand() * 200 - 100;
		RandRot.Yaw = FRand() * 200 - 100;
		RandRot.Roll = FRand() * 200 - 100;
		Velocity = Velocity >> RandRot;
		if( Region.zone.bWaterZone )
			Velocity=0.7*Velocity;
	}
}

///////////////////////////////////////////////////////
function Explode(vector HitLocation, vector HitNormal)
{
}

state Exploding
{
Begin:
	FinishAnim();
	Destroy();
}

defaultproperties
{
     speed=1600.000000
     Damage=14.000000
     MomentumTransfer=4000
     ImpactSound=Sound'Unreal.Stinger.Ricochet'
     MiscSound=Sound'Unreal.Razorjack.BladeHit'
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=6.000000
     AnimRate=1.000000
     Mesh=Mesh'Unreal.TarydiumProjectile'
     AmbientGlow=215
     bUnlit=True
     bNoSmooth=True
     bMeshCurvy=False
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=80
     LightHue=152
     LightSaturation=32
     LightRadius=5
     LightPeriod=50
     bBounce=True
     Mass=2.000000
     NetPriority=6.000000
}
