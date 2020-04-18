//=============================================================================
// UdpLink: An Internet UDP connectionless socket.
//=============================================================================
class UdpLink expands Info
	intrinsic
	transient;

// Types.
struct IpAddr
{
	var int Addr;
	var int Port;
};

// Variables.
const BroadcastAddr=-1;
var const int Socket;
var enum EUdpMode {UDP_Text, UDP_Binary} UdpMode;

// Intrinsics.
intrinsic function bool Resolve( string[240] URL );
intrinsic function bool BindPort( optional int Port );
intrinsic function bool SendText( IpAddr Addr, coerce string[240] Str );
intrinsic function bool SendBinary( IpAddr Addr, int Count, byte B[240] );
intrinsic function string[240] IpAddrToURL( IpAddr Addr );

// Events.
event Resolved( IpAddr Addr );
event ResolveFailed();
event ReceivedText( IpAddr Addr, string[240] Text );
//event ReceivedBinary( IpAddr Addr, int Count, byte B[240] );

defaultproperties
{
	bAlwaysTick=true
}