#include "Acheron/EventSink.h"

#include "Acheron/Defeat.h"
#include "Acheron/Hooks/Processing.h"
#include "Acheron/Resolution.h"

namespace Acheron
{
	EventResult EventHandler::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
  {
		if (!a_event || a_event->holder->IsNot(RE::FormType::ActorCharacter))
			return EventResult::kContinue;

		auto source = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());
		// constexpr std::array events{ "MTState", "IdleStop", "JumpLandEnd" };
		if (a_event->tag == "MTState") {
			if (auto process = source->currentProcess) {
				process->PlayIdle(source, GameForms::BleedoutStart, source);
			} else {
				source->NotifyAnimationGraph("BleedoutStart");
			}
		}
		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESActorLocationChangeEvent* a_event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*)
	{
		if (!a_event || !a_event->actor || !a_event->actor->IsPlayerRef())
			return EventResult::kContinue;
		// Dont process playing loading game
		if (!RE::PlayerCharacter::GetSingleton()->playerFlags.isLoading)
			if (RE::UI::GetSingleton()->IsMenuOpen(RE::InterfaceStrings::GetSingleton()->loadingMenu))
				return EventResult::kContinue;
		// Dont process loading child/parent locs
		if (a_event->newLoc && a_event->oldLoc)
			if (a_event->newLoc->IsChild(a_event->oldLoc) || a_event->newLoc->IsParent(a_event->oldLoc))
				return EventResult::kContinue;

		std::vector<RE::Actor*> rescuethis{};
		Defeat::ForEachVictim([&](RE::FormID a_formid, std::shared_ptr<Defeat::VictimData> a_data) {
			RE::Actor* victim = RE::TESForm::LookupByID<RE::Actor>(a_formid);
			if (!victim || !victim->Is3DLoaded()) {
				a_data->mark_for_recovery = true;
			} else if (Settings::iFollowerRescue == 2 && victim->IsPlayerTeammate()) {
				if (!victim->Is3DLoaded() || victim->GetPosition().GetDistance(a_event->actor->GetPosition()) > 4096)
					rescuethis.push_back(victim);
			}
			return Defeat::VictimVistor::Continue;
		});
		for (auto& rescue : rescuethis) {
			Defeat::RescueActor(rescue, true);
		}
		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*)
	{
		if (!a_event || a_event->newState != RE::ACTOR_COMBAT_STATE::kNone)
			return EventResult::kContinue;

		const auto worn = ExtractCachedArmor(a_event->actor->GetFormID());
		const auto actor = a_event->actor->As<RE::Actor>();
		if (actor && actor->Is3DLoaded() && !actor->IsDead() && !Defeat::IsDefeated(actor)) {
			if (!worn.empty()) {
				const auto em = RE::ActorEquipManager::GetSingleton();
				for (auto& gear : worn) {
					em->EquipObject(actor, gear);
				}
			}
		}
		return EventResult::kContinue;
	}

	EventResult EventHandler::ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*)
  {
		if (a_event)
			Defeat::Delete(a_event->formID);

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
		if (!a_event || (Settings::iHunterPrideKey == -1 && Settings::iSurrenderKey == -1))
			return EventResult::kContinue;

		const auto intfcStr = RE::InterfaceStrings::GetSingleton();
		const auto ui = RE::UI::GetSingleton();
		if (ui->IsMenuOpen(intfcStr->console) || ui->GameIsPaused())
			return EventResult::kContinue;

		const auto controlMap = RE::ControlMap::GetSingleton();
		if (!Acheron::IsMovementControlsEnabled(controlMap))
			return EventResult::kContinue;

		const auto getKey = [&](int key) {
			if (key >= SKSE::InputMap::kMacro_GamepadOffset) {
				return SKSE::InputMap::GamepadKeycodeToMask(key);
			} else {
				return static_cast<uint32_t>(key);
			}
		};

		const auto hpkey = getKey(Settings::iHunterPrideKey);
		const auto surkey = getKey(Settings::iSurrenderKey);
		for (const RE::InputEvent* input = *a_event; input; input = input->next) {
			const auto event = input->AsButtonEvent();
			if (!event || !event->IsDown()) {
				continue;
			}
			const auto idcode = event->GetIDCode();
			if (idcode == hpkey || idcode == surkey) {
				if (Settings::iHunterPrideKeyMod > -1) {
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
					if (device) {
						const auto modkey = getKey(Settings::iHunterPrideKeyMod);
						if (!device->IsPressed(modkey)) {
							continue;
						}
					}
				}
				auto player = RE::PlayerCharacter::GetSingleton();
				if (idcode == hpkey) {
					if (player->HasSpell(GameForms::HunterPride)) {
						player->RemoveSpell(GameForms::HunterPride);
						RE::DebugNotification("$Achr_HunterPrideRemoved");
					} else {
						player->AddSpell(GameForms::HunterPride);
						RE::DebugNotification("$Achr_HunterPrideAdded");
					}
				} else if (idcode == surkey) {
					const auto agr = Processing::AggressorInfo(nullptr, player);
					if (!agr.actor) {
						logger::info("Failed to find a valid aggressor for Surrender");
						RE::DebugNotification("$Achr_SurrenderNoAggressor");
						return EventResult::kContinue;
					}
					const auto memberList = Resolution::BuildMemberList(player, agr.actor, Resolution::Type::Surrender);
					if (!Resolution::SelectQuest(Resolution::Type::Surrender, player, memberList, player->IsInCombat(), false)) {
						logger::info("Failed to select a quest for Surrender");
						RE::DebugNotification("$Achr_SurrenderNoQuest");
					}
				}
				break;
			}
		}
		return EventResult::kContinue;
	}

	void EventHandler::CacheWornArmor(const RE::FormID a_form, RE::TESObjectARMO* a_armor)
	{
		std::scoped_lock _lock{cache_lock};
		worn_cache[a_form].push_back(a_armor);
	}

	std::vector<RE::TESObjectARMO*> EventHandler::ExtractCachedArmor(const RE::FormID a_form)
	{
		std::scoped_lock _lock{cache_lock};
		if (auto node = worn_cache.extract(a_form); !node.empty())
			return std::move(node.mapped());
		return {};
	}

}	 // namespace Acheron
