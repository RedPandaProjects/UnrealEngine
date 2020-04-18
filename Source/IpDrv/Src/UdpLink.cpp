/*=============================================================================
	IpDrv.cpp: Unreal TCP/IP driver.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#include "IpDrvPrivate.h"

/*-----------------------------------------------------------------------------
	AUdpLink implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(AUdpLink);

//
// Constructor.
//
AUdpLink::AUdpLink()
{
	guard(AUdpLink::AUdpLink);
	char Error[256];
	wsaInit( Error );
	unguard;
}

//
// Destroy.
//
void AUdpLink::Destroy()
{
	guard(AUdpLink::Destroy);
	if( GetSocket() )
		closesocket(GetSocket());
	Super::Destroy();
	unguard;
}

//
// Resolve a string URL and return its IP address.
//
void AUdpLink::execResolve( FFrame& Stack, BYTE*& Result )
{
	guard(AUdpLink::execResolve);
	P_GET_STRING(URL);
	P_FINISH;

	((FIpAddr*)Result)->Addr = 0;
	((FIpAddr*)Result)->Port = 0;

	unguard;
}
AUTOREGISTER_INTRINSIC( AUdpLink, INDEX_NONE, execResolve );

//
// Listen for packets on a port.
//
void AUdpLink::execBindPort( FFrame& Stack, BYTE*& Result )
{
	guard(AUdpLink::execBindPort);
	P_GET_INT_OPT(Port,0);
	P_FINISH;
	if( GInitialized )
	{
		if( GetSocket()==NULL )
		{
			GetSocket() = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
			if( GetSocket() != INVALID_SOCKET )
			{
				BOOL TrueBuffer=1;
				if( setsockopt( GetSocket(), SOL_SOCKET, SO_BROADCAST, (char*)&TrueBuffer, sizeof(TrueBuffer) )==0 )
				{
					sockaddr_in Addr;
					Addr.sin_family      = AF_INET;
					Addr.sin_addr.s_addr = 0;
					Addr.sin_port        = htons(Port);
					if( bind( Socket, (sockaddr*)&Addr, sizeof(Addr) )==0 )
					{
						DWORD NoBlock = 1;
						if( ioctlsocket( Socket, FIONBIO, &NoBlock )==0 )
						{
							// Success.
							*(DWORD*)Result = 1;
							return;
						}
						else Stack.ScriptWarn( 0, "BindPort: ioctlsocket failed" );
					} else Stack.ScriptWarn( 0, "BindPort: bind failed" );
				} else Stack.ScriptWarn( 0, "BindPort: setsockopt failed" );
			} else Stack.ScriptWarn( 0, "BindPort: socket failed" );
			closesocket(GetSocket());
			GetSocket()=NULL;
		} else Stack.ScriptWarn( 0, "BindPort: already bound" );
	} else Stack.ScriptWarn( 0, "BindPort: winsock failed" );
	*(DWORD*)Result = 0;
	unguard;
}
AUTOREGISTER_INTRINSIC( AUdpLink, INDEX_NONE, execBindPort );

//
// Send text in a UDP packet.
//
void AUdpLink::execSendText( FFrame& Stack, BYTE*& Result )
{
	guard(AUdpLink::execSendText);
	P_GET_STRUCT(FIpAddr,IpAddr);
	P_GET_STRING(Str);
	P_FINISH;
	if( GetSocket() )
	{
		sockaddr_in Addr;
		Addr.sin_family      = AF_INET;
		Addr.sin_port        = htons(IpAddr.Port);
		Addr.sin_addr.s_addr = htonl(IpAddr.Addr);
		if( sendto( Socket, Str, appStrlen(Str), 0, (sockaddr*)&Addr, sizeof(Addr) )==0 )
		{
			Stack.ScriptWarn( 0, "SentText: sendto failed" );
			*(DWORD*)Result = 1;
			return;
		}
	}
	*(DWORD*)Result = 0;
	unguard;
}
AUTOREGISTER_INTRINSIC( AUdpLink, INDEX_NONE, execSendText );

//
// Convert an address to a string URL.
//
void AUdpLink::execIpAddrToURL( FFrame& Stack, BYTE*& Result )
{
	guard(AUdpLink::execIpAddrToURL);
	P_GET_STRUCT(FIpAddr,Addr);
	P_FINISH;

	DWORD D=Addr.Addr;
	appSprintf( (char*)Result, "%i.%i.%i.%i:%i", (BYTE)(D>>24), (BYTE)(D>>16), (BYTE)(D>>8), (BYTE)(D>>0), Addr.Port );

	unguard;
}
AUTOREGISTER_INTRINSIC( AUdpLink, INDEX_NONE, execIpAddrToURL );

//
// Send binary data.
//
void AUdpLink::execSendBinary( FFrame& Stack, BYTE*& Result )
{
	guard(AUdpLink::execSendBinary);
	P_GET_STRUCT(FIpAddr,Addr);
	P_GET_INT(Count);
	//P_GET_BYTE_ARRAY(b)!!
	P_FINISH;

	unguard;
}
AUTOREGISTER_INTRINSIC( AUdpLink, INDEX_NONE, execSendBinary );

//
// Time passes...
//
UBOOL AUdpLink::Tick( FLOAT DeltaTime, enum ELevelTick TickType )
{
	guard(AUdpLink::Tick);

	// Dispatch received data.
	if( GetSocket() )
	{
		char Buffer[512];
		sockaddr_in FromAddr;
		INT FromSize = sizeof(FromAddr);
		INT Count = recvfrom( GetSocket(), Buffer, ARRAY_COUNT(Buffer)-1, 0, (sockaddr*)&FromAddr, &FromSize );
		if( Count!=SOCKET_ERROR )
		{
			if( UdpMode==UDP_Text )
			{
				Buffer[Count]=0;
				FIpAddr Addr;
				Addr.Addr = ntohl( FromAddr.sin_addr.s_addr );
				Addr.Port = ntohs( FromAddr.sin_port );
				if( Count>0 )
					eventReceivedText( Addr, Buffer );
			}
			else
			{
				//!!eventReceivedBinary
			}
		}
	}

	// Update actor.
	return AActor::Tick( DeltaTime, TickType );

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
