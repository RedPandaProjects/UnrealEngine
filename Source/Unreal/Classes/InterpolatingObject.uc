//=============================================================================
// InterpolatingObject.
//=============================================================================
class InterpolatingObject expands Decoration;

#exec MESH IMPORT MESH=CandleM ANIVFILE=MODELS\candle_a.3D DATAFILE=MODELS\candle_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=CandleM X=0 Y=0 Z=-50 YAW=64
#exec MESH SEQUENCE MESH=candleM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=candleM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JCandle1 FILE=MODELS\candle.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=candleM X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=candleM NUM=1 TEXTURE=Jcandle1


function Destroyed()
{
	event=''; // no event on destruction;
	Super.Destroyed();
}

function interpolateend(actor Other)
{
}

function Trigger( actor Other, pawn EventInstigator )
{
	local InterpolationPoint i;	
	foreach AllActors( class 'InterpolationPoint', i, Event )
	{
		if( i.Position == 0 )
		{
			log("first point "$i);
			SetCollision(false,false,false);
			Target = i;
			SetPhysics(PHYS_Interpolating);
			PhysRate = 1.0;
			PhysAlpha = 0.0;
			bInterpolating = true;
		}
	}
}
defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Mesh=CandleM
     bMeshCurvy=False
}
