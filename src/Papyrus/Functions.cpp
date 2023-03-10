#include "Papyrus/Functions.h"

#include "Acheron/Animation/Animation.h"
namespace Papyrus
{
	void Defeat::DefeatActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot Defeat a none Actor", a_stackID);
			return;
		}
		SKSE::GetTaskInterface()->AddTask([=]() {
			Acheron::Defeat::DefeatActor(subject);
		});
	}

	void Defeat::RescueActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool undopacify)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot Rescue a none Actor", a_stackID);
			return;
		}
		SKSE::GetTaskInterface()->AddTask([=]() {
			Acheron::Defeat::RescueActor(subject, undopacify);
		});
	}

	void Defeat::PacifyActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot Pacify a none Actor", a_stackID);
			return;
		}
		Acheron::Defeat::Pacify(subject);
	}

	void Defeat::ReleaseActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot reset Pacification. Actor is none.", a_stackID);
			return;
		}
		Acheron::Defeat::UndoPacify(subject);
	}

	bool Defeat::IsDefeated(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot check Defeat Status. Actor is none", a_stackID);
			return false;
		}
		return Acheron::Defeat::IsDefeated(subject);
	}

	bool Defeat::IsPacified(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot check Pacification. Actor is none", a_stackID);
			return false;
		}
		return Acheron::Defeat::IsPacified(subject);
	}

	std::vector<RE::Actor*> Defeat::GetDefeated(RE::StaticFunctionTag*, bool a_loadedonly)
	{
		return Acheron::Defeat::GetAllDefeated(a_loadedonly);
	}

	void ObjectRef::SetLinkedRef(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword)
	{
		if (!object) {
			a_vm->TraceStack("Cannot set Linked Ref. Source is none", a_stackID);
			return;
		}
		object->extraList.SetLinkedRef(target, keyword);
	}


	std::vector<RE::TESObjectARMO*> Actor::GetWornArmor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, uint32_t ignoredmasks)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot get worn armor from a none reference", a_stackID);
			return {};
		}
		return Acheron::GetWornArmor(subject, ignoredmasks);
	}

	std::vector<RE::TESObjectARMO*> Actor::StripActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, uint32_t ignoredmasks)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot strip a none reference", a_stackID);
			return {};
		}
		const auto em = RE::ActorEquipManager::GetSingleton();
		const auto armors = Acheron::GetWornArmor(subject, ignoredmasks);
		std::vector<RE::TESObjectARMO*> ret{};
		for (auto& armor : armors) {
			if (armor->ContainsKeywordString("NoStrip")) {
				continue;
			}
			em->UnequipObject(subject, armor);
			ret.push_back(armor);
		}
		return ret;
	}

	void ObjectRef::RemoveAllItems(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* from, RE::TESObjectREFR* to, bool excludeworn)
	{
		if (!from) {
			a_vm->TraceStack("Cannot remove Items from a none reference", a_stackID);
			return;
		}
		const auto reason = [&]() {
			using REASON = RE::ITEM_REMOVE_REASON;
			if (!to)
				return REASON::kRemove;
			else if (auto actor = to->As<RE::Actor>(); actor)
				return actor->IsPlayerTeammate() ? REASON::kStoreInTeammate : REASON::kSteal;
			else
				return REASON::kStoreInContainer;
		}();

		auto inventory = from->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (!form->GetPlayable() || form->GetName()[0] == '\0')
				continue;
			else if (data.second->IsQuestObject())
				continue;
			else if (data.second->IsWorn()) {
				if (excludeworn) {
					continue;
				}
				const auto kywdform = form->As<RE::BGSKeywordForm>();
				if (kywdform && kywdform->ContainsKeywordString("NoStrip")) {
					continue;
				}
			}
			from->RemoveItem(form, data.first, reason, nullptr, to, 0, 0);
		}
	}

	RE::AlchemyItem* Actor::GetMostEfficientPotion(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::TESObjectREFR* container)
	{
		using Flag = RE::EffectSetting::EffectSettingData::Flag;
		if (!subject) {
			a_vm->TraceStack("Actor is none", a_stackID);
			return nullptr;
		} else if (!container) {
			a_vm->TraceStack("Container Reference is none", a_stackID);
			return nullptr;
		}
		const float tmphp = subject->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);
		const float maxhp = subject->GetPermanentActorValue(RE::ActorValue::kHealth) + tmphp;
		const float missinghp = maxhp - subject->GetActorValue(RE::ActorValue::kHealth);
		RE::AlchemyItem* ret = nullptr;
		float closest = FLT_MAX;
		const auto inventory = container->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (data.first <= 0 || data.second->IsQuestObject() || !form->Is(RE::FormType::AlchemyItem))
				continue;
			const auto potion = form->As<RE::AlchemyItem>();
			if (potion->IsFood())
				continue;

			const float healing = [&potion]() {
				float ret = 0.0f;
				for (auto& e : potion->effects) {
					const auto base = e->baseEffect;
					if (!base)
						continue;

					const auto& effectdata = base->data;
					if (effectdata.flags.any(Flag::kDetrimental, Flag::kHostile)) {
						ret = 0.0f;
						break;
					} else if (effectdata.flags.none(Flag::kRecover)) {
						if (effectdata.primaryAV == RE::ActorValue::kHealth)
							ret += e->effectItem.magnitude;
						else if (effectdata.secondaryAV == RE::ActorValue::kHealth)
							ret += e->effectItem.magnitude * effectdata.secondAVWeight;
					}
				}
				return ret;
			}();
			if (healing > 0.0f) {
				const auto result = fabs(missinghp - healing);
				if (result < closest) {
					closest = result;
					ret = potion;
				}
			}
		}
		return ret;
	}

	std::vector<RE::Actor*> Actor::GetFollowers(RE::StaticFunctionTag*)
	{
		return Acheron::GetFollowers();
	}

	void Interface::OpenHunterPrideMenu(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::Actor* a_target)
	{
		if (!a_target) {
			a_vm->TraceStack("Cannot open hunter pride menu with a null reference as target", a_stackID);
			return;
		}
		Acheron::Interface::HunterPride::SetTarget(a_target);
		Acheron::Interface::HunterPride::Show();
	}

	bool Interface::AddOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, std::string a_name, std::string a_url, std::string a_condition)
	{
		if (a_id.empty()) {
			a_vm->TraceStack("id may not be empty", a_stackID);
			return false;
		}
		return Acheron::Interface::HunterPride::AddOption(a_id, a_condition, a_name, a_url);
	}

	bool Interface::RemoveOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
	{
		if (a_id.empty()) {
			a_vm->TraceStack("id may not be empty", a_stackID);
			return false;
		}
		return Acheron::Interface::HunterPride::RemoveOption(a_id);
	}

	bool Interface::HasOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
	{
		if (a_id.empty()) {
			a_vm->TraceStack("id may not be empty", a_stackID);
			return false;
		}
		return Acheron::Interface::HunterPride::HasOption(a_id);
	}

}	 // namespace Papyrus
