/*=============================================================================
	AActor.h.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

	// Constructors.

	// UObject interface.
	void ProcessEvent( UFunction* Function, void* Parms );
	void ProcessState( FLOAT DeltaSeconds );
	UBOOL ProcessRemoteFunction( UFunction* Function, void* Parms, FFrame* Stack );
	void Serialize( FArchive& Ar );
	void InitExecution();
	void PostEditChange();

	// AActor interface.
	class APlayerPawn* GetPlayerPawn() const;
	class ULevel* GetLevel() const;
	UBOOL IsPlayer() const;
	UBOOL IsOwnedBy( const AActor *TestOwner ) const;
	FLOAT WorldLightRadius() const {return 25.0 * ((int)LightRadius+1);}
	FLOAT WorldSoundRadius() const {return 25.0 * ((int)SoundRadius+1);}
	FLOAT WorldVolumetricRadius() const {return 25.0 * ((int)VolumeRadius+1);}
	UBOOL IsBlockedBy( const AActor* Other ) const;
	UBOOL IsInZone( const AZoneInfo* Other ) const;
	UBOOL IsBasedOn( const AActor *Other ) const;
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	virtual void PostEditMove() {}
	virtual void PreRaytrace() {}
	virtual void PostRaytrace() {}
	virtual void Spawned() {}
	virtual FCoords ToLocal() const
	{
		return GMath.UnitCoords / Rotation / Location;
	}
	virtual FCoords ToWorld() const
	{
		return GMath.UnitCoords * Location * Rotation;
	}
	FLOAT LifeFraction()
	{
		return Clamp( 1.0 - LifeSpan / GetClass()->GetDefaultActor()->LifeSpan, 0.0, 1.0 );
	}
	FVector GetCylinderExtent() const {return FVector(CollisionRadius,CollisionRadius,CollisionHeight);}
	AActor* GetTopOwner();
	UBOOL IsPendingKill() {return bDeleteMe;}

	// AActor collision functions.
	UPrimitive* GetPrimitive() const;
	UBOOL IsOverlapping( const AActor *Other ) const;

	// AActor general functions.
	void BeginTouch(AActor *Other);
	void EndTouch(AActor *Other, UBOOL NoNotifySelf);
	void SetOwner( AActor *Owner );
	UBOOL IsBrush()       const;
	UBOOL IsStaticBrush() const;
	UBOOL IsMovingBrush() const;
	inline UBOOL IsAnimating()   const {
		return ( (AnimSequence != NAME_None) &&
			((AnimFrame>=0) ? (AnimRate!=0.0) : (TweenRate!=0.0)) );}
	void SetCollision( UBOOL NewCollideActors, UBOOL NewBlockActors, UBOOL NewBlockPlayers );
	void SetCollisionSize( FLOAT NewRadius, FLOAT NewHeight );
	void SetBase(AActor *NewBase, int bNotifyActor=1);
	FRotator GetViewRotation();

	// AActor audio.
	void MakeSound( USound *Sound, FLOAT Radius=0.f, FLOAT Volume=1.f, FLOAT Pitch=1.f );

	// Physics functions.
	void setPhysics(BYTE NewPhysics, AActor *NewFloor = NULL);
	void FindBase();
	virtual void performPhysics(FLOAT DeltaSeconds);
	void physProjectile(FLOAT deltaTime, INT Iterations);
	void processHitWall(FVector HitNormal, AActor *HitActor);
	void processLanded(FVector HitNormal, AActor *HitActor, FLOAT remainingTime, INT Iterations);
	void physFalling(FLOAT deltaTime, INT Iterations);
	void physRolling(FLOAT deltaTime, INT Iterations);
	void physicsRotation(FLOAT deltaTime);
	int fixedTurn(int current, int desired, int deltaRate); 
	void TwoWallAdjust(FVector &DesiredDir, FVector &Delta, FVector &HitNormal, FVector &OldHitNormal, FLOAT HitTime);
	void physPathing(FLOAT DeltaTime);
	void physMovingBrush(FLOAT DeltaTime);
	void physTrailer(FLOAT DeltaTime);
	int moveSmooth(FVector Delta);

	// AI functions.
	void CheckNoiseHearing(FLOAT Loudness);

	// Intrinsics.
	INTRINSIC(execRotationConst)
	INTRINSIC(execVectorConst)
	INTRINSIC(execStringToVector)
	INTRINSIC(execStringToRotation)
	INTRINSIC(execVectorToBool)
	INTRINSIC(execVectorToString)
	INTRINSIC(execVectorToRotation)
	INTRINSIC(execRotationToBool)
	INTRINSIC(execRotationToVector)
	INTRINSIC(execRotationToString)
	INTRINSIC(execPollSleep)
	INTRINSIC(execPollFinishAnim)
	INTRINSIC(execPollFinishInterpolation)

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
