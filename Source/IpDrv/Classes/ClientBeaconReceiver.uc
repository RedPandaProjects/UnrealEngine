//=============================================================================
// ClientBeaconReceiver: Receives LAN beacons from servers.
//=============================================================================
class ClientBeaconReceiver expands UdpBeacon
	transient;

var struct BeaconInfo
{
	var IpAddr      Addr;
	var float       Time;
	var string[240] Text;
} Beacons[32];

function string[240] GetBeaconAddress(int i)
{
	local int s;

	s = InStr(Beacons[i].Text, " ");
	return Left(Beacons[i].Text, s);
}

function String[240] GetBeaconText(int i)
{
	local int s;
	s = InStr(Beacons[i].Text, " ");
	return Right(Beacons[i].Text, Len(Beacons[i].Text) - s);
}

function BeginPlay()
{
	if( BindPort( BeaconPort ) )
	{
		SetTimer( BeaconTime, true );
		log( "ClientBeaconReceiver begin" );
	}
	else
	{
		log( "ClientBeaconReceiver failed; beacon port in use" );
	}
}
function Destroyed()
{
	log( "ClientBeaconReceiver end" );
}
function Timer()
{
	local int i, j;
	for( i=0; i<arraycount(Beacons); i++ )
		if
		(	Beacons[i].Addr.Addr!=0
		&&	Level.TimeSeconds-Beacons[i].Time<BeaconTimeout )
			Beacons[j++] = Beacons[i];
	for( j=j; j<arraycount(Beacons); j++ )
		Beacons[j].Addr.Addr=0;
}
event ReceivedText( IpAddr Addr, string[240] Text )
{
	local int i, n;
	
	n = len(BeaconProduct);
	if( left(Text,n)==BeaconProduct )
	{
		Text = mid(Text,n+1);
		for( i=0; i<arraycount(Beacons); i++ )
			if( Beacons[i].Addr==Addr )
				break;
		if( i==arraycount(Beacons) )
			for( i=0; i<arraycount(Beacons); i++ )
				if( Beacons[i].Addr.Addr==0 )
					break;
		if( i==arraycount(Beacons) )
			return;
		Beacons[i].Addr = Addr;
		Beacons[i].Time = Level.TimeSeconds;
		Beacons[i].Text = Text;
	}
}
