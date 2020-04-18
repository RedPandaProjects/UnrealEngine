/*===========================================================================
	C++ class definitions exported from UnrealScript.

   This is automatically generated using 'Unreal.exe -make -h'
   DO NOT modify this manually! Edit the corresponding .uc files instead!
===========================================================================*/
#pragma pack (push,4)

#ifndef ENGINE_API
#define ENGINE_API DLL_IMPORT
#endif

#ifndef NAMES_ONLY
#define DECLARE_NAME(name) extern ENGINE_API FName ENGINE_##name;
#endif

DECLARE_NAME(Spawned)
DECLARE_NAME(Destroyed)
DECLARE_NAME(GainedChild)
DECLARE_NAME(LostChild)
DECLARE_NAME(Trigger)
DECLARE_NAME(UnTrigger)
DECLARE_NAME(Timer)
DECLARE_NAME(HitWall)
DECLARE_NAME(Falling)
DECLARE_NAME(Landed)
DECLARE_NAME(ZoneChange)
DECLARE_NAME(Touch)
DECLARE_NAME(UnTouch)
DECLARE_NAME(Bump)
DECLARE_NAME(BaseChange)
DECLARE_NAME(Attach)
DECLARE_NAME(Detach)
DECLARE_NAME(ActorEntered)
DECLARE_NAME(ActorLeaving)
DECLARE_NAME(KillCredit)
DECLARE_NAME(AnimEnd)
DECLARE_NAME(EndedRotation)
DECLARE_NAME(InterpolateEnd)
DECLARE_NAME(EncroachingOn)
DECLARE_NAME(EncroachedBy)
DECLARE_NAME(FootZoneChange)
DECLARE_NAME(HeadZoneChange)
DECLARE_NAME(PainTimer)
DECLARE_NAME(SpeechTimer)
DECLARE_NAME(MayFall)
DECLARE_NAME(Tick)
DECLARE_NAME(PlayerTick)
DECLARE_NAME(Expired)
DECLARE_NAME(SeePlayer)
DECLARE_NAME(EnemyNotVisible)
DECLARE_NAME(HearNoise)
DECLARE_NAME(UpdateEyeHeight)
DECLARE_NAME(SpecialHandling)
DECLARE_NAME(BotDesireability)
DECLARE_NAME(Generate)
DECLARE_NAME(PlayerCalcView)
DECLARE_NAME(InvCalcView)
DECLARE_NAME(PlayerInput)
DECLARE_NAME(PlayerTimeout)
DECLARE_NAME(Possess)
DECLARE_NAME(UnPossess)
DECLARE_NAME(PreTeleport)
DECLARE_NAME(PostTeleport)
DECLARE_NAME(Login)
DECLARE_NAME(AcceptInventory)
DECLARE_NAME(ClientHearSound)
DECLARE_NAME(ClientMessage)
DECLARE_NAME(LongFall)
DECLARE_NAME(BeginEvent)
DECLARE_NAME(EndEvent)
DECLARE_NAME(KilledBy)
DECLARE_NAME(TakeDamage)
DECLARE_NAME(BeginPlay)
DECLARE_NAME(PreBeginPlay)
DECLARE_NAME(PostBeginPlay)
DECLARE_NAME(TravelPreAccept)
DECLARE_NAME(TravelPostAccept)
DECLARE_NAME(PreRender)
DECLARE_NAME(PostRender)
DECLARE_NAME(ForceGenerate)
DECLARE_NAME(InitGame)
DECLARE_NAME(Accept)
DECLARE_NAME(GetBeaconText)
DECLARE_NAME(SetInitialState)
DECLARE_NAME(PreLogin)
DECLARE_NAME(DetailChange)
DECLARE_NAME(ClientTravel)
DECLARE_NAME(ShowUpgradeMenu)

#ifndef NAMES_ONLY

enum EInputKey
{
    IK_None                 =0,
    IK_LeftMouse            =1,
    IK_RightMouse           =2,
    IK_Cancel               =3,
    IK_MiddleMouse          =4,
    IK_Unknown05            =5,
    IK_Unknown06            =6,
    IK_Unknown07            =7,
    IK_Backspace            =8,
    IK_Tab                  =9,
    IK_Unknown0A            =10,
    IK_Unknown0B            =11,
    IK_Unknown0C            =12,
    IK_Enter                =13,
    IK_Unknown0E            =14,
    IK_Unknown0F            =15,
    IK_Shift                =16,
    IK_Ctrl                 =17,
    IK_Alt                  =18,
    IK_Pause                =19,
    IK_CapsLock             =20,
    IK_Unknown15            =21,
    IK_Unknown16            =22,
    IK_Unknown17            =23,
    IK_Unknown18            =24,
    IK_Unknown19            =25,
    IK_Unknown1A            =26,
    IK_Escape               =27,
    IK_Unknown1C            =28,
    IK_Unknown1D            =29,
    IK_Unknown1E            =30,
    IK_Unknown1F            =31,
    IK_Space                =32,
    IK_PageUp               =33,
    IK_PageDown             =34,
    IK_End                  =35,
    IK_Home                 =36,
    IK_Left                 =37,
    IK_Up                   =38,
    IK_Right                =39,
    IK_Down                 =40,
    IK_Select               =41,
    IK_Print                =42,
    IK_Execute              =43,
    IK_PrintScrn            =44,
    IK_Insert               =45,
    IK_Delete               =46,
    IK_Help                 =47,
    IK_0                    =48,
    IK_1                    =49,
    IK_2                    =50,
    IK_3                    =51,
    IK_4                    =52,
    IK_5                    =53,
    IK_6                    =54,
    IK_7                    =55,
    IK_8                    =56,
    IK_9                    =57,
    IK_Unknown3A            =58,
    IK_Unknown3B            =59,
    IK_Unknown3C            =60,
    IK_Unknown3D            =61,
    IK_Unknown3E            =62,
    IK_Unknown3F            =63,
    IK_Unknown40            =64,
    IK_A                    =65,
    IK_B                    =66,
    IK_C                    =67,
    IK_D                    =68,
    IK_E                    =69,
    IK_F                    =70,
    IK_G                    =71,
    IK_H                    =72,
    IK_I                    =73,
    IK_J                    =74,
    IK_K                    =75,
    IK_L                    =76,
    IK_M                    =77,
    IK_N                    =78,
    IK_O                    =79,
    IK_P                    =80,
    IK_Q                    =81,
    IK_R                    =82,
    IK_S                    =83,
    IK_T                    =84,
    IK_U                    =85,
    IK_V                    =86,
    IK_W                    =87,
    IK_X                    =88,
    IK_Y                    =89,
    IK_Z                    =90,
    IK_Unknown5B            =91,
    IK_Unknown5C            =92,
    IK_Unknown5D            =93,
    IK_Unknown5E            =94,
    IK_Unknown5F            =95,
    IK_NumPad0              =96,
    IK_NumPad1              =97,
    IK_NumPad2              =98,
    IK_NumPad3              =99,
    IK_NumPad4              =100,
    IK_NumPad5              =101,
    IK_NumPad6              =102,
    IK_NumPad7              =103,
    IK_NumPad8              =104,
    IK_NumPad9              =105,
    IK_GreyStar             =106,
    IK_GreyPlus             =107,
    IK_Separator            =108,
    IK_GreyMinus            =109,
    IK_NumPadPeriod         =110,
    IK_GreySlash            =111,
    IK_F1                   =112,
    IK_F2                   =113,
    IK_F3                   =114,
    IK_F4                   =115,
    IK_F5                   =116,
    IK_F6                   =117,
    IK_F7                   =118,
    IK_F8                   =119,
    IK_F9                   =120,
    IK_F10                  =121,
    IK_F11                  =122,
    IK_F12                  =123,
    IK_F13                  =124,
    IK_F14                  =125,
    IK_F15                  =126,
    IK_F16                  =127,
    IK_F17                  =128,
    IK_F18                  =129,
    IK_F19                  =130,
    IK_F20                  =131,
    IK_F21                  =132,
    IK_F22                  =133,
    IK_F23                  =134,
    IK_F24                  =135,
    IK_Unknown88            =136,
    IK_Unknown89            =137,
    IK_Unknown8A            =138,
    IK_Unknown8B            =139,
    IK_Unknown8C            =140,
    IK_Unknown8D            =141,
    IK_Unknown8E            =142,
    IK_Unknown8F            =143,
    IK_NumLock              =144,
    IK_ScrollLock           =145,
    IK_Unknown92            =146,
    IK_Unknown93            =147,
    IK_Unknown94            =148,
    IK_Unknown95            =149,
    IK_Unknown96            =150,
    IK_Unknown97            =151,
    IK_Unknown98            =152,
    IK_Unknown99            =153,
    IK_Unknown9A            =154,
    IK_Unknown9B            =155,
    IK_Unknown9C            =156,
    IK_Unknown9D            =157,
    IK_Unknown9E            =158,
    IK_Unknown9F            =159,
    IK_LShift               =160,
    IK_RShift               =161,
    IK_LControl             =162,
    IK_RControl             =163,
    IK_UnknownA4            =164,
    IK_UnknownA5            =165,
    IK_UnknownA6            =166,
    IK_UnknownA7            =167,
    IK_UnknownA8            =168,
    IK_UnknownA9            =169,
    IK_UnknownAA            =170,
    IK_UnknownAB            =171,
    IK_UnknownAC            =172,
    IK_UnknownAD            =173,
    IK_UnknownAE            =174,
    IK_UnknownAF            =175,
    IK_UnknownB0            =176,
    IK_UnknownB1            =177,
    IK_UnknownB2            =178,
    IK_UnknownB3            =179,
    IK_UnknownB4            =180,
    IK_UnknownB5            =181,
    IK_UnknownB6            =182,
    IK_UnknownB7            =183,
    IK_UnknownB8            =184,
    IK_UnknownB9            =185,
    IK_Semicolon            =186,
    IK_Equals               =187,
    IK_Comma                =188,
    IK_Minus                =189,
    IK_Period               =190,
    IK_Slash                =191,
    IK_Tilde                =192,
    IK_UnknownC1            =193,
    IK_UnknownC2            =194,
    IK_UnknownC3            =195,
    IK_UnknownC4            =196,
    IK_UnknownC5            =197,
    IK_UnknownC6            =198,
    IK_UnknownC7            =199,
    IK_Joy1                 =200,
    IK_Joy2                 =201,
    IK_Joy3                 =202,
    IK_Joy4                 =203,
    IK_Joy5                 =204,
    IK_Joy6                 =205,
    IK_Joy7                 =206,
    IK_Joy8                 =207,
    IK_Joy9                 =208,
    IK_Joy10                =209,
    IK_Joy11                =210,
    IK_Joy12                =211,
    IK_Joy13                =212,
    IK_Joy14                =213,
    IK_Joy15                =214,
    IK_Joy16                =215,
    IK_UnknownD8            =216,
    IK_UnknownD9            =217,
    IK_UnknownDA            =218,
    IK_LeftBracket          =219,
    IK_Backslash            =220,
    IK_RightBracket         =221,
    IK_SingleQuote          =222,
    IK_UnknownDF            =223,
    IK_JoyX                 =224,
    IK_JoyY                 =225,
    IK_JoyZ                 =226,
    IK_JoyR                 =227,
    IK_MouseX               =228,
    IK_MouseY               =229,
    IK_MouseZ               =230,
    IK_MouseW               =231,
    IK_JoyU                 =232,
    IK_JoyV                 =233,
    IK_UnknownEA            =234,
    IK_UnknownEB            =235,
    IK_MouseWheelUp         =236,
    IK_MouseWheelDown       =237,
    IK_Unknown10E           =238,
    UK_Unknown10F           =239,
    IK_JoyPovUp             =240,
    IK_JoyPovDown           =241,
    IK_JoyPovLeft           =242,
    IK_JoyPovRight          =243,
    IK_UnknownF4            =244,
    IK_UnknownF5            =245,
    IK_Attn                 =246,
    IK_CrSel                =247,
    IK_ExSel                =248,
    IK_ErEof                =249,
    IK_Play                 =250,
    IK_Zoom                 =251,
    IK_NoName               =252,
    IK_PA1                  =253,
    IK_OEMClear             =254,
    IK_MAX                  =255,
};

enum EInputAction
{
    IST_None                =0,
    IST_Press               =1,
    IST_Hold                =2,
    IST_Release             =3,
    IST_Axis                =4,
    IST_MAX                 =5,
};

enum ETravelType
{
    TRAVEL_Absolute         =0,
    TRAVEL_Partial          =1,
    TRAVEL_Relative         =2,
    TRAVEL_MAX              =3,
};

enum EDodgeDir
{
    DODGE_None              =0,
    DODGE_Left              =1,
    DODGE_Right             =2,
    DODGE_Forward           =3,
    DODGE_Back              =4,
    DODGE_Active            =5,
    DODGE_Done              =6,
    DODGE_MAX               =7,
};

enum ELightEffect
{
    LE_None                 =0,
    LE_TorchWaver           =1,
    LE_FireWaver            =2,
    LE_WateryShimmer        =3,
    LE_Searchlight          =4,
    LE_SlowWave             =5,
    LE_FastWave             =6,
    LE_CloudCast            =7,
    LE_StaticSpot           =8,
    LE_Shock                =9,
    LE_Disco                =10,
    LE_Warp                 =11,
    LE_Spotlight            =12,
    LE_NonIncidence         =13,
    LE_Shell                =14,
    LE_OmniBumpMap          =15,
    LE_Interference         =16,
    LE_Cylinder             =17,
    LE_Rotor                =18,
    LE_Unused               =19,
    LE_MAX                  =20,
};

enum ELightType
{
    LT_None                 =0,
    LT_Steady               =1,
    LT_Pulse                =2,
    LT_Blink                =3,
    LT_Flicker              =4,
    LT_Strobe               =5,
    LT_BackdropLight        =6,
    LT_SubtlePulse          =7,
    LT_TexturePaletteOnce   =8,
    LT_TexturePaletteLoop   =9,
    LT_MAX                  =10,
};

enum EMusicTransition
{
    MTRAN_None              =0,
    MTRAN_Instant           =1,
    MTRAN_Segue             =2,
    MTRAN_Fade              =3,
    MTRAN_FastFade          =4,
    MTRAN_SlowFade          =5,
    MTRAN_MAX               =6,
};

enum ESoundSlot
{
    SLOT_None               =0,
    SLOT_Misc               =1,
    SLOT_Pain               =2,
    SLOT_Interact           =3,
    SLOT_Ambient            =4,
    SLOT_Talk               =5,
    SLOT_Interface          =6,
    SLOT_MAX                =7,
};

enum ERenderStyle
{
    STY_None                =0,
    STY_Normal              =1,
    STY_Masked              =2,
    STY_Translucent         =3,
    STY_Modulated           =4,
    STY_MAX                 =5,
};

enum EDrawType
{
    DT_None                 =0,
    DT_Sprite               =1,
    DT_Mesh                 =2,
    DT_Brush                =3,
    DT_RopeSprite           =4,
    DT_VerticalSprite       =5,
    DT_Terraform            =6,
    DT_SpriteAnimOnce       =7,
    DT_MAX                  =8,
};

enum ENetRole
{
    ROLE_None               =0,
    ROLE_DumbProxy          =1,
    ROLE_SimulatedProxy     =2,
    ROLE_AutonomousProxy    =3,
    ROLE_Authority          =4,
    ROLE_MAX                =5,
};

enum EPhysics
{
    PHYS_None               =0,
    PHYS_Walking            =1,
    PHYS_Falling            =2,
    PHYS_Swimming           =3,
    PHYS_Flying             =4,
    PHYS_Rotating           =5,
    PHYS_Projectile         =6,
    PHYS_Rolling            =7,
    PHYS_Interpolating      =8,
    PHYS_MovingBrush        =9,
    PHYS_Spider             =10,
    PHYS_Trailer            =11,
    PHYS_MAX                =12,
};

class ENGINE_API AActor : public UObject
{
public:
    DWORD bStatic:1;
    DWORD bHidden:1;
    DWORD bHiddenEd:1;
    DWORD bDirectional:1;
    DWORD bSelected:1;
    DWORD bMemorized:1;
    DWORD bHighlighted:1;
    DWORD bNoDelete:1;
    DWORD bAnimFinished:1;
    DWORD bAnimLoop:1;
    DWORD bAnimNotify:1;
    DWORD bTempEditor:1;
    DWORD bDeleteMe:1;
    DWORD bAssimilated:1;
    DWORD bTicked:1;
    DWORD bLightChanged:1;
    DWORD bDynamicLight:1;
    DWORD bTimerLoop:1;
    DWORD bEdLocked:1;
    DWORD bEdShouldSnap:1;
    DWORD bEdSnap:1;
    DWORD bCanTeleport:1;
    DWORD bIsSecretGoal:1;
    DWORD bIsKillGoal:1;
    DWORD bIsItemGoal:1;
    DWORD bCollideWhenPlacing:1;
    DWORD bTravel:1;
    DWORD bMovable:1;
    DWORD bHighDetail:1;
    DWORD bStasis:1;
    DWORD bForceStasis:1;
    DWORD bIsPawn:1;
    BYTE Physics;
    BYTE Role;
    BYTE RemoteRole;
    class AActor* Owner;
    FName InitialState;
    FName Group;
    FLOAT TimerRate;
    FLOAT TimerCounter;
    FLOAT LifeSpan;
    FName AnimSequence;
    FLOAT AnimFrame;
    FLOAT AnimRate;
    FLOAT TweenRate;
    class ALevelInfo* Level;
    class ULevel* XLevel;
    FName Tag;
    FName Event;
    class AActor* Target;
    class APawn* Instigator;
    class AInventory* Inventory;
    class AActor* Base;
    FPointRegion Region;
    BYTE StandingCount;
    BYTE MiscNumber;
    BYTE LatentByte;
    INT LatentInt;
    FLOAT LatentFloat;
    class AActor* LatentActor;
    class AActor* Touching[4];
    class AActor* Deleted;
    INT CollisionTag;
    INT LightingTag;
    INT NetTag;
    INT OtherTag;
    INT ExtraTag;
    INT SpecialTag;
    FVector Location;
    FRotator Rotation;
    FVector OldLocation;
    FVector ColLocation;
    FVector Velocity;
    FVector Acceleration;
    DWORD bDifficulty0:1;
    DWORD bDifficulty1:1;
    DWORD bDifficulty2:1;
    DWORD bDifficulty3:1;
    DWORD bSinglePlayer:1;
    DWORD bNet:1;
    DWORD bNetSpecial:1;
    FLOAT OddsOfAppearing;
    BYTE DrawType;
    BYTE Style;
    class UTexture* Sprite;
    class UTexture* Texture;
    class UTexture* Skin;
    class UMesh* Mesh;
    class UModel* Brush;
    FLOAT DrawScale;
    FVector PrePivot;
    FLOAT ScaleGlow;
    BYTE AmbientGlow;
    BYTE Fatness;
    DWORD bUnlit:1;
    DWORD bNoSmooth:1;
    DWORD bParticles:1;
    DWORD bMeshEnviroMap:1;
    DWORD bMeshCurvy:1;
    DWORD bShadowCast:1;
    DWORD bOnlyOwnerSee:1;
    DWORD bIsMover:1;
    DWORD bAlwaysRelevant:1;
    DWORD bAlwaysTick:1;
    DWORD bHurtEntry:1;
    BYTE SoundRadius;
    BYTE SoundVolume;
    BYTE SoundPitch;
    class USound* AmbientSound;
    FLOAT TransientSoundVolume;
    FLOAT CollisionRadius;
    FLOAT CollisionHeight;
    DWORD bCollideActors:1;
    DWORD bCollideWorld:1;
    DWORD bBlockActors:1;
    DWORD bBlockPlayers:1;
    DWORD bProjTarget:1;
    BYTE LightType;
    BYTE LightEffect;
    BYTE LightBrightness;
    BYTE LightHue;
    BYTE LightSaturation;
    BYTE LightRadius;
    BYTE LightPeriod;
    BYTE LightPhase;
    BYTE LightCone;
    BYTE VolumeBrightness;
    BYTE VolumeRadius;
    BYTE VolumeFog;
    DWORD bSpecialLit:1;
    DWORD bActorShadows:1;
    DWORD bCorona:1;
    DWORD bLensFlare:1;
    DWORD bBounce:1;
    DWORD bFixedRotationDir:1;
    DWORD bRotateToDesired:1;
    DWORD bInterpolating:1;
    DWORD bJustTeleported:1;
    BYTE DodgeDir;
    FLOAT Mass;
    FLOAT Buoyancy;
    FRotator RotationRate;
    FRotator DesiredRotation;
    FLOAT PhysAlpha;
    FLOAT PhysRate;
    FLOAT AnimLast;
    FLOAT AnimMinRate;
    FLOAT OldAnimRate;
    FPlane SimAnim;
    FLOAT NetPriority;
    DWORD bNetInitial:1;
    DWORD bNetOwner:1;
    DWORD bNetSee:1;
    DWORD bNetHear:1;
    DWORD bNetFeel:1;
    DWORD bSimulatedPawn:1;
    DWORD bNetOptional:1;
    void execVisibleCollidingActors( FFrame& Stack, BYTE*& Result );
    void execVisibleActors( FFrame& Stack, BYTE*& Result );
    void execRadiusActors( FFrame& Stack, BYTE*& Result );
    void execTraceActors( FFrame& Stack, BYTE*& Result );
    void execTouchingActors( FFrame& Stack, BYTE*& Result );
    void execBasedActors( FFrame& Stack, BYTE*& Result );
    void execChildActors( FFrame& Stack, BYTE*& Result );
    void execAllActors( FFrame& Stack, BYTE*& Result );
    void execGetNextSkin( FFrame& Stack, BYTE*& Result );
    void execGetMapName( FFrame& Stack, BYTE*& Result );
    void execPlayerCanSeeMe( FFrame& Stack, BYTE*& Result );
    void execMakeNoise( FFrame& Stack, BYTE*& Result );
    void execPlaySound( FFrame& Stack, BYTE*& Result );
    void execSetTimer( FFrame& Stack, BYTE*& Result );
    void execDestroy( FFrame& Stack, BYTE*& Result );
    void execSpawn( FFrame& Stack, BYTE*& Result );
    void execTrace( FFrame& Stack, BYTE*& Result );
    void execSetPhysics( FFrame& Stack, BYTE*& Result );
    void execFinishInterpolation( FFrame& Stack, BYTE*& Result );
    void execFinishAnim( FFrame& Stack, BYTE*& Result );
    void execGetAnimGroup( FFrame& Stack, BYTE*& Result );
    void execIsAnimating( FFrame& Stack, BYTE*& Result );
    void execTweenAnim( FFrame& Stack, BYTE*& Result );
    void execLoopAnim( FFrame& Stack, BYTE*& Result );
    void execPlayAnim( FFrame& Stack, BYTE*& Result );
    void execIsA( FFrame& Stack, BYTE*& Result );
    void execSetOwner( FFrame& Stack, BYTE*& Result );
    void execSetBase( FFrame& Stack, BYTE*& Result );
    void execMoveSmooth( FFrame& Stack, BYTE*& Result );
    void execSetRotation( FFrame& Stack, BYTE*& Result );
    void execSetLocation( FFrame& Stack, BYTE*& Result );
    void execMove( FFrame& Stack, BYTE*& Result );
    void execSetCollisionSize( FFrame& Stack, BYTE*& Result );
    void execSetCollision( FFrame& Stack, BYTE*& Result );
    void execSleep( FFrame& Stack, BYTE*& Result );
    void execError( FFrame& Stack, BYTE*& Result );
    void execRotRand( FFrame& Stack, BYTE*& Result );
    void execGetUnAxes( FFrame& Stack, BYTE*& Result );
    void execGetAxes( FFrame& Stack, BYTE*& Result );
    void execSubtractEqual_RotatorRotator( FFrame& Stack, BYTE*& Result );
    void execAddEqual_RotatorRotator( FFrame& Stack, BYTE*& Result );
    void execSubtract_RotatorRotator( FFrame& Stack, BYTE*& Result );
    void execAdd_RotatorRotator( FFrame& Stack, BYTE*& Result );
    void execDivideEqual_RotatorFloat( FFrame& Stack, BYTE*& Result );
    void execMultiplyEqual_RotatorFloat( FFrame& Stack, BYTE*& Result );
    void execDivide_RotatorFloat( FFrame& Stack, BYTE*& Result );
    void execMultiply_FloatRotator( FFrame& Stack, BYTE*& Result );
    void execMultiply_RotatorFloat( FFrame& Stack, BYTE*& Result );
    void execNotEqual_RotatorRotator( FFrame& Stack, BYTE*& Result );
    void execEqualEqual_RotatorRotator( FFrame& Stack, BYTE*& Result );
    void execMirrorVectorByNormal( FFrame& Stack, BYTE*& Result );
    void execVRand( FFrame& Stack, BYTE*& Result );
    void execInvert( FFrame& Stack, BYTE*& Result );
    void execNormal( FFrame& Stack, BYTE*& Result );
    void execVSize( FFrame& Stack, BYTE*& Result );
    void execSubtractEqual_VectorVector( FFrame& Stack, BYTE*& Result );
    void execAddEqual_VectorVector( FFrame& Stack, BYTE*& Result );
    void execDivideEqual_VectorFloat( FFrame& Stack, BYTE*& Result );
    void execMultiplyEqual_VectorVector( FFrame& Stack, BYTE*& Result );
    void execMultiplyEqual_VectorFloat( FFrame& Stack, BYTE*& Result );
    void execCross_VectorVector( FFrame& Stack, BYTE*& Result );
    void execDot_VectorVector( FFrame& Stack, BYTE*& Result );
    void execNotEqual_VectorVector( FFrame& Stack, BYTE*& Result );
    void execEqualEqual_VectorVector( FFrame& Stack, BYTE*& Result );
    void execGreaterGreater_VectorRotator( FFrame& Stack, BYTE*& Result );
    void execLessLess_VectorRotator( FFrame& Stack, BYTE*& Result );
    void execSubtract_VectorVector( FFrame& Stack, BYTE*& Result );
    void execAdd_VectorVector( FFrame& Stack, BYTE*& Result );
    void execDivide_VectorFloat( FFrame& Stack, BYTE*& Result );
    void execMultiply_VectorVector( FFrame& Stack, BYTE*& Result );
    void execMultiply_FloatVector( FFrame& Stack, BYTE*& Result );
    void execMultiply_VectorFloat( FFrame& Stack, BYTE*& Result );
    void execSubtract_PreVector( FFrame& Stack, BYTE*& Result );
    void eventTravelPostAccept()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_TravelPostAccept),NULL);
    }
    void eventTravelPreAccept()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_TravelPreAccept),NULL);
    }
    void eventSetInitialState()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_SetInitialState),NULL);
    }
    void eventPostBeginPlay()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_PostBeginPlay),NULL);
    }
    void eventPreBeginPlay()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_PreBeginPlay),NULL);
    }
    void eventBeginPlay()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_BeginPlay),NULL);
    }
    void eventPostTeleport(class ATeleporter* OutTeleporter)
    {
        struct {class ATeleporter* OutTeleporter; } Parms;
        Parms.OutTeleporter=OutTeleporter;
        ProcessEvent(FindFunctionChecked(ENGINE_PostTeleport),&Parms);
    }
    DWORD eventPreTeleport(class ATeleporter* InTeleporter)
    {
        struct {class ATeleporter* InTeleporter; DWORD ReturnValue; } Parms;
        Parms.InTeleporter=InTeleporter;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(ENGINE_PreTeleport),&Parms);
        return Parms.ReturnValue;
    }
    void eventTakeDamage(INT Damage, class APawn* EventInstigator, FVector HitLocation, FVector Momentum, FName DamageType)
    {
        struct {INT Damage; class APawn* EventInstigator; FVector HitLocation; FVector Momentum; FName DamageType; } Parms;
        Parms.Damage=Damage;
        Parms.EventInstigator=EventInstigator;
        Parms.HitLocation=HitLocation;
        Parms.Momentum=Momentum;
        Parms.DamageType=DamageType;
        ProcessEvent(FindFunctionChecked(ENGINE_TakeDamage),&Parms);
    }
    void eventKilledBy(class APawn* EventInstigator)
    {
        struct {class APawn* EventInstigator; } Parms;
        Parms.EventInstigator=EventInstigator;
        ProcessEvent(FindFunctionChecked(ENGINE_KilledBy),&Parms);
    }
    void eventEndedRotation()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_EndedRotation),NULL);
    }
    void eventInterpolateEnd(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_InterpolateEnd),&Parms);
    }
    void eventEncroachedBy(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_EncroachedBy),&Parms);
    }
    DWORD eventEncroachingOn(class AActor* Other)
    {
        struct {class AActor* Other; DWORD ReturnValue; } Parms;
        Parms.Other=Other;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(ENGINE_EncroachingOn),&Parms);
        return Parms.ReturnValue;
    }
    class AActor* eventSpecialHandling(class APawn* Other)
    {
        struct {class APawn* Other; class AActor* ReturnValue; } Parms;
        Parms.Other=Other;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(ENGINE_SpecialHandling),&Parms);
        return Parms.ReturnValue;
    }
    void eventKillCredit(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_KillCredit),&Parms);
    }
    void eventDetach(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_Detach),&Parms);
    }
    void eventAttach(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_Attach),&Parms);
    }
    void eventBaseChange()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_BaseChange),NULL);
    }
    void eventBump(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_Bump),&Parms);
    }
    void eventUnTouch(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_UnTouch),&Parms);
    }
    void eventTouch(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_Touch),&Parms);
    }
    void eventZoneChange(class AZoneInfo* NewZone)
    {
        struct {class AZoneInfo* NewZone; } Parms;
        Parms.NewZone=NewZone;
        ProcessEvent(FindFunctionChecked(ENGINE_ZoneChange),&Parms);
    }
    void eventLanded(FVector HitNormal)
    {
        struct {FVector HitNormal; } Parms;
        Parms.HitNormal=HitNormal;
        ProcessEvent(FindFunctionChecked(ENGINE_Landed),&Parms);
    }
    void eventFalling()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_Falling),NULL);
    }
    void eventHitWall(FVector HitNormal, class AActor* HitWall)
    {
        struct {FVector HitNormal; class AActor* HitWall; } Parms;
        Parms.HitNormal=HitNormal;
        Parms.HitWall=HitWall;
        ProcessEvent(FindFunctionChecked(ENGINE_HitWall),&Parms);
    }
    void eventTimer()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_Timer),NULL);
    }
    void eventEndEvent()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_EndEvent),NULL);
    }
    void eventBeginEvent()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_BeginEvent),NULL);
    }
    void eventUnTrigger(class AActor* Other, class APawn* EventInstigator)
    {
        struct {class AActor* Other; class APawn* EventInstigator; } Parms;
        Parms.Other=Other;
        Parms.EventInstigator=EventInstigator;
        ProcessEvent(FindFunctionChecked(ENGINE_UnTrigger),&Parms);
    }
    void eventTrigger(class AActor* Other, class APawn* EventInstigator)
    {
        struct {class AActor* Other; class APawn* EventInstigator; } Parms;
        Parms.Other=Other;
        Parms.EventInstigator=EventInstigator;
        ProcessEvent(FindFunctionChecked(ENGINE_Trigger),&Parms);
    }
    void eventTick(FLOAT DeltaTime)
    {
        struct {FLOAT DeltaTime; } Parms;
        Parms.DeltaTime=DeltaTime;
        ProcessEvent(FindFunctionChecked(ENGINE_Tick),&Parms);
    }
    void eventLostChild(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_LostChild),&Parms);
    }
    void eventGainedChild(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_GainedChild),&Parms);
    }
    void eventExpired()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_Expired),NULL);
    }
    void eventDestroyed()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_Destroyed),NULL);
    }
    void eventSpawned()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_Spawned),NULL);
    }
    void eventAnimEnd()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_AnimEnd),NULL);
    }
    DECLARE_CLASS(AActor,UObject,0)
    #include "AActor.h"
};

enum EIntelligence
{
    BRAINS_NONE             =0,
    BRAINS_REPTILE          =1,
    BRAINS_MAMMAL           =2,
    BRAINS_HUMAN            =3,
    BRAINS_MAX              =4,
};

enum EAttitude
{
    ATTITUDE_Fear           =0,
    ATTITUDE_Hate           =1,
    ATTITUDE_Frenzy         =2,
    ATTITUDE_Threaten       =3,
    ATTITUDE_Ignore         =4,
    ATTITUDE_Friendly       =5,
    ATTITUDE_Follow         =6,
    ATTITUDE_MAX            =7,
};

class ENGINE_API APawn : public AActor
{
public:
    DWORD bBehindView:1;
    DWORD bIsPlayer:1;
    DWORD bJustLanded:1;
    DWORD bUpAndOut:1;
    DWORD bIsWalking:1;
    DWORD bHitSlopedWall:1;
    DWORD bNeverSwitchOnPickup:1;
    DWORD bCanStrafe:1;
    DWORD bFixedStart:1;
    DWORD bReducedSpeed:1;
    DWORD bCanJump:1;
    DWORD bCanWalk:1;
    DWORD bCanSwim:1;
    DWORD bCanFly:1;
    DWORD bCanOpenDoors:1;
    DWORD bCanDoSpecial:1;
    DWORD bDrowning:1;
    DWORD bLOSflag:1;
    DWORD bFromWall:1;
    DWORD bHunting:1;
    DWORD bAvoidLedges:1;
    DWORD bJumpOffPawn:1;
    DWORD bShootSpecial:1;
    DWORD bAutoActivate:1;
    FLOAT SightCounter;
    FLOAT PainTime;
    FLOAT SpeechTime;
    FLOAT AvgPhysicsTime;
    FPointRegion FootRegion;
    FPointRegion HeadRegion;
    FLOAT MoveTimer;
    class AActor* MoveTarget;
    FVector Destination;
    FVector Focus;
    FLOAT DesiredSpeed;
    FLOAT MaxDesiredSpeed;
    FLOAT MeleeRange;
    FLOAT GroundSpeed;
    FLOAT WaterSpeed;
    FLOAT AirSpeed;
    FLOAT AccelRate;
    FLOAT JumpZ;
    FLOAT MaxStepHeight;
    FLOAT MinHitWall;
    BYTE Visibility;
    FLOAT Alertness;
    FLOAT Stimulus;
    FLOAT SightRadius;
    FLOAT PeripheralVision;
    FLOAT HearingThreshold;
    FVector LastSeenPos;
    FVector LastSeeingPos;
    class APawn* Enemy;
    CHAR PlayerName[20];
    CHAR TeamName[20];
    BYTE Team;
    class AWeapon* Weapon;
    class AWeapon* PendingWeapon;
    class AInventory* SelectedItem;
    FRotator ViewRotation;
    FVector WalkBob;
    FLOAT BaseEyeHeight;
    FLOAT EyeHeight;
    FVector Floor;
    FLOAT SplashTime;
    FLOAT OrthoZoom;
    FLOAT FovAngle;
    INT DieCount;
    INT ItemCount;
    INT KillCount;
    INT SecretCount;
    FLOAT score;
    INT Health;
    FName ReducedDamageType;
    FLOAT ReducedDamagePct;
    class UClass* DropWhenKilled;
    FLOAT UnderWaterTime;
    BYTE AttitudeToPlayer;
    BYTE Intelligence;
    FLOAT Skill;
    class AActor* SpecialGoal;
    FLOAT SpecialPause;
    FVector noise1spot;
    FLOAT noise1time;
    class APawn* noise1other;
    FLOAT noise1loudness;
    FVector noise2spot;
    FLOAT noise2time;
    class APawn* noise2other;
    FLOAT noise2loudness;
    FLOAT LastPainSound;
    class APawn* nextPawn;
    class USound* HitSound1;
    class USound* HitSound2;
    class USound* Land;
    class USound* Die;
    class USound* WaterStep;
    BYTE bZoom;
    BYTE bRun;
    BYTE bLook;
    BYTE bDuck;
    BYTE bSnapLevel;
    BYTE bStrafe;
    BYTE bFire;
    BYTE bAltFire;
    BYTE bExtra0;
    BYTE bExtra1;
    BYTE bExtra2;
    BYTE bExtra3;
    FLOAT CombatStyle;
    class ANavigationPoint* home;
    FName NextState;
    FName NextLabel;
    FLOAT SoundDampening;
    FLOAT DamageScaling;
    FName AlarmTag;
    FName SharedAlarmTag;
    class ADecoration* carriedDecoration;
    void execClientHearSound( FFrame& Stack, BYTE*& Result );
    void execStopWaiting( FFrame& Stack, BYTE*& Result );
    void execPickAnyTarget( FFrame& Stack, BYTE*& Result );
    void execPickTarget( FFrame& Stack, BYTE*& Result );
    void execRemovePawn( FFrame& Stack, BYTE*& Result );
    void execAddPawn( FFrame& Stack, BYTE*& Result );
    void execFindBestInventoryPath( FFrame& Stack, BYTE*& Result );
    void execWaitForLanding( FFrame& Stack, BYTE*& Result );
    void execFindStairRotation( FFrame& Stack, BYTE*& Result );
    void execPickWallAdjust( FFrame& Stack, BYTE*& Result );
    void execactorReachable( FFrame& Stack, BYTE*& Result );
    void execpointReachable( FFrame& Stack, BYTE*& Result );
    void execEAdjustJump( FFrame& Stack, BYTE*& Result );
    void execClearPaths( FFrame& Stack, BYTE*& Result );
    void execFindRandomDest( FFrame& Stack, BYTE*& Result );
    void execFindPathToward( FFrame& Stack, BYTE*& Result );
    void execFindPathTo( FFrame& Stack, BYTE*& Result );
    void execCanSee( FFrame& Stack, BYTE*& Result );
    void execLineOfSightTo( FFrame& Stack, BYTE*& Result );
    void execTurnToward( FFrame& Stack, BYTE*& Result );
    void execTurnTo( FFrame& Stack, BYTE*& Result );
    void execStrafeFacing( FFrame& Stack, BYTE*& Result );
    void execStrafeTo( FFrame& Stack, BYTE*& Result );
    void execMoveToward( FFrame& Stack, BYTE*& Result );
    void execMoveTo( FFrame& Stack, BYTE*& Result );
    void eventPainTimer()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_PainTimer),NULL);
    }
    void eventSpeechTimer()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_SpeechTimer),NULL);
    }
    void eventHeadZoneChange(class AZoneInfo* newHeadZone)
    {
        struct {class AZoneInfo* newHeadZone; } Parms;
        Parms.newHeadZone=newHeadZone;
        ProcessEvent(FindFunctionChecked(ENGINE_HeadZoneChange),&Parms);
    }
    void eventFootZoneChange(class AZoneInfo* newFootZone)
    {
        struct {class AZoneInfo* newFootZone; } Parms;
        Parms.newFootZone=newFootZone;
        ProcessEvent(FindFunctionChecked(ENGINE_FootZoneChange),&Parms);
    }
    void eventEnemyNotVisible()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_EnemyNotVisible),NULL);
    }
    void eventUpdateEyeHeight(FLOAT DeltaTime)
    {
        struct {FLOAT DeltaTime; } Parms;
        Parms.DeltaTime=DeltaTime;
        ProcessEvent(FindFunctionChecked(ENGINE_UpdateEyeHeight),&Parms);
    }
    void eventSeePlayer(class AActor* Seen)
    {
        struct {class AActor* Seen; } Parms;
        Parms.Seen=Seen;
        ProcessEvent(FindFunctionChecked(ENGINE_SeePlayer),&Parms);
    }
    void eventHearNoise(FLOAT Loudness, class AActor* NoiseMaker)
    {
        struct {FLOAT Loudness; class AActor* NoiseMaker; } Parms;
        Parms.Loudness=Loudness;
        Parms.NoiseMaker=NoiseMaker;
        ProcessEvent(FindFunctionChecked(ENGINE_HearNoise),&Parms);
    }
    void eventClientHearSound(class AActor* Actor, INT Id, class USound* S, FVector SoundLocation, FVector Parameters)
    {
        struct {class AActor* Actor; INT Id; class USound* S; FVector SoundLocation; FVector Parameters; } Parms;
        Parms.Actor=Actor;
        Parms.Id=Id;
        Parms.S=S;
        Parms.SoundLocation=SoundLocation;
        Parms.Parameters=Parameters;
        ProcessEvent(FindFunctionChecked(ENGINE_ClientHearSound),&Parms);
    }
    void eventLongFall()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_LongFall),NULL);
    }
    void eventPlayerTimeout()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_PlayerTimeout),NULL);
    }
    void eventClientMessage(const CHAR* S)
    {
        struct {CHAR S[255]; } Parms;
        appStrncpy(Parms.S,S,255);
        ProcessEvent(FindFunctionChecked(ENGINE_ClientMessage),&Parms);
    }
    void eventMayFall()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_MayFall),NULL);
    }
    DECLARE_CLASS(APawn,AActor,0|CLASS_Config)
    #include "APawn.h"
};

class ENGINE_API APlayerPawn : public APawn
{
public:
    class UPlayer* Player;
    CHAR Password[64];
    FLOAT DodgeClickTimer;
    FLOAT DodgeClickTime;
    FLOAT Bob;
    FLOAT bobtime;
    INT ShowFlags;
    INT RendMap;
    INT Misc1;
    INT Misc2;
    class AActor* ViewTarget;
    FVector FlashScale;
    FVector FlashFog;
    class AHUD* myHUD;
    class AScoreBoard* Scoring;
    class UClass* HUDType;
    class UClass* ScoringType;
    class UTextBuffer* CarryInfo;
    FLOAT DesiredFlashScale;
    FLOAT ConstantGlowScale;
    FVector DesiredFlashFog;
    FVector ConstantGlowFog;
    FLOAT DesiredFOV;
    class UMusic* Song;
    BYTE SongSection;
    BYTE CdTrack;
    BYTE Transition;
    FLOAT shaketimer;
    INT shakemag;
    FLOAT shakevert;
    FLOAT maxshake;
    FLOAT verttimer;
    class UClass* CarcassType;
    FLOAT MyAutoAim;
    FLOAT Handedness;
    class USound* JumpSound;
    DWORD bAdmin:1;
    DWORD bLookUpStairs:1;
    DWORD bSnapToLevel:1;
    DWORD bAlwaysMouseLook:1;
    DWORD bKeyboardLook:1;
    DWORD bWasForward:1;
    DWORD bWasBack:1;
    DWORD bWasLeft:1;
    DWORD bWasRight:1;
    DWORD bEdgeForward:1;
    DWORD bEdgeBack:1;
    DWORD bEdgeLeft:1;
    DWORD bEdgeRight:1;
    DWORD bIsCrouching:1;
    DWORD bShakeDir:1;
    DWORD bAnimTransition:1;
    DWORD bCountJumps:1;
    DWORD bIsTurning:1;
    DWORD bFrozen:1;
    DWORD bInvertMouse:1;
    DWORD bShowScores:1;
    DWORD bShowMenu:1;
    DWORD bSpecialMenu:1;
    DWORD bWokeUp:1;
    DWORD bPressedJump:1;
    DWORD bUpdatePosition:1;
    DWORD bDelayedCommand:1;
    DWORD bRising:1;
    DWORD bReducedVis:1;
    class UClass* SpecialMenu;
    CHAR DelayedCommand[255];
    FLOAT MouseSensitivity;
    FName WeaponPriority[20];
    INT NetSpeed;
    FLOAT SmoothMouseX;
    FLOAT SmoothMouseY;
    FLOAT aBaseX;
    FLOAT aBaseY;
    FLOAT aBaseZ;
    FLOAT aMouseX;
    FLOAT aMouseY;
    FLOAT aForward;
    FLOAT aTurn;
    FLOAT aStrafe;
    FLOAT aUp;
    FLOAT aLookUp;
    FLOAT aExtra4;
    FLOAT aExtra3;
    FLOAT aExtra2;
    FLOAT aExtra1;
    FLOAT aExtra0;
    class ASavedMove* SavedMoves;
    class ASavedMove* FreeMoves;
    FLOAT ClientNetMinDelta;
    FLOAT ServerNetMinDelta;
    FLOAT CurrentTimeStamp;
    FLOAT LastUpdateTime;
    CHAR ProgressMessage[128];
    CHAR ProgressMessageTwo[128];
    FLOAT ProgressTimeOut;
    void execAutonomousPhysics( FFrame& Stack, BYTE*& Result );
    void execUpdateURL( FFrame& Stack, BYTE*& Result );
    void execResetKeyboard( FFrame& Stack, BYTE*& Result );
    void execConsoleCommandResult( FFrame& Stack, BYTE*& Result );
    void execConsoleCommand( FFrame& Stack, BYTE*& Result );
    void execClientMessage( FFrame& Stack, BYTE*& Result );
    void execClientTravel( FFrame& Stack, BYTE*& Result );
    void eventPlayerCalcView(class AActor*& ViewActor, FVector& CameraLocation, FRotator& CameraRotation)
    {
        struct {class AActor* ViewActor; FVector CameraLocation; FRotator CameraRotation; } Parms;
        Parms.ViewActor=ViewActor;
        Parms.CameraLocation=CameraLocation;
        Parms.CameraRotation=CameraRotation;
        ProcessEvent(FindFunctionChecked(ENGINE_PlayerCalcView),&Parms);
        ViewActor=Parms.ViewActor;
        CameraLocation=Parms.CameraLocation;
        CameraRotation=Parms.CameraRotation;
    }
    void eventPlayerTick(FLOAT Time)
    {
        struct {FLOAT Time; } Parms;
        Parms.Time=Time;
        ProcessEvent(FindFunctionChecked(ENGINE_PlayerTick),&Parms);
    }
    void eventUnPossess()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_UnPossess),NULL);
    }
    void eventPossess()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_Possess),NULL);
    }
    void eventPlayerInput(FLOAT DeltaTime)
    {
        struct {FLOAT DeltaTime; } Parms;
        Parms.DeltaTime=DeltaTime;
        ProcessEvent(FindFunctionChecked(ENGINE_PlayerInput),&Parms);
    }
    void eventShowUpgradeMenu()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_ShowUpgradeMenu),NULL);
    }
    void eventPostRender(class UCanvas* Canvas)
    {
        struct {class UCanvas* Canvas; } Parms;
        Parms.Canvas=Canvas;
        ProcessEvent(FindFunctionChecked(ENGINE_PostRender),&Parms);
    }
    void eventPreRender(class UCanvas* Canvas)
    {
        struct {class UCanvas* Canvas; } Parms;
        Parms.Canvas=Canvas;
        ProcessEvent(FindFunctionChecked(ENGINE_PreRender),&Parms);
    }
    void eventClientTravel(const CHAR* URL, BYTE TravelType, DWORD bItems)
    {
        struct {CHAR URL[240]; BYTE TravelType; DWORD bItems; } Parms;
        appStrncpy(Parms.URL,URL,240);
        Parms.TravelType=TravelType;
        Parms.bItems=bItems;
        ProcessEvent(FindFunctionChecked(ENGINE_ClientTravel),&Parms);
    }
    DECLARE_CLASS(APlayerPawn,APawn,0|CLASS_Config)
    #include "APlayerPawn.h"
};

class ENGINE_API ACamera : public APlayerPawn
{
public:
    DECLARE_CLASS(ACamera,APlayerPawn,0|CLASS_Config)
    #include "ACamera.h"
};

class ENGINE_API AScout : public APawn
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AScout,APawn,0|CLASS_Config)
    NO_DEFAULT_CONSTRUCTOR(AScout)
};

enum ECsgOper
{
    CSG_Active              =0,
    CSG_Add                 =1,
    CSG_Subtract            =2,
    CSG_Intersect           =3,
    CSG_Deintersect         =4,
    CSG_MAX                 =5,
};

class ENGINE_API ABrush : public AActor
{
public:
    BYTE CsgOper;
    class UObject* UnusedLightMesh;
    FVector PostPivot;
    FScale MainScale;
    FScale PostScale;
    FScale TempScale;
    FColor BrushColor;
    INT PolyFlags;
    DWORD bColored:1;
    DECLARE_CLASS(ABrush,AActor,0)
    #include "ABrush.h"
};

enum EBumpType
{
    BT_PlayerBump           =0,
    BT_PawnBump             =1,
    BT_AnyBump              =2,
    BT_MAX                  =3,
};

enum EMoverGlideType
{
    MV_MoveByTime           =0,
    MV_GlideByTime          =1,
    MV_MAX                  =2,
};

enum EMoverEncroachType
{
    ME_StopWhenEncroach     =0,
    ME_ReturnWhenEncroach   =1,
    ME_CrushWhenEncroach    =2,
    ME_IgnoreWhenEncroach   =3,
    ME_MAX                  =4,
};

class ENGINE_API AMover : public ABrush
{
public:
    BYTE MoverEncroachType;
    BYTE MoverGlideType;
    BYTE BumpType;
    BYTE KeyNum;
    BYTE PrevKeyNum;
    BYTE NumKeys;
    BYTE WorldRaytraceKey;
    BYTE BrushRaytraceKey;
    FLOAT MoveTime;
    FLOAT StayOpenTime;
    FLOAT OtherTime;
    INT EncroachDamage;
    DWORD bTriggerOnceOnly:1;
    DWORD bSlave:1;
    DWORD bUseTriggered:1;
    DWORD bDamageTriggered:1;
    DWORD bDynamicLightMover:1;
    FName PlayerBumpEvent;
    FName BumpEvent;
    class AActor* SavedTrigger;
    FLOAT DamageThreshold;
    INT numTriggerEvents;
    class AMover* Leader;
    class AMover* Follower;
    FName ReturnGroup;
    class USound* OpeningSound;
    class USound* OpenedSound;
    class USound* ClosingSound;
    class USound* ClosedSound;
    class USound* MoveAmbientSound;
    FVector KeyPos[8];
    FRotator KeyRot[8];
    FVector BasePos;
    FVector OldPos;
    FVector OldPrePivot;
    FVector SavedPos;
    FRotator BaseRot;
    FRotator OldRot;
    FRotator SavedRot;
    class ANavigationPoint* myMarker;
    class AActor* TriggerActor;
    class AActor* TriggerActor2;
    class APawn* WaitingPawn;
    DWORD bOpening:1;
    DWORD bPlayerOnly:1;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AMover,ABrush,0)
    #include "AMover.h"
};

class ENGINE_API AInfo : public AActor
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AInfo,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(AInfo)
};

class ENGINE_API ASavedMove : public AInfo
{
public:
    class ASavedMove* NextMove;
    FLOAT TimeStamp;
    FLOAT Delta;
    BYTE MoveFlags;
    BYTE DodgeMove;
    DWORD bSent:1;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ASavedMove,AInfo,0)
    NO_DEFAULT_CONSTRUCTOR(ASavedMove)
};

class ENGINE_API AZoneInfo : public AInfo
{
public:
    FName ZoneTag;
    FVector ZoneGravity;
    FVector ZoneVelocity;
    FLOAT ZoneGroundFriction;
    FLOAT ZoneFluidFriction;
    FLOAT ZoneTerminalVelocity;
    FName ZonePlayerEvent;
    INT ZonePlayerCount;
    INT NumCarcasses;
    INT DamagePerSec;
    FName DamageType;
    CHAR DamageString[64];
    INT MaxCarcasses;
    class USound* EntrySound;
    class USound* ExitSound;
    class UClass* EntryActor;
    class UClass* ExitActor;
    class ASkyZoneInfo* SkyZone;
    DWORD bWaterZone:1;
    DWORD bFogZone:1;
    DWORD bKillZone:1;
    DWORD bNeutralZone:1;
    DWORD bGravityZone:1;
    DWORD bPainZone:1;
    DWORD bDestructive:1;
    BYTE AmbientBrightness;
    BYTE AmbientHue;
    BYTE AmbientSaturation;
    FColor FogColor;
    FLOAT FogDistance;
    class UTexture* EnvironmentMap;
    FLOAT TexUPanSpeed;
    FLOAT TexVPanSpeed;
    FVector ViewFlash;
    FVector ViewFog;
    DWORD bReverbZone:1;
    DWORD bRaytraceReverb:1;
    FLOAT SpeedOfSound;
    BYTE MasterGain;
    INT CutoffHz;
    BYTE Delay[6];
    BYTE Gain[6];
    void execZoneActors( FFrame& Stack, BYTE*& Result );
    void eventActorLeaving(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_ActorLeaving),&Parms);
    }
    void eventActorEntered(class AActor* Other)
    {
        struct {class AActor* Other; } Parms;
        Parms.Other=Other;
        ProcessEvent(FindFunctionChecked(ENGINE_ActorEntered),&Parms);
    }
    DECLARE_CLASS(AZoneInfo,AInfo,0)
    #include "AZoneInfo.h"
};

class ENGINE_API AWarpZoneInfo : public AZoneInfo
{
public:
    CHAR OtherSideURL[80];
    FName ThisTag;
    INT iWarpZone;
    FCoords WarpCoords;
    class AWarpZoneInfo* OtherSideActor;
    class UObject* OtherSideLevel;
    CHAR Destinations[80][8];
    INT numDestinations;
    void execUnWarp( FFrame& Stack, BYTE*& Result );
    void execWarp( FFrame& Stack, BYTE*& Result );
    void eventForceGenerate()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_ForceGenerate),NULL);
    }
    void eventGenerate()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_Generate),NULL);
    }
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AWarpZoneInfo,AZoneInfo,0)
    NO_DEFAULT_CONSTRUCTOR(AWarpZoneInfo)
};

class ENGINE_API ASkyZoneInfo : public AZoneInfo
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ASkyZoneInfo,AZoneInfo,0)
    NO_DEFAULT_CONSTRUCTOR(ASkyZoneInfo)
};

enum ENetMode
{
    NM_Standalone           =0,
    NM_DedicatedServer      =1,
    NM_ListenServer         =2,
    NM_Client               =3,
    NM_MAX                  =4,
};

enum ELevelAction
{
    LEVACT_None             =0,
    LEVACT_Loading          =1,
    LEVACT_Saving           =2,
    LEVACT_Connecting       =3,
    LEVACT_MAX              =4,
};

class ENGINE_API ALevelInfo : public AZoneInfo
{
public:
    FLOAT TimeDilation;
    FLOAT TimeSeconds;
    INT Year;
    INT Month;
    INT Day;
    INT DayOfWeek;
    INT Hour;
    INT Minute;
    INT Second;
    INT Millisecond;
    CHAR Title[64];
    CHAR Author[64];
    CHAR LevelEnterText[64];
    CHAR LocalizedPkg[64];
    CHAR Pauser[32];
    DWORD bLonePlayer:1;
    DWORD bBegunPlay:1;
    DWORD bPlayersOnly:1;
    DWORD bHighDetailMode:1;
    DWORD bStartup:1;
    class UMusic* Song;
    BYTE SongSection;
    BYTE CdTrack;
    FLOAT PlayerDoppler;
    FLOAT Brightness;
    class UTexture* DefaultTexture;
    INT HubStackLevel;
    BYTE LevelAction;
    BYTE NetMode;
    DWORD bInternet:1;
    CHAR ComputerName[32];
    CHAR EngineVersion[32];
    class UClass* DefaultGameType;
    class AGameInfo* Game;
    class ANavigationPoint* NavigationPointList;
    class APawn* PawnList;
    CHAR NextURL[240];
    DWORD bNextItems:1;
    FLOAT NextSwitchCountdown;
    void execGetAddressURL( FFrame& Stack, BYTE*& Result );
    void execGetLocalURL( FFrame& Stack, BYTE*& Result );
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ALevelInfo,AZoneInfo,0)
    NO_DEFAULT_CONSTRUCTOR(ALevelInfo)
};

class ENGINE_API AGameInfo : public AInfo
{
public:
    INT ItemGoals;
    INT KillGoals;
    INT SecretGoals;
    BYTE Difficulty;
    DWORD bNoMonsters:1;
    DWORD bMuteSpectators:1;
    DWORD bHumansOnly:1;
    DWORD bRestartLevel:1;
    DWORD bAllowRemoteAdmin:1;
    DWORD bPauseable:1;
    DWORD bCoopWeaponMode:1;
    DWORD bLowGore:1;
    DWORD bShareware:1;
    DWORD bCanChangeSkin:1;
    FLOAT AutoAim;
    FLOAT GameSpeed;
    FLOAT StartTime;
    class UClass* DefaultPlayerClass;
    class UClass* DefaultWeapon;
    INT MaxSpectators;
    CHAR AdminPassword[32];
    class AScoreBoard* Scores;
    class UClass* ScoreBoardType;
    class UClass* GameMenuType;
    class UClass* HUDType;
    class UClass* MapListType;
    CHAR MapPrefix[16];
    CHAR BeaconName[15];
    CHAR SpecialDamageString[64];
    CHAR SwitchLevelMessage[64];
    INT SentText;
    CHAR DefaultPlayerName[16];
    CHAR LeftMessage[255];
    CHAR FailedSpawnMessage[255];
    CHAR FailedPlaceMessage[255];
    CHAR NameChangedMessage[255];
    CHAR EnteredMessage[255];
    class UClass* DefaultWaterEntryActor;
    class UClass* DefaultWaterExitActor;
    class USound* DefaultWaterEntrySound;
    class USound* DefaultWaterExitSound;
    void eventAcceptInventory(class APawn* PlayerPawn)
    {
        struct {class APawn* PlayerPawn; } Parms;
        Parms.PlayerPawn=PlayerPawn;
        ProcessEvent(FindFunctionChecked(ENGINE_AcceptInventory),&Parms);
    }
    class APlayerPawn* eventLogin(const CHAR* Portal, const CHAR* Options, CHAR* Error, class UClass* SpawnClass)
    {
        struct {CHAR Portal[32]; CHAR Options[120]; CHAR Error[80]; class UClass* SpawnClass; class APlayerPawn* ReturnValue; } Parms;
        appStrncpy(Parms.Portal,Portal,32);
        appStrncpy(Parms.Options,Options,120);
        appStrncpy(Parms.Error,Error,80);
        Parms.SpawnClass=SpawnClass;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(ENGINE_Login),&Parms);
        appStrcpy(Error,Parms.Error);
        return Parms.ReturnValue;
    }
    class APlayerPawn* eventPreLogin(const CHAR* Options, CHAR* Error)
    {
        struct {CHAR Options[120]; CHAR Error[80]; class APlayerPawn* ReturnValue; } Parms;
        appStrncpy(Parms.Options,Options,120);
        appStrncpy(Parms.Error,Error,80);
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(ENGINE_PreLogin),&Parms);
        appStrcpy(Error,Parms.Error);
        return Parms.ReturnValue;
    }
    void eventGetBeaconText(CHAR* Result)
    {
        struct {CHAR Result[240]; } Parms;
        appStrncpy(Parms.Result,Result,240);
        ProcessEvent(FindFunctionChecked(ENGINE_GetBeaconText),&Parms);
        appStrcpy(Result,Parms.Result);
    }
    void eventInitGame(const CHAR* Options, CHAR* Error)
    {
        struct {CHAR Options[120]; CHAR Error[80]; } Parms;
        appStrncpy(Parms.Options,Options,120);
        appStrncpy(Parms.Error,Error,80);
        ProcessEvent(FindFunctionChecked(ENGINE_InitGame),&Parms);
        appStrcpy(Error,Parms.Error);
    }
    void eventDetailChange()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_DetailChange),NULL);
    }
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AGameInfo,AInfo,0|CLASS_Config)
    NO_DEFAULT_CONSTRUCTOR(AGameInfo)
};

class ENGINE_API AMenu : public AActor
{
public:
    class AMenu* ParentMenu;
    INT Selection;
    INT MenuLength;
    DWORD bConfigChanged:1;
    DWORD bExitAllMenus:1;
    class APlayerPawn* PlayerOwner;
    CHAR HelpMessage[255][24];
    CHAR MenuList[128][24];
    CHAR LeftString[32];
    CHAR RightString[32];
    CHAR CenterString[32];
    CHAR EnabledString[32];
    CHAR DisabledString[32];
    CHAR MenuTitle[32];
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AMenu,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(AMenu)
};

class ENGINE_API AHUD : public AActor
{
public:
    INT HudMode;
    INT Crosshair;
    class UClass* MainMenuType;
    class AMenu* MainMenu;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AHUD,AActor,0|CLASS_Config)
    NO_DEFAULT_CONSTRUCTOR(AHUD)
};

class ENGINE_API ATriggers : public AActor
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ATriggers,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(ATriggers)
};

enum ETriggerType
{
    TT_PlayerProximity      =0,
    TT_PawnProximity        =1,
    TT_ClassProximity       =2,
    TT_AnyProximity         =3,
    TT_Shoot                =4,
    TT_MAX                  =5,
};

class ENGINE_API ATrigger : public ATriggers
{
public:
    BYTE TriggerType;
    CHAR Message[80];
    DWORD bTriggerOnceOnly:1;
    DWORD bInitiallyActive:1;
    class UClass* ClassProximityType;
    FLOAT RepeatTriggerTime;
    FLOAT ReTriggerDelay;
    FLOAT TriggerTime;
    FLOAT DamageThreshold;
    class AActor* TriggerActor;
    class AActor* TriggerActor2;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ATrigger,ATriggers,0)
    NO_DEFAULT_CONSTRUCTOR(ATrigger)
};

class ENGINE_API AInventory : public AActor
{
public:
    BYTE AutoSwitchPriority;
    BYTE InventoryGroup;
    DWORD bActivatable:1;
    DWORD bDisplayableInv:1;
    DWORD bActive:1;
    DWORD bSleepTouch:1;
    DWORD bHeldItem:1;
    DWORD bAmbientGlow:1;
    DWORD bInstantRespawn:1;
    DWORD bRotatingPickup:1;
    CHAR PickupMessage[64];
    FLOAT RespawnTime;
    FName PlayerLastTouched;
    FVector PlayerViewOffset;
    class UMesh* PlayerViewMesh;
    FLOAT PlayerViewScale;
    FLOAT BobDamping;
    class UMesh* PickupViewMesh;
    FLOAT PickupViewScale;
    class UMesh* ThirdPersonMesh;
    FLOAT ThirdPersonScale;
    class UTexture* StatusIcon;
    FName ProtectionType1;
    FName ProtectionType2;
    INT Charge;
    INT ArmorAbsorption;
    DWORD bIsAnArmor:1;
    INT AbsorptionPriority;
    class AInventory* NextArmor;
    FLOAT MaxDesireability;
    class AInventorySpot* myMarker;
    class USound* PickupSound;
    class USound* ActivateSound;
    class USound* DeActivateSound;
    class USound* RespawnSound;
    class UTexture* Icon;
    CHAR M_Activated[32];
    CHAR M_Selected[32];
    CHAR M_Deactivated[32];
    void eventInvCalcView()
    {
        ProcessEvent(FindFunctionChecked(ENGINE_InvCalcView),NULL);
    }
    FLOAT eventBotDesireability(class APawn* Bot)
    {
        struct {class APawn* Bot; FLOAT ReturnValue; } Parms;
        Parms.Bot=Bot;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(ENGINE_BotDesireability),&Parms);
        return Parms.ReturnValue;
    }
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AInventory,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(AInventory)
};

class ENGINE_API AWeapon : public AInventory
{
public:
    FLOAT MaxTargetRange;
    class UClass* AmmoName;
    BYTE ReloadCount;
    INT PickupAmmoCount;
    class AAmmo* AmmoType;
    DWORD bPointing:1;
    DWORD bInstantHit:1;
    DWORD bAltInstantHit:1;
    DWORD bWarnTarget:1;
    DWORD bAltWarnTarget:1;
    DWORD bWeaponUp:1;
    DWORD bChangeWeapon:1;
    DWORD bLockedOn:1;
    DWORD bSplashDamage:1;
    FVector FireOffset;
    class UClass* ProjectileClass;
    class UClass* AltProjectileClass;
    FLOAT ProjectileSpeed;
    FLOAT AltProjectileSpeed;
    FLOAT aimerror;
    FLOAT shakemag;
    FLOAT shaketime;
    FLOAT shakevert;
    FLOAT AIRating;
    FLOAT RefireRate;
    FLOAT AltRefireRate;
    class USound* FireSound;
    class USound* AltFireSound;
    class USound* CockingSound;
    class USound* SelectSound;
    class USound* Misc1Sound;
    class USound* Misc2Sound;
    class USound* Misc3Sound;
    CHAR MessageNoAmmo[64];
    FRotator AdjustedAim;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AWeapon,AInventory,0)
    NO_DEFAULT_CONSTRUCTOR(AWeapon)
};

class ENGINE_API AKeypoint : public AActor
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AKeypoint,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(AKeypoint)
};

class ENGINE_API AInterpolationPoint : public AKeypoint
{
public:
    INT Position;
    FLOAT RateModifier;
    DWORD bEndOfPath:1;
    class AInterpolationPoint* Prev;
    class AInterpolationPoint* Next;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AInterpolationPoint,AKeypoint,0)
    NO_DEFAULT_CONSTRUCTOR(AInterpolationPoint)
};

class ENGINE_API AProjectile : public AActor
{
public:
    FLOAT speed;
    FLOAT MaxSpeed;
    FLOAT Damage;
    INT MomentumTransfer;
    class USound* SpawnSound;
    class USound* ImpactSound;
    class USound* MiscSound;
    FLOAT ExploWallOut;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AProjectile,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(AProjectile)
};

class ENGINE_API ADecoration : public AActor
{
public:
    class UClass* EffectWhenDestroyed;
    DWORD bPushable:1;
    DWORD bOnlyTriggerable:1;
    DWORD bSplash:1;
    DWORD bBobbing:1;
    DWORD bWasCarried:1;
    class USound* PushSound;
    INT numLandings;
    class UClass* contents;
    class UClass* content2;
    class UClass* content3;
    class USound* EndPushSound;
    DWORD bPushSoundPlaying:1;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ADecoration,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(ADecoration)
};

class ENGINE_API ACarcass : public ADecoration
{
public:
    DWORD bPlayerCarcass:1;
    BYTE flies;
    BYTE rats;
    DWORD bReducedHeight:1;
    DWORD bDecorative:1;
    DWORD bSlidingCarcass:1;
    INT CumulativeDamage;
    class APawn* Bugs;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ACarcass,ADecoration,0)
    NO_DEFAULT_CONSTRUCTOR(ACarcass)
};

class ENGINE_API ANavigationPoint : public AActor
{
public:
    FName ownerTeam;
    DWORD taken:1;
    INT upstreamPaths[16];
    INT Paths[16];
    INT PrunedPaths[16];
    INT visitedWeight;
    class AActor* routeCache;
    INT bestPathWeight;
    class ANavigationPoint* nextNavigationPoint;
    class ANavigationPoint* nextOrdered;
    class ANavigationPoint* prevOrdered;
    class ANavigationPoint* startPath;
    INT cost;
    DWORD bPlayerOnly:1;
    DWORD bEndPoint:1;
    DWORD bEndPointOnly:1;
    void execdescribeSpec( FFrame& Stack, BYTE*& Result );
    DWORD eventAccept(class AActor* Incoming)
    {
        struct {class AActor* Incoming; DWORD ReturnValue; } Parms;
        Parms.Incoming=Incoming;
        Parms.ReturnValue=0;
        ProcessEvent(FindFunctionChecked(ENGINE_Accept),&Parms);
        return Parms.ReturnValue;
    }
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ANavigationPoint,AActor,0)
    NO_DEFAULT_CONSTRUCTOR(ANavigationPoint)
};

class ENGINE_API ALiftExit : public ANavigationPoint
{
public:
    FName LiftTag;
    class AMover* MyLift;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ALiftExit,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(ALiftExit)
};

class ENGINE_API ALiftCenter : public ANavigationPoint
{
public:
    FName LiftTag;
    class AMover* MyLift;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ALiftCenter,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(ALiftCenter)
};

class ENGINE_API AWarpZoneMarker : public ANavigationPoint
{
public:
    class AWarpZoneInfo* markedWarpZone;
    class AActor* TriggerActor;
    class AActor* TriggerActor2;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AWarpZoneMarker,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(AWarpZoneMarker)
};

class ENGINE_API AButtonMarker : public ANavigationPoint
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AButtonMarker,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(AButtonMarker)
};

class ENGINE_API ATriggerMarker : public ANavigationPoint
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ATriggerMarker,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(ATriggerMarker)
};

class ENGINE_API AInventorySpot : public ANavigationPoint
{
public:
    class AInventory* markedItem;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(AInventorySpot,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(AInventorySpot)
};

class ENGINE_API APlayerStart : public ANavigationPoint
{
public:
    BYTE TeamNumber;
    DWORD bSinglePlayerStart:1;
    DWORD bCoopStart:1;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(APlayerStart,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(APlayerStart)
};

class ENGINE_API ATeleporter : public ANavigationPoint
{
public:
    CHAR URL[64];
    FName ProductRequired;
    DWORD bChangesVelocity:1;
    DWORD bChangesYaw:1;
    DWORD bReversesX:1;
    DWORD bReversesY:1;
    DWORD bReversesZ:1;
    DWORD bEnabled:1;
    FVector TargetVelocity;
    class AActor* TriggerActor;
    class AActor* TriggerActor2;
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ATeleporter,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(ATeleporter)
};

class ENGINE_API APathNode : public ANavigationPoint
{
public:
	DECLARE_CLASS_WITHOUT_CONSTRUCT(APathNode,ANavigationPoint,0)
    NO_DEFAULT_CONSTRUCTOR(APathNode)
};

#undef DECLARE_NAME
#endif
#pragma pack (pop)
