//=============================================================================
// CrucifiedNali.
//=============================================================================
class CrucifiedNali expands NaliCarcass;

#exec MESH IMPORT MESH=CrossNali ANIVFILE=MODELS\nali_x.3D DATAFILE=MODELS\nali_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=CrossNali X=00 Y=-130 Z=30 YAW=64 ROLL=-64

#exec MESH SEQUENCE MESH=CrossNali SEQ=All      	STARTFRAME=0   NUMFRAMES=61
#exec MESH SEQUENCE MESH=CrossNali SEQ=HeadDown 	STARTFRAME=0   NUMFRAMES=20	RATE=15
#exec MESH SEQUENCE MESH=CrossNali SEQ=Writhe      	STARTFRAME=20  NUMFRAMES=30	RATE=15
#exec MESH SEQUENCE MESH=CrossNali SEQ=Dead1      	STARTFRAME=50  NUMFRAMES=1			
#exec MESH SEQUENCE MESH=CrossNali SEQ=Dead2      	STARTFRAME=51  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CrossNali SEQ=Dead3      	STARTFRAME=52  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CrossNali SEQ=Dead4      	STARTFRAME=53  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CrossNali SEQ=Hang1      	STARTFRAME=54  NUMFRAMES=1			GROUP=Upright
#exec MESH SEQUENCE MESH=CrossNali SEQ=Hang2      	STARTFRAME=55  NUMFRAMES=1			GROUP=Upright
#exec MESH SEQUENCE MESH=CrossNali SEQ=Hang3      	STARTFRAME=56  NUMFRAMES=1			GROUP=Upright
#exec MESH SEQUENCE MESH=CrossNali SEQ=HangFeet    	STARTFRAME=57  NUMFRAMES=1			GROUP=Upright
#exec MESH SEQUENCE MESH=CrossNali SEQ=Impaled     	STARTFRAME=58  NUMFRAMES=1			GROUP=Upright
#exec MESH SEQUENCE MESH=CrossNali SEQ=Stocks      	STARTFRAME=59  NUMFRAMES=1			GROUP=Upright
#exec MESH SEQUENCE MESH=CrossNali SEQ=Stretch     	STARTFRAME=60  NUMFRAMES=1


#exec TEXTURE IMPORT NAME=JNali1 FILE=MODELS\nali.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=crossnali X=0.072 Y=0.072 Z=0.144
#exec MESHMAP SETTEXTURE MESHMAP=crossnali NUM=0 TEXTURE=Jnali1

auto state Dying
{
	function AnimEnd()
	{
		if ( (AnimSequence == 'HeadDown') && (FRand() < 0.1) )
			PlayAnim('Writhe', 0.3 + 0.6 * FRand());
		else
			PlayAnim('HeadDown', 0.3 + 0.6 * FRand());
	}
	
	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		if ( (Damage > 40) || (momentum.Z/mass > 150) || (CumulativeDamage > 100) )
		{
			Velocity = momentum/Mass;
			ChunkUp(Damage);
		}
		else
		{
			CumulativeDamage += Damage;
			if ( CumulativeDamage > 40 )
			{
				Disable('AnimEnd');
				TweenAnim('HeadDown', 0.22);
			}
			else
				PlayAnim('Writhe', 0.7, 0.1);
		}
	}
	
Begin:
	if ( AnimSequence == 'HeadDown' )
		PlayAnim('HeadDown');
	else
	{
		 if ( GetAnimGroup(AnimSequence) != 'Upright' )
		 	ReduceCylinder();
		bReducedHeight = true;
		GotoState('Dead');
	}
}

defaultproperties
{
     Mesh=CrossNali
     Physics=PHYS_None
     LifeSpan=+00000.000000
	 AnimSequence=HeadDown
}
