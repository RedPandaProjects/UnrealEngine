//=============================================================================
// Panel.
//=============================================================================
class Panel expands Decoration;


#exec MESH IMPORT MESH=IPanel ANIVFILE=MODELS\Inoxx_a.3D DATAFILE=MODELS\Inoxx_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=IPanel X=0 Y=0 Z=0  
#exec MESH SEQUENCE MESH=IPanel SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=IPanel SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=IPanel X=0.05 Y=0.05 Z=0.1

var() bool bFade;
var float Count;



function Timer()
{
	if (bFade)
	{
		ScaleGlow = 1.0-Count;
	}
	else
	{
		ScaleGlow = Count;
	}
	Count += 0.1;
	if (Count>1.0) SetTimer(0.0,False);		
}


function Trigger( actor Other, pawn EventInstigator )
{
	bFade = False;
	Count = ScaleGlow;
	if (ScaleGlow>0.5) 
	{
		Count = 1.0 - ScaleGlow;
		bFade = True;
	}
	SetTimer(0.10,True);
}

defaultproperties
{
     bStatic=False
     bDirectional=True
     DrawType=DT_Mesh
     Style=STY_Translucent
     Texture=Texture'Engine.S_Corpse'
     Mesh=Mesh'Unreal.IPanel'
     bUnlit=True
}
