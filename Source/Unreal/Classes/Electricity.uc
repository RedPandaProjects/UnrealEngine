//=============================================================================
// Electricity.
//=============================================================================
class Electricity expands Effects;

#exec MESH IMPORT MESH=Electr ANIVFILE=MODELS\Electr_a.3D DATAFILE=MODELS\Electr_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Electr X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=Electr SEQ=All        STARTFRAME=0   NUMFRAMES=11
#exec MESH SEQUENCE MESH=Electr SEQ=ElectBurst STARTFRAME=0   NUMFRAMES=11
#exec OBJ LOAD FILE=Textures\fireeffect7.utx PACKAGE=Unreal.Effect7
#exec MESHMAP SCALE MESHMAP=Electr X=0.1 Y=0.1 Z=0.2 YAW=128 
#exec MESHMAP SETTEXTURE MESHMAP=Electr NUM=1 TEXTURE=Unreal.Effect7.MyTex16

auto state Explode
{
Begin:
	PlayAnim( 'ElectBurst', 0.6 );
	PlaySound (EffectSound1);	
	FinishAnim();
	Destroy ();
}

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.Electr
     bUnlit=True
     Physics=PHYS_None
     RemoteRole=ROLE_SimulatedProxy
}
