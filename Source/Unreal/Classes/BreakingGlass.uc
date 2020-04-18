//=============================================================================
// BreakingGlass.
//=============================================================================
class BreakingGlass expands ExplodingWall;

#exec AUDIO IMPORT FILE="Sounds\general\glass.WAV" NAME="BreakGlass" GROUP="General"

var() float ParticleSize;
var() float Numparticles;

Auto State Exploding
{
	singular function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation,
						Vector momentum, name damageType)
	{
		if ( !bOnlyTriggerable ) 
			Explode(instigatedBy, Momentum);
	}

	function BeginState()
	{
		Super.BeginState();
		NumGlassChunks = NumParticles;
		GlassParticleSize = ParticleSize;
	}
}

defaultproperties
{
     ParticleSize=+00000.750000
     ExplosionSize=+00100.000000
     Numparticles=+00016.000000
     ExplosionDimensions=+00090.000000
     GlassTexture=Engine.Cloudcast
     NumWallChunks=0
     NumWoodChunks=0
     BreakingSound=Unreal.BreakGlass
     DrawType=DT_Sprite
     CollisionRadius=+00045.000000
     CollisionHeight=+00045.000000
     bCollideActors=True
     bCollideWorld=True
     bProjTarget=True
     Physics=PHYS_None
     RemoteRole=ROLE_SimulatedProxy
     Class=Unreal.BreakingGlass
}
