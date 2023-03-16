#pragma once

namespace Acheron
{
	struct GameForms
	{
#define LOOKUPFORM(form, formid)                                                                                           \
	form = RE::TESDataHandler::GetSingleton()->LookupForm<std::remove_pointer<decltype(form)>::type>(formid, "Acheron.esl"); \
	if (!form)                                                                                                               \
		return false;

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
			LOOKUPFORM(Defeated, 0x801);
			LOOKUPFORM(Pacified, 0x802);

			LOOKUPFORM(DefaultCommon, 0x807);
			LOOKUPFORM(DefaultGuard, 0x80C);

			LOOKUPFORM(HunterPride, 0x809);
			LOOKUPFORM(HunterPrideEffect, 0x808);

			LOOKUPFORM(BlankPackage, 0x806);

			LOOKUPFORM(IgnoredFaction, 0x82B);

			LOOKUPFORM(InteractionPerk, 0x805);

			return true;
		}
	};

}	 // namespace Acheron
