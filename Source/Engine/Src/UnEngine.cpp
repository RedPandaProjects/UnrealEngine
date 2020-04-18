/*=============================================================================
	UnEngine.cpp: Unreal engine main.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Object class implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UEngine);
IMPLEMENT_CLASS(URenderBase);
IMPLEMENT_CLASS(URenderDevice);

/*-----------------------------------------------------------------------------
	Engine init and exit.
-----------------------------------------------------------------------------*/

//
// Construct the engine.
//
UEngine::UEngine()
{
	guard(UEngine::UEngine);
	unguard;
}

//
// Init class.
//
void UEngine::InternalClassInitializer( UClass* Class )
{
	guard(UEngine::InternalClassInitializer);
	if( appStricmp(Class->GetName(),"Engine")==0 )
	{
		new(Class,"CacheSizeMegs",      RF_Public)UIntProperty (CPP_PROPERTY(CacheSizeMegs      ), "Settings", CPF_Config );
		new(Class,"UseSound",           RF_Public)UBoolProperty(CPP_PROPERTY(UseSound           ), "Settings", CPF_Config );
	}
	unguard;
}

// Register names.
#define NAMES_ONLY
#define DECLARE_NAME(name) ENGINE_API FName ENGINE_##name;
#include "EngineClasses.h"
#undef DECLARE_NAME
#undef NAMES_ONLY

//
// Initialize the engine.
//
void UEngine::Init()
{
	guard(UEngine::Init);

	#define NAMES_ONLY
	#define DECLARE_NAME(name) ENGINE_##name = FName(#name,FNAME_Intrinsic);
	#include "EngineClasses.h"
	#undef DECLARE_NAME
	#undef NAMES_ONLY

	// Subsystems.
	FURL::Init();
	GCache.Init( 1024 * 1024 * Clamp(CacheSizeMegs,1,1024), 4096 );

	// Objects.
	Cylinder = new UPrimitive;

	// Add to root.
	GObj.AddToRoot( this );

	// Init audio.
	if
	(	UseSound
	&&	GIsClient
	&&	!ParseParam(appCmdLine(),"NOSOUND") )
	{
		UClass* AudioClass = GObj.LoadClass( UAudioSubsystem::StaticClass, NULL, "ini:Engine.Engine.AudioDevice", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
		Audio = ConstructClassObject<UAudioSubsystem>( AudioClass );
		if( !Audio->Init() )
		{
			debugf( NAME_Log, "Audio initialization failed" );
			delete Audio;
			Audio = NULL;
		}
	}

	debugf( NAME_Init, "Unreal engine initialized" );
	unguard;
}

//
// Exit the engine.
//
void UEngine::Destroy()
{
	guard(UEngine::Destroy);

	// Remove from root.
	GObj.RemoveFromRoot( this );

	// Shut down all subsystems.
	if( Audio )
	{
		delete Audio;
		Audio = NULL;
	}
	if( Render )
	{
		delete Render;
		Render = NULL;
	}
	if( Client )
	{
		delete Client;
		Client = NULL;
	}

	debugf( NAME_Exit, "Engine shut down" );

	USubsystem::Destroy();
	unguard;
}

//
// Flush all caches.
//
void UEngine::Flush()
{
	guard(UEngine::Flush);

	GCache.Flush();
	if( Client )
		Client->Flush();

	unguard;
}

//
// Tick rate.
//
INT UEngine::GetMaxTickRate()
{
	guard(UEngine::GetMaxTickRate);
	return 0;
	unguard;
}

//
// Progress indicator.
//
void UEngine::SetProgress( const char* Str1, const char* Str2, FLOAT Seconds )
{
	guard(UEngine::SetProgress);
	unguard;
}

//
// Serialize.
//
void UEngine::Serialize( FArchive& Ar )
{
	guard(UGameEngine::Serialize);

	USubsystem::Serialize( Ar );
	Ar << Cylinder << Client << Render << Audio;

	unguardobj;
}

/*-----------------------------------------------------------------------------
	Input.
-----------------------------------------------------------------------------*/

//
// This always going to be the last exec handler in the chain. It
// handles passing the command to all other global handlers.
//
UBOOL UEngine::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(UEngine::Exec);
	const char* Str = Cmd;

	// See if any other subsystems claim the command.
	if( GObj.Exec					(Cmd,Out) ) return 1;
	if( GCache.Exec					(Cmd,Out) ) return 1;
	if( GExecHook && GExecHook->Exec(Cmd,Out) ) return 1;
	if( GSystem && GSystem->Exec	(Cmd,Out) ) return 1;
	if( Client  && Client->Exec		(Cmd,Out) ) return 1;
	if( Render  && Render->Exec		(Cmd,Out) ) return 1;
	if( Audio   && Audio->Exec		(Cmd,Out) ) return 1;

	// Handle engine command line.
	if( ParseCommand(&Str,"FLUSH") )
	{
		Flush();
		Out->Log( "Flushed engine caches" );
		return 1;
	}
	else return 0;
	unguard;
}

//
// Key handler.
//
UBOOL UEngine::Key( UViewport* Viewport, EInputKey Key )
{
	guard(UEngine::Key);
	return Viewport->Console && Viewport->Console->eventKeyType( Key );
	unguard;
}

//
// Input event handler.
//
int	UEngine::InputEvent( UViewport* Viewport, EInputKey iKey, EInputAction State, FLOAT Delta )
{
	guard(UEngine::InputEvent);
	if( Viewport->Console && Viewport->Console->eventKeyEvent( iKey, State, Delta ) )
	{
		// Player console handled it.
		return 1;
	}
	else if( Viewport->Input->Process( Viewport->Console ? (FOutputDevice&)*Viewport->Console : (FOutputDevice&)*GSystem, iKey, State, Delta ) )
	{
		// Input system handled it.
		return 1;
	}
	else
	{
		// Nobody handled it.
		return 0;
	}
	unguard;
}

INT UEngine::ChallengeResponse( INT Challenge )
{
	guard(UEngine::ChallengeResponse);
	return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
