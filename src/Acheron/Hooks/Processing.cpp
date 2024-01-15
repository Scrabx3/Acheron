#include "Acheron/Hooks/Processing.h"

#include "Acheron/Defeat.h"
#include "Acheron/Resolution.h"
#include "Serialization/EventManager.h"

namespace Acheron
{
	bool Processing::RegisterDefeat(RE::Actor* a_victim, RE::Actor* a_aggressor)
	{
		logger::info("Aggressor {} -> Register Defeat for Victim {}", a_aggressor->GetFormID(), a_victim->GetFormID());
		assert(a_victim != a_aggressor);
		if (a_aggressor->IsCommandedActor()) {
			auto tmp = a_aggressor->GetCommandingActor().get();
			if (tmp) {
				logger::info("Aggressor {} is summon, using Summoner {} as Aggressor", a_aggressor->GetFormID(), tmp->GetFormID());
				a_aggressor = tmp;
			} else {
				logger::warn("Aggressor {} is summon but no Summoner found? Abandon", a_aggressor->GetFormID());
				return false;
			}
		}
		const auto& process = a_victim->currentProcess;
		const auto middlehigh = process ? process->middleHigh : nullptr;
		if (middlehigh) {
			for (auto& commandedActorData : middlehigh->commandedActors) {
				const auto summon = commandedActorData.activeEffect;
				if (!summon)
					summon->Dispel(true);
			}
		}
		if (!a_victim->IsPlayerRef() && Settings::bNotifyDefeat) {
			std::string base = fmt::format("{} has been defeated by {}", a_victim->GetDisplayFullName(), a_aggressor->GetDisplayFullName());
			if (Settings::bNotifyColored) {
				base = fmt::format("<font color = '{}'>{}</font color>", Settings::rNotifyColor, base);
			}
			RE::DebugNotification(base.c_str());
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		switch (GetDefeatType(a_aggressor)) {
		case DefeatResult::Defeat:
			Defeat::DefeatActor(a_victim);
			if (Settings::ConsequenceEnabled && a_victim->IsPlayerRef()) {
				if (Random::draw<float>(0, 99.9f) < Settings::fMidCombatBlackout) {
					if (CreateResolution(player, a_aggressor, true))
						Defeat::DisableRecovery(true);
				}
			}
			break;
		case DefeatResult::Resolution:
			{
				Defeat::DefeatActor(a_victim);
				if (!Settings::ConsequenceEnabled)
					break;

				if (a_victim == player || Defeat::IsDefeated(player)) {
					if (CreateResolution(player, a_aggressor, false)) {
						Defeat::DisableRecovery(true);
						break;
					} else if (Settings::DoesPlayerAutoRecover()) {
						break;
					}
					std::thread([player]() {
						std::this_thread::sleep_for(6s);
						SKSE::GetTaskInterface()->AddTask([player]() {
							Defeat::RescueDelayed(player, false);
						});
					}).detach();
				} else {
					CreateResolution(a_victim, a_aggressor, false);
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}

	Processing::DefeatResult Processing::GetDefeatType(RE::Actor* a_aggressor)
	{
		if (a_aggressor->IsPlayerRef())
			return DefeatResult::Defeat;

		auto agrzone = a_aggressor->GetCombatGroup();
		if (!agrzone || agrzone->members.empty()) {
			logger::warn("Aggressor = {} has no Combat Group, Abandon", a_aggressor->GetFormID());
			return DefeatResult::Cancel;
		}
		const auto validtarget = [](const RE::ActorPtr ptr) -> bool {
			return ptr && !ptr->IsCommandedActor() && ptr->Is3DLoaded() && !ptr->IsDead() && !Defeat::IsDamageImmune(ptr.get());
		};
		std::set<RE::FormID> targets{};	 // avoid duplicates
		for (auto& e : agrzone->targets) {
			const auto& target = e.targetHandle.get();
			if (!target)
				continue;
			if (validtarget(target))
				targets.insert(target->formID);

			const auto& targetgroup = target->GetCombatGroup();
			if (!targetgroup)
				continue;
			for (const auto& member : targetgroup->members) {
				if (const auto t = member.memberHandle.get(); validtarget(t))
					targets.insert(target->formID);
			}
		}
		logger::info("Aggressor {} has {} targets", a_aggressor->GetFormID(), targets.size());
		switch (targets.size()) {
		case 0:
			return DefeatResult::Cancel;
		case 1:
			return DefeatResult::Resolution;
		default:
			return DefeatResult::Defeat;
		}
	}

	bool Processing::CreateResolution(RE::Actor* a_victim, RE::Actor* a_victoire, bool a_incombat)
	{
		using Type = Resolution::Type;

		const auto processLists = RE::ProcessLists::GetSingleton();
		const auto GuardDiaFac = RE::TESForm::LookupByID<RE::TESFaction>(0x0002BE3B);
		const auto isguard = [&GuardDiaFac](RE::Actor* ptr) { return ptr->IsGuard() && ptr->IsInFaction(GuardDiaFac); };
		const auto gethostile = [&a_victim](RE::Actor* actor) {
			if (actor->IsHostileToActor(a_victim)) {
				return true;
			}
			const auto target = actor->currentCombatTarget.get();
			return target ? target.get() == a_victim || !target->IsHostileToActor(a_victim) || a_victim->IsPlayerRef() && target->IsPlayerTeammate() : false;
		};

		auto type = !a_victim->IsPlayerRef()			 ? Type::NPC :
								a_victoire->IsPlayerTeammate() ? Type::Follower :
								!gethostile(a_victoire)				 ? Type::Civilian :
								isguard(a_victoire)						 ? Type::Guard :
																								 Type::Hostile;
		std::vector<RE::Actor*> memberlist{ a_victoire };
		for (auto& e : processLists->highActorHandles) {
			auto actor = e.get().get();
			if (!actor || actor == a_victoire || actor->IsDead() || !actor->Is3DLoaded() || actor->IsHostileToActor(a_victoire))
				continue;
			if (!actor->IsInCombat() && !Defeat::IsDamageImmune(actor))
				continue;

			switch (type) {
			case Type::Follower:
				if (actor->IsPlayerTeammate())
					memberlist.push_back(actor);
				break;
			case Type::Hostile:
				if (gethostile(actor)) {
					// if a guard allied with enemy, let them sort out the situation
					if (isguard(actor)) {
						memberlist.clear();
						memberlist.push_back(actor);
						type = Type::Guard;
					} else {
						memberlist.push_back(actor);
					}
				}
				break;
			case Type::Civilian:
				if (!gethostile(actor))
					memberlist.push_back(actor);
				break;
			case Type::Guard:
				if (isguard(actor))
					memberlist.push_back(actor);
				break;
			case Type::NPC:
				if (actor->IsHostileToActor(a_victim))
					memberlist.push_back(actor);
				break;
			}
		}
		// List fully build. Request a Post Combat Quest & start it
		if (Resolution::SelectQuest(type, a_victim, memberlist, a_incombat)) {
			Serialization::EventManager::GetSingleton()->_playerdeathevent.QueueEvent();
			return true;
		}
		return false;
	}
}
