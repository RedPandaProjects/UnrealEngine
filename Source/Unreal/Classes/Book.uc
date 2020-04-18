//=============================================================================
// Book.
//=============================================================================
class Book expands Decoration;

#exec MESH IMPORT MESH=BookM ANIVFILE=MODELS\book_a.3D DATAFILE=MODELS\book_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=BookM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=BookM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=BookM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JBook1 FILE=MODELS\Book1.PCX GROUP=Skins
#exec TEXTURE IMPORT NAME=JBook2 FILE=MODELS\Book2.PCX GROUP=Skins
#exec TEXTURE IMPORT NAME=JBook3 FILE=MODELS\Book3.PCX GROUP=Skins
#exec TEXTURE IMPORT NAME=JBook4 FILE=MODELS\Book4.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=BookM X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=BookM NUM=0 TEXTURE=JBook1

#exec AUDIO IMPORT FILE="Sounds\General\Chunkhit2.WAV" NAME="Chunkhit2" GROUP="General"

var bool bFirstHit;

Auto State Animate
{
	function HitWall (vector HitNormal, actor Wall)
	{
		local float speed;

		Velocity = 0.5*(( Velocity dot HitNormal ) * HitNormal * (-2.0) + Velocity);   // Reflect off Wall w/damping
		speed = VSize(Velocity);
		if (speed>500) PlaySound(PushSound, SLOT_Misc,1.0);			
		if (bFirstHit && speed<400) 
		{
			bFirstHit=False;
			bRotatetoDesired=True;
			bFixedRotationDir=False;
			DesiredRotation.Pitch=0;	
			DesiredRotation.Yaw=FRand()*65536;
			DesiredRotation.Roll=0;		
		}
		RotationRate.Yaw = RotationRate.Yaw*0.75;
		RotationRate.Roll = RotationRate.Roll*0.75;
		RotationRate.Pitch = RotationRate.Pitch*0.75;	
		If (speed < 30) 
			bBounce = False;
	}	

	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		SetPhysics(PHYS_Falling);
		bBounce = True;
		Momentum.Z = abs(Momentum.Z*4+3000);
		Velocity=Momentum*0.02;
		RotationRate.Yaw = 250000*FRand() - 125000;
		RotationRate.Pitch = 250000*FRand() - 125000;
		RotationRate.Roll = 250000*FRand() - 125000;	
		DesiredRotation = RotRand();
		bRotateToDesired=False;
		bFixedRotationDir=True;
		bFirstHit=True;
	}
}

defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Unreal.BookM
     bMeshCurvy=False
     CollisionRadius=+00012.000000
     CollisionHeight=+00004.000000
	 bPushable=true
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     Mass=+00001.000000
     PushSound=Unreal.Chunkhit2
}
