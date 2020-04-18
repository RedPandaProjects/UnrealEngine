//=============================================================================
// MasterChunk.
//=============================================================================
class MasterChunk expands Chunk1;

var bool bNoExtraChunks;

/*
function Timer()
{
	if ( bNoExtraChunks )
		Super.Timer();
	else
		bNoExtraChunks = true;
}
*/

simulated function PostBeginPlay()
{
//	local chunk c;
//	local vector X,Y,Z;

	Velocity = Vector(Rotation) * (Speed + (FRand() * 200 - 100));
	if (Region.zone.bWaterZone)
		SetPhysics(PHYS_Falling);

/*
	// spawn other chunks
	if ( !bNoExtraChunks )
	{
		GetAxes(Rotation,X,Y,Z);
		c = Spawn( class 'Chunk2',, '', Location - Z);
		c.RemoteRole = ROLE_None;
		c = Spawn( class 'Chunk3',, '', Location + 2 * Y + Z);
		c.RemoteRole = ROLE_None;
		c = Spawn( class 'Chunk4',, '', Location - Y);
		c.RemoteRole = ROLE_None;
		c = Spawn( class 'Chunk1',, '', Location + 2 * Y - Z);
		c.RemoteRole = ROLE_None;
		c = Spawn( class 'Chunk2',, '', Location);
		c.RemoteRole = ROLE_None;
		c = Spawn( class 'Chunk3',, '', Location + Y - Z);
		c.RemoteRole = ROLE_None;
		c = Spawn( class 'Chunk4',, '', Location + 2 * Y + Z);
		c.RemoteRole = ROLE_None;

		if (Role == ROLE_Authority)
			SetTimer(0.3, false);
	}
*/
}