//=============================================================================
// Actor: The base class of all actors.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Actor expands Object
	abstract
	intrinsic;

// Imported data (during full rebuild).
#exec Texture Import File=Textures\S_Actor.pcx Name=S_Actor Mips=Off Flags=2

// Flags.
var(Advanced) const bool  bStatic;       // Does not move or change over time.
var(Advanced) bool        bHidden;       // Is hidden during gameplay.
var(Advanced) bool        bHiddenEd;     // Is hidden during editing.
var(Advanced) bool        bDirectional;  // Actor shows direction arrow during editing.
var const bool            bSelected;     // Selected in UnrealEd.
var const bool            bMemorized;    // Remembered in UnrealEd.
var const bool            bHighlighted;  // Highlighted in UnrealEd.
var(Advanced) const bool  bNoDelete;     // Cannot be deleted during play.
var bool				  bAnimFinished; // Unlooped animation sequence has finished.
var bool				  bAnimLoop;     // Whether animation is looping.
var bool				  bAnimNotify;   // Whether a notify is applied to the current sequence.
var transient const bool  bTempEditor;   // Internal UnrealEd.
var const bool            bDeleteMe;     // About to be deleted.
var transient const bool  bAssimilated;  // Actor dynamics are assimilated in world geometry.
var transient const bool  bTicked;       // Actor has been updated.
var transient bool        bLightChanged; // Recalculate this light's lighting now.
var bool                  bDynamicLight; // Temporarily treat this as a dynamic light.
var bool                  bTimerLoop;    // Timer loops (else is one-shot).
var bool                  bEdLocked;     // Locked in editor (no movement or rotation).
var(Advanced) bool        bEdShouldSnap; // Snap to grid in editor.
var transient bool        bEdSnap;       // Should snap to grid in UnrealEd.

// Other flags.
var(Advanced) bool        bCanTeleport;  // This actor can be teleported.
var(Advanced) bool        bIsSecretGoal; // This actor counts in the "secret" total.
var(Advanced) bool        bIsKillGoal;   // This actor counts in the "death" toll.
var(Advanced) bool        bIsItemGoal;   // This actor counts in the "item" count.
var(Advanced) bool		  bCollideWhenPlacing; // This actor collides with the world when placing.
var(Advanced) bool		  bTravel;       // Actor is capable of travelling among servers.
var(Advanced) bool		  bMovable;      // Actor is capable of travelling among servers.
var(Advanced) bool        bHighDetail;	 // Only show up on high-detail.
var(Advanced) bool		  bStasis;		 // In StandAlone games, turn off if not in a recently rendered zone
										 // turned off if  bCanStasis  and physics = PHYS_None or PHYS_Rotating.
var(Advanced) bool		  bForceStasis;	 // Force stasis when not recently rendered, even if physics not none or rotating.
var const	  bool		  bIsPawn;		 // True only for pawns.

// Priority Parameters
// Actor's current physics mode.
var(Movement) const enum EPhysics
{
	PHYS_None,
	PHYS_Walking,
	PHYS_Falling,
	PHYS_Swimming,
	PHYS_Flying,
	PHYS_Rotating,
	PHYS_Projectile,
	PHYS_Rolling,
	PHYS_Interpolating,
	PHYS_MovingBrush,
	PHYS_Spider,
	PHYS_Trailer
} Physics;

// Net variables.
enum ENetRole
{
	ROLE_None,              // No role at all.
	ROLE_DumbProxy,			// Dumb proxy of this actor.
	ROLE_SimulatedProxy,	// Locally simulated proxy of this actor.
	ROLE_AutonomousProxy,	// Locally autonomous proxy of this actor.
	ROLE_Authority,			// Authoritative control over the actor.
};
var ENetRole Role;
var(Networking) ENetRole RemoteRole;

// Owner.
var         const Actor   Owner;         // Owner actor.
var(Object) name InitialState;
var(Object) name Group;

// Execution and timer variables.
var float                 TimerRate;     // Timer event, 0=no timer.
var const float           TimerCounter;	 // Counts up until it reaches TimerRate.
var(Advanced) float		  LifeSpan;      // How old the object lives before dying, 0=forever.

// Animation variables.
var(Display) name         AnimSequence;  // Animation sequence we're playing.
var(Display) float        AnimFrame;     // Current animation frame, 0.0 to 1.0.
var(Display) float        AnimRate;      // Animation rate in frames per second, 0=none, negative=velocity scaled.
var          float        TweenRate;     // Tween-into rate.

//-----------------------------------------------------------------------------
// Structures.

// Identifies a unique convex volume in the world.
struct PointRegion
{
	var zoneinfo Zone;       // Zone.
	var int      iLeaf;      // Bsp leaf.
	var byte     ZoneNumber; // Zone number, to be eliminated!!
};

//-----------------------------------------------------------------------------
// Major actor properties.

// Scriptable.
var       const LevelInfo Level;         // Level this actor is on.
var       const Level     XLevel;        // Level object.
var(Events) name		  Tag;			 // Actor's tag name.
var(Events) name          Event;         // The event this actor causes.
var Actor                 Target;        // Actor we're aiming at (other uses as well).
var Pawn                  Instigator;    // Pawn responsible for damage.
var Inventory             Inventory;     // Inventory chain.
var const Actor           Base;          // Moving brush actor we're standing on.
var const PointRegion     Region;        // Region this actor is in.

// Internal.
var const byte            StandingCount; // Count of actors standing on this actor.
var const byte            MiscNumber;    // Internal use.
var const byte            LatentByte;    // Internal latent function use.
var const int             LatentInt;     // Internal latent function use.
var const float           LatentFloat;   // Internal latent function use.
var const actor           LatentActor;   // Internal latent function use.
var const actor           Touching[4];   // List of touching actors.
var const actor           Deleted;       // Next actor in just-deleted chain.

// Internal tags.
var const transient int CollisionTag, LightingTag, NetTag, OtherTag, ExtraTag, SpecialTag;

// The actor's position and rotation.
var(Movement) const vector Location;     // Actor's location; use Move to set.
var(Movement) const rotator Rotation;    // Rotation.
var       const vector    OldLocation;   // Actor's old location one tick ago.
var       const vector    ColLocation;   // Actor's old location one move ago.
var(Movement) vector      Velocity;      // Velocity.
var       vector          Acceleration;  // Acceleration.

// What kind of gameplay scenarios to appear in.
var(Filter) bool          bDifficulty0;  // Appear in difficulty 0.
var(Filter) bool          bDifficulty1;  // Appear in difficulty 1.
var(Filter) bool          bDifficulty2;  // Appear in difficulty 2.
var(Filter) bool          bDifficulty3;  // Appear in difficulty 3.
var(Filter) bool          bSinglePlayer; // Appear in single player.
var(Filter) bool          bNet;          // Appear in regular network play.
var(Filter) bool          bNetSpecial;   // Appear in special network play mode.
var(Filter) float		  OddsOfAppearing; // 0-1 - chance actor will appear in relevant game modes.

//-----------------------------------------------------------------------------
// Display properties.

// Drawing effect.
var(Display) enum EDrawType
{
	DT_None,
	DT_Sprite,
	DT_Mesh,
	DT_Brush,
	DT_RopeSprite,
	DT_VerticalSprite,
	DT_Terraform,
	DT_SpriteAnimOnce,
} DrawType;

// Style for rendering sprites, meshes.
var(Display) enum ERenderStyle
{
	STY_None,
	STY_Normal,
	STY_Masked,
	STY_Translucent,
	STY_Modulated,
} Style;

// Other display properties.
var(Display) texture    Sprite;			 // Sprite texture if DrawType=DT_Sprite.
var(Display) texture    Texture;		 // Misc texture.
var(Display) texture    Skin;            // Special skin or enviro map texture.
var(Display) mesh       Mesh;            // Mesh if DrawType=DT_Mesh.
var const export model  Brush;           // Brush if DrawType=DT_Brush.
var(Display) float      DrawScale;		 // Scaling factor, 1.0=normal size.
var			 vector		PrePivot;		 // Offset from box center for drawing.
var(Display) float      ScaleGlow;		 // Multiplies lighting.
var(Display) byte       AmbientGlow;     // Ambient brightness, or 255=pulsing.
var(Display) byte       Fatness;         // Fatness (mesh distortion).

// Display.
var(Display)  bool      bUnlit;          // Lights don't affect actor.
var(Display)  bool      bNoSmooth;       // Don't smooth actor's texture.
var(Display)  bool      bParticles;      // Mesh is a particle system.
var(Display)  bool      bMeshEnviroMap;  // Environment-map the mesh.
var(Display)  bool      bMeshCurvy;      // Curvy mesh.

// Not yet implemented.
var(Display) bool       bShadowCast;     // Casts shadows.

// Advanced.
var(Advanced) bool      bOnlyOwnerSee;   // Only owner can see this actor.
var Const     bool		bIsMover;		 // Is a mover.
var(Advanced) bool		bAlwaysRelevant; // Never destroy based on game.
var Const	  bool		bAlwaysTick;     // Update even when players-only.
var			  bool		bHurtEntry;	     // keep HurtRadius from being reentrant

//-----------------------------------------------------------------------------
// Sound.

// Ambient sound.
var(Sound) byte         SoundRadius;	 // Radius of ambient sound.
var(Sound) byte         SoundVolume;	 // Volume of amient sound.
var(Sound) byte         SoundPitch;	     // Sound pitch shift, 64.0=none.
var(Sound) sound        AmbientSound;    // Ambient sound effect.

// Regular sounds.
var(Sound) float TransientSoundVolume;

// Sound slots for actors.
enum ESoundSlot
{
	SLOT_None,
	SLOT_Misc,
	SLOT_Pain,
	SLOT_Interact,
	SLOT_Ambient,
	SLOT_Talk,
	SLOT_Interface,
};

// Music transitions.
enum EMusicTransition
{
	MTRAN_None,
	MTRAN_Instant,
	MTRAN_Segue,
	MTRAN_Fade,
	MTRAN_FastFade,
	MTRAN_SlowFade,
};

//-----------------------------------------------------------------------------
// Collision.

// Collision size.
var(Collision) const float CollisionRadius; // Radius of collision cyllinder.
var(Collision) const float CollisionHeight; // Half-height cyllinder.

// Collision flags.
var(Collision) const bool bCollideActors;   // Collides with other actors.
var(Collision) bool       bCollideWorld;    // Collides with the world.
var(Collision) bool       bBlockActors;	    // Blocks other nonplayer actors.
var(Collision) bool       bBlockPlayers;    // Blocks other player actors.
var(Collision) bool       bProjTarget;      // Projectiles should potentially target this actor.

//-----------------------------------------------------------------------------
// Lighting.

// Light modulation.
var(Lighting) enum ELightType
{
	LT_None,
	LT_Steady,
	LT_Pulse,
	LT_Blink,
	LT_Flicker,
	LT_Strobe,
	LT_BackdropLight,
	LT_SubtlePulse,
	LT_TexturePaletteOnce,
	LT_TexturePaletteLoop
} LightType;

// Spatial light effect to use.
var(Lighting) enum ELightEffect
{
	LE_None,
	LE_TorchWaver,
	LE_FireWaver,
	LE_WateryShimmer,
	LE_Searchlight,
	LE_SlowWave,
	LE_FastWave,
	LE_CloudCast,
	LE_StaticSpot,
	LE_Shock,
	LE_Disco,
	LE_Warp,
	LE_Spotlight,
	LE_NonIncidence,
	LE_Shell,
	LE_OmniBumpMap,
	LE_Interference,
	LE_Cylinder,
	LE_Rotor,
	LE_Unused
} LightEffect;

// Lighting info.
var(LightColor) byte
	LightBrightness,
	LightHue,
	LightSaturation;

// Light properties.
var(Lighting) byte
	LightRadius,
	LightPeriod,
	LightPhase,
	LightCone,
	VolumeBrightness,
	VolumeRadius,
	VolumeFog;

// Lighting.
var(Lighting) bool	     bSpecialLit;	 // Only affects special-lit surfaces.
var(Lighting) bool	     bActorShadows;  // Light casts actor shadows.
var(Lighting) bool	     bCorona;        // Light uses Skin as a corona.
var(Lighting) bool	     bLensFlare;     // Whether to use zone lens flare.

//-----------------------------------------------------------------------------
// Physics.

// Options.
var(Movement) bool        bBounce;           // Bounces when hits ground fast.
var(Movement) bool		  bFixedRotationDir; // Fixed direction of rotation.
var(Movement) bool		  bRotateToDesired;  // Rotate to DesiredRotation.
var           bool        bInterpolating;    // Performing interpolating.
var			  const bool  bJustTeleported;   // Used by engine physics - not valid for scripts.

// Dodge move direction.
var enum EDodgeDir
{
	DODGE_None,
	DODGE_Left,
	DODGE_Right,
	DODGE_Forward,
	DODGE_Back,
	DODGE_Active,
	DODGE_Done
} DodgeDir;

// Physics properties.
var(Movement) float       Mass;            // Mass of this actor.
var(Movement) float       Buoyancy;        // Water buoyancy.
var(Movement) rotator	  RotationRate;    // Change in rotation per second.
var(Movement) rotator     DesiredRotation; // Physics will rotate pawn to this if bRotateToDesired.
var           float       PhysAlpha;       // Interpolating position, 0.0-1.0.
var           float       PhysRate;        // Interpolation rate per second.

//-----------------------------------------------------------------------------
// Animation.

// Animation control.
var          float        AnimLast;        // Last frame.
var          float        AnimMinRate;     // Minimum rate for velocity-scaled animation.
var			 float		  OldAnimRate;	   // Animation rate of previous animation (= AnimRate until animation completes).
var			 plane		  SimAnim;		   // replicated to simulated proxies.

//-----------------------------------------------------------------------------
// Networking.

// Network control.
var(Networking) float NetPriority; // Higher priorities means update it more frequently.

// Symmetric network flags, valid during replication only.
var const bool bNetInitial;       // Initial network update.
var const bool bNetOwner;         // Player owns this actor.
var const bool bNetSee;           // Player sees it in network play.
var const bool bNetHear;          // Player hears it in network play.
var const bool bNetFeel;          // Player collides with/feels it in network play.
var const bool bSimulatedPawn;	  // True if Pawn and simulated proxy.
var const bool bNetOptional;	  // Actor should only be replicated if bandwidth available.

//-----------------------------------------------------------------------------
// Enums.

// Travelling from server to server.
enum ETravelType
{
	TRAVEL_Absolute,	// Absolute URL.
	TRAVEL_Partial,		// Partial (carry name, reset server).
	TRAVEL_Relative,	// Relative URL.
};

// Input system states.
enum EInputAction
{
	IST_None,    // Not performing special input processing.
	IST_Press,   // Handling a keypress or button press.
	IST_Hold,    // Handling holding a key or button.
	IST_Release, // Handling a key or button release.
	IST_Axis,    // Handling analog axis movement.
};

// Input keys.
enum EInputKey
{
/*00*/	IK_None			,IK_LeftMouse	,IK_RightMouse	,IK_Cancel		,
/*04*/	IK_MiddleMouse	,IK_Unknown05	,IK_Unknown06	,IK_Unknown07	,
/*08*/	IK_Backspace	,IK_Tab         ,IK_Unknown0A	,IK_Unknown0B	,
/*0C*/	IK_Unknown0C	,IK_Enter	    ,IK_Unknown0E	,IK_Unknown0F	,
/*10*/	IK_Shift		,IK_Ctrl	    ,IK_Alt			,IK_Pause       ,
/*14*/	IK_CapsLock		,IK_Unknown15	,IK_Unknown16	,IK_Unknown17	,
/*18*/	IK_Unknown18	,IK_Unknown19	,IK_Unknown1A	,IK_Escape		,
/*1C*/	IK_Unknown1C	,IK_Unknown1D	,IK_Unknown1E	,IK_Unknown1F	,
/*20*/	IK_Space		,IK_PageUp      ,IK_PageDown    ,IK_End         ,
/*24*/	IK_Home			,IK_Left        ,IK_Up          ,IK_Right       ,
/*28*/	IK_Down			,IK_Select      ,IK_Print       ,IK_Execute     ,
/*2C*/	IK_PrintScrn	,IK_Insert      ,IK_Delete      ,IK_Help		,
/*30*/	IK_0			,IK_1			,IK_2			,IK_3			,
/*34*/	IK_4			,IK_5			,IK_6			,IK_7			,
/*38*/	IK_8			,IK_9			,IK_Unknown3A	,IK_Unknown3B	,
/*3C*/	IK_Unknown3C	,IK_Unknown3D	,IK_Unknown3E	,IK_Unknown3F	,
/*40*/	IK_Unknown40	,IK_A			,IK_B			,IK_C			,
/*44*/	IK_D			,IK_E			,IK_F			,IK_G			,
/*48*/	IK_H			,IK_I			,IK_J			,IK_K			,
/*4C*/	IK_L			,IK_M			,IK_N			,IK_O			,
/*50*/	IK_P			,IK_Q			,IK_R			,IK_S			,
/*54*/	IK_T			,IK_U			,IK_V			,IK_W			,
/*58*/	IK_X			,IK_Y			,IK_Z			,IK_Unknown5B	,
/*5C*/	IK_Unknown5C	,IK_Unknown5D	,IK_Unknown5E	,IK_Unknown5F	,
/*60*/	IK_NumPad0		,IK_NumPad1     ,IK_NumPad2     ,IK_NumPad3     ,
/*64*/	IK_NumPad4		,IK_NumPad5     ,IK_NumPad6     ,IK_NumPad7     ,
/*68*/	IK_NumPad8		,IK_NumPad9     ,IK_GreyStar    ,IK_GreyPlus    ,
/*6C*/	IK_Separator	,IK_GreyMinus	,IK_NumPadPeriod,IK_GreySlash   ,
/*70*/	IK_F1			,IK_F2          ,IK_F3          ,IK_F4          ,
/*74*/	IK_F5			,IK_F6          ,IK_F7          ,IK_F8          ,
/*78*/	IK_F9           ,IK_F10         ,IK_F11         ,IK_F12         ,
/*7C*/	IK_F13			,IK_F14         ,IK_F15         ,IK_F16         ,
/*80*/	IK_F17			,IK_F18         ,IK_F19         ,IK_F20         ,
/*84*/	IK_F21			,IK_F22         ,IK_F23         ,IK_F24         ,
/*88*/	IK_Unknown88	,IK_Unknown89	,IK_Unknown8A	,IK_Unknown8B	,
/*8C*/	IK_Unknown8C	,IK_Unknown8D	,IK_Unknown8E	,IK_Unknown8F	,
/*90*/	IK_NumLock		,IK_ScrollLock  ,IK_Unknown92	,IK_Unknown93	,
/*94*/	IK_Unknown94	,IK_Unknown95	,IK_Unknown96	,IK_Unknown97	,
/*98*/	IK_Unknown98	,IK_Unknown99	,IK_Unknown9A	,IK_Unknown9B	,
/*9C*/	IK_Unknown9C	,IK_Unknown9D	,IK_Unknown9E	,IK_Unknown9F	,
/*A0*/	IK_LShift		,IK_RShift      ,IK_LControl    ,IK_RControl    ,
/*A4*/	IK_UnknownA4	,IK_UnknownA5	,IK_UnknownA6	,IK_UnknownA7	,
/*A8*/	IK_UnknownA8	,IK_UnknownA9	,IK_UnknownAA	,IK_UnknownAB	,
/*AC*/	IK_UnknownAC	,IK_UnknownAD	,IK_UnknownAE	,IK_UnknownAF	,
/*B0*/	IK_UnknownB0	,IK_UnknownB1	,IK_UnknownB2	,IK_UnknownB3	,
/*B4*/	IK_UnknownB4	,IK_UnknownB5	,IK_UnknownB6	,IK_UnknownB7	,
/*B8*/	IK_UnknownB8	,IK_UnknownB9	,IK_Semicolon	,IK_Equals		,
/*BC*/	IK_Comma		,IK_Minus		,IK_Period		,IK_Slash		,
/*C0*/	IK_Tilde		,IK_UnknownC1	,IK_UnknownC2	,IK_UnknownC3	,
/*C4*/	IK_UnknownC4	,IK_UnknownC5	,IK_UnknownC6	,IK_UnknownC7	,
/*C8*/	IK_Joy1	        ,IK_Joy2	    ,IK_Joy3	    ,IK_Joy4	    ,
/*CC*/	IK_Joy5	        ,IK_Joy6	    ,IK_Joy7	    ,IK_Joy8	    ,
/*D0*/	IK_Joy9	        ,IK_Joy10	    ,IK_Joy11	    ,IK_Joy12		,
/*D4*/	IK_Joy13		,IK_Joy14	    ,IK_Joy15	    ,IK_Joy16	    ,
/*D8*/	IK_UnknownD8	,IK_UnknownD9	,IK_UnknownDA	,IK_LeftBracket	,
/*DC*/	IK_Backslash	,IK_RightBracket,IK_SingleQuote	,IK_UnknownDF	,
/*E0*/  IK_JoyX			,IK_JoyY		,IK_JoyZ		,IK_JoyR		,
/*E4*/	IK_MouseX		,IK_MouseY		,IK_MouseZ		,IK_MouseW		,
/*E8*/	IK_JoyU			,IK_JoyV		,IK_UnknownEA	,IK_UnknownEB	,
/*EC*/	IK_MouseWheelUp ,IK_MouseWheelDown,IK_Unknown10E,UK_Unknown10F  ,
/*F0*/	IK_JoyPovUp     ,IK_JoyPovDown	,IK_JoyPovLeft	,IK_JoyPovRight	,
/*F4*/	IK_UnknownF4	,IK_UnknownF5	,IK_Attn		,IK_CrSel		,
/*F8*/	IK_ExSel		,IK_ErEof		,IK_Play		,IK_Zoom		,
/*FC*/	IK_NoName		,IK_PA1			,IK_OEMClear
};

//-----------------------------------------------------------------------------
// Intrinsics.

// Vector operators.
intrinsic(211) static final preoperator  vector -     ( vector A );
intrinsic(212) static final operator(16) vector *     ( vector A, float B );
intrinsic(213) static final operator(16) vector *     ( float A, vector B );
intrinsic(296) static final operator(16) vector *     ( vector A, vector B );
intrinsic(214) static final operator(16) vector /     ( vector A, float B );
intrinsic(215) static final operator(20) vector +     ( vector A, vector B );
intrinsic(216) static final operator(20) vector -     ( vector A, vector B );
intrinsic(275) static final operator(22) vector <<    ( vector A, rotator B );
intrinsic(276) static final operator(22) vector >>    ( vector A, rotator B );
intrinsic(217) static final operator(24) bool   ==    ( vector A, vector B );
intrinsic(218) static final operator(26) bool   !=    ( vector A, vector B );
intrinsic(219) static final operator(16) float  Dot   ( vector A, vector B );
intrinsic(220) static final operator(16) vector Cross ( vector A, vector B );
intrinsic(221) static final operator(34) vector *=    ( out vector A, float B );
intrinsic(297) static final operator(34) vector *=    ( out vector A, vector B );
intrinsic(222) static final operator(34) vector /=    ( out vector A, float B );
intrinsic(223) static final operator(34) vector +=    ( out vector A, vector B );
intrinsic(224) static final operator(34) vector -=    ( out vector A, vector B );

// Vector functions.
intrinsic(225) static final function float  VSize  ( vector A );
intrinsic(226) static final function vector Normal ( vector A );
intrinsic(227) static final function        Invert ( out vector X, out vector Y, out vector Z );
intrinsic(252) static final function vector VRand  ( );
intrinsic(300) static final function vector MirrorVectorByNormal( vector Vect, vector Normal );

// Rotator operators and functions.
intrinsic(142) static final operator(24) bool ==     ( rotator A, rotator B );
intrinsic(203) static final operator(26) bool !=     ( rotator A, rotator B );
intrinsic(287) static final operator(16) rotator *   ( rotator A, float    B );
intrinsic(288) static final operator(16) rotator *   ( float    A, rotator B );
intrinsic(289) static final operator(16) rotator /   ( rotator A, float    B );
intrinsic(290) static final operator(34) rotator *=  ( out rotator A, float B  );
intrinsic(291) static final operator(34) rotator /=  ( out rotator A, float B  );
intrinsic(316) static final operator(20) rotator +   ( rotator A, rotator B );
intrinsic(317) static final operator(20) rotator -   ( rotator A, rotator B );
intrinsic(318) static final operator(34) rotator +=  ( out rotator A, rotator B );
intrinsic(319) static final operator(34) rotator -=  ( out rotator A, rotator B );
intrinsic(229) static final function GetAxes         ( rotator A, out vector X, out vector Y, out vector Z );
intrinsic(230) static final function GetUnAxes       ( rotator A, out vector X, out vector Y, out vector Z );
intrinsic(320) static final function rotator RotRand ( optional bool bRoll );

//-----------------------------------------------------------------------------
// Network replication.

replication
{
	// Relationships.
	reliable if( Role==ROLE_Authority )
		Owner, Role, RemoteRole;
	reliable if( Role==ROLE_Authority && bNetOwner )
		bNetOwner, Inventory;

	// Ambient sound.
	reliable if( Role==ROLE_Authority )
		AmbientSound;
	reliable if( Role==ROLE_Authority && AmbientSound!=None )
		SoundRadius, SoundVolume, SoundPitch;

	// Collision.
	reliable if( Role==ROLE_Authority )
		bCollideActors;
	reliable if( Role==ROLE_Authority )
		bCollideWorld;
	reliable if( Role==ROLE_Authority && bCollideActors )
		bBlockActors, bBlockPlayers;
	reliable if( Role==ROLE_Authority && (bCollideActors || bCollideWorld) )
		CollisionRadius, CollisionHeight;

	// Location.
	unreliable if( Role==ROLE_Authority && (bNetInitial || bSimulatedPawn || RemoteRole<ROLE_SimulatedProxy) )
		Location, Rotation;
	unreliable if( Role==ROLE_Authority && (bNetInitial || bSimulatedPawn) && RemoteRole==ROLE_SimulatedProxy )
		Base;

	// Velocity.
	unreliable if( (RemoteRole==ROLE_SimulatedProxy && (bNetInitial || bSimulatedPawn)) || bIsMover )
		Velocity;

	// Physics.
	unreliable if( RemoteRole==ROLE_SimulatedProxy && bNetInitial )
		Physics, Acceleration, bBounce;
	unreliable if( RemoteRole==ROLE_SimulatedProxy && Physics==PHYS_Rotating && bNetInitial )
		bFixedRotationDir, bRotateToDesired, RotationRate, DesiredRotation;

	// Animation. 
	unreliable if( DrawType==DT_Mesh && (RemoteRole<=ROLE_SimulatedProxy) )
		AnimSequence;
	unreliable if( DrawType==DT_Mesh && (RemoteRole==ROLE_SimulatedProxy) )
		bAnimNotify;
	unreliable if( DrawType==DT_Mesh && (RemoteRole<ROLE_AutonomousProxy) )
		SimAnim, AnimMinRate;

	// Rendering.
	reliable if( Role==ROLE_Authority )
		bHidden, bOnlyOwnerSee;
	unreliable if( Role==ROLE_Authority ) // and see...
		Texture, DrawScale, PrePivot, DrawType, AmbientGlow, Fatness, ScaleGlow, bUnlit, bNoSmooth, bShadowCast, bActorShadows;
	unreliable if( Role==ROLE_Authority && DrawType==DT_Sprite && !bHidden && (!bOnlyOwnerSee || bNetOwner) )
		Sprite;
	unreliable if( Role==ROLE_Authority && DrawType==DT_Mesh )
		Mesh, bMeshEnviroMap, bMeshCurvy, Skin;
	unreliable if( Role==ROLE_Authority && DrawType==DT_Brush )
		Brush;

	// Lighting.
	unreliable if( Role==ROLE_Authority )
		LightType;
	unreliable if( Role==ROLE_Authority && LightType!=LT_None )
		LightEffect, LightBrightness, LightHue, LightSaturation,
		LightRadius, LightPeriod, LightPhase, LightCone,
		VolumeBrightness, VolumeRadius,
		bSpecialLit;

	// Messages
	unreliable if( Role < ROLE_Authority )
		BroadcastMessage;
}

//=============================================================================
// Actor error handling.

// Handle an error and kill this one actor.
intrinsic(233) final function Error( coerce string[255] S );

//=============================================================================
// General functions.

// Latent functions.
intrinsic(256) final latent function Sleep( float Seconds );

// Collision.
intrinsic(262) final function SetCollision( optional bool NewColActors, optional bool NewBlockActors, optional bool NewBlockPlayers );
intrinsic(283) final function bool SetCollisionSize( float NewRadius, float NewHeight );

// Movement.
intrinsic(266) final function bool Move( vector Delta );
intrinsic(267) final function bool SetLocation( vector NewLocation );
intrinsic(299) final function bool SetRotation( rotator NewRotation );
intrinsic(3969) final function bool MoveSmooth( vector Delta );

// Relations.
intrinsic(298) final function SetBase( actor NewBase );
intrinsic(272) final function SetOwner( actor NewOwner );
intrinsic(303) final function bool IsA( name ClassName );

//=============================================================================
// Animation.

// Animation functions.
intrinsic(259) final function PlayAnim( name Sequence, optional float Rate, optional float TweenTime );
intrinsic(260) final function LoopAnim( name Sequence, optional float Rate, optional float TweenTime, optional float MinRate );
intrinsic(294) final function TweenAnim( name Sequence, float Time );
intrinsic(282) final function bool IsAnimating();
intrinsic(293) final function name GetAnimGroup( name Sequence );
intrinsic(261) final latent function FinishAnim();

// Animation notifications.
event AnimEnd();

//=========================================================================
// Physics.

// Physics control.
intrinsic(301) final latent function FinishInterpolation();
intrinsic(3970) final function SetPhysics( EPhysics newPhysics );

//=============================================================================
// Engine notification functions.

//
// Major notifications.
//
event Spawned();
event Destroyed();
event Expired();
event GainedChild( Actor Other );
event LostChild( Actor Other );
event Tick( float DeltaTime );

//
// Triggers.
//
event Trigger( Actor Other, Pawn EventInstigator );
event UnTrigger( Actor Other, Pawn EventInstigator );
event BeginEvent();
event EndEvent();

//
// Physics & world interaction.
//
event Timer();
event HitWall( vector HitNormal, actor HitWall );
event Falling();
event Landed( vector HitNormal );
event ZoneChange( ZoneInfo NewZone );
event Touch( Actor Other );
event UnTouch( Actor Other );
event Bump( Actor Other );
event BaseChange();
event Attach( Actor Other );
event Detach( Actor Other );
event KillCredit( Actor Other );
event Actor SpecialHandling(Pawn Other);
event bool EncroachingOn( actor Other );
event EncroachedBy( actor Other );
event InterpolateEnd( actor Other );
event EndedRotation();

//
// Damage and kills.
//
event KilledBy( pawn EventInstigator );
event TakeDamage( int Damage, Pawn EventInstigator, vector HitLocation, vector Momentum, name DamageType);

//
// Trace a line and see what it collides with first.
// Takes this actor's collision properties into account.
// Returns first hit actor, Level if hit level, or None if hit nothing.
//
intrinsic(277) final function Actor Trace
(
	out vector      HitLocation,
	out vector      HitNormal,
	vector          TraceEnd,
	optional vector TraceStart,
	optional bool   bTraceActors,
	optional vector Extent
);

//
// Spawn an actor. Returns an actor of the specified class, not
// of class Actor (this is hardcoded in the compiler). Returns None
// if the actor could not be spawned (either the actor wouldn't fit in
// the specified location, or the actor list is full).
// Defaults to spawning at the spawner's location.
//
intrinsic(278) final function actor Spawn
(
	class<actor>      SpawnClass,
	optional actor	  SpawnOwner,
	optional name     SpawnTag,
	optional vector   SpawnLocation,
	optional rotator  SpawnRotation
);

//
// Destroy this actor. Returns true if destroyed, false if indestructable.
// Destruction is latent. It occurs at the end of the tick.
//
intrinsic(279) final function bool Destroy();

//=============================================================================
// Timing.

// Causes Timer() events every NewTimerRate seconds.
intrinsic(280) final function SetTimer( float NewTimerRate, bool bLoop );

//=============================================================================
// Sound functions.

// Play a sound effect.
intrinsic(264) final function PlaySound
(
	sound				Sound,
	optional ESoundSlot Slot,
	optional float		Volume,
	optional bool		bNoOverride,
	optional float		Radius,
	optional float		Pitch
);

//=============================================================================
// AI functions.

//
// Inform other creatures that you've made a noise
// they might hear (they are sent a HearNoise message)
// Senders of MakeNoise should have an instigator if they are not pawns.
//
intrinsic(512) final function MakeNoise( float Loudness );

//
// PlayerCanSeeMe returns true if some player has a line of sight to 
// actor's location.
//
intrinsic(532) final function bool PlayerCanSeeMe();

//=============================================================================
// Regular engine functions.

// Teleportation.
event bool PreTeleport( Teleporter InTeleporter );
event PostTeleport( Teleporter OutTeleporter );

// Level state.
event BeginPlay();

//========================================================================
// Disk access.

// Find files.
intrinsic(539) final function string[32] GetMapName( string[32] NameEnding, string[32] MapName, int Dir );
intrinsic(545) final function string[64] GetNextSkin( string[64] Prefix, string[64] CurrentSkin, int Dir );

//=============================================================================
// Iterator functions.

// Iterator functions for dealing with sets of actors.
intrinsic(304) final iterator function AllActors     ( class<actor> BaseClass, out actor Actor, optional name MatchTag );
intrinsic(305) final iterator function ChildActors   ( class<actor> BaseClass, out actor Actor );
intrinsic(306) final iterator function BasedActors   ( class<actor> BaseClass, out actor Actor );
intrinsic(307) final iterator function TouchingActors( class<actor> BaseClass, out actor Actor );
intrinsic(309) final iterator function TraceActors   ( class<actor> BaseClass, out actor Actor, out vector HitLoc, out vector HitNorm, vector End, optional vector Start, optional vector Extent );
intrinsic(310) final iterator function RadiusActors  ( class<actor> BaseClass, out actor Actor, float Radius, optional vector Loc );
intrinsic(311) final iterator function VisibleActors ( class<actor> BaseClass, out actor Actor, optional float Radius, optional vector Loc );
intrinsic(312) final iterator function VisibleCollidingActors ( class<actor> BaseClass, out actor Actor, optional float Radius, optional vector Loc );

//=============================================================================
// Scripted Actor functions.

//
// Called immediately before gameplay begins.
//
event PreBeginPlay()
{
	// Handle autodestruction if desired.
	if( !bAlwaysRelevant && (Level.NetMode != NM_Client) && !Level.Game.IsRelevant(Self) )
		Destroy();
}

//
// Broadcast a message to all players.
//
function BroadcastMessage( coerce string[240] Msg, bool bBeep )
{
	local Pawn P;

	if ( Level.Game.AllowsBroadcast(self, Len(Msg)) )
		for( P=Level.PawnList; P!=None; P=P.nextPawn )
			if( P.bIsPlayer )
				P.ClientMessage( Msg );
}

//
// Called immediately after gameplay begins.
//
event PostBeginPlay();

//
// Called after PostBeginPlay.
//
simulated event SetInitialState()
{
	if( InitialState!='' )
		GotoState( InitialState );
	else
		GotoState( 'Auto' );
}

//
// Hurt actors within the radius.
//
final function HurtRadius( float DamageAmount, float DamageRadius, name DamageName, float Momentum, vector HitLocation )
{
	local actor Victims;
	local float damageScale, dist;
	local vector dir;
	
	if( bHurtEntry )
		return;

	bHurtEntry = true;
	foreach VisibleCollidingActors( class 'Actor', Victims, DamageRadius, HitLocation )
	{
		if( Victims != self )
		{
			dir = Victims.Location - HitLocation;
			dist = FMax(1,VSize(dir));
			dir = dir/dist; 
			damageScale = 1 - FMax(0,(dist - Victims.CollisionRadius)/DamageRadius);
			Victims.TakeDamage
			(
				damageScale * DamageAmount,
				Instigator, 
				Victims.Location - 0.5 * (Victims.CollisionHeight + Victims.CollisionRadius) * dir,
				(damageScale * Momentum * dir),
				DamageName
			);
		} 
	}
	bHurtEntry = false;
}

//
// Called when carried onto a new level, before AcceptInventory.
//
event TravelPreAccept();

//
// Called when carried into a new level, after AcceptInventory.
//
event TravelPostAccept();

defaultproperties
{
     bDifficulty0=True
     bDifficulty1=True
     bDifficulty2=True
     bDifficulty3=True
     bSinglePlayer=True
     bNet=True
     bNetSpecial=True
	 OddsOfAppearing=+00001.000000
     DrawType=DT_Sprite
     Texture=S_Actor
     DrawScale=+00001.000000
     bMeshCurvy=True
     SoundRadius=32
     SoundVolume=128
     SoundPitch=64
	 TransientSoundVolume=+00001.000000
     CollisionRadius=+00022.000000
     CollisionHeight=+00022.000000
     bJustTeleported=True
     Mass=+00100.000000
     AnimEnd=+00001.000000
     Role=ROLE_Authority
     RemoteRole=ROLE_DumbProxy
     NetPriority=+00001.000000
	 ScaleGlow=1.0
	 Fatness=128
	 Style=STY_Normal
	 bMovable=True
	 bHighDetail=False
	 VolumeFog=0
	 InitialState=None
}
