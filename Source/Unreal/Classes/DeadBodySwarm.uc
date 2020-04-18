//=============================================================================
// DeadBodySwarm.
//=============================================================================
class DeadBodySwarm expands HorseFlySwarm;

auto state buzzing
{
ignores EnemyNotVisible;

	function SpawnFly()
	{
		if ( swarmsize > 0 )
		{
			swarmsize--;
			spawn(class 'horsefly',self,'', Location + VRand() * CollisionRadius);
		}
		if ( swarmsize > 0 )
			SetTimer(5.0 * FRand(), false);
	}

	function Timer()
	{
		SpawnFly();
	}

	function SeePlayer(Actor SeenPlayer)
	{
		SpawnFly();
		Disable('SeePlayer');
	}
}

defaultproperties
{
	 bOnlyIfEnemy=false
}
