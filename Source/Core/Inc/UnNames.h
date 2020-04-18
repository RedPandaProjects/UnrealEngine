/*=============================================================================
	UnNames.h: Header file registering global hardcoded Unreal names.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Macros.
-----------------------------------------------------------------------------*/

// Define a message as an enumeration.
#ifndef REGISTER_NAME
	#define REGISTER_NAME(num,name) NAME_##name = num,
	#define REG_NAME_HIGH(num,name) NAME_##name = num,
	#define REGISTERING_ENUM
	enum EName {
#endif

/*-----------------------------------------------------------------------------
	Hardcoded names which are not messages.
-----------------------------------------------------------------------------*/

// Special zero value, meaning no name.
REG_NAME_HIGH(   0, None             )

// Class property types; these map straight onto hardcoded property types.
REGISTER_NAME(   1, ByteProperty     )
REGISTER_NAME(   2, IntProperty      )
REGISTER_NAME(   3, BoolProperty     )
REGISTER_NAME(   4, FloatProperty    )
REGISTER_NAME(   5, ObjectProperty   )
REGISTER_NAME(   6, NameProperty     )
REGISTER_NAME(   7, StringProperty   )
REGISTER_NAME(   8, ClassProperty    )
REGISTER_NAME(  10, StructProperty   )
REGISTER_NAME(  11, VectorProperty   )
REGISTER_NAME(  12, RotatorProperty  )

// Packages.
REGISTER_NAME(  20, Core			 )
REGISTER_NAME(  21, Engine			 )
REGISTER_NAME(  22, Editor           )
REGISTER_NAME(  23, UnrealI          )
REGISTER_NAME(  24, Master           )

// UnrealScript types.
REG_NAME_HIGH(  80, Byte		     )
REG_NAME_HIGH(  81, Int			     )
REG_NAME_HIGH(  82, Bool		     )
REG_NAME_HIGH(  83, Float		     )
REG_NAME_HIGH(  84, Name		     )
REG_NAME_HIGH(  85, String		     )
REG_NAME_HIGH(  86, Struct			 )
REG_NAME_HIGH(  87, Vector		     )
REG_NAME_HIGH(  88, Rotator	         )
REG_NAME_HIGH(  89, DynamicString    )
REG_NAME_HIGH(  90, Color            )
REG_NAME_HIGH(  91, Plane            )

// Keywords.
REGISTER_NAME( 100, Begin			 )
REG_NAME_HIGH( 102, State            )
REG_NAME_HIGH( 103, Function         )
REG_NAME_HIGH( 104, Self             )
REG_NAME_HIGH( 105, True             )
REG_NAME_HIGH( 106, False            )
REG_NAME_HIGH( 107, Transient        )
REG_NAME_HIGH( 117, Enum			 )
REG_NAME_HIGH( 119, Replication      )
REG_NAME_HIGH( 120, Reliable         )
REG_NAME_HIGH( 121, Unreliable       )
REG_NAME_HIGH( 122, Always           )

// Object class names.
REGISTER_NAME( 150, Field            )
REGISTER_NAME( 151, Object           )
REGISTER_NAME( 152, TextBuffer       )
REGISTER_NAME( 153, Linker           )
REGISTER_NAME( 154, LinkerLoad       )
REGISTER_NAME( 155, LinkerSave       )
REGISTER_NAME( 156, Subsystem        )
REGISTER_NAME( 157, Factory          )
REGISTER_NAME( 158, TextBufferFactory)
REGISTER_NAME( 159, Exporter         )
REGISTER_NAME( 160, StackNode        )
REGISTER_NAME( 161, Property         )

/*-----------------------------------------------------------------------------
	Special engine-generated probe messages.
-----------------------------------------------------------------------------*/

//
// In the description for each message, the type of parameter associated with the
// message is shown in square brackets, such as [PActor], or [null] if there are no
// parameters. These parameter types are found in UnMsgPar.h.
//
//warning: All entries must be filled in, otherwise non-probe names might be mapped
// to probe name indices.
//

#define NAME_PROBEMIN ((EName)300)
#define NAME_PROBEMAX ((EName)364)

// Creation and destruction.
REGISTER_NAME( 300, Spawned			 ) // Sent to actor immediately after spawning.
REGISTER_NAME( 301, Destroyed        ) // Called immediately before actor is removed from actor list.

// Gaining/losing actors.
REGISTER_NAME( 302, GainedChild		 ) // Sent to a parent actor when another actor attaches to it.
REGISTER_NAME( 303, LostChild		 ) // Sent to a parent actor when another actor detaches from it.
REGISTER_NAME( 304, Probe4 			 )
REGISTER_NAME( 305, Probe5			 )

// Triggers.
REGISTER_NAME( 306, Trigger			 ) // Message sent by Trigger actors.
REGISTER_NAME( 307, UnTrigger		 ) // Message sent by Trigger actors.

// Physics & world interaction.
REGISTER_NAME( 308, Timer			 ) // The per-actor timer has fired.
REGISTER_NAME( 309, HitWall			 ) // Ran into a wall.
REGISTER_NAME( 310, Falling			 ) // Actor is falling.
REGISTER_NAME( 311, Landed			 ) // Actor has landed.
REGISTER_NAME( 312, ZoneChange		 ) // Actor has changed into a new zone.
REGISTER_NAME( 313, Touch			 ) // Actor was just touched by another actor.
REGISTER_NAME( 314, UnTouch			 ) // Actor touch just ended, always sent sometime after Touch.
REGISTER_NAME( 315, Bump			 ) // Actor was just touched and blocked. No interpenetration. No UnBump.
REGISTER_NAME( 316, BeginState		 ) // Just entered a new state.
REGISTER_NAME( 317, EndState		 ) // About to leave the current state.
REGISTER_NAME( 318, BaseChange		 ) // Sent to actor when its floor changes.
REGISTER_NAME( 319, Attach			 ) // Sent to actor when it's stepped on by another actor.
REGISTER_NAME( 320, Detach			 ) // Sent to actor when another actor steps off it.
REGISTER_NAME( 321, ActorEntered	 ) // Sent to a ZoneInfo actor when an actor enters.
REGISTER_NAME( 322, ActorLeaving	 ) // Sent to a ZoneInfo actor when an actor is leaving.
REGISTER_NAME( 323, KillCredit		 ) // Actor has just received credit for a kill.
REGISTER_NAME( 324, AnimEnd			 ) // Animation sequence has ended.
REGISTER_NAME( 325, EndedRotation	 ) // Physics based rotation just ended.
REGISTER_NAME( 326, InterpolateEnd   ) // Movement interpolation sequence finished.
REGISTER_NAME( 327, EncroachingOn    ) // Encroaching on another actor.
REGISTER_NAME( 328, EncroachedBy     ) // Being encroached by another actor.
REGISTER_NAME( 329, FootZoneChange   ) // Pawn's feet changed zones
REGISTER_NAME( 330, HeadZoneChange   ) // Pawn's head changed zones
REGISTER_NAME( 331, PainTimer        ) // pain timer expired
REGISTER_NAME( 332, SpeechTimer      ) // speech timer expired
REGISTER_NAME( 333, MayFall 		 )
REGISTER_NAME( 334, Probe34			 )

// Kills.
REGISTER_NAME( 335, Die				 ) // Actor died (sent if specific die message not processed).

// Updates.
REGISTER_NAME( 336, Tick			 ) // Clock tick update for nonplayer.
REGISTER_NAME( 337, PlayerTick		 ) // Clock tick update for player.
REGISTER_NAME( 338, Expired		     ) // Actor's LifeSpan expired.
REGISTER_NAME( 339, Probe39			 )

// AI.
REGISTER_NAME( 340, SeePlayer        ) // Can see player.
REGISTER_NAME( 341, EnemyNotVisible  ) // Current Enemy is not visible.
REGISTER_NAME( 342, HearNoise        ) // Noise nearby.
REGISTER_NAME( 343, UpdateEyeHeight  ) // Update eye level (after physics).
REGISTER_NAME( 344, SeeMonster       ) // Can see non-player.
REGISTER_NAME( 345, SeeFriend        ) // Can see non-player friend.
REGISTER_NAME( 346, SpecialHandling	 ) // Navigation point requests special handling.
REGISTER_NAME( 347, BotDesireability ) // Value of this inventory to bot.
REGISTER_NAME( 348, Probe48			 )
REGISTER_NAME( 349, Probe49			 )
REGISTER_NAME( 350, Probe50			 )
REGISTER_NAME( 351, Probe51			 )
REGISTER_NAME( 352, Probe52			 )
REGISTER_NAME( 353, Probe53			 )
REGISTER_NAME( 354, Probe54			 )
REGISTER_NAME( 355, Probe55			 )
REGISTER_NAME( 356, Probe56			 )
REGISTER_NAME( 357, Probe57			 )
REGISTER_NAME( 358, Probe58			 )
REGISTER_NAME( 359, Probe59			 )
REGISTER_NAME( 360, Probe60			 )
REGISTER_NAME( 361, Probe61			 )
REGISTER_NAME( 362, Probe62			 )

// Special tag meaning 'All probes'.
REGISTER_NAME( 363, All				 ) // Special meaning, not a message.

/*-----------------------------------------------------------------------------
	Hardcoded names used by the compiler.
-----------------------------------------------------------------------------*/

// Constants.
REG_NAME_HIGH( 600, Vect)
REG_NAME_HIGH( 601, Rot)
REG_NAME_HIGH( 605, ArrayCount)
REG_NAME_HIGH( 606, EnumCount)

// Flow control.
REG_NAME_HIGH( 620, Else)
REG_NAME_HIGH( 621, If)
REG_NAME_HIGH( 622, Goto)
REG_NAME_HIGH( 623, Stop)
REG_NAME_HIGH( 625, Until)
REG_NAME_HIGH( 626, While)
REG_NAME_HIGH( 627, Do)
REG_NAME_HIGH( 628, Break)
REG_NAME_HIGH( 629, For)
REG_NAME_HIGH( 630, ForEach)
REG_NAME_HIGH( 631, Assert)
REG_NAME_HIGH( 632, Switch)
REG_NAME_HIGH( 633, Case)
REG_NAME_HIGH( 634, Default)

// Variable overrides.
REG_NAME_HIGH( 640, Private)
REG_NAME_HIGH( 641, Const)
REG_NAME_HIGH( 642, Out)
REG_NAME_HIGH( 643, Export)
REG_NAME_HIGH( 646, Skip)
REG_NAME_HIGH( 647, Coerce)
REG_NAME_HIGH( 648, Optional)
REG_NAME_HIGH( 649, Input)
REG_NAME_HIGH( 650, Config)
REG_NAME_HIGH( 652, Travel)
REG_NAME_HIGH( 653, EditConst)
REG_NAME_HIGH( 654, Localized)
REG_NAME_HIGH( 655, GlobalConfig)
REG_NAME_HIGH( 656, SafeReplace)

// Class overrides.
REG_NAME_HIGH( 660, Expands)
REG_NAME_HIGH( 661, Intrinsic)
REG_NAME_HIGH( 663, Abstract)
REG_NAME_HIGH( 664, Package)
REG_NAME_HIGH( 665, Guid)
REG_NAME_HIGH( 667, Class)

// State overrides.
REG_NAME_HIGH( 670, Auto)
REG_NAME_HIGH( 672, Ignores)

// Calling overrides.
REG_NAME_HIGH( 680, Global)
REG_NAME_HIGH( 681, Super)

// Function overrides.
REG_NAME_HIGH( 690, Operator)
REG_NAME_HIGH( 691, PreOperator)
REG_NAME_HIGH( 692, PostOperator)
REG_NAME_HIGH( 693, Final)
REG_NAME_HIGH( 694, Iterator)
REG_NAME_HIGH( 695, Latent)
REG_NAME_HIGH( 696, Return)
REG_NAME_HIGH( 697, Singular)
REG_NAME_HIGH( 698, Simulated)
REG_NAME_HIGH( 699, Exec)
REG_NAME_HIGH( 700, Event)
REG_NAME_HIGH( 701, Static)

// Variable declaration.
REG_NAME_HIGH( 710, Var)
REG_NAME_HIGH( 711, Local)
REG_NAME_HIGH( 712, Import)
REG_NAME_HIGH( 713, From)

// Special commands.
REG_NAME_HIGH( 720, Spawn)

// Misc.
REGISTER_NAME( 740, Tag)
REGISTER_NAME( 742, Role)
REGISTER_NAME( 743, RemoteRole)
REGISTER_NAME( 746, Generate)
REGISTER_NAME( 747, AllInput)

// Log messages.
REGISTER_NAME( 760, Log)
REGISTER_NAME( 761, Critical)
REGISTER_NAME( 762, Init)
REGISTER_NAME( 763, Exit)
REGISTER_NAME( 764, Cmd)
REGISTER_NAME( 765, Play)
REGISTER_NAME( 766, Console)
REGISTER_NAME( 767, Warning)
REGISTER_NAME( 768, ExecWarning)
REGISTER_NAME( 769, ScriptWarning)
REGISTER_NAME( 770, ScriptLog)
REGISTER_NAME( 771, Dev)
REGISTER_NAME( 772, DevNet)
REGISTER_NAME( 773, DevPath)
REGISTER_NAME( 774, DevNetTraffic)
REGISTER_NAME( 775, DevAudio)
REGISTER_NAME( 776, DevLoad)
REGISTER_NAME( 777, DevSave)
REGISTER_NAME( 778, DevGarbage)
REGISTER_NAME( 779, DevKill)
REGISTER_NAME( 780, DevReplace)
REGISTER_NAME( 781, DevMusic)
REGISTER_NAME( 782, DevSound)
REGISTER_NAME( 783, DevCompile)
REGISTER_NAME( 784, DevBind)
REGISTER_NAME( 785, Localization)
REGISTER_NAME( 786, Compatibility)

// Console text colors.
REGISTER_NAME( 800, White)
REGISTER_NAME( 801, Black)
REGISTER_NAME( 802, Red)
REGISTER_NAME( 803, Green)
REGISTER_NAME( 804, Blue)
REGISTER_NAME( 805, Cyan)
REGISTER_NAME( 806, Magenta)
REGISTER_NAME( 807, Yellow)
REGISTER_NAME( 808, DefaultColor)

// Misc.
REGISTER_NAME( 820, KeyType)
REGISTER_NAME( 821, KeyEvent)
REGISTER_NAME( 822, Write)
REGISTER_NAME( 823, Message)
REGISTER_NAME( 824, InitialState)
REGISTER_NAME( 825, Texture)
REGISTER_NAME( 826, Sound)
REGISTER_NAME( 827, FireTexture)
REGISTER_NAME( 828, IceTexture)
REGISTER_NAME( 829, WaterTexture)
REGISTER_NAME( 830, WaveTexture)
REGISTER_NAME( 831, WetTexture)

/*-----------------------------------------------------------------------------
	Closing.
-----------------------------------------------------------------------------*/

#ifdef REGISTERING_ENUM
	};
	#undef REGISTER_NAME
	#undef REG_NAME_HIGH
	#undef REGISTERING_ENUM
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
