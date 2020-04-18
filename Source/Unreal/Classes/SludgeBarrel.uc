//=============================================================================
// SludgeBarrel.
//=============================================================================
class SludgeBarrel expands Decoration;

#exec AUDIO IMPORT FILE="sounds\general\bPush1.wav" NAME="ObjectPush" GROUP="General"
#exec AUDIO IMPORT FILE="sounds\general\EndPush.wav" NAME="Endpush" GROUP="General"

#exec MESH IMPORT MESH=sbarrel ANIVFILE=MODELS\oilbar_a.3D DATAFILE=MODELS\oilbar_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=sbarrel X=0 Y=0 Z=-190 YAW=0
#exec MESH SEQUENCE MESH=sbarrel SEQ=All    STARTFRAME=0  NUMFRAMES=21
#exec MESH SEQUENCE MESH=sbarrel SEQ=Swirl  STARTFRAME=0  NUMFRAMES=21
#exec TEXTURE IMPORT NAME=Jsbarrel1 FILE=MODELS\oilbarel.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=sbarrel X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=sbarrel NUM=1 TEXTURE=Jsbarrel1


var() int Health;
var name DamageTypeb;

Auto State Animate
{
	singular function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		DamageTypeb = DamageType;
		Instigator = InstigatedBy;
		if (Health<0) Return;
		if ( Instigator != None )
			MakeNoise(1.0);
		Health -= NDamage;
		if (Health <0) 	GoToState('exploding');
		else 
		{
			SetPhysics(PHYS_Falling);
			bBounce = True;
			Momentum.Z = 1000;
			Velocity=Momentum*0.01;
		}
	}


Begin:
	LoopAnim('swirl',0.4);
}

state exploding
{

	function Timer()
	{
		local int i;
		local BarrelSludge b;
		
		HurtRadius(200, 100, 'burned', 0, Location);
		SetCollision(False,False,False);
		for (i=0 ; i<int(FRand()*5+1) ; i++) {
			b = Spawn( class 'BarrelSludge',, '', Location + (vect(0,0,1.2)+VRand())*(70+FRand()*50));
			if ( b != None )
				b.DrawScale = FRand()+1.2;
		}
		Spawn(Class 'GreenSmokePuff');
		if (DamageTypeb == 'burned') skinnedFrag(class'Fragment1', texture'jSBarrel1', Vect(0,0,30000),1.0, 4);	
		else  skinnedFrag(class'Fragment1', texture'jSBarrel1', Vect(0,0,30000),1.0, 12);
		Destroy();
}

Begin:
	if (DamageTypeb != 'burned') Timer();	
	else SetTimer(FRand()*0.4+0.1,False);
}

defaultproperties
{
     Health=2
     bPushable=True
     PushSound=Sound'Unreal.General.ObjectPush'
     EndPushSound=Sound'Unreal.General.Endpush'
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.sbarrel'
     AmbientGlow=96
     bMeshCurvy=False
     CollisionRadius=17.000000
     CollisionHeight=26.299999
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     LightType=LT_Steady
     LightEffect=LE_TorchWaver
     LightBrightness=64
     LightHue=88
     LightRadius=10
     Mass=100.000000
}
