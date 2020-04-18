//=============================================================================
// DripGenerator.
//=============================================================================
class DripGenerator expands Decoration;

#exec MESH IMPORT MESH=dripMesh ANIVFILE=MODELS\drip_a.3D DATAFILE=MODELS\drip_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=dripMesh X=0 Y=0 Z=-50 YAW=64
#exec MESH SEQUENCE MESH=dripMesh SEQ=All       STARTFRAME=0   NUMFRAMES=6
#exec MESH SEQUENCE MESH=dripMesh SEQ=Dripping  STARTFRAME=0   NUMFRAMES=6
#exec TEXTURE IMPORT NAME=Jmisc1 FILE=MODELS\misc.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=dripMesh X=0.01 Y=0.01 Z=0.02
#exec MESHMAP SETTEXTURE MESHMAP=dripMesh NUM=0 TEXTURE=Jmisc1

var() float DripPause;		// pause between drips
var() float DripVariance;		// how different each drip is 
var() Texture DripTexture;

auto state Dripping
{

	function Timer()
	{
		local drip d;
		d = Spawn(class'Drip');
		d.DrawScale = 0.5+FRand()*DripVariance;
		d.Skin = DripTexture;
	}
	
	
Begin:
	SetTimer(DripPause+FRand()*DripPause,True);

}

defaultproperties
{
     DripPause=+00000.700000
     DripVariance=+00000.500000
     DripTexture=JMisc1
     bStatic=False
     bHidden=True
     DrawType=DT_Mesh
     Mesh=DripMesh
     bMeshCurvy=False
}
