//=============================================================================
// SpriteExplosion.
//=============================================================================
class SpriteExplosion expands Effects;

var() Texture s_Exp[20];

#exec Texture Import File=models\exp001.pcx Name=s_Exp001 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp002.pcx Name=s_Exp002 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp003.pcx Name=s_Exp003 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp004.pcx Name=s_Exp004 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp005.pcx Name=s_Exp005 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp006.pcx Name=s_Exp006 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp007.pcx Name=s_Exp007 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp008.pcx Name=s_Exp008 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp009.pcx Name=s_Exp009 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp010.pcx Name=s_Exp010 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp011.pcx Name=s_Exp011 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp012.pcx Name=s_Exp012 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp013.pcx Name=s_Exp013 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp014.pcx Name=s_Exp014 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp015.pcx Name=s_Exp015 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp016.pcx Name=s_Exp016 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp017.pcx Name=s_Exp017 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp018.pcx Name=s_Exp018 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp019.pcx Name=s_Exp019 Mips=Off Mask=On Flags=2
#exec Texture Import File=models\exp020.pcx Name=s_Exp020 Mips=Off Mask=On Flags=2

var int i;

simulated function BeginPlay()
{
	Super.BeginPlay();
	i = 1;
	Texture = s_Exp[i];
	PlaySound (EffectSound1);	
	MakeNoise(1.0);		
}

simulated function Tick( float DeltaTime )
{
	i++;
	if ( Level.NetMode != NM_DedicatedServer )
		Texture = s_Exp[i];
	if (i > 18) Destroy();
}

defaultproperties
{
     s_Exp(1)=Unreal.s_Exp001
     s_Exp(2)=Unreal.s_Exp002
     s_Exp(3)=Unreal.s_Exp003
     s_Exp(4)=Unreal.s_Exp004
     s_Exp(5)=Unreal.s_Exp005
     s_Exp(6)=Unreal.s_Exp006
     s_Exp(7)=Unreal.s_Exp007
     s_Exp(8)=Unreal.s_Exp008
     s_Exp(9)=Unreal.s_Exp009
     s_Exp(10)=Unreal.s_Exp010
     s_Exp(11)=Unreal.s_Exp011
     s_Exp(12)=Unreal.s_Exp012
     s_Exp(13)=Unreal.s_Exp013
     s_Exp(14)=Unreal.s_Exp014
     s_Exp(15)=Unreal.s_Exp015
     s_Exp(16)=Unreal.s_Exp016
     s_Exp(17)=Unreal.s_Exp017
     s_Exp(18)=Unreal.s_Exp018
     s_Exp(19)=Unreal.s_Exp019
     DrawType=DT_Sprite
     Texture=Unreal.s_Exp001
     AmbientGlow=160
     Physics=PHYS_None
     RemoteRole=ROLE_SimulatedProxy
}
