#pragma once

namespace Acheron
{
	struct GameForms
	{
#define LOOKUPFORM(form, formid)                                                                                           \
	form = RE::TESDataHandler::GetSingleton()->LookupForm<std::remove_pointer<decltype(form)>::type>(formid, "Acheron.esm"); \
	if (!form)                                                                                                               \
		return false;

		// Vanilla Game
		static inline RE::TESFaction* GuardDiaFac;
		static inline RE::TESGlobal* KillMove;
		static inline RE::TESIdleForm* BleedoutStart;
		static inline RE::TESIdleForm* BleedoutStop;

		// Acheron Forms
		static inline RE::BGSKeyword* Defeated;
		static inline RE::BGSKeyword* Pacified;

		static inline RE::TESQuest* DefaultCommon;
		static inline RE::TESQuest* DefaultGuard;

		static inline RE::SpellItem* HunterPride;
		static inline RE::EffectSetting* HunterPrideEffect;

		static inline RE::TESPackage* BlankPackage;

		static inline RE::TESFaction* IgnoredFaction;

		static inline RE::BGSPerk* InteractionPerk;


		[[nodiscard]] static bool LoadForms()
		{
			GuardDiaFac = RE::TESForm::LookupByID<RE::TESFaction>(0x0002BE3B);
			if (!GuardDiaFac)
				return false;

			KillMove = RE::TESForm::LookupByID<RE::TESGlobal>(0x00100F19);
			if (!KillMove)
				return false;

			BleedoutStart = RE::TESForm::LookupByID<RE::TESIdleForm>(0x00013ECC);
			if (!BleedoutStart)
				return false;
			BleedoutStop = RE::TESForm::LookupByID<RE::TESIdleForm>(0x00013ECE);
			if (!BleedoutStop)
				return false;

			LOOKUPFORM(Defeated, 0x801);
			LOOKUPFORM(Pacified, 0x802);

			LOOKUPFORM(DefaultCommon, 0x805);
			LOOKUPFORM(DefaultGuard, 0x809);

			LOOKUPFORM(HunterPride, 0x807);
			LOOKUPFORM(HunterPrideEffect, 0x806);

			LOOKUPFORM(BlankPackage, 0x804);

			LOOKUPFORM(IgnoredFaction, 0x80C);

			LOOKUPFORM(InteractionPerk, 0x803);

			return true;
		}
	};

}	 // namespace Acheron
