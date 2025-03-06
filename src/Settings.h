#pragma once

struct Settings
{
	static void Initialize();
	static void Save();

	// Processing
	static inline bool ProcessingEnabled{ true };
	static inline bool ConsequenceEnabled{ true };
	static inline bool bCreatureDefeat{ false };
	static inline bool bKillMove{ true };

	// Hunter Pride
	static inline int iHunterPrideKey{ -1 };					// hotkey to enable/disable hunter pride
	static inline int iHunterPrideKeyMod{ -1 };				// modifier key for hunter pride activaition
	static inline bool bHunterPrideFollower{ true };	// follower may onle defeat while hunter pride is active

	// Notifications
	static inline bool bNotifyDefeat{ false };						// notify when a NPC gets knocked down
	static inline bool bNotifyDestroy{ false };						// notify when item gets destroyed
	static inline bool bNotifyColored{ false };						// colored notify
	static inline std::string rNotifyColor{ "#FF0000" };	// color code

	// Knockdown - General
	static inline float fMidCombatBlackout{ 10.0f };	// Chance to blackout upon defeat (player only)
	static inline bool bPlayerDefeat{ true };					// if the player may be defeated
	static inline bool bFollowerDefeat{ true };				// if followers may be defeated
	static inline bool bFolWithPlDefeat{ false };			// If followers get knocked down alongside player
	static inline bool bNPCDefeat{ true };						// if NPC may be defeated

	// Passive Recovery
	static inline float fKdHealthThresh{ 0.20f };	 // Player: Minimum health required to recover from defeat status
	static inline int iKdFallbackTimer{ 90 };			 // Player: maximum time the player can stay defeated mid combat, independent of health
	static inline int iFollowerRescue{ 1 };				 // Follower: Auto rescue behavior | 0 Never / 1 - Combat End / 2 - Unload
	static inline bool bNPCRescueReload{ true };	 // NPC: if NPC are rescued when their 3d is being reloaded

	// Lethal
	static inline bool bLethalEssential{ true };
	static inline float fLethalPlayer{ 100.0f };
	static inline float fLethalFollower{ 100.0f };
	static inline float fLethalNPC{ 100.0f };

	// Trauma
	static inline bool bTraumaEnabled{ false };
	static inline bool bTraumaHealth{ true };
	static inline float fTraumaMult{ 32.0f };
	static inline float fTraumeBackAttack{ 2.0f };

	// Exposed
	static inline int iStrips{ 1'066'390'941 };	 // Slots to recognize
	static inline int iExposed{ 2 };						 // Min amount of slots equipped to NOT be exposed
	static inline float fExposedChance{ 0.0f };	 // Chance for an exposed strike to defeat

	// Stripping
	static inline float fStripChance{ 0.0f };
	static inline float fStripDestroy{ 5.0f };	// Chance for stripped item to be destroyed
	static inline bool bStripDrop{ false };			// should non destroyed items be dropped

	// Quick Access Funcs
	static bool DoesPlayerAutoRecover() { return iKdFallbackTimer || fKdHealthThresh && RE::PlayerCharacter::GetSingleton()->GetActorValue(RE::ActorValue::kHealRate); }

	// Table for dynamic access
	struct StringCmp
	{
		bool operator()(const std::string& a_lhs, const std::string& a_rhs) const
		{
			return _strcmpi(a_lhs.c_str(), a_rhs.c_str()) < 0;
		}
	};

	enum class VariantType
	{
		FLOAT = 0,
		STRING = 1,
		BOOL = 2,
		INT = 3
	};

#define ENTRY(var) \
	{                \
		#var##s, &var  \
	}
	static inline std::map<std::string, std::variant<float*, std::string*, bool*, int*>, StringCmp> table{
		ENTRY(ProcessingEnabled),
		ENTRY(ConsequenceEnabled),
		ENTRY(bCreatureDefeat),
		ENTRY(bKillMove),
		ENTRY(iHunterPrideKey),
		ENTRY(iHunterPrideKeyMod),
		ENTRY(bHunterPrideFollower),
		ENTRY(bNotifyDefeat),
		ENTRY(bNotifyDestroy),
		ENTRY(bNotifyColored),
		ENTRY(rNotifyColor),
		ENTRY(bPlayerDefeat),
		ENTRY(bFollowerDefeat),
		ENTRY(bNPCDefeat),
		ENTRY(fMidCombatBlackout),
		ENTRY(bLethalEssential),
		ENTRY(fLethalPlayer),
		ENTRY(fLethalFollower),
		ENTRY(fLethalNPC),
		ENTRY(bTraumaEnabled),
		ENTRY(bTraumaHealth),
		ENTRY(fTraumaMult),
		ENTRY(fTraumeBackAttack),
		ENTRY(iStrips),
		ENTRY(iExposed),
		ENTRY(fExposedChance),
		ENTRY(fStripChance),
		ENTRY(fStripDestroy),
		ENTRY(bStripDrop),
		ENTRY(fKdHealthThresh),
		ENTRY(iKdFallbackTimer),
		ENTRY(iFollowerRescue),
		ENTRY(bNPCRescueReload),
		ENTRY(bFolWithPlDefeat)
	};
};
