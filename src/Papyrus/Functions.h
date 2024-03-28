#pragma once

#include "Acheron/Misc.h"
#include "Acheron/Validation.h"

namespace Papyrus
{	namespace Status
	{
		inline void DisableProcessing(RE::StaticFunctionTag*, bool a_disable) { Settings::ProcessingEnabled = !a_disable; }
		inline void DisableConsequence(RE::StaticFunctionTag*, bool a_disable) { Settings::ConsequenceEnabled = !a_disable; }
		inline bool IsProcessingDisabled(RE::StaticFunctionTag*) { return !Settings::ProcessingEnabled; }
		inline bool IsConsequenceDisabled(RE::StaticFunctionTag*) { return !Settings::ConsequenceEnabled; }
		inline bool IsTeleportAllowed(RE::StaticFunctionTag*) { return Acheron::Validation::AllowTeleport(); }

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(DisableProcessing, "Acheron");
			REGISTERFUNC(IsProcessingDisabled, "Acheron");
			REGISTERFUNC(DisableConsequence, "Acheron");
			REGISTERFUNC(IsConsequenceDisabled, "Acheron");
			REGISTERFUNC(IsTeleportAllowed, "Acheron");
		}
	}	 // namespace Status

	namespace Defeat
	{
		void DefeatActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
		void RescueActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, bool undo_pacify);
		void PacifyActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
		void ReleaseActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
		bool IsDefeated(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
		bool IsPacified(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);

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

	namespace Interface
	{
		int AddOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, std::string a_name, std::string a_url, std::string a_condition);
		bool RemoveOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
		bool HasOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
		int GetOptionID(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);

		void OpenHunterPrideMenu(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::Actor* a_target);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(AddOption, "Acheron");
			REGISTERFUNC(RemoveOption, "Acheron");
			REGISTERFUNC(HasOption, "Acheron");
			REGISTERFUNC(GetOptionID, "Acheron");

			REGISTERFUNC(OpenHunterPrideMenu, "AcheronHunterPride");
		}
	}
	namespace ObjectRef
	{
		void SetLinkedRef(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword);
		std::vector<RE::TESForm*> GetItemsByKeywords(
			VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_container, std::vector<RE::BGSKeyword*> a_kywds, int32_t a_minvalue, bool a_qstitms);
		void RemoveAllItems(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_from, RE::TESObjectREFR* a_to, bool a_excludeworn);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(SetLinkedRef, "Acheron");
			REGISTERFUNC(GetItemsByKeywords, "Acheron");
			REGISTERFUNC(RemoveAllItems, "Acheron");
		}
	}

	namespace Actor
	{
		std::vector<RE::TESObjectARMO*> GetWornArmor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, uint32_t ignoredmasks);
		std::vector<RE::TESObjectARMO*> StripActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, uint32_t ignoredmasks);
		RE::AlchemyItem* GetMostEfficientPotion(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::TESObjectREFR* container);
		std::vector<RE::Actor*> GetFollowers(RE::StaticFunctionTag*);
		std::string GetRaceType(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(GetWornArmor, "Acheron");
			REGISTERFUNC(StripActor, "Acheron");
			REGISTERFUNC(GetMostEfficientPotion, "Acheron");
			REGISTERFUNC(GetFollowers, "Acheron");
			REGISTERFUNC(GetRaceType, "Acheron");
		}
	}

	namespace Utility
	{
		inline void PrintConsole(RE::StaticFunctionTag*, RE::BSFixedString a_msg) { Acheron::PrintConsole(a_msg.c_str()); }
		bool OpenCustomMenu(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string_view a_filepath);
		void CloseCustomMenu(RE::StaticFunctionTag*);
	
		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(PrintConsole, "Acheron");
			REGISTERFUNC(OpenCustomMenu, "Acheron");
			REGISTERFUNC(CloseCustomMenu, "Acheron");
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
