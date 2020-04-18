//=============================================================================
// ServerBeacon: Broadcasts a LAN beacon so clients can find the server.
//=============================================================================
class ServerBeacon expands UdpBeacon
	transient;

function BeginPlay()
{
	if( !DoBeacon )
	{
		Destroy();
		return;
	}
	if( BindPort() )
	{
		SetTimer( BeaconTime, true );
		log( "ServerBeacon begin" );
		return;
	}
	else log( "ServerBeacon failed" );
}
function Timer()
{
	local IpAddr Addr;
	local string[240] BeaconText;
	Addr.Addr = BroadcastAddr;
	Addr.Port = BeaconPort;
	Level.Game.GetBeaconText( BeaconText );
	SendText( Addr, BeaconProduct $ " " $ Level.GetAddressURL() $ " " $ BeaconText );
}
