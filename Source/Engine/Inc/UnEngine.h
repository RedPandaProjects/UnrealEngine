/*=============================================================================
	UnEngine.h: Unreal engine definition.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Unreal engine.
-----------------------------------------------------------------------------*/

class ENGINE_API UEngine : public USubsystem
{
	DECLARE_ABSTRACT_CLASS(UEngine,USubsystem,CLASS_Config|CLASS_Transient)

	// Subsystems.
	UClass*					GameRenderDeviceClass;
	UClass*					AudioDeviceClass;
	UClass*					ConsoleClass;
	UClass*					NetworkDriverClass;
	UClass*					LanguageClass;

	// Variables.
	class UPrimitive*		Cylinder;
	class UClient*			Client;
	class URenderBase*		Render;
	class UAudioSubsystem*	Audio;
	INT						TickCycles, GameCycles, ClientCycles;
	INT						CacheSizeMegs;
	UBOOL					UseSound;

	// Constructors.
	static void InternalClassInitializer( UClass* Class );
	UEngine();

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// UEngine interface.
	virtual void Init();
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );
	virtual void Flush();
	virtual UBOOL Key( UViewport* Viewport, EInputKey Key );
	virtual UBOOL InputEvent( UViewport* Viewport, EInputKey iKey, EInputAction State, FLOAT Delta=0.0 );
	virtual void Tick( FLOAT DeltaSeconds )=0;
	virtual void Draw( UViewport* Viewport, BYTE* HitData=NULL, INT* HitSize=NULL )=0;
	virtual void MouseDelta( UViewport* Viewport, DWORD Buttons, FLOAT DX, FLOAT DY )=0;
	virtual void MousePosition( UViewport* Viewport, DWORD Buttons, FLOAT X, FLOAT Y )=0;
	virtual void Click( UViewport* Viewport, DWORD Buttons, FLOAT X, FLOAT Y )=0;
	virtual void SetClientTravel( UPlayer* Viewport, const char* NextURL, UBOOL bURL, UBOOL bItems, ETravelType TravelType )=0;
	virtual INT ChallengeResponse( INT Challenge );
	virtual INT GetMaxTickRate();
	virtual void SetProgress( const char* Str1, const char* Str2, FLOAT Seconds );

	// Temporary!!
	virtual int edcamMode( UViewport* Viewport ) {return 0;}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
