#pragma once

#include "Serialization/EventManager.h"

namespace Papyrus
{
	using VM = RE::BSScript::IVirtualMachine;

#define REGISTER(type)                          \
	if (!obj) {                                   \
		a_vm->TraceStack("obj is none", a_stackID); \
		return;                                     \
	}                                             \
	Serialization::EventManager::GetSingleton()->type.Register(obj);
#define UNREGISTER(type)                        \
	if (!obj) {                                   \
		a_vm->TraceStack("obj is none", a_stackID); \
		return;                                     \
	}                                             \
	Serialization::EventManager::GetSingleton()->type.Unregister(obj);


	inline void RegisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) {	REGISTER(_actordefeated); }
	inline void UnregisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) { UNREGISTER(_actordefeated); }
	inline void RegisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { REGISTER(_actordefeated); }
	inline void UnregisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { UNREGISTER(_actordefeated); }
	inline void RegisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { REGISTER(_actordefeated); }
	inline void UnregisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { UNREGISTER(_actordefeated); }

	inline void RegisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) { REGISTER(_actorrescued); }
	inline void UnregisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) { UNREGISTER(_actorrescued); }
	inline void RegisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { REGISTER(_actorrescued); }
	inline void UnregisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { UNREGISTER(_actorrescued); }
	inline void RegisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { REGISTER(_actorrescued); }
	inline void UnregisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { UNREGISTER(_actorrescued); }
	
	inline void RegisterForPlayerDeathEvent(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) { REGISTER(_playerdeathevent); }
	inline void UnregisterForPlayerDeathEvent(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) { UNREGISTER(_playerdeathevent); }
	inline void RegisterForPlayerDeathEvent_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { REGISTER(_playerdeathevent); }
	inline void UnregisterForPlayerDeathEvent_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { UNREGISTER(_playerdeathevent); }
	inline void RegisterForPlayerDeathEvent_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { REGISTER(_playerdeathevent); }
	inline void UnregisterForPlayerDeathEvent_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { UNREGISTER(_playerdeathevent); }

	inline void RegisterForHunterPrideSelect(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) { REGISTER(_hunterprideselect); }
	inline void UnregisterForHunterPrideSelect(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* obj) { UNREGISTER(_hunterprideselect); }
	inline void RegisterForHunterPrideSelect_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { REGISTER(_hunterprideselect); }
	inline void UnregisterForHunterPrideSelect_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* obj) { UNREGISTER(_hunterprideselect); }
	inline void RegisterForHunterPrideSelect_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { REGISTER(_hunterprideselect); }
	inline void UnregisterForHunterPrideSelect_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* obj) { UNREGISTER(_hunterprideselect); }

	inline bool RegisterEvents(VM* vm)
	{
		vm->RegisterFunction("RegisterForActorDefeated", "Acheron", RegisterForActorDefeated, true);
		vm->RegisterFunction("UnregisterForActorDefeated", "Acheron", UnregisterForActorDefeated, true);
		vm->RegisterFunction("RegisterForActorDefeated_Alias", "Acheron", RegisterForActorDefeated_Alias, true);
		vm->RegisterFunction("UnregisterForActorDefeated_Alias", "Acheron", UnregisterForActorDefeated_Alias, true);
		vm->RegisterFunction("RegisterForActorDefeated_MgEff", "Acheron", RegisterForActorDefeated_MgEff, true);
		vm->RegisterFunction("UnregisterForActorDefeated_MgEff", "Acheron", UnregisterForActorDefeated_MgEff, true);

		vm->RegisterFunction("RegisterForActorRescued", "Acheron", RegisterForActorRescued, true);
		vm->RegisterFunction("UnregisterForActorRescued", "Acheron", UnregisterForActorRescued, true);
		vm->RegisterFunction("RegisterForActorRescued_Alias", "Acheron", RegisterForActorRescued_Alias, true);
		vm->RegisterFunction("UnregisterForActorRescued_Alias", "Acheron", UnregisterForActorRescued_Alias, true);
		vm->RegisterFunction("RegisterForActorRescued_MgEff", "Acheron", RegisterForActorRescued_MgEff, true);
		vm->RegisterFunction("UnregisterForActorRescued_MgEff", "Acheron", UnregisterForActorRescued_MgEff, true);

		vm->RegisterFunction("RegisterForHunterPrideSelect", "Acheron", RegisterForHunterPrideSelect, true);
		vm->RegisterFunction("UnregisterForHunterPrideSelect", "Acheron", UnregisterForHunterPrideSelect, true);
		vm->RegisterFunction("RegisterForHunterPrideSelect_Alias", "Acheron", RegisterForHunterPrideSelect_Alias, true);
		vm->RegisterFunction("UnregisterForHunterPrideSelect_Alias", "Acheron", UnregisterForHunterPrideSelect_Alias, true);
		vm->RegisterFunction("RegisterForHunterPrideSelect_MgEff", "Acheron", RegisterForHunterPrideSelect_MgEff, true);
		vm->RegisterFunction("UnregisterForHunterPrideSelect_MgEff", "Acheron", UnregisterForHunterPrideSelect_MgEff, true);

		return true;
	}
}	 // namespace Papyrus
