#include "Papyrus/Functions.h"

#include "Acheron/Animation/Animation.h"
#include "Acheron/Interface/CustomMenu.h"
#include "Acheron/Resolution.h"
#include "Acheron/Defeat.h"
#include "Acheron/Interface/HunterPride.h"

namespace Papyrus
{
	void Defeat::DefeatActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot Defeat a none Actor", a_stackID);
			return;
		}
		SKSE::GetTaskInterface()->AddTask([=]() {
			Acheron::Defeat::DefeatActor(a_actor);
		});
	}

	void Defeat::RescueActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, bool undopacify)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot Rescue a none Actor", a_stackID);
			return;
		}
		SKSE::GetTaskInterface()->AddTask([=]() {
			Acheron::Defeat::RescueActor(a_actor, undopacify);
		});
	}

	void Defeat::PacifyActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot Pacify a none Actor", a_stackID);
			return;
		}
		Acheron::Defeat::Pacify(a_actor);
	}

	void Defeat::ReleaseActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot reset Pacification. Actor is none.", a_stackID);
			return;
		}
		Acheron::Defeat::UndoPacify(a_actor);
	}

	bool Defeat::IsDefeated(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot check Defeat Status. Actor is none", a_stackID);
			return false;
		}
		return Acheron::Defeat::IsDefeated(a_actor);
	}

	bool Defeat::IsPacified(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot check Pacification. Actor is none", a_stackID);
			return false;
		}
		return Acheron::Defeat::IsPacified(a_actor);
	}

	std::vector<RE::Actor*> Defeat::GetDefeated(RE::StaticFunctionTag*, bool a_loadedonly)
	{
		return Acheron::Defeat::GetAllDefeated(a_loadedonly);
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

	int Interface::AddOption(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, std::string a_name, std::string a_url, std::string a_condition)
	{
		if (a_id.empty()) {
			a_vm->TraceStack("id may not be empty", a_stackID);
			return false;
		}
		Acheron::ToLower(a_condition);
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

	int Interface::GetOptionID(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
	{
		if (a_id.empty()) {
			a_vm->TraceStack("id may not be empty", a_stackID);
			return false;
		}
		return Acheron::Interface::HunterPride::GetOptionID(a_id);
	}

	void ObjectRef::SetLinkedRef(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword)
	{
		if (!object) {
			a_vm->TraceStack("Cannot set Linked Ref. Source is none", a_stackID);
			return;
		}
		object->extraList.SetLinkedRef(target, keyword);
	}

	std::vector<RE::TESObjectARMO*> Actor::GetWornArmor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, uint32_t ignoredmasks)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot get worn armor from a none reference", a_stackID);
			return {};
		}
		return Acheron::GetWornArmor(a_actor, ignoredmasks);
	}

	std::vector<RE::TESObjectARMO*> Actor::StripActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, uint32_t ignoredmasks)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot strip a none reference", a_stackID);
			return {};
		}
		const auto em = RE::ActorEquipManager::GetSingleton();
		const auto armors = Acheron::GetWornArmor(a_actor, ignoredmasks);
		std::vector<RE::TESObjectARMO*> ret{};
		for (auto& armor : armors) {
			if (armor->ContainsKeywordString("NoStrip")) {
				continue;
			}
			em->UnequipObject(a_actor, armor);
			ret.push_back(armor);
		}
		return ret;
	}

	std::vector<RE::TESForm*> ObjectRef::GetItemsByKeywords(
		VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
		RE::TESObjectREFR* a_container,
		std::vector<RE::BGSKeyword*> a_kywds,
		int32_t a_minvalue,
		bool a_qstitms)
	{
		if (!a_container) {
			a_vm->TraceStack("Cannot retrieve Items from a none reference", a_stackID);
			return {};
		}
		std::vector<RE::TESForm*> ret{};

		auto inventory = a_container->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (!form->GetPlayable() || form->GetName()[0] == '\0')
				continue;
			if (!a_qstitms && data.second->IsQuestObject())
				continue;
			if (form->GetGoldValue() < a_minvalue)
				continue;
			if (!a_kywds.empty() && !form->HasKeywordInArray(a_kywds, false))
				continue;

			ret.push_back(form);
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
			if (data.second->IsQuestObject())
				continue;
			if (data.second->IsWorn()) {
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

	RE::AlchemyItem* Actor::GetMostEfficientPotion(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::TESObjectREFR* container)
	{
		using Flag = RE::EffectSetting::EffectSettingData::Flag;
		if (!a_actor) {
			a_vm->TraceStack("Actor is none", a_stackID);
			return nullptr;
		} else if (!container) {
			a_vm->TraceStack("Container Reference is none", a_stackID);
			return nullptr;
		}
		const float tmphp = a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);
		const float maxhp = a_actor->GetPermanentActorValue(RE::ActorValue::kHealth) + tmphp;
		const float missinghp = maxhp - a_actor->GetActorValue(RE::ActorValue::kHealth);
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

	std::string Actor::GetRaceType(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		if (!a_actor) {
			a_vm->TraceStack("Cannot get racetype from a none reference", a_stackID);
			return ""s;
		}
		return Acheron::Animation::GetRaceType(a_actor);
	}

	bool Utility::OpenCustomMenu(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string_view a_filepath)
	{
		if (a_filepath.empty()) {
			a_vm->TraceStack("File path to swf file is empty", a_stackID);
			return false;
		} else if (!fs::exists(fmt::format("Data\\Interface\\{}.swf", a_filepath))) {
			a_vm->TraceStack("File path does not lead to a valid file", a_stackID);
			return false;
		}

		if (Acheron::Interface::CustomMenu::IsOpen()) {
			return false;
		}

		Acheron::Interface::CustomMenu::SetSwfPath(a_filepath);
		Acheron::Interface::CustomMenu::Show();
		return true;
	}

	void Utility::CloseCustomMenu(RE::StaticFunctionTag*)
	{
		Acheron::Interface::CustomMenu::Hide();
	}

}	 // namespace Papyrus
