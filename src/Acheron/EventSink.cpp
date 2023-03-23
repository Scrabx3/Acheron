#include "Acheron/EventSink.h"

#include "Acheron/Defeat.h"

namespace Acheron
{
	EventResult EventHandler::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
  {
		if (!a_event || a_event->holder->IsNot(RE::FormType::ActorCharacter))
			return EventResult::kContinue;

		auto source = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());
		// constexpr std::array events{ "MTState", "IdleStop", "JumpLandEnd" };
		if (a_event->tag == "MTState") {
			source->NotifyAnimationGraph("BleedoutStart");
		}
		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*)
	{
		if (!a_event || a_event->newState != RE::ACTOR_COMBAT_STATE::kNone)
			return EventResult::kContinue;

		const auto actor = a_event->actor->As<RE::Actor>();
		if (!actor || !actor->Is3DLoaded() || actor->IsDead())
			return EventResult::kContinue;

		if (!Defeat::IsDefeated(actor)) {
			auto w = worn_cache.find(actor->GetFormID());
			if (w == worn_cache.end())
				return EventResult::kContinue;

			auto& p = actor->currentProcess;
			if (p && p->InHighProcess()) {
				auto high = p->high;
				if (high)
					high->reEquipArmorTimer = 1.0f;
			}
		}
		worn_cache.erase(actor->GetFormID());
		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*)
	{
		if (!a_event->loaded) {
			worn_cache.erase(a_event->formID);

      auto w = Defeat::Victims.find(a_event->formID);
			if (w != Defeat::Victims.end() && w->second.allow_recovery) {
				auto victim = RE::TESForm::LookupByID<RE::Actor>(a_event->formID);
        if (victim && Settings::bKdFollowerUnload && victim->IsPlayerTeammate()) {
					SKSE::GetTaskInterface()->AddTask([victim]() {
						Defeat::RescueActor(victim, true);
					});
				}
			}
		}
		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*)
  {
		if (!a_event)
			return EventResult::kContinue;

		if (Defeat::Pacified.erase(a_event->formID) > 0) {
			Defeat::Victims.erase(a_event->formID);
			logger::info("Form {:X} has been deleted and thus removed from Pacified & Victim lists", a_event->formID);
		}
		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
		if (!a_event || !a_event->actorDying)
      return EventResult::kContinue;

		auto dying = a_event->actorDying->As<RE::Actor>();
		if (!dying)
			return EventResult::kContinue;

		if (Defeat::IsDefeated(dying)) {
			Defeat::RescueActor(dying, true);
			logger::info("Form {:X} died and has been removed from the Victims list", dying->GetFormID());
		} else if (Defeat::IsPacified(dying)) {
			Defeat::UndoPacify(dying);
			logger::info("Form {:X} died and has been removed from the Pacified list", dying->GetFormID());
		}
		worn_cache.erase(dying->GetFormID());

		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESResetEvent* a_event, RE::BSTEventSource<RE::TESResetEvent>*)
	{
    if (!a_event || !a_event->object)
      return EventResult::kContinue;

    auto reset = a_event->object->As<RE::Actor>();
    if (!reset)
      return EventResult::kContinue;

		if (Defeat::IsDefeated(reset)) {
			Defeat::RescueActor(reset, true);
			logger::info("Form {:X} has been reset and thus been removed from the Victims list", reset->GetFormID());
		} else if (Defeat::IsPacified(reset)) {
			Defeat::UndoPacify(reset);
			logger::info("Form {:X} has been reset and thus been removed from the Pacified list", reset->GetFormID());
		}

		return EventResult::kContinue;
  }


	EventResult EventHandler::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
	{
		if (!a_event)
			return EventResult::kContinue;

		const auto intfcStr = RE::InterfaceStrings::GetSingleton();
		const auto ui = RE::UI::GetSingleton();
		if (ui->IsMenuOpen(intfcStr->console) || ui->GameIsPaused())
			return EventResult::kContinue;

		const auto controlMap = RE::ControlMap::GetSingleton();
		if (!controlMap->IsMovementControlsEnabled())
			return EventResult::kContinue;

		for (const RE::InputEvent* input = *a_event; input; input = input->next) {
			const auto event = input->AsButtonEvent();
			if (!event || !event->IsDown())
				continue;

			RE::BSInputDevice* device;
			auto dmanager = RE::BSInputDeviceManager::GetSingleton();
			switch (input->GetDevice()) {
			case RE::INPUT_DEVICE::kGamepad:
				device = dmanager->GetGamepad();
				break;
			case RE::INPUT_DEVICE::kKeyboard:
				device = dmanager->GetKeyboard();
				break;
			case RE::INPUT_DEVICE::kVirtualKeyboard:
				device = dmanager->GetVirtualKeyboard();
				break;
			default:
				device = nullptr;
				break;
			}

			if (!device)
				continue;

			const auto idcode = event->GetIDCode();
			if (Settings::iHunterPrideKey > -1 && idcode == static_cast<uint32_t>(Settings::iHunterPrideKey)) {
				if (Settings::iHunterPrideKeyMod > -1 && !device->IsPressed(static_cast<uint32_t>(Settings::iHunterPrideKeyMod)))
					continue;

				auto player = RE::PlayerCharacter::GetSingleton();
				if (player->HasSpell(GameForms::HunterPride)) {
					player->RemoveSpell(GameForms::HunterPride);
					RE::DebugNotification("$Achr_HunterPrideRemoved");
				} else {
					player->AddSpell(GameForms::HunterPride);
					RE::DebugNotification("$Achr_HunterPrideAdded");
				}
				break;
			}
		}
		return EventResult::kContinue;
	}

}	 // namespace Acheron
