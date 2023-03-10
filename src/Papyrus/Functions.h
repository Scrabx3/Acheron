#pragma once

#include "Acheron/Resolution.h"
#include "Acheron/Defeat.h"
// #include "Acheron/Interface/GameMenu.h"
#include "Acheron/Interface/HunterPride.h"
#include "Acheron/Misc.h"

namespace Papyrus
{	namespace Status
	{
		inline void DisableProcessing(RE::StaticFunctionTag*, bool a_disable) { Settings::ProcessingEnabled = !a_disable; }
		inline void DisableConsequence(RE::StaticFunctionTag*, bool a_disable) { Settings::ConsequenceEnabled = !a_disable; }
		inline bool IsProcessingDisabled(RE::StaticFunctionTag*) { return !Settings::ProcessingEnabled; }
		inline bool IsConsequenceDisabled(RE::StaticFunctionTag*) { return !Settings::ConsequenceEnabled; }

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(DisableProcessing, "Acheron");
			REGISTERFUNC(IsProcessingDisabled, "Acheron");
			REGISTERFUNC(DisableConsequence, "Acheron");
			REGISTERFUNC(IsConsequenceDisabled, "Acheron");
		}
	}	 // namespace Status

	namespace Defeat
	{
		void DefeatActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		void RescueActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool undo_pacify);
		void PacifyActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		void ReleaseActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		bool IsDefeated(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		bool IsPacified(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);

		std::vector<RE::Actor*> GetDefeated(RE::StaticFunctionTag*, bool a_loadedonly);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(DefeatActor, "Acheron");
			REGISTERFUNC(RescueActor, "Acheron");
			REGISTERFUNC(PacifyActor, "Acheron");
			REGISTERFUNC(ReleaseActor, "Acheron");
			REGISTERFUNC(IsDefeated, "Acheron");
			REGISTERFUNC(IsPacified, "Acheron");

			REGISTERFUNC(GetDefeated, "Acheron");
		}
	}

	namespace ObjectRef
	{
		void SetLinkedRef(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword);
		void RemoveAllItems(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* transferfrom, RE::TESObjectREFR* transferto, bool excludeworn);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(SetLinkedRef, "Acheron");
			REGISTERFUNC(RemoveAllItems, "Acheron");
		}
	}

	namespace Actor
	{
		std::vector<RE::TESObjectARMO*> GetWornArmor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, uint32_t ignoredmasks);
		std::vector<RE::TESObjectARMO*> StripActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, uint32_t ignoredmasks);
		RE::AlchemyItem* GetMostEfficientPotion(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::TESObjectREFR* container);
		std::vector<RE::Actor*> GetFollowers(RE::StaticFunctionTag*);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(GetWornArmor, "Acheron");
			REGISTERFUNC(StripActor, "Acheron");
			REGISTERFUNC(GetMostEfficientPotion, "Acheron");
			REGISTERFUNC(GetFollowers, "Acheron");
		}
	}

	namespace Utility
	{
		inline void PrintConsole(RE::StaticFunctionTag*, RE::BSFixedString a_msg) { Acheron::PrintConsole(a_msg.c_str()); }
	
		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(PrintConsole, "Acheron");
		}
	}

	namespace Interface
	{
		bool AddOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, std::string a_name, std::string a_url, std::string a_condition);
		bool RemoveOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
		bool HasOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);

		void OpenHunterPrideMenu(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::Actor* a_target);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(AddOption, "Acheron");
			REGISTERFUNC(RemoveOption, "Acheron");
			REGISTERFUNC(HasOption, "Acheron");

			REGISTERFUNC(OpenHunterPrideMenu, "AcheronHunterPride");
		}
	}

	inline bool RegisterFuncs(VM* a_vm)
	{
		Status::Register(a_vm);
		Defeat::Register(a_vm);
		ObjectRef::Register(a_vm);
		Actor::Register(a_vm);
		Utility::Register(a_vm);
		Interface::Register(a_vm);

		logger::info("Registered Papyrus Functions");
		return true;
	}
}  // namespace Papyrus
