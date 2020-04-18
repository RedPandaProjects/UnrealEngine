//=============================================================================
// SpriteBlueExplo.
//=============================================================================
class SpriteBlueExplo expands SpriteBallExplosion;

#exec OBJ LOAD FILE=textures\DispExpl.utx PACKAGE=Unreal.DispExpl
#exec TEXTURE IMPORT NAME=BluePal FILE=textures\expal2a.pcx GROUP=Effects

#exec AUDIO IMPORT FILE="sounds\dispersion\dpexplo4.wav" NAME="DispEX1" GROUP="General"

simulated function PostBeginPlay()
{
	PlaySound (EffectSound1,,7.0);	
	if (Level.bHighDetailMode)	DrawScale = DrawScale * 2.0;
}

defaultproperties
{
     EffectSound1=Sound'Unreal.General.DispEX1'
     Texture=Texture'Unreal.DispExpl.dseb_A00'
     Skin=Texture'Unreal.Effects.BluePal'
     DrawScale=2.700000
     LightRadius=6
}
