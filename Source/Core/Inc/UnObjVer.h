/*=============================================================================
	UnObjVer.h: Unreal object version.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Unrealfile versions.
-----------------------------------------------------------------------------*/

// Package file information.
// Prevents incorrect files from being loaded.
#define PACKAGE_FILE_TAG 0x9E2A83C1

// The current Unrealfile version.
#define PACKAGE_FILE_VERSION 61

// The earliest file version which we can load with complete
// backwards compatibility. Must be at least PACKAGE_FILE_VERSION.
#define PACKAGE_MIN_VERSION 34

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
