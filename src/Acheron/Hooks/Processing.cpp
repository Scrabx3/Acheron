#include "Acheron/Hooks/Processing.h"

#include "Acheron/Defeat.h"
#include "Acheron/Resolution.h"
#include "Serialization/EventManager.h"

namespace Acheron
{
	Processing::AggressorInfo::AggressorInfo(RE::Actor* a_actor, RE::Actor* a_victim) :
		actor(a_actor), legal(a_actor != nullptr)
	{
		assert(a_victim);
		if (actor && actor->IsCommandedActor()) {
			actor = a_actor->GetCommandingActor().get();
		}
		if (!actor || actor == a_victim) {
			actor = GetNearValidAggressor(a_victim);
		}
	}

	RE::Actor* Processing::GetNearValidAggressor(RE::Actor* a_victim)
	{
		const auto valid = [&](RE::Actor* subject) {
			return subject->IsHostileToActor(a_victim) && subject->IsInCombat();
		};
		std::vector<RE::Actor*> valids{};
		if (const auto player = RE::PlayerCharacter::GetSingleton(); valid(player) && IsHunter(player))
			valids.push_back(player);

		const auto& processLists = RE::ProcessLists::GetSingleton();
		for (auto& pHandle : processLists->highActorHandles) {
			auto potential = pHandle.get().get();
			if (!potential || potential->IsDead() || Defeat::IsDamageImmune(potential) || !valid(potential))
				continue;

			if (const auto group = potential->GetCombatGroup(); group) {
				for (auto& e : group->targets) {
					if (e.targetHandle.get().get() == a_victim) {
						valids.push_back(potential);
						break;
					}
				}
			}
		}
		if (valids.empty()) {
			return nullptr;
		}
		const auto targetposition = a_victim->GetPosition();
		auto distance = valids[0]->GetPosition().GetDistance(targetposition);
		size_t where = 0;
		for (size_t i = 1; i < valids.size(); i++) {
			const auto d = valids[i]->GetPosition().GetDistance(targetposition);
			if (d < distance) {
				distance = d;
				where = i;
			}
		}
		return distance < 4096.0f ? valids[where] : nullptr;
	}

	bool Processing::RegisterDefeat(RE::Actor* a_victim, const AggressorInfo& a_aggressor)
	{
		logger::info("Aggressor {:X} -> Register Defeat for Victim {:X}", a_aggressor ? a_aggressor->GetFormID() : 0, a_victim->GetFormID());
		assert(a_victim);
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
			std::string base = fmt::format("{} has been defeated", a_victim->GetDisplayFullName());
			if (a_aggressor) {
				base = fmt::format("{} by {}", base, a_aggressor->GetDisplayFullName());
			}
			if (Settings::bNotifyColored) {
				base = fmt::format("<font color = '{}'>{}</font color>", Settings::rNotifyColor, base);
			}
			RE::DebugNotification(base.c_str());
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		switch (GetDefeatType(a_aggressor.actor, a_victim)) {
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

				if (a_victim->IsPlayerRef() || Defeat::IsDefeated(player)) {
					if (CreateResolution(player, a_aggressor, false)) {
						Defeat::DisableRecovery(true);
						break;
					} else if (!a_aggressor.legal) {
						// NOTE: This here might not be good to ignore but
						// inconsistency that would be created is likely more
						// severe than the possible issues that could arise
						// return false;
					}
					if (Settings::DoesPlayerAutoRecover()) {
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
		if (Settings::bFolWithPlDefeat && a_victim->IsPlayerRef()) {
			const auto flist = GetFollowers();
			for (auto&& f : flist) {
				Defeat::DefeatActor(f);
			}
		}
		return true;
	}

	Processing::DefeatResult Processing::GetDefeatType(RE::Actor* a_aggressor, RE::Actor* a_victim)
	{
		if (!a_aggressor)
			return DefeatResult::Resolution;

		if (a_aggressor->IsPlayerRef())
			return DefeatResult::Defeat;

		auto agrzone = a_aggressor->GetCombatGroup();
		if (!agrzone || agrzone->members.empty()) {
			logger::warn("Aggressor = {} has no Combat Group, Abandon", a_aggressor->GetFormID());
			return DefeatResult::Cancel;
		}
		const auto validtarget = [&](const RE::ActorPtr ptr) -> bool {
			if (!ptr)
				return false;
			if (a_victim->IsPlayerRef() && ptr->IsPlayerTeammate() && Settings::bFolWithPlDefeat)
				return false;
			return !ptr->IsCommandedActor() && ptr->Is3DLoaded() && !ptr->IsDead() && !Defeat::IsDamageImmune(ptr.get());
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
		logger::info("Aggressor {:X} has {} targets", a_aggressor->GetFormID(), targets.size());
		switch (targets.size()) {
		case 0:
			return DefeatResult::Cancel;
		case 1:
			return DefeatResult::Resolution;
		default:
			return DefeatResult::Defeat;
		}
	}

	bool Processing::CreateResolution(RE::Actor* a_victim, const AggressorInfo& a_victoire, bool a_incombat)
	{
		using Type = Resolution::Type;

		if (!a_victoire) {
			if (Resolution::SelectQuest(Type::Any, a_victim, {}, a_incombat, true)) {
				Serialization::EventManager::GetSingleton()->_playerdeathevent.QueueEvent();
				return true;
			}
			return false;
		}

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

		assert(a_victoire);
		auto type = !a_victim->IsPlayerRef()			 ? Type::NPC :
								a_victoire->IsPlayerTeammate() ? Type::Follower :
								!gethostile(a_victoire.actor)	 ? Type::Civilian :
								isguard(a_victoire.actor)			 ? Type::Guard :
																								 Type::Hostile;
		std::vector<RE::Actor*> memberlist{ a_victoire.actor };
		for (auto& e : processLists->highActorHandles) {
			auto actor = e.get().get();
			if (!actor || actor == a_victoire.actor || actor->IsDead() || !actor->Is3DLoaded() || actor->IsHostileToActor(a_victoire.actor))
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
		if (Resolution::SelectQuest(type, a_victim, memberlist, a_incombat, !a_victoire.legal)) {
			Serialization::EventManager::GetSingleton()->_playerdeathevent.QueueEvent();
			return true;
		}
		return false;
	}
}
