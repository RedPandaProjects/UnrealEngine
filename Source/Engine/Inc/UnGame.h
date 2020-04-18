/*=============================================================================
	UnGame.h: Unreal game class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Unreal game engine.
-----------------------------------------------------------------------------*/

//
// The Unreal game engine.
//
class ENGINE_API UGameEngine : public UEngine
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UGameEngine,UEngine,CLASS_Config|CLASS_Transient)

	// Variables.
	ULevel*			GLevel;
	ULevel*			GEntry;
	UPendingLevel*	GPendingLevel;
	FURL			LastURL;
	char			ServerActors[16][96];
	char			ServerPackages[16][96];

	// Constructors.
	static void InternalClassInitializer( UClass* Class );
	UGameEngine();

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// UEngine interface.
	void Init();
	void Tick( FLOAT DeltaSeconds );
	void Draw( UViewport* Viewport, BYTE* HitData=NULL, INT* HitSize=NULL );
	UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );
	void MouseDelta( UViewport*, DWORD, FLOAT, FLOAT );
	void MousePosition( class UViewport*, DWORD, FLOAT, FLOAT );
	void Click( UViewport*, DWORD, FLOAT, FLOAT );
	void SetClientTravel( UPlayer* Viewport, const char* NextURL, UBOOL bURL, UBOOL bItems, ETravelType TravelType );
	INT GetMaxTickRate();
	INT ChallengeResponse( INT Challenge );
	void SetProgress( const char* Str1, const char* Str2, FLOAT Seconds );

	// UGameEngine interface.
	virtual UBOOL Browse( FURL URL, char* Error256 );
	virtual ULevel* LoadMap( const FURL& URL, UPendingLevel* Pending, char* Error256 );
	virtual void SaveGame( INT Position );
	virtual void CancelPending();
	virtual void PaintProgress();
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
