/*=============================================================================
	UnCamMgr.cpp: Unreal viewport manager, generic implementation.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	UClient implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UClient::UClient()
{
	guard(UClient::UClient);

	// Defaults.
	ViewportX	= 320;
	ViewportY	= 200;
	Brightness	= 0.5;
	MipFactor	= 1.0;

	// Hook in.
	UBitmap::Client = this;

	unguard;
}
void UClient::PostEditChange()
{
	guard(UClient::PostEditChange);
	Super::PostEditChange();
	Brightness = Clamp(Brightness,0.f,1.f);
	MipFactor = Clamp(MipFactor,-3.f,3.f);
	unguard;
}
void UClient::InternalClassInitializer( UClass* Class )
{
	guard(UClient::InternalClassInitializer);
	if( appStricmp( Class->GetName(), "Client" )==0 )
	{
		new(Class,"ViewportX",			RF_Public)UIntProperty  (CPP_PROPERTY(ViewportX         ), "Client",  CPF_Config );
		new(Class,"ViewportY",			RF_Public)UIntProperty  (CPP_PROPERTY(ViewportY         ), "Client",  CPF_Config );
		new(Class,"MipFactor",			RF_Public)UFloatProperty(CPP_PROPERTY(MipFactor         ), "Client",  CPF_Config );
		new(Class,"Brightness",			RF_Public)UFloatProperty(CPP_PROPERTY(Brightness        ), "Display", CPF_Config );
		new(Class,"CaptureMouse",		RF_Public)UBoolProperty (CPP_PROPERTY(CaptureMouse      ), "Display", CPF_Config );
		new(Class,"CurvedSurfaces",     RF_Public)UBoolProperty (CPP_PROPERTY(CurvedSurfaces    ), "Display", CPF_Config );
		new(Class,"LowDetailTextures",  RF_Public)UBoolProperty (CPP_PROPERTY(LowDetailTextures ), "Display", CPF_Config );
		new(Class,"ScreenFlashes",      RF_Public)UBoolProperty (CPP_PROPERTY(ScreenFlashes     ), "Display", CPF_Config );
		new(Class,"NoLighting",         RF_Public)UBoolProperty (CPP_PROPERTY(NoLighting        ), "Display", CPF_Config );
	}
	unguard;
}
void UClient::Destroy()
{
	guard(UClient::Destroy);
	UBitmap::Client = NULL;
	Super::Destroy();
	unguard;
}

//
// Command line.
//
UBOOL UClient::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(UClient::Exec);
	if( ParseCommand(&Cmd,"BRIGHTNESS") )
	{
		if( (Brightness+=0.1) >= 1.0 )
			Brightness = 0;
		Engine->Flush();
		SaveConfig();
		Out->Logf( NAME_Log, "Brightness level %i/10", (int)(Brightness*10.0)+1 );
		return 1;
	}
	else return 0;
	unguard;
}

//
// Init.
//
void UClient::Init( UEngine* InEngine )
{
	guard(UClient::Init);
	Engine = InEngine;
	unguard;
}

//
// Flush.
//
void UClient::Flush()
{
	guard(UClient::Flush);

	for( INT i=0; i<Viewports.Num(); i++ )
		if( Viewports(i)->RenDev )
			Viewports(i)->RenDev->Flush();

	unguard;
}

//
// Serializer.
//
void UClient::Serialize( FArchive& Ar )
{
	guard(UClient::Serialize);
	UObject::Serialize( Ar );

	// Only serialize objects, since this can't be loaded or saved.
	Ar << Viewports;

	unguard;
}

IMPLEMENT_CLASS(UClient);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
