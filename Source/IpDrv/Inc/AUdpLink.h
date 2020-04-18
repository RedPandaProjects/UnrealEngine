/*=============================================================================
	AUdpLink.h.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

	AUdpLink();
	void Destroy();
	SOCKET& GetSocket() { return *(SOCKET*)&Socket;}
	UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
