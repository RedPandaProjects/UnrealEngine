/*=============================================================================
	IpDrv.cpp: Unreal TCP/IP driver.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Implemented by M.Michon
	* Created by Tim Sweeney.
=============================================================================*/

#include "IpDrvPrivate.h"

/*-----------------------------------------------------------------------------
	ATcpLink class implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(ATcpLink);

//
// Initializes winsock and the main socket
//
int InitWSAMain( ATcpLink *ATInst ) 
{
	// Attempts to initialize winsock and the main socket, returns 0 on success.
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1,1);
	LINGER ling;
	u_long ulCmdArg=1;

	// Initialize Winsock
	if( ATInst->bWSAInitialized == 0 )
	{
		if( WSAStartup( wVersionRequested, &wsaData ) != 0 )
		{
			// Error, WSA will not initialize, cannot do anything.  Failed.
			return 1;
		}
		else
		{
			ATInst->bWSAInitialized = 1;
		}
	}
	
	// Initialize Main Socket
	// Use stream sockets, TCP/IP
	if( ATInst->bSocketInitialized != 1 )
	{
		ATInst->MainSocket = socket( AF_INET, SOCK_STREAM, 0 );

		if( ATInst->MainSocket == INVALID_SOCKET ) 
		{
			// Error is WSAGetLastError().  Failed.
			return 1;
		}
		else
		{
			// Set to non-blocking Berkley mode.
			ioctlsocket( ATInst->MainSocket, FIONBIO, &ulCmdArg );	// 1 for non-blocking
			// Set lingering options for a Hard close
			ling.l_onoff  = 1;	// linger on
			ling.l_linger = 0;	// timeout in seconds
			setsockopt( ATInst->MainSocket, SOL_SOCKET, SO_LINGER, (LPSTR)&ling, sizeof(ling) );
			ATInst->bSocketInitialized = 1;
		}
	}

	return 0;
}

/*-----------------------------------------------------------------------------
	ATcpLink functions.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
ATcpLink::ATcpLink()
{
	guard(ATcpLink::ATcpLink);
	bSocketInitialized = bSocketBound = bWSAInitialized = 0;

	MainSocket = ConnectSocket = INVALID_SOCKET;
	
	LinkState = TCP_Closed;
	LinkMode  = TMOD_Binary;
	
	if( InitWSAMain( this ) != 0 )
	{
		debugf( NAME_Log, "TcpLink: WSAInitialize failed (%i)", WSAGetLastError() );
	}
	else
	{
		debugf( NAME_Log, "TcpLink: WSAInitialize successful", WSAGetLastError() );
	}

	unguard;
}

//
// Destroy function called by the core.
//
void ATcpLink::Destroy()
{
	guard(ATcpLink::Destroy);
	AInfo::Destroy();

	if( ConnectSocket != INVALID_SOCKET )
		closesocket( (SOCKET)ConnectSocket );
	if( MainSocket != INVALID_SOCKET )
		closesocket( (SOCKET)MainSocket );

	unguard;
}

/*-----------------------------------------------------------------------------
	ATcpLink intrinsics.
-----------------------------------------------------------------------------*/

//
// Given a string containing an IP address or Domain name, returns 
// the corresponding IP address (if successful)
//
void ATcpLink::execGetIPByName( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execGetIPByName);
	P_GET_STRING( Domain );
	P_GET_STRING( IpAddr );
	P_FINISH;

	LPHOSTENT lpstHost;
	in_addr   inAddrStr;
	u_long lAddr = INADDR_ANY;

	// In the event the domain is already an ip address
	appSprintf( IpAddr, "%s", Domain );
	*(DWORD*)Result = 0;

	if( Domain != NULL )
	{
		// Check for dotted IP address string
		lAddr = inet_addr( Domain );

		// If not an address, then try to resolve it as a hostname
		if( (lAddr == INADDR_NONE) &&
			(strcmp( Domain, "255.255.255.255" ) != 0) )
		{
			lpstHost = gethostbyname( Domain );
			if( lpstHost )
			{
				// success
				inAddrStr.S_un.S_addr = *((u_long FAR *)lpstHost->h_addr);
				//debugf( NAME_Log, "gethostbyname value=%u.%u.%u.%u", 
				//	     (u_long)inAddrStr.S_un.S_un_b.s_b1, (u_long)inAddrStr.S_un.S_un_b.s_b2,
				//		 (u_long)inAddrStr.S_un.S_un_b.s_b3, (u_long)inAddrStr.S_un.S_un_b.s_b4 );
				appSprintf( IpAddr, "%u.%u.%u.%u", 
					   		(u_long)inAddrStr.S_un.S_un_b.s_b1, (u_long)inAddrStr.S_un.S_un_b.s_b2,
							(u_long)inAddrStr.S_un.S_un_b.s_b3, (u_long)inAddrStr.S_un.S_un_b.s_b4 );

				//IpAddr = inet_ntoa( inAddrStr );
				*(DWORD*)Result = 1;
			}
			else
			{
				*(DWORD*)Result = 0;
			}
		}

	}

	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execGetIPByName );

//
// Mangles the given integer by a function, for handshaking purposes
//
void ATcpLink::execEncrypt( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execEncrypt);
	P_GET_INT( Key );
	P_FINISH;

	*(DWORD*)Result = ((Key % (Key*3/4))^2) % (Key/2);
	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execEncrypt );

//
// Listen for an incoming connection.
//
void ATcpLink::execListen( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execListen);
	P_GET_INT( OpenPort );
	P_FINISH;

	SOCKADDR_IN addr;
	fd_set		readFDS, exceptFDS;
	timeval		waitTime={0,0};
	int         numSockets, i;

	// Check that Winsock and the Main socket are initialized

	// A socket must exist, or be created.
	// Call winsock listen() function in non-blocking mode

	if( bWSAInitialized && bSocketInitialized )
	{
		if( ConnectSocket == INVALID_SOCKET )
		{
			if( bSocketBound == 0 )
			{
				// Bind the main socket to a port
				addr.sin_family	= AF_INET;
				addr.sin_port	= htons( OpenPort );
				addr.sin_addr.s_addr = htonl(INADDR_ANY);

				if( bind( (SOCKET)MainSocket, (LPSOCKADDR)&addr, sizeof(addr) ) == SOCKET_ERROR ) 
				{ 
					debugf( NAME_Log, "TcpLink: Cannot bind socket to port (%i)", WSAGetLastError() );
					*(DWORD*)Result = 0;
					return; 
				}

				// Up to 5 connections may be queued up before
				// the server application processes them.
				if( listen( (SOCKET)MainSocket, 5 ) == SOCKET_ERROR ) 
				{ 
					debugf( NAME_Log, "TcpLink: Cannot set up listen parameters (%i)", WSAGetLastError() );
					*(DWORD*)Result = 0;
					return;
				}
				bSocketBound = 1;
			}
			
			for(i=0; i<200; i++)
			{
				// Check for readability using select()
				FD_ZERO( &readFDS );
				FD_SET( (SOCKET)MainSocket, &readFDS );
				FD_ZERO( &exceptFDS );
				FD_SET( (SOCKET)MainSocket, &exceptFDS );
	
				numSockets = select( 0, &readFDS, NULL, &exceptFDS, &waitTime );
	
				if( numSockets == SOCKET_ERROR )
				{
					// BAD Exception occured.
					*(DWORD*)Result = 0;
					return;
				}
				else if( FD_ISSET( (SOCKET)MainSocket, &readFDS ) != 0 )
				{
					// The socket is readable, meaning that an accept
					// will be successful in this state.
	
					// Try Accepting a connection
					ConnectSocket = accept( (SOCKET)MainSocket, NULL, NULL );
	
					if( ConnectSocket == INVALID_SOCKET )
					{
		 				// What the hell???
						// No response.
						// error should be WSAEWOULDBLOCK if nothing is trying to reach this
						if( WSAGetLastError() == WSAEWOULDBLOCK )
						{
							//debugf( NAME_Log, "TcpLink: Error, no response");
							*(DWORD*)Result = 1;
							return;
						}
						else
						{
							// Something bad happened.  Perhaps the network subsystem is down.
							//debugf( NAME_Log, "TcpLink: Error attempting connect (%i)", WSAGetLastError() );
							*(DWORD*)Result = 0;
							return;
						}
					}
					else
					{
						// Connection established.
						//debugf( NAME_Log, "TcpLink: Connection established, socket=%i", ConnectSocket);
						LinkState = TCP_Open;
						Port = OpenPort;
						eventAccepted();
						*(DWORD*)Result = 1;
						return;
					}
				}
				else if( FD_ISSET( (SOCKET)MainSocket, &exceptFDS ) != 0 )
				{
					// Exception occured.
					*(DWORD*)Result = 0;
					return;
				}
				else
				{
					//debugf( NAME_Log, "TcpLink: No response");
					// Try again through the loop
					*(DWORD*)Result = 1;
				}
			}
			*(DWORD*)Result = 1;
			return;
		}
		else
		{
			// Already connected!! Do nothing.
			//debugf( NAME_Log, "TcpLink: Error: Already connected.");
			*(DWORD*)Result = 0;
			return;
		}
	}
	else
	{
		if( InitWSAMain( this ) != 0 )
		{
			// Error is WSAGetLastError()
			debugf( NAME_Log, "TcpLink: WSAInitialize failed (%i)", WSAGetLastError() );			
			*(DWORD*)Result = 0;
			return;
		}
	}

	*(DWORD*)Result = 0;
	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execListen );

//
// Try to connect to a remote host.
//
void ATcpLink::execOpen( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execOpen);
	P_GET_STRING( OpenURL );
	P_GET_INT   ( OpenPort );
	P_FINISH;

	SOCKADDR_IN addrServer;
	fd_set		writeFDS, exceptFDS;
	timeval		waitTime={0,0};
	int         numSockets, errNum, i;

	// Check that winsock and the main socket are initialized
	if( bWSAInitialized && bSocketInitialized )
	{
		if( ConnectSocket == INVALID_SOCKET )
		{
			addrServer.sin_family	   = AF_INET;
			addrServer.sin_port	 	   = htons( OpenPort );
			addrServer.sin_addr.s_addr = inet_addr( OpenURL );

			if( connect( (SOCKET)MainSocket, (LPSOCKADDR)&addrServer, sizeof(addrServer)) == SOCKET_ERROR )
			{
				errNum = WSAGetLastError();
				if( errNum == WSAEWOULDBLOCK )
				{
					for( i=0; i<200; i++ )
					{
						// Check if the socket is writeable, which would indicate
						// that connection is successful.
						FD_ZERO( &writeFDS );
						FD_SET( (SOCKET)MainSocket, &writeFDS );
						FD_ZERO( &exceptFDS );
						FD_SET( (SOCKET)MainSocket, &exceptFDS );
						numSockets = select( 0, NULL, &writeFDS, &exceptFDS, &waitTime );
						
						if( numSockets == SOCKET_ERROR )
						{
							// BAD Exception
							*(DWORD*)Result = 0;
							return;
						}
						else if( FD_ISSET( (SOCKET)MainSocket, &writeFDS ) != 0 )
						{
							// The socket we passed is writable, meaning connect.
							//debugf( NAME_Log, "Connected!." );
							LinkState = TCP_Open;
							ConnectSocket = MainSocket;		// For consistency
	
							if( strlen( OpenURL ) < 80 )
								memcpy( URL, OpenURL, strlen( OpenURL ) );
							Port = OpenPort;
							eventConnected();
							*(DWORD*)Result = 1;
							return;
						}
						else if( FD_ISSET( (SOCKET)MainSocket, &exceptFDS ) != 0 )
						{
							// Exception occured
							*(DWORD*)Result = 0;
							return;
						}
						else
						{
							// The socket wasn't writable, don't understand why
							// Try again
							*(DWORD*)Result = 1;
						}
					}
					return;
				}
				else if( errNum == WSAEISCONN )
				{
					// Connect successful
					//debugf( NAME_Log, "Connected!." );
					LinkState = TCP_Open;
					ConnectSocket = MainSocket;		// For consistency

					if( strlen( OpenURL ) < 80 )
						memcpy( URL, OpenURL, strlen( OpenURL ) );
					Port = OpenPort;
					eventConnected();
					*(DWORD*)Result = 1;
					return;
				}
			}
			else
			{
				// Connect successful
				//debugf( NAME_Log, "Connected!." );
				LinkState = TCP_Open;
				ConnectSocket = MainSocket;		// For consistency

				if( strlen( OpenURL ) < 80 )
					memcpy( URL, OpenURL, strlen( OpenURL ) );
				Port = OpenPort;
				eventConnected();
				*(DWORD*)Result = 1;
			}
			
			// Unable to connect
			//debugf( NAME_Log, "TcpLink: Unable to connect.");
			*(DWORD*)Result = 1;
			return;
		}
		else
		{
			// Already connected!! Do nothing.
			//debugf( NAME_Log, "TcpLink: Error: Already connected.");
			*(DWORD*)Result = 0;
			return;
		}
	}
	else
	{
		if( InitWSAMain( this ) != 0 )
		{
			// Error is WSAGetLastError()
			debugf( NAME_Log, "TcpLink: WSAInitialize failed (%i)", WSAGetLastError() );			
			*(DWORD*)Result = 0;
			return;
		}
	}

	*(DWORD*)Result = 0;
	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execOpen );

//
// Close the connection.
//
void ATcpLink::execClose( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execClose);
	P_FINISH;

	if( LinkState == TCP_Open )
	{
		// Close the connection
		if( MainSocket == ConnectSocket ) MainSocket = INVALID_SOCKET;

		if( closesocket( (SOCKET)ConnectSocket ) == SOCKET_ERROR )
		{
			// Error is WSAGetLastError
			debugf( NAME_Log, "TcpLink: Error while attempting to close connection socket (%i)", WSAGetLastError() );
		}
		else
		{
			// Reset variables
			bSocketBound = 0;
			if( MainSocket != INVALID_SOCKET )
			{
				if( closesocket( (SOCKET)MainSocket ) == SOCKET_ERROR )
				{
					// Error is WSAGetLastError
					debugf( NAME_Log, "TcpLink: Error while attempting to close main socket (%i)", WSAGetLastError() );
				}
			}
			bSocketInitialized = 0;
			MainSocket = ConnectSocket = INVALID_SOCKET;
			LinkState = TCP_Closed;
			URL[0]=NULL;
			Port=0;
		}
	}

	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execClose );

//
// Send text.
//
void ATcpLink::execSendText( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execSendText);
	P_GET_STRING( Str );
	P_GET_INT( Offset );
	P_FINISH;
	
	int NumSent, StringLen = strlen((char*)Str)+1;		// add 1, strlen does not include NULL

	if( LinkState == TCP_Open )
	{
		if( Offset < StringLen )
		{
			NumSent = send( (SOCKET)ConnectSocket, (char *)(Str+Offset), StringLen-Offset, 0);

			if( NumSent == SOCKET_ERROR )
			{
				NumSent = 0;
				// Could not send.
				if( WSAGetLastError() == WSAEWOULDBLOCK )
				{
					// Other end not synchronized.
				}
				else
				{
					// Something more serious
					//debugf( NAME_Log, "TcpLink: Error while attempting to send data (%i)", WSAGetLastError() );
				}

			}
			//debugf( NAME_Log, "TcpLink: Sent %i characters. Stringlen=%i", NumSent, StringLen );
			*(DWORD*)Result = NumSent;
			return;
		}
	}

	*(DWORD*)Result = 0;
	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execSendText );

//
// Read text
//
void ATcpLink::execReadText( FFrame& Stack, BYTE*& Result )
{
	// Returns the number of characters recieved, not including
	// the terminating NULL.
	
	guard(ATcpLink::execReadBinary);
	P_GET_STRING( Str );
	P_GET_INT( ReadLen );
	P_FINISH;

	int numRcvd;

	if( LinkState == TCP_Open )
	{
		// Assume that ReadLen is less than the length of the buffer
		numRcvd = recv( (SOCKET)ConnectSocket, (char *)Str, ReadLen, 0 );
		if( numRcvd == SOCKET_ERROR ) 
			numRcvd = 0;
		// Add a terminating NULL to the end of the string
		Str[numRcvd] = NULL;
		*(DWORD*)Result = numRcvd;
		return;
	}

	// Make sure it's a valid string before returning it
	Str[0] = NULL;
	*(DWORD*)Result = 0;
	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execReadText );

//
// Send raw binary data.
//
void ATcpLink::execSendBinary( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execSendBinary);
	P_GET_INT(Count);
	//P_GET_STRING_REF(out byte b[240]);
	P_FINISH;

	// This is currently broken because UnrealScript doesn't
	// support passing arrays to intrinsics.

	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execSendBinary );

//
// Extract.
//
void ATcpLink::execReadBinary( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execReadBinary);
	P_GET_INT(Count);
	//P_GET_STRING(byte b[240]);
	P_FINISH;

	// This is currently broken because UnrealScript doesn't
	// support passing arrays to intrinsics.

	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execReadBinary );


//
// Return last error which occured with Winsock.
//
void ATcpLink::execGetLastError( FFrame& Stack, BYTE*& Result )
{
	guard(ATcpLink::execGetLastError);
	P_FINISH;
	*(DWORD*)Result = WSAGetLastError();
	unguardexec;
}
AUTOREGISTER_INTRINSIC( ATcpLink, INDEX_NONE, execGetLastError );


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
